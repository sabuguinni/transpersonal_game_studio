#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Perf_PhysicsIntegrationMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsPerformanceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 SimulatingBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float CollisionQueryTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 CollisionQueries;

    FPerf_PhysicsPerformanceData()
        : PhysicsFrameTime(0.0f)
        , ActivePhysicsActors(0)
        , SimulatingBodies(0)
        , PhysicsMemoryUsage(0.0f)
        , CollisionQueryTime(0.0f)
        , CollisionQueries(0)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float MaxPhysicsFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    int32 MaxSimulatingBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableAdaptiveLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float LODDistanceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnablePhysicsCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float CullingDistance;

    FPerf_PhysicsOptimizationSettings()
        : MaxPhysicsFrameTime(16.67f) // Target 60fps
        , MaxSimulatingBodies(500)
        , bEnableAdaptiveLOD(true)
        , LODDistanceThreshold(2000.0f)
        , bEnablePhysicsCulling(true)
        , CullingDistance(5000.0f)
    {
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsIntegrationMonitor : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsIntegrationMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FPerf_PhysicsPerformanceData GetCurrentPerformanceData() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    bool IsPerformanceWithinTargets() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ApplyOptimizations();

    // Physics integration validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Integration")
    void ValidatePhysicsIntegration();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Integration")
    void RunPhysicsStressTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Integration")
    void GeneratePerformanceReport();

    // Optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    // Performance data
    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    FPerf_PhysicsPerformanceData CurrentPerformanceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    bool bEnableRealTimeMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float MonitoringUpdateInterval;

private:
    // Internal monitoring state
    bool bIsMonitoring;
    float LastUpdateTime;
    TArray<FPerf_PhysicsPerformanceData> PerformanceHistory;

    // Performance tracking
    void UpdatePerformanceData();
    void CheckPerformanceThresholds();
    void OptimizePhysicsActors();
    void UpdatePhysicsLOD();
    void CullDistantPhysicsActors();

    // Utility functions
    int32 CountActivePhysicsActors() const;
    int32 CountSimulatingBodies() const;
    float CalculatePhysicsMemoryUsage() const;
    void LogPerformanceWarning(const FString& Warning) const;
};