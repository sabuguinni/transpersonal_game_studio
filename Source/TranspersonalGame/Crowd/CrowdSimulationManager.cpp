// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements herd/pack behavior for prehistoric crowd simulation

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "Engine/TargetPoint.h"
#include "NavigationSystem.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

// ============================================================
// UCrowdSimulationManager — UActorComponent implementation
// ============================================================

UCrowdSimulationManager::UCrowdSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz crowd tick for performance

    MaxCrowdAgents = 50000;
    ActiveAgentCount = 0;
    CrowdTickInterval = 0.1f;
    LODNearDistance = 2000.0f;
    LODMidDistance = 8000.0f;
    LODFarDistance = 20000.0f;
    bCrowdSystemActive = true;
    bDebugDrawEnabled = false;
}

void UCrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize agent pools for each herd type
    InitializeAgentPool(ECrowd_AgentType::RaptorPackMember, 20);
    InitializeAgentPool(ECrowd_AgentType::TriceratopsHerdMember, 15);
    InitializeAgentPool(ECrowd_AgentType::BrachiosaurusHerdMember, 10);
    InitializeAgentPool(ECrowd_AgentType::HumanTribal, 50);

    // Start crowd update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CrowdUpdateTimer,
            this,
            &UCrowdSimulationManager::UpdateCrowdBehavior,
            CrowdTickInterval,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized. MaxAgents=%d"), MaxCrowdAgents);
}

void UCrowdSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bCrowdSystemActive) return;

    // Update LOD for all active agents based on player distance
    UpdateAgentLOD();

    if (bDebugDrawEnabled)
    {
        DrawDebugCrowdState();
    }
}

void UCrowdSimulationManager::InitializeAgentPool(ECrowd_AgentType AgentType, int32 PoolSize)
{
    FCrowd_AgentPool NewPool;
    NewPool.AgentType = AgentType;
    NewPool.MaxPoolSize = PoolSize;
    NewPool.ActiveCount = 0;

    // Set behavior parameters per species
    switch (AgentType)
    {
        case ECrowd_AgentType::RaptorPackMember:
            NewPool.PatrolRadius = 1500.0f;
            NewPool.FlockingRadius = 400.0f;
            NewPool.AlertRadius = 2000.0f;
            NewPool.MovementSpeed = 600.0f;
            NewPool.BehaviorState = ECrowd_BehaviorState::Patrolling;
            break;

        case ECrowd_AgentType::TriceratopsHerdMember:
            NewPool.PatrolRadius = 2000.0f;
            NewPool.FlockingRadius = 800.0f;
            NewPool.AlertRadius = 1500.0f;
            NewPool.MovementSpeed = 350.0f;
            NewPool.BehaviorState = ECrowd_BehaviorState::Grazing;
            break;

        case ECrowd_AgentType::BrachiosaurusHerdMember:
            NewPool.PatrolRadius = 3000.0f;
            NewPool.FlockingRadius = 1200.0f;
            NewPool.AlertRadius = 2500.0f;
            NewPool.MovementSpeed = 200.0f;
            NewPool.BehaviorState = ECrowd_BehaviorState::Grazing;
            break;

        case ECrowd_AgentType::HumanTribal:
            NewPool.PatrolRadius = 1000.0f;
            NewPool.FlockingRadius = 300.0f;
            NewPool.AlertRadius = 1200.0f;
            NewPool.MovementSpeed = 250.0f;
            NewPool.BehaviorState = ECrowd_BehaviorState::Patrolling;
            break;

        default:
            NewPool.PatrolRadius = 1000.0f;
            NewPool.FlockingRadius = 500.0f;
            NewPool.AlertRadius = 1500.0f;
            NewPool.MovementSpeed = 300.0f;
            NewPool.BehaviorState = ECrowd_BehaviorState::Idle;
            break;
    }

    AgentPools.Add(AgentType, NewPool);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Pool initialized: Type=%d PoolSize=%d"), (int32)AgentType, PoolSize);
}

void UCrowdSimulationManager::UpdateCrowdBehavior()
{
    if (!bCrowdSystemActive) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Get player location for threat detection
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;

    // Update each agent pool behavior
    for (auto& PoolPair : AgentPools)
    {
        FCrowd_AgentPool& Pool = PoolPair.Value;
        UpdatePoolBehavior(Pool, PlayerLocation);
    }

    // Update active agent count
    ActiveAgentCount = 0;
    for (auto& PoolPair : AgentPools)
    {
        ActiveAgentCount += PoolPair.Value.ActiveCount;
    }
}

