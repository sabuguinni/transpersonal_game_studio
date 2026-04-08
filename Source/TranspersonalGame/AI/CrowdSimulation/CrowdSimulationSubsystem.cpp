#include "CrowdSimulationSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogCrowdSimulation);

void UCrowdSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Initializing Crowd Simulation Subsystem"));
    
    // Get Mass Entity subsystem references
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Failed to get MassEntitySubsystem"));
        return;
    }
    
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogCrowdSimulation, Error, TEXT("Failed to get MassSpawnerSubsystem"));
        return;
    }
    
    // Initialize simulation state
    bIsSimulationActive = false;
    ActiveAgentCount = 0;
    CurrentPerformanceLoad = 0.0f;
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Crowd Simulation Subsystem initialized successfully"));
}

void UCrowdSimulationSubsystem::Deinitialize()
{
    UE_LOG(LogCrowdSimulation, Log, TEXT("Deinitializing Crowd Simulation Subsystem"));
    
    StopEcosystemSimulation();
    
    Super::Deinitialize();
}

bool UCrowdSimulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UCrowdSimulationSubsystem::StartEcosystemSimulation()
{
    if (bIsSimulationActive)
    {
        UE_LOG(LogCrowdSimulation, Warning, TEXT("Ecosystem simulation is already active"));
        return;
    }
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Starting ecosystem simulation"));
    
    bIsSimulationActive = true;
    
    // Start ecosystem update timer
    GetWorld()->GetTimerManager().SetTimer(
        EcosystemUpdateTimer,
        this,
        &UCrowdSimulationSubsystem::UpdateEcosystemState,
        EcosystemUpdateFrequency,
        true
    );
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Ecosystem simulation started successfully"));
}

void UCrowdSimulationSubsystem::StopEcosystemSimulation()
{
    if (!bIsSimulationActive)
    {
        return;
    }
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Stopping ecosystem simulation"));
    
    bIsSimulationActive = false;
    
    // Clear ecosystem update timer
    GetWorld()->GetTimerManager().ClearTimer(EcosystemUpdateTimer);
    
    // Despawn all active herds
    for (int32 HerdID : ActiveHerdIDs)
    {
        DespawnHerd(HerdID);
    }
    ActiveHerdIDs.Empty();
    
    ActiveAgentCount = 0;
    CurrentPerformanceLoad = 0.0f;
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Ecosystem simulation stopped"));
}

void UCrowdSimulationSubsystem::PauseEcosystemSimulation(bool bPause)
{
    if (!bIsSimulationActive)
    {
        return;
    }
    
    if (bPause)
    {
        GetWorld()->GetTimerManager().PauseTimer(EcosystemUpdateTimer);
        UE_LOG(LogCrowdSimulation, Log, TEXT("Ecosystem simulation paused"));
    }
    else
    {
        GetWorld()->GetTimerManager().UnPauseTimer(EcosystemUpdateTimer);
        UE_LOG(LogCrowdSimulation, Log, TEXT("Ecosystem simulation resumed"));
    }
}

void UCrowdSimulationSubsystem::SpawnDinosaurHerd(const FVector& Location, int32 HerdSize, const FString& SpeciesType)
{
    if (!bIsSimulationActive)
    {
        UE_LOG(LogCrowdSimulation, Warning, TEXT("Cannot spawn herd - ecosystem simulation not active"));
        return;
    }
    
    if (ActiveAgentCount + HerdSize > MaxSimultaneousAgents)
    {
        UE_LOG(LogCrowdSimulation, Warning, TEXT("Cannot spawn herd - would exceed max agent limit (%d/%d)"), 
               ActiveAgentCount + HerdSize, MaxSimultaneousAgents);
        return;
    }
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Spawning %s herd of %d at location %s"), 
           *SpeciesType, HerdSize, *Location.ToString());
    
    // Generate unique herd ID
    int32 NewHerdID = ActiveHerdIDs.Num() + 1;
    ActiveHerdIDs.Add(NewHerdID);
    
    // Update agent count
    ActiveAgentCount += HerdSize;
    
    // TODO: Implement actual Mass Entity spawning logic here
    // This will be connected to the Mass spawning system
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Herd %d spawned successfully. Active agents: %d"), 
           NewHerdID, ActiveAgentCount);
}

void UCrowdSimulationSubsystem::DespawnHerd(int32 HerdID)
{
    if (!ActiveHerdIDs.Contains(HerdID))
    {
        UE_LOG(LogCrowdSimulation, Warning, TEXT("Cannot despawn herd %d - not found"), HerdID);
        return;
    }
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Despawning herd %d"), HerdID);
    
    // TODO: Implement actual Mass Entity despawning logic here
    
    ActiveHerdIDs.Remove(HerdID);
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Herd %d despawned successfully"), HerdID);
}

void UCrowdSimulationSubsystem::NotifyPlayerPresence(const FVector& PlayerLocation, float ThreatLevel)
{
    if (!bIsSimulationActive)
    {
        return;
    }
    
    UE_LOG(LogCrowdSimulation, VeryVerbose, TEXT("Player presence detected at %s with threat level %.2f"), 
           *PlayerLocation.ToString(), ThreatLevel);
    
    // TODO: Implement player presence reaction logic
    // - Nearby herbivores should become alert or flee
    // - Predators might investigate or stalk
    // - Herd behaviors should adjust based on threat level
}

void UCrowdSimulationSubsystem::TriggerPanicResponse(const FVector& ThreatLocation, float PanicRadius)
{
    if (!bIsSimulationActive)
    {
        return;
    }
    
    UE_LOG(LogCrowdSimulation, Log, TEXT("Triggering panic response at %s with radius %.2f"), 
           *ThreatLocation.ToString(), PanicRadius);
    
    // TODO: Implement panic response logic
    // - All agents within radius should enter panic state
    // - Stampede behaviors for large herds
    // - Predator-prey dynamics during panic
}

void UCrowdSimulationSubsystem::UpdateEcosystemState()
{
    if (!bIsSimulationActive)
    {
        return;
    }
    
    // Update performance monitoring
    ManagePerformance();
    
    // Update herd behaviors
    UpdateHerdBehaviors();
    
    UE_LOG(LogCrowdSimulation, VeryVerbose, TEXT("Ecosystem update - Agents: %d, Load: %.2f"), 
           ActiveAgentCount, CurrentPerformanceLoad);
}

void UCrowdSimulationSubsystem::ManagePerformance()
{
    // Calculate current performance load
    CurrentPerformanceLoad = static_cast<float>(ActiveAgentCount) / static_cast<float>(MaxSimultaneousAgents);
    
    // If performance threshold exceeded, implement LOD strategies
    if (CurrentPerformanceLoad > PerformanceThreshold)
    {
        UE_LOG(LogCrowdSimulation, Warning, TEXT("Performance threshold exceeded (%.2f > %.2f) - implementing LOD"), 
               CurrentPerformanceLoad, PerformanceThreshold);
        
        // TODO: Implement LOD strategies:
        // - Reduce update frequency for distant herds
        // - Simplify behavior trees for background agents
        // - Cull agents outside player view frustum
        // - Merge small herds into larger ones
    }
}

void UCrowdSimulationSubsystem::UpdateHerdBehaviors()
{
    // TODO: Implement herd behavior updates:
    // - Migration patterns based on time of day
    // - Feeding behaviors in appropriate areas
    // - Predator-prey interactions
    // - Territorial behaviors
    // - Seasonal movement patterns
}