// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric survival game — Mass AI crowd simulation for dinosaur herds and human tribes

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// UCrowdSimulationManager — UWorldSubsystem implementation
// ─────────────────────────────────────────────────────────────────────────────

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxAgentsTotal = 500;
    MaxAgentsHighLOD = 50;
    MaxAgentsMedLOD = 150;
    HighLODRadius = 500.0f;
    MedLODRadius = 1500.0f;
    TickInterval = 0.1f;
    bSimulationActive = false;
    bDebugDrawEnabled = false;
    CurrentAgentCount = 0;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Register tick timer for crowd updates
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            TickTimerHandle,
            this,
            &UCrowdSimulationManager::TickCrowdSimulation,
            TickInterval,
            true
        );
        bSimulationActive = true;
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Manager initialized — max agents: %d"), MaxAgentsTotal);
    }
}

void UCrowdSimulationManager::Deinitialize()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(TickTimerHandle);
    }
    bSimulationActive = false;
    RegisteredGroups.Empty();
    Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────
// Group Registration
// ─────────────────────────────────────────────────────────────────────────────

int32 UCrowdSimulationManager::RegisterCrowdGroup(const FCrowd_GroupConfig& Config)
{
    if (RegisteredGroups.Num() >= 64)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Max group limit reached (64)"));
        return -1;
    }

    FCrowd_GroupState NewGroup;
    NewGroup.GroupID = NextGroupID++;
    NewGroup.Config = Config;
    NewGroup.bIsActive = true;
    NewGroup.CurrentBehavior = ECrowd_GroupBehavior::Idle;
    NewGroup.ThreatLevel = 0.0f;
    NewGroup.AgentCount = 0;
    NewGroup.CenterLocation = Config.SpawnCenter;

    RegisteredGroups.Add(NewGroup);
    CurrentAgentCount += Config.DesiredAgentCount;

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group registered: ID=%d Type=%d Agents=%d"),
        NewGroup.GroupID, (int32)Config.GroupType, Config.DesiredAgentCount);

    return NewGroup.GroupID;
}

