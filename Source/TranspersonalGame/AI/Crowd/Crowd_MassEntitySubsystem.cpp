#include "Crowd_MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd Mass Entity Subsystem Initialized"));
    
    // Initialize Mass Entity system
    InitializeMassEntity();
    
    // Setup tick timer for simulation updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UCrowd_MassEntitySubsystem::UpdateSimulationStats,
            1.0f / TickRate,
            true
        );
    }
    
    // Initialize stats
    CurrentStats = FCrowd_SimulationStats();
    CurrentStats.MaxEntities = MaxEntityCount;
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    DespawnAllEntities();
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }
    
    Super::Deinitialize();
}

bool UCrowd_MassEntitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UCrowd_MassEntitySubsystem::SpawnCrowdEntities(const TArray<FCrowd_EntitySpawnData>& SpawnData)
{
    if (ActiveEntities.Num() + SpawnData.Num() > MaxEntityCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn %d entities - would exceed max count of %d"), 
               SpawnData.Num(), MaxEntityCount);
        return;
    }
    
    for (const FCrowd_EntitySpawnData& EntityData : SpawnData)
    {
        // Add to active entities list
        ActiveEntities.Add(EntityData);
        
        // TODO: Integrate with actual Mass Entity spawning system
        UE_LOG(LogTemp, Log, TEXT("Spawned crowd entity at %s with behavior %d"), 
               *EntityData.Location.ToString(), (int32)EntityData.BehaviorType);
    }
    
    CurrentStats.ActiveEntities = ActiveEntities.Num();
    UE_LOG(LogTemp, Log, TEXT("Total active entities: %d"), CurrentStats.ActiveEntities);
}

void UCrowd_MassEntitySubsystem::DespawnAllEntities()
{
    ActiveEntities.Empty();
    CurrentStats.ActiveEntities = 0;
    
    UE_LOG(LogTemp, Log, TEXT("All crowd entities despawned"));
}

