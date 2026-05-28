#include "Crowd_MassHerdSystem.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "NavigationSystem.h"

UCrowd_MassHerdSystem::UCrowd_MassHerdSystem()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UCrowd_MassHerdSystem::ConfigureQueries()
{
    HerdEntityQuery.AddRequirement<FCrowd_HerdFragment>(EMassFragmentAccess::ReadWrite);
    HerdEntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    HerdEntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    HerdEntityQuery.AddRequirement<FCrowd_DinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    HerdEntityQuery.AddConstSharedRequirement<FCrowd_HerdBehaviorParameters>();
}

void UCrowd_MassHerdSystem::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
    if (!GetWorld())
    {
        return;
    }

    // Obter parâmetros de comportamento de manada
    const FCrowd_HerdBehaviorParameters* HerdParams = Context.GetConstSharedFragment<FCrowd_HerdBehaviorParameters>();
    if (!HerdParams)
    {
        return;
    }

    const float DeltaTime = GetWorld()->GetDeltaSeconds();
    const float CurrentTime = GetWorld()->GetTimeSeconds();

    HerdEntityQuery.ForEachEntityChunk(EntitySubsystem, Context, 
        [this, DeltaTime, CurrentTime, HerdParams](FMassExecutionContext& QueryContext)
        {
            const TArrayView<FCrowd_HerdFragment> HerdFragments = QueryContext.GetMutableFragmentView<FCrowd_HerdFragment>();
            const TArrayView<FMassVelocityFragment> VelocityFragments = QueryContext.GetMutableFragmentView<FMassVelocityFragment>();
            const TArrayView<FTransformFragment> TransformFragments = QueryContext.GetMutableFragmentView<FTransformFragment>();
            const TConstArrayView<FCrowd_DinosaurSpeciesFragment> SpeciesFragments = QueryContext.GetFragmentView<FCrowd_DinosaurSpeciesFragment>();

            const int32 NumEntities = QueryContext.GetNumEntities();

            for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
            {
                FCrowd_HerdFragment& HerdFragment = HerdFragments[EntityIndex];
                FMassVelocityFragment& VelocityFragment = VelocityFragments[EntityIndex];
                FTransformFragment& TransformFragment = TransformFragments[EntityIndex];
                const FCrowd_DinosaurSpeciesFragment& SpeciesFragment = SpeciesFragments[EntityIndex];

                ProcessHerdBehavior(HerdFragment, VelocityFragment, TransformFragment, SpeciesFragment, 
                                  *HerdParams, DeltaTime, CurrentTime, EntityIndex);
            }
        });
}

