#include "DinosaurCrowdSystem.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"

// ===== DINOSAUR HERD PROCESSOR =====

UDinosaurHerdProcessor::UDinosaurHerdProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UDinosaurHerdProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FDinosaurHerdFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
}

void UDinosaurHerdProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Mapa para agrupar entidades por manada
    TMap<int32, TArray<FMassEntityHandle>> HerdGroups;
    TMap<int32, FVector> HerdCenters;
    TMap<int32, FVector> HerdVelocities;
    TMap<int32, int32> HerdCounts;

    // Primeira passagem: calcular centros e velocidades médias das manadas
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassArchetypeEntityCollection& EntityCollection)
    {
        const auto TransformList = EntityCollection.GetFragmentArrayView<FTransformFragment>();
        const auto VelocityList = EntityCollection.GetFragmentArrayView<FMassVelocityFragment>();
        const auto HerdList = EntityCollection.GetFragmentArrayView<FDinosaurHerdFragment>();

        for (int32 EntityIndex = 0; EntityIndex < EntityCollection.GetNumEntities(); ++EntityIndex)
        {
            const FMassEntityHandle Entity = EntityCollection.GetEntity(EntityIndex);
            const FTransformFragment& Transform = TransformList[EntityIndex];
            const FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FDinosaurHerdFragment& Herd = HerdList[EntityIndex];

            if (Herd.HerdID >= 0)
            {
                HerdGroups.FindOrAdd(Herd.HerdID).Add(Entity);
                HerdCenters.FindOrAdd(Herd.HerdID) += Transform.GetTransform().GetLocation();
                HerdVelocities.FindOrAdd(Herd.HerdID) += Velocity.Value;
                HerdCounts.FindOrAdd(Herd.HerdID)++;
            }
        }
    });

    // Calcular médias
    for (auto& HerdPair : HerdCenters)
    {
        int32 HerdID = HerdPair.Key;
        int32 Count = HerdCounts[HerdID];
        if (Count > 0)
        {
            HerdCenters[HerdID] /= Count;
            HerdVelocities[HerdID] /= Count;
        }
    }

    // Segunda passagem: aplicar forças de manada
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassArchetypeEntityCollection& EntityCollection)
    {
        const auto TransformList = EntityCollection.GetFragmentArrayView<FTransformFragment>();
        auto VelocityList = EntityCollection.GetMutableFragmentArrayView<FMassVelocityFragment>();
        auto HerdList = EntityCollection.GetMutableFragmentArrayView<FDinosaurHerdFragment>();
        const auto SpeciesList = EntityCollection.GetFragmentArrayView<FDinosaurSpeciesFragment>();

        for (int32 EntityIndex = 0; EntityIndex < EntityCollection.GetNumEntities(); ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FDinosaurHerdFragment& Herd = HerdList[EntityIndex];
            const FDinosaurSpeciesFragment& Species = SpeciesList[EntityIndex];

            if (Herd.HerdID >= 0 && HerdCenters.Contains(Herd.HerdID))
            {
                FVector Position = Transform.GetTransform().GetLocation();
                FVector HerdCenter = HerdCenters[Herd.HerdID];
                FVector AverageVelocity = HerdVelocities[Herd.HerdID];

                // Atualizar posição do líder da manada
                Herd.HerdLeaderPosition = HerdCenter;

                // Calcular forças de manada
                FVector CohesionForce = CalculateCohesionForce(Position, HerdCenter, Herd.CohesionRadius);
                FVector AlignmentForce = CalculateAlignmentForce(Velocity.Value, AverageVelocity);

                // Aplicar forças baseadas no estado da manada
                FVector TotalForce = FVector::ZeroVector;
                
                switch (Herd.HerdState)
                {
                case 0: // Grazing - movimento lento e disperso
                    TotalForce = CohesionForce * 0.3f + AlignmentForce * 0.2f;
                    break;
                case 1: // Migrating - movimento coordenado
                    TotalForce = CohesionForce * Herd.CohesionForce + AlignmentForce * Herd.AlignmentForce;
                    break;
                case 2: // Fleeing - movimento rápido e coordenado
                    TotalForce = CohesionForce * 1.5f + AlignmentForce * 2.0f;
                    break;
                case 3: // Drinking - movimento em direção à água
                    TotalForce = CohesionForce * 0.8f + AlignmentForce * 0.5f;
                    break;
                }

                // Aplicar força limitada pela velocidade máxima da espécie
                Velocity.Value += TotalForce * Context.GetDeltaTimeSeconds();
                if (Velocity.Value.Size() > Species.MaxSpeed)
                {
                    Velocity.Value = Velocity.Value.GetSafeNormal() * Species.MaxSpeed;
                }
            }
        }
    });
}

FVector UDinosaurHerdProcessor::CalculateCohesionForce(const FVector& Position, const FVector& HerdCenter, float CohesionRadius)
{
    FVector ToCenter = HerdCenter - Position;
    float Distance = ToCenter.Size();
    
    if (Distance > CohesionRadius)
    {
        return ToCenter.GetSafeNormal() * (Distance - CohesionRadius) * 0.1f;
    }
    
    return FVector::ZeroVector;
}

FVector UDinosaurHerdProcessor::CalculateSeparationForce(const FVector& Position, const TArray<FVector>& NearbyPositions, float SeparationRadius)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;

    for (const FVector& NearbyPos : NearbyPositions)
    {
        FVector Diff = Position - NearbyPos;
        float Distance = Diff.Size();
        
        if (Distance > 0 && Distance < SeparationRadius)
        {
            SeparationForce += Diff.GetSafeNormal() / Distance;
            Count++;
        }
    }

    if (Count > 0)
    {
        SeparationForce /= Count;
    }

    return SeparationForce;
}

