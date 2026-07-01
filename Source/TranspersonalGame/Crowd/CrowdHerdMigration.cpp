// CrowdHerdMigration.cpp
// Crowd & Traffic Simulation Agent #13 — Cycle AUTO_20260701_004
// Herd Migration System — up to 50,000 agents using Mass AI LOD

#include "CrowdHerdMigration.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

// ─── ACrowdHerdMigration ────────────────────────────────────────────────────

ACrowdHerdMigration::ACrowdHerdMigration()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick for performance

    HerdSpecies = ECrowd_HerdSpecies::Parasaurolophus;
    HerdSize = 40;
    MigrationSpeed = 350.0f;
    bMigrationActive = false;
    CurrentWaypointIndex = 0;
    LODLevel = 0;
    StampedeRadius = 1500.0f;
    bStampedeActive = false;
    AgentSpacing = 180.0f;
}

void ACrowdHerdMigration::BeginPlay()
{
    Super::BeginPlay();

    // Initialize herd agents
    InitializeHerd();

    // Start migration after a short delay
    FTimerHandle StartTimer;
    GetWorldTimerManager().SetTimer(StartTimer, this,
        &ACrowdHerdMigration::StartMigration, 2.0f, false);

    UE_LOG(LogTemp, Log, TEXT("CrowdHerdMigration: Herd initialized — %d agents, species %d"),
        HerdSize, (int32)HerdSpecies);
}

void ACrowdHerdMigration::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bMigrationActive) return;

    UpdateLOD();
    AdvanceMigration(DeltaTime);

    if (bStampedeActive)
    {
        UpdateStampede(DeltaTime);
    }
}

void ACrowdHerdMigration::InitializeHerd()
{
    HerdAgents.Empty();
    HerdAgents.Reserve(HerdSize);

    FVector BaseLocation = GetActorLocation();

    for (int32 i = 0; i < HerdSize; i++)
    {
        FCrowd_HerdAgent Agent;
        Agent.AgentID = i;

        // Spread agents in a loose formation around base location
        float Row = FMath::Floor(i / 8.0f);
        float Col = i % 8;
        Agent.Location = BaseLocation + FVector(
            (Col - 4.0f) * AgentSpacing + FMath::RandRange(-30.0f, 30.0f),
            Row * AgentSpacing + FMath::RandRange(-30.0f, 30.0f),
            0.0f
        );

        Agent.Velocity = FVector::ZeroVector;
        Agent.bIsLeader = (i == 0); // First agent is herd leader
        Agent.Health = 100.0f;
        Agent.LODLevel = 0;
        Agent.BehaviorState = ECrowd_AgentBehavior::Grazing;

        HerdAgents.Add(Agent);
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdHerdMigration: %d agents initialized"), HerdAgents.Num());
}

void ACrowdHerdMigration::StartMigration()
{
    if (MigrationWaypoints.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdHerdMigration: Not enough waypoints to migrate (%d)"),
            MigrationWaypoints.Num());
        return;
    }

    bMigrationActive = true;
    CurrentWaypointIndex = 0;

    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        Agent.BehaviorState = ECrowd_AgentBehavior::Migrating;
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdHerdMigration: Migration started — %d waypoints"),
        MigrationWaypoints.Num());

    OnMigrationStarted.Broadcast(this);
}

void ACrowdHerdMigration::StopMigration()
{
    bMigrationActive = false;

    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        Agent.BehaviorState = ECrowd_AgentBehavior::Resting;
        Agent.Velocity = FVector::ZeroVector;
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdHerdMigration: Migration stopped"));
}

void ACrowdHerdMigration::TriggerStampede(FVector ThreatLocation, float ThreatRadius)
{
    if (bStampedeActive) return;

    bStampedeActive = true;
    StampedeThreatLocation = ThreatLocation;
    StampedeRadius = ThreatRadius;

    // All agents within radius enter panic
    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        float DistToThreat = FVector::Dist(Agent.Location, ThreatLocation);
        if (DistToThreat < ThreatRadius)
        {
            Agent.BehaviorState = ECrowd_AgentBehavior::Fleeing;
            // Flee direction = away from threat
            FVector FleeDir = (Agent.Location - ThreatLocation).GetSafeNormal();
            Agent.Velocity = FleeDir * MigrationSpeed * 2.5f; // Panic speed boost
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdHerdMigration: STAMPEDE triggered at %s, radius %.0f"),
        *ThreatLocation.ToString(), ThreatRadius);

    OnStampedeTriggered.Broadcast(ThreatLocation, ThreatRadius);

    // Auto-resolve stampede after 15 seconds
    FTimerHandle StampedeTimer;
    GetWorldTimerManager().SetTimer(StampedeTimer, this,
        &ACrowdHerdMigration::ResolveStampede, 15.0f, false);
}

