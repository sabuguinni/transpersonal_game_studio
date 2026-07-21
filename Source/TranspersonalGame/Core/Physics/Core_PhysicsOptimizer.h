#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SharedTypes.h"
#include "Core_PhysicsOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float NearDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float MidDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float FarDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    int32 MaxNearPhysicsActors = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    int32 MaxMidPhysicsActors = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    int32 MaxFarPhysicsActors = 10;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SleepingRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionChecks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AveragePhysicsStepTime = 0.0f;
};

/**
 * Core Physics Optimizer Component
 * Manages physics performance through LOD systems, culling, and adaptive quality settings
 * Ensures stable 60fps by dynamically adjusting physics complexity based on distance and importance
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Physics LOD Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    FCore_PhysicsLODSettings LODSettings;

    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FCore_PhysicsPerformanceMetrics CurrentMetrics;

    // Optimization Controls
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnablePhysicsLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAdaptiveQuality = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float TargetFrameTime = 16.67f; // 60fps target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsSubstepScale = 1.0f;

private:
    // Internal tracking
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;
    float LastOptimizationTime = 0.0f;
    float OptimizationInterval = 0.5f; // Run optimization every 0.5 seconds
    
    // Performance history for adaptive scaling
    TArray<float> FrameTimeHistory;
    static constexpr int32 MaxFrameHistory = 60;

public:
    // Physics LOD Management
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsLOD();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void RegisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void UnregisterPhysicsActor(AActor* Actor);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FCore_PhysicsPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameTime() const;

    // Adaptive Quality Control
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void AdjustPhysicsQuality(float TargetFrameTime);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetPhysicsSubstepScale(float NewScale);

    // Debug and Profiling
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugPhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPhysicsStats();

private:
    void UpdatePerformanceMetrics();
    void ApplyLODToActor(AActor* Actor, float Distance);
    void OptimizePhysicsSettings();
    ECore_PhysicsLODLevel CalculateLODLevel(float Distance) const;
};