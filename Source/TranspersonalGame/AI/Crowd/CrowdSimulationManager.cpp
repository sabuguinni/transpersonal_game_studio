// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric survival game — tribal crowd AI using UE5 Mass Entity framework
// Up to 50,000 simultaneous agents with LOD chain

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

// ─────────────────────────────────────────────────────────────
// UCrowdSimulationManager — UWorldSubsystem implementation
// ─────────────────────────────────────────────────────────────

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize agent pools for each LOD tier
    AgentPool_LOD0.Reserve(MAX_AGENTS_LOD0);
    AgentPool_LOD1.Reserve(MAX_AGENTS_LOD1);
    AgentPool_LOD2.Reserve(MAX_AGENTS_LOD2);

    // Set default simulation parameters
    SimulationRadius = 5000.0f;
    bSimulationActive = true;
    TickInterval_LOD0 = 0.05f;   // 20Hz for close agents
    TickInterval_LOD1 = 0.2f;    // 5Hz for mid-range
    TickInterval_LOD2 = 1.0f;    // 1Hz for distant agents

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized. Max agents: LOD0=%d LOD1=%d LOD2=%d"),
        MAX_AGENTS_LOD0, MAX_AGENTS_LOD1, MAX_AGENTS_LOD2);
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    AgentPool_LOD0.Empty();
    AgentPool_LOD1.Empty();
    AgentPool_LOD2.Empty();
    RegisteredGroups.Empty();

    Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────
// Agent Registration
// ─────────────────────────────────────────────────────────────

int32 UCrowdSimulationManager::RegisterAgent(AActor* AgentActor, ECrowd_AgentType AgentType)
{
    if (!AgentActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] RegisterAgent: null actor"));
        return INDEX_NONE;
    }

    FCrowd_AgentData NewAgent;
    NewAgent.AgentID = NextAgentID++;
    NewAgent.AgentActor = AgentActor;
    NewAgent.AgentType = AgentType;
    NewAgent.CurrentLocation = AgentActor->GetActorLocation();
    NewAgent.CurrentState = ECrowd_AgentState::Idle;
    NewAgent.LODLevel = 0;
    NewAgent.bIsActive = true;
    NewAgent.WanderRadius = 800.0f;
    NewAgent.HomeLocation = NewAgent.CurrentLocation;

    // Assign to LOD0 initially (will be sorted on tick)
    AgentPool_LOD0.Add(NewAgent);

    UE_LOG(LogTemp, Verbose, TEXT("[CrowdSim] Registered agent %d type=%d at %s"),
        NewAgent.AgentID,
        (int32)AgentType,
        *NewAgent.CurrentLocation.ToString());

    return NewAgent.AgentID;
}

void UCrowdSimulationManager::UnregisterAgent(int32 AgentID)
{
    // Search all LOD pools
    for (int32 i = AgentPool_LOD0.Num() - 1; i >= 0; --i)
    {
        if (AgentPool_LOD0[i].AgentID == AgentID)
        {
            AgentPool_LOD0.RemoveAtSwap(i);
            return;
        }
    }
    for (int32 i = AgentPool_LOD1.Num() - 1; i >= 0; --i)
    {
        if (AgentPool_LOD1[i].AgentID == AgentID)
        {
            AgentPool_LOD1.RemoveAtSwap(i);
            return;
        }
    }
    for (int32 i = AgentPool_LOD2.Num() - 1; i >= 0; --i)
    {
        if (AgentPool_LOD2[i].AgentID == AgentID)
        {
            AgentPool_LOD2.RemoveAtSwap(i);
            return;
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Group Management
// ─────────────────────────────────────────────────────────────

int32 UCrowdSimulationManager::CreateGroup(ECrowd_GroupBehavior Behavior, FVector GroupCenter)
{
    FCrowd_GroupData NewGroup;
    NewGroup.GroupID = NextGroupID++;
    NewGroup.GroupBehavior = Behavior;
    NewGroup.GroupCenter = GroupCenter;
    NewGroup.bIsActive = true;
    NewGroup.MaxSize = 20;
    NewGroup.CohesionRadius = 1200.0f;
    NewGroup.AlertLevel = 0.0f;

    RegisteredGroups.Add(NewGroup.GroupID, NewGroup);

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Created group %d behavior=%d at %s"),
        NewGroup.GroupID, (int32)Behavior, *GroupCenter.ToString());

    return NewGroup.GroupID;
}

bool UCrowdSimulationManager::AddAgentToGroup(int32 AgentID, int32 GroupID)
{
    FCrowd_GroupData* Group = RegisteredGroups.Find(GroupID);
    if (!Group)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] AddAgentToGroup: group %d not found"), GroupID);
        return false;
    }

    if (Group->MemberIDs.Num() >= Group->MaxSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Group %d is full (%d/%d)"),
            GroupID, Group->MemberIDs.Num(), Group->MaxSize);
        return false;
    }

    Group->MemberIDs.AddUnique(AgentID);
    return true;
}

