#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Physics/PhysicsInterfaceCore.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Perf_PhysicsPerformanceAnalyzer.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsOptimizationLevel : uint8
{
    None        UMETA(DisplayName = "No Optimization"),
    Low         UMETA(DisplayName = "Low Optimization"),
    Medium      UMETA(DisplayName = "Medium Optimization"),
    High        UMETA(DisplayName = "High Optimization"),
    Extreme     UMETA(DisplayName = "Extreme Optimization")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 ActiveRigidBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 SleepingRigidBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 ActiveConstraints;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float PhysicsStepTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float CollisionDetectionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 CollisionPairs;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float RagdollUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 ActiveRagdolls;

    FPerf_PhysicsMetrics()
        : ActiveRigidBodies(0)
        , SleepingRigidBodies(0)
        , ActiveConstraints(0)
        , PhysicsStepTime(0.0f)
        , CollisionDetectionTime(0.0f)
        , CollisionPairs(0)
        , RagdollUpdateTime(0.0f)
        , ActiveRagdolls(0)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float SleepThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxActiveRigidBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnablePhysicsLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableCollisionCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsTickRate;

    FPerf_PhysicsOptimizationSettings()
        : CullingDistance(5000.0f)
        , SleepThreshold(0.1f)
        , MaxActiveRigidBodies(500)
        , bEnablePhysicsLOD(true)
        , bEnableCollisionCulling(true)
        , PhysicsTickRate(60.0f)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PhysicsPerformanceAnalyzer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PhysicsPerformanceAnalyzer();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics performance analysis
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StartPhysicsAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StopPhysicsAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FPerf_PhysicsMetrics GetCurrentPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ApplyOptimizationLevel(EPerf_PhysicsOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizePhysicsForFrameRate(float TargetFrameRate);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void CullDistantPhysicsObjects(const FVector& PlayerLocation, float CullDistance);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ForcePhysicsSleep(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void WakePhysicsObject(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    bool IsPhysicsOptimizationActive() const { return bAnalysisActive; }

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetPhysicsPerformanceScore() const;

    // Ragdoll optimization
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void OptimizeRagdollPerformance();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void SetRagdollLODLevel(AActor* Actor, int32 LODLevel);

    // Destruction system optimization
    UFUNCTION(BlueprintCallable, Category = "Destruction Performance")
    void OptimizeDestructionSystem();

    UFUNCTION(BlueprintCallable, Category = "Destruction Performance")
    void LimitActiveDebris(int32 MaxDebrisCount);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Physics Analysis")
    bool bAnalysisActive;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Analysis")
    FPerf_PhysicsMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFramePhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AveragePhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 FramesSampled;

private:
    // Internal analysis methods
    void UpdatePhysicsMetrics();
    void AnalyzePhysicsPerformance();
    void ApplyPhysicsOptimizations();
    void CullPhysicsObjects();
    void OptimizeCollisionDetection();
    void ManagePhysicsLOD();
    void UpdateRagdollOptimization();
    void CleanupDestructionDebris();

    // Performance tracking
    TArray<float> PhysicsTimeSamples;
    float PhysicsAnalysisTimer;
    float OptimizationUpdateTimer;

    // Cached references
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;
    TArray<TWeakObjectPtr<AActor>> ActiveRagdolls;
    TArray<TWeakObjectPtr<AActor>> DestructionDebris;

    // Optimization state
    EPerf_PhysicsOptimizationLevel CurrentOptimizationLevel;
    bool bPhysicsLODEnabled;
    bool bCollisionCullingEnabled;
    float PhysicsCullingDistance;
};