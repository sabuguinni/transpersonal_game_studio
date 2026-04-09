#include "MotionMatchingDatabase.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMotionMatchingDatabase::UMotionMatchingDatabase()
{
    // Initialize default context weights
    ContextWeights.Add(TEXT("MovementState"), 1.0f);
    ContextWeights.Add(TEXT("ThreatLevel"), 0.8f);
    ContextWeights.Add(TEXT("FearIntensity"), 0.6f);
    ContextWeights.Add(TEXT("StaminaLevel"), 0.4f);
    ContextWeights.Add(TEXT("Environment"), 0.3f);
    ContextWeights.Add(TEXT("TerrainSlope"), 0.2f);

    // Setup default LOD levels
    FMotionMatchingLOD LOD0;
    LOD0.LODLevel = 0;
    LOD0.MaxDistance = 500.0f;
    LOD0.MaxAnimationsInDatabase = 100;
    LOD0.QueryFrequency = 60.0f;
    LOD0.bUseReducedPrecision = false;
    LODLevels.Add(LOD0);

    FMotionMatchingLOD LOD1;
    LOD1.LODLevel = 1;
    LOD1.MaxDistance = 1500.0f;
    LOD1.MaxAnimationsInDatabase = 50;
    LOD1.QueryFrequency = 30.0f;
    LOD1.bUseReducedPrecision = false;
    LODLevels.Add(LOD1);

    FMotionMatchingLOD LOD2;
    LOD2.LODLevel = 2;
    LOD2.MaxDistance = 3000.0f;
    LOD2.MaxAnimationsInDatabase = 25;
    LOD2.QueryFrequency = 15.0f;
    LOD2.bUseReducedPrecision = true;
    LODLevels.Add(LOD2);
}

UPoseSearchDatabase* UMotionMatchingDatabase::GetBestDatabase(const FAnimationContextData& Context, int32 LODLevel) const
{
    if (DatabaseEntries.IsEmpty())
    {
        UE_LOG(LogAnimation, Warning, TEXT("MotionMatchingDatabase: No database entries available"));
        return nullptr;
    }

    float BestScore = -1.0f;
    UPoseSearchDatabase* BestDatabase = nullptr;

    for (const FDatabaseEntry& Entry : DatabaseEntries)
    {
        if (!Entry.bIsActive || !Entry.Database.IsValid())
        {
            continue;
        }

        // Check LOD constraints
        if (LODLevel < LODLevels.Num())
        {
            const FMotionMatchingLOD& LOD = LODLevels[LODLevel];
            // Additional LOD-specific filtering could go here
        }

        // Calculate context match score
        float ContextScore = CalculateContextMatch(Entry.RequiredContext, Context);
        
        if (ContextScore < ContextMatchingThreshold)
        {
            continue;
        }

        // Calculate final score including priority and blend weight
        float FinalScore = ContextScore * Entry.Priority * Entry.BlendWeight;

        if (FinalScore > BestScore)
        {
            BestScore = FinalScore;
            BestDatabase = Entry.Database.LoadSynchronous();
        }
    }

    if (!BestDatabase)
    {
        UE_LOG(LogAnimation, Warning, TEXT("MotionMatchingDatabase: No suitable database found for context"));
    }

    return BestDatabase;
}

TArray<UPoseSearchDatabase*> UMotionMatchingDatabase::GetDatabasesForContext(const FAnimationContextData& Context) const
{
    TArray<UPoseSearchDatabase*> MatchingDatabases;

    for (const FDatabaseEntry& Entry : DatabaseEntries)
    {
        if (!Entry.bIsActive || !Entry.Database.IsValid())
        {
            continue;
        }

        float ContextScore = CalculateContextMatch(Entry.RequiredContext, Context);
        
        if (ContextScore >= ContextMatchingThreshold)
        {
            if (UPoseSearchDatabase* Database = Entry.Database.LoadSynchronous())
            {
                MatchingDatabases.Add(Database);
            }
        }
    }

    return MatchingDatabases;
}

