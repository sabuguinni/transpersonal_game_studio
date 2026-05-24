#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Perf_PerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS;

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
    int32 VisibleActors;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        ActiveActors = 0;
        VisibleActors = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MinAcceptableFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAutomaticLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableOcclusionCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableFrustumCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float LODDistanceScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxMemoryUsageMB;

    FPerf_OptimizationSettings()
    {
        TargetFPS = 60.0f;
        MinAcceptableFPS = 30.0f;
        bEnableAutomaticLOD = true;
        bEnableOcclusionCulling = true;
        bEnableFrustumCulling = true;
        LODDistanceScale = 1.0f;
        MaxDrawCalls = 2000;
        MaxMemoryUsageMB = 4096.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PerformanceMonitor : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitor")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor")
    bool bEnableLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor")
    bool bEnableAutomaticOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor")
    bool bShowDebugInfo;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void ApplyOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void SetOptimizationLevel(int32 Level);

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance Monitor")
    void OnPerformanceThresholdExceeded(const FPerf_PerformanceMetrics& Metrics);

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance Monitor")
    void OnOptimizationApplied(int32 OptimizationLevel);

private:
    FTimerHandle MonitoringTimerHandle;
    TArray<float> FPSHistory;
    float TotalFPSSum;
    int32 FPSMeasurements;
    float LastMonitorTime;

    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void LogPerformanceData();
    void ApplyAutomaticOptimizations();
    float CalculateAverageFPS() const;
    void UpdateFPSHistory(float NewFPS);
};