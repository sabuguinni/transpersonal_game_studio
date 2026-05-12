#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"
#include "Stats/Stats.h"
#include "TranspersonalGame.h"
#include "Perf_PhysicsWorldPerformanceMonitor.generated.h"

DECLARE_STATS_GROUP(TEXT("Physics World Performance"), STATGROUP_PhysicsWorldPerf, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Physics World Simulation"), STAT_PhysicsWorldSimulation, STATGROUP_PhysicsWorldPerf);
DECLARE_CYCLE_STAT(TEXT("Physics World Update"), STAT_PhysicsWorldUpdate, STATGROUP_PhysicsWorldPerf);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Physics Bodies"), STAT_ActivePhysicsBodies, STATGROUP_PhysicsWorldPerf);
DECLARE_MEMORY_STAT(TEXT("Physics Memory Usage"), STAT_PhysicsMemoryUsage, STATGROUP_PhysicsWorldPerf);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsWorldMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float SimulationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float UpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActiveBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 SleepingBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    bool bIsPerformanceOptimal;

    FPerf_PhysicsWorldMetrics()
    {
        SimulationTime = 0.0f;
        UpdateTime = 0.0f;
        ActiveBodies = 0;
        SleepingBodies = 0;
        MemoryUsageMB = 0.0f;
        AverageFrameTime = 0.0f;
        bIsPerformanceOptimal = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PhysicsWorldPerformanceMonitor : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsWorldPerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FPerf_PhysicsWorldMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    bool IsPerformanceOptimal() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ResetPerformanceMetrics();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Performance")
    void RunPerformanceTest();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float TargetFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MaxPhysicsSimulationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    int32 MaxActivePhysicsBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bAutoOptimize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MonitoringInterval;

private:
    UPROPERTY()
    FPerf_PhysicsWorldMetrics CurrentMetrics;

    UPROPERTY()
    bool bIsMonitoring;

    UPROPERTY()
    float LastMonitoringTime;

    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    TArray<float> SimulationTimeHistory;

    void UpdatePhysicsMetrics();
    void AnalyzePerformance();
    void ApplyOptimizations();
    float CalculateAverageFrameTime() const;
    void LogPerformanceWarnings() const;
    void UpdatePerformanceStats();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PhysicsWorldPerformanceManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_PhysicsWorldPerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void InitializePerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FPerf_PhysicsWorldMetrics GetGlobalPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPerformanceTargets(float TargetFPS, float MaxSimTime, int32 MaxBodies);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Performance")
    void RunGlobalPerformanceAnalysis();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPerf_PhysicsWorldPerformanceMonitor* PerformanceMonitor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bGlobalPerformanceMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalTargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bEnableAdaptiveOptimization;

private:
    void MonitorGlobalPerformance();
    void ApplyGlobalOptimizations();
};