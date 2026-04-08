// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "Stats/Stats.h"
#include "HAL/IConsoleManager.h"
#include "Misc/App.h"
#include "PerformanceOptimizer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPerformanceOptimizer, Log, All);

DECLARE_STATS_GROUP(TEXT("TranspersonalGame Performance"), STATGROUP_TranspersonalPerformance, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Physics Tick Time"), STAT_PhysicsTickTime, STATGROUP_TranspersonalPerformance);
DECLARE_CYCLE_STAT(TEXT("AI Tick Time"), STAT_AITickTime, STATGROUP_TranspersonalPerformance);
DECLARE_CYCLE_STAT(TEXT("Rendering Time"), STAT_RenderingTime, STATGROUP_TranspersonalPerformance);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Physics Bodies"), STAT_ActivePhysicsBodies, STATGROUP_TranspersonalPerformance);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active AI Agents"), STAT_ActiveAIAgents, STATGROUP_TranspersonalPerformance);
DECLARE_DWORD_COUNTER_STAT(TEXT("Visible Dinosaurs"), STAT_VisibleDinosaurs, STATGROUP_TranspersonalPerformance);

/**
 * Performance Quality Level
 */
UENUM(BlueprintType)
enum class EPerformanceQualityLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"), 
    High        UMETA(DisplayName = "High"),
    Epic        UMETA(DisplayName = "Epic"),
    Custom      UMETA(DisplayName = "Custom")
};

/**
 * Platform Performance Profile
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPlatformPerformanceProfile
{
    GENERATED_BODY()

    /** Target FPS for this platform */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TargetFPS = 60;

    /** Physics tick time budget in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsTickBudget = 4.17f;

    /** Maximum active physics bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActivePhysicsBodies = 500;

    /** Maximum collision checks per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxCollisionChecksPerFrame = 10000;

    /** Maximum AI agents to simulate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxAIAgents = 100;

    /** Maximum visible dinosaurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleDinosaurs = 50;

    /** LOD distance multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceMultiplier = 1.0f;

    /** Memory budget in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MemoryBudgetMB = 8192;

    FPlatformPerformanceProfile()
    {
        TargetFPS = 60;
        PhysicsTickBudget = 4.17f;
        MaxActivePhysicsBodies = 500;
        MaxCollisionChecksPerFrame = 10000;
        MaxAIAgents = 100;
        MaxVisibleDinosaurs = 50;
        LODDistanceMultiplier = 1.0f;
        MemoryBudgetMB = 8192;
    }
};

/**
 * Performance Metrics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    /** Current FPS */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CurrentFPS = 0.0f;

    /** Frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float FrameTimeMS = 0.0f;

    /** Game thread time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float GameThreadTimeMS = 0.0f;

    /** Render thread time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float RenderThreadTimeMS = 0.0f;

    /** GPU time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float GPUTimeMS = 0.0f;

    /** Physics time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float PhysicsTimeMS = 0.0f;

    /** AI time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AITimeMS = 0.0f;

    /** Memory usage in MB */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB = 0.0f;

    /** Active physics bodies count */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActivePhysicsBodies = 0;

    /** Active AI agents count */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActiveAIAgents = 0;

    /** Visible dinosaurs count */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 VisibleDinosaurs = 0;

    /** Draw calls count */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DrawCalls = 0;

    /** Triangles rendered */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TrianglesRendered = 0;
};

/**
 * Performance Optimizer
 * Manages performance optimization for the Transpersonal Game
 * Targets: 60fps on PC, 30fps on Console with large-scale dinosaur simulation
 */
