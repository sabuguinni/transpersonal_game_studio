#include "Perf_StreamingOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY(LogPerfStreaming);

UPerf_StreamingOptimizer::UPerf_StreamingOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update twice per second
    
    // Initialize configuration
    StreamingConfig = FPerf_StreamingConfig();
    bAutoManageStreaming = true;
    UpdateInterval = 0.5f;
    
    // Initialize state
    LastUpdateTime = 0.0f;
    CurrentConcurrentLoads = 0;
    CurrentMemoryUsage = 0.0f;
}

void UPerf_StreamingOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeStreamingLevels();
    
    UE_LOG(LogPerfStreaming, Log, TEXT("Streaming Optimizer initialized with %d levels"), StreamingLevels.Num());
}

void UPerf_StreamingOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bAutoManageStreaming)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateInterval)
    {
        UpdateStreamingLevels();
        LastUpdateTime = CurrentTime;
    }
}

void UPerf_StreamingOptimizer::InitializeStreamingLevels()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    StreamingLevels.Empty();
    
    // Get all streaming levels
    const TArray<ULevelStreaming*>& StreamingLevelsArray = World->GetStreamingLevels();
    
    for (ULevelStreaming* StreamingLevel : StreamingLevelsArray)
    {
        if (StreamingLevel)
        {
            FString LevelName = StreamingLevel->GetWorldAssetPackageName();
            
            FPerf_StreamingLevelInfo LevelInfo;
            LevelInfo.LevelName = LevelName;
            LevelInfo.State = StreamingLevel->IsLevelLoaded() ? EPerf_StreamingState::Loaded : EPerf_StreamingState::Unloaded;
            LevelInfo.Priority = EPerf_StreamingPriority::Medium;
            LevelInfo.DistanceToPlayer = 0.0f;
            LevelInfo.LoadProgress = StreamingLevel->IsLevelLoaded() ? 1.0f : 0.0f;
            LevelInfo.MemoryUsageMB = CalculateMemoryUsage(LevelName);
            LevelInfo.bShouldBeLoaded = false;
            
            StreamingLevels.Add(LevelName, LevelInfo);
        }
    }
    
    UE_LOG(LogPerfStreaming, Log, TEXT("Initialized %d streaming levels"), StreamingLevels.Num());
}

void UPerf_StreamingOptimizer::UpdateStreamingLevels()
{
    // Update level states and distances
    UpdateLevelStates();
    UpdatePlayerDistance();
    
    // Process queues
    ProcessLoadQueue();
    ProcessUnloadQueue();
    
    // Check memory usage
    CheckMemoryUsage();
    
    // Auto-manage streaming based on distance
    if (bAutoManageStreaming)
    {
        if (StreamingConfig.bEnablePreloading)
        {
            PreloadNearbyLevels();
        }
        
        if (StreamingConfig.bUnloadDistantLevels)
        {
            UnloadDistantLevels();
        }
    }
}

void UPerf_StreamingOptimizer::UpdateLevelStates()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (auto& LevelPair : StreamingLevels)
    {
        FString& LevelName = LevelPair.Key;
        FPerf_StreamingLevelInfo& LevelInfo = LevelPair.Value;
        
        ULevelStreaming* StreamingLevel = FindStreamingLevel(LevelName);
        if (StreamingLevel)
        {
            EPerf_StreamingState OldState = LevelInfo.State;
            
            if (StreamingLevel->IsLevelLoaded())
            {
                LevelInfo.State = EPerf_StreamingState::Loaded;
                LevelInfo.LoadProgress = 1.0f;
            }
            else if (StreamingLevel->IsLevelLoadedRequestPending())
            {
                LevelInfo.State = EPerf_StreamingState::Loading;
                LevelInfo.LoadProgress = 0.5f; // Approximate
            }
            else if (StreamingLevel->IsLevelUnloadRequestPending())
            {
                LevelInfo.State = EPerf_StreamingState::Unloading;
                LevelInfo.LoadProgress = 0.5f; // Approximate
            }
            else
            {
                LevelInfo.State = EPerf_StreamingState::Unloaded;
                LevelInfo.LoadProgress = 0.0f;
            }
            
            // Broadcast state change if needed
            if (OldState != LevelInfo.State)
            {
                BroadcastStateChange(LevelName, LevelInfo.State);
            }
        }
    }
}

