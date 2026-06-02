#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Perf_PhysicsPerformanceMonitor.generated.h"

/**
 * Physics Performance Monitor - Real-time physics performance tracking and optimization
 * Monitors physics simulation performance, memory usage, and frame impact
 * Provides adaptive optimization based on performance thresholds
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsPerformanceMonitor : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsPerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 SimulatingRigidBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsCPUPercent;

    // Performance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxPhysicsFrameTime = 8.33f; // 8.33ms for 120fps physics

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    int32 MaxActivePhysicsActors = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxPhysicsMemoryMB = 256.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxPhysicsCPUPercent = 25.0f;

    // Optimization Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAdaptiveOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float OptimizationCheckInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    EPerf_OptimizationLevel CurrentOptimizationLevel = EPerf_OptimizationLevel::Balanced;

    // Performance Status
    UPROPERTY(BlueprintReadOnly, Category = "Performance Status")
    bool bPhysicsPerformanceGood = true;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Status")
    FString PerformanceStatus = "Good";

    UPROPERTY(BlueprintReadOnly, Category = "Performance Status")
    TArray<FString> PerformanceWarnings;

    // Physics Optimization Functions
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePhysicsMetrics();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void CheckPerformanceThresholds();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ApplyPhysicsOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetOptimizationLevel(EPerf_OptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetPhysicsFrameTimeMS() const { return PhysicsFrameTime; }

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    bool IsPhysicsPerformanceWithinLimits() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizePhysicsActorsByDistance(float MaxDistance = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ReducePhysicsComplexity();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void EnablePhysicsLOD(bool bEnable);

private:
    // Internal tracking
    float LastOptimizationCheck = 0.0f;
    TArray<AActor*> CachedPhysicsActors;
    float FrameTimeAccumulator = 0.0f;
    int32 FrameCount = 0;

    // Performance monitoring helpers
    void GatherPhysicsActors();
    void CalculatePhysicsMemoryUsage();
    void UpdatePerformanceStatus();
    void LogPerformanceMetrics();
};