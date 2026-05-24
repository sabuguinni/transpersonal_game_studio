#include "Crowd_MassEntityCrowdProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UCrowd_MassEntityCrowdProcessor::UCrowd_MassEntityCrowdProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
    
    MaxEntitiesPerFrame = 100;
    ProcessingTimeLimit = 0.016f;
    GlobalFlockingStrength = 1.0f;
    bUseBiomeAttraction = true;
}

void UCrowd_MassEntityCrowdProcessor::ConfigureQueries()
{
    // Query para entidades com comportamento de flocking
    FlockingQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    FlockingQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    FlockingQuery.AddRequirement<FCrowd_FlockingFragment>(EMassFragmentAccess::ReadWrite);
    FlockingQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadOnly);

    // Query para entidades com comportamento de bioma
    BiomeQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    BiomeQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    BiomeQuery.AddRequirement<FCrowd_BiomeFragment>(EMassFragmentAccess::ReadOnly);
    BiomeQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);

    // Query para comportamento geral
    BehaviorQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    BehaviorQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    BehaviorQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_MassEntityCrowdProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    double CurrentTime = FPlatformTime::Seconds();
    ProcessedEntitiesThisFrame = 0;
    
    // Processar comportamentos em ordem de prioridade
    ProcessFlockingBehavior(EntityManager, Context);
    ProcessBiomeBehavior(EntityManager, Context);
    ProcessGeneralBehavior(EntityManager, Context);
    
    LastExecutionTime = CurrentTime;
}

void UCrowd_MassEntityCrowdProcessor::ProcessFlockingBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    FlockingQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FCrowd_FlockingFragment> FlockingList = Context.GetMutableFragmentView<FCrowd_FlockingFragment>();
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetFragmentView<FCrowd_BehaviorFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities && ProcessedEntitiesThisFrame < MaxEntitiesPerFrame; ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FCrowd_FlockingFragment& Flocking = FlockingList[EntityIndex];
            const FCrowd_BehaviorFragment& Behavior = BehaviorList[EntityIndex];

            // Coletar vizinhos próximos
            TArray<FVector> NeighborLocations;
            TArray<FVector> NeighborVelocities;

            for (int32 OtherIndex = 0; OtherIndex < NumEntities; ++OtherIndex)
            {
                if (OtherIndex == EntityIndex) continue;

                const FVector& OtherLocation = TransformList[OtherIndex].GetTransform().GetLocation();
                const FVector& OtherVelocity = VelocityList[OtherIndex].Value;
                
                float Distance = FVector::Dist(Transform.GetTransform().GetLocation(), OtherLocation);
                
                if (Distance <= Flocking.CohesionRadius)
                {
                    NeighborLocations.Add(OtherLocation);
                    NeighborVelocities.Add(OtherVelocity);
                }
            }

            // Calcular força de flocking
            if (NeighborLocations.Num() > 0)
            {
                FVector FlockingForce = CalculateFlockingForce(
                    Transform.GetTransform().GetLocation(),
                    Velocity.Value,
                    NeighborLocations,
                    NeighborVelocities,
                    Flocking
                );

                Flocking.FlockingForce = FlockingForce * GlobalFlockingStrength;

                // Aplicar força à velocidade
                Velocity.Value += Flocking.FlockingForce * Context.GetDeltaTimeSeconds();
                
                // Limitar velocidade máxima
                if (Velocity.Value.Size() > Behavior.MaxSpeed)
                {
                    Velocity.Value = Velocity.Value.GetSafeNormal() * Behavior.MaxSpeed;
                }
            }

            ProcessedEntitiesThisFrame++;
        }
    });
}

void UCrowd_MassEntityCrowdProcessor::ProcessBiomeBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    if (!bUseBiomeAttraction) return;

    BiomeQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FCrowd_BiomeFragment> BiomeList = Context.GetFragmentView<FCrowd_BiomeFragment>();
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities && ProcessedEntitiesThisFrame < MaxEntitiesPerFrame; ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FCrowd_BiomeFragment& Biome = BiomeList[EntityIndex];
            FCrowd_BehaviorFragment& Behavior = BehaviorList[EntityIndex];

            FVector CurrentLocation = Transform.GetTransform().GetLocation();
            FVector BiomeCenter = GetBiomeCenter(Biome.BiomeType);
            
            // Verificar se está fora do bioma
            float DistanceToBiome = FVector::Dist(CurrentLocation, BiomeCenter);
            
            if (DistanceToBiome > Biome.BiomeRadius)
            {
                // Aplicar força de atração para o bioma
                FVector ToBiome = (BiomeCenter - CurrentLocation).GetSafeNormal();
                FVector BiomeAttraction = ToBiome * Biome.BiomeAttraction * 200.0f; // Força base
                
                Velocity.Value += BiomeAttraction * Context.GetDeltaTimeSeconds();
                
                // Atualizar estado de comportamento
                Behavior.CurrentState = ECrowd_BehaviorState::Returning;
                Behavior.TargetLocation = BiomeCenter;
            }
            else if (Behavior.CurrentState == ECrowd_BehaviorState::Returning)
            {
                // Voltar ao comportamento normal
                Behavior.CurrentState = ECrowd_BehaviorState::Wandering;
            }

            ProcessedEntitiesThisFrame++;
        }
    });
}

