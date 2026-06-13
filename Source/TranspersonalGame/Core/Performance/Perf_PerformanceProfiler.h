#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "SharedTypes.h"
#include "Perf_PerformanceProfiler.generated.h"

DECLARE_STATS_GROUP(TEXT("TranspersonalGame"), STATGROUP_TranspersonalGame, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("Survival Physics Update"), STAT_SurvivalPhysicsUpdate, STATGROUP_TranspersonalGame);
DECLARE_CYCLE_STAT(TEXT("Dinosaur AI Tick"), STAT_DinosaurAITick, STATGROUP_TranspersonalGame);
DECLARE_CYCLE_STAT(TEXT("World Generation"), STAT_WorldGeneration, STATGROUP_TranspersonalGame);
DECLARE_CYCLE_STAT(TEXT("Foliage Rendering"), STAT_FoliageRendering, STATGROUP_TranspersonalGame);
DECLARE_CYCLE_STAT(TEXT("Crowd Simulation"), STAT_CrowdSimulation, STATGROUP_TranspersonalGame);

DECLARE_DWORD_COUNTER_STAT(TEXT("Active Dinosaurs"), STAT_ActiveDinosaurs, STATGROUP_TranspersonalGame);
DECLARE_DWORD_COUNTER_STAT(TEXT("Visible Actors"), STAT_VisibleActors, STATGROUP_TranspersonalGame);
DECLARE_DWORD_COUNTER_STAT(TEXT("Physics Bodies"), STAT_PhysicsBodies, STATGROUP_TranspersonalGame);

DECLARE_FLOAT_COUNTER_STAT(TEXT("Frame Time (ms)"), STAT_FrameTime, STATGROUP_TranspersonalGame);
DECLARE_FLOAT_COUNTER_STAT(TEXT("GPU Time (ms)"), STAT_GPUTime, STATGROUP_TranspersonalGame);
DECLARE_FLOAT_COUNTER_STAT(TEXT("Memory Usage (MB)"), STAT_MemoryUsage, STATGROUP_TranspersonalGame);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsage;

    FPerf_FrameMetrics()
        : FrameTime(0.0f)
        , GPUTime(0.0f)
        , CPUTime(0.0f)
        , DrawCalls(0)
        , Triangles(0)
        , MemoryUsage(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float SurvivalPhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DinosaurAITime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float WorldGenTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FoliageRenderTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CrowdSimTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveDinosaurs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsBodies;

    FPerf_SystemMetrics()
        : SurvivalPhysicsTime(0.0f)
        , DinosaurAITime(0.0f)
        , WorldGenTime(0.0f)
        , FoliageRenderTime(0.0f)
        , CrowdSimTime(0.0f)
        , ActiveDinosaurs(0)
        , VisibleActors(0)
        , PhysicsBodies(0)
    {}
};

UENUM(BlueprintType)
enum class EPerf_ProfilerMode : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Basic           UMETA(DisplayName = "Basic Metrics"),
    Detailed        UMETA(DisplayName = "Detailed Profiling"),
    Development     UMETA(DisplayName = "Development Mode")
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PerformanceProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Profiler Control
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetProfilerMode(EPerf_ProfilerMode NewMode);

    // Metrics Access
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_FrameMetrics GetCurrentFrameMetrics() const { return CurrentFrameMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_SystemMetrics GetCurrentSystemMetrics() const { return CurrentSystemMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetAverageFrameRate() const;

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetMinFrameRate() const { return MinFrameRate; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetMaxFrameRate() const { return MaxFrameRate; }

    // Performance Warnings
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceCritical() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetPerformanceWarnings() const;

    // Debug Display
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableOnScreenDisplay(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

protected:
    // Core profiling data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    EPerf_ProfilerMode ProfilerMode;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bIsProfilingActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FPerf_FrameMetrics CurrentFrameMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FPerf_SystemMetrics CurrentSystemMetrics;

    // Frame rate tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float MinFrameRate;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float MaxFrameRate;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float FrameRateSum;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 FrameCount;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float CriticalFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxGPUTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxMemoryUsage;

    // Display settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    bool bShowOnScreenDisplay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    float UpdateInterval;

private:
    // Internal tracking
    float LastUpdateTime;
    TArray<FString> PerformanceWarnings;
    
    // Console variables
    static TAutoConsoleVariable<int32> CVarProfilerEnabled;
    static TAutoConsoleVariable<int32> CVarShowStats;
    static TAutoConsoleVariable<float> CVarTargetFrameRate;

    // Internal methods
    void UpdateFrameMetrics();
    void UpdateSystemMetrics();
    void CheckPerformanceThresholds();
    void DisplayOnScreenStats();
    void UpdateConsoleVariables();
};