void UCrowd_MassEntitySubsystem::SetMaxEntityCount(int32 NewMaxCount)
{
    MaxEntityCount = FMath::Max(1, NewMaxCount);
    CurrentStats.MaxEntities = MaxEntityCount;
    
    // If we're over the new limit, remove excess entities
    if (ActiveEntities.Num() > MaxEntityCount)
    {
        int32 EntitiesToRemove = ActiveEntities.Num() - MaxEntityCount;
        ActiveEntities.RemoveAt(MaxEntityCount, EntitiesToRemove);
        CurrentStats.ActiveEntities = ActiveEntities.Num();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Max entity count set to %d"), MaxEntityCount);
}

FCrowd_SimulationStats UCrowd_MassEntitySubsystem::GetSimulationStats() const
{
    return CurrentStats;
}

void UCrowd_MassEntitySubsystem::SetGlobalBehaviorState(ECrowd_BehaviorType NewBehavior)
{
    for (FCrowd_EntitySpawnData& Entity : ActiveEntities)
    {
        Entity.BehaviorType = NewBehavior;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Global behavior state changed to %d"), (int32)NewBehavior);
}

void UCrowd_MassEntitySubsystem::AddBehaviorZone(const FVector& Center, float Radius, ECrowd_BehaviorType ZoneBehavior)
{
    BehaviorZones.Add(Center, ZoneBehavior);
    
    UE_LOG(LogTemp, Log, TEXT("Added behavior zone at %s with radius %.1f and behavior %d"), 
           *Center.ToString(), Radius, (int32)ZoneBehavior);
}

void UCrowd_MassEntitySubsystem::RemoveBehaviorZone(const FVector& Center)
{
    if (BehaviorZones.Remove(Center) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Removed behavior zone at %s"), *Center.ToString());
    }
}

void UCrowd_MassEntitySubsystem::SetLODDistances(float HighLOD, float MediumLOD, float LowLOD)
{
    HighLODDistance = FMath::Max(0.0f, HighLOD);
    MediumLODDistance = FMath::Max(HighLODDistance, MediumLOD);
    LowLODDistance = FMath::Max(MediumLODDistance, LowLOD);
    
    UE_LOG(LogTemp, Log, TEXT("LOD distances set - High: %.1f, Medium: %.1f, Low: %.1f"), 
           HighLODDistance, MediumLODDistance, LowLODDistance);
}

void UCrowd_MassEntitySubsystem::EnableDynamicLOD(bool bEnable)
{
    bEnableLODSystem = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Dynamic LOD system %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCrowd_MassEntitySubsystem::DebugSpawnTestCrowd()
{
    TArray<FCrowd_EntitySpawnData> TestSpawnData;
    
    // Create a grid of test entities
    for (int32 X = -5; X <= 5; X++)
    {
        for (int32 Y = -5; Y <= 5; Y++)
        {
            FCrowd_EntitySpawnData SpawnData;
            SpawnData.Location = FVector(X * 200.0f, Y * 200.0f, 100.0f);
            SpawnData.BehaviorType = (X + Y) % 2 == 0 ? ECrowd_BehaviorType::Wandering : ECrowd_BehaviorType::Following;
            SpawnData.MovementSpeed = FMath::RandRange(100.0f, 200.0f);
            SpawnData.GroupID = FMath::Abs(X + Y) % 4;
            
            TestSpawnData.Add(SpawnData);
        }
    }
    
    SpawnCrowdEntities(TestSpawnData);
    UE_LOG(LogTemp, Log, TEXT("Spawned test crowd with %d entities"), TestSpawnData.Num());
}

void UCrowd_MassEntitySubsystem::DebugClearAllEntities()
{
    DespawnAllEntities();
    UE_LOG(LogTemp, Log, TEXT("Debug: All entities cleared"));
}

void UCrowd_MassEntitySubsystem::DebugPrintStats()
{
    UE_LOG(LogTemp, Log, TEXT("=== CROWD SIMULATION STATS ==="));
    UE_LOG(LogTemp, Log, TEXT("Active Entities: %d / %d"), CurrentStats.ActiveEntities, CurrentStats.MaxEntities);
    UE_LOG(LogTemp, Log, TEXT("Average Frame Time: %.3f ms"), CurrentStats.AverageFrameTime);
    UE_LOG(LogTemp, Log, TEXT("High Density Zones: %d entities"), CurrentStats.EntitiesInHighDensityZones);
    UE_LOG(LogTemp, Log, TEXT("Low Density Zones: %d entities"), CurrentStats.EntitiesInLowDensityZones);
    UE_LOG(LogTemp, Log, TEXT("Behavior Zones: %d"), BehaviorZones.Num());
    UE_LOG(LogTemp, Log, TEXT("LOD System: %s"), bEnableLODSystem ? TEXT("Enabled") : TEXT("Disabled"));
}

void UCrowd_MassEntitySubsystem::InitializeMassEntity()
{
    // Initialize Mass Entity processor
    MassProcessor = nullptr; // TODO: Create actual Mass Entity processor
    BehaviorTree = nullptr;  // TODO: Create behavior tree processor
    
    UE_LOG(LogTemp, Log, TEXT("Mass Entity system initialized"));
}

void UCrowd_MassEntitySubsystem::UpdateSimulationStats()
{
    // Update frame time
    static float LastFrameTime = 0.0f;
    float CurrentTime = FPlatformTime::Seconds();
    CurrentStats.AverageFrameTime = (CurrentTime - LastFrameTime) * 1000.0f; // Convert to ms
    LastFrameTime = CurrentTime;
    
    // Process LOD system if enabled
    if (bEnableLODSystem)
    {
        ProcessLODSystem();
    }
    
    // Handle behavior zones
    HandleBehaviorZones();
}

void UCrowd_MassEntitySubsystem::ProcessLODSystem()
{
    // TODO: Implement actual LOD processing based on distance to player
    // For now, just simulate the counts
    int32 TotalEntities = ActiveEntities.Num();
    CurrentStats.EntitiesInHighDensityZones = FMath::RandRange(0, TotalEntities / 3);
    CurrentStats.EntitiesInLowDensityZones = TotalEntities - CurrentStats.EntitiesInHighDensityZones;
}

void UCrowd_MassEntitySubsystem::HandleBehaviorZones()
{
    // TODO: Process entities within behavior zones and update their behavior
    // This would involve checking entity positions against zone centers and radii
    for (const auto& ZonePair : BehaviorZones)
    {
        const FVector& ZoneCenter = ZonePair.Key;
        const ECrowd_BehaviorType& ZoneBehavior = ZonePair.Value;
        
        // Process entities in this zone
        // Implementation would go here
    }
}