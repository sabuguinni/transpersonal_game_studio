#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "SharedTypes.h"
#include "Crowd_MassProcessor.generated.h"

/**
 * Mass processor for crowd simulation entities
 * Handles movement, pathfinding, and behavior for large numbers of NPCs
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query for entities with movement and navigation
    FMassEntityQuery MovementQuery;
    
    // Query for entities needing pathfinding
    FMassEntityQuery PathfindingQuery;
    
    // Query for entities with behavior state
    FMassEntityQuery BehaviorQuery;

    // Process movement for crowd entities
    void ProcessMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    // Process pathfinding for crowd entities
    void ProcessPathfinding(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    // Process behavior state changes
    void ProcessBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    UPROPERTY(EditAnywhere, Category = "Crowd Settings")
    float MovementSpeed = 150.0f;
    
    UPROPERTY(EditAnywhere, Category = "Crowd Settings")
    float PathfindingUpdateInterval = 0.5f;
    
    UPROPERTY(EditAnywhere, Category = "Crowd Settings")
    float BehaviorUpdateInterval = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Crowd Settings")
    float MaxCrowdDistance = 5000.0f;

private:
    float LastPathfindingUpdate = 0.0f;
    float LastBehaviorUpdate = 0.0f;
};