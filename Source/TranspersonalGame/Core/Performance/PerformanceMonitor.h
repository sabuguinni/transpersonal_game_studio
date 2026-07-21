#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "SharedTypes.h"
#include "PerformanceMonitor.generated.h"

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
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActorCount;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        DrawCalls = 0;
        MemoryUsageMB = 0.0f;
        ActorCount = 0;
        VisibleActorCount = 0;
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
 * Performance monitoring actor that tracks FPS, memory usage, and rendering metrics
 * Provides real-time performance feedback and optimization suggestions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerformanceMonitor : public AActor
{
    GENERATED_BODY()

public:
    APerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // === PERFORMANCE MONITORING ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    EPerf_PerformanceLevel PerformanceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableDetailedLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableAutoOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float TargetFPS;

    // === PERFORMANCE THRESHOLDS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float CriticalFPSThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float PoorFPSThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float AcceptableFPSThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float GoodFPSThreshold;

    // === OPTIMIZATION CONTROLS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization")
    bool bEnableLODOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization")
    bool bEnableCullingOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization")
    bool bEnableShadowOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization")
    float OptimizationCooldown;

    // === PERFORMANCE FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance Events")
    void OnPerformanceLevelChanged(EPerf_PerformanceLevel NewLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance Events")
    void OnCriticalPerformance(float CurrentFPS);

private:
    // === INTERNAL TRACKING ===
    
    float LastMonitorTime;
    float LastOptimizationTime;
    TArray<float> FPSSamples;
    int32 SampleCount;
    bool bIsMonitoring;
    
    // === INTERNAL FUNCTIONS ===
    
    void UpdatePerformanceMetrics();
    void CalculateAverages();
    EPerf_PerformanceLevel DeterminePerformanceLevel(float FPS) const;
    void CheckForOptimizations();
    void OptimizeLOD();
    void OptimizeCulling();
    void OptimizeShadows();
    void WritePerformanceLog();
};