void UPerf_StreamingOptimizer::UpdatePlayerDistance()
{
    FVector PlayerLocation = GetPlayerLocation();
    
    for (auto& LevelPair : StreamingLevels)
    {
        FPerf_StreamingLevelInfo& LevelInfo = LevelPair.Value;
        
        // For now, use a simple distance calculation
        // In a real implementation, you'd get the level's bounds center
        FVector LevelCenter = FVector::ZeroVector; // Placeholder
        LevelInfo.DistanceToPlayer = FVector::Dist(PlayerLocation, LevelCenter);
        
        // Update priority based on distance
        LevelInfo.Priority = CalculatePriority(LevelInfo.DistanceToPlayer);
        
        // Determine if level should be loaded
        LevelInfo.bShouldBeLoaded = ShouldLoadLevel(LevelInfo.LevelName, LevelInfo.DistanceToPlayer);
    }
}

void UPerf_StreamingOptimizer::ProcessLoadQueue()
{
    if (LoadQueue.Num() == 0 || CurrentConcurrentLoads >= StreamingConfig.MaxConcurrentLoads)
    {
        return;
    }
    
    // Sort by priority (process high priority first)
    LoadQueue.Sort([this](const FString& A, const FString& B) {
        FPerf_StreamingLevelInfo* InfoA = StreamingLevels.Find(A);
        FPerf_StreamingLevelInfo* InfoB = StreamingLevels.Find(B);
        
        if (InfoA && InfoB)
        {
            return static_cast<int32>(InfoA->Priority) > static_cast<int32>(InfoB->Priority);
        }
        return false;
    });
    
    // Process load requests
    int32 ProcessedCount = 0;
    for (int32 i = LoadQueue.Num() - 1; i >= 0 && CurrentConcurrentLoads < StreamingConfig.MaxConcurrentLoads; i--)
    {
        FString LevelName = LoadQueue[i];
        
        ULevelStreaming* StreamingLevel = FindStreamingLevel(LevelName);
        if (StreamingLevel && !StreamingLevel->IsLevelLoaded() && !StreamingLevel->IsLevelLoadedRequestPending())
        {
            StreamingLevel->SetShouldBeLoaded(true);
            CurrentConcurrentLoads++;
            ProcessedCount++;
            
            UE_LOG(LogPerfStreaming, Log, TEXT("Started loading level: %s"), *LevelName);
        }
        
        LoadQueue.RemoveAt(i);
    }
}

void UPerf_StreamingOptimizer::ProcessUnloadQueue()
{
    for (int32 i = UnloadQueue.Num() - 1; i >= 0; i--)
    {
        FString LevelName = UnloadQueue[i];
        
        ULevelStreaming* StreamingLevel = FindStreamingLevel(LevelName);
        if (StreamingLevel && StreamingLevel->IsLevelLoaded() && !StreamingLevel->IsLevelUnloadRequestPending())
        {
            StreamingLevel->SetShouldBeLoaded(false);
            
            UE_LOG(LogPerfStreaming, Log, TEXT("Started unloading level: %s"), *LevelName);
        }
        
        UnloadQueue.RemoveAt(i);
    }
}

void UPerf_StreamingOptimizer::CheckMemoryUsage()
{
    CurrentMemoryUsage = GetTotalStreamingMemoryUsage();
    
    if (CurrentMemoryUsage > StreamingConfig.MaxMemoryUsageMB)
    {
        UE_LOG(LogPerfStreaming, Warning, TEXT("Streaming memory usage exceeded: %.1f MB / %.1f MB"), 
               CurrentMemoryUsage, StreamingConfig.MaxMemoryUsageMB);
        
        OnStreamingMemoryWarning.Broadcast(CurrentMemoryUsage);
        
        // Emergency unload distant levels
        EmergencyUnloadLevels();
    }
}

