#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Perf_PhysicsOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsLOD : uint8
{
    High UMETA(DisplayName = "High Quality"),
    Medium UMETA(DisplayName = "Medium Quality"),
    Low UMETA(DisplayName = "Low Quality"),
    Disabled UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    int32 MaxSimulatingBodies = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float UpdateFrequency = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    bool bEnableComplexCollision = true;

    FPerf_PhysicsLODSettings()
    {
        MaxDistance = 5000.0f;
        MaxSimulatingBodies = 100;
        UpdateFrequency = 60.0f;
        bEnableComplexCollision = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 ActivePhysicsBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 SleepingBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float PhysicsStepTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 CollisionChecks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float MemoryUsageMB = 0.0f;

    FPerf_PhysicsMetrics()
    {
        ActivePhysicsBodies = 0;
        SleepingBodies = 0;
        PhysicsStepTime = 0.0f;
        CollisionChecks = 0;
        MemoryUsageMB = 0.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics LOD Management
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetPhysicsLOD(AActor* Actor, EPerf_PhysicsLOD LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    EPerf_PhysicsLOD GetPhysicsLOD(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void UpdatePhysicsLODForAllActors();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    FPerf_PhysicsMetrics GetPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsSettings();

    // Distance-based Culling
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetPhysicsCullingDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    float GetPhysicsCullingDistance() const;

    // Body Management
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void PutPhysicsBodyToSleep(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void WakePhysicsBody(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    bool IsPhysicsBodySleeping(AActor* Actor) const;

    // Performance Budget
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetMaxPhysicsBodies(int32 MaxBodies);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    int32 GetMaxPhysicsBodies() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void EnforcePhysicsBudget();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    FPerf_PhysicsLODSettings HighLODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    FPerf_PhysicsLODSettings MediumLODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    FPerf_PhysicsLODSettings LowLODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsCullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsBodies = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationUpdateInterval = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PhysicsMetrics CurrentMetrics;

private:
    float TimeSinceLastOptimization = 0.0f;
    TMap<AActor*, EPerf_PhysicsLOD> ActorLODMap;

    void UpdatePhysicsMetrics();
    void ApplyLODSettings(AActor* Actor, const FPerf_PhysicsLODSettings& Settings);
    float GetDistanceToPlayer(AActor* Actor) const;
    void OptimizeActorPhysics(AActor* Actor);
};