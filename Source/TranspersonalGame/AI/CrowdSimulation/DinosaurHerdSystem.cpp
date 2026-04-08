#include "DinosaurHerdSystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassSimulationLOD.h"
#include "Engine/World.h"

UDinosaurHerdProcessor::UDinosaurHerdProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UDinosaurHerdProcessor::ConfigureQueries()
{
    // Query para membros de manada
    HerdMemberQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    HerdMemberQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    HerdMemberQuery.AddRequirement<FDinosaurHerdFragment>(EMassFragmentAccess::ReadWrite);
    HerdMemberQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
    HerdMemberQuery.AddChunkRequirement<FMassSimulationLODFragment>(EMassFragmentAccess::ReadOnly);
    
    // Query para dados compartilhados de manada
    HerdDataQuery.AddRequirement<FHerdDataFragment>(EMassFragmentAccess::ReadWrite);
}

void UDinosaurHerdProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Primeiro, coletamos dados de todas as manadas
    TMap<int32, TArray<FVector>> HerdPositions;
    TMap<int32, TArray<FVector>> HerdVelocities;
    TMap<int32, FHerdDataFragment*> HerdDataMap;
    
    // Coleta posições e velocidades por manada
    HerdMemberQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const auto& TransformList = Context.GetFragmentView<FTransformFragment>();
        const auto& VelocityList = Context.GetFragmentView<FMassVelocityFragment>();
        const auto& HerdList = Context.GetFragmentView<FDinosaurHerdFragment>();
        
        for (int32 i = 0; i < Context.GetNumEntities(); ++i)
        {
            const int32 HerdID = HerdList[i].HerdID;
            if (HerdID >= 0)
            {
                HerdPositions.FindOrAdd(HerdID).Add(TransformList[i].GetTransform().GetLocation());
                HerdVelocities.FindOrAdd(HerdID).Add(VelocityList[i].Value);
            }
        }
    });
    
    // Atualiza dados globais de cada manada
    HerdDataQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        auto& HerdDataList = Context.GetMutableFragmentView<FHerdDataFragment>();
        
        for (int32 i = 0; i < Context.GetNumEntities(); ++i)
        {
            FHerdDataFragment& HerdData = HerdDataList[i];
            // Assumindo que o HerdID está implícito na ordem dos dados
            const int32 HerdID = i;
            
            if (HerdPositions.Contains(HerdID))
            {
                UpdateHerdData(HerdData, HerdPositions[HerdID], HerdVelocities[HerdID]);
                HerdDataMap.Add(HerdID, &HerdData);
            }
        }
    });
    
    // Agora aplica comportamento de manada a cada membro
    HerdMemberQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const auto& TransformList = Context.GetFragmentView<FTransformFragment>();
        const auto& VelocityList = Context.GetFragmentView<FMassVelocityFragment>();
        auto& HerdList = Context.GetMutableFragmentView<FDinosaurHerdFragment>();
        auto& ForceList = Context.GetMutableFragmentView<FMassForceFragment>();
        const auto& LODList = Context.GetChunkFragmentView<FMassSimulationLODFragment>();
        
        const float DeltaTime = Context.GetDeltaTimeSeconds();
        const EMassLOD::Type LOD = LODList.Get().LOD;
        
        // Skip se LOD muito baixo
        if (LOD < EMassLOD::Medium)
        {
            return;
        }
        
        for (int32 i = 0; i < Context.GetNumEntities(); ++i)
        {
            FDinosaurHerdFragment& HerdFragment = HerdList[i];
            const FVector CurrentPosition = TransformList[i].GetTransform().GetLocation();
            const FVector CurrentVelocity = VelocityList[i].Value;
            
            // Atualiza estado do membro
            UpdateHerdMemberState(HerdFragment, CurrentPosition, DeltaTime);
            
            // Calcula forças de manada apenas se parte de uma manada válida
            if (HerdFragment.HerdID >= 0 && HerdDataMap.Contains(HerdFragment.HerdID))
            {
                const FHerdDataFragment* HerdData = HerdDataMap[HerdFragment.HerdID];
                FVector TotalForce = FVector::ZeroVector;
                
                // Força de coesão - aproximar do centro da manada
                if (HerdFragment.CurrentState != EHerdState::Fleeing)
                {
                    TotalForce += CalculateCohesionForce(CurrentPosition, HerdData->HerdCenter, HerdFragment.CohesionStrength);
                }
                
                // Força de alinhamento - seguir direção da manada
                TotalForce += CalculateAlignmentForce(CurrentVelocity, HerdData->AverageDirection, HerdFragment.AlignmentStrength);
                
                // Força de separação - evitar colisões com outros membros
                if (HerdPositions.Contains(HerdFragment.HerdID))
                {
                    TotalForce += CalculateSeparationForce(CurrentPosition, HerdPositions[HerdFragment.HerdID], HerdFragment.SeparationStrength);
                }
                
                // Força de fuga se há ameaça
                if (HerdData->AlertLevel > 0.1f && HerdData->TimeSinceLastThreat < 10.0f)
                {
                    TotalForce += CalculateFleeForce(CurrentPosition, HerdData->LastThreatPosition, FleeRadius);
                }
                
                // Limita força máxima
                if (TotalForce.SizeSquared() > MaxForce * MaxForce)
                {
                    TotalForce = TotalForce.GetSafeNormal() * MaxForce;
                }
                
                // Aplica força
                ForceList[i].Value += TotalForce;
            }
        }
    });
}

