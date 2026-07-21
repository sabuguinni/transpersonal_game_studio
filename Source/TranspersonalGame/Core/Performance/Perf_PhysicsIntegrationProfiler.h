#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Perf_PhysicsIntegrationProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsProfileData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActivePhysicsBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsTickTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 CollisionChecksPerFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float MemoryUsageMB;

    FPerf_PhysicsProfileData()
        : AverageFrameTime(0.0f)
        , ActivePhysicsBodies(0)
        , PhysicsTickTime(0.0f)
        , CollisionChecksPerFrame(0)
        , MemoryUsageMB(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    int32 MaxPhysicsBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsTickRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableAdaptiveLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float LODDistanceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnablePhysicsCulling;

    FPerf_PhysicsOptimizationSettings()
        : MaxPhysicsBodies(500)
        , PhysicsTickRate(60.0f)
        , bEnableAdaptiveLOD(true)
        , LODDistanceThreshold(2000.0f)
        , bEnablePhysicsCulling(true)
    {}
};

/**
 * Physics Integration Profiler - Monitors physics performance after Core Systems integration
 * Tracks physics body count, collision detection overhead, and memory usage
 * Provides adaptive optimization recommendations based on performance metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PhysicsIntegrationProfiler : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PhysicsIntegrationProfiler();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics profiling methods
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StartPhysicsProfiler();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StopPhysicsProfiler();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FPerf_PhysicsProfileData GetCurrentPhysicsProfile() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePhysicsOptimizationSettings(const FPerf_PhysicsOptimizationSettings& NewSettings);

    // Physics analysis methods
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    int32 CountActivePhysicsBodies() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetPhysicsMemoryUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizePhysicsPerformance();

    // Integration testing methods
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    bool ValidatePhysicsIntegration() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void RunPhysicsStressTest(int32 TestDuration = 10);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Performance")
    void GeneratePhysicsPerformanceReport();

protected:
    // Internal profiling data
    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    FPerf_PhysicsProfileData CurrentProfileData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    bool bIsProfilingActive;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float ProfilingStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    TArray<float> FrameTimeHistory;

private:
    // Internal methods
    void UpdatePhysicsMetrics();
    void AnalyzePhysicsPerformance();
    void ApplyPhysicsOptimizations();
    
    // Timer handle for periodic updates
    FTimerHandle ProfilerUpdateTimer;
    
    // Performance tracking
    mutable float LastFrameTime;
    mutable int32 FrameCounter;
};