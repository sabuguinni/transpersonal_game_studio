#include "DinosaurHerdSystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

UDinosaurHerdProcessor::UDinosaurHerdProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UDinosaurHerdProcessor::ConfigureQueries()
{
    HerdEntityQuery.AddRequirement<FDinosaurHerdFragment>(EMassFragmentAccess::ReadWrite);
    HerdEntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    HerdEntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    HerdEntityQuery.AddOptionalRequirement<FFleeingBehaviorFragment>(EMassFragmentAccess::ReadWrite);
}

void UDinosaurHerdProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
    HerdEntityQuery.ForEachEntityChunk(EntitySubsystem, Context, 
        [this](FMassExecutionContext& Context)
        {
            const auto& HerdList = Context.GetMutableFragmentView<FDinosaurHerdFragment>();
            const auto& TransformList = Context.GetMutableFragmentView<FTransformFragment>();
            const auto& VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
            const auto* FleeingList = Context.GetOptionalMutableFragmentView<FFleeingBehaviorFragment>();

            for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
            {
                FDinosaurHerdFragment& HerdData = HerdList[EntityIndex];
                FTransformFragment& Transform = TransformList[EntityIndex];
                FMassVelocityFragment& Velocity = VelocityList[EntityIndex];

                // Atualizar timer do estado
                HerdData.StateTimer -= Context.GetDeltaTimeSeconds();

                // Verificar se precisa trocar de estado
                if (HerdData.StateTimer <= 0.0f)
                {
                    // Lógica para transição de estados
                    switch (HerdData.CurrentState)
                    {
                        case EHerdState::Grazing:
                            // 20% chance de migrar, 80% continuar pastando
                            if (FMath::RandRange(0.0f, 1.0f) < 0.2f)
                            {
                                HerdData.CurrentState = EHerdState::Migrating;
                                HerdData.StateTimer = FMath::RandRange(30.0f, 120.0f);
                            }
                            else
                            {
                                HerdData.StateTimer = FMath::RandRange(60.0f, 300.0f);
                            }
                            break;

                        case EHerdState::Migrating:
                            // Voltar a pastar após migração
                            HerdData.CurrentState = EHerdState::Grazing;
                            HerdData.StateTimer = FMath::RandRange(120.0f, 600.0f);
                            break;

                        case EHerdState::Fleeing:
                            // Voltar a pastar após fuga
                            HerdData.CurrentState = EHerdState::Grazing;
                            HerdData.StateTimer = FMath::RandRange(60.0f, 180.0f);
                            break;

                        case EHerdState::Drinking:
                            HerdData.CurrentState = EHerdState::Grazing;
                            HerdData.StateTimer = FMath::RandRange(30.0f, 120.0f);
                            break;

                        case EHerdState::Resting:
                            HerdData.CurrentState = EHerdState::Grazing;
                            HerdData.StateTimer = FMath::RandRange(60.0f, 240.0f);
                            break;
                    }
                }

                // Detectar ameaças
                FVector ThreatPosition;
                if (DetectThreat(Transform.GetTransform().GetLocation(), 2000.0f, ThreatPosition))
                {
                    HerdData.CurrentState = EHerdState::Fleeing;
                    HerdData.StateTimer = FMath::RandRange(10.0f, 30.0f);

                    // Adicionar componente de fuga se não existir
                    if (FleeingList)
                    {
                        FFleeingBehaviorFragment& FleeData = (*FleeingList)[EntityIndex];
                        FleeData.ThreatPosition = ThreatPosition;
                        FleeData.FearLevel = 1.0f;
                        FleeData.TimeSinceThreatDetected = 0.0f;
                    }
                }

                // Executar comportamento baseado no estado atual
                switch (HerdData.CurrentState)
                {
                    case EHerdState::Grazing:
                        ProcessGrazingBehavior(Context, HerdData, Transform);
                        break;

                    case EHerdState::Migrating:
                        ProcessMigrationBehavior(Context, HerdData, Transform);
                        break;

                    case EHerdState::Fleeing:
                        if (FleeingList)
                        {
                            ProcessFleeingBehavior(Context, HerdData, (*FleeingList)[EntityIndex], Transform);
                        }
                        break;

                    case EHerdState::Drinking:
                        // Comportamento de beber água (movimento lento em direção à água)
                        break;

                    case EHerdState::Resting:
                        // Comportamento de descanso (movimento mínimo)
                        break;
                }
            }
        });
}

void UDinosaurHerdProcessor::ProcessGrazingBehavior(FMassExecutionContext& Context, FDinosaurHerdFragment& HerdData, FTransformFragment& Transform)
{
    // Movimento lento e aleatório para simular pastagem
    FVector CurrentLocation = Transform.GetTransform().GetLocation();
    
    // Gerar movimento aleatório suave
    static FVector LastRandomDirection = FVector::ForwardVector;
    
    // Pequena chance de mudar direção
    if (FMath::RandRange(0.0f, 1.0f) < 0.1f)
    {
        LastRandomDirection = FMath::VRand();
        LastRandomDirection.Z = 0.0f; // Manter no plano horizontal
        LastRandomDirection.Normalize();
    }
    
    // Aplicar movimento suave
    FVector NewLocation = CurrentLocation + LastRandomDirection * HerdData.PreferredSpeed * 0.3f * Context.GetDeltaTimeSeconds();
    Transform.GetMutableTransform().SetLocation(NewLocation);
}

