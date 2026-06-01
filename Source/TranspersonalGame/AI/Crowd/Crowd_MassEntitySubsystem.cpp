#include "Crowd_MassEntitySubsystem.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UCrowd_MassEntitySubsystem::UCrowd_MassEntitySubsystem()
{
    bIsSimulationActive = false;
    SimulationTime = 0.0f;
    MassEntitySubsystem = nullptr;
}

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::Initialize - Starting crowd simulation subsystem"));
    
    // Get Mass Entity subsystem
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::Initialize - Mass Entity subsystem found"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UCrowd_MassEntitySubsystem::Initialize - Mass Entity subsystem not found"));
        }
    }
    
    // Initialize biome entity counts
    BiomeEntityCounts.Empty();
    BiomeEntityCounts.Add(ECrowd_BiomeType::Savanna, 0);
    BiomeEntityCounts.Add(ECrowd_BiomeType::Forest, 0);
    BiomeEntityCounts.Add(ECrowd_BiomeType::Desert, 0);
    BiomeEntityCounts.Add(ECrowd_BiomeType::Swamp, 0);
    BiomeEntityCounts.Add(ECrowd_BiomeType::Mountain, 0);
    
    ActiveEntities.Empty();
    bIsSimulationActive = false;
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::Deinitialize - Shutting down crowd simulation"));
    
    ClearAllEntities();
    MassEntitySubsystem = nullptr;
    
    Super::Deinitialize();
}

void UCrowd_MassEntitySubsystem::Tick(float DeltaTime)
{
    if (!bIsSimulationActive || !MassEntitySubsystem)
    {
        return;
    }
    
    SimulationTime += DeltaTime;
    
    // Update entity movement every frame
    UpdateEntityMovement(DeltaTime);
    
    // Process biome distribution every 5 seconds
    if (FMath::Fmod(SimulationTime, 5.0f) < DeltaTime)
    {
        ProcessBiomeDistribution();
    }
    
    // Cleanup invalid entities every 10 seconds
    if (FMath::Fmod(SimulationTime, 10.0f) < DeltaTime)
    {
        CleanupInvalidEntities();
    }
}

void UCrowd_MassEntitySubsystem::InitializeCrowdSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::InitializeCrowdSimulation - Starting crowd simulation"));
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("UCrowd_MassEntitySubsystem::InitializeCrowdSimulation - Mass Entity subsystem not available"));
        return;
    }
    
    // Clear existing entities
    ClearAllEntities();
    
    // Spawn initial entities in each biome
    for (ECrowd_BiomeType BiomeType : SpawnConfig.AllowedBiomes)
    {
        int32 InitialCount = SpawnConfig.MaxEntitiesPerBiome / 4; // Start with 25% capacity
        SpawnEntitiesInBiome(BiomeType, InitialCount);
    }
    
    bIsSimulationActive = true;
    SimulationTime = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::InitializeCrowdSimulation - Crowd simulation initialized with %d total entities"), ActiveEntities.Num());
}

void UCrowd_MassEntitySubsystem::SpawnEntitiesInBiome(ECrowd_BiomeType BiomeType, int32 Count)
{
    if (!MassEntitySubsystem || Count <= 0)
    {
        return;
    }
    
    int32 CurrentCount = GetEntityCountInBiome(BiomeType);
    int32 MaxAllowed = SpawnConfig.MaxEntitiesPerBiome;
    int32 ActualSpawnCount = FMath::Min(Count, MaxAllowed - CurrentCount);
    
    if (ActualSpawnCount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::SpawnEntitiesInBiome - Biome %d at capacity (%d/%d)"), 
               (int32)BiomeType, CurrentCount, MaxAllowed);
        return;
    }
    
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    int32 SpawnedCount = 0;
    
    for (int32 i = 0; i < ActualSpawnCount * 2 && SpawnedCount < ActualSpawnCount; ++i) // Try twice as many attempts
    {
        // Generate random position within biome
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(1000.0f, SpawnConfig.SpawnRadius);
        
        FVector SpawnLocation = BiomeCenter + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            100.0f // Height above ground
        );
        
        if (IsValidSpawnLocation(SpawnLocation, BiomeType))
        {
            // Create entity using Mass Entity framework
            FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
            if (EntityHandle.IsValid())
            {
                ActiveEntities.Add(EntityHandle);
                BiomeEntityCounts[BiomeType]++;
                SpawnedCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::SpawnEntitiesInBiome - Spawned %d entities in biome %d"), 
           SpawnedCount, (int32)BiomeType);
}

void UCrowd_MassEntitySubsystem::UpdateEntityBehavior(FMassEntityHandle EntityHandle, ECrowd_NPCBehaviorState NewState)
{
    if (!EntityHandle.IsValid() || !MassEntitySubsystem)
    {
        return;
    }
    
    // Update entity behavior state
    // This would typically involve updating Mass Entity components
    UE_LOG(LogTemp, Log, TEXT("UCrowd_MassEntitySubsystem::UpdateEntityBehavior - Entity behavior updated to state %d"), (int32)NewState);
}

int32 UCrowd_MassEntitySubsystem::GetEntityCountInBiome(ECrowd_BiomeType BiomeType) const
{
    if (const int32* Count = BiomeEntityCounts.Find(BiomeType))
    {
        return *Count;
    }
    return 0;
}