void UPerf_StreamingOptimizer::LoadLevel(const FString& LevelName, EPerf_StreamingPriority Priority)
{
    if (StreamingLevels.Contains(LevelName))
    {
        StreamingLevels[LevelName].Priority = Priority;
        
        if (!LoadQueue.Contains(LevelName))
        {
            LoadQueue.Add(LevelName);
        }
    }
}

void UPerf_StreamingOptimizer::UnloadLevel(const FString& LevelName)
{
    if (StreamingLevels.Contains(LevelName))
    {
        if (!UnloadQueue.Contains(LevelName))
        {
            UnloadQueue.Add(LevelName);
        }
    }
}

void UPerf_StreamingOptimizer::PreloadNearbyLevels()
{
    for (const auto& LevelPair : StreamingLevels)
    {
        const FString& LevelName = LevelPair.Key;
        const FPerf_StreamingLevelInfo& LevelInfo = LevelPair.Value;
        
        if (LevelInfo.bShouldBeLoaded && LevelInfo.State == EPerf_StreamingState::Unloaded)
        {
            LoadLevel(LevelName, LevelInfo.Priority);
        }
    }
}

void UPerf_StreamingOptimizer::UnloadDistantLevels()
{
    for (const auto& LevelPair : StreamingLevels)
    {
        const FString& LevelName = LevelPair.Key;
        const FPerf_StreamingLevelInfo& LevelInfo = LevelPair.Value;
        
        if (!LevelInfo.bShouldBeLoaded && LevelInfo.State == EPerf_StreamingState::Loaded)
        {
            if (ShouldUnloadLevel(LevelName, LevelInfo.DistanceToPlayer))
            {
                UnloadLevel(LevelName);
            }
        }
    }
}

bool UPerf_StreamingOptimizer::ShouldLoadLevel(const FString& LevelName, float Distance) const
{
    return Distance <= StreamingConfig.StreamingDistance;
}

bool UPerf_StreamingOptimizer::ShouldUnloadLevel(const FString& LevelName, float Distance) const
{
    return Distance >= StreamingConfig.UnloadDistance;
}

float UPerf_StreamingOptimizer::CalculateMemoryUsage(const FString& LevelName) const
{
    // Placeholder implementation - would need actual memory tracking
    return 50.0f; // Assume 50MB per level for now
}

EPerf_StreamingPriority UPerf_StreamingOptimizer::CalculatePriority(float Distance) const
{
    if (Distance < StreamingConfig.StreamingDistance * 0.25f)
    {
        return EPerf_StreamingPriority::Critical;
    }
    else if (Distance < StreamingConfig.StreamingDistance * 0.5f)
    {
        return EPerf_StreamingPriority::High;
    }
    else if (Distance < StreamingConfig.StreamingDistance * 0.75f)
    {
        return EPerf_StreamingPriority::Medium;
    }
    else if (Distance < StreamingConfig.StreamingDistance)
    {
        return EPerf_StreamingPriority::Low;
    }
    else
    {
        return EPerf_StreamingPriority::Background;
    }
}

FVector UPerf_StreamingOptimizer::GetPlayerLocation() const
{
    UWorld* World = GetWorld();
    if (World)
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* Pawn = PC->GetPawn())
            {
                return Pawn->GetActorLocation();
            }
        }
    }
    return FVector::ZeroVector;
}

ULevelStreaming* UPerf_StreamingOptimizer::FindStreamingLevel(const FString& LevelName) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    const TArray<ULevelStreaming*>& StreamingLevelsArray = World->GetStreamingLevels();
    
    for (ULevelStreaming* StreamingLevel : StreamingLevelsArray)
    {
        if (StreamingLevel && StreamingLevel->GetWorldAssetPackageName() == LevelName)
        {
            return StreamingLevel;
        }
    }
    
    return nullptr;
}

// Query Functions
TArray<FPerf_StreamingLevelInfo> UPerf_StreamingOptimizer::GetStreamingLevelInfo() const
{
    TArray<FPerf_StreamingLevelInfo> Result;
    for (const auto& LevelPair : StreamingLevels)
    {
        Result.Add(LevelPair.Value);
    }
    return Result;
}

