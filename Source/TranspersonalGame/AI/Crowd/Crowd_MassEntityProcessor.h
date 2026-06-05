#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityProcessor.generated.h"

// Forward declarations
class UMassEntitySubsystem;
struct FCrowd_AgentFragment;
struct FCrowd_BehaviorFragment;

/**
 * Mass Entity Processor for handling 50,000+ crowd agents simultaneously
 * Implements high-performance crowd simulation using UE5 Mass Entity framework
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntityProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityProcessor();

    // UMassProcessor interface
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

protected:
    // Core processing queries
    FMassEntityQuery MovementQuery;
    FMassEntityQuery BehaviorQuery;
    FMassEntityQuery LODQuery;
    FMassEntityQuery PathfindingQuery;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxAgentsPerFrame = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance") 
    float ProcessingTimeSlice = 0.016f; // 16ms per frame

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODCulling = true;

    // Crowd behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SeparationRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlignmentRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CohesionRadius = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MaxSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MaxForce = 100.0f;

private:
    // Processing methods
    void ProcessMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessLOD(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessPathfinding(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    // Utility methods
    FVector CalculateFlockingForce(const FVector& Position, const FVector& Velocity, 
                                  const TArray<FVector>& Neighbors, const TArray<FVector>& NeighborVelocities);
    
    float CalculateLODLevel(const FVector& AgentPosition, const FVector& ViewerPosition);
    
    bool ShouldProcessAgent(int32 AgentIndex, float DeltaTime);

    // Performance tracking
    mutable int32 ProcessedAgentsThisFrame = 0;
    mutable float AccumulatedProcessingTime = 0.0f;
};

/**
 * Fragment for individual crowd agent data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentFragment : public FMassFragment
{
    GENERATED_BODY()

    // Agent identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowd_AgentType AgentType = ECrowd_AgentType::Civilian;

    // Movement data
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 200.0f;

    // Behavior state
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowd_BehaviorState BehaviorState = ECrowd_BehaviorState::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateTimer = 0.0f;

    // LOD data
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LODLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToViewer = 0.0f;

    // Social data
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> NearbyAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GroupID = -1;
};

/**
 * Fragment for crowd behavior logic
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentWeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionWeight = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeekWeight = 2.0f;

    // Pathfinding data
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PathPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentPathIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasValidPath = false;

    // Reaction data
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReactionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PanicThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentStress = 0.0f;
};