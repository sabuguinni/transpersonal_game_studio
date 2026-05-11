#include "Perf_WorldStreamingOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UPerf_WorldStreamingOptimizer::UPerf_WorldStreamingOptimizer()
{
    // Initialize default values
    BaseStreamingDistance = 15000.0f;
    MaxStreamingDistance = 50000.0f;
    StreamingDistanceMultiplier = 1.0f;
    MaxConcurrentStreamingRequests = 8;
    TargetFrameRate = 60.0f;
    MinFrameRate = 30.0f;
    MaxMemoryUsageMB = 4096.0f;
    bAdaptiveStreamingEnabled = true;
    AdaptiveUpdateInterval = 1.0f;
    PerformanceThreshold = 0.8f;
    bIsOptimizing = false;
    LastPerformanceCheck = 0.0f;
}

void UPerf_WorldStreamingOptimizer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("WorldStreamingOptimizer: Initializing world streaming optimization system"));
    
    // Start adaptive streaming timer if enabled
    if (bAdaptiveStreamingEnabled)
    {
        GetGameInstance()->GetTimerManager().SetTimer(
            AdaptiveTimerHandle,
            this,
            &UPerf_WorldStreamingOptimizer::UpdateAdaptiveStreaming,
            AdaptiveUpdateInterval,
            true
        );
    }
    
    // Initialize default streaming regions for prehistoric world
    FPerf_StreamingRegion CentralRegion;
    CentralRegion.Center = FVector::ZeroVector;
    CentralRegion.Extent = FVector(20000.0f, 20000.0f, 5000.0f);
    CentralRegion.Priority = EPerf_StreamingPriority::High;
    CentralRegion.StreamingDistance = 25000.0f;
    CentralRegion.UnloadDistance = 35000.0f;
    CentralRegion.bForceLoaded = true;
    StreamingRegions.Add(CentralRegion);
    
    UE_LOG(LogTemp, Log, TEXT("WorldStreamingOptimizer: Initialization complete"));
}

void UPerf_WorldStreamingOptimizer::Deinitialize()
{
    if (AdaptiveTimerHandle.IsValid())
    {
        GetGameInstance()->GetTimerManager().ClearTimer(AdaptiveTimerHandle);
    }
    
    Super::Deinitialize();
}

void UPerf_WorldStreamingOptimizer::OptimizeWorldStreaming()
{
    if (bIsOptimizing)
    {
        return;
    }
    
    bIsOptimizing = true;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        bIsOptimizing = false;
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("WorldStreamingOptimizer: Starting world streaming optimization"));
    
    // Get player location for optimization
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    FVector PlayerVelocity = PlayerPawn ? PlayerPawn->GetVelocity() : FVector::ZeroVector;
    
    // Update streaming priorities based on player position
    UpdateStreamingPriorities(PlayerLocation, PlayerVelocity);
    
    // Calculate optimal streaming distance
    CalculateOptimalStreamingDistance(PlayerLocation);
    
    // Optimize memory usage
    OptimizeMemoryUsage();
    
    // Apply optimizations
    ApplyStreamingOptimizations();
    
    // Monitor performance
    MonitorStreamingPerformance();
    
    bIsOptimizing = false;
    
    UE_LOG(LogTemp, Log, TEXT("WorldStreamingOptimizer: Optimization cycle complete"));
}

void UPerf_WorldStreamingOptimizer::SetStreamingQuality(float QualityLevel)
{
    QualityLevel = FMath::Clamp(QualityLevel, 0.1f, 2.0f);
    
    StreamingDistanceMultiplier = QualityLevel;
    
    // Adjust streaming parameters based on quality
    float AdjustedBaseDistance = BaseStreamingDistance * QualityLevel;
    float AdjustedMaxDistance = MaxStreamingDistance * QualityLevel;
    
    // Update concurrent request limit based on quality
    int32 AdjustedMaxRequests = FMath::RoundToInt(MaxConcurrentStreamingRequests * QualityLevel);
    AdjustedMaxRequests = FMath::Clamp(AdjustedMaxRequests, 2, 16);
    
    UE_LOG(LogTemp, Log, TEXT("WorldStreamingOptimizer: Set streaming quality to %.2f (Distance: %.0f, Max Requests: %d)"),
        QualityLevel, AdjustedBaseDistance, AdjustedMaxRequests);
}

