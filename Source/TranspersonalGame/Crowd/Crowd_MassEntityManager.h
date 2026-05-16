#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

// Mass Entity configuration for crowd simulation
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassEntityConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float FlockingRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float AvoidanceRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float LODDistance2 = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float LODDistance3 = 5000.0f;

    FCrowd_MassEntityConfig()
    {
        MaxEntities = 1000;
        SpawnRadius = 5000.0f;
        MovementSpeed = 300.0f;
        FlockingRadius = 500.0f;
        AvoidanceRadius = 200.0f;
        LODDistance1 = 1000.0f;
        LODDistance2 = 2000.0f;
        LODDistance3 = 5000.0f;
    }
};

// Mass Entity behavior data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WanderRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FlockWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AvoidanceWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SeekWeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsActive = true;

    FCrowd_EntityBehaviorData()
    {
        TargetLocation = FVector::ZeroVector;
        WanderRadius = 1000.0f;
        FlockWeight = 1.0f;
        AvoidanceWeight = 2.0f;
        SeekWeight = 0.5f;
        bIsActive = true;
    }
};

/**
 * Mass Entity Manager for large-scale crowd simulation
 * Manages up to 50,000 entities using UE5 Mass Entity framework
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Mass Entity configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity Config")
    FCrowd_MassEntityConfig EntityConfig;

    // Behavior configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Config")
    FCrowd_EntityBehaviorData DefaultBehavior;

    // Mass Entity subsystem references
    UPROPERTY(BlueprintReadOnly, Category = "Mass Entity")
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY(BlueprintReadOnly, Category = "Mass Entity")
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY(BlueprintReadOnly, Category = "Mass Entity")
    class UMassSimulationSubsystem* MassSimulationSubsystem;

    // Entity management functions
    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SpawnMassEntities(int32 NumEntities, FVector SpawnCenter);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void DestroyAllMassEntities();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateEntityBehaviors(const FCrowd_EntityBehaviorData& NewBehavior);

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateLODDistances(float LOD1, float LOD2, float LOD3);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    int32 GetCurrentEntityCount() const;

    // Flocking behavior
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void SetFlockingParameters(float FlockRadius, float AvoidRadius, float MoveSpeed);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void AddFlockingTarget(FVector TargetLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void RemoveFlockingTarget(FVector TargetLocation);

    // Crowd density management
    UFUNCTION(BlueprintCallable, Category = "Density")
    void SetCrowdDensity(float DensityMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Density")
    float GetCurrentCrowdDensity() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMassEntityPerformanceMetric() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMassEntityPerformance();

private:
    // Internal entity tracking
    TArray<FMassEntityHandle> ManagedEntities;
    
    // Performance tracking
    float LastFrameTime;
    int32 CurrentEntityCount;
    float CrowdDensityMultiplier;

    // Internal helper functions
    void UpdateEntityLOD(float DeltaTime);
    void ProcessFlockingBehavior(float DeltaTime);
    void ValidateMassEntitySubsystems();
    FVector CalculateFlockingForce(const FMassEntityHandle& Entity);
    FVector CalculateAvoidanceForce(const FMassEntityHandle& Entity);
    FVector CalculateSeekForce(const FMassEntityHandle& Entity, FVector Target);
};