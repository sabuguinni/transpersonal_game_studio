#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassZoneGraphNavigationFragments.h"
#include "MassSimulationLOD.h"
#include "CrowdMovementProcessor.generated.h"

USTRUCT()
struct TRANSPERSONALGAME_API FCrowdAgentFragment : public FMassFragment
{
    GENERATED_BODY()

    // Crowd-specific properties
    float PersonalSpace = 100.0f;
    float PreferredSpeed = 150.0f;
    int32 GroupID = 0;
    bool bIsLeader = false;
    
    // Behavioral state
    enum class ECrowdBehaviorState : uint8
    {
        Wandering,
        Following,
        Fleeing,
        Gathering,
        Evacuating
    };
    
    ECrowdBehaviorState BehaviorState = ECrowdBehaviorState::Wandering;
    
    // Social dynamics
    float SocialAttraction = 1.0f;
    float PanicLevel = 0.0f;
    FVector LastKnownThreatLocation = FVector::ZeroVector;
};

USTRUCT()
struct TRANSPERSONALGAME_API FCrowdGroupFragment : public FMassFragment
{
    GENERATED_BODY()

    int32 GroupID = 0;
    FMassEntityHandle LeaderEntity;
    TArray<FMassEntityHandle> Members;
    FVector GroupTarget = FVector::ZeroVector;
    float CohesionStrength = 1.0f;
};

/**
 * Mass processor for crowd movement using flocking algorithms and social dynamics
 * Handles movement, avoidance, and group coordination for large crowds
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

    // Movement calculation methods
    FVector CalculateFlockingForce(const FMassEntityHandle& Entity, const FTransformFragment& Transform, 
                                   const FCrowdAgentFragment& CrowdAgent, FMassExecutionContext& Context);
    
    FVector CalculateSeparationForce(const FVector& Position, const FCrowdAgentFragment& CrowdAgent, 
                                     const TArray<FMassEntityHandle>& NearbyEntities, FMassExecutionContext& Context);
    
    FVector CalculateAlignmentForce(const FVector& Velocity, const TArray<FMassEntityHandle>& NearbyEntities, 
                                    FMassExecutionContext& Context);
    
    FVector CalculateCohesionForce(const FVector& Position, const TArray<FMassEntityHandle>& NearbyEntities, 
                                   FMassExecutionContext& Context);
    
    FVector CalculateAvoidanceForce(const FVector& Position, const FVector& Velocity, 
                                    const FCrowdAgentFragment& CrowdAgent, FMassExecutionContext& Context);

    // Emergency behavior
    FVector CalculateFleeForce(const FVector& Position, const FVector& ThreatLocation, float ThreatRadius);
    
    void UpdatePanicLevel(FCrowdAgentFragment& CrowdAgent, const FVector& Position, 
                          const FVector& ThreatLocation, float ThreatRadius, float DeltaTime);

private:
    // Query for crowd agents with movement
    FMassEntityQuery CrowdMovementQuery;
    
    // Query for nearby entities (spatial partitioning)
    FMassEntityQuery NearbyEntitiesQuery;
    
    // Movement parameters
    UPROPERTY(EditAnywhere, Category = "Crowd Movement")
    float MaxSpeed = 200.0f;
    
    UPROPERTY(EditAnywhere, Category = "Crowd Movement")
    float MaxForce = 50.0f;
    
    UPROPERTY(EditAnywhere, Category = "Flocking")
    float SeparationWeight = 2.0f;
    
    UPROPERTY(EditAnywhere, Category = "Flocking")
    float AlignmentWeight = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Flocking")
    float CohesionWeight = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float AvoidanceWeight = 3.0f;
    
    UPROPERTY(EditAnywhere, Category = "Avoidance")
    float AvoidanceRadius = 150.0f;
    
    // Emergency response
    UPROPERTY(EditAnywhere, Category = "Emergency")
    float FleeWeight = 5.0f;
    
    UPROPERTY(EditAnywhere, Category = "Emergency")
    float PanicSpeedMultiplier = 2.0f;
    
    UPROPERTY(EditAnywhere, Category = "Emergency")
    float PanicDecayRate = 1.0f;
};