#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"
#include "Perf_RealTimePerformanceTracker.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RealTimeMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsPerformanceGood;

    FPerf_RealTimeMetrics()
    {
        CurrentFPS = 0.0f;
        FrameTimeMS = 0.0f;
        GameThreadTimeMS = 0.0f;
        RenderThreadTimeMS = 0.0f;
        GPUTimeMS = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        ActiveActors = 0;
        bIsPerformanceGood = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_RealTimePerformanceTracker : public AActor
{
    GENERATED_BODY()

public:
    APerf_RealTimePerformanceTracker();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Performance tracking methods
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceTracking();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceTracking();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_RealTimeMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTargets(float TargetFPS, float MaxFrameTimeMS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinTargets() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    // Performance optimization triggers
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void TriggerLODOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void TriggerCullingOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void TriggerMemoryCleanup();

protected:
    // Performance tracking properties
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsTracking;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_RealTimeMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxFrameTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxGPUTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxMemoryUsageMB;

    // Tracking intervals
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MetricsUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float OptimizationCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bAutoOptimize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bLogPerformanceWarnings;

private:
    // Internal tracking
    float LastMetricsUpdateTime;
    float LastOptimizationCheckTime;
    int32 FrameCounter;
    float AccumulatedFrameTime;

    // Performance history for trend analysis
    TArray<float> FPSHistory;
    TArray<float> FrameTimeHistory;
    TArray<float> MemoryHistory;

    static const int32 MaxHistorySize = 60; // 1 second at 60fps

    // Internal methods
    void UpdatePerformanceMetrics();
    void CheckPerformanceTargets();
    void UpdatePerformanceHistory();
    float CalculateAverageFPS() const;
    float CalculateAverageFrameTime() const;
    bool ShouldTriggerOptimization() const;
    void ExecuteAutoOptimization();
};