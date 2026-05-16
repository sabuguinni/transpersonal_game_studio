#include "Crowd_MassSimulationManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Crowd_FlockingComponent.h"

UCrowd_MassSimulationManager::UCrowd_MassSimulationManager()
{
    MaxSimultaneousEntities = 50000;
    LODNearDistance = 1000.0f;
    LODFarDistance = 5000.0f;
    PanicSpeedMultiplier = 2.5f;
    PanicFleeDistance = 3000.0f;
    bSimulationActive = false;
    CurrentActiveEntities = 0;
    CurrentGlobalBehavior = ECrowd_BehaviorState::Wandering;
}

void UCrowd_MassSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Initializing crowd simulation subsystem"));
    
    // Get Mass Entity subsystem reference
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (!MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Crowd_MassSimulationManager: Failed to get MassEntitySubsystem"));
            return;
        }
    }
    
    SetupDefaultArchetypes();
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Initialization complete"));
}

void UCrowd_MassSimulationManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Shutting down crowd simulation"));
    
    ShutdownMassSimulation();
    CleanupMassEntities();
    
    Super::Deinitialize();
}

bool UCrowd_MassSimulationManager::InitializeMassSimulation()
{
    if (bSimulationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Simulation already active"));
        return true;
    }
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassSimulationManager: MassEntitySubsystem not available"));
        return false;
    }
    
    // Initialize Mass Entity framework
    bSimulationActive = true;
    CurrentActiveEntities = 0;
    CurrentGlobalBehavior = ECrowd_BehaviorState::Wandering;
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Mass simulation initialized successfully"));
    return true;
}

void UCrowd_MassSimulationManager::ShutdownMassSimulation()
{
    if (!bSimulationActive)
    {
        return;
    }
    
    // Clear panic timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PanicTimerHandle);
    }
    
    DespawnAllCrowds();
    bSimulationActive = false;
    CurrentActiveEntities = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Mass simulation shutdown complete"));
}

bool UCrowd_MassSimulationManager::SpawnCrowdAtLocation(const FCrowd_SpawnConfig& SpawnConfig)
{
    if (!bSimulationActive || !MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassSimulationManager: Cannot spawn crowd - simulation not active"));
        return false;
    }
    
    if (CurrentActiveEntities + SpawnConfig.EntityCount > MaxSimultaneousEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Cannot spawn %d entities - would exceed limit of %d"), 
               SpawnConfig.EntityCount, MaxSimultaneousEntities);
        return false;
    }
    
    // Create Mass entities in circular formation
    int32 EntitiesSpawned = 0;
    const float AngleStep = 2.0f * PI / FMath::Max(1, SpawnConfig.EntityCount);
    
    for (int32 i = 0; i < SpawnConfig.EntityCount; ++i)
    {
        // Calculate spawn position in circle
        float Angle = i * AngleStep;
        float RandomRadius = FMath::RandRange(100.0f, SpawnConfig.SpawnRadius);
        FVector SpawnOffset = FVector(
            FMath::Cos(Angle) * RandomRadius,
            FMath::Sin(Angle) * RandomRadius,
            0.0f
        );
        FVector FinalSpawnLocation = SpawnConfig.SpawnLocation + SpawnOffset;
        
        // Create Mass entity (simplified for now - would use proper Mass Entity archetype in full implementation)
        FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
        if (EntityHandle.IsValid())
        {
            SpawnedEntities.Add(EntityHandle);
            EntityConfigurations.Add(EntityHandle, SpawnConfig);
            EntitiesSpawned++;
        }
    }
    
    CurrentActiveEntities += EntitiesSpawned;
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Spawned %d entities at location %s"), 
           EntitiesSpawned, *SpawnConfig.SpawnLocation.ToString());
    
    return EntitiesSpawned > 0;
}

void UCrowd_MassSimulationManager::DespawnAllCrowds()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Destroy all spawned entities
    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (EntityHandle.IsValid())
        {
            MassEntitySubsystem->DestroyEntity(EntityHandle);
        }
    }
    
    SpawnedEntities.Empty();
    EntityConfigurations.Empty();
    CurrentActiveEntities = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: All crowds despawned"));
}

void UCrowd_MassSimulationManager::TriggerPanicResponse(FVector ThreatLocation, float ThreatRadius, float PanicDuration)
{
    if (!bSimulationActive)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Triggering panic response at %s, radius %f"), 
           *ThreatLocation.ToString(), ThreatRadius);
    
    // Set global behavior to fleeing
    SetGlobalCrowdBehavior(ECrowd_BehaviorState::Fleeing);
    
    // Broadcast panic event
    OnPanicTriggered.Broadcast(ThreatLocation, ThreatRadius);
    
    // Set timer to return to normal behavior
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(PanicTimerHandle, this, 
                                         &UCrowd_MassSimulationManager::OnPanicTimerExpired, 
                                         PanicDuration, false);
    }
}

