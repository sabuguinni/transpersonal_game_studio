#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassFlockingProcessor.generated.h"

// Fragment for flocking behavior data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingFragment : public FMassFragment
{
    GENERATED_BODY()

    // Separation distance - how far to stay from neighbors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationDistance = 300.0f;

    // Alignment weight - how much to match neighbor velocities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight = 0.5f;

    // Cohesion weight - how much to move toward group center
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight = 0.3f;

    // Separation weight - how much to avoid crowding
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight = 0.8f;

    // Maximum speed for this entity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxSpeed = 600.0f;

    // Neighbor search radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float NeighborRadius = 500.0f;

    // Current flocking force
    FVector FlockingForce = FVector::ZeroVector;

    // Last update time
    float LastUpdateTime = 0.0f;
};

// Fragment for group membership
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_GroupFragment : public FMassFragment
{
    GENERATED_BODY()

    // Group ID this entity belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    int32 GroupID = 0;

    // Role within the group
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    ECrowd_GroupRole GroupRole = ECrowd_GroupRole::Follower;

    // Leader entity (if follower)
    FMassEntityHandle LeaderEntity;

    // Group center position
    FVector GroupCenter = FVector::ZeroVector;

    // Group velocity
    FVector GroupVelocity = FVector::ZeroVector;

    // Number of group members
    int32 GroupSize = 1;
};

/**
 * Mass processor that implements flocking behavior for dinosaur crowds
 * Uses boids algorithm with separation, alignment, and cohesion
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassFlockingProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassFlockingProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query for entities with flocking behavior
    FMassEntityQuery FlockingQuery;

    // Calculate separation force
    FVector CalculateSeparation(const FVector& Position, const TArray<FVector>& NeighborPositions, float SeparationDistance);

    // Calculate alignment force
    FVector CalculateAlignment(const FVector& Velocity, const TArray<FVector>& NeighborVelocities);

    // Calculate cohesion force
    FVector CalculateCohesion(const FVector& Position, const TArray<FVector>& NeighborPositions);

    // Find neighbors within radius
    void FindNeighbors(const FVector& Position, float Radius, const TArray<FMassEntityHandle>& AllEntities,
                      FMassEntityManager& EntityManager, TArray<FVector>& OutPositions, TArray<FVector>& OutVelocities);

    // Update group data
    void UpdateGroupData(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

public:
    // Flocking behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float DefaultSeparationDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float DefaultAlignmentWeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float DefaultCohesionWeight = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float DefaultSeparationWeight = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float DefaultMaxSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float DefaultNeighborRadius = 500.0f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 10.0f; // Updates per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxNeighborsToCheck = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseOctreeForNeighborSearch = true;
};