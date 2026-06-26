// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements prehistoric herd/pack crowd simulation using UE5 Mass AI principles

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// ─────────────────────────────────────────────────────────────────────────────
// UCrowdSimulationManager — World Subsystem
// ─────────────────────────────────────────────────────────────────────────────

UCrowdSimulationManager::UCrowdSimulationManager()
{
    bSimulationActive = false;
    GlobalAlertLevel = 0.0f;
    SimulationTickRate = 0.25f; // 4 Hz crowd tick
    MaxActiveAgents = 500;
    CurrentAgentCount = 0;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] UCrowdSimulationManager initialized"));
    RegisterDefaultGroups();
    StartSimulationTick();
}

void UCrowdSimulationManager::Deinitialize()
{
    StopSimulationTick();
    CrowdGroups.Empty();
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] UCrowdSimulationManager deinitialized"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Group Registration
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::RegisterDefaultGroups()
{
    // Herbivore herd A — Triceratops zone
    FCrowd_GroupData HerdA;
    HerdA.GroupID = FName("HerbivoreHerd_A");
    HerdA.GroupType = ECrowd_GroupType::HerbivoreHerd;
    HerdA.AgentCount = 8;
    HerdA.HomeLocation = FVector(2000.f, 2800.f, 400.f);
    HerdA.WanderRadius = 1200.f;
    HerdA.MovementSpeed = 280.f;
    HerdA.FleeThreshold = 0.4f;
    HerdA.bIsAlerting = false;
    HerdA.AlertLevel = 0.0f;
    RegisterCrowdGroup(HerdA);

    // Herbivore herd B — Brachiosaurus zone
    FCrowd_GroupData HerdB;
    HerdB.GroupID = FName("HerbivoreHerd_B");
    HerdB.GroupType = ECrowd_GroupType::HerbivoreHerd;
    HerdB.AgentCount = 5;
    HerdB.HomeLocation = FVector(2700.f, 1800.f, 400.f);
    HerdB.WanderRadius = 1800.f;
    HerdB.MovementSpeed = 220.f;
    HerdB.FleeThreshold = 0.35f;
    HerdB.bIsAlerting = false;
    HerdB.AlertLevel = 0.0f;
    RegisterCrowdGroup(HerdB);

    // Predator pack — Raptor pack
    FCrowd_GroupData RaptorPack;
    RaptorPack.GroupID = FName("PredatorPack_Raptors");
    RaptorPack.GroupType = ECrowd_GroupType::PredatorPack;
    RaptorPack.AgentCount = 4;
    RaptorPack.HomeLocation = FVector(3200.f, 2200.f, 400.f);
    RaptorPack.WanderRadius = 2500.f;
    RaptorPack.MovementSpeed = 520.f;
    RaptorPack.FleeThreshold = 0.85f;
    RaptorPack.bIsAlerting = false;
    RaptorPack.AlertLevel = 0.0f;
    RegisterCrowdGroup(RaptorPack);

    // Migration group — seasonal movement
    FCrowd_GroupData MigrationGroup;
    MigrationGroup.GroupID = FName("MigrationGroup_A");
    MigrationGroup.GroupType = ECrowd_GroupType::MigrationGroup;
    MigrationGroup.AgentCount = 12;
    MigrationGroup.HomeLocation = FVector(-2000.f, 0.f, 400.f);
    MigrationGroup.WanderRadius = 5000.f;
    MigrationGroup.MovementSpeed = 180.f;
    MigrationGroup.FleeThreshold = 0.3f;
    MigrationGroup.bIsAlerting = false;
    MigrationGroup.AlertLevel = 0.0f;
    RegisterCrowdGroup(MigrationGroup);

    // Scavenger flock — pterosaurs
    FCrowd_GroupData ScavengerFlock;
    ScavengerFlock.GroupID = FName("ScavengerFlock_Ptero");
    ScavengerFlock.GroupType = ECrowd_GroupType::ScavengerFlock;
    ScavengerFlock.AgentCount = 6;
    ScavengerFlock.HomeLocation = FVector(1000.f, 1000.f, 1200.f);
    ScavengerFlock.WanderRadius = 3000.f;
    ScavengerFlock.MovementSpeed = 400.f;
    ScavengerFlock.FleeThreshold = 0.6f;
    ScavengerFlock.bIsAlerting = false;
    ScavengerFlock.AlertLevel = 0.0f;
    RegisterCrowdGroup(ScavengerFlock);

    CurrentAgentCount = 0;
    for (const FCrowd_GroupData& G : CrowdGroups)
    {
        CurrentAgentCount += G.AgentCount;
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Registered %d groups, %d total agents"), CrowdGroups.Num(), CurrentAgentCount);
}

void UCrowdSimulationManager::RegisterCrowdGroup(const FCrowd_GroupData& GroupData)
{
    // Prevent duplicate group IDs
    for (const FCrowd_GroupData& Existing : CrowdGroups)
    {
        if (Existing.GroupID == GroupData.GroupID)
        {
            UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Group %s already registered, skipping"), *GroupData.GroupID.ToString());
            return;
        }
    }
    CrowdGroups.Add(GroupData);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Registered group: %s (%d agents)"), *GroupData.GroupID.ToString(), GroupData.AgentCount);
}

// ─────────────────────────────────────────────────────────────────────────────
// Alert System
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::AlertGroupsInRadius(FVector Origin, float Radius, float AlertStrength)
{
    int32 AlertedCount = 0;
    for (FCrowd_GroupData& Group : CrowdGroups)
    {
        float Distance = FVector::Dist(Group.HomeLocation, Origin);
        if (Distance <= Radius)
        {
            float FalloffFactor = 1.0f - (Distance / Radius);
            float AppliedAlert = AlertStrength * FalloffFactor;
            Group.AlertLevel = FMath::Clamp(Group.AlertLevel + AppliedAlert, 0.0f, 1.0f);
            Group.bIsAlerting = (Group.AlertLevel >= Group.FleeThreshold);
            AlertedCount++;
            UE_LOG(LogTemp, Verbose, TEXT("[CrowdSim] Group %s alerted: %.2f (dist=%.0f)"),
                *Group.GroupID.ToString(), Group.AlertLevel, Distance);
        }
    }

    // Update global alert level
    GlobalAlertLevel = FMath::Clamp(GlobalAlertLevel + (AlertStrength * 0.3f), 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Alert propagated to %d groups from (%.0f,%.0f,%.0f) r=%.0f"),
        AlertedCount, Origin.X, Origin.Y, Origin.Z, Radius);
}

void UCrowdSimulationManager::ResetAlerts()
{
    for (FCrowd_GroupData& Group : CrowdGroups)
    {
        Group.AlertLevel = FMath::Max(0.0f, Group.AlertLevel - 0.1f);
        if (Group.AlertLevel < Group.FleeThreshold)
        {
            Group.bIsAlerting = false;
        }
    }
    GlobalAlertLevel = FMath::Max(0.0f, GlobalAlertLevel - 0.05f);
}

// ─────────────────────────────────────────────────────────────────────────────
// LOD Management
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::UpdateCrowdLOD(FVector PlayerLocation)
{
    for (FCrowd_GroupData& Group : CrowdGroups)
    {
        float Distance = FVector::Dist(Group.HomeLocation, PlayerLocation);

        if (Distance < 1500.f)
        {
            Group.CurrentLOD = ECrowd_LODLevel::LOD_Full;
        }
        else if (Distance < 4000.f)
        {
            Group.CurrentLOD = ECrowd_LODLevel::LOD_Medium;
        }
        else if (Distance < 8000.f)
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
// Pathfinding Queries
// ─────────────────────────────────────────────────────────────────────────────

FVector UCrowdSimulationManager::GetFleeDestination(FName GroupID, FVector ThreatLocation)
{
    for (const FCrowd_GroupData& Group : CrowdGroups)
    {
        if (Group.GroupID == GroupID)
        {
            // Flee directly away from threat
            FVector FleeDir = (Group.HomeLocation - ThreatLocation).GetSafeNormal();
            float FleeDistance = Group.WanderRadius * 0.8f;
            FVector FleeDest = Group.HomeLocation + (FleeDir * FleeDistance);
            UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group %s fleeing to (%.0f,%.0f,%.0f)"),
                *GroupID.ToString(), FleeDest.X, FleeDest.Y, FleeDest.Z);
            return FleeDest;
        }
    }
    return ThreatLocation + FVector(2000.f, 0.f, 0.f);
}

FVector UCrowdSimulationManager::GetWanderTarget(FName GroupID)
{
    for (const FCrowd_GroupData& Group : CrowdGroups)
    {
        if (Group.GroupID == GroupID)
        {
            // Random point within wander radius of home
            float Angle = FMath::RandRange(0.f, 360.f) * (PI / 180.f);
            float Dist = FMath::RandRange(Group.WanderRadius * 0.2f, Group.WanderRadius);
            FVector WanderTarget = Group.HomeLocation + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
            return WanderTarget;
        }
    }
    return FVector::ZeroVector;
}

// ─────────────────────────────────────────────────────────────────────────────
// Simulation Tick
// ─────────────────────────────────────────────────────────────────────────────

void UCrowdSimulationManager::StartSimulationTick()
{
    UWorld* World = GetWorld();
    if (!World) return;

    bSimulationActive = true;
    World->GetTimerManager().SetTimer(
        SimulationTickHandle,
        this,
        &UCrowdSimulationManager::OnSimulationTick,
        SimulationTickRate,
        true
    );
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Simulation tick started at %.2fHz"), 1.0f / SimulationTickRate);
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
    if (!bSimulationActive) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Decay alerts over time
    ResetAlerts();

    // Update LOD based on player location
    APlayerController* PC = World->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
        UpdateCrowdLOD(PlayerLoc);
    }

    // Tick each group's behavioral state
    for (FCrowd_GroupData& Group : CrowdGroups)
    {
        TickGroupBehavior(Group);
    }
}

void UCrowdSimulationManager::TickGroupBehavior(FCrowd_GroupData& Group)
{
    if (Group.CurrentLOD == ECrowd_LODLevel::LOD_Culled)
    {
        return; // Skip fully culled groups
    }

    if (Group.bIsAlerting)
    {
        // Fleeing behavior — move home location toward flee destination
        UWorld* World = GetWorld();
        APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
        if (PC && PC->GetPawn())
        {
            FVector ThreatLoc = PC->GetPawn()->GetActorLocation();
            FVector FleeDest = GetFleeDestination(Group.GroupID, ThreatLoc);
            FVector MoveDir = (FleeDest - Group.HomeLocation).GetSafeNormal();
            float DeltaTime = SimulationTickRate;
            Group.HomeLocation += MoveDir * Group.MovementSpeed * DeltaTime;
        }
    }
    else
    {
        // Idle wander — small drift toward wander target
        // (In full implementation, each agent has individual target)
        // For simulation, we drift the group centroid slightly
        float DriftAmount = FMath::RandRange(-50.f, 50.f);
        Group.HomeLocation.X += DriftAmount * SimulationTickRate;
        Group.HomeLocation.Y += DriftAmount * SimulationTickRate;
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
