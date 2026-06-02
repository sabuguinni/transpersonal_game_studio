#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Core_PhysicsProfiler.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsProfiler, Log, All);

/**
 * Performance metrics for physics profiling
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsProfileMetrics
{
    GENERATED_BODY()

    /** Total physics simulation time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    float PhysicsSimulationTime;

    /** Collision detection time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    float CollisionDetectionTime;

    /** Constraint solving time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    float ConstraintSolvingTime;

    /** Number of active rigid bodies */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    int32 ActiveRigidBodies;

    /** Number of active constraints */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    int32 ActiveConstraints;

    /** Number of collision pairs processed */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    int32 CollisionPairs;

    /** Memory usage by physics system in MB */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    float PhysicsMemoryUsage;

    /** Frame time impact of physics (percentage) */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    float PhysicsFrameImpact;

    FCore_PhysicsProfileMetrics()
    {
        PhysicsSimulationTime = 0.0f;
        CollisionDetectionTime = 0.0f;
        ConstraintSolvingTime = 0.0f;
        ActiveRigidBodies = 0;
        ActiveConstraints = 0;
        CollisionPairs = 0;
        PhysicsMemoryUsage = 0.0f;
        PhysicsFrameImpact = 0.0f;
    }
};

/**
 * Performance history for tracking trends
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsProfileHistory
{
    GENERATED_BODY()

    /** Historical performance samples */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    TArray<FCore_PhysicsProfileMetrics> HistorySamples;

    /** Maximum number of samples to keep */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling")
    int32 MaxHistorySamples;

    /** Current sample index */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    int32 CurrentSampleIndex;

    FCore_PhysicsProfileHistory()
    {
        MaxHistorySamples = 300; // 5 minutes at 60fps
        CurrentSampleIndex = 0;
    }
};

/**
 * Profiling settings and thresholds
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsProfileSettings
{
    GENERATED_BODY()

    /** Enable detailed profiling (impacts performance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling")
    bool bEnableDetailedProfiling;

    /** Enable memory tracking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling")
    bool bEnableMemoryTracking;

    /** Enable performance warnings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling")
    bool bEnablePerformanceWarnings;

    /** Warning threshold for physics time (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling")
    float PhysicsTimeWarningThreshold;

    /** Critical threshold for physics time (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling")
    float PhysicsTimeCriticalThreshold;

    /** Maximum allowed rigid bodies before warning */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling")
    int32 MaxRigidBodiesWarning;

    /** Profiling update frequency (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling")
    float ProfilingUpdateFrequency;

    FCore_PhysicsProfileSettings()
    {
        bEnableDetailedProfiling = true;
        bEnableMemoryTracking = true;
        bEnablePerformanceWarnings = true;
        PhysicsTimeWarningThreshold = 5.0f;
        PhysicsTimeCriticalThreshold = 10.0f;
        MaxRigidBodiesWarning = 500;
        ProfilingUpdateFrequency = 0.1f;
    }
};

/**
 * Physics Profiler Component
 * Provides comprehensive performance monitoring and profiling for the physics system
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Current performance metrics */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    FCore_PhysicsProfileMetrics CurrentMetrics;

    /** Performance history */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    FCore_PhysicsProfileHistory PerformanceHistory;

    /** Profiling settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling")
    FCore_PhysicsProfileSettings ProfileSettings;

    /** Start profiling session */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void StartProfiling();

    /** Stop profiling session */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void StopProfiling();

    /** Reset profiling data */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void ResetProfilingData();

    /** Get average performance over time window */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    FCore_PhysicsProfileMetrics GetAveragePerformance(float TimeWindowSeconds = 1.0f);

    /** Get performance trend (positive = improving, negative = degrading) */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    float GetPerformanceTrend(float TimeWindowSeconds = 5.0f);

    /** Check if performance is within acceptable limits */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    bool IsPerformanceAcceptable();

    /** Generate performance report */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    FString GeneratePerformanceReport();

    /** Export profiling data to file */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling", CallInEditor = true)
    void ExportProfilingData(const FString& FilePath = TEXT(""));

private:
    /** Update performance metrics */
    void UpdateMetrics();

    /** Collect physics simulation metrics */
    void CollectSimulationMetrics();

    /** Collect memory usage metrics */
    void CollectMemoryMetrics();

    /** Add sample to history */
    void AddSampleToHistory(const FCore_PhysicsProfileMetrics& Metrics);

    /** Check performance thresholds */
    void CheckPerformanceThresholds();

    /** Log performance warning */
    void LogPerformanceWarning(const FString& Warning);

    /** Get physics world */
    class UWorld* GetPhysicsWorld() const;

    /** Calculate frame impact percentage */
    float CalculateFrameImpact(float PhysicsTime, float FrameTime) const;

private:
    /** Is profiling currently active */
    bool bIsProfilingActive;

    /** Last profiling update time */
    float LastProfilingTime;

    /** Cached world reference */
    UPROPERTY()
    class UWorld* CachedWorld;

    /** Frame time accumulator for averaging */
    float FrameTimeAccumulator;

    /** Number of frames accumulated */
    int32 AccumulatedFrames;

    /** Last warning time to prevent spam */
    float LastWarningTime;
};