float UMotionMatchingDatabase::CalculateContextMatch(const FAnimationContextData& RequiredContext, const FAnimationContextData& CurrentContext) const
{
    float TotalScore = 0.0f;
    float TotalWeight = 0.0f;

    // Movement State matching
    float StateWeight = ContextWeights.FindRef(TEXT("MovementState"));
    float StateScore = CalculateStateWeight(RequiredContext.MovementState, CurrentContext.MovementState);
    TotalScore += StateScore * StateWeight;
    TotalWeight += StateWeight;

    // Threat Level matching
    float ThreatWeight = ContextWeights.FindRef(TEXT("ThreatLevel"));
    float ThreatScore = CalculateThreatWeight(RequiredContext.ThreatLevel, CurrentContext.ThreatLevel);
    TotalScore += ThreatScore * ThreatWeight;
    TotalWeight += ThreatWeight;

    // Fear Intensity matching
    float FearWeight = ContextWeights.FindRef(TEXT("FearIntensity"));
    float FearDifference = FMath::Abs(RequiredContext.FearIntensity - CurrentContext.FearIntensity);
    float FearScore = FMath::Max(0.0f, 1.0f - FearDifference);
    TotalScore += FearScore * FearWeight;
    TotalWeight += FearWeight;

    // Stamina Level matching
    float StaminaWeight = ContextWeights.FindRef(TEXT("StaminaLevel"));
    float StaminaDifference = FMath::Abs(RequiredContext.StaminaLevel - CurrentContext.StaminaLevel);
    float StaminaScore = FMath::Max(0.0f, 1.0f - StaminaDifference);
    TotalScore += StaminaScore * StaminaWeight;
    TotalWeight += StaminaWeight;

    // Environment Type matching
    float EnvironmentWeight = ContextWeights.FindRef(TEXT("Environment"));
    float EnvironmentScore = (RequiredContext.EnvironmentType == CurrentContext.EnvironmentType) ? 1.0f : 0.5f;
    TotalScore += EnvironmentScore * EnvironmentWeight;
    TotalWeight += EnvironmentWeight;

    // Terrain Slope matching
    float TerrainWeight = ContextWeights.FindRef(TEXT("TerrainSlope"));
    float TerrainDifference = FMath::Abs(RequiredContext.TerrainSlope - CurrentContext.TerrainSlope) / 90.0f; // Normalize to 0-1
    float TerrainScore = FMath::Max(0.0f, 1.0f - TerrainDifference);
    TotalScore += TerrainScore * TerrainWeight;
    TotalWeight += TerrainWeight;

    // Binary state matching
    if (RequiredContext.bIsInjured == CurrentContext.bIsInjured)
    {
        TotalScore += 0.2f;
    }
    TotalWeight += 0.2f;

    if (RequiredContext.bIsCarryingObject == CurrentContext.bIsCarryingObject)
    {
        TotalScore += 0.2f;
    }
    TotalWeight += 0.2f;

    return TotalWeight > 0.0f ? TotalScore / TotalWeight : 0.0f;
}

int32 UMotionMatchingDatabase::GetOptimalLODLevel(float DistanceToPlayer) const
{
    for (int32 i = 0; i < LODLevels.Num(); ++i)
    {
        if (DistanceToPlayer <= LODLevels[i].MaxDistance)
        {
            return i;
        }
    }

    return LODLevels.Num() - 1; // Return highest LOD if beyond all distances
}

void UMotionMatchingDatabase::RefreshDatabaseCache()
{
    DatabaseCache.Empty();
    
    for (const FDatabaseEntry& Entry : DatabaseEntries)
    {
        if (Entry.bIsActive && Entry.Database.IsValid())
        {
            FString CacheKey = FString::Printf(TEXT("%s_%d_%d"), 
                *UEnum::GetValueAsString(Entry.RequiredContext.MovementState),
                (int32)Entry.RequiredContext.ThreatLevel,
                Entry.RequiredContext.bIsInjured ? 1 : 0);
                
            DatabaseCache.Add(CacheKey, Entry.Database.LoadSynchronous());
        }
    }

    LastCacheRefresh = FPlatformTime::Seconds();
    
    UE_LOG(LogAnimation, Log, TEXT("MotionMatchingDatabase: Refreshed cache with %d entries"), DatabaseCache.Num());
}

bool UMotionMatchingDatabase::ValidateDatabase(const FDatabaseEntry& Entry) const
{
    if (!Entry.Database.IsValid())
    {
        UE_LOG(LogAnimation, Warning, TEXT("MotionMatchingDatabase: Invalid database reference in entry"));
        return false;
    }

    if (Entry.Priority <= 0.0f)
    {
        UE_LOG(LogAnimation, Warning, TEXT("MotionMatchingDatabase: Entry has invalid priority: %f"), Entry.Priority);
        return false;
    }

    if (Entry.BlendWeight < 0.0f || Entry.BlendWeight > 1.0f)
    {
        UE_LOG(LogAnimation, Warning, TEXT("MotionMatchingDatabase: Entry has invalid blend weight: %f"), Entry.BlendWeight);
        return false;
    }

    return true;
}