void ACrowdHerdMigration::ResolveStampede()
{
    bStampedeActive = false;

    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        if (Agent.BehaviorState == ECrowd_AgentBehavior::Fleeing)
        {
            Agent.BehaviorState = bMigrationActive ?
                ECrowd_AgentBehavior::Migrating : ECrowd_AgentBehavior::Grazing;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdHerdMigration: Stampede resolved — herd calming"));
}

void ACrowdHerdMigration::AdvanceMigration(float DeltaTime)
{
    if (MigrationWaypoints.Num() == 0) return;

    FVector TargetWaypoint = MigrationWaypoints[CurrentWaypointIndex];

    // Move leader toward waypoint
    if (HerdAgents.Num() == 0) return;

    FCrowd_HerdAgent& Leader = HerdAgents[0];
    FVector ToTarget = TargetWaypoint - Leader.Location;
    float DistToTarget = ToTarget.Size();

    if (DistToTarget < 200.0f)
    {
        // Reached waypoint — advance
        CurrentWaypointIndex++;
        if (CurrentWaypointIndex >= MigrationWaypoints.Num())
        {
            // Migration complete — loop or stop
            CurrentWaypointIndex = 0;
            UE_LOG(LogTemp, Log, TEXT("CrowdHerdMigration: Waypoint loop complete"));
        }
        return;
    }

    // Move leader
    FVector MoveDir = ToTarget.GetSafeNormal();
    Leader.Velocity = MoveDir * MigrationSpeed;
    Leader.Location += Leader.Velocity * DeltaTime;

    // Followers track leader with offset (LOD-aware)
    for (int32 i = 1; i < HerdAgents.Num(); i++)
    {
        FCrowd_HerdAgent& Follower = HerdAgents[i];

        if (Follower.BehaviorState == ECrowd_AgentBehavior::Fleeing) continue;

        // LOD: only update every N frames based on LOD level
        if (LODLevel > 1 && (i % (LODLevel * 2)) != 0) continue;

        // Follow the agent ahead with spacing
        int32 LeaderIdx = FMath::Max(0, i - 1);
        FVector LeaderPos = HerdAgents[LeaderIdx].Location;
        FVector ToLeader = LeaderPos - Follower.Location;
        float DistToLeader = ToLeader.Size();

        if (DistToLeader > AgentSpacing * 1.5f)
        {
            FVector FollowDir = ToLeader.GetSafeNormal();
            Follower.Velocity = FollowDir * MigrationSpeed * 0.9f;
            Follower.Location += Follower.Velocity * DeltaTime;
        }
        else if (DistToLeader < AgentSpacing * 0.5f)
        {
            // Too close — slow down
            Follower.Velocity *= 0.5f;
            Follower.Location += Follower.Velocity * DeltaTime;
        }
    }
}

void ACrowdHerdMigration::UpdateStampede(float DeltaTime)
{
    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        if (Agent.BehaviorState != ECrowd_AgentBehavior::Fleeing) continue;

        // Continue fleeing away from threat
        FVector FleeDir = (Agent.Location - StampedeThreatLocation).GetSafeNormal();
        Agent.Velocity = FleeDir * MigrationSpeed * 2.0f;
        Agent.Location += Agent.Velocity * DeltaTime;

        // Separation from other fleeing agents
        for (const FCrowd_HerdAgent& Other : HerdAgents)
        {
            if (Other.AgentID == Agent.AgentID) continue;
            float Dist = FVector::Dist(Agent.Location, Other.Location);
            if (Dist < 100.0f && Dist > 0.1f)
            {
                FVector Sep = (Agent.Location - Other.Location).GetSafeNormal();
                Agent.Location += Sep * (100.0f - Dist) * 0.5f * DeltaTime;
            }
        }
    }
}

void ACrowdHerdMigration::UpdateLOD()
{
    // Determine LOD based on distance to player camera
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();
    FVector HerdCenter = GetActorLocation();
    float DistToPlayer = FVector::Dist(PlayerLoc, HerdCenter);

    // LOD thresholds: 0=full, 1=medium, 2=low, 3=minimal
    if (DistToPlayer < 2000.0f)       LODLevel = 0;
    else if (DistToPlayer < 5000.0f)  LODLevel = 1;
    else if (DistToPlayer < 10000.0f) LODLevel = 2;
    else                               LODLevel = 3;
}

int32 ACrowdHerdMigration::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        if (Agent.BehaviorState != ECrowd_AgentBehavior::Dead) Count++;
    }
    return Count;
}

void ACrowdHerdMigration::AddWaypoint(FVector WaypointLocation)
{
    MigrationWaypoints.Add(WaypointLocation);
}

void ACrowdHerdMigration::SetHerdSize(int32 NewSize)
{
    HerdSize = FMath::Clamp(NewSize, 1, 50000);
    InitializeHerd();
}