void UCrowd_MassEntityCrowdProcessor::ProcessGeneralBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    BehaviorQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities && ProcessedEntitiesThisFrame < MaxEntitiesPerFrame; ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FCrowd_BehaviorFragment& Behavior = BehaviorList[EntityIndex];

            FVector CurrentLocation = Transform.GetTransform().GetLocation();
            
            // Atualizar timer de estado
            Behavior.StateTimer += Context.GetDeltaTimeSeconds();

            // Processar estado atual
            switch (Behavior.CurrentState)
            {
                case ECrowd_BehaviorState::Wandering:
                {
                    // Mudança de direção aleatória a cada 3-8 segundos
                    if (Behavior.StateTimer > UKismetMathLibrary::RandomFloatInRange(3.0f, 8.0f))
                    {
                        FVector RandomDirection = UKismetMathLibrary::RandomUnitVector();
                        RandomDirection.Z = 0.0f; // Manter no plano horizontal
                        
                        Behavior.TargetLocation = CurrentLocation + (RandomDirection * Behavior.WanderRadius);
                        Behavior.StateTimer = 0.0f;
                    }

                    // Mover em direção ao target
                    FVector ToTarget = (Behavior.TargetLocation - CurrentLocation).GetSafeNormal();
                    Velocity.Value += ToTarget * 100.0f * Context.GetDeltaTimeSeconds();
                    break;
                }

                case ECrowd_BehaviorState::Fleeing:
                {
                    // Comportamento de fuga (implementar quando necessário)
                    if (Behavior.StateTimer > 5.0f)
                    {
                        Behavior.CurrentState = ECrowd_BehaviorState::Wandering;
                        Behavior.StateTimer = 0.0f;
                    }
                    break;
                }

                case ECrowd_BehaviorState::Returning:
                {
                    // Retorno ao bioma (já processado em ProcessBiomeBehavior)
                    break;
                }

                default:
                    Behavior.CurrentState = ECrowd_BehaviorState::Wandering;
                    break;
            }

            // Aplicar movimento
            FVector NewLocation = CurrentLocation + (Velocity.Value * Context.GetDeltaTimeSeconds());
            Transform.GetMutableTransform().SetLocation(NewLocation);

            ProcessedEntitiesThisFrame++;
        }
    });
}

FVector UCrowd_MassEntityCrowdProcessor::CalculateFlockingForce(const FVector& EntityLocation, const FVector& EntityVelocity,
    const TArray<FVector>& NeighborLocations, const TArray<FVector>& NeighborVelocities,
    const FCrowd_FlockingFragment& FlockingData)
{
    FVector Separation = CalculateSeparation(EntityLocation, NeighborLocations, FlockingData.SeparationRadius, FlockingData.SeparationWeight);
    FVector Alignment = CalculateAlignment(EntityVelocity, NeighborVelocities, FlockingData.AlignmentWeight);
    FVector Cohesion = CalculateCohesion(EntityLocation, NeighborLocations, FlockingData.CohesionWeight);

    return Separation + Alignment + Cohesion;
}

FVector UCrowd_MassEntityCrowdProcessor::CalculateSeparation(const FVector& EntityLocation, const TArray<FVector>& NeighborLocations, float Radius, float Weight)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;

    for (const FVector& NeighborLocation : NeighborLocations)
    {
        float Distance = FVector::Dist(EntityLocation, NeighborLocation);
        if (Distance < Radius && Distance > 0.0f)
        {
            FVector Diff = (EntityLocation - NeighborLocation).GetSafeNormal();
            Diff /= Distance; // Peso inversamente proporcional à distância
            SeparationForce += Diff;
            Count++;
        }
    }

    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce = SeparationForce.GetSafeNormal() * Weight;
    }

    return SeparationForce;
}

FVector UCrowd_MassEntityCrowdProcessor::CalculateAlignment(const FVector& EntityVelocity, const TArray<FVector>& NeighborVelocities, float Weight)
{
    FVector AverageVelocity = FVector::ZeroVector;
    
    for (const FVector& NeighborVelocity : NeighborVelocities)
    {
        AverageVelocity += NeighborVelocity;
    }

    if (NeighborVelocities.Num() > 0)
    {
        AverageVelocity /= NeighborVelocities.Num();
        FVector AlignmentForce = (AverageVelocity - EntityVelocity).GetSafeNormal() * Weight;
        return AlignmentForce;
    }

    return FVector::ZeroVector;
}

FVector UCrowd_MassEntityCrowdProcessor::CalculateCohesion(const FVector& EntityLocation, const TArray<FVector>& NeighborLocations, float Weight)
{
    FVector CenterOfMass = FVector::ZeroVector;
    
    for (const FVector& NeighborLocation : NeighborLocations)
    {
        CenterOfMass += NeighborLocation;
    }

    if (NeighborLocations.Num() > 0)
    {
        CenterOfMass /= NeighborLocations.Num();
        FVector CohesionForce = (CenterOfMass - EntityLocation).GetSafeNormal() * Weight;
        return CohesionForce;
    }

    return FVector::ZeroVector;
}

FVector UCrowd_MassEntityCrowdProcessor::GetBiomeCenter(ECrowd_BiomeType BiomeType)
{
    switch (BiomeType)
    {
        case ECrowd_BiomeType::Pantano:
            return FVector(-50000, -45000, 0);
        case ECrowd_BiomeType::Floresta:
            return FVector(-45000, 40000, 0);
        case ECrowd_BiomeType::Savana:
            return FVector(0, 0, 0);
        case ECrowd_BiomeType::Deserto:
            return FVector(55000, 0, 0);
        case ECrowd_BiomeType::Montanha:
            return FVector(40000, 50000, 500);
        default:
            return FVector::ZeroVector;
    }
}

bool UCrowd_MassEntityCrowdProcessor::IsInBiome(const FVector& Location, ECrowd_BiomeType BiomeType)
{
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    float BiomeRadius = 15000.0f; // Raio padrão dos biomas
    
    return FVector::Dist(Location, BiomeCenter) <= BiomeRadius;
}