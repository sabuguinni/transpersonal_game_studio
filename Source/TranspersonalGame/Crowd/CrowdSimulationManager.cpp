// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric crowd simulation: herds, packs, migration groups, scavenger flocks

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    bSimulationActive   = false;
    GlobalAlertLevel    = 0.0f;
    CurrentAgentCount   = 0;
    MaxActiveAgents     = 500;
    SimulationTickRate  = 0.5f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterDefaultGroups();
    StartSimulationTick();
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized with %d groups"), CrowdGroups.Num());
}

void UCrowdSimulationManager::Deinitialize()
{
    StopSimulationTick();
    CrowdGroups.Empty();
    Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────
// Group Registration
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::RegisterCrowdGroup(const FCrowd_GroupData& GroupData)
{
    // Prevent duplicate group IDs
    for (const FCrowd_GroupData& Existing : CrowdGroups)
    {
        if (Existing.GroupID == GroupData.GroupID)
        {
            UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Group '%s' already registered — skipping"), *GroupData.GroupID.ToString());
            return;
        }
    }
    CrowdGroups.Add(GroupData);
    CurrentAgentCount += GroupData.AgentCount;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Registered group '%s' (%d agents)"), *GroupData.GroupID.ToString(), GroupData.AgentCount);
}

void UCrowdSimulationManager::RegisterDefaultGroups()
{
    // ── Herbivore Herd A — Triceratops territory ──────────────────────────
    {
        FCrowd_GroupData Trike;
        Trike.GroupID        = FName("TrikeHerd_A");
        Trike.GroupType      = ECrowd_GroupType::HerbivoreHerd;
        Trike.BehaviorState  = ECrowd_BehaviorState::Grazing;
        Trike.CurrentLOD     = ECrowd_LODLevel::LOD_Full;
        Trike.AgentCount     = 8;
        Trike.HomeLocation   = FVector(1700.f, 2900.f, 400.f);
        Trike.WanderRadius   = 1200.f;
        Trike.MovementSpeed  = 220.f;
        Trike.FleeThreshold  = 0.6f;
        Trike.AlertLevel     = 0.0f;
        Trike.bIsAlerting    = false;
        RegisterCrowdGroup(Trike);
    }

    // ── Herbivore Herd B — Parasaurolophus territory ──────────────────────
    {
        FCrowd_GroupData Para;
        Para.GroupID        = FName("ParaHerd_B");
        Para.GroupType      = ECrowd_GroupType::HerbivoreHerd;
        Para.BehaviorState  = ECrowd_BehaviorState::Wandering;
        Para.CurrentLOD     = ECrowd_LODLevel::LOD_Full;
        Para.AgentCount     = 12;
        Para.HomeLocation   = FVector(3400.f, 2300.f, 400.f);
        Para.WanderRadius   = 1500.f;
        Para.MovementSpeed  = 280.f;
        Para.FleeThreshold  = 0.5f;
        Para.AlertLevel     = 0.0f;
        Para.bIsAlerting    = false;
        RegisterCrowdGroup(Para);
    }

    // ── Predator Pack — Velociraptor hunting group ────────────────────────
    {
        FCrowd_GroupData Raptors;
        Raptors.GroupID        = FName("RaptorPack_Alpha");
        Raptors.GroupType      = ECrowd_GroupType::PredatorPack;
        Raptors.BehaviorState  = ECrowd_BehaviorState::Hunting;
        Raptors.CurrentLOD     = ECrowd_LODLevel::LOD_Full;
        Raptors.AgentCount     = 5;
        Raptors.HomeLocation   = FVector(2450.f, 2600.f, 400.f);
        Raptors.WanderRadius   = 2000.f;
        Raptors.MovementSpeed  = 480.f;
        Raptors.FleeThreshold  = 0.2f;  // Predators rarely flee
        Raptors.AlertLevel     = 0.0f;
        Raptors.bIsAlerting    = false;
        RegisterCrowdGroup(Raptors);
    }

    // ── Solitary Roamer — T-Rex apex predator ────────────────────────────
    {
        FCrowd_GroupData TRex;
        TRex.GroupID        = FName("TRex_Roamer");
        TRex.GroupType      = ECrowd_GroupType::SolitaryRoamer;
        TRex.BehaviorState  = ECrowd_BehaviorState::Wandering;
        TRex.CurrentLOD     = ECrowd_LODLevel::LOD_Full;
        TRex.AgentCount     = 1;
        TRex.HomeLocation   = FVector(2000.f, 2500.f, 400.f);
        TRex.WanderRadius   = 4000.f;
        TRex.MovementSpeed  = 350.f;
        TRex.FleeThreshold  = 0.0f;  // Never flees
        TRex.AlertLevel     = 0.0f;
        TRex.bIsAlerting    = false;
        RegisterCrowdGroup(TRex);
    }

    // ── Migration Group — Brachiosaurus long-range corridor ───────────────
    {
        FCrowd_GroupData Brachio;
        Brachio.GroupID        = FName("BrachioMigration");
        Brachio.GroupType      = ECrowd_GroupType::MigrationGroup;
        Brachio.BehaviorState  = ECrowd_BehaviorState::Migrating;
        Brachio.CurrentLOD     = ECrowd_LODLevel::LOD_Medium;
        Brachio.AgentCount     = 4;
        Brachio.HomeLocation   = FVector(2700.f, 1800.f, 400.f);
        Brachio.WanderRadius   = 6000.f;
        Brachio.MovementSpeed  = 180.f;
        Brachio.FleeThreshold  = 0.7f;
        Brachio.AlertLevel     = 0.0f;
        Brachio.bIsAlerting    = false;
        RegisterCrowdGroup(Brachio);
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] RegisterDefaultGroups complete — %d groups, %d total agents"),
        CrowdGroups.Num(), CurrentAgentCount);
}