FVector UDinosaurHerdProcessor::CalculateCohesionForce(const FVector& Position, const FVector& HerdCenter, float Strength) const
{
    FVector ToCenter = HerdCenter - Position;
    float Distance = ToCenter.Size();
    
    if (Distance > 0.1f)
    {
        // Força mais fraca quando já próximo do centro
        float ForceMultiplier = FMath::Clamp(Distance / 1000.0f, 0.1f, 1.0f);
        return ToCenter.GetSafeNormal() * Strength * ForceMultiplier * 100.0f;
    }
    
    return FVector::ZeroVector;
}

FVector UDinosaurHerdProcessor::CalculateSeparationForce(const FVector& Position, const TArray<FVector>& NearbyPositions, float Strength) const
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;
    
    for (const FVector& OtherPosition : NearbyPositions)
    {
        float Distance = FVector::Dist(Position, OtherPosition);
        
        if (Distance > 0.1f && Distance < NeighborRadius)
        {
            FVector Away = Position - OtherPosition;
            // Força inversamente proporcional à distância
            float ForceMultiplier = (NeighborRadius - Distance) / NeighborRadius;
            SeparationForce += Away.GetSafeNormal() * ForceMultiplier;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        SeparationForce /= Count;
        return SeparationForce * Strength * 200.0f;
    }
    
    return FVector::ZeroVector;
}

FVector UDinosaurHerdProcessor::CalculateAlignmentForce(const FVector& CurrentVelocity, const FVector& HerdDirection, float Strength) const
{
    FVector DesiredVelocity = HerdDirection.GetSafeNormal() * MaxSpeed * 0.7f; // 70% da velocidade máxima
    FVector SteeringForce = DesiredVelocity - CurrentVelocity;
    
    return SteeringForce * Strength;
}

FVector UDinosaurHerdProcessor::CalculateFleeForce(const FVector& Position, const FVector& ThreatPosition, float FleeRadius) const
{
    FVector ToThreat = ThreatPosition - Position;
    float Distance = ToThreat.Size();
    
    if (Distance < FleeRadius && Distance > 0.1f)
    {
        // Força de fuga inversamente proporcional à distância
        float ForceMultiplier = (FleeRadius - Distance) / FleeRadius;
        FVector FleeDirection = -ToThreat.GetSafeNormal();
        return FleeDirection * ForceMultiplier * MaxForce * 2.0f; // Força de fuga é dobrada
    }
    
    return FVector::ZeroVector;
}

void UDinosaurHerdProcessor::UpdateHerdMemberState(FDinosaurHerdFragment& HerdFragment, const FVector& Position, float DeltaTime) const
{
    HerdFragment.StateTimer += DeltaTime;
    
    // Decai stress ao longo do tempo
    HerdFragment.StressLevel = FMath::Max(0.0f, HerdFragment.StressLevel - StressDecayRate * DeltaTime);
    
    // Lógica de transição de estados baseada em stress e tempo
    switch (HerdFragment.CurrentState)
    {
        case EHerdState::Grazing:
            if (HerdFragment.StressLevel > 0.3f)
            {
                HerdFragment.CurrentState = EHerdState::Alert;
                HerdFragment.StateTimer = 0.0f;
            }
            else if (HerdFragment.StateTimer > 30.0f) // 30 segundos pastando
            {
                HerdFragment.CurrentState = EHerdState::Moving;
                HerdFragment.StateTimer = 0.0f;
            }
            break;
            
        case EHerdState::Alert:
            if (HerdFragment.StressLevel > 0.7f)
            {
                HerdFragment.CurrentState = EHerdState::Fleeing;
                HerdFragment.StateTimer = 0.0f;
            }
            else if (HerdFragment.StressLevel < 0.1f && HerdFragment.StateTimer > 5.0f)
            {
                HerdFragment.CurrentState = EHerdState::Grazing;
                HerdFragment.StateTimer = 0.0f;
            }
            break;
            
        case EHerdState::Fleeing:
            if (HerdFragment.StressLevel < 0.2f && HerdFragment.StateTimer > 10.0f)
            {
                HerdFragment.CurrentState = EHerdState::Moving;
                HerdFragment.StateTimer = 0.0f;
            }
            break;
            
        case EHerdState::Moving:
            if (HerdFragment.StressLevel > 0.3f)
            {
                HerdFragment.CurrentState = EHerdState::Alert;
                HerdFragment.StateTimer = 0.0f;
            }
            else if (HerdFragment.StateTimer > 20.0f) // 20 segundos movendo
            {
                HerdFragment.CurrentState = EHerdState::Grazing;
                HerdFragment.StateTimer = 0.0f;
            }
            break;
    }
}

