#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "AI/NavigationSystemBase.h"
#include "Crowd_PathfindingProcessor.generated.h"

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_PathfindingFragment : public FMassFragment
{
    GENERATED_BODY()

    FVector TargetLocation;
    FVector CurrentPath[10]; // Simple path array
    int32 CurrentPathIndex;
    int32 PathLength;
    float PathfindingCooldown;
    bool bHasValidPath;
    bool bReachedTarget;

    FCrowd_PathfindingFragment()
        : TargetLocation(FVector::ZeroVector)
        , CurrentPathIndex(0)
        , PathLength(0)
        , PathfindingCooldown(0.0f)
        , bHasValidPath(false)
        , bReachedTarget(false)
    {
        for (int32 i = 0; i < 10; i++)
        {
            CurrentPath[i] = FVector::ZeroVector;
        }
    }
};

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_NavigationFragment : public FMassFragment
{
    GENERATED_BODY()

    float MovementSpeed;
    float AvoidanceRadius;
    float StoppingDistance;
    int32 NavigationFlags;
    float MaxPathLength;

    FCrowd_NavigationFragment()
        : MovementSpeed(150.0f)
        , AvoidanceRadius(100.0f)
        , StoppingDistance(50.0f)
        , NavigationFlags(0)
        , MaxPathLength(2000.0f)
    {}
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

private:
    FMassEntityQuery EntityQuery;

    // Pathfinding methods
    bool FindPathToTarget(const FVector& StartLocation, const FVector& TargetLocation, 
                         FVector* OutPath, int32& OutPathLength, int32 MaxPathPoints = 10);
    
    bool IsPathClear(const FVector& Start, const FVector& End, float AvoidanceRadius);
    
    FVector GetNextPathPoint(const FCrowd_PathfindingFragment& PathFragment);
    
    void UpdatePathfinding(FCrowd_PathfindingFragment& PathFragment, 
                          const FTransformFragment& Transform,
                          const FCrowd_NavigationFragment& NavFragment,
                          float DeltaTime);
    
    void ProcessMovementAlongPath(FTransformFragment& Transform,
                                 FMassVelocityFragment& Velocity,
                                 const FCrowd_PathfindingFragment& PathFragment,
                                 const FCrowd_NavigationFragment& NavFragment,
                                 float DeltaTime);

    // Navigation system reference
    UPROPERTY()
    class UNavigationSystemV1* NavigationSystem;

    // Pathfinding settings
    UPROPERTY(EditAnywhere, Category = "Pathfinding")
    float PathfindingUpdateInterval;

    UPROPERTY(EditAnywhere, Category = "Pathfinding")
    float PathValidationDistance;

    UPROPERTY(EditAnywhere, Category = "Pathfinding")
    int32 MaxEntitiesPerFrame;

    UPROPERTY(EditAnywhere, Category = "Pathfinding")
    bool bUseSimplePathfinding;

    // Performance tracking
    int32 ProcessedEntitiesThisFrame;
    float LastPathfindingTime;
};