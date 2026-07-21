#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/Platform.h"
#include "SharedTypes.h"
#include "Perf_PhysicsProfilerIntegrator.generated.h"

// Forward declarations
class UWorld;
class AActor;
class UPrimitiveComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsProfileMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    float PhysicsTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    float CollisionDetectionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    float RigidBodySimulationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    float ConstraintSolverTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    int32 ActiveCollisionPairs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    int32 PhysicsMemoryUsage = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    bool bIsPhysicsPerformanceGood = true;

    FPerf_PhysicsProfileMetrics()
    {
        PhysicsTickTime = 0.0f;
        CollisionDetectionTime = 0.0f;
        RigidBodySimulationTime = 0.0f;
        ConstraintSolverTime = 0.0f;
        ActiveRigidBodies = 0;
        ActiveCollisionPairs = 0;
        PhysicsMemoryUsage = 0;
        AverageFrameTime = 0.0f;
        bIsPhysicsPerformanceGood = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float MaxPhysicsTickTime = 16.67f; // Target: 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    int32 MaxActiveRigidBodies = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    int32 MaxCollisionPairs = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableAdaptiveLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsCullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableAsyncPhysics = true;

    FPerf_PhysicsOptimizationSettings()
    {
        MaxPhysicsTickTime = 16.67f;
        MaxActiveRigidBodies = 500;
        MaxCollisionPairs = 1000;
        bEnableAdaptiveLOD = true;
        bEnableDistanceCulling = true;
        PhysicsCullingDistance = 5000.0f;
        bEnableAsyncPhysics = true;
    }
};

/**
 * Physics Profiler Integrator for Performance Optimization
 * Integrates with Core Systems Programmer's physics profiler to provide
 * real-time optimization based on physics performance metrics
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance))
class TRANSPERSONALGAME_API UPerf_PhysicsProfilerIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsProfilerIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core profiling functions
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void StartPhysicsProfiling();

    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void StopPhysicsProfiling();

    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    FPerf_PhysicsProfileMetrics GetCurrentPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void UpdatePhysicsOptimizationSettings(const FPerf_PhysicsOptimizationSettings& NewSettings);

    // Integration with Core Systems
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void IntegrateWithCorePhysicsProfiler();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    bool ValidatePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void OptimizePhysicsBasedOnMetrics();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void MonitorPhysicsPerformance(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void ApplyPerformanceOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void ResetPhysicsOptimizations();

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Physics Profiling")
    void TestPhysicsProfilerIntegration();

    UFUNCTION(CallInEditor, Category = "Physics Profiling")
    void ExportPhysicsProfileData();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling", meta = (AllowPrivateAccess = "true"))
    bool bIsProfilingActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling", meta = (AllowPrivateAccess = "true"))
    FPerf_PhysicsProfileMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling", meta = (AllowPrivateAccess = "true"))
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling", meta = (AllowPrivateAccess = "true"))
    float ProfilingUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiling", meta = (AllowPrivateAccess = "true"))
    float LastProfilingUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration", meta = (AllowPrivateAccess = "true"))
    bool bIntegratedWithCoreProfiler;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring", meta = (AllowPrivateAccess = "true"))
    TArray<float> PhysicsTickTimeHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring", meta = (AllowPrivateAccess = "true"))
    int32 MaxHistorySize;

private:
    // Internal profiling functions
    void UpdatePhysicsMetrics();
    void CalculateAverageFrameTime();
    void CheckPerformanceThresholds();
    void ApplyDistanceCulling();
    void ApplyLODOptimizations();
    void ManageRigidBodyCount();
    
    // Integration helpers
    void FindCorePhysicsProfiler();
    void SynchronizeWithCoreProfiler();
    void ValidateIntegration();

    // Performance tracking
    float AccumulatedPhysicsTime;
    int32 FrameCount;
    bool bPerformanceWarningIssued;
};