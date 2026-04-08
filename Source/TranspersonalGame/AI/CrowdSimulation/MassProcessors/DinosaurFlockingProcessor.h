#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "DinosaurFlockingProcessor.generated.h"

// Custom fragments for dinosaur behavior
USTRUCT()
struct FDinosaurFlockingFragment : public FMassFragment
{
    GENERATED_BODY()

    // Flocking parameters
    float CohesionWeight = 0.3f;
    float SeparationWeight = 0.5f;
    float AlignmentWeight = 0.2f;
    float AvoidanceWeight = 0.8f;

    // Flocking radii
    float CohesionRadius = 500.0f;
    float SeparationRadius = 200.0f;
    float AlignmentRadius = 400.0f;
    float AvoidanceRadius = 300.0f;

    // Behavior state
    bool bInPanicMode = false;
    FVector PanicSource = FVector::ZeroVector;
    float PanicStartTime = 0.0f;
    float PanicDuration = 5.0f;

    // Herd identification
    uint32 HerdID = 0;
    bool bIsHerdLeader = false;
};

USTRUCT()
struct FDinosaurTypeFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY()
    uint8 DinosaurType = 0; // Maps to EDinosaurHerdType

    UPROPERTY()
    float MaxSpeed = 300.0f;

    UPROPERTY()
    float TurnRate = 180.0f;

    UPROPERTY()
    bool bAvoidsPredators = true;

    UPROPERTY()
    bool bIsPredator = false;

    UPROPERTY()
    float DetectionRadius = 1000.0f;
};

USTRUCT()
struct FDinosaurStaminaFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY()
    float CurrentStamina = 100.0f;

    UPROPERTY()
    float MaxStamina = 100.0f;

    UPROPERTY()
    float StaminaRegenRate = 10.0f;

    UPROPERTY()
    float StaminaDrainRate = 20.0f;

    UPROPERTY()
    bool bIsExhausted = false;
};

/**
 * Mass processor that handles flocking behavior for dinosaur entities
 * Implements Reynolds' boids algorithm with prehistoric creature adaptations
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurFlockingProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurFlockingProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Mass queries for different entity types
    FMassEntityQuery FlockingQuery;
    FMassEntityQuery NearbyEntitiesQuery;
    FMassEntityQuery PredatorQuery;
    FMassEntityQuery PreyQuery;

    // Core flocking calculations
    FVector CalculateCohesion(const FMassExecutionContext& Context, 
                             const FTransformFragment& Transform,
                             const FDinosaurFlockingFragment& Flocking,
                             uint32 HerdID);

    FVector CalculateSeparation(const FMassExecutionContext& Context,
                               const FTransformFragment& Transform,
                               const FDinosaurFlockingFragment& Flocking,
                               uint32 HerdID);

    FVector CalculateAlignment(const FMassExecutionContext& Context,
                              const FTransformFragment& Transform,
                              const FDinosaurFlockingFragment& Flocking,
                              uint32 HerdID);

    FVector CalculateAvoidance(const FMassExecutionContext& Context,
                              const FTransformFragment& Transform,
                              const FDinosaurFlockingFragment& Flocking,
                              const FDinosaurTypeFragment& Type);

    // Predator-prey interactions
    FVector CalculateFleeFromPredators(const FMassExecutionContext& Context,
                                      const FTransformFragment& Transform,
                                      const FDinosaurTypeFragment& Type);

    FVector CalculateHuntPrey(const FMassExecutionContext& Context,
                             const FTransformFragment& Transform,
                             const FDinosaurTypeFragment& Type);

    // Environmental awareness
    FVector CalculateObstacleAvoidance(const FTransformFragment& Transform,
                                      const FMassVelocityFragment& Velocity);

    FVector CalculateTerrainFollowing(const FTransformFragment& Transform);

    // Utility functions
    bool IsInSameHerd(uint32 HerdID1, uint32 HerdID2);
    bool IsPredatorOfType(uint8 PredatorType, uint8 PreyType);
    float CalculateStaminaModifier(const FDinosaurStaminaFragment& Stamina);
    void UpdateStamina(FDinosaurStaminaFragment& Stamina, float DeltaTime, bool bIsRunning);

    // Performance optimization
    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxEntitiesPerFrame = 1000;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float ProcessingFrequency = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Flocking")
    float MaxFlockingDistance = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float PanicSpeedMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float ExhaustionSpeedMultiplier = 0.3f;

    float LastProcessTime = 0.0f;
};