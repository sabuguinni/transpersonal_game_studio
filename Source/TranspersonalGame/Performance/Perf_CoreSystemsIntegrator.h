#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Perf_CoreSystemsIntegrator.generated.h"

UENUM(BlueprintType)
enum class EPerf_CoreSystemPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CoreSystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems")
    float PhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems")
    float RigidBodyMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems")
    int32 VehicleSystemsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems")
    float DestructionSystemLoad;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems")
    float FluidSimulationLoad;

    FPerf_CoreSystemMetrics()
    {
        PhysicsFrameTime = 0.0f;
        ActivePhysicsActors = 0;
        RigidBodyMemoryUsage = 0.0f;
        VehicleSystemsActive = 0;
        DestructionSystemLoad = 0.0f;
        FluidSimulationLoad = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CoreOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    EPerf_CoreSystemPriority PhysicsSystemPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsTickRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    int32 MaxSimulatingBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Optimization")
    EPerf_CoreSystemPriority VehicleSystemPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Optimization")
    int32 MaxActiveVehicles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Optimization")
    EPerf_CoreSystemPriority DestructionSystemPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Optimization")
    int32 MaxDestructibleActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Optimization")
    EPerf_CoreSystemPriority FluidSystemPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Optimization")
    float FluidSimulationQuality;

    FPerf_CoreOptimizationSettings()
    {
        PhysicsSystemPriority = EPerf_CoreSystemPriority::High;
        PhysicsTickRate = 60.0f;
        MaxSimulatingBodies = 500;
        VehicleSystemPriority = EPerf_CoreSystemPriority::Medium;
        MaxActiveVehicles = 10;
        DestructionSystemPriority = EPerf_CoreSystemPriority::Medium;
        MaxDestructibleActors = 100;
        FluidSystemPriority = EPerf_CoreSystemPriority::Low;
        FluidSimulationQuality = 0.7f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_CoreSystemsIntegrator : public AActor
{
    GENERATED_BODY()

public:
    APerf_CoreSystemsIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Integration")
    FPerf_CoreOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems Metrics")
    FPerf_CoreSystemMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float MetricsUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    bool bEnableAutomaticOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float TargetFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float CriticalFrameTime;

private:
    float LastMetricsUpdate;
    float AccumulatedFrameTime;
    int32 FrameTimesamples;

public:
    UFUNCTION(BlueprintCallable, Category = "Core Systems Integration")
    void UpdateCoreSystemMetrics();

    UFUNCTION(BlueprintCallable, Category = "Core Systems Integration")
    void OptimizePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Core Systems Integration")
    void OptimizeVehicleSystem();

    UFUNCTION(BlueprintCallable, Category = "Core Systems Integration")
    void OptimizeDestructionSystem();

    UFUNCTION(BlueprintCallable, Category = "Core Systems Integration")
    void OptimizeFluidSystem();

    UFUNCTION(BlueprintCallable, Category = "Core Systems Integration")
    void ApplyOptimizationSettings(const FPerf_CoreOptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Core Systems Integration")
    FPerf_CoreSystemMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Core Systems Integration")
    void SetSystemPriority(EPerf_CoreSystemPriority NewPriority, const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    bool IsPerformanceWithinTargets() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    float GetOverallPerformanceScore() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Core Systems Events")
    void OnPerformanceThresholdExceeded(float CurrentFrameTime);

    UFUNCTION(BlueprintImplementableEvent, Category = "Core Systems Events")
    void OnOptimizationApplied(const FString& SystemName, EPerf_CoreSystemPriority NewPriority);
};