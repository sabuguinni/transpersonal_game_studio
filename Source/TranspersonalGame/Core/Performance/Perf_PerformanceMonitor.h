#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Perf_PerformanceMonitor.generated.h"

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
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel PerformanceLevel;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        VisibleActors = 0;
        MemoryUsageMB = 0.0f;
        PerformanceLevel = EPerf_PerformanceLevel::Good;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableAutoLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistanceMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bEnableDynamicBatching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnablePhysicsLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float PhysicsLODDistance;

    FPerf_OptimizationSettings()
    {
        bEnableAutoLOD = true;
        LODDistanceMultiplier = 1.0f;
        bEnableOcclusionCulling = true;
        CullingDistance = 5000.0f;
        bEnableDynamicBatching = true;
        MaxDrawCalls = 2000;
        bEnablePhysicsLOD = true;
        PhysicsLODDistance = 1000.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnPerformanceChanged, EPerf_PerformanceLevel, NewLevel);

/**
 * Performance monitoring and optimization subsystem for TranspersonalGame
 * Tracks FPS, frame times, memory usage, and automatically applies optimizations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PerformanceMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PerformanceMonitor();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const;

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ApplyOptimizationSettings(const FPerf_OptimizationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void EnableAutoOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeForCurrentPerformance();

    // Performance targets
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFPS() const { return TargetFPS; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FPerf_OnPerformanceChanged OnPerformanceChanged;

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void ShowPerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void TogglePerformanceHUD();

protected:
    // Internal monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceLevel();
    void ApplyAutoOptimizations();

    // Optimization methods
    void OptimizeLOD();
    void OptimizeCulling();
    void OptimizeRendering();
    void OptimizePhysics();

private:
    UPROPERTY()
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY()
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY()
    float TargetFPS;

    UPROPERTY()
    bool bMonitoringActive;

    UPROPERTY()
    bool bAutoOptimizationEnabled;

    UPROPERTY()
    bool bPerformanceHUDVisible;

    // Timing
    FTimerHandle MonitoringTimerHandle;
    float MonitoringInterval;

    // Performance history
    TArray<float> FPSHistory;
    TArray<float> FrameTimeHistory;
    int32 MaxHistorySize;

    // Performance thresholds
    float ExcellentFPSThreshold;
    float GoodFPSThreshold;
    float FairFPSThreshold;
    float PoorFPSThreshold;
};