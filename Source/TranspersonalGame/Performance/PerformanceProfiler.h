#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "PerformanceTargets.h"
#include "PerformanceProfiler.generated.h"

/**
 * Real-time performance monitoring and profiling system
 * Tracks frame times, memory usage, and system-specific metrics
 * Integrates with Unreal Insights for detailed analysis
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerformanceThresholdExceeded, FString, SystemName, float, ActualTime);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    // Frame timing
    UPROPERTY(BlueprintReadOnly, Category = "Frame Timing")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Frame Timing")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Frame Timing")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Frame Timing")
    float GPUTime = 0.0f;

    // System-specific timings
    UPROPERTY(BlueprintReadOnly, Category = "System Performance")
    float MassAITime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "System Performance")
    float PhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "System Performance")
    float RenderingTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "System Performance")
    float AudioTime = 0.0f;

    // Memory usage (MB)
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 TotalMemoryUsed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 TextureMemoryUsed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 MeshMemoryUsed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 AudioMemoryUsed = 0;

    // Rendering stats
    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 VisiblePrimitives = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 CulledPrimitives = 0;

    // Mass AI stats (critical for dinosaur ecosystem)
    UPROPERTY(BlueprintReadOnly, Category = "Mass AI")
    int32 ActiveMassAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Mass AI")
    int32 ActiveBehaviorTrees = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Mass AI")
    int32 PerceptionQueries = 0;

    // Streaming stats
    UPROPERTY(BlueprintReadOnly, Category = "Streaming")
    int32 LoadedWorldPartitionCells = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Streaming")
    float TextureStreamingPoolUsage = 0.0f;

    FPerformanceMetrics()
    {
        // Initialize all values to 0
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceProfiler : public UObject
{
    GENERATED_BODY()

public:
    UPerformanceProfiler();

    // Initialize profiler with target platform
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void Initialize(EPerformanceTarget Target);

    // Update performance metrics (called every frame)
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateMetrics();

    // Get current performance metrics
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    // Get performance budget for current target
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceBudget GetCurrentBudget() const { return CurrentBudget; }

    // Check if performance is within budget
    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsPerformanceWithinBudget() const;

    // Get performance health (0.0 = over budget, 1.0 = well within budget)
    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetPerformanceHealth() const;

    // Start/stop detailed profiling session
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfilingSession(const FString& SessionName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfilingSession();

    // Event fired when performance threshold is exceeded
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceThresholdExceeded OnPerformanceThresholdExceeded;

    // Console commands for runtime performance tuning
    UFUNCTION(Exec, Category = "Performance")
    void SetPerformanceTarget(int32 TargetIndex);

    UFUNCTION(Exec, Category = "Performance")
    void ShowPerformanceStats(bool bShow);

    UFUNCTION(Exec, Category = "Performance")
    void DumpPerformanceReport();

protected:
    UPROPERTY()
    EPerformanceTarget CurrentTarget;

    UPROPERTY()
    FPerformanceBudget CurrentBudget;

    UPROPERTY()
    FPerformanceMetrics CurrentMetrics;

    UPROPERTY()
    bool bIsProfilingActive;

    UPROPERTY()
    FString CurrentSessionName;

    // Performance history for trend analysis
    UPROPERTY()
    TArray<FPerformanceMetrics> MetricsHistory;

    static constexpr int32 MAX_HISTORY_SAMPLES = 300; // 5 seconds at 60fps

    // Internal methods
    void CollectFrameTimingMetrics();
    void CollectMemoryMetrics();
    void CollectRenderingMetrics();
    void CollectMassAIMetrics();
    void CollectStreamingMetrics();
    void CheckPerformanceThresholds();
    void LogPerformanceWarning(const FString& SystemName, float ActualTime, float BudgetTime);

private:
    // Cached stat values to avoid constant lookups
    float CachedFrameTime;
    float CachedGameThreadTime;
    float CachedRenderThreadTime;
    float CachedGPUTime;
    
    // Timing helpers
    double LastUpdateTime;
    int32 FrameCounter;
};

// Global performance profiler instance
UCLASS()
class TRANSPERSONALGAME_API UPerformanceProfilerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintPure, Category = "Performance", meta = (CallInEditor = "true"))
    static UPerformanceProfiler* GetPerformanceProfiler();

    UPROPERTY()
    UPerformanceProfiler* ProfilerInstance;
};