void UCrowdSimulationManager::UpdatePoolBehavior(FCrowd_AgentPool& Pool, const FVector& PlayerLocation)
{
    if (Pool.ActiveCount == 0) return;

    // Determine threat level from player proximity
    float ThreatRadius = Pool.AlertRadius;
    bool bPlayerNearby = false;

    for (const AActor* Agent : Pool.ActiveAgents)
    {
        if (!Agent) continue;
        float DistToPlayer = FVector::Dist(Agent->GetActorLocation(), PlayerLocation);
        if (DistToPlayer < ThreatRadius)
        {
            bPlayerNearby = true;
            break;
        }
    }

    // State machine transition
    if (bPlayerNearby)
    {
        switch (Pool.AgentType)
        {
            case ECrowd_AgentType::RaptorPackMember:
                // Raptors become aggressive — switch to flanking behavior
                if (Pool.BehaviorState != ECrowd_BehaviorState::Attacking)
                {
                    Pool.BehaviorState = ECrowd_BehaviorState::Alerted;
                    ExecuteFlankingFormation(Pool, PlayerLocation);
                }
                break;

            case ECrowd_AgentType::TriceratopsHerdMember:
                // Triceratops form defensive circle
                if (Pool.BehaviorState == ECrowd_BehaviorState::Grazing)
                {
                    Pool.BehaviorState = ECrowd_BehaviorState::Alerted;
                    ExecuteDefensiveFormation(Pool, PlayerLocation);
                }
                break;

            case ECrowd_AgentType::BrachiosaurusHerdMember:
                // Brachiosaurus flee from threats
                Pool.BehaviorState = ECrowd_BehaviorState::Fleeing;
                break;

            case ECrowd_AgentType::HumanTribal:
                // Humans scatter and hide
                Pool.BehaviorState = ECrowd_BehaviorState::Fleeing;
                break;

            default:
                Pool.BehaviorState = ECrowd_BehaviorState::Alerted;
                break;
        }
    }
    else
    {
        // Return to default behavior when player is gone
        switch (Pool.AgentType)
        {
            case ECrowd_AgentType::RaptorPackMember:
                Pool.BehaviorState = ECrowd_BehaviorState::Patrolling;
                break;
            case ECrowd_AgentType::TriceratopsHerdMember:
            case ECrowd_AgentType::BrachiosaurusHerdMember:
                Pool.BehaviorState = ECrowd_BehaviorState::Grazing;
                break;
            default:
                Pool.BehaviorState = ECrowd_BehaviorState::Idle;
                break;
        }
    }
}

void UCrowdSimulationManager::ExecuteFlankingFormation(FCrowd_AgentPool& Pool, const FVector& TargetLocation)
{
    // Raptor pack flanking: spread agents in arc around target
    int32 NumAgents = Pool.ActiveAgents.Num();
    if (NumAgents == 0) return;

    float AngleStep = 360.0f / FMath::Max(NumAgents, 1);
    float FlankRadius = Pool.FlockingRadius * 2.0f;

    for (int32 i = 0; i < NumAgents; i++)
    {
        AActor* Agent = Pool.ActiveAgents[i];
        if (!Agent) continue;

        float Angle = FMath::DegreesToRadians(i * AngleStep);
        FVector FlankOffset(
            FMath::Cos(Angle) * FlankRadius,
            FMath::Sin(Angle) * FlankRadius,
            0.0f
        );

        FVector FlankTarget = TargetLocation + FlankOffset;
        Pool.FormationTargets.Add(FlankTarget);
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] RaptorPack flanking formation: %d agents"), NumAgents);
}

void UCrowdSimulationManager::ExecuteDefensiveFormation(FCrowd_AgentPool& Pool, const FVector& ThreatLocation)
{
    // Triceratops defensive circle: face outward from herd center
    int32 NumAgents = Pool.ActiveAgents.Num();
    if (NumAgents == 0) return;

    // Calculate herd centroid
    FVector Centroid = FVector::ZeroVector;
    for (const AActor* Agent : Pool.ActiveAgents)
    {
        if (Agent) Centroid += Agent->GetActorLocation();
    }
    Centroid /= FMath::Max(NumAgents, 1);

    float CircleRadius = Pool.FlockingRadius;
    float AngleStep = 360.0f / FMath::Max(NumAgents, 1);

    for (int32 i = 0; i < NumAgents; i++)
    {
        float Angle = FMath::DegreesToRadians(i * AngleStep);
        FVector CirclePos = Centroid + FVector(
            FMath::Cos(Angle) * CircleRadius,
            FMath::Sin(Angle) * CircleRadius,
            0.0f
        );
        Pool.FormationTargets.Add(CirclePos);
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] TrikeHerd defensive circle: %d agents around (%.0f,%.0f)"),
        NumAgents, Centroid.X, Centroid.Y);
}