void UDinosaurHerdProcessor::UpdateHerdData(FHerdDataFragment& HerdData, const TArray<FVector>& MemberPositions, const TArray<FVector>& MemberVelocities) const
{
    if (MemberPositions.Num() == 0)
    {
        return;
    }
    
    // Calcula centro da manada
    FVector CenterSum = FVector::ZeroVector;
    for (const FVector& Position : MemberPositions)
    {
        CenterSum += Position;
    }
    HerdData.HerdCenter = CenterSum / MemberPositions.Num();
    
    // Calcula direção média
    FVector DirectionSum = FVector::ZeroVector;
    for (const FVector& Velocity : MemberVelocities)
    {
        if (Velocity.SizeSquared() > 1.0f)
        {
            DirectionSum += Velocity.GetSafeNormal();
        }
    }
    
    if (DirectionSum.SizeSquared() > 0.1f)
    {
        HerdData.AverageDirection = DirectionSum.GetSafeNormal();
    }
    
    // Calcula velocidade média
    float SpeedSum = 0.0f;
    for (const FVector& Velocity : MemberVelocities)
    {
        SpeedSum += Velocity.Size();
    }
    HerdData.AverageSpeed = SpeedSum / MemberVelocities.Num();
    
    // Atualiza contagem de membros
    HerdData.MemberCount = MemberPositions.Num();
    
    // Decai nível de alerta
    HerdData.AlertLevel = FMath::Max(0.0f, HerdData.AlertLevel - AlertDecayRate * 0.016f); // Assumindo ~60fps
    HerdData.TimeSinceLastThreat += 0.016f;
}

// ============================================================================
// PREDATOR DETECTION PROCESSOR
// ============================================================================

UPredatorDetectionProcessor::UPredatorDetectionProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UPredatorDetectionProcessor::ConfigureQueries()
{
    // Query para presas (herbívoros)
    PreyQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    PreyQuery.AddRequirement<FDinosaurHerdFragment>(EMassFragmentAccess::ReadWrite);
    PreyQuery.AddTagRequirement<FDinosaurPreyTag>(EMassFragmentPresence::All);
    
    // Query para predadores
    PredatorQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    PredatorQuery.AddTagRequirement<FDinosaurPredatorTag>(EMassFragmentPresence::All);
}

void UPredatorDetectionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Coleta posições de todos os predadores
    TArray<FVector> PredatorPositions;
    PredatorQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const auto& TransformList = Context.GetFragmentView<FTransformFragment>();
        
        for (int32 i = 0; i < Context.GetNumEntities(); ++i)
        {
            PredatorPositions.Add(TransformList[i].GetTransform().GetLocation());
        }
    });
    
    // Verifica detecção de predadores para cada presa
    PreyQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const auto& TransformList = Context.GetFragmentView<FTransformFragment>();
        auto& HerdList = Context.GetMutableFragmentView<FDinosaurHerdFragment>();
        
        for (int32 i = 0; i < Context.GetNumEntities(); ++i)
        {
            const FVector PreyPosition = TransformList[i].GetTransform().GetLocation();
            DetectNearbyPredators(HerdList[i], PreyPosition, PredatorPositions);
        }
    });
}

void UPredatorDetectionProcessor::DetectNearbyPredators(FDinosaurHerdFragment& HerdFragment, const FVector& Position, const TArray<FVector>& PredatorPositions) const
{
    float ClosestDistance = DetectionRadius;
    FVector ClosestThreat = FVector::ZeroVector;
    bool ThreatDetected = false;
    
    for (const FVector& PredatorPos : PredatorPositions)
    {
        float Distance = FVector::Dist(Position, PredatorPos);
        
        if (Distance < DetectionRadius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestThreat = PredatorPos;
            ThreatDetected = true;
        }
    }
    
    if (ThreatDetected)
    {
        // Aumenta stress baseado na proximidade
        float StressIncrease = (DetectionRadius - ClosestDistance) / DetectionRadius;
        HerdFragment.StressLevel = FMath::Min(1.0f, HerdFragment.StressLevel + StressIncrease * 0.5f);
        
        // TODO: Propagar alerta para outros membros da manada
        // PropagateAlert(EntityManager, HerdFragment.HerdID, ClosestThreat, StressIncrease);
    }
}