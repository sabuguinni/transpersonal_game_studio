#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Perf_PerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

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
    float UsedMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AvailableMemoryMB;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        UsedMemoryMB = 0.0f;
        AvailableMemoryMB = 0.0f;
    }
};

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Excellent   UMETA(DisplayName = "Excellent (60+ FPS)"),
    Good        UMETA(DisplayName = "Good (45-60 FPS)"),
    Acceptable  UMETA(DisplayName = "Acceptable (30-45 FPS)"),
    Poor        UMETA(DisplayName = "Poor (15-30 FPS)"),
    Critical    UMETA(DisplayName = "Critical (<15 FPS)")
};

/**
 * Performance monitoring system for real-time FPS, memory, and rendering statistics
 * Provides automatic performance profiling and optimization recommendations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PerformanceMonitor : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === PERFORMANCE MONITORING ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_PerformanceLevel CurrentPerformanceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDetailedLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS;

    // === PERFORMANCE THRESHOLDS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float ExcellentFPSThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float GoodFPSThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float AcceptableFPSThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float PoorFPSThreshold;

    // === OPTIMIZATION SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bAutoAdjustLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bAutoAdjustShadowQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bAutoAdjustPostProcessing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float OptimizationCooldown;

private:
    float LastMonitorTime;
    float LastOptimizationTime;
    TArray<float> FPSHistory;
    int32 MaxHistorySize;

public:
    // === PERFORMANCE MONITORING FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS(int32 SampleCount = 60) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    // === OPTIMIZATION FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void TriggerAutomaticOptimization();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeLODSettings();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeShadowSettings();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizePostProcessSettings();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ResetToDefaultSettings();

    // === LOGGING AND REPORTING ===
    
    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void SavePerformanceLog();

    UFUNCTION(BlueprintCallable, Category = "Reporting", CallInEditor)
    void GeneratePerformanceReport();

    // === EDITOR FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Editor", CallInEditor)
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Editor", CallInEditor)
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Editor", CallInEditor)
    void RunPerformanceBenchmark();

protected:
    void CalculatePerformanceLevel();
    void UpdateFPSHistory(float NewFPS);
    bool ShouldOptimize() const;
    void ApplyOptimizationLevel(int32 Level);
    FString GetPerformanceReportString() const;
};