void UCrowdSimulationManager::UpdateAgentLOD()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    for (auto& PoolPair : AgentPools)
    {
        FCrowd_AgentPool& Pool = PoolPair.Value;
        for (AActor* Agent : Pool.ActiveAgents)
        {
            if (!Agent) continue;

            float Dist = FVector::Dist(Agent->GetActorLocation(), PlayerLoc);
            USkeletalMeshComponent* SkelComp = Agent->FindComponentByClass<USkeletalMeshComponent>();
            if (!SkelComp) continue;

            // LOD tier assignment based on distance
            if (Dist < LODNearDistance)
            {
                // Full quality — near agents
                SkelComp->SetForcedLOD(0);
            }
            else if (Dist < LODMidDistance)
            {
                // Mid LOD
                SkelComp->SetForcedLOD(1);
            }
            else if (Dist < LODFarDistance)
            {
                // Far LOD — minimal animation
                SkelComp->SetForcedLOD(2);
            }
            else
            {
                // Cull — beyond far distance
                SkelComp->SetVisibility(false);
            }
        }
    }
}

void UCrowdSimulationManager::RegisterCrowdAgent(AActor* Agent, ECrowd_AgentType AgentType)
{
    if (!Agent) return;

    FCrowd_AgentPool* Pool = AgentPools.Find(AgentType);
    if (!Pool)
    {
        InitializeAgentPool(AgentType, 10);
        Pool = AgentPools.Find(AgentType);
    }

    if (Pool && Pool->ActiveCount < Pool->MaxPoolSize)
    {
        Pool->ActiveAgents.Add(Agent);
        Pool->ActiveCount++;
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Agent registered: %s Type=%d"), *Agent->GetName(), (int32)AgentType);
    }
}

void UCrowdSimulationManager::UnregisterCrowdAgent(AActor* Agent)
{
    if (!Agent) return;

    for (auto& PoolPair : AgentPools)
    {
        FCrowd_AgentPool& Pool = PoolPair.Value;
        int32 Removed = Pool.ActiveAgents.Remove(Agent);
        if (Removed > 0)
        {
            Pool.ActiveCount = FMath::Max(0, Pool.ActiveCount - Removed);
            UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Agent unregistered: %s"), *Agent->GetName());
            return;
        }
    }
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgentCount;
}

ECrowd_BehaviorState UCrowdSimulationManager::GetPoolBehaviorState(ECrowd_AgentType AgentType) const
{
    const FCrowd_AgentPool* Pool = AgentPools.Find(AgentType);
    return Pool ? Pool->BehaviorState : ECrowd_BehaviorState::Idle;
}

void UCrowdSimulationManager::SetCrowdSystemActive(bool bActive)
{
    bCrowdSystemActive = bActive;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] System %s"), bActive ? TEXT("ACTIVATED") : TEXT("DEACTIVATED"));
}

void UCrowdSimulationManager::DrawDebugCrowdState()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (auto& PoolPair : AgentPools)
    {
        const FCrowd_AgentPool& Pool = PoolPair.Value;
        FColor DebugColor = FColor::Green;

        switch (Pool.BehaviorState)
        {
            case ECrowd_BehaviorState::Attacking: DebugColor = FColor::Red; break;
            case ECrowd_BehaviorState::Alerted:   DebugColor = FColor::Orange; break;
            case ECrowd_BehaviorState::Fleeing:   DebugColor = FColor::Yellow; break;
            case ECrowd_BehaviorState::Patrolling: DebugColor = FColor::Cyan; break;
            case ECrowd_BehaviorState::Grazing:   DebugColor = FColor::Green; break;
            default: DebugColor = FColor::White; break;
        }

        for (const AActor* Agent : Pool.ActiveAgents)
        {
            if (!Agent) continue;
            DrawDebugSphere(World, Agent->GetActorLocation() + FVector(0,0,200), 80.0f, 8, DebugColor, false, 0.15f);
        }

        // Draw formation targets
        for (const FVector& Target : Pool.FormationTargets)
        {
            DrawDebugPoint(World, Target, 20.0f, FColor::Magenta, false, 0.15f);
        }
    }
}
