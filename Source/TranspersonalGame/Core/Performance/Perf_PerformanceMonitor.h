#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
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
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedPhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedVirtualMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 Triangles;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        UsedPhysicalMemoryMB = 0.0f;
        UsedVirtualMemoryMB = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
    }
};

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Excellent   UMETA(DisplayName = "Excellent (60+ FPS)"),
    Good        UMETA(DisplayName = "Good (45-60 FPS)"),
    Fair        UMETA(DisplayName = "Fair (30-45 FPS)"),
    Poor        UMETA(DisplayName = "Poor (15-30 FPS)"),
    Critical    UMETA(DisplayName = "Critical (<15 FPS)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    bool bEnableDynamicLOD;

    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    float LODBias;

    UPROPERTY(BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling;

    UPROPERTY(BlueprintReadWrite, Category = "Culling")
    float CullingDistance;

    UPROPERTY(BlueprintReadWrite, Category = "Shadows")
    bool bEnableDynamicShadows;

    UPROPERTY(BlueprintReadWrite, Category = "Shadows")
    int32 ShadowQuality;

    UPROPERTY(BlueprintReadWrite, Category = "Effects")
    bool bEnableParticleEffects;

    UPROPERTY(BlueprintReadWrite, Category = "Effects")
    float EffectQuality;

    FPerf_OptimizationSettings()
    {
        bEnableDynamicLOD = true;
        LODBias = 0.0f;
        bEnableOcclusionCulling = true;
        CullingDistance = 10000.0f;
        bEnableDynamicShadows = true;
        ShadowQuality = 3;
        bEnableParticleEffects = true;
        EffectQuality = 1.0f;
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

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    // Optimization
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ApplyOptimizationSettings(const FPerf_OptimizationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void AutoOptimizeForTargetFPS(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeLODForDistance(float ViewDistance);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeMemoryUsage();

    // Memory Profiling
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void StartMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void StopMemoryProfiling();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Memory")
    float GetMemoryUsageMB() const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForceGarbageCollection();

    // Performance Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceChanged, EPerf_PerformanceLevel, NewLevel);
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceChanged OnPerformanceChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLowPerformance, float, CurrentFPS);
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnLowPerformance OnLowPerformance;

protected:
    // Core monitoring data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float LowPerformanceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MetricsHistorySize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoOptimize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bLogPerformanceWarnings;

private:
    // Internal monitoring state
    bool bIsMonitoring;
    bool bIsMemoryProfiling;
    float MonitoringTimer;
    float TotalFrameTime;
    int32 FrameCount;
    
    // Performance history for statistical analysis
    TArray<float> FPSHistory;
    TArray<float> FrameTimeHistory;
    TArray<float> MemoryHistory;
    
    // Performance tracking
    EPerf_PerformanceLevel LastPerformanceLevel;
    float LastOptimizationTime;
    
    // Internal methods
    void UpdatePerformanceMetrics(float DeltaTime);
    void UpdateMemoryMetrics();
    void CheckPerformanceThresholds();
    void ApplyAutoOptimizations();
    float CalculateAverageFPS() const;
    float CalculateMinFPS() const;
    float CalculateMaxFPS() const;
    void AddToHistory(TArray<float>& History, float Value);
};