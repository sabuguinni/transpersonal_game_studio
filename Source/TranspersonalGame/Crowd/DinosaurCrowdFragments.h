#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "DinosaurCrowdFragments.generated.h"

// Forward declarations
enum class ECrowdType : uint8;
enum class ECrowdBehaviorState : uint8;

/**
 * Fragment that defines the dinosaur species and crowd type
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    ECrowdType SpeciesType = ECrowdType::HerbivoreHerd;

    UPROPERTY(EditAnywhere)
    float BodySize = 1.0f; // Relative size multiplier

    UPROPERTY(EditAnywhere)
    float MovementSpeed = 300.0f; // Base movement speed in cm/s

    UPROPERTY(EditAnywhere)
    float DetectionRange = 1000.0f; // How far they can detect threats/food

    UPROPERTY(EditAnywhere)
    bool bIsAlpha = false; // Is this the alpha of the group?
};

/**
 * Fragment for crowd behavior state and transitions
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    ECrowdBehaviorState CurrentState = ECrowdBehaviorState::Grazing;

    UPROPERTY(EditAnywhere)
    ECrowdBehaviorState PreviousState = ECrowdBehaviorState::Grazing;

    UPROPERTY(EditAnywhere)
    float StateTimer = 0.0f; // Time in current state

    UPROPERTY(EditAnywhere)
    float StateTransitionCooldown = 0.0f; // Prevent rapid state changes

    UPROPERTY(EditAnywhere)
    FVector TargetLocation = FVector::ZeroVector; // Current movement target

    UPROPERTY(EditAnywhere)
    float StressLevel = 0.0f; // 0.0 = calm, 1.0 = panicked
};

/**
 * Fragment for boids flocking behavior
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurFlockingFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FVector CohesionForce = FVector::ZeroVector; // Force towards group center

    UPROPERTY(EditAnywhere)
    FVector SeparationForce = FVector::ZeroVector; // Force away from nearby individuals

    UPROPERTY(EditAnywhere)
    FVector AlignmentForce = FVector::ZeroVector; // Force to match group direction

    UPROPERTY(EditAnywhere)
    FVector AvoidanceForce = FVector::ZeroVector; // Force to avoid obstacles/predators

    UPROPERTY(EditAnywhere)
    float CohesionRadius = 500.0f; // Distance to consider for cohesion

    UPROPERTY(EditAnywhere)
    float SeparationRadius = 100.0f; // Distance to maintain from others

    UPROPERTY(EditAnywhere)
    float AlignmentRadius = 300.0f; // Distance to consider for alignment

    UPROPERTY(EditAnywhere)
    int32 FlockID = 0; // Which flock this dinosaur belongs to
};

/**
 * Fragment for environmental needs and influences
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurNeedsFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    float Hunger = 0.0f; // 0.0 = full, 1.0 = starving

    UPROPERTY(EditAnywhere)
    float Thirst = 0.0f; // 0.0 = hydrated, 1.0 = dehydrated

    UPROPERTY(EditAnywhere)
    float Energy = 1.0f; // 0.0 = exhausted, 1.0 = full energy

    UPROPERTY(EditAnywhere)
    float LastFeedTime = 0.0f; // Game time when last fed

    UPROPERTY(EditAnywhere)
    float LastDrinkTime = 0.0f; // Game time when last drank

    UPROPERTY(EditAnywhere)
    float LastRestTime = 0.0f; // Game time when last rested

    UPROPERTY(EditAnywhere)
    FVector NearestWaterSource = FVector::ZeroVector;

    UPROPERTY(EditAnywhere)
    FVector NearestFoodSource = FVector::ZeroVector;
};

/**
 * Fragment for threat detection and response
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurThreatFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FVector ThreatLocation = FVector::ZeroVector; // Location of detected threat

    UPROPERTY(EditAnywhere)
    float ThreatLevel = 0.0f; // 0.0 = no threat, 1.0 = immediate danger

    UPROPERTY(EditAnywhere)
    float ThreatDetectionTime = 0.0f; // When threat was first detected

    UPROPERTY(EditAnywhere)
    float LastThreatCheckTime = 0.0f; // Last time we checked for threats

    UPROPERTY(EditAnywhere)
    bool bFleeingFromThreat = false;

    UPROPERTY(EditAnywhere)
    FVector FleeDirection = FVector::ZeroVector; // Direction to flee

    UPROPERTY(EditAnywhere)
    TArray<FMassEntityHandle> NearbyPredators; // Detected predator entities
};

/**
 * Fragment for migration behavior
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurMigrationFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    bool bIsMigrating = false;

    UPROPERTY(EditAnywhere)
    FVector MigrationTarget = FVector::ZeroVector; // Final migration destination

    UPROPERTY(EditAnywhere)
    TArray<FVector> MigrationWaypoints; // Route waypoints

    UPROPERTY(EditAnywhere)
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere)
    float MigrationStartTime = 0.0f; // When migration began

    UPROPERTY(EditAnywhere)
    float MigrationSpeed = 200.0f; // Speed during migration (usually slower)

    UPROPERTY(EditAnywhere)
    int32 MigrationSeason = 0; // Which seasonal migration this is
};

/**
 * Fragment for audio and visual representation
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurAudioVisualFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    float LastVocalizationTime = 0.0f; // When last made a sound

    UPROPERTY(EditAnywhere)
    float VocalizationCooldown = 5.0f; // Minimum time between sounds

    UPROPERTY(EditAnywhere)
    bool bShouldPlayMovementSound = false;

    UPROPERTY(EditAnywhere)
    bool bShouldPlayAlertSound = false;

    UPROPERTY(EditAnywhere)
    float AnimationBlendWeight = 1.0f; // For LOD animation blending

    UPROPERTY(EditAnywhere)
    int32 LODLevel = 0; // Current level of detail (0 = highest, 3 = lowest)
};