void UMotionMatchingDatabase::OptimizeForPerformance()
{
    // Sort entries by priority for faster lookup
    SortEntriesByPriority();
    
    // Preload high-priority databases
    for (const FDatabaseEntry& Entry : DatabaseEntries)
    {
        if (Entry.bIsActive && Entry.Priority >= 0.8f && Entry.Database.IsValid())
        {
            Entry.Database.LoadSynchronous();
        }
    }
    
    // Refresh cache
    RefreshDatabaseCache();
    
    UE_LOG(LogAnimation, Log, TEXT("MotionMatchingDatabase: Performance optimization complete"));
}

void UMotionMatchingDatabase::SortEntriesByPriority()
{
    SortedEntries = DatabaseEntries;
    SortedEntries.Sort([](const FDatabaseEntry& A, const FDatabaseEntry& B)
    {
        return A.Priority > B.Priority;
    });
}

float UMotionMatchingDatabase::CalculateStateWeight(ECharacterMovementState RequiredState, ECharacterMovementState CurrentState) const
{
    if (RequiredState == CurrentState)
    {
        return 1.0f;
    }

    // Define state compatibility matrix
    switch (RequiredState)
    {
        case ECharacterMovementState::Idle:
            return (CurrentState == ECharacterMovementState::Walking) ? 0.7f : 0.3f;
        
        case ECharacterMovementState::Walking:
            if (CurrentState == ECharacterMovementState::Idle || CurrentState == ECharacterMovementState::Running)
                return 0.8f;
            return 0.4f;
        
        case ECharacterMovementState::Running:
            return (CurrentState == ECharacterMovementState::Walking) ? 0.9f : 0.2f;
        
        case ECharacterMovementState::Sprinting:
            return (CurrentState == ECharacterMovementState::Running) ? 0.8f : 0.1f;
        
        case ECharacterMovementState::Crouching:
            return (CurrentState == ECharacterMovementState::Idle) ? 0.6f : 0.2f;
        
        case ECharacterMovementState::Crawling:
            return (CurrentState == ECharacterMovementState::Crouching) ? 0.7f : 0.1f;
        
        default:
            return 0.5f;
    }
}

float UMotionMatchingDatabase::CalculateThreatWeight(EThreatLevel RequiredThreat, EThreatLevel CurrentThreat) const
{
    if (RequiredThreat == CurrentThreat)
    {
        return 1.0f;
    }

    int32 ThreatDifference = FMath::Abs((int32)RequiredThreat - (int32)CurrentThreat);
    
    switch (ThreatDifference)
    {
        case 1: return 0.8f;
        case 2: return 0.6f;
        case 3: return 0.4f;
        default: return 0.2f;
    }
}

bool UMotionMatchingDatabase::DoesEntryMatchTags(const FDatabaseEntry& Entry, const TArray<FString>& CurrentTags) const
{
    // Check required tags
    for (const FString& RequiredTag : Entry.RequiredTags)
    {
        if (!CurrentTags.Contains(RequiredTag))
        {
            return false;
        }
    }

    // Check excluded tags
    for (const FString& ExcludedTag : Entry.ExcludedTags)
    {
        if (CurrentTags.Contains(ExcludedTag))
        {
            return false;
        }
    }

    return true;
}

void UMotionMatchingDatabase::LoadDatabasesAsync()
{
    for (const FDatabaseEntry& Entry : DatabaseEntries)
    {
        if (Entry.bIsActive && Entry.Database.IsValid() && !Entry.Database.IsLoaded())
        {
            Entry.Database.LoadSynchronous(); // For now, could be made async later
        }
    }
}

void UMotionMatchingDatabase::UnloadUnusedDatabases()
{
    // Implementation for unloading databases that haven't been used recently
    // This would help with memory management in larger projects
    float CurrentTime = FPlatformTime::Seconds();
    float UnloadThreshold = 300.0f; // 5 minutes
    
    if (CurrentTime - LastCacheRefresh > UnloadThreshold)
    {
        // Logic to unload unused databases would go here
        UE_LOG(LogAnimation, Log, TEXT("MotionMatchingDatabase: Checking for unused databases to unload"));
    }
}