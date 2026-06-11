#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "Crowd_PathfindingProcessor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathfindingFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float MaxSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float AcceptanceRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bHasValidPath = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FVector> PathPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 CurrentPathIndex = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AvoidanceFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance")
    float AvoidanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance")
    float AvoidanceWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance")
    FVector AvoidanceForce = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance")
    TArray<FMassEntityHandle> NearbyEntities;
};

/**
 * Mass Entity processor for crowd pathfinding and navigation
 * Handles pathfinding, obstacle avoidance, and movement for crowd agents
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_PathfindingProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_PathfindingProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery PathfindingQuery;

    UPROPERTY(EditAnywhere, Category = "Pathfinding")
    float PathUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Pathfinding") 
    float MaxPathfindingRange = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float NeighborSearchRadius = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float SeparationWeight = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float CohesionWeight = 0.8f;

    // Pathfinding functions
    bool FindPathToTarget(const FVector& StartLocation, const FVector& TargetLocation, TArray<FVector>& OutPath);
    FVector CalculateSteeringForce(const FCrowd_PathfindingFragment& PathData, const FVector& CurrentLocation);
    FVector CalculateAvoidanceForce(const FCrowd_AvoidanceFragment& AvoidanceData, const FVector& CurrentLocation, const TArray<FVector>& NearbyPositions);
    
    // Flocking behavior functions
    FVector CalculateSeparation(const FVector& CurrentLocation, const TArray<FVector>& NearbyPositions, float Radius);
    FVector CalculateAlignment(const FVector& CurrentVelocity, const TArray<FVector>& NearbyVelocities);
    FVector CalculateCohesion(const FVector& CurrentLocation, const TArray<FVector>& NearbyPositions);

    // Utility functions
    void UpdateNearbyEntities(FMassEntityManager& EntityManager, const FVector& Location, float SearchRadius, TArray<FMassEntityHandle>& OutNearbyEntities);
    bool IsLocationReachable(const FVector& Location);
    FVector GetRandomWanderTarget(const FVector& CurrentLocation, float WanderRadius);
};