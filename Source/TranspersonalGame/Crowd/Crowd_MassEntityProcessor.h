#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassLODFragments.h"
#include "MassRepresentationFragments.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "TranspersonalGame.h"
#include "Crowd_MassEntityProcessor.generated.h"

// Crowd-specific fragments for Mass Entity system
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MovementFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Movement")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Movement")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Movement")
    float WanderRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Movement")
    float FlockingStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Movement")
    float SeparationDistance = 100.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    float AlertLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    float FleeThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    FVector FleeDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    bool bIsFleeing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    float IdleTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    float MaxIdleTime = 5.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    int32 LODLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float DistanceToPlayer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    bool bIsVisible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float UpdateFrequency = 1.0f;
};

/**
 * Mass Entity processor for crowd movement and flocking behavior
 * Handles up to 50,000 crowd entities with LOD-based optimization
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassMovementProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassMovementProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery MovementQuery;

    UPROPERTY(EditAnywhere, Category = "Crowd Movement")
    float FlockingRadius = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Crowd Movement")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Crowd Movement")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Crowd Movement")
    float CohesionWeight = 1.0f;

    // Calculate flocking forces
    FVector CalculateSeparation(const FVector& EntityLocation, const TArray<FVector>& NearbyLocations);
    FVector CalculateAlignment(const FVector& EntityVelocity, const TArray<FVector>& NearbyVelocities);
    FVector CalculateCohesion(const FVector& EntityLocation, const TArray<FVector>& NearbyLocations);
};

/**
 * Mass Entity processor for crowd behavior and reactions
 * Handles threat detection, fleeing, and idle behaviors
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery BehaviorQuery;

    UPROPERTY(EditAnywhere, Category = "Crowd Behavior")
    float ThreatDetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Crowd Behavior")
    float FleeSpeed = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Crowd Behavior")
    float FleeDistance = 2000.0f;

    // Detect threats in the environment
    bool DetectThreats(const FVector& EntityLocation, UWorld* World);
    
    // Calculate flee direction from threats
    FVector CalculateFleeDirection(const FVector& EntityLocation, const FVector& ThreatLocation);
};

/**
 * Mass Entity processor for LOD management and performance optimization
 * Dynamically adjusts update frequency and visual representation based on distance
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassLODProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassLODProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery LODQuery;

    UPROPERTY(EditAnywhere, Category = "Crowd LOD")
    float LOD0Distance = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Crowd LOD")
    float LOD1Distance = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Crowd LOD")
    float LOD2Distance = 3000.0f;

    UPROPERTY(EditAnywhere, Category = "Crowd LOD")
    float CullingDistance = 5000.0f;

    // Calculate LOD level based on distance to player
    int32 CalculateLODLevel(float DistanceToPlayer);
    
    // Get player location for distance calculations
    FVector GetPlayerLocation(UWorld* World);
};