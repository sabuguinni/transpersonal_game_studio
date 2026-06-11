#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
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
    int32 CollisionChecks;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float RagdollPerformanceCost;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float DestructionPerformanceCost;

    FPerf_PhysicsPerformanceMetrics()
    {
        PhysicsFrameTime = 0.0f;
        ActivePhysicsBodies = 0;
        CollisionChecks = 0;
        RagdollPerformanceCost = 0.0f;
        DestructionPerformanceCost = 0.0f;
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
    bool bEnablePhysicsLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableCollisionOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsLODDistance;

    FPerf_PhysicsOptimizationSettings()
    {
        MaxPhysicsFrameTime = 5.0f; // 5ms budget for physics
        MaxActivePhysicsBodies = 100;
        bEnablePhysicsLOD = true;
        bEnableCollisionOptimization = true;
        PhysicsLODDistance = 2000.0f;
    }
};

/**
 * Physics Performance Integrator - Optimizes physics systems for target framerate
 * Integrates with Core Physics System Manager to ensure 60fps on PC, 30fps on console
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PhysicsPerformanceIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === PHYSICS PERFORMANCE MONITORING ===
    
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FPerf_PhysicsPerformanceMetrics GetCurrentPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePhysicsPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    bool IsPhysicsPerformanceWithinBudget() const;

    // === PHYSICS OPTIMIZATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void ApplyPhysicsLOD();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeCollisionShapes();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void ManageRagdollPerformance();

    // === INTEGRATION WITH CORE SYSTEMS ===
    
    UFUNCTION(BlueprintCallable, Category = "Core Integration")
    void IntegrateWithPhysicsSystemManager();

    UFUNCTION(BlueprintCallable, Category = "Core Integration")
    void RegisterWithPerformanceManager();

    // === PERFORMANCE SCALING ===
    
    UFUNCTION(BlueprintCallable, Category = "Performance Scaling")
    void ScalePhysicsQuality(float QualityScale);

    UFUNCTION(BlueprintCallable, Category = "Performance Scaling")
    void SetPhysicsLODLevel(int32 LODLevel);

protected:
    // === PERFORMANCE METRICS ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    // === PHYSICS TRACKING ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Tracking")
    TArray<AActor*> TrackedPhysicsActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Tracking")
    TArray<AActor*> RagdollActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Tracking")
    float LastPhysicsUpdateTime;

    // === OPTIMIZATION STATE ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Optimization State")
    bool bPhysicsOptimizationActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Optimization State")
    int32 CurrentPhysicsLODLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Optimization State")
    float PhysicsQualityScale;

private:
    // === INTERNAL TRACKING ===
    
    float PhysicsFrameTimeAccumulator;
    int32 PhysicsFrameCount;
    float LastOptimizationTime;
    
    // === HELPER FUNCTIONS ===
    
    void UpdatePhysicsActorTracking();
    void CalculatePhysicsFrameTime();
    void ApplyPhysicsOptimizations();
    bool ShouldOptimizePhysicsActor(AActor* Actor) const;
    float CalculateActorPhysicsCost(AActor* Actor) const;
};