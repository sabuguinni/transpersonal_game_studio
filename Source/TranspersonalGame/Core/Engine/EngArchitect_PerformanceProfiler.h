#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "EngArchitect_PerformanceProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 StreamedLevels;

    FEng_PerformanceMetrics()
    {
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        ActiveActors = 0;
        StreamedLevels = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float TargetFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxTriangles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxMemoryUsageMB;

    FEng_PerformanceThresholds()
    {
        TargetFrameTime = 16.67f; // 60 FPS
        MaxFrameTime = 33.33f;    // 30 FPS minimum
        MaxDrawCalls = 2000;
        MaxTriangles = 1000000;
        MaxMemoryUsageMB = 4096.0f;
    }
};

/**
 * Engine Architect - Performance Profiler
 * Real-time performance monitoring and automatic optimization
 * Tracks frame times, memory usage, draw calls, and system performance
 * Automatically adjusts quality settings to maintain target performance
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngArchitect_PerformanceProfiler : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngArchitect_PerformanceProfiler();

    // USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetAverageMetrics() const { return AverageMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerformanceLevel GetRecommendedPerformanceLevel() const;

    // Profiling Control
    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    bool IsProfilingActive() const { return bProfilingActive; }

    // Thresholds
    UFUNCTION(BlueprintCallable, Category = "Thresholds")
    void SetPerformanceThresholds(const FEng_PerformanceThresholds& NewThresholds);

    UFUNCTION(BlueprintCallable, Category = "Thresholds")
    FEng_PerformanceThresholds GetPerformanceThresholds() const { return Thresholds; }

    // Auto-optimization
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void EnableAutoOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    bool IsAutoOptimizationEnabled() const { return bAutoOptimizationEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void TriggerOptimizationPass();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Reporting")
    FString GeneratePerformanceReport() const;

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void LogPerformanceMetrics() const;

protected:
    // Current Performance Data
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FEng_PerformanceMetrics AverageMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    FEng_PerformanceThresholds Thresholds;

    // Profiling State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bProfilingActive;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bAutoOptimizationEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float ProfilingDuration;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 SampleCount;

    // Performance History
    UPROPERTY()
    TArray<FEng_PerformanceMetrics> MetricsHistory;

    UPROPERTY()
    int32 MaxHistorySize;

    // Optimization State
    UPROPERTY()
    float LastOptimizationTime;

    UPROPERTY()
    float OptimizationCooldown;

    UPROPERTY()
    EPerformanceLevel LastRecommendedLevel;

private:
    // Metric Collection
    void UpdateCurrentMetrics();
    void UpdateAverageMetrics();
    void CollectFrameTimeMetrics();
    void CollectRenderingMetrics();
    void CollectMemoryMetrics();
    void CollectActorMetrics();

    // Performance Analysis
    bool CheckFrameTimeThreshold() const;
    bool CheckMemoryThreshold() const;
    bool CheckRenderingThreshold() const;
    EPerformanceLevel CalculateRecommendedLevel() const;

    // Auto-optimization
    void ProcessAutoOptimization();
    void ApplyPerformanceLevel(EPerformanceLevel Level);
    void OptimizeWorldStreaming();
    void OptimizeRenderingSettings();
    void OptimizeActorCulling();

    // Utility
    void AddToHistory(const FEng_PerformanceMetrics& Metrics);
    float GetMetricWeight(int32 SampleIndex) const;
    void LogOptimizationAction(const FString& Action) const;
};