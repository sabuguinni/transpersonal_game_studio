// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements prehistoric herd, pack, and territorial group behaviours.

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

// ─────────────────────────────────────────────────────────────────────────────
// UCrowdSimulationManager — UWorldSubsystem
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Seed registered groups with default prehistoric ecology
    RegisterDefaultGroups();

    // Tick crowd simulation every 2 seconds (performance-friendly)
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CrowdTickHandle,
            this,
            &UCrowdSimulationManager::TickCrowdSimulation,
            CrowdTickInterval,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] UCrowdSimulationManager initialized — %d groups registered"), RegisteredGroups.Num());
}

void UCrowdSimulationManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CrowdTickHandle);
    }
    RegisteredGroups.Empty();
    Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────
// Group Registration
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::RegisterDefaultGroups()
{
    // Parasaurolophus herd — large herbivore, open plains
    FCrowd_GroupData ParaHerd;
    ParaHerd.GroupID = FName("Para_Herd_Savana");
    ParaHerd.GroupType = ECrowd_GroupType::HerbivoreHerd;
    ParaHerd.BehaviourState = ECrowd_BehaviourState::Grazing;
    ParaHerd.HomeLocation = FVector(3000.f, 1600.f, 400.f);
    ParaHerd.WanderRadius = 1500.f;
    ParaHerd.MaxGroupSize = 8;
    ParaHerd.CurrentSize = 8;
    ParaHerd.FleeThreshold = 0.4f;
    ParaHerd.bIsActive = true;
    RegisteredGroups.Add(ParaHerd);

    // Velociraptor pack — predator, hunting formation
    FCrowd_GroupData RaptorPack;
    RaptorPack.GroupID = FName("Raptor_Pack_Alpha");
    RaptorPack.GroupType = ECrowd_GroupType::PredatorPack;
    RaptorPack.BehaviourState = ECrowd_BehaviourState::Patrolling;
    RaptorPack.HomeLocation = FVector(1800.f, 3000.f, 400.f);
    RaptorPack.WanderRadius = 2000.f;
    RaptorPack.MaxGroupSize = 5;
    RaptorPack.CurrentSize = 5;
    RaptorPack.FleeThreshold = 0.1f;  // Predators rarely flee
    RaptorPack.bIsActive = true;
    RegisteredGroups.Add(RaptorPack);

    // Pachycephalosaurus — small territorial group
    FCrowd_GroupData PachyGroup;
    PachyGroup.GroupID = FName("Pachy_Territory_East");
    PachyGroup.GroupType = ECrowd_GroupType::TerritorialGroup;
    PachyGroup.BehaviourState = ECrowd_BehaviourState::Grazing;
    PachyGroup.HomeLocation = FVector(4000.f, 3100.f, 400.f);
    PachyGroup.WanderRadius = 800.f;
    PachyGroup.MaxGroupSize = 3;
    PachyGroup.CurrentSize = 3;
    PachyGroup.FleeThreshold = 0.6f;
    PachyGroup.bIsActive = true;
    RegisteredGroups.Add(PachyGroup);

    // Ankylosaurus — solitary patrol
    FCrowd_GroupData AnkyloPatrol;
    AnkyloPatrol.GroupID = FName("Ankylo_Solitary_North");
    AnkyloPatrol.GroupType = ECrowd_GroupType::SolitaryPatrol;
    AnkyloPatrol.BehaviourState = ECrowd_BehaviourState::Patrolling;
    AnkyloPatrol.HomeLocation = FVector(500.f, 4200.f, 400.f);
    AnkyloPatrol.WanderRadius = 1200.f;
    AnkyloPatrol.MaxGroupSize = 4;
    AnkyloPatrol.CurrentSize = 4;
    AnkyloPatrol.FleeThreshold = 0.2f;  // Armoured — rarely flees
    AnkyloPatrol.bIsActive = true;
    RegisteredGroups.Add(AnkyloPatrol);
}

void UCrowdSimulationManager::RegisterGroup(const FCrowd_GroupData& GroupData)
{
    // Prevent duplicate group IDs
    for (const FCrowd_GroupData& Existing : RegisteredGroups)
    {
        if (Existing.GroupID == GroupData.GroupID)
        {
            UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Group '%s' already registered — skipping"), *GroupData.GroupID.ToString());
            return;
        }
    }
    RegisteredGroups.Add(GroupData);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Registered group '%s' (type=%d, size=%d)"),
        *GroupData.GroupID.ToString(), (int32)GroupData.GroupType, GroupData.CurrentSize);
}