UCLASS()
class TRANSPERSONALGAME_API UPerformanceOptimizer : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Get the performance optimizer instance */
    UFUNCTION(BlueprintPure, Category = "Performance")
    static UPerformanceOptimizer* Get(const UObject* WorldContext);

    /** Initialize performance optimization */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void InitializePerformanceOptimization();

    /** Set performance quality level */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceQualityLevel(EPerformanceQualityLevel QualityLevel);

    /** Get current performance quality level */
    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerformanceQualityLevel GetPerformanceQualityLevel() const { return CurrentQualityLevel; }

    /** Set platform performance profile */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPlatformPerformanceProfile(const FPlatformPerformanceProfile& Profile);

    /** Get platform performance profile */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPlatformPerformanceProfile GetPlatformPerformanceProfile() const { return PlatformProfile; }

    /** Get current performance metrics */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceMetrics GetPerformanceMetrics() const { return CurrentMetrics; }

    /** Enable/disable automatic performance scaling */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAutoPerformanceScaling(bool bEnabled);

    /** Check if performance target is being met */
    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    /** Get performance bottleneck information */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FString GetPerformanceBottleneck() const;

    /** Force performance optimization pass */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForcePerformanceOptimization();

    /** Set LOD bias for performance */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODBias(float Bias);

    /** Set physics quality level */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPhysicsQuality(int32 QualityLevel);

    /** Set AI quality level */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAIQuality(int32 QualityLevel);

    /** Set rendering quality level */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetRenderingQuality(int32 QualityLevel);

    /** Enable performance profiling */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceProfiling(bool bEnabled);

    /** Start performance capture */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceCapture(const FString& CaptureName);

    /** Stop performance capture */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceCapture();

protected:
    /** Current performance quality level */
    UPROPERTY()
    EPerformanceQualityLevel CurrentQualityLevel = EPerformanceQualityLevel::High;

    /** Platform performance profile */
    UPROPERTY()
    FPlatformPerformanceProfile PlatformProfile;

    /** Current performance metrics */
    UPROPERTY()
    FPerformanceMetrics CurrentMetrics;

    /** Auto performance scaling enabled */
    UPROPERTY()
    bool bAutoPerformanceScaling = true;

    /** Performance profiling enabled */
    UPROPERTY()
    bool bPerformanceProfilingEnabled = false;

    /** Performance capture active */
    UPROPERTY()
    bool bPerformanceCaptureActive = false;

    /** Current capture name */
    UPROPERTY()
    FString CurrentCaptureName;

    /** Performance update frequency */
    UPROPERTY()
    float PerformanceUpdateFrequency = 1.0f;

    /** Last performance update time */
    UPROPERTY()
    float LastPerformanceUpdateTime = 0.0f;

    /** Performance history for averaging */
    UPROPERTY()
    TArray<float> FPSHistory;

    /** Maximum history samples */
    UPROPERTY()
    int32 MaxHistorySamples = 60;

private:
    void UpdatePerformanceMetrics();
    void ApplyPerformanceOptimizations();
    void DetectPerformanceBottlenecks();
    void ApplyQualitySettings();
    void ConfigurePlatformSettings();
    void UpdateLODSettings();
    void UpdatePhysicsSettings();
    void UpdateAISettings();
    void UpdateRenderingSettings();
    void LogPerformanceMetrics();

    FTimerHandle PerformanceUpdateTimer;
    FTimerHandle OptimizationTimer;
};

/**
 * Performance Monitor Component
 * Monitors performance for individual actors and systems
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerformanceMonitorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceMonitorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Enable/disable performance monitoring */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void SetMonitoringEnabled(bool bEnabled);

    /** Get performance cost of this actor */
    UFUNCTION(BlueprintPure, Category = "Performance Monitor")
    float GetPerformanceCost() const { return CurrentPerformanceCost; }

    /** Get performance impact category */
    UFUNCTION(BlueprintPure, Category = "Performance Monitor")
    FString GetPerformanceImpactCategory() const;

    /** Set performance budget for this actor */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void SetPerformanceBudget(float Budget) { PerformanceBudget = Budget; }

    /** Check if actor is within performance budget */
    UFUNCTION(BlueprintPure, Category = "Performance Monitor")
    bool IsWithinPerformanceBudget() const;

protected:
    /** Performance monitoring enabled */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Monitor")
    bool bMonitoringEnabled = true;

    /** Performance budget for this actor */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Monitor")
    float PerformanceBudget = 1.0f;

    /** Current performance cost */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitor")
    float CurrentPerformanceCost = 0.0f;

    /** Performance monitoring frequency */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Monitor")
    float MonitoringFrequency = 0.5f;

private:
    void MeasurePerformanceCost();
    void RegisterWithOptimizer();
    void UnregisterFromOptimizer();

    float LastMonitoringTime = 0.0f;
    float AccumulatedCost = 0.0f;
    int32 SampleCount = 0;
};