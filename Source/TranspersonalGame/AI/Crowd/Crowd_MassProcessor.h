#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Crowd_MassEntityConfig.h"
#include "Engine/World.h"
#include "Crowd_MassProcessor.generated.h"

// Mass processor for crowd behavior logic
UCLASS()
class TRANSPERSONALGAME_API UCrowd_BehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_BehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

private:
    // Query for entities with crowd behavior
    FMassEntityQuery CrowdQuery;

    // Helper functions
    void ProcessBehaviorState(FMassExecutionContext& Context, FCrowd_BehaviorFragment& BehaviorFragment, 
                             FTransformFragment& TransformFragment, FMassVelocityFragment& VelocityFragment, float DeltaTime);
    
    void UpdateIdleBehavior(FCrowd_BehaviorFragment& BehaviorFragment, FTransformFragment& TransformFragment, 
                           FMassVelocityFragment& VelocityFragment, float DeltaTime);
    
    void UpdateWalkingBehavior(FCrowd_BehaviorFragment& BehaviorFragment, FTransformFragment& TransformFragment, 
                              FMassVelocityFragment& VelocityFragment, float DeltaTime);
    
    void UpdateFleeingBehavior(FCrowd_BehaviorFragment& BehaviorFragment, FTransformFragment& TransformFragment, 
                              FMassVelocityFragment& VelocityFragment, float DeltaTime);

    FVector GetRandomWalkTarget(const FVector& CurrentLocation, float Radius = 500.0f);
    bool ShouldChangeState(const FCrowd_BehaviorFragment& BehaviorFragment, float DeltaTime);
    ECrowd_BehaviorState GetNextRandomState(ECrowd_AgentType AgentType);
};

// Mass processor for crowd social behavior and flocking
UCLASS()
class TRANSPERSONALGAME_API UCrowd_SocialProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_SocialProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

private:
    // Query for entities with social behavior
    FMassEntityQuery SocialQuery;

    // Flocking behavior functions
    FVector CalculateSeparation(const FVector& EntityLocation, const TArray<FMassEntityHandle>& NearbyEntities, 
                               UMassEntitySubsystem& EntitySubsystem, float AvoidanceRadius);
    
    FVector CalculateCohesion(const FVector& EntityLocation, const TArray<FMassEntityHandle>& NearbyEntities, 
                             UMassEntitySubsystem& EntitySubsystem);
    
    FVector CalculateAlignment(const TArray<FMassEntityHandle>& NearbyEntities, 
                              UMassEntitySubsystem& EntitySubsystem);

    void FindNearbyEntities(const FVector& EntityLocation, FMassExecutionContext& Context, 
                           TArray<FMassEntityHandle>& OutNearbyEntities, float SearchRadius);
};

// Mass processor for crowd LOD management
UCLASS()
class TRANSPERSONALGAME_API UCrowd_LODProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_LODProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

private:
    // Query for entities with LOD representation
    FMassEntityQuery LODQuery;

    // LOD management functions
    void UpdateLODLevel(FMassRepresentationFragment& RepresentationFragment, 
                       const FVector& EntityLocation, const FVector& ViewerLocation);
    
    float CalculateDistanceToViewer(const FVector& EntityLocation);
    FVector GetPlayerViewLocation();

    // LOD distance thresholds
    float HighLODDistance = 500.0f;
    float MediumLODDistance = 1500.0f;
    float LowLODDistance = 3000.0f;
};