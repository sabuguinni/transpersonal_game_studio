#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionStreamingSource.h"
#include "Components/ActorComponent.h"
#include "Perf_WorldStreamingOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_StreamingPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Background  UMETA(DisplayName = "Background")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_StreamingRegion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector Extent = FVector(10000.0f, 10000.0f, 5000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    EPerf_StreamingPriority Priority = EPerf_StreamingPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingDistance = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UnloadDistance = 30000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bForceLoaded = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_StreamingMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 LoadedCells = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 StreamingCells = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float StreamingTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 PendingRequests = 0;
};

/**
 * World Streaming Optimizer - Manages world partition streaming for optimal performance
 * Optimizes streaming distances, priorities, and memory usage for large prehistoric worlds
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_WorldStreamingOptimizer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_WorldStreamingOptimizer();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Main optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance|Streaming")
    void OptimizeWorldStreaming();

    UFUNCTION(BlueprintCallable, Category = "Performance|Streaming")
    void SetStreamingQuality(float QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance|Streaming")
    void UpdateStreamingPriorities(const FVector& PlayerLocation, const FVector& PlayerVelocity);

    // Streaming region management
    UFUNCTION(BlueprintCallable, Category = "Performance|Streaming")
    void AddStreamingRegion(const FPerf_StreamingRegion& Region);

    UFUNCTION(BlueprintCallable, Category = "Performance|Streaming")
    void RemoveStreamingRegion(int32 RegionIndex);

    UFUNCTION(BlueprintCallable, Category = "Performance|Streaming")
    void ClearStreamingRegions();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance|Streaming")
    FPerf_StreamingMetrics GetStreamingMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Streaming")
    float GetStreamingPerformanceScore() const;

    // Adaptive streaming
    UFUNCTION(BlueprintCallable, Category = "Performance|Streaming")
    void EnableAdaptiveStreaming(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance|Streaming")
    void SetTargetFrameRate(float TargetFPS);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Performance|Streaming", CallInEditor)
    void DebugDrawStreamingRegions();

    UFUNCTION(BlueprintCallable, Category = "Performance|Streaming")
    void LogStreamingStatus();

protected:
    // Core streaming parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    float BaseStreamingDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    float MaxStreamingDistance = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    float StreamingDistanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    int32 MaxConcurrentStreamingRequests = 8;

    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB = 4096.0f;

    // Adaptive streaming
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    bool bAdaptiveStreamingEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    float AdaptiveUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    float PerformanceThreshold = 0.8f;

    // Streaming regions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Regions")
    TArray<FPerf_StreamingRegion> StreamingRegions;

private:
    // Internal state
    FTimerHandle AdaptiveTimerHandle;
    FPerf_StreamingMetrics CachedMetrics;
    float LastPerformanceCheck = 0.0f;
    bool bIsOptimizing = false;

    // Internal methods
    void UpdateAdaptiveStreaming();
    void CalculateOptimalStreamingDistance(const FVector& PlayerLocation);
    void OptimizeMemoryUsage();
    void UpdateStreamingSource(const FVector& Location, const FVector& Velocity);
    float CalculateRegionPriority(const FPerf_StreamingRegion& Region, const FVector& PlayerLocation) const;
    void ApplyStreamingOptimizations();
    void MonitorStreamingPerformance();
};