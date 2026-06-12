#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "MassCommonFragments.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Crowd_MassEntityTraits.h"
#include "Crowd_PathfindingProcessor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathfindingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FVector> WaypointPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathRecalculationTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bPathValid = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector LastKnownTarget = FVector::ZeroVector;
};

UCLASS()
class TRANSPERSONALGAME_API UCrowd_PathfindingProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_PathfindingProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

    // Pathfinding methods
    bool FindPathToTarget(const FVector& StartLocation, const FVector& TargetLocation, TArray<FVector>& OutPath);
    FVector GetNextWaypoint(const FCrowd_PathfindingData& PathData, const FVector& CurrentLocation);
    bool ShouldRecalculatePath(const FCrowd_PathfindingData& PathData, const FVector& CurrentTarget);
    
    // Flocking and avoidance
    FVector CalculateFlockingForce(const FVector& EntityLocation, const TArray<FVector>& NearbyEntities);
    FVector CalculateAvoidanceForce(const FVector& EntityLocation, const FVector& EntityVelocity);
    FVector CalculateSeekForce(const FVector& EntityLocation, const FVector& TargetLocation, float MaxSpeed);

private:
    FMassEntityQuery EntityQuery;

    UPROPERTY(EditAnywhere, Category = "Pathfinding Settings")
    float PathRecalculationInterval = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Pathfinding Settings")
    float WaypointReachDistance = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Pathfinding Settings")
    float MaxPathDistance = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "Flocking Settings")
    float FlockingWeight = 0.3f;

    UPROPERTY(EditAnywhere, Category = "Flocking Settings")
    float AvoidanceWeight = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Flocking Settings")
    float SeekWeight = 0.7f;

    UPROPERTY(EditAnywhere, Category = "Flocking Settings")
    float NeighborSearchRadius = 300.0f;

    // Cache for navigation system
    UPROPERTY()
    UNavigationSystemV1* NavigationSystem;

    // Waypoint cache for performance
    TArray<FVector> GlobalWaypoints;
    float WaypointCacheUpdateTimer = 0.0f;
};