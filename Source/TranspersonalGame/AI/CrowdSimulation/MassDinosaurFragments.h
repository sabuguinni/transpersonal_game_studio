#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassDinosaurFragments.generated.h"

/**
 * Fragment for dinosaur species identification and characteristics
 */
USTRUCT()
struct TRANSPERSONALGAME_API FMassDinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    // Species type identifier
    UPROPERTY(EditAnywhere, Category = "Species")
    FName SpeciesName = NAME_None;

    // Diet type
    UPROPERTY(EditAnywhere, Category = "Species")
    uint8 bIsHerbivore : 1 = true;

    UPROPERTY(EditAnywhere, Category = "Species")
    uint8 bIsCarnivore : 1 = false;

    UPROPERTY(EditAnywhere, Category = "Species")
    uint8 bIsOmnivore : 1 = false;

    // Size category
    UPROPERTY(EditAnywhere, Category = "Species")
    uint8 SizeCategory = 1; // 1=Small, 2=Medium, 3=Large, 4=Massive

    // Behavior characteristics
    UPROPERTY(EditAnywhere, Category = "Behavior")
    float AggressionLevel = 0.5f; // 0.0 = Passive, 1.0 = Highly Aggressive

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float FlightResponse = 0.7f; // 0.0 = Never flees, 1.0 = Flees easily

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float SocialTendency = 0.8f; // 0.0 = Solitary, 1.0 = Highly social
};

/**
 * Fragment for individual dinosaur physical variations
 * Each dinosaur should be visually unique and identifiable
 */
USTRUCT()
struct TRANSPERSONALGAME_API FMassDinosaurVariationFragment : public FMassFragment
{
    GENERATED_BODY()

    // Unique identifier for this individual
    UPROPERTY(EditAnywhere, Category = "Identity")
    FGuid IndividualID;

    // Physical variations (0.0-1.0 normalized values)
    UPROPERTY(EditAnywhere, Category = "Physical")
    float SizeVariation = 0.5f; // Scale multiplier variation

    UPROPERTY(EditAnywhere, Category = "Physical")
    float ColorVariation = 0.5f; // Color pattern variation

    UPROPERTY(EditAnywhere, Category = "Physical")
    float FeatureVariation = 0.5f; // Horn size, crest shape, etc.

    // Behavioral variations
    UPROPERTY(EditAnywhere, Category = "Behavior")
    float PersonalityTrait = 0.5f; // Individual personality modifier

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float DominanceLevel = 0.5f; // Position in herd hierarchy

    // Health and condition
    UPROPERTY(EditAnywhere, Category = "Condition")
    float HealthLevel = 1.0f; // Current health (0.0-1.0)

    UPROPERTY(EditAnywhere, Category = "Condition")
    float HungerLevel = 0.5f; // Current hunger (0.0=starving, 1.0=full)

    UPROPERTY(EditAnywhere, Category = "Condition")
    float FatigueLevel = 0.0f; // Current fatigue (0.0=rested, 1.0=exhausted)
};

/**
 * Fragment for herd membership and social behaviors
 */
USTRUCT()
struct TRANSPERSONALGAME_API FMassDinosaurHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    // Herd identification
    UPROPERTY(EditAnywhere, Category = "Herd")
    int32 HerdID = -1; // -1 means no herd (solitary)

    UPROPERTY(EditAnywhere, Category = "Herd")
    uint8 bIsHerdLeader : 1 = false;

    UPROPERTY(EditAnywhere, Category = "Herd")
    uint8 bIsHerdScout : 1 = false;

    UPROPERTY(EditAnywhere, Category = "Herd")
    uint8 bIsProtector : 1 = false;

    // Social relationships
    UPROPERTY(EditAnywhere, Category = "Social")
    TArray<FMassEntityHandle> KnownAllies; // Friendly individuals

    UPROPERTY(EditAnywhere, Category = "Social")
    TArray<FMassEntityHandle> KnownThreats; // Remembered threats

    // Herd behavior state
    UPROPERTY(EditAnywhere, Category = "Behavior")
    float CohesionDesire = 0.8f; // Desire to stay with herd

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float SeparationDesire = 0.3f; // Desire for personal space

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float AlignmentDesire = 0.6f; // Desire to align with herd movement
};

