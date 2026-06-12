#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Perf_PerformanceProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUTime = 0.0f;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        MinFPS = 60.0f;
        MaxFPS = 60.0f;
        FrameTime = 16.67f;
        DrawCalls = 0;
        TriangleCount = 0;
        GPUTime = 0.0f;
        CPUTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    float TargetFPS = 60.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    float MinAcceptableFPS = 30.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    bool bAutoOptimize = true;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    bool bEnableLODOptimization = true;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    bool bEnableTextureStreaming = true;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    float ViewDistanceScale = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    int32 ShadowQuality = 3;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    int32 PostProcessQuality = 3;

    FPerf_OptimizationSettings()
    {
        TargetFPS = 60.0f;
        MinAcceptableFPS = 30.0f;
        bAutoOptimize = true;
        bEnableLODOptimization = true;
        bEnableTextureStreaming = true;
        ViewDistanceScale = 1.0f;
        ShadowQuality = 3;
        PostProcessQuality = 3;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PerformanceProfiler : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core profiling functionality
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SavePerformanceLog();

    // Optimization functions
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ApplyOptimizationSettings();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void AutoOptimizeForTarget();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    bool IsPerformanceAcceptable() const;

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ProfilerMesh;

    // Performance data
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    FPerf_OptimizationSettings OptimizationSettings;

    // Profiling state
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsProfiling = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float ProfilingStartTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TotalProfilingTime = 0.0f;

    // FPS tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<float> FPSSamples;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaxSamples = 300; // 5 seconds at 60fps

    // Performance history
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<FPerf_PerformanceMetrics> MetricsHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaxHistoryEntries = 1000;

private:
    // Internal profiling functions
    void UpdatePerformanceMetrics(float DeltaTime);
    void CalculateAverages();
    void CheckPerformanceThresholds();
    void ApplyAutoOptimizations();
    
    // Utility functions
    float GetCurrentFrameTime() const;
    int32 GetCurrentDrawCalls() const;
    int32 GetCurrentTriangleCount() const;
    float GetGPUTime() const;
    float GetCPUTime() const;
    
    // Logging
    void LogMetrics(const FPerf_PerformanceMetrics& Metrics);
    FString FormatMetricsString(const FPerf_PerformanceMetrics& Metrics) const;
};