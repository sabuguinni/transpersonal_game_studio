#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Perf_SystemMonitor.generated.h"

// Performance data structures
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_SystemStats
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
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsObjectCount;

    FPerf_SystemStats()
    {
        CurrentFPS = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        PhysicsObjectCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoOptimize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationCheckInterval;

    FPerf_OptimizationSettings()
    {
        TargetFPS = 60.0f;
        MaxFrameTime = 16.67f; // 60 FPS
        MaxPhysicsObjects = 500;
        MaxDrawCalls = 2000;
        MaxMemoryUsageMB = 4096.0f;
        bAutoOptimize = true;
        OptimizationCheckInterval = 1.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnPerformanceWarning, const FString&, WarningMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnOptimizationApplied, const FString&, OptimizationDescription);

/**
 * System-wide performance monitoring component for the prehistoric world
 * Tracks FPS, memory usage, physics load, and applies automatic optimizations
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_SystemMonitor : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_SystemMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_SystemStats GetCurrentSystemStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinTargets();

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationSettings(const FPerf_OptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_OptimizationSettings GetOptimizationSettings() const { return OptimizationSettings; }

    // Physics optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsObjects();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetPhysicsObjectCount();

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsageMB();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FPerf_OnPerformanceWarning OnPerformanceWarning;

    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FPerf_OnOptimizationApplied OnOptimizationApplied;

protected:
    // Performance data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FPerf_SystemStats CurrentStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float TimeSinceLastOptimization;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    TArray<float> FrameTimeHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxFrameHistorySize;

private:
    void UpdateSystemStats();
    void CheckPerformanceThresholds();
    void OptimizeRenderingSettings();
    void OptimizePhysicsSettings();
    float CalculateAverageFrameTime();
    void LogPerformanceWarning(const FString& Warning);
    void LogOptimizationApplied(const FString& Optimization);
};