#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "SharedTypes.h"
#include "Crowd_PathfindingProcessor.generated.h"

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_PathfindingFragment : public FMassFragment
{
    GENERATED_BODY()

    FVector TargetLocation = FVector::ZeroVector;
    FVector CurrentDestination = FVector::ZeroVector;
    float PathfindingRadius = 100.0f;
    float MovementSpeed = 150.0f;
    bool bHasValidPath = false;
    bool bReachedDestination = false;
    float LastPathfindTime = 0.0f;
    int32 CurrentWaypointIndex = 0;
    TArray<FVector> PathWaypoints;
};

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_MovementFragment : public FMassFragment
{
    GENERATED_BODY()

    FVector Velocity = FVector::ZeroVector;
    FVector Acceleration = FVector::ZeroVector;
    float MaxSpeed = 200.0f;
    float MaxAcceleration = 400.0f;
    float BrakingDeceleration = 800.0f;
    float TurningSpeed = 180.0f;
    bool bIsMoving = false;
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

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void UpdateEntityPathfinding(FMassEntityHandle Entity, FCrowd_PathfindingFragment& PathfindingFragment, 
                                FTransformFragment& TransformFragment, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool FindPathToTarget(const FVector& StartLocation, const FVector& TargetLocation, 
                         TArray<FVector>& OutPath, float SearchRadius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void UpdateMovementTowardsTarget(FCrowd_PathfindingFragment& PathfindingFragment, 
                                   FCrowd_MovementFragment& MovementFragment, 
                                   FTransformFragment& TransformFragment, float DeltaTime);

private:
    FMassEntityQuery EntityQuery;
    
    UPROPERTY(EditAnywhere, Category = "Configuration")
    float PathfindingUpdateInterval = 0.5f;
    
    UPROPERTY(EditAnywhere, Category = "Configuration")
    float DestinationReachedDistance = 50.0f;
    
    UPROPERTY(EditAnywhere, Category = "Configuration")
    float MaxPathfindingDistance = 2000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Configuration")
    bool bEnableAvoidance = true;
    
    UPROPERTY(EditAnywhere, Category = "Configuration")
    float AvoidanceRadius = 100.0f;

    UNavigationSystemV1* NavigationSystem;
    
    void InitializeNavigationSystem();
    bool IsValidPathfindingTarget(const FVector& TargetLocation) const;
    FVector GetAvoidanceVector(const FVector& CurrentLocation, const TArray<FVector>& NearbyEntities) const;
    void SmoothPath(TArray<FVector>& Path) const;
};