// ─────────────────────────────────────────────────────────────────────────────
// Alert System
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::AlertGroupsInRadius(FVector Origin, float Radius, float AlertStrength)
{
    int32 AlertedCount = 0;
    for (FCrowd_GroupData& Group : CrowdGroups)
    {
        float Dist = FVector::Dist(Group.HomeLocation, Origin);
        if (Dist <= Radius)
        {
            float FalloffFactor = 1.0f - (Dist / Radius);
            float NewAlert = Group.AlertLevel + (AlertStrength * FalloffFactor);
            Group.AlertLevel = FMath::Clamp(NewAlert, 0.0f, 1.0f);
            Group.bIsAlerting = Group.AlertLevel >= Group.FleeThreshold;

            if (Group.bIsAlerting && Group.BehaviorState != ECrowd_BehaviorState::Fleeing)
            {
                // Herbivores flee; predators switch to hunting
                if (Group.GroupType == ECrowd_GroupType::HerbivoreHerd ||
                    Group.GroupType == ECrowd_GroupType::MigrationGroup)
                {
                    Group.BehaviorState = ECrowd_BehaviorState::Fleeing;
                }
                else if (Group.GroupType == ECrowd_GroupType::PredatorPack)
                {
                    Group.BehaviorState = ECrowd_BehaviorState::Hunting;
                }
            }
            AlertedCount++;
        }
    }

    // Propagate to global alert level
    GlobalAlertLevel = FMath::Max(GlobalAlertLevel, AlertStrength * 0.5f);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Alert propagated to %d groups (radius=%.0f, strength=%.2f)"),
        AlertedCount, Radius, AlertStrength);
}

void UCrowdSimulationManager::ResetAlerts()
{
    for (FCrowd_GroupData& Group : CrowdGroups)
    {
        Group.AlertLevel  = 0.0f;
        Group.bIsAlerting = false;
        if (Group.BehaviorState == ECrowd_BehaviorState::Fleeing)
        {
            Group.BehaviorState = ECrowd_BehaviorState::Wandering;
        }
    }
    GlobalAlertLevel = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] All alerts reset"));
}

