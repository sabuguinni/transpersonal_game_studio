#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Perf_RealTimePerformanceMonitor.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Excellent   UMETA(DisplayName = "Excellent (60+ FPS)"),
    Good        UMETA(DisplayName = "Good (45-59 FPS)"),
    Fair        UMETA(DisplayName = "Fair (30-44 FPS)"),
    Poor        UMETA(DisplayName = "Poor (15-29 FPS)"),
    Critical    UMETA(DisplayName = "Critical (<15 FPS)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

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
    int32 PhysicsActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel PerformanceLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FDateTime Timestamp;

    FPerf_PerformanceSnapshot()
    {
        FrameRate = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        PhysicsActorCount = 0;
        PerformanceLevel = EPerf_PerformanceLevel::Good;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    float TargetFrameRate;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    float MaxPhysicsActors;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    float MaxDrawCalls;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    bool bEnableAdaptiveLOD;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    bool bEnableDynamicCulling;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    float CullingDistance;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    float LODDistanceMultiplier;

    FPerf_OptimizationSettings()
    {
        TargetFrameRate = 60.0f;
        MaxPhysicsActors = 100.0f;
        MaxDrawCalls = 2000.0f;
        bEnableAdaptiveLOD = true;
        bEnableDynamicCulling = true;
        CullingDistance = 10000.0f;
        LODDistanceMultiplier = 1.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceThresholdExceeded, const FPerf_PerformanceSnapshot&, Snapshot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptimizationApplied, const FString&, OptimizationType);

/**
 * Real-time performance monitoring system for the Transpersonal Game
 * Tracks FPS, memory, draw calls, and physics performance
 * Automatically applies optimizations when performance drops
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_RealTimePerformanceMonitor : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_RealTimePerformanceMonitor();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    FPerf_PerformanceSnapshot GetCurrentPerformanceSnapshot();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void SetOptimizationSettings(const FPerf_OptimizationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    FPerf_OptimizationSettings GetOptimizationSettings() const;

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    float GetAverageFrameRate(float TimeWindow = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    bool IsPerformanceCritical();

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void ApplyPerformanceOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void ResetOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void SetAdaptiveLODEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void SetDynamicCullingEnabled(bool bEnabled);

    // Debug and logging
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor", CallInEditor)
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor", CallInEditor)
    void ExportPerformanceData(const FString& FilePath);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Performance Monitor")
    FOnPerformanceThresholdExceeded OnPerformanceThresholdExceeded;

    UPROPERTY(BlueprintAssignable, Category = "Performance Monitor")
    FOnOptimizationApplied OnOptimizationApplied;

protected:
    // Internal monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void ApplyAdaptiveOptimizations();

    // Performance data collection
    float CalculateFrameRate();
    float CalculateGameThreadTime();
    float CalculateRenderThreadTime();
    int32 CountPhysicsActors();
    float GetMemoryUsage();

    // Optimization implementations
    void OptimizeLODDistances();
    void OptimizeCullingDistances();
    void OptimizePhysicsSettings();
    void OptimizeRenderSettings();

private:
    UPROPERTY()
    bool bIsMonitoring;

    UPROPERTY()
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY()
    TArray<FPerf_PerformanceSnapshot> PerformanceHistory;

    UPROPERTY()
    FTimerHandle MonitoringTimerHandle;

    UPROPERTY()
    float MonitoringInterval;

    UPROPERTY()
    int32 MaxHistorySize;

    // Performance tracking
    float LastFrameTime;
    float FrameTimeAccumulator;
    int32 FrameCount;
    bool bOptimizationsApplied;
};