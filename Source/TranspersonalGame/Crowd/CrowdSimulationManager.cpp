// ============================================================
// Agent #13 — Crowd & Traffic Simulation
// CrowdSimulationManager.cpp — Full implementation
// Prehistoric survival game — Mass AI crowd system
// Up to 50,000 simultaneous agents with LOD chain
// ============================================================

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxAgents = 50000;
    ActiveAgentCount = 0;
    NextHerdID = 0;
    bSimulationActive = false;
    SimulationTickRate = 0.1f; // 10Hz update for performance
    LODFullRadius = 5000.0f;
    LODReducedRadius = 20000.0f;
    LODMinimalRadius = 50000.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initializing Crowd Simulation Manager — max agents: %d"), MaxAgents);
    
    // Reserve memory for agent pools
    AgentPool.Reserve(FMath::Min(MaxAgents, 10000));
    HerdRegistry.Reserve(500);
    ActiveStampedeEvents.Reserve(32);
    
    bSimulationActive = true;
    
    // Start simulation tick timer
    GetWorld()->GetTimerManager().SetTimer(
        SimulationTickHandle,
        this,
        &UCrowdSimulationManager::SimulationTick,
        SimulationTickRate,
        true
    );
    
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Simulation started at %.1fHz"), 1.0f / SimulationTickRate);
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(SimulationTickHandle);
    }
    
    AgentPool.Empty();
    HerdRegistry.Empty();
    ActiveStampedeEvents.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Simulation stopped — %d agents cleaned up"), ActiveAgentCount);
    
    Super::Deinitialize();
}

void UCrowdSimulationManager::SimulationTick()
{
    if (!bSimulationActive) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Get player location for LOD calculations
    APlayerController* PC = World->GetFirstPlayerController();
    FVector PlayerLocation = FVector::ZeroVector;
    if (PC && PC->GetPawn())
    {
        PlayerLocation = PC->GetPawn()->GetActorLocation();
    }
    
    float CurrentTime = World->GetTimeSeconds();
    
    // Update stampede events — decay over time
    for (int32 i = ActiveStampedeEvents.Num() - 1; i >= 0; --i)
    {
        FCrowd_StampedeEvent& Event = ActiveStampedeEvents[i];
        float Age = CurrentTime - Event.Timestamp;
        if (Age > 30.0f) // Stampede lasts 30 seconds max
        {
            ActiveStampedeEvents.RemoveAt(i);
        }
        else
        {
            // Decay intensity over time
            Event.Intensity = FMath::Max(0.0f, 1.0f - (Age / 30.0f));
        }
    }
    
    // Update herd centers
    UpdateHerdCenters();
    
    // Update LOD for all agents based on player distance
    UpdateAgentLOD(PlayerLocation);
    
    // Process agent state machines
    ProcessAgentStates(CurrentTime, PlayerLocation);
}

void UCrowdSimulationManager::UpdateHerdCenters()
{
    // Reset herd centers
    TMap<int32, FVector> HerdPositionSums;
    TMap<int32, int32> HerdMemberCounts;
    
    for (auto& Pair : HerdRegistry)
    {
        HerdPositionSums.Add(Pair.Key, FVector::ZeroVector);
        HerdMemberCounts.Add(Pair.Key, 0);
        Pair.Value.MemberCount = 0;
    }
    
    // Sum agent positions per herd
    for (const FCrowd_AgentRuntimeData& Agent : AgentPool)
    {
        if (Agent.HerdID >= 0 && HerdRegistry.Contains(Agent.HerdID))
        {
            HerdPositionSums[Agent.HerdID] += Agent.CurrentTarget;
            HerdMemberCounts[Agent.HerdID]++;
        }
    }
    
    // Compute averages
    for (auto& Pair : HerdRegistry)
    {
        int32 Count = HerdMemberCounts[Pair.Key];
        if (Count > 0)
        {
            Pair.Value.HerdCenter = HerdPositionSums[Pair.Key] / (float)Count;
            Pair.Value.MemberCount = Count;
        }
    }
}

