#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "Perf_PhysicsPerformanceIntegrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActivePhysicsBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 CollisionChecksPerFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float AveragePhysicsStepTime;

    FPerf_PhysicsPerformanceMetrics()
    {
        PhysicsFrameTime = 0.0f;
        ActivePhysicsBodies = 0;
        CollisionChecksPerFrame = 0;
        PhysicsMemoryUsage = 0.0f;
        AveragePhysicsStepTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float MaxPhysicsFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    int32 MaxActivePhysicsBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsLODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsLODDistance2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsLODDistance3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnablePhysicsLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnablePhysicsCulling;

    FPerf_PhysicsOptimizationSettings()
    {
        MaxPhysicsFrameTime = 16.67f; // 60fps target
        MaxActivePhysicsBodies = 1000;
        PhysicsLODDistance1 = 5000.0f;
        PhysicsLODDistance2 = 15000.0f;
        PhysicsLODDistance3 = 30000.0f;
        bEnablePhysicsLOD = true;
        bEnablePhysicsCulling = true;
    }
};

/**
 * Advanced physics performance integrator that works with Core_PhysicsWorldManager
 * to optimize physics simulation performance for large-scale prehistoric environments.
 * Monitors physics frame time, manages LOD systems, and optimizes collision detection.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PhysicsPerformanceIntegrator : public AActor
{
    GENERATED_BODY()

public:
    APerf_PhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    FPerf_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float PerformanceUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    bool bEnablePerformanceOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    bool bLogPerformanceMetrics;

private:
    float LastPerformanceUpdate;
    TArray<float> PhysicsFrameTimeHistory;
    TArray<AActor*> TrackedPhysicsActors;
    class ACore_PhysicsWorldManager* PhysicsWorldManager;

public:
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePhysicsPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPhysicsLODLevel(AActor* Actor, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void RegisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UnregisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FPerf_PhysicsPerformanceMetrics GetCurrentPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetOptimizationSettings(const FPerf_PhysicsOptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    bool IsPhysicsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void EnablePhysicsOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void CullDistantPhysicsActors();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePhysicsLODSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    int32 GetOptimalPhysicsBodyCount() const;

protected:
    void CalculatePhysicsFrameTime();
    void CountActivePhysicsBodies();
    void AnalyzePhysicsMemoryUsage();
    void ApplyPhysicsOptimizations();
    float GetDistanceToPlayer(AActor* Actor) const;
    void SetPhysicsSimulationEnabled(AActor* Actor, bool bEnabled);
    void AdjustPhysicsComplexity(AActor* Actor, float Distance);
};