// ─────────────────────────────────────────────────────────────────────────────
// LOD Management
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::UpdateCrowdLOD(FVector PlayerLocation)
{
    for (FCrowd_GroupData& Group : CrowdGroups)
    {
        float Dist = FVector::Dist(Group.HomeLocation, PlayerLocation);

        if (Dist < 2000.f)
        {
            Group.CurrentLOD = ECrowd_LODLevel::LOD_Full;
        }
        else if (Dist < 5000.f)
        {
            Group.CurrentLOD = ECrowd_LODLevel::LOD_Medium;
        }
        else if (Dist < 10000.f)
        {
            Group.CurrentLOD = ECrowd_LODLevel::LOD_Low;
        }
        else
        {
            Group.CurrentLOD = ECrowd_LODLevel::LOD_Culled;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Pathfinding
// ─────────────────────────────────────────────────────────────────────────────

FVector UCrowdSimulationManager::GetFleeDestination(FName GroupID, FVector ThreatLocation)
{
    for (const FCrowd_GroupData& Group : CrowdGroups)
    {
        if (Group.GroupID == GroupID)
        {
            // Flee directly away from threat, within wander radius
            FVector AwayDir = (Group.HomeLocation - ThreatLocation).GetSafeNormal();
            FVector FleeTarget = Group.HomeLocation + (AwayDir * Group.WanderRadius * 0.8f);
            return FleeTarget;
        }
    }
    return FVector::ZeroVector;
}

FVector UCrowdSimulationManager::GetWanderTarget(FName GroupID)
{
    for (const FCrowd_GroupData& Group : CrowdGroups)
    {
        if (Group.GroupID == GroupID)
        {
            // Random point within wander radius of home
            float Angle  = FMath::FRandRange(0.f, 2.f * PI);
            float Radius = FMath::FRandRange(0.f, Group.WanderRadius);
            FVector Offset(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
            return Group.HomeLocation + Offset;
        }
    }
    return FVector::ZeroVector;
}

// ─────────────────────────────────────────────────────────────────────────────
// Simulation Control
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::StartSimulationTick()
{
    UWorld* World = GetWorld();
    if (!World) return;

    World->GetTimerManager().SetTimer(
        SimulationTickHandle,
        this,
        &UCrowdSimulationManager::OnSimulationTick,
        SimulationTickRate,
        true
    );
    bSimulationActive = true;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Simulation tick started (%.2fs interval)"), SimulationTickRate);
}

void UCrowdSimulationManager::StopSimulationTick()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(SimulationTickHandle);
    }
    bSimulationActive = false;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Simulation tick stopped"));
}

void UCrowdSimulationManager::OnSimulationTick()
{
    for (FCrowd_GroupData& Group : CrowdGroups)
    {
        TickGroupBehavior(Group);
    }

    // Decay global alert over time
    GlobalAlertLevel = FMath::Max(0.0f, GlobalAlertLevel - 0.02f);
}

void UCrowdSimulationManager::TickGroupBehavior(FCrowd_GroupData& Group)
{
    // Decay individual alert levels
    Group.AlertLevel = FMath::Max(0.0f, Group.AlertLevel - 0.05f);
    if (Group.AlertLevel < Group.FleeThreshold)
    {
        Group.bIsAlerting = false;
        if (Group.BehaviorState == ECrowd_BehaviorState::Fleeing)
        {
            Group.BehaviorState = ECrowd_BehaviorState::Wandering;
        }
    }

    // LOD-based behavior: culled groups skip full simulation
    if (Group.CurrentLOD == ECrowd_LODLevel::LOD_Culled) return;

    // State machine transitions
    switch (Group.BehaviorState)
    {
    case ECrowd_BehaviorState::Grazing:
        // Occasional transition to wandering
        if (FMath::FRand() < 0.05f)
        {
            Group.BehaviorState = ECrowd_BehaviorState::Wandering;
        }
        break;

    case ECrowd_BehaviorState::Wandering:
        // Occasional transition back to grazing (herbivores only)
        if (Group.GroupType == ECrowd_GroupType::HerbivoreHerd && FMath::FRand() < 0.1f)
        {
            Group.BehaviorState = ECrowd_BehaviorState::Grazing;
        }
        break;

    case ECrowd_BehaviorState::Migrating:
        // Migration groups keep moving
        break;

    case ECrowd_BehaviorState::Hunting:
        // Predators return to wandering after hunt
        if (FMath::FRand() < 0.03f)
        {
            Group.BehaviorState = ECrowd_BehaviorState::Wandering;
        }
        break;

    default:
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Query Interface
// ─────────────────────────────────────────────────────────────────────────────

int32 UCrowdSimulationManager::GetGroupCount() const
{
    return CrowdGroups.Num();
}

bool UCrowdSimulationManager::IsGroupAlerting(FName GroupID) const
{
    for (const FCrowd_GroupData& Group : CrowdGroups)
    {
        if (Group.GroupID == GroupID)
        {
            return Group.bIsAlerting;
        }
    }
    return false;
}

float UCrowdSimulationManager::GetGroupAlertLevel(FName GroupID) const
{
    for (const FCrowd_GroupData& Group : CrowdGroups)
    {
        if (Group.GroupID == GroupID)
        {
            return Group.AlertLevel;
        }
    }
    return 0.0f;
}

FCrowd_GroupData UCrowdSimulationManager::GetGroupData(FName GroupID) const
{
    for (const FCrowd_GroupData& Group : CrowdGroups)
    {
        if (Group.GroupID == GroupID)
        {
            return Group;
        }
    }
    return FCrowd_GroupData();
}

TArray<FName> UCrowdSimulationManager::GetAlertingGroups() const
{
    TArray<FName> Result;
    for (const FCrowd_GroupData& Group : CrowdGroups)
    {
        if (Group.bIsAlerting)
        {
            Result.Add(Group.GroupID);
        }
    }
    return Result;
}