void UDinosaurHerdProcessor::ProcessMigrationBehavior(FMassExecutionContext& Context, FDinosaurHerdFragment& HerdData, FTransformFragment& Transform)
{
    // Movimento direcionado em direção ao líder da manada
    FVector CurrentLocation = Transform.GetTransform().GetLocation();
    FVector DirectionToLeader = (HerdData.LeaderPosition - CurrentLocation).GetSafeNormal();
    
    // Aplicar forças de manada
    FVector CohesionForce = CalculateCohesionForce(CurrentLocation, HerdData.LeaderPosition, HerdData.CohesionRadius);
    
    // Combinar forças
    FVector FinalDirection = (DirectionToLeader + CohesionForce * 0.5f).GetSafeNormal();
    FVector NewLocation = CurrentLocation + FinalDirection * HerdData.PreferredSpeed * Context.GetDeltaTimeSeconds();
    
    Transform.GetMutableTransform().SetLocation(NewLocation);
}

void UDinosaurHerdProcessor::ProcessFleeingBehavior(FMassExecutionContext& Context, FDinosaurHerdFragment& HerdData, FFleeingBehaviorFragment& FleeData, FTransformFragment& Transform)
{
    FVector CurrentLocation = Transform.GetTransform().GetLocation();
    FVector FleeDirection = (CurrentLocation - FleeData.ThreatPosition).GetSafeNormal();
    
    // Aumentar velocidade durante fuga
    float FleeSpeed = HerdData.PreferredSpeed * 2.0f * FleeData.FearLevel;
    FVector NewLocation = CurrentLocation + FleeDirection * FleeSpeed * Context.GetDeltaTimeSeconds();
    
    Transform.GetMutableTransform().SetLocation(NewLocation);
    
    // Diminuir medo ao longo do tempo
    FleeData.FearLevel = FMath::Max(0.0f, FleeData.FearLevel - Context.GetDeltaTimeSeconds() * 0.5f);
    FleeData.TimeSinceThreatDetected += Context.GetDeltaTimeSeconds();
}

FVector UDinosaurHerdProcessor::CalculateCohesionForce(const FVector& EntityPosition, const FVector& LeaderPosition, float CohesionRadius)
{
    FVector ToLeader = LeaderPosition - EntityPosition;
    float Distance = ToLeader.Size();
    
    if (Distance > CohesionRadius)
    {
        return ToLeader.GetSafeNormal() * (Distance - CohesionRadius) / CohesionRadius;
    }
    
    return FVector::ZeroVector;
}

FVector UDinosaurHerdProcessor::CalculateSeparationForce(const FVector& EntityPosition, const TArray<FVector>& NearbyPositions, float SeparationRadius)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;
    
    for (const FVector& NearbyPosition : NearbyPositions)
    {
        FVector Difference = EntityPosition - NearbyPosition;
        float Distance = Difference.Size();
        
        if (Distance > 0.0f && Distance < SeparationRadius)
        {
            SeparationForce += Difference.GetSafeNormal() / Distance;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce.Normalize();
    }
    
    return SeparationForce;
}

FVector UDinosaurHerdProcessor::CalculateAlignmentForce(const FVector& EntityVelocity, const FVector& AverageVelocity)
{
    return (AverageVelocity - EntityVelocity).GetSafeNormal();
}

bool UDinosaurHerdProcessor::DetectThreat(const FVector& Position, float DetectionRadius, FVector& ThreatPosition)
{
    // Implementar detecção de ameaças (jogador, predadores, etc.)
    // Por enquanto, retorna false - será implementado quando integrar com outros sistemas
    return false;
}

// Implementação do Seasonal Migration Processor
USeasonalMigrationProcessor::USeasonalMigrationProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void USeasonalMigrationProcessor::ConfigureQueries()
{
    MigrationQuery.AddRequirement<FDinosaurHerdFragment>(EMassFragmentAccess::ReadWrite);
    MigrationQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void USeasonalMigrationProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
    // Implementar lógica de migração sazonal
    // Será expandido quando integrar com sistema de clima/estações
}

TArray<FVector> USeasonalMigrationProcessor::GetMigrationPointsForSeason(ESeason CurrentSeason)
{
    switch (CurrentSeason)
    {
        case ESeason::Spring:
            return SpringMigrationPoints;
        case ESeason::Summer:
            return SummerMigrationPoints;
        case ESeason::Autumn:
            return AutumnMigrationPoints;
        case ESeason::Winter:
            return WinterMigrationPoints;
        default:
            return SpringMigrationPoints;
    }
}