FVector UDinosaurHerdProcessor::CalculateAlignmentForce(const FVector& Velocity, const FVector& AverageHerdVelocity)
{
    return (AverageHerdVelocity - Velocity) * 0.1f;
}

// ===== DINOSAUR NEEDS PROCESSOR =====

UDinosaurNeedsProcessor::UDinosaurNeedsProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UDinosaurNeedsProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FDinosaurNeedsFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FDinosaurHerdFragment>(EMassFragmentAccess::ReadWrite);
}

void UDinosaurNeedsProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    float DeltaTime = Context.GetDeltaTimeSeconds();

    EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassArchetypeEntityCollection& EntityCollection)
    {
        auto NeedsList = EntityCollection.GetMutableFragmentArrayView<FDinosaurNeedsFragment>();
        auto HerdList = EntityCollection.GetMutableFragmentArrayView<FDinosaurHerdFragment>();

        for (int32 EntityIndex = 0; EntityIndex < EntityCollection.GetNumEntities(); ++EntityIndex)
        {
            FDinosaurNeedsFragment& Needs = NeedsList[EntityIndex];
            FDinosaurHerdFragment& Herd = HerdList[EntityIndex];

            // Aumentar necessidades ao longo do tempo
            Needs.HungerLevel = FMath::Clamp(Needs.HungerLevel + HungerIncreaseRate * DeltaTime, 0.0f, 1.0f);
            Needs.ThirstLevel = FMath::Clamp(Needs.ThirstLevel + ThirstIncreaseRate * DeltaTime, 0.0f, 1.0f);
            Needs.FatigueLevel = FMath::Clamp(Needs.FatigueLevel + FatigueIncreaseRate * DeltaTime, 0.0f, 1.0f);

            // Atualizar timers
            Needs.TimeSinceLastMeal += DeltaTime;
            Needs.TimeSinceLastDrink += DeltaTime;

            // Mudar estado da manada baseado nas necessidades
            if (Needs.ThirstLevel > 0.7f)
            {
                Herd.HerdState = 3; // Drinking
            }
            else if (Needs.HungerLevel > 0.6f)
            {
                Herd.HerdState = 0; // Grazing
            }
            else if (Needs.FatigueLevel < 0.3f && Needs.HungerLevel < 0.4f)
            {
                Herd.HerdState = 1; // Migrating
            }
        }
    });
}

// ===== DINOSAUR PREDATOR AVOIDANCE PROCESSOR =====

UDinosaurPredatorAvoidanceProcessor::UDinosaurPredatorAvoidanceProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Avoidance;
}

void UDinosaurPredatorAvoidanceProcessor::ConfigureQueries()
{
    PreyQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    PreyQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    PreyQuery.AddRequirement<FDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    PreyQuery.AddRequirement<FDinosaurHerdFragment>(EMassFragmentAccess::ReadWrite);
    PreyQuery.AddTagRequirement<FMassTag>("Herbivore", EMassFragmentPresence::All);

    PredatorQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    PredatorQuery.AddTagRequirement<FMassTag>("Carnivore", EMassFragmentPresence::All);
}

void UDinosaurPredatorAvoidanceProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Coletar posições de predadores
    TArray<FVector> PredatorPositions;
    PredatorQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassArchetypeEntityCollection& EntityCollection)
    {
        const auto TransformList = EntityCollection.GetFragmentArrayView<FTransformFragment>();
        
        for (int32 EntityIndex = 0; EntityIndex < EntityCollection.GetNumEntities(); ++EntityIndex)
        {
            PredatorPositions.Add(TransformList[EntityIndex].GetTransform().GetLocation());
        }
    });

    // Processar presas
    PreyQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassArchetypeEntityCollection& EntityCollection)
    {
        const auto TransformList = EntityCollection.GetFragmentArrayView<FTransformFragment>();
        auto VelocityList = EntityCollection.GetMutableFragmentArrayView<FMassVelocityFragment>();
        const auto SpeciesList = EntityCollection.GetFragmentArrayView<FDinosaurSpeciesFragment>();
        auto HerdList = EntityCollection.GetMutableFragmentArrayView<FDinosaurHerdFragment>();

        for (int32 EntityIndex = 0; EntityIndex < EntityCollection.GetNumEntities(); ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FDinosaurSpeciesFragment& Species = SpeciesList[EntityIndex];
            FDinosaurHerdFragment& Herd = HerdList[EntityIndex];

            FVector Position = Transform.GetTransform().GetLocation();
            FVector FleeDirection = FVector::ZeroVector;
            bool PredatorDetected = false;

            // Verificar predadores próximos
            for (const FVector& PredatorPos : PredatorPositions)
            {
                float Distance = FVector::Dist(Position, PredatorPos);
                if (Distance < Species.PredatorDetectionRange)
                {
                    FVector AwayFromPredator = (Position - PredatorPos).GetSafeNormal();
                    float Urgency = 1.0f - (Distance / Species.PredatorDetectionRange);
                    FleeDirection += AwayFromPredator * Urgency;
                    PredatorDetected = true;
                }
            }

            if (PredatorDetected)
            {
                // Mudar estado da manada para fuga
                Herd.HerdState = 2; // Fleeing
                
                // Aplicar força de fuga
                FleeDirection.Normalize();
                Velocity.Value += FleeDirection * Species.MaxSpeed * FleeSpeedMultiplier * Context.GetDeltaTimeSeconds();
                
                // Limitar velocidade
                if (Velocity.Value.Size() > Species.MaxSpeed * FleeSpeedMultiplier)
                {
                    Velocity.Value = Velocity.Value.GetSafeNormal() * Species.MaxSpeed * FleeSpeedMultiplier;
                }
            }
        }
    });
}