void UCrowd_MassSimulationManager::SetGlobalCrowdBehavior(ECrowd_BehaviorState NewBehavior)
{
    if (CurrentGlobalBehavior == NewBehavior)
    {
        return;
    }
    
    ECrowd_BehaviorState PreviousBehavior = CurrentGlobalBehavior;
    CurrentGlobalBehavior = NewBehavior;
    
    // Apply behavior change to all active entities
    // In full implementation, this would update Mass Entity components
    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (EntityHandle.IsValid())
        {
            // Update entity behavior state via Mass Entity component system
            // This is a simplified version - full implementation would use Mass Entity queries
        }
    }
    
    OnCrowdStateChanged.Broadcast(NewBehavior);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Global behavior changed from %d to %d"), 
           (int32)PreviousBehavior, (int32)NewBehavior);
}

void UCrowd_MassSimulationManager::SetCrowdLODDistance(float NearDistance, float FarDistance)
{
    LODNearDistance = FMath::Max(100.0f, NearDistance);
    LODFarDistance = FMath::Max(LODNearDistance + 100.0f, FarDistance);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: LOD distances set - Near: %f, Far: %f"), 
           LODNearDistance, LODFarDistance);
}

void UCrowd_MassSimulationManager::SetMaxActiveEntities(int32 MaxEntities)
{
    MaxSimultaneousEntities = FMath::Max(100, MaxEntities);
    
    // If we're over the new limit, despawn excess entities
    if (CurrentActiveEntities > MaxSimultaneousEntities)
    {
        int32 EntitiesToRemove = CurrentActiveEntities - MaxSimultaneousEntities;
        
        for (int32 i = SpawnedEntities.Num() - 1; i >= 0 && EntitiesToRemove > 0; --i)
        {
            if (SpawnedEntities[i].IsValid() && MassEntitySubsystem)
            {
                MassEntitySubsystem->DestroyEntity(SpawnedEntities[i]);
                EntityConfigurations.Remove(SpawnedEntities[i]);
                SpawnedEntities.RemoveAt(i);
                CurrentActiveEntities--;
                EntitiesToRemove--;
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Removed %d entities to meet new limit"), 
               CurrentActiveEntities - MaxSimultaneousEntities + EntitiesToRemove);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Max entities set to %d"), MaxSimultaneousEntities);
}

int32 UCrowd_MassSimulationManager::GetActiveEntityCount() const
{
    return CurrentActiveEntities;
}

void UCrowd_MassSimulationManager::SetupDefaultArchetypes()
{
    // Create default crowd archetypes for different scenarios
    CrowdArchetypes.Empty();
    
    // Wandering crowd archetype
    FCrowd_MassArchetype WanderingArchetype;
    WanderingArchetype.MaxEntities = 1000;
    WanderingArchetype.SpawnRadius = 2000.0f;
    WanderingArchetype.DefaultBehaviorState = ECrowd_BehaviorState::Wandering;
    CrowdArchetypes.Add(WanderingArchetype);
    
    // Gathering crowd archetype
    FCrowd_MassArchetype GatheringArchetype;
    GatheringArchetype.MaxEntities = 500;
    GatheringArchetype.SpawnRadius = 1000.0f;
    GatheringArchetype.DefaultBehaviorState = ECrowd_BehaviorState::Gathering;
    CrowdArchetypes.Add(GatheringArchetype);
    
    // Fleeing crowd archetype
    FCrowd_MassArchetype FleeingArchetype;
    FleeingArchetype.MaxEntities = 2000;
    FleeingArchetype.SpawnRadius = 3000.0f;
    FleeingArchetype.DefaultBehaviorState = ECrowd_BehaviorState::Fleeing;
    CrowdArchetypes.Add(FleeingArchetype);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Setup %d default archetypes"), CrowdArchetypes.Num());
}

void UCrowd_MassSimulationManager::CleanupMassEntities()
{
    DespawnAllCrowds();
    CrowdArchetypes.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Mass entities cleanup complete"));
}

void UCrowd_MassSimulationManager::OnPanicTimerExpired()
{
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Panic timer expired, returning to wandering behavior"));
    SetGlobalCrowdBehavior(ECrowd_BehaviorState::Wandering);
}