// ─────────────────────────────────────────────────────────────────────────────
// Crowd Tick — State Machine
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::TickCrowdSimulation()
{
    for (FCrowd_GroupData& Group : RegisteredGroups)
    {
        if (!Group.bIsActive) continue;

        switch (Group.BehaviourState)
        {
        case ECrowd_BehaviourState::Grazing:
            TickGrazing(Group);
            break;
        case ECrowd_BehaviourState::Patrolling:
            TickPatrolling(Group);
            break;
        case ECrowd_BehaviourState::Fleeing:
            TickFleeing(Group);
            break;
        case ECrowd_BehaviourState::Hunting:
            TickHunting(Group);
            break;
        case ECrowd_BehaviourState::Resting:
            // Resting — no movement, just idle
            break;
        default:
            break;
        }
    }
}

void UCrowdSimulationManager::TickGrazing(FCrowd_GroupData& Group)
{
    // Herbivores slowly drift within wander radius
    // In a full implementation this would move actual actor references
    // For now, update the logical home location slightly (simulates drift)
    float DriftX = FMath::RandRange(-50.f, 50.f);
    float DriftY = FMath::RandRange(-50.f, 50.f);
    FVector NewPos = Group.HomeLocation + FVector(DriftX, DriftY, 0.f);

    // Clamp drift to wander radius from original spawn
    // (simplified — real impl would use NavMesh queries)
    Group.HomeLocation = NewPos;
}

void UCrowdSimulationManager::TickPatrolling(FCrowd_GroupData& Group)
{
    // Predators/solitary animals patrol in a wider arc
    float PatrolX = FMath::RandRange(-100.f, 100.f);
    float PatrolY = FMath::RandRange(-100.f, 100.f);
    Group.HomeLocation += FVector(PatrolX, PatrolY, 0.f);
}

void UCrowdSimulationManager::TickFleeing(FCrowd_GroupData& Group)
{
    // Move away from threat — in full impl, threat location tracked
    // Transition back to Grazing after flee distance reached
    Group.BehaviourState = ECrowd_BehaviourState::Grazing;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group '%s' calmed down — returning to Grazing"), *Group.GroupID.ToString());
}

void UCrowdSimulationManager::TickHunting(FCrowd_GroupData& Group)
{
    // Predator pack hunting — coordinate flanking
    // Simplified: after hunt attempt, return to Patrolling
    Group.BehaviourState = ECrowd_BehaviourState::Patrolling;
}

// ─────────────────────────────────────────────────────────────────────────────
// Threat Response
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::NotifyThreatAtLocation(FVector ThreatLocation, float ThreatRadius)
{
    for (FCrowd_GroupData& Group : RegisteredGroups)
    {
        if (!Group.bIsActive) continue;

        float DistToThreat = FVector::Dist(Group.HomeLocation, ThreatLocation);
        if (DistToThreat < ThreatRadius)
        {
            // Herbivores flee; predators may investigate or hunt
            if (Group.GroupType == ECrowd_GroupType::HerbivoreHerd ||
                Group.GroupType == ECrowd_GroupType::TerritorialGroup)
            {
                Group.BehaviourState = ECrowd_BehaviourState::Fleeing;
                UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group '%s' FLEEING from threat at (%.0f,%.0f,%.0f)"),
                    *Group.GroupID.ToString(), ThreatLocation.X, ThreatLocation.Y, ThreatLocation.Z);
            }
            else if (Group.GroupType == ECrowd_GroupType::PredatorPack)
            {
                Group.BehaviourState = ECrowd_BehaviourState::Hunting;
                UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group '%s' HUNTING — threat detected nearby"),
                    *Group.GroupID.ToString());
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Query
// ─────────────────────────────────────────────────────────────────────────────

int32 UCrowdSimulationManager::GetTotalActiveAgents() const
{
    int32 Total = 0;
    for (const FCrowd_GroupData& Group : RegisteredGroups)
    {
        if (Group.bIsActive)
        {
            Total += Group.CurrentSize;
        }
    }
    return Total;
}

FCrowd_GroupData UCrowdSimulationManager::GetGroupByID(FName GroupID) const
{
    for (const FCrowd_GroupData& Group : RegisteredGroups)
    {
        if (Group.GroupID == GroupID)
        {
            return Group;
        }
    }
    return FCrowd_GroupData();  // Empty default
}
