// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonTypes.h"
#include "MassMovementTypes.h"
#include "MassNavigationTypes.h"
#include "MassLODTypes.h"
#include "MassRepresentationTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/World.h"
#include "CrowdMovementProcessor.generated.h"

class UZoneGraphSubsystem;
class UNavigationSystemV1;

UENUM(BlueprintType)
enum class ECrowdMovementState : uint8
{
    Idle,
    Wandering,
    Following,
    Fleeing,
    Investigating,
    Grazing,
    Hunting,
    Migrating,
    Socializing,
    Resting
};

USTRUCT()
struct FCrowdMovementFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Movement")
    ECrowdMovementState MovementState = ECrowdMovementState::Wandering;

    UPROPERTY(EditAnywhere, Category = "Movement")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Movement")
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxSpeed = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float Acceleration = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float Deceleration = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float TurnRate = 180.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float WanderRadius = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float WanderDistance = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float WanderJitter = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    FVector WanderTarget = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float NextStateChangeTime = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    bool bUseZoneGraph = true;

    UPROPERTY(EditAnywhere, Category = "Movement")
    bool bAvoidObstacles = true;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float ObstacleAvoidanceRadius = 150.0f;
};

USTRUCT()
struct FCrowdBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Behavior")
    FGameplayTagContainer BehaviorTags;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float Fearfulness = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float Energy = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float Hunger = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float LastInteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    TWeakObjectPtr<AActor> FollowTarget;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    TWeakObjectPtr<AActor> FleeTarget;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    FVector InterestPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float InterestRadius = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    bool bIsGroupLeader = false;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    int32 GroupID = -1;
};

USTRUCT()
struct FCrowdPerceptionFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Perception")
    float SightRange = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Perception")
    float SightAngle = 120.0f;

    UPROPERTY(EditAnywhere, Category = "Perception")
    float HearingRange = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Perception")
    TArray<TWeakObjectPtr<AActor>> VisibleActors;

    UPROPERTY(EditAnywhere, Category = "Perception")
    TArray<TWeakObjectPtr<AActor>> NearbyAgents;

    UPROPERTY(EditAnywhere, Category = "Perception")
    TWeakObjectPtr<AActor> NearestThreat;

    UPROPERTY(EditAnywhere, Category = "Perception")
    TWeakObjectPtr<AActor> NearestFood;

    UPROPERTY(EditAnywhere, Category = "Perception")
    TWeakObjectPtr<AActor> NearestAlly;

    UPROPERTY(EditAnywhere, Category = "Perception")
    float LastPerceptionUpdate = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Perception")
    float PerceptionUpdateInterval = 0.5f;
};

USTRUCT()
struct FCrowdAvoidanceFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float AvoidanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float SeparationWeight = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float AlignmentWeight = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float CohesionWeight = 0.3f;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float ObstacleAvoidanceWeight = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    FVector SeparationForce = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    FVector AlignmentForce = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    FVector CohesionForce = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    FVector ObstacleAvoidanceForce = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    FVector TotalAvoidanceForce = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    bool bEnableSeparation = true;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    bool bEnableAlignment = true;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    bool bEnableCohesion = true;

    UPROPERTY(EditAnywhere, Category = "Avoidance")
    bool bEnableObstacleAvoidance = true;
};

/**
 * Mass processor for crowd movement behaviors
 * Handles flocking, wandering, following, and avoidance behaviors for crowd entities
 * Integrates with Zone Graph for navigation and supports various movement states
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdMovementProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowdMovementProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

    // Query for entities with movement components
    FMassEntityQuery MovementQuery;

    // Subsystem references
    UPROPERTY()
    UZoneGraphSubsystem* ZoneGraphSubsystem;

    UPROPERTY()
    UNavigationSystemV1* NavigationSystem;

    // Movement parameters
    UPROPERTY(EditAnywhere, Category = "Movement Settings")
    float WanderUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Movement Settings")
    float PerceptionUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Movement Settings")
    float MaxMovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Movement Settings")
    float MinMovementSpeed = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Movement Settings")
    float FlockingRadius = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Movement Settings")
    float AvoidanceRadius = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Movement Settings")
    float ObstacleCheckDistance = 300.0f;

    // Behavior weights
    UPROPERTY(EditAnywhere, Category = "Behavior Weights")
    float WanderWeight = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior Weights")
    float FlockingWeight = 0.8f;

    UPROPERTY(EditAnywhere, Category = "Behavior Weights")
    float AvoidanceWeight = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Behavior Weights")
    float FollowWeight = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior Weights")
    float FleeWeight = 3.0f;

private:
    // Movement behavior functions
    FVector CalculateWanderForce(const FTransform& Transform, FCrowdMovementFragment& Movement, float DeltaTime);
    FVector CalculateFlockingForce(const FTransform& Transform, const FCrowdMovementFragment& Movement, 
                                  const FCrowdPerceptionFragment& Perception);
    FVector CalculateAvoidanceForce(const FTransform& Transform, const FCrowdMovementFragment& Movement,
                                   const FCrowdAvoidanceFragment& Avoidance, const FCrowdPerceptionFragment& Perception);
    FVector CalculateFollowForce(const FTransform& Transform, const FCrowdBehaviorFragment& Behavior);
    FVector CalculateFleeForce(const FTransform& Transform, const FCrowdBehaviorFragment& Behavior);
    FVector CalculateObstacleAvoidance(const FTransform& Transform, const FCrowdMovementFragment& Movement);

    // Flocking sub-behaviors
    FVector CalculateSeparation(const FTransform& Transform, const FCrowdPerceptionFragment& Perception, float Radius);
    FVector CalculateAlignment(const FTransform& Transform, const FCrowdPerceptionFragment& Perception, float Radius);
    FVector CalculateCohesion(const FTransform& Transform, const FCrowdPerceptionFragment& Perception, float Radius);

    // Utility functions
    void UpdateMovementState(FCrowdMovementFragment& Movement, const FCrowdBehaviorFragment& Behavior, 
                            const FCrowdPerceptionFragment& Perception, float DeltaTime);
    void UpdatePerception(FCrowdPerceptionFragment& Perception, const FTransform& Transform, 
                         FMassEntityManager& EntityManager, float DeltaTime);
    bool IsValidNavLocation(const FVector& Location) const;
    FVector ClampVelocity(const FVector& Velocity, float MaxSpeed) const;
    FVector SteerTowards(const FVector& CurrentVelocity, const FVector& DesiredDirection, float MaxForce) const;

    // Performance tracking
    float LastProcessTime = 0.0f;
    int32 ProcessedEntityCount = 0;
};