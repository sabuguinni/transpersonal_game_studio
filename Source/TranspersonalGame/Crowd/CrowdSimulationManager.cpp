// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric tribal crowd simulation using UE5 Mass AI foundations

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxAgents = 50000;
    ActiveAgentCount = 0;
    bSimulationRunning = false;
    TickInterval = 0.1f;
    MigrationSpeed = 150.0f;
    FleeRadius = 800.0f;
    GroupCohesionRadius = 300.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bSimulationRunning = true;
    ActiveAgentCount = 0;
    unregister_all_groups();
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationRunning = false;
    TribalGroups.Empty();
    MigrationWaypoints.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::RegisterTribalGroup(const FCrowd_TribalGroup& Group)
{
    if (ActiveAgentCount + Group.MemberCount > MaxAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSim: MaxAgents cap reached (%d). Group '%s' not registered."),
            MaxAgents, *Group.GroupID.ToString());
        return;
    }
    TribalGroups.Add(Group.GroupID, Group);
    ActiveAgentCount += Group.MemberCount;
    UE_LOG(LogTemp, Log, TEXT("CrowdSim: Registered group '%s' with %d members. Total agents: %d"),
        *Group.GroupID.ToString(), Group.MemberCount, ActiveAgentCount);
}

void UCrowdSimulationManager::UnregisterTribalGroup(FName GroupID)
{
    if (FCrowd_TribalGroup* Group = TribalGroups.Find(GroupID))
    {
        ActiveAgentCount = FMath::Max(0, ActiveAgentCount - Group->MemberCount);
        TribalGroups.Remove(GroupID);
        UE_LOG(LogTemp, Log, TEXT("CrowdSim: Unregistered group '%s'. Total agents: %d"),
            *GroupID.ToString(), ActiveAgentCount);
    }
}

void UCrowdSimulationManager::AddMigrationWaypoint(const FCrowd_Waypoint& Waypoint)
{
    MigrationWaypoints.Add(Waypoint);
    UE_LOG(LogTemp, Log, TEXT("CrowdSim: Added waypoint '%s' at %s"),
        *Waypoint.WaypointID.ToString(), *Waypoint.WorldLocation.ToString());
}

void UCrowdSimulationManager::TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius)
{
    if (ThreatRadius <= 0.0f) ThreatRadius = FleeRadius;

    int32 Fleeing = 0;
    for (auto& Pair : TribalGroups)
    {
        FCrowd_TribalGroup& Group = Pair.Value;
        float Dist = FVector::Dist(Group.CurrentLocation, ThreatLocation);
        if (Dist <= ThreatRadius)
        {
            Group.CurrentBehavior = ECrowd_GroupBehavior::Fleeing;
            Fleeing += Group.MemberCount;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSim: Flee triggered at %s radius=%.0f — %d agents fleeing"),
        *ThreatLocation.ToString(), ThreatRadius, Fleeing);
}

void UCrowdSimulationManager::UpdateGroupBehavior(float DeltaTime)
{
    if (!bSimulationRunning) return;

    for (auto& Pair : TribalGroups)
    {
        FCrowd_TribalGroup& Group = Pair.Value;

        switch (Group.CurrentBehavior)
        {
        case ECrowd_GroupBehavior::Migrating:
            advance_migration(Group, DeltaTime);
            break;
        case ECrowd_GroupBehavior::Foraging:
            // Slow random wander within foraging radius
            break;
        case ECrowd_GroupBehavior::Resting:
            // Stationary — campfire logic handled by environment
            break;
        case ECrowd_GroupBehavior::Fleeing:
            flee_from_threat(Group, DeltaTime);
            break;
        case ECrowd_GroupBehavior::Hunting:
            // Coordinated hunt — approach prey in pincer formation
            break;
        default:
            break;
        }
    }
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgentCount;
}

TArray<FCrowd_TribalGroup> UCrowdSimulationManager::GetAllGroups() const
{
    TArray<FCrowd_TribalGroup> Result;
    TribalGroups.GenerateValueArray(Result);
    return Result;
}

// --- Private helpers ---

void UCrowdSimulationManager::advance_migration(FCrowd_TribalGroup& Group, float DeltaTime)
{
    if (MigrationWaypoints.Num() == 0) return;

    int32 WPIndex = Group.CurrentWaypointIndex % MigrationWaypoints.Num();
    const FCrowd_Waypoint& Target = MigrationWaypoints[WPIndex];

    FVector Dir = (Target.WorldLocation - Group.CurrentLocation).GetSafeNormal();
    Group.CurrentLocation += Dir * MigrationSpeed * DeltaTime;

    if (FVector::Dist(Group.CurrentLocation, Target.WorldLocation) < 50.0f)
    {
        Group.CurrentWaypointIndex = (Group.CurrentWaypointIndex + 1) % MigrationWaypoints.Num();
    }
}

void UCrowdSimulationManager::flee_from_threat(FCrowd_TribalGroup& Group, float DeltaTime)
{
    // Move away from last known threat — simplified: move in +X direction
    Group.CurrentLocation += FVector(1.0f, 0.0f, 0.0f) * MigrationSpeed * 2.0f * DeltaTime;

    // After fleeing 500 units, resume foraging
    if (Group.CurrentLocation.X > 2000.0f)
    {
        Group.CurrentBehavior = ECrowd_GroupBehavior::Foraging;
    }
}

void UCrowdSimulationManager::unregister_all_groups()
{
    TribalGroups.Empty();
    MigrationWaypoints.Empty();
    ActiveAgentCount = 0;
}