bool UCrowdSimulationManager::UnregisterCrowdGroup(int32 GroupID)
{
    for (int32 i = 0; i < RegisteredGroups.Num(); ++i)
    {
        if (RegisteredGroups[i].GroupID == GroupID)
        {
            CurrentAgentCount -= RegisteredGroups[i].Config.DesiredAgentCount;
            RegisteredGroups.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group unregistered: ID=%d"), GroupID);
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// LOD Management
// ─────────────────────────────────────────────────────────────────────────────

ECrowd_LODLevel UCrowdSimulationManager::GetLODForDistance(float Distance) const
{
    if (Distance <= HighLODRadius)
        return ECrowd_LODLevel::High;
    else if (Distance <= MedLODRadius)
        return ECrowd_LODLevel::Medium;
    else
        return ECrowd_LODLevel::Low;
}

void UCrowdSimulationManager::UpdateGroupLOD(FCrowd_GroupState& Group, const FVector& PlayerLocation)
{
    float Distance = FVector::Dist(Group.CenterLocation, PlayerLocation);
    ECrowd_LODLevel NewLOD = GetLODForDistance(Distance);

    if (NewLOD != Group.CurrentLOD)
    {
        Group.CurrentLOD = NewLOD;
        OnGroupLODChanged.Broadcast(Group.GroupID, NewLOD);

        if (bDebugDrawEnabled)
        {
            UE_LOG(LogTemp, Verbose, TEXT("[CrowdSim] Group %d LOD changed to %d (dist=%.0f)"),
                Group.GroupID, (int32)NewLOD, Distance);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Behavior State Machine
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::SetGroupBehavior(int32 GroupID, ECrowd_GroupBehavior NewBehavior)
{
    for (FCrowd_GroupState& Group : RegisteredGroups)
    {
        if (Group.GroupID == GroupID)
        {
            ECrowd_GroupBehavior OldBehavior = Group.CurrentBehavior;
            Group.CurrentBehavior = NewBehavior;
            OnGroupBehaviorChanged.Broadcast(GroupID, OldBehavior, NewBehavior);

            UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group %d behavior: %d -> %d"),
                GroupID, (int32)OldBehavior, (int32)NewBehavior);
            return;
        }
    }
}

void UCrowdSimulationManager::NotifyThreatEvent(const FVector& ThreatLocation, float ThreatRadius, float ThreatIntensity)
{
    for (FCrowd_GroupState& Group : RegisteredGroups)
    {
        float DistToThreat = FVector::Dist(Group.CenterLocation, ThreatLocation);
        if (DistToThreat <= ThreatRadius)
        {
            // Scale threat by distance falloff
            float FalloffFactor = 1.0f - (DistToThreat / ThreatRadius);
            Group.ThreatLevel = FMath::Max(Group.ThreatLevel, ThreatIntensity * FalloffFactor);

            // Trigger flee behavior for prey, attack for predators
            if (Group.Config.GroupType == ECrowd_GroupType::HerbivoreHerd ||
                Group.Config.GroupType == ECrowd_GroupType::HumanTribe)
            {
                SetGroupBehavior(Group.GroupID, ECrowd_GroupBehavior::Fleeing);
            }
            else if (Group.Config.GroupType == ECrowd_GroupType::PredatorPack)
            {
                SetGroupBehavior(Group.GroupID, ECrowd_GroupBehavior::Hunting);
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Migration Pathfinding
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::SetMigrationPath(int32 GroupID, const TArray<FVector>& Waypoints)
{
    for (FCrowd_GroupState& Group : RegisteredGroups)
    {
        if (Group.GroupID == GroupID)
        {
            Group.MigrationWaypoints = Waypoints;
            Group.CurrentWaypointIndex = 0;
            SetGroupBehavior(GroupID, ECrowd_GroupBehavior::Migrating);

            UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Migration path set for group %d: %d waypoints"),
                GroupID, Waypoints.Num());
            return;
        }
    }
}

FVector UCrowdSimulationManager::GetNextMigrationWaypoint(const FCrowd_GroupState& Group) const
{
    if (Group.MigrationWaypoints.IsValidIndex(Group.CurrentWaypointIndex))
    {
        return Group.MigrationWaypoints[Group.CurrentWaypointIndex];
    }
    return Group.CenterLocation;
}

// ─────────────────────────────────────────────────────────────────────────────
// Main Tick
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::TickCrowdSimulation()
{
    if (!bSimulationActive) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Get player location for LOD updates
    APlayerController* PC = World->GetFirstPlayerController();
    FVector PlayerLocation = FVector::ZeroVector;
    if (PC && PC->GetPawn())
    {
        PlayerLocation = PC->GetPawn()->GetActorLocation();
    }

    // Update each group
    for (FCrowd_GroupState& Group : RegisteredGroups)
    {
        if (!Group.bIsActive) continue;

        // Update LOD
        UpdateGroupLOD(Group, PlayerLocation);

        // Skip heavy updates for low LOD groups
        if (Group.CurrentLOD == ECrowd_LODLevel::Low) continue;

        // Update migration
        if (Group.CurrentBehavior == ECrowd_GroupBehavior::Migrating)
        {
            UpdateMigration(Group);
        }

        // Decay threat level over time
        if (Group.ThreatLevel > 0.0f)
        {
            Group.ThreatLevel = FMath::Max(0.0f, Group.ThreatLevel - 0.01f);
            if (Group.ThreatLevel <= 0.0f && Group.CurrentBehavior == ECrowd_GroupBehavior::Fleeing)
            {
                SetGroupBehavior(Group.GroupID, ECrowd_GroupBehavior::Idle);
            }
        }
    }
}

void UCrowdSimulationManager::UpdateMigration(FCrowd_GroupState& Group)
{
    if (Group.MigrationWaypoints.IsEmpty()) return;

    FVector Target = GetNextMigrationWaypoint(Group);
    float DistToWaypoint = FVector::Dist(Group.CenterLocation, Target);

    // Move group center toward waypoint
    float MoveSpeed = 100.0f * TickInterval; // 100 cm/s
    if (DistToWaypoint > MoveSpeed)
    {
        FVector Direction = (Target - Group.CenterLocation).GetSafeNormal();
        Group.CenterLocation += Direction * MoveSpeed;
    }
    else
    {
        // Reached waypoint — advance to next
        Group.CurrentWaypointIndex++;
        if (Group.CurrentWaypointIndex >= Group.MigrationWaypoints.Num())
        {
            // Loop migration
            Group.CurrentWaypointIndex = 0;
            UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group %d completed migration loop"), Group.GroupID);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Query Interface
// ─────────────────────────────────────────────────────────────────────────────

TArray<FCrowd_GroupState> UCrowdSimulationManager::GetGroupsInRadius(const FVector& Center, float Radius) const
{
    TArray<FCrowd_GroupState> Result;
    for (const FCrowd_GroupState& Group : RegisteredGroups)
    {
        if (FVector::Dist(Group.CenterLocation, Center) <= Radius)
        {
            Result.Add(Group);
        }
    }
    return Result;
}

int32 UCrowdSimulationManager::GetTotalActiveAgents() const
{
    int32 Total = 0;
    for (const FCrowd_GroupState& Group : RegisteredGroups)
    {
        if (Group.bIsActive)
        {
            Total += Group.Config.DesiredAgentCount;
        }
    }
    return Total;
}

FCrowd_GroupState UCrowdSimulationManager::GetGroupState(int32 GroupID) const
{
    for (const FCrowd_GroupState& Group : RegisteredGroups)
    {
        if (Group.GroupID == GroupID)
        {
            return Group;
        }
    }
    return FCrowd_GroupState();
}