void UCrowdSimulationManager::UpdateAgentLOD(const FVector& PlayerLocation)
{
    for (FCrowd_AgentRuntimeData& Agent : AgentPool)
    {
        float DistSq = FVector::DistSquared(Agent.CurrentTarget, PlayerLocation);
        
        if (DistSq < LODFullRadius * LODFullRadius)
        {
            Agent.LODLevel = ECrowd_LODLevel::Full;
        }
        else if (DistSq < LODReducedRadius * LODReducedRadius)
        {
            Agent.LODLevel = ECrowd_LODLevel::Reduced;
        }
        else if (DistSq < LODMinimalRadius * LODMinimalRadius)
        {
            Agent.LODLevel = ECrowd_LODLevel::Minimal;
        }
        else
        {
            Agent.LODLevel = ECrowd_LODLevel::Ghost;
        }
    }
}

void UCrowdSimulationManager::ProcessAgentStates(float CurrentTime, const FVector& PlayerLocation)
{
    for (FCrowd_AgentRuntimeData& Agent : AgentPool)
    {
        // Skip ghost-LOD agents (too far — no simulation)
        if (Agent.LODLevel == ECrowd_LODLevel::Ghost) continue;
        
        // Skip dead agents
        if (Agent.CurrentState == ECrowd_AgentState::Dead) continue;
        
        Agent.StateTimer += SimulationTickRate;
        
        switch (Agent.CurrentState)
        {
            case ECrowd_AgentState::Idle:
                ProcessIdleState(Agent, CurrentTime);
                break;
                
            case ECrowd_AgentState::Wandering:
                ProcessWanderingState(Agent, CurrentTime);
                break;
                
            case ECrowd_AgentState::Grazing:
                ProcessGrazingState(Agent, CurrentTime);
                break;
                
            case ECrowd_AgentState::Fleeing:
                ProcessFleeingState(Agent, CurrentTime, PlayerLocation);
                break;
                
            case ECrowd_AgentState::Stampeding:
                ProcessStampedeState(Agent, CurrentTime);
                break;
                
            case ECrowd_AgentState::Migrating:
                ProcessMigratingState(Agent, CurrentTime);
                break;
                
            default:
                break;
        }
        
        // Check for stampede contagion from nearby events
        CheckStampedeContagion(Agent, CurrentTime);
    }
}

void UCrowdSimulationManager::ProcessIdleState(FCrowd_AgentRuntimeData& Agent, float CurrentTime)
{
    // After 3-8 seconds of idle, transition to wandering or grazing
    float IdleDuration = FMath::RandRange(3.0f, 8.0f);
    if (Agent.StateTimer > IdleDuration)
    {
        Agent.StateTimer = 0.0f;
        float Roll = FMath::FRand();
        if (Roll < 0.4f)
        {
            Agent.CurrentState = ECrowd_AgentState::Grazing;
        }
        else
        {
            Agent.CurrentState = ECrowd_AgentState::Wandering;
            // Pick a random wander target
            FVector RandomOffset = FVector(
                FMath::RandRange(-500.0f, 500.0f),
                FMath::RandRange(-500.0f, 500.0f),
                0.0f
            );
            Agent.CurrentTarget = Agent.CurrentTarget + RandomOffset;
        }
    }
}

void UCrowdSimulationManager::ProcessWanderingState(FCrowd_AgentRuntimeData& Agent, float CurrentTime)
{
    // After reaching target or 10 seconds, go idle
    if (Agent.StateTimer > 10.0f)
    {
        Agent.CurrentState = ECrowd_AgentState::Idle;
        Agent.StateTimer = 0.0f;
    }
}

void UCrowdSimulationManager::ProcessGrazingState(FCrowd_AgentRuntimeData& Agent, float CurrentTime)
{
    // Graze for 5-15 seconds then idle
    float GrazeDuration = FMath::RandRange(5.0f, 15.0f);
    if (Agent.StateTimer > GrazeDuration)
    {
        Agent.CurrentState = ECrowd_AgentState::Idle;
        Agent.StateTimer = 0.0f;
    }
}

void UCrowdSimulationManager::ProcessFleeingState(FCrowd_AgentRuntimeData& Agent, float CurrentTime, const FVector& PlayerLocation)
{
    // Flee for 8-15 seconds then slow down
    float FleeDuration = FMath::RandRange(8.0f, 15.0f);
    if (Agent.StateTimer > FleeDuration)
    {
        Agent.CurrentState = ECrowd_AgentState::Wandering;
        Agent.FleeReason = ECrowd_FleeReason::None;
        Agent.StateTimer = 0.0f;
    }
}