// ─────────────────────────────────────────────────────────────
// LOD Management
// ─────────────────────────────────────────────────────────────

void UCrowdSimulationManager::UpdateAgentLOD(FVector PlayerLocation)
{
    // Move agents between LOD pools based on distance to player
    // LOD0: < 2000 units (full simulation, visible mesh)
    // LOD1: 2000-8000 units (simplified movement, no mesh)
    // LOD2: > 8000 units (state machine only, no movement)

    const float LOD0_Dist = 2000.0f;
    const float LOD1_Dist = 8000.0f;

    TArray<FCrowd_AgentData> ToLOD0, ToLOD1, ToLOD2;

    auto ClassifyAgent = [&](FCrowd_AgentData& Agent)
    {
        if (!Agent.bIsActive) return;
        float Dist = FVector::Dist(Agent.CurrentLocation, PlayerLocation);
        if (Dist < LOD0_Dist)
        {
            Agent.LODLevel = 0;
            ToLOD0.Add(Agent);
        }
        else if (Dist < LOD1_Dist)
        {
            Agent.LODLevel = 1;
            ToLOD1.Add(Agent);
        }
        else
        {
            Agent.LODLevel = 2;
            ToLOD2.Add(Agent);
        }
    };

    for (auto& A : AgentPool_LOD0) ClassifyAgent(A);
    for (auto& A : AgentPool_LOD1) ClassifyAgent(A);
    for (auto& A : AgentPool_LOD2) ClassifyAgent(A);

    // Enforce pool limits
    while (ToLOD0.Num() > MAX_AGENTS_LOD0) { ToLOD1.Add(ToLOD0.Last()); ToLOD0.Pop(); }
    while (ToLOD1.Num() > MAX_AGENTS_LOD1) { ToLOD2.Add(ToLOD1.Last()); ToLOD1.Pop(); }

    AgentPool_LOD0 = MoveTemp(ToLOD0);
    AgentPool_LOD1 = MoveTemp(ToLOD1);
    AgentPool_LOD2 = MoveTemp(ToLOD2);
}

// ─────────────────────────────────────────────────────────────
// Panic System — Dinosaur Threat Response
// ─────────────────────────────────────────────────────────────

void UCrowdSimulationManager::TriggerPanic(FVector ThreatLocation, float ThreatRadius, float PanicIntensity)
{
    if (!bSimulationActive) return;

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] PANIC triggered at %s radius=%.0f intensity=%.2f"),
        *ThreatLocation.ToString(), ThreatRadius, PanicIntensity);

    int32 AffectedCount = 0;

    auto ProcessPanic = [&](TArray<FCrowd_AgentData>& Pool)
    {
        for (FCrowd_AgentData& Agent : Pool)
        {
            if (!Agent.bIsActive) continue;
            float Dist = FVector::Dist(Agent.CurrentLocation, ThreatLocation);
            if (Dist > ThreatRadius) continue;

            // Calculate panic based on distance (closer = more panic)
            float DistFactor = 1.0f - (Dist / ThreatRadius);
            float AgentPanic = PanicIntensity * DistFactor;

            // Set agent to flee state
            Agent.CurrentState = ECrowd_AgentState::Fleeing;
            Agent.PanicLevel = FMath::Clamp(Agent.PanicLevel + AgentPanic, 0.0f, 1.0f);

            // Calculate flee direction (away from threat)
            FVector FleeDir = (Agent.CurrentLocation - ThreatLocation).GetSafeNormal();
            Agent.TargetLocation = Agent.CurrentLocation + FleeDir * (800.0f + AgentPanic * 1200.0f);

            AffectedCount++;
        }
    };

    ProcessPanic(AgentPool_LOD0);
    ProcessPanic(AgentPool_LOD1);
    ProcessPanic(AgentPool_LOD2);

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Panic affected %d agents"), AffectedCount);

    // Propagate panic through groups
    for (auto& GroupPair : RegisteredGroups)
    {
        FCrowd_GroupData& Group = GroupPair.Value;
        float GroupDist = FVector::Dist(Group.GroupCenter, ThreatLocation);
        if (GroupDist < ThreatRadius * 1.5f)
        {
            Group.AlertLevel = FMath::Min(1.0f, Group.AlertLevel + PanicIntensity * 0.8f);
            Group.GroupBehavior = ECrowd_GroupBehavior::Fleeing;
        }
    }
}