void UCrowd_MassHerdSystem::ProcessHerdBehavior(FCrowd_HerdFragment& HerdFragment, 
                                               FMassVelocityFragment& VelocityFragment,
                                               FTransformFragment& TransformFragment,
                                               const FCrowd_DinosaurSpeciesFragment& SpeciesFragment,
                                               const FCrowd_HerdBehaviorParameters& HerdParams,
                                               float DeltaTime, float CurrentTime, int32 EntityIndex)
{
    const FVector CurrentLocation = TransformFragment.GetTransform().GetLocation();
    
    // Actualizar estado da manada
    UpdateHerdState(HerdFragment, SpeciesFragment, CurrentTime);
    
    // Calcular força de coesão (manter-se junto da manada)
    FVector CohesionForce = CalculateCohesionForce(CurrentLocation, HerdFragment, HerdParams);
    
    // Calcular força de separação (evitar colisões)
    FVector SeparationForce = CalculateSeparationForce(CurrentLocation, HerdFragment, HerdParams);
    
    // Calcular força de alinhamento (seguir direcção da manada)
    FVector AlignmentForce = CalculateAlignmentForce(VelocityFragment.Value, HerdFragment, HerdParams);
    
    // Calcular força de liderança (seguir líder da manada)
    FVector LeadershipForce = CalculateLeadershipForce(CurrentLocation, HerdFragment, HerdParams);
    
    // Calcular força de fuga (evitar predadores)
    FVector AvoidanceForce = CalculateAvoidanceForce(CurrentLocation, HerdFragment, HerdParams);
    
    // Combinar todas as forças
    FVector TotalForce = CohesionForce * HerdParams.CohesionWeight +
                        SeparationForce * HerdParams.SeparationWeight +
                        AlignmentForce * HerdParams.AlignmentWeight +
                        LeadershipForce * HerdParams.LeadershipWeight +
                        AvoidanceForce * HerdParams.AvoidanceWeight;
    
    // Aplicar força baseada na espécie
    float SpeciesMultiplier = GetSpeciesMovementMultiplier(SpeciesFragment.Species);
    TotalForce *= SpeciesMultiplier;
    
    // Limitar força máxima
    if (TotalForce.SizeSquared() > HerdParams.MaxForce * HerdParams.MaxForce)
    {
        TotalForce = TotalForce.GetSafeNormal() * HerdParams.MaxForce;
    }
    
    // Actualizar velocidade
    VelocityFragment.Value += TotalForce * DeltaTime;
    
    // Limitar velocidade máxima
    float MaxSpeed = HerdParams.MaxSpeed * SpeciesMultiplier;
    if (VelocityFragment.Value.SizeSquared() > MaxSpeed * MaxSpeed)
    {
        VelocityFragment.Value = VelocityFragment.Value.GetSafeNormal() * MaxSpeed;
    }
    
    // Actualizar posição
    FVector NewLocation = CurrentLocation + VelocityFragment.Value * DeltaTime;
    TransformFragment.GetMutableTransform().SetLocation(NewLocation);
    
    // Actualizar rotação para olhar na direcção do movimento
    if (!VelocityFragment.Value.IsNearlyZero())
    {
        FRotator NewRotation = VelocityFragment.Value.Rotation();
        TransformFragment.GetMutableTransform().SetRotation(NewRotation.Quaternion());
    }
}

void UCrowd_MassHerdSystem::UpdateHerdState(FCrowd_HerdFragment& HerdFragment, 
                                           const FCrowd_DinosaurSpeciesFragment& SpeciesFragment, 
                                           float CurrentTime)
{
    // Actualizar tempo desde última actualização de estado
    float TimeSinceLastUpdate = CurrentTime - HerdFragment.LastStateUpdateTime;
    
    if (TimeSinceLastUpdate > 1.0f) // Actualizar estado a cada segundo
    {
        // Determinar novo estado baseado em factores ambientais
        ECrowd_HerdState NewState = DetermineHerdState(HerdFragment, SpeciesFragment);
        
        if (NewState != HerdFragment.CurrentState)
        {
            HerdFragment.CurrentState = NewState;
            HerdFragment.StateChangeTime = CurrentTime;
        }
        
        HerdFragment.LastStateUpdateTime = CurrentTime;
    }
}

ECrowd_HerdState UCrowd_MassHerdSystem::DetermineHerdState(const FCrowd_HerdFragment& HerdFragment, 
                                                          const FCrowd_DinosaurSpeciesFragment& SpeciesFragment)
{
    // Lógica simplificada para determinar estado da manada
    // Em implementação completa, isto seria baseado em:
    // - Proximidade de predadores
    // - Disponibilidade de recursos
    // - Hora do dia
    // - Condições meteorológicas
    
    if (HerdFragment.ThreatLevel > 0.7f)
    {
        return ECrowd_HerdState::Fleeing;
    }
    else if (HerdFragment.ThreatLevel > 0.3f)
    {
        return ECrowd_HerdState::Alert;
    }
    else if (SpeciesFragment.Species == ECrowd_DinosaurSpecies::Herbivore)
    {
        return ECrowd_HerdState::Grazing;
    }
    else
    {
        return ECrowd_HerdState::Wandering;
    }
}