EPerf_StreamingState UPerf_StreamingOptimizer::GetLevelStreamingState(const FString& LevelName) const
{
    if (const FPerf_StreamingLevelInfo* LevelInfo = StreamingLevels.Find(LevelName))
    {
        return LevelInfo->State;
    }
    return EPerf_StreamingState::Unloaded;
}

float UPerf_StreamingOptimizer::GetTotalStreamingMemoryUsage() const
{
    float TotalMemory = 0.0f;
    for (const auto& LevelPair : StreamingLevels)
    {
        if (LevelPair.Value.State == EPerf_StreamingState::Loaded)
        {
            TotalMemory += LevelPair.Value.MemoryUsageMB;
        }
    }
    return TotalMemory;
}

int32 UPerf_StreamingOptimizer::GetActiveStreamingLevels() const
{
    int32 ActiveCount = 0;
    for (const auto& LevelPair : StreamingLevels)
    {
        if (LevelPair.Value.State == EPerf_StreamingState::Loaded || 
            LevelPair.Value.State == EPerf_StreamingState::Loading)
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

bool UPerf_StreamingOptimizer::IsLevelLoaded(const FString& LevelName) const
{
    if (const FPerf_StreamingLevelInfo* LevelInfo = StreamingLevels.Find(LevelName))
    {
        return LevelInfo->State == EPerf_StreamingState::Loaded;
    }
    return false;
}

// Configuration Functions
void UPerf_StreamingOptimizer::SetStreamingDistance(float Distance)
{
    StreamingConfig.StreamingDistance = FMath::Clamp(Distance, 1000.0f, 50000.0f);
    UE_LOG(LogPerfStreaming, Log, TEXT("Streaming distance set to: %.1f"), StreamingConfig.StreamingDistance);
}

void UPerf_StreamingOptimizer::SetMaxConcurrentLoads(int32 MaxLoads)
{
    StreamingConfig.MaxConcurrentLoads = FMath::Clamp(MaxLoads, 1, 10);
    UE_LOG(LogPerfStreaming, Log, TEXT("Max concurrent loads set to: %d"), StreamingConfig.MaxConcurrentLoads);
}

void UPerf_StreamingOptimizer::SetAutoManageStreaming(bool bEnabled)
{
    bAutoManageStreaming = bEnabled;
    UE_LOG(LogPerfStreaming, Log, TEXT("Auto manage streaming %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

// Performance Functions
void UPerf_StreamingOptimizer::OptimizeStreamingForPerformance()
{
    // Reduce streaming distance and concurrent loads for better performance
    StreamingConfig.StreamingDistance *= 0.75f;
    StreamingConfig.MaxConcurrentLoads = FMath::Max(1, StreamingConfig.MaxConcurrentLoads - 1);
    
    UE_LOG(LogPerfStreaming, Log, TEXT("Optimized streaming for performance"));
}

void UPerf_StreamingOptimizer::FlushStreamingRequests()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->FlushLevelStreaming();
    }
}

void UPerf_StreamingOptimizer::EmergencyUnloadLevels()
{
    // Find the most distant loaded levels and unload them
    TArray<TPair<FString, float>> LevelDistances;
    
    for (const auto& LevelPair : StreamingLevels)
    {
        if (LevelPair.Value.State == EPerf_StreamingState::Loaded)
        {
            LevelDistances.Add(TPair<FString, float>(LevelPair.Key, LevelPair.Value.DistanceToPlayer));
        }
    }
    
    // Sort by distance (farthest first)
    LevelDistances.Sort([](const TPair<FString, float>& A, const TPair<FString, float>& B) {
        return A.Value > B.Value;
    });
    
    // Unload the farthest levels until memory is under control
    int32 UnloadCount = FMath::Min(3, LevelDistances.Num()); // Unload up to 3 levels
    for (int32 i = 0; i < UnloadCount; i++)
    {
        UnloadLevel(LevelDistances[i].Key);
    }
    
    UE_LOG(LogPerfStreaming, Warning, TEXT("Emergency unloaded %d levels"), UnloadCount);
}

void UPerf_StreamingOptimizer::BroadcastStateChange(const FString& LevelName, EPerf_StreamingState NewState)
{
    OnLevelStreamingStateChanged.Broadcast(LevelName, NewState);
}