void UCrowdSimulationManager::CalmAgents(FVector CalmCenter, float CalmRadius)
{
    auto ProcessCalm = [&](TArray<FCrowd_AgentData>& Pool)
    {
        for (FCrowd_AgentData& Agent : Pool)
        {
            if (!Agent.bIsActive) continue;
            float Dist = FVector::Dist(Agent.CurrentLocation, CalmCenter);
            if (Dist > CalmRadius) continue;

            Agent.PanicLevel = FMath::Max(0.0f, Agent.PanicLevel - 0.3f);
            if (Agent.PanicLevel < 0.1f)
            {
                Agent.CurrentState = ECrowd_AgentState::Idle;
            }
        }
    };

    ProcessCalm(AgentPool_LOD0);
    ProcessCalm(AgentPool_LOD1);
    ProcessCalm(AgentPool_LOD2);
}

// ─────────────────────────────────────────────────────────────
// Tribal Behavior Patterns
// ─────────────────────────────────────────────────────────────

void UCrowdSimulationManager::SetGroupBehavior(int32 GroupID, ECrowd_GroupBehavior NewBehavior)
{
    FCrowd_GroupData* Group = RegisteredGroups.Find(GroupID);
    if (!Group) return;

    Group->GroupBehavior = NewBehavior;

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group %d behavior changed to %d"),
        GroupID, (int32)NewBehavior);
}

void UCrowdSimulationManager::SpawnTribalGroup(FVector Location, int32 MemberCount, ECrowd_GroupBehavior InitialBehavior)
{
    if (!GetWorld()) return;

    // Create the group
    int32 GroupID = CreateGroup(InitialBehavior, Location);

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Spawning tribal group %d with %d members at %s"),
        GroupID, MemberCount, *Location.ToString());

    // In a full implementation, this would spawn actual pawns
    // For now, register virtual agents (Mass Entity style)
    for (int32 i = 0; i < MemberCount; ++i)
    {
        FCrowd_AgentData VirtualAgent;
        VirtualAgent.AgentID = NextAgentID++;
        VirtualAgent.AgentActor = nullptr; // Virtual agent — no physical actor at LOD2
        VirtualAgent.AgentType = ECrowd_AgentType::TribalHuman;
        VirtualAgent.CurrentState = ECrowd_AgentState::Idle;
        VirtualAgent.LODLevel = 2; // Start at LOD2, promote when player approaches
        VirtualAgent.bIsActive = true;

        // Scatter around group center
        float Angle = (float)i / (float)MemberCount * 2.0f * PI;
        float Radius = FMath::RandRange(100.0f, 400.0f);
        VirtualAgent.CurrentLocation = Location + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );
        VirtualAgent.HomeLocation = VirtualAgent.CurrentLocation;
        VirtualAgent.WanderRadius = 600.0f;
        VirtualAgent.PanicLevel = 0.0f;

        AgentPool_LOD2.Add(VirtualAgent);

        // Add to group
        FCrowd_GroupData* Group = RegisteredGroups.Find(GroupID);
        if (Group)
        {
            Group->MemberIDs.Add(VirtualAgent.AgentID);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Tribal group %d spawned. Total LOD2 agents: %d"),
        GroupID, AgentPool_LOD2.Num());
}

// ─────────────────────────────────────────────────────────────
// Statistics
// ─────────────────────────────────────────────────────────────

int32 UCrowdSimulationManager::GetTotalAgentCount() const
{
    return AgentPool_LOD0.Num() + AgentPool_LOD1.Num() + AgentPool_LOD2.Num();
}

FCrowd_SimulationStats UCrowdSimulationManager::GetSimulationStats() const
{
    FCrowd_SimulationStats Stats;
    Stats.TotalAgents = GetTotalAgentCount();
    Stats.ActiveLOD0 = AgentPool_LOD0.Num();
    Stats.ActiveLOD1 = AgentPool_LOD1.Num();
    Stats.ActiveLOD2 = AgentPool_LOD2.Num();
    Stats.TotalGroups = RegisteredGroups.Num();
    Stats.bSimulationRunning = bSimulationActive;

    // Count panicking agents
    int32 PanicCount = 0;
    for (const auto& A : AgentPool_LOD0) if (A.CurrentState == ECrowd_AgentState::Fleeing) PanicCount++;
    for (const auto& A : AgentPool_LOD1) if (A.CurrentState == ECrowd_AgentState::Fleeing) PanicCount++;
    for (const auto& A : AgentPool_LOD2) if (A.CurrentState == ECrowd_AgentState::Fleeing) PanicCount++;
    Stats.PanickingAgents = PanicCount;

    return Stats;
}
