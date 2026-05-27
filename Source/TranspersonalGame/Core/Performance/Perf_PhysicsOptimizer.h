#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
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
struct FPerf_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Limits")
    int32 MaxRagdolls = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Limits")
    int32 MaxDestructionObjects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Limits")
    int32 MaxSimulatingActors = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float HighLODDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float MediumLODDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float LowLODDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePhysicsLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDistanceCulling = true;

    FPerf_PhysicsSettings()
    {
        MaxRagdolls = 10;
        MaxDestructionObjects = 50;
        MaxSimulatingActors = 100;
        HighLODDistance = 1000.0f;
        MediumLODDistance = 2500.0f;
        LowLODDistance = 5000.0f;
        bEnablePhysicsLOD = true;
        bEnableDistanceCulling = true;
    }
};

USTRUCT(BlueprintType)
struct FPerf_PhysicsStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 ActiveRagdolls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 ActiveDestructionObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 SimulatingActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CulledActors = 0;

    FPerf_PhysicsStats()
    {
        ActiveRagdolls = 0;
        ActiveDestructionObjects = 0;
        SimulatingActors = 0;
        PhysicsFrameTime = 0.0f;
        CulledActors = 0;
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    FPerf_PhysicsSettings PhysicsSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Stats")
    FPerf_PhysicsStats CurrentStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticOptimization = true;

    // Physics optimization functions
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsActors();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetPhysicsLOD(AActor* Actor, EPerf_PhysicsLOD LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void CullDistantPhysicsActors();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void EnforcePhysicsLimits();

    UFUNCTION(BlueprintCallable, Category = "Physics Stats")
    void UpdatePhysicsStats();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void DisablePhysicsForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void EnablePhysicsForActor(AActor* Actor);

    UFUNCTION(BlueprintPure, Category = "Physics Stats")
    bool IsPhysicsPerformanceGood() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void ApplyPerformancePreset(const FString& PresetName);

private:
    float LastOptimizationTime;
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;
    TArray<TWeakObjectPtr<AActor>> CulledActors;

    void FindPhysicsActors();
    float GetDistanceToPlayer(AActor* Actor) const;
    void RestoreCulledActors();
    EPerf_PhysicsLOD CalculateLODLevel(float Distance) const;
};