TArray<FCrowd_EntityData> UCrowd_MassEntitySubsystem::GetEntitiesInRadius(FVector Center, float Radius) const
{
    TArray<FCrowd_EntityData> Result;
    
    // This would query Mass Entity system for entities within radius
    // For now, return empty array as placeholder
    
    return Result;
}

void UCrowd_MassEntitySubsystem::SetCrowdDensity(ECrowd_BiomeType BiomeType, float DensityMultiplier)
{
    int32 NewMaxEntities = FMath::RoundToInt(SpawnConfig.MaxEntitiesPerBiome * FMath::Clamp(DensityMultiplier, 0.1f, 2.0f));
    int32 CurrentCount = GetEntityCountInBiome(BiomeType);
    
    if (NewMaxEntities > CurrentCount)
    {
        // Spawn more entities
        SpawnEntitiesInBiome(BiomeType, NewMaxEntities - CurrentCount);
    }
    else if (NewMaxEntities < CurrentCount)
    {
        // Remove excess entities
        // This would involve destroying Mass Entity handles
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::SetCrowdDensity - Need to remove %d entities from biome %d"), 
               CurrentCount - NewMaxEntities, (int32)BiomeType);
    }
}

void UCrowd_MassEntitySubsystem::PauseSimulation(bool bPause)
{
    bIsSimulationActive = !bPause;
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::PauseSimulation - Simulation %s"), 
           bIsSimulationActive ? TEXT("resumed") : TEXT("paused"));
}

void UCrowd_MassEntitySubsystem::ClearAllEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Destroy all active entities
    for (FMassEntityHandle EntityHandle : ActiveEntities)
    {
        if (EntityHandle.IsValid())
        {
            MassEntitySubsystem->DestroyEntity(EntityHandle);
        }
    }
    
    ActiveEntities.Empty();
    
    // Reset biome counts
    for (auto& Pair : BiomeEntityCounts)
    {
        Pair.Value = 0;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::ClearAllEntities - All crowd entities cleared"));
}

void UCrowd_MassEntitySubsystem::UpdateEntityMovement(float DeltaTime)
{
    // Update movement for all active entities
    // This would involve updating Mass Entity transform components
    
    // Placeholder implementation
    for (FMassEntityHandle EntityHandle : ActiveEntities)
    {
        if (EntityHandle.IsValid())
        {
            // Update entity position, velocity, etc.
        }
    }
}

void UCrowd_MassEntitySubsystem::ProcessBiomeDistribution()
{
    // Ensure entities are properly distributed across biomes
    int32 TotalEntities = ActiveEntities.Num();
    int32 TargetPerBiome = TotalEntities / SpawnConfig.AllowedBiomes.Num();
    
    for (ECrowd_BiomeType BiomeType : SpawnConfig.AllowedBiomes)
    {
        int32 CurrentCount = GetEntityCountInBiome(BiomeType);
        int32 Difference = TargetPerBiome - CurrentCount;
        
        if (FMath::Abs(Difference) > 10) // Only rebalance if significant difference
        {
            if (Difference > 0)
            {
                SpawnEntitiesInBiome(BiomeType, FMath::Min(Difference, 20)); // Spawn in small batches
            }
            // Note: Entity removal would be implemented here for negative difference
        }
    }
}

FVector UCrowd_MassEntitySubsystem::GetBiomeCenter(ECrowd_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case ECrowd_BiomeType::Savanna:
            return FVector(0.0f, 0.0f, 100.0f);
        case ECrowd_BiomeType::Swamp:
            return FVector(-50000.0f, -45000.0f, 100.0f);
        case ECrowd_BiomeType::Forest:
            return FVector(-45000.0f, 40000.0f, 100.0f);
        case ECrowd_BiomeType::Desert:
            return FVector(55000.0f, 0.0f, 100.0f);
        case ECrowd_BiomeType::Mountain:
            return FVector(40000.0f, 50000.0f, 100.0f);
        default:
            return FVector::ZeroVector;
    }
}

bool UCrowd_MassEntitySubsystem::IsValidSpawnLocation(FVector Location, ECrowd_BiomeType BiomeType) const
{
    // Check if location is within biome bounds
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    float DistanceFromCenter = FVector::Dist2D(Location, BiomeCenter);
    
    if (DistanceFromCenter > SpawnConfig.SpawnRadius)
    {
        return false;
    }
    
    // Check minimum distance from other entities
    // This would involve querying nearby entities
    
    return true;
}

void UCrowd_MassEntitySubsystem::CleanupInvalidEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    int32 RemovedCount = 0;
    
    // Remove invalid entity handles
    for (int32 i = ActiveEntities.Num() - 1; i >= 0; --i)
    {
        if (!ActiveEntities[i].IsValid())
        {
            ActiveEntities.RemoveAt(i);
            RemovedCount++;
        }
    }
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySubsystem::CleanupInvalidEntities - Removed %d invalid entities"), RemovedCount);
        
        // Recalculate biome counts
        for (auto& Pair : BiomeEntityCounts)
        {
            Pair.Value = 0;
        }
        
        // This would involve re-counting entities per biome
    }
}