void UCrowdSimulationManager::ProcessStampedeState(FCrowd_AgentRuntimeData& Agent, float CurrentTime)
{
    // Stampede runs for 20-40 seconds
    float StampedeDuration = FMath::RandRange(20.0f, 40.0f);
    if (Agent.StateTimer > StampedeDuration)
    {
        Agent.CurrentState = ECrowd_AgentState::Fleeing;
        Agent.StateTimer = 0.0f;
    }
}

void UCrowdSimulationManager::ProcessMigratingState(FCrowd_AgentRuntimeData& Agent, float CurrentTime)
{
    // Migration continues until reaching target (handled by movement component)
    // After 120 seconds, reset to idle (arrived or gave up)
    if (Agent.StateTimer > 120.0f)
    {
        Agent.CurrentState = ECrowd_AgentState::Idle;
        Agent.StateTimer = 0.0f;
    }
}

void UCrowdSimulationManager::CheckStampedeContagion(FCrowd_AgentRuntimeData& Agent, float CurrentTime)
{
    if (Agent.CurrentState == ECrowd_AgentState::Stampeding) return;
    if (Agent.CurrentState == ECrowd_AgentState::Dead) return;
    
    // Check if any stampede event is close enough to trigger contagion
    for (const FCrowd_StampedeEvent& Event : ActiveStampedeEvents)
    {
        float DistSq = FVector::DistSquared(Agent.CurrentTarget, Event.Origin);
        float ContagionRadiusSq = 600.0f * 600.0f; // Default contagion radius
        
        if (DistSq < ContagionRadiusSq && Event.Intensity > 0.3f)
        {
            // Trigger stampede on this agent
            Agent.CurrentState = ECrowd_AgentState::Stampeding;
            Agent.FleeDirection = Event.Direction;
            Agent.StateTimer = 0.0f;
            Agent.LastStampedeSignalTime = CurrentTime;
            break;
        }
    }
}

// ============================================================
// Public API — Blueprint callable
// ============================================================

void UCrowdSimulationManager::TriggerStampedeEvent(FVector Origin, FVector Direction, float Radius, ECrowd_FleeReason Cause)
{
    FCrowd_StampedeEvent NewEvent;
    NewEvent.Origin = Origin;
    NewEvent.Direction = Direction.GetSafeNormal();
    NewEvent.Radius = Radius;
    NewEvent.Intensity = 1.0f;
    NewEvent.Cause = Cause;
    NewEvent.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    ActiveStampedeEvents.Add(NewEvent);
    
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Stampede triggered at (%.0f, %.0f, %.0f) radius=%.0f cause=%d"),
        Origin.X, Origin.Y, Origin.Z, Radius, (int32)Cause);
}

int32 UCrowdSimulationManager::RegisterHerd(ECrowd_AgentSpecies Species, FVector Center, int32 InitialSize)
{
    FCrowd_HerdData NewHerd;
    NewHerd.HerdID = NextHerdID++;
    NewHerd.Species = Species;
    NewHerd.HerdCenter = Center;
    NewHerd.MemberCount = InitialSize;
    NewHerd.bIsStampeding = false;
    NewHerd.StampedeIntensity = 0.0f;
    
    HerdRegistry.Add(NewHerd.HerdID, NewHerd);
    
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd registered: ID=%d Species=%d Size=%d"),
        NewHerd.HerdID, (int32)Species, InitialSize);
    
    return NewHerd.HerdID;
}

void UCrowdSimulationManager::SetHerdMigrationTarget(int32 HerdID, FVector Target)
{
    if (FCrowd_HerdData* Herd = HerdRegistry.Find(HerdID))
    {
        Herd->MigrationTarget = Target;
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd %d migration target set to (%.0f, %.0f, %.0f)"),
            HerdID, Target.X, Target.Y, Target.Z);
    }
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return AgentPool.Num();
}

int32 UCrowdSimulationManager::GetHerdCount() const
{
    return HerdRegistry.Num();
}

FCrowd_HerdData UCrowdSimulationManager::GetHerdData(int32 HerdID) const
{
    if (const FCrowd_HerdData* Herd = HerdRegistry.Find(HerdID))
    {
        return *Herd;
    }
    return FCrowd_HerdData();
}

bool UCrowdSimulationManager::IsSimulationActive() const
{
    return bSimulationActive;
}

void UCrowdSimulationManager::SetSimulationActive(bool bActive)
{
    bSimulationActive = bActive;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Simulation %s"), bActive ? TEXT("RESUMED") : TEXT("PAUSED"));
}