/**
 * Fragment for dinosaur survival needs and states
 */
USTRUCT()
struct TRANSPERSONALGAME_API FMassDinosaurNeedsFragment : public FMassFragment
{
    GENERATED_BODY()

    // Basic needs (0.0-1.0, where 1.0 is maximum need)
    UPROPERTY(EditAnywhere, Category = "Needs")
    float FoodNeed = 0.3f;

    UPROPERTY(EditAnywhere, Category = "Needs")
    float WaterNeed = 0.2f;

    UPROPERTY(EditAnywhere, Category = "Needs")
    float RestNeed = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Needs")
    float SafetyNeed = 0.0f;

    // Current activity state
    UPROPERTY(EditAnywhere, Category = "Activity")
    uint8 CurrentActivity = 0; // 0=Idle, 1=Feeding, 2=Drinking, 3=Resting, 4=Migrating, 5=Fleeing, 6=Hunting

    // Activity timers
    UPROPERTY(EditAnywhere, Category = "Activity")
    float ActivityTimer = 0.0f; // Time spent in current activity

    UPROPERTY(EditAnywhere, Category = "Activity")
    float LastFeedTime = 0.0f; // Game time when last fed

    UPROPERTY(EditAnywhere, Category = "Activity")
    float LastDrinkTime = 0.0f; // Game time when last drank

    UPROPERTY(EditAnywhere, Category = "Activity")
    float LastRestTime = 0.0f; // Game time when last rested
};

/**
 * Fragment for dinosaur awareness and perception
 */
USTRUCT()
struct TRANSPERSONALGAME_API FMassDinosaurAwarenessFragment : public FMassFragment
{
    GENERATED_BODY()

    // Perception ranges (in Unreal units)
    UPROPERTY(EditAnywhere, Category = "Perception")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Perception")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Perception")
    float SmellRange = 1000.0f;

    // Current threat awareness
    UPROPERTY(EditAnywhere, Category = "Awareness")
    float ThreatLevel = 0.0f; // 0.0=calm, 1.0=maximum alert

    UPROPERTY(EditAnywhere, Category = "Awareness")
    FVector LastKnownThreatLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Awareness")
    float TimeSinceLastThreat = 0.0f;

    // Detected entities
    UPROPERTY(EditAnywhere, Category = "Awareness")
    TArray<FMassEntityHandle> VisibleEntities;

    UPROPERTY(EditAnywhere, Category = "Awareness")
    TArray<FMassEntityHandle> AudibleEntities;

    // Player tracking
    UPROPERTY(EditAnywhere, Category = "Player")
    uint8 bPlayerDetected : 1 = false;

    UPROPERTY(EditAnywhere, Category = "Player")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Player")
    float PlayerThreatAssessment = 0.5f; // How threatening player appears
};

/**
 * Fragment for dinosaur territory and migration patterns
 */
USTRUCT()
struct TRANSPERSONALGAME_API FMassDinosaurTerritoryFragment : public FMassFragment
{
    GENERATED_BODY()

    // Territory information
    UPROPERTY(EditAnywhere, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Territory")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "Territory")
    uint8 bIsTerritorial : 1 = false;

    // Migration patterns
    UPROPERTY(EditAnywhere, Category = "Migration")
    TArray<FVector> MigrationWaypoints;

    UPROPERTY(EditAnywhere, Category = "Migration")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, Category = "Migration")
    uint8 bIsMigrating : 1 = false;

    // Preferred locations
    UPROPERTY(EditAnywhere, Category = "Preferences")
    FVector PreferredFeedingLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Preferences")
    FVector PreferredWateringLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Preferences")
    FVector PreferredRestingLocation = FVector::ZeroVector;
};