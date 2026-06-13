#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "Engine/World.h"
#include "Crowd_MassEntityBehaviorProcessor.generated.h"

// Crowd behavior states for Mass Entity system
UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Following   UMETA(DisplayName = "Following"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Interacting UMETA(DisplayName = "Interacting")
};

// Crowd behavior fragment for Mass Entity
USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_BehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowd_BehaviorState CurrentState = ECrowd_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxStateTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReactionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsReactingToPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsReactingToDinosaur = false;
};

// Crowd formation fragment for group behaviors
USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_FormationFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FormationID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PositionInFormation = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FormationOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFormationLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FormationTightness = 1.0f;
};

// Crowd LOD fragment for performance optimization
USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_LODFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentLODLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToPlayer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UpdateFrequency = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShouldRender = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShouldSimulate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastUpdateTime = 0.0f;
};

/**
 * Mass Entity processor for crowd behavior simulation
 * Handles state transitions, formations, and LOD optimization
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassEntityBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query for entities with behavior components
    FMassEntityQuery BehaviorQuery;
    
    // Query for entities with formation components  
    FMassEntityQuery FormationQuery;
    
    // Query for entities with LOD components
    FMassEntityQuery LODQuery;

    // Behavior processing methods
    void ProcessBehaviorStates(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessFormations(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessLODOptimization(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    // State transition logic
    void UpdateBehaviorState(FCrowd_BehaviorFragment& BehaviorFragment, const FTransformFragment& Transform, float DeltaTime);
    void HandlePlayerReaction(FCrowd_BehaviorFragment& BehaviorFragment, const FVector& EntityLocation);
    void HandleDinosaurReaction(FCrowd_BehaviorFragment& BehaviorFragment, const FVector& EntityLocation);

    // Formation logic
    void UpdateFormationPosition(FCrowd_FormationFragment& FormationFragment, FTransformFragment& Transform);
    void MaintainFormationCohesion(const TArrayView<FMassEntityHandle>& FormationEntities);

    // LOD optimization
    void CalculateLODLevel(FCrowd_LODFragment& LODFragment, const FVector& EntityLocation);
    void UpdateEntityVisibility(const FCrowd_LODFragment& LODFragment, FMassEntityHandle Entity);

    // Performance settings
    UPROPERTY(EditAnywhere, Category = "Performance")
    float MaxProcessingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "Performance") 
    int32 MaxEntitiesPerFrame = 500;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float StateChangeInterval = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float PlayerDetectionRadius = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float DinosaurDetectionRadius = 1200.0f;

    // LOD distance thresholds
    UPROPERTY(EditAnywhere, Category = "LOD")
    float HighLODDistance = 500.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float MediumLODDistance = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float LowLODDistance = 3000.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float CullingDistance = 5000.0f;
};