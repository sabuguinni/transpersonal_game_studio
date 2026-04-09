// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsPerformanceProfiler.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsProfiler, Log, All);

/**
 * Performance data for physics simulation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPhysicsPerformanceData
{
    GENERATED_BODY()

    /** Number of active rigid bodies in simulation */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRigidBodies = 0;

    /** Number of active collision pairs */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveCollisionPairs = 0;

    /** Physics simulation time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float SimulationTimeMs = 0.0f;

    /** Number of physics substeps taken */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SubstepCount = 0;

    /** Memory usage by physics simulation in MB */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    /** Average FPS impact from physics */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsFPSImpact = 0.0f;

    /** Number of sleeping rigid bodies */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SleepingBodies = 0;

    /** Number of kinematic bodies */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 KinematicBodies = 0;
};

/**
 * Physics Performance Profiler Component
 * Monitors and profiles Chaos Physics performance for optimization
 * Tracks simulation metrics, memory usage, and performance bottlenecks
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsPerformanceProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsPerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Start performance profiling */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void StartProfiling();

    /** Stop performance profiling */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void StopProfiling();

    /** Get current performance data */
    UFUNCTION(BlueprintPure, Category = "Physics Profiling")
    FPhysicsPerformanceData GetCurrentPerformanceData() const;

    /** Get average performance data over time window */
    UFUNCTION(BlueprintPure, Category = "Physics Profiling")
    FPhysicsPerformanceData GetAveragePerformanceData() const;

    /** Reset performance statistics */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void ResetStatistics();

    /** Set profiling update frequency in seconds */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void SetProfilingFrequency(float FrequencySeconds);

    /** Enable/disable automatic performance warnings */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void SetPerformanceWarningsEnabled(bool bEnabled);

    /** Get physics memory usage in MB */
    UFUNCTION(BlueprintPure, Category = "Physics Profiling")
    float GetPhysicsMemoryUsage() const;

    /** Get physics simulation bottlenecks */
    UFUNCTION(BlueprintPure, Category = "Physics Profiling")
    TArray<FString> GetPerformanceBottlenecks() const;

    /** Export performance data to CSV file */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    bool ExportPerformanceData(const FString& FilePath) const;

protected:
    /** Whether profiling is currently active */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    bool bIsProfiling = false;

    /** Profiling update frequency */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float ProfilingFrequency = 1.0f;

    /** Enable automatic performance warnings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling")
    bool bPerformanceWarningsEnabled = true;

    /** Maximum allowed simulation time before warning (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling", meta = (ClampMin = "1.0", ClampMax = "50.0"))
    float MaxSimulationTimeMs = 16.67f; // 60 FPS target

    /** Maximum allowed memory usage before warning (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
    float MaxMemoryUsageMB = 100.0f;

    /** Current performance data */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    FPhysicsPerformanceData CurrentPerformanceData;

    /** Historical performance data for averaging */
    UPROPERTY()
    TArray<FPhysicsPerformanceData> PerformanceHistory;

    /** Maximum number of performance samples to keep */
    UPROPERTY(EditAnywhere, Category = "Physics Profiling", meta = (ClampMin = "10", ClampMax = "1000"))
    int32 MaxHistorySamples = 60;

private:
    /** Update performance metrics */
    void UpdatePerformanceMetrics();

    /** Check for performance warnings */
    void CheckPerformanceWarnings();

    /** Get Chaos Physics scene data */
    void GatherChaosPhysicsData();

    /** Calculate memory usage */
    float CalculatePhysicsMemoryUsage() const;

    /** Timer handle for profiling updates */
    FTimerHandle ProfilingTimerHandle;

    /** Last profiling update time */
    double LastProfilingTime = 0.0;

    /** Performance warning cooldown */
    float WarningCooldownTime = 5.0f;
    float LastWarningTime = 0.0f;
};