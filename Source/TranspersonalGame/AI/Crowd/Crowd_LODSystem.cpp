#include "Crowd_LODSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"

UCrowd_LODSystem::UCrowd_LODSystem()
{
    // Initialize LOD counts array
    LODCounts.SetNum(4);
    for (int32 i = 0; i < 4; i++)
    {
        LODCounts[i] = 0;
    }
}

void UCrowd_LODSystem::Initialize()
{
    // Clear existing data
    EntityLODMap.Empty();
    for (int32 i = 0; i < LODCounts.Num(); i++)
    {
        LODCounts[i] = 0;
    }
    
    LastUpdateTime = GetWorldTime();
    
    UE_LOG(LogTemp, Log, TEXT("Crowd LOD System initialized"));
}

void UCrowd_LODSystem::UpdateLOD(const FVector& ViewerLocation)
{
    float CurrentTime = GetWorldTime();
    
    // Only update LOD at specified intervals for performance
    if (CurrentTime - LastUpdateTime < LODUpdateInterval)
    {
        return;
    }
    
    LastUpdateTime = CurrentTime;
    
    // Reset LOD counts
    for (int32 i = 0; i < LODCounts.Num(); i++)
    {
        LODCounts[i] = 0;
    }
    
    // Track entities that need LOD adjustment
    TArray<FMassEntityHandle> EntitiesToAdjust;
    TArray<ECrowd_LODLevel> NewLODLevels;
    
    // Update LOD for all tracked entities
    for (auto& EntityLODPair : EntityLODMap)
    {
        FMassEntityHandle Entity = EntityLODPair.Key;
        ECrowd_LODLevel CurrentLOD = EntityLODPair.Value;
        
        // Calculate distance to viewer (simplified - would need actual entity position)
        float Distance = FMath::RandRange(100.0f, 6000.0f); // Placeholder distance
        
        ECrowd_LODLevel NewLOD = CalculateLODFromDistance(Distance);
        
        // Apply LOD limits based on performance settings
        if (NewLOD == ECrowd_LODLevel::High && LODCounts[0] >= MaxHighDetailEntities)
        {
            NewLOD = ECrowd_LODLevel::Medium;
        }
        if (NewLOD == ECrowd_LODLevel::Medium && LODCounts[1] >= MaxMediumDetailEntities)
        {
            NewLOD = ECrowd_LODLevel::Low;
        }
        if (NewLOD == ECrowd_LODLevel::Low && LODCounts[2] >= MaxLowDetailEntities)
        {
            NewLOD = ECrowd_LODLevel::Culled;
        }
        
        // Update LOD if changed
        if (NewLOD != CurrentLOD)
        {
            EntitiesToAdjust.Add(Entity);
            NewLODLevels.Add(NewLOD);
        }
        
        // Update counts
        int32 LODIndex = static_cast<int32>(NewLOD);
        if (LODIndex >= 0 && LODIndex < LODCounts.Num())
        {
            LODCounts[LODIndex]++;
        }
    }
    
    // Apply LOD changes
    for (int32 i = 0; i < EntitiesToAdjust.Num(); i++)
    {
        SetEntityLOD(EntitiesToAdjust[i], NewLODLevels[i]);
    }
}

ECrowd_LODLevel UCrowd_LODSystem::GetEntityLOD(FMassEntityHandle Entity) const
{
    if (const ECrowd_LODLevel* LOD = EntityLODMap.Find(Entity))
    {
        return *LOD;
    }
    return ECrowd_LODLevel::High; // Default to high detail
}

void UCrowd_LODSystem::SetEntityLOD(FMassEntityHandle Entity, ECrowd_LODLevel NewLOD)
{
    ECrowd_LODLevel OldLOD = GetEntityLOD(Entity);
    
    if (OldLOD != NewLOD)
    {
        EntityLODMap.Add(Entity, NewLOD);
        ApplyLODToEntity(Entity, NewLOD);
        
        // Log LOD changes for debugging
        UE_LOG(LogTemp, VeryVerbose, TEXT("Entity LOD changed from %d to %d"), 
               static_cast<int32>(OldLOD), static_cast<int32>(NewLOD));
    }
}

void UCrowd_LODSystem::GetLODCounts(int32& LOD0Count, int32& LOD1Count, int32& LOD2Count, int32& LOD3Count) const
{
    LOD0Count = LODCounts.IsValidIndex(0) ? LODCounts[0] : 0;
    LOD1Count = LODCounts.IsValidIndex(1) ? LODCounts[1] : 0;
    LOD2Count = LODCounts.IsValidIndex(2) ? LODCounts[2] : 0;
    LOD3Count = LODCounts.IsValidIndex(3) ? LODCounts[3] : 0;
}

ECrowd_LODLevel UCrowd_LODSystem::CalculateLODFromDistance(float Distance) const
{
    if (Distance <= LOD0Distance)
    {
        return ECrowd_LODLevel::High;
    }
    else if (Distance <= LOD1Distance)
    {
        return ECrowd_LODLevel::Medium;
    }
    else if (Distance <= LOD2Distance)
    {
        return ECrowd_LODLevel::Low;
    }
    else
    {
        return ECrowd_LODLevel::Culled;
    }
}

void UCrowd_LODSystem::ApplyLODToEntity(FMassEntityHandle Entity, ECrowd_LODLevel LODLevel)
{
    // Update entity visibility
    UpdateEntityVisibility(Entity, LODLevel);
    
    // Update entity update frequency
    UpdateEntityUpdateFrequency(Entity, LODLevel);
}

void UCrowd_LODSystem::UpdateEntityVisibility(FMassEntityHandle Entity, ECrowd_LODLevel LODLevel)
{
    // Culled entities should be invisible
    bool bShouldBeVisible = (LODLevel != ECrowd_LODLevel::Culled);
    
    // In a full implementation, this would update the entity's rendering visibility
    // For now, we just log the change
    UE_LOG(LogTemp, VeryVerbose, TEXT("Entity visibility set to %s for LOD %d"), 
           bShouldBeVisible ? TEXT("true") : TEXT("false"), static_cast<int32>(LODLevel));
}

void UCrowd_LODSystem::UpdateEntityUpdateFrequency(FMassEntityHandle Entity, ECrowd_LODLevel LODLevel)
{
    float UpdateFrequency = LOD0UpdateFrequency;
    
    switch (LODLevel)
    {
        case ECrowd_LODLevel::High:
            UpdateFrequency = LOD0UpdateFrequency;
            break;
        case ECrowd_LODLevel::Medium:
            UpdateFrequency = LOD1UpdateFrequency;
            break;
        case ECrowd_LODLevel::Low:
            UpdateFrequency = LOD2UpdateFrequency;
            break;
        case ECrowd_LODLevel::Culled:
            UpdateFrequency = LOD3UpdateFrequency;
            break;
    }
    
    // In a full implementation, this would adjust the entity's processing frequency
    // For now, we just log the change
    UE_LOG(LogTemp, VeryVerbose, TEXT("Entity update frequency set to %f for LOD %d"), 
           UpdateFrequency, static_cast<int32>(LODLevel));
}

float UCrowd_LODSystem::GetWorldTime() const
{
    if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
    {
        return World->GetTimeSeconds();
    }
    return 0.0f;
}