void UPerf_WorldStreamingOptimizer::UpdateStreamingPriorities(const FVector& PlayerLocation, const FVector& PlayerVelocity)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Update streaming source with player location and velocity
    UpdateStreamingSource(PlayerLocation, PlayerVelocity);
    
    // Calculate priorities for each streaming region
    for (FPerf_StreamingRegion& Region : StreamingRegions)
    {
        float Priority = CalculateRegionPriority(Region, PlayerLocation);
        
        // Adjust streaming distance based on priority
        if (Priority > 0.8f)
        {
            Region.Priority = EPerf_StreamingPriority::Critical;
        }
        else if (Priority > 0.6f)
        {
            Region.Priority = EPerf_StreamingPriority::High;
        }
        else if (Priority > 0.4f)
        {
            Region.Priority = EPerf_StreamingPriority::Medium;
        }
        else if (Priority > 0.2f)
        {
            Region.Priority = EPerf_StreamingPriority::Low;
        }
        else
        {
            Region.Priority = EPerf_StreamingPriority::Background;
        }
    }
}

void UPerf_WorldStreamingOptimizer::AddStreamingRegion(const FPerf_StreamingRegion& Region)
{
    StreamingRegions.Add(Region);
    UE_LOG(LogTemp, Log, TEXT("WorldStreamingOptimizer: Added streaming region at %s"), *Region.Center.ToString());
}

void UPerf_WorldStreamingOptimizer::RemoveStreamingRegion(int32 RegionIndex)
{
    if (StreamingRegions.IsValidIndex(RegionIndex))
    {
        StreamingRegions.RemoveAt(RegionIndex);
        UE_LOG(LogTemp, Log, TEXT("WorldStreamingOptimizer: Removed streaming region at index %d"), RegionIndex);
    }
}

void UPerf_WorldStreamingOptimizer::ClearStreamingRegions()
{
    StreamingRegions.Empty();
    UE_LOG(LogTemp, Log, TEXT("WorldStreamingOptimizer: Cleared all streaming regions"));
}

FPerf_StreamingMetrics UPerf_WorldStreamingOptimizer::GetStreamingMetrics() const
{
    FPerf_StreamingMetrics Metrics;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return Metrics;
    }
    
    // Calculate memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Metrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Get streaming statistics (simplified for now)
    Metrics.LoadedCells = StreamingRegions.Num();
    Metrics.StreamingCells = 0;
    Metrics.StreamingTimeMs = LastPerformanceCheck;
    Metrics.PendingRequests = 0;
    
    return Metrics;
}

float UPerf_WorldStreamingOptimizer::GetStreamingPerformanceScore() const
{
    FPerf_StreamingMetrics Metrics = GetStreamingMetrics();
    
    // Calculate performance score based on memory usage and frame rate
    float MemoryScore = 1.0f - FMath::Clamp(Metrics.MemoryUsageMB / MaxMemoryUsageMB, 0.0f, 1.0f);
    
    // Get current frame rate (simplified)
    float CurrentFPS = 1.0f / FApp::GetDeltaTime();
    float FrameRateScore = FMath::Clamp(CurrentFPS / TargetFrameRate, 0.0f, 1.0f);
    
    // Combine scores
    float OverallScore = (MemoryScore * 0.4f) + (FrameRateScore * 0.6f);
    
    return OverallScore;
}