FVector UCrowd_MassHerdSystem::CalculateCohesionForce(const FVector& CurrentLocation, 
                                                     const FCrowd_HerdFragment& HerdFragment, 
                                                     const FCrowd_HerdBehaviorParameters& HerdParams)
{
    // Força que puxa o indivíduo para o centro da manada
    FVector CenterOfMass = HerdFragment.HerdCenter;
    FVector ToCenterForce = CenterOfMass - CurrentLocation;
    
    float Distance = ToCenterForce.Size();
    if (Distance > HerdParams.CohesionRadius)
    {
        return ToCenterForce.GetSafeNormal() * HerdParams.CohesionStrength;
    }
    
    return FVector::ZeroVector;
}

FVector UCrowd_MassHerdSystem::CalculateSeparationForce(const FVector& CurrentLocation, 
                                                       const FCrowd_HerdFragment& HerdFragment, 
                                                       const FCrowd_HerdBehaviorParameters& HerdParams)
{
    // Força que empurra o indivíduo para longe de vizinhos muito próximos
    // Implementação simplificada - numa versão completa usaria spatial hashing
    FVector SeparationForce = FVector::ZeroVector;
    
    // Simular vizinhos próximos baseado na densidade da manada
    if (HerdFragment.LocalDensity > HerdParams.MinSeparationDistance)
    {
        // Criar força de repulsão aleatória
        FVector RandomDirection = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ).GetSafeNormal();
        
        SeparationForce = RandomDirection * HerdParams.SeparationStrength;
    }
    
    return SeparationForce;
}

FVector UCrowd_MassHerdSystem::CalculateAlignmentForce(const FVector& CurrentVelocity, 
                                                      const FCrowd_HerdFragment& HerdFragment, 
                                                      const FCrowd_HerdBehaviorParameters& HerdParams)
{
    // Força que alinha a velocidade com a velocidade média da manada
    FVector AverageVelocity = HerdFragment.AverageVelocity;
    FVector AlignmentForce = AverageVelocity - CurrentVelocity;
    
    return AlignmentForce.GetSafeNormal() * HerdParams.AlignmentStrength;
}

FVector UCrowd_MassHerdSystem::CalculateLeadershipForce(const FVector& CurrentLocation, 
                                                       const FCrowd_HerdFragment& HerdFragment, 
                                                       const FCrowd_HerdBehaviorParameters& HerdParams)
{
    // Força que puxa o indivíduo na direcção do líder da manada
    if (HerdFragment.bHasLeader)
    {
        FVector ToLeaderForce = HerdFragment.LeaderPosition - CurrentLocation;
        float Distance = ToLeaderForce.Size();
        
        if (Distance > HerdParams.LeaderFollowDistance)
        {
            return ToLeaderForce.GetSafeNormal() * HerdParams.LeadershipStrength;
        }
    }
    
    return FVector::ZeroVector;
}

FVector UCrowd_MassHerdSystem::CalculateAvoidanceForce(const FVector& CurrentLocation, 
                                                      const FCrowd_HerdFragment& HerdFragment, 
                                                      const FCrowd_HerdBehaviorParameters& HerdParams)
{
    // Força que empurra o indivíduo para longe de ameaças
    FVector AvoidanceForce = FVector::ZeroVector;
    
    if (HerdFragment.ThreatLevel > 0.0f && !HerdFragment.ThreatDirection.IsZero())
    {
        // Fugir na direcção oposta à ameaça
        AvoidanceForce = -HerdFragment.ThreatDirection * HerdParams.AvoidanceStrength * HerdFragment.ThreatLevel;
    }
    
    return AvoidanceForce;
}

float UCrowd_MassHerdSystem::GetSpeciesMovementMultiplier(ECrowd_DinosaurSpecies Species)
{
    switch (Species)
    {
        case ECrowd_DinosaurSpecies::SmallHerbivore:
            return 1.2f; // Mais rápidos
        case ECrowd_DinosaurSpecies::LargeHerbivore:
            return 0.8f; // Mais lentos
        case ECrowd_DinosaurSpecies::SmallCarnivore:
            return 1.5f; // Muito rápidos
        case ECrowd_DinosaurSpecies::LargeCarnivore:
            return 1.0f; // Velocidade normal
        case ECrowd_DinosaurSpecies::Herbivore:
        default:
            return 1.0f; // Velocidade base
    }
}