#include "Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UCrowd_MassEntityManager::UCrowd_MassEntityManager()
{
    ActiveEntityCount = 0;
    CurrentDensityMultiplier = 1.0f;
    bIsCrowdSystemActive = false;
}

void UCrowd_MassEntityManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Manager Initialized"));
    
    // Initialize default configurations
    SpawnConfig = FCrowd_SpawnConfiguration();
    LODConfig = FCrowd_LODConfiguration();
    
    // Reserve space for entity arrays
    EntityLocations.Reserve(SpawnConfig.MaxEntities);
    EntityLODLevels.Reserve(SpawnConfig.MaxEntities);
    EntityActiveStates.Reserve(SpawnConfig.MaxEntities);
    
    InitializeCrowdSystem();
}

void UCrowd_MassEntityManager::Deinitialize()
{
    ClearAllCrowdEntities();
    bIsCrowdSystemActive = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Manager Deinitialized"));
    
    Super::Deinitialize();
}

void UCrowd_MassEntityManager::InitializeCrowdSystem()
{
    if (bIsCrowdSystemActive)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Initializing Crowd System - Max Entities: %d"), SpawnConfig.MaxEntities);
    
    // Clear existing data
    EntityLocations.Empty();
    EntityLODLevels.Empty();
    EntityActiveStates.Empty();
    
    ActiveEntityCount = 0;
    bIsCrowdSystemActive = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd System Initialized Successfully"));
}

void UCrowd_MassEntityManager::SpawnCrowdEntities(const FVector& CenterLocation, int32 EntityCount)
{
    if (!bIsCrowdSystemActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd system not active - cannot spawn entities"));
        return;
    }
    
    // Clamp entity count to available slots
    int32 AvailableSlots = SpawnConfig.MaxEntities - ActiveEntityCount;
    int32 EntitiesToSpawn = FMath::Min(EntityCount, AvailableSlots);
    
    UE_LOG(LogTemp, Warning, TEXT("Spawning %d crowd entities at location: %s"), EntitiesToSpawn, *CenterLocation.ToString());
    
    for (int32 i = 0; i < EntitiesToSpawn; i++)
    {
        // Generate random spawn location within radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(SpawnConfig.MinSpawnDistance, SpawnConfig.SpawnRadius);
        
        FVector SpawnLocation = CenterLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        SpawnEntityAtLocation(SpawnLocation);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d entities - Total active: %d"), EntitiesToSpawn, ActiveEntityCount);
}

void UCrowd_MassEntityManager::UpdateCrowdLOD(const FVector& ViewerLocation)
{
    if (!bIsCrowdSystemActive || EntityLocations.Num() == 0)
    {
        return;
    }
    
    int32 UpdatedEntities = 0;
    
    for (int32 i = 0; i < EntityLocations.Num(); i++)
    {
        if (!EntityActiveStates[i])
        {
            continue;
        }
        
        float Distance = FVector::Dist(ViewerLocation, EntityLocations[i]);
        UpdateEntityLOD(i, Distance);
        UpdatedEntities++;
    }
    
    // Cull distant entities
    CullDistantEntities(ViewerLocation);
    
    // Log LOD update (throttled)
    static float LastLogTime = 0.0f;
    float CurrentTime = FPlatformTime::Seconds();
    if (CurrentTime - LastLogTime > 5.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Updated LOD for %d crowd entities"), UpdatedEntities);
        LastLogTime = CurrentTime;
    }
}

void UCrowd_MassEntityManager::SetCrowdDensity(float NewDensity)
{
    CurrentDensityMultiplier = FMath::Clamp(NewDensity, 0.1f, 2.0f);
    UE_LOG(LogTemp, Warning, TEXT("Crowd density set to: %f"), CurrentDensityMultiplier);
}

int32 UCrowd_MassEntityManager::GetActiveCrowdCount() const
{
    return ActiveEntityCount;
}

void UCrowd_MassEntityManager::ClearAllCrowdEntities()
{
    UE_LOG(LogTemp, Warning, TEXT("Clearing all crowd entities - Count: %d"), ActiveEntityCount);
    
    EntityLocations.Empty();
    EntityLODLevels.Empty();
    EntityActiveStates.Empty();
    ActiveEntityCount = 0;
}

void UCrowd_MassEntityManager::UpdateEntityLOD(int32 EntityIndex, float DistanceToViewer)
{
    if (EntityIndex < 0 || EntityIndex >= EntityLODLevels.Num())
    {
        return;
    }
    
    int32 NewLODLevel = 3; // Default to culled
    
    if (DistanceToViewer <= LODConfig.HighLODDistance)
    {
        NewLODLevel = 0; // High LOD
    }
    else if (DistanceToViewer <= LODConfig.MediumLODDistance)
    {
        NewLODLevel = 1; // Medium LOD
    }
    else if (DistanceToViewer <= LODConfig.LowLODDistance)
    {
        NewLODLevel = 2; // Low LOD
    }
    
    EntityLODLevels[EntityIndex] = NewLODLevel;
}

void UCrowd_MassEntityManager::CullDistantEntities(const FVector& ViewerLocation)
{
    int32 CulledCount = 0;
    
    for (int32 i = EntityLocations.Num() - 1; i >= 0; i--)
    {
        if (!EntityActiveStates[i])
        {
            continue;
        }
        
        float Distance = FVector::Dist(ViewerLocation, EntityLocations[i]);
        
        if (Distance > LODConfig.CullDistance)
        {
            DespawnEntity(i);
            CulledCount++;
        }
    }
    
    if (CulledCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Culled %d distant crowd entities"), CulledCount);
    }
}

void UCrowd_MassEntityManager::SpawnEntityAtLocation(const FVector& Location)
{
    if (ActiveEntityCount >= SpawnConfig.MaxEntities)
    {
        return;
    }
    
    EntityLocations.Add(Location);
    EntityLODLevels.Add(0); // Start with high LOD
    EntityActiveStates.Add(true);
    
    ActiveEntityCount++;
}

void UCrowd_MassEntityManager::DespawnEntity(int32 EntityIndex)
{
    if (EntityIndex < 0 || EntityIndex >= EntityActiveStates.Num())
    {
        return;
    }
    
    if (EntityActiveStates[EntityIndex])
    {
        EntityActiveStates[EntityIndex] = false;
        ActiveEntityCount--;
    }
}