void UPerf_WorldStreamingOptimizer::EnableAdaptiveStreaming(bool bEnable)
{
    bAdaptiveStreamingEnabled = bEnable;
    
    if (bEnable && !AdaptiveTimerHandle.IsValid())
    {
        GetGameInstance()->GetTimerManager().SetTimer(
            AdaptiveTimerHandle,
            this,
            &UPerf_WorldStreamingOptimizer::UpdateAdaptiveStreaming,
            AdaptiveUpdateInterval,
            true
        );
    }
    else if (!bEnable && AdaptiveTimerHandle.IsValid())
    {
        GetGameInstance()->GetTimerManager().ClearTimer(AdaptiveTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("WorldStreamingOptimizer: Adaptive streaming %s"), 
        bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_WorldStreamingOptimizer::SetTargetFrameRate(float TargetFPS)
{
    TargetFrameRate = FMath::Clamp(TargetFPS, 15.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("WorldStreamingOptimizer: Set target frame rate to %.1f FPS"), TargetFrameRate);
}

void UPerf_WorldStreamingOptimizer::DebugDrawStreamingRegions()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (const FPerf_StreamingRegion& Region : StreamingRegions)
    {
        FColor RegionColor = FColor::Green;
        
        switch (Region.Priority)
        {
            case EPerf_StreamingPriority::Critical:
                RegionColor = FColor::Red;
                break;
            case EPerf_StreamingPriority::High:
                RegionColor = FColor::Orange;
                break;
            case EPerf_StreamingPriority::Medium:
                RegionColor = FColor::Yellow;
                break;
            case EPerf_StreamingPriority::Low:
                RegionColor = FColor::Blue;
                break;
            case EPerf_StreamingPriority::Background:
                RegionColor = FColor::Gray;
                break;
        }
        
        DrawDebugBox(World, Region.Center, Region.Extent, RegionColor, false, 5.0f, 0, 100.0f);
    }
}

void UPerf_WorldStreamingOptimizer::LogStreamingStatus()
{
    FPerf_StreamingMetrics Metrics = GetStreamingMetrics();
    float PerformanceScore = GetStreamingPerformanceScore();
    
    UE_LOG(LogTemp, Log, TEXT("=== STREAMING STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("Loaded Cells: %d"), Metrics.LoadedCells);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.1f MB"), Metrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Performance Score: %.2f"), PerformanceScore);
    UE_LOG(LogTemp, Log, TEXT("Streaming Regions: %d"), StreamingRegions.Num());
    UE_LOG(LogTemp, Log, TEXT("Adaptive Streaming: %s"), bAdaptiveStreamingEnabled ? TEXT("ON") : TEXT("OFF"));
}

void UPerf_WorldStreamingOptimizer::UpdateAdaptiveStreaming()
{
    if (!bAdaptiveStreamingEnabled)
    {
        return;
    }
    
    float PerformanceScore = GetStreamingPerformanceScore();
    
    if (PerformanceScore < PerformanceThreshold)
    {
        // Performance is below threshold, reduce streaming quality
        float NewQuality = StreamingDistanceMultiplier * 0.9f;
        SetStreamingQuality(NewQuality);
        
        UE_LOG(LogTemp, Warning, TEXT("WorldStreamingOptimizer: Performance below threshold (%.2f), reducing quality to %.2f"),
            PerformanceScore, NewQuality);
    }
    else if (PerformanceScore > PerformanceThreshold + 0.1f)
    {
        // Performance is good, can increase streaming quality
        float NewQuality = FMath::Min(StreamingDistanceMultiplier * 1.05f, 2.0f);
        SetStreamingQuality(NewQuality);
    }
    
    // Update cached metrics
    CachedMetrics = GetStreamingMetrics();
    LastPerformanceCheck = FApp::GetCurrentTime();
}

void UPerf_WorldStreamingOptimizer::CalculateOptimalStreamingDistance(const FVector& PlayerLocation)
{
    // Calculate optimal streaming distance based on player movement and performance
    float CurrentFPS = 1.0f / FApp::GetDeltaTime();
    float PerformanceRatio = CurrentFPS / TargetFrameRate;
    
    // Adjust base streaming distance based on performance
    float OptimalDistance = BaseStreamingDistance;
    
    if (PerformanceRatio < 0.8f)
    {
        // Performance is poor, reduce streaming distance
        OptimalDistance *= 0.8f;
    }
    else if (PerformanceRatio > 1.2f)
    {
        // Performance is good, can increase streaming distance
        OptimalDistance *= 1.2f;
    }
    
    OptimalDistance = FMath::Clamp(OptimalDistance, BaseStreamingDistance * 0.5f, MaxStreamingDistance);
    
    // Apply the calculated distance
    StreamingDistanceMultiplier = OptimalDistance / BaseStreamingDistance;
}

void UPerf_WorldStreamingOptimizer::OptimizeMemoryUsage()
{
    FPerf_StreamingMetrics Metrics = GetStreamingMetrics();
    
    if (Metrics.MemoryUsageMB > MaxMemoryUsageMB * 0.9f)
    {
        // Memory usage is high, force garbage collection and reduce streaming distance
        GEngine->ForceGarbageCollection(true);
        
        float MemoryPressureRatio = Metrics.MemoryUsageMB / MaxMemoryUsageMB;
        float ReductionFactor = FMath::Clamp(2.0f - MemoryPressureRatio, 0.5f, 1.0f);
        
        StreamingDistanceMultiplier *= ReductionFactor;
        
        UE_LOG(LogTemp, Warning, TEXT("WorldStreamingOptimizer: High memory usage (%.1f MB), reducing streaming distance by %.2f"),
            Metrics.MemoryUsageMB, ReductionFactor);
    }
}

void UPerf_WorldStreamingOptimizer::UpdateStreamingSource(const FVector& Location, const FVector& Velocity)
{
    // This would integrate with UE5's World Partition streaming system
    // For now, we'll log the update
    UE_LOG(LogTemp, VeryVerbose, TEXT("WorldStreamingOptimizer: Updated streaming source - Location: %s, Velocity: %s"),
        *Location.ToString(), *Velocity.ToString());
}

float UPerf_WorldStreamingOptimizer::CalculateRegionPriority(const FPerf_StreamingRegion& Region, const FVector& PlayerLocation) const
{
    float Distance = FVector::Dist(PlayerLocation, Region.Center);
    float MaxDistance = Region.StreamingDistance;
    
    if (Distance > MaxDistance)
    {
        return 0.0f;
    }
    
    // Calculate priority based on distance and region settings
    float DistancePriority = 1.0f - (Distance / MaxDistance);
    
    // Apply region-specific priority multiplier
    float RegionMultiplier = 1.0f;
    switch (Region.Priority)
    {
        case EPerf_StreamingPriority::Critical:
            RegionMultiplier = 2.0f;
            break;
        case EPerf_StreamingPriority::High:
            RegionMultiplier = 1.5f;
            break;
        case EPerf_StreamingPriority::Medium:
            RegionMultiplier = 1.0f;
            break;
        case EPerf_StreamingPriority::Low:
            RegionMultiplier = 0.7f;
            break;
        case EPerf_StreamingPriority::Background:
            RegionMultiplier = 0.3f;
            break;
    }
    
    return DistancePriority * RegionMultiplier;
}

void UPerf_WorldStreamingOptimizer::ApplyStreamingOptimizations()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Apply calculated optimizations to the world streaming system
    // This would integrate with UE5's World Partition system
    UE_LOG(LogTemp, VeryVerbose, TEXT("WorldStreamingOptimizer: Applied streaming optimizations"));
}

void UPerf_WorldStreamingOptimizer::MonitorStreamingPerformance()
{
    // Monitor streaming performance and log issues
    FPerf_StreamingMetrics Metrics = GetStreamingMetrics();
    float PerformanceScore = GetStreamingPerformanceScore();
    
    if (PerformanceScore < 0.5f)
    {
        UE_LOG(LogTemp, Warning, TEXT("WorldStreamingOptimizer: Poor streaming performance detected (Score: %.2f)"), PerformanceScore);
    }
    
    // Update cached metrics for external access
    CachedMetrics = Metrics;
}