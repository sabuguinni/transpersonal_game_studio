#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "DinosaurMassFragments_CrowdSimulation.generated.h"

// Dinosaur behavior states
UENUM(BlueprintType)
enum class ECrowdSim_DinosaurBehaviorState_98F : uint8
{
    Idle,
    Wandering,
    Grazing,
    Moving,
    Searching,
    Drinking,
    Resting,
    Hunting,
    Fleeing,
    Fighting,
    Mating,
    Nesting
};

// Dinosaur species types
UENUM(BlueprintType)
enum class ECrowdSim_DinosaurSpeciesType : uint8
{
    SmallHerbivore,     // Compsognathus, Hypsilophodon
    MediumHerbivore,    // Parasaurolophus, Triceratops
    LargeHerbivore,     // Brontosaurus, Diplodocus
    SmallCarnivore,     // Velociraptor, Deinonychus
    MediumCarnivore,    // Allosaurus, Carnotaurus
    LargeCarnivore,     // T-Rex, Giganotosaurus
    Flying,             // Pteranodon, Quetzalcoatlus
    Aquatic             // Mosasaurus, Plesiosaur
};

/**
 * Fragment containing species-specific data for dinosaurs
 */
USTRUCT()
struct TRANSPERSONALGAME_API FCrowdSim_DinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    // Species type determines behavior patterns
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdSim_DinosaurSpeciesType SpeciesType = ECrowdSim_DinosaurSpeciesType::SmallHerbivore;

    // Visual variation ID for this individual
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 VariationID = 0;

    // Size multiplier from base species size
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SizeMultiplier = 1.0f;

    // Speed multiplier from base species speed
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeedMultiplier = 1.0f;

    // Aggression level (0.0 = peaceful, 1.0 = highly aggressive)
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.3f;

    // Fear threshold (how easily spooked)
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearThreshold = 0.5f;

    // Hunger level (affects behavior)
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HungerLevel = 0.0f;

    // Age (affects behavior and appearance)
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Age = 1.0f; // 0.0 = juvenile, 1.0 = adult, 2.0 = elder
};

/**
 * Fragment containing current behavior state and timers
 */
USTRUCT()
struct TRANSPERSONALGAME_API FCrowdSim_DinosaurBehaviorFragment_98F : public FMassFragment
{
    GENERATED_BODY()

    // Current behavior state
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdSim_DinosaurBehaviorState_98F CurrentState = ECrowdSim_DinosaurBehaviorState_98F::Idle;

    // Previous behavior state (for transitions)
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdSim_DinosaurBehaviorState_98F PreviousState = ECrowdSim_DinosaurBehaviorState_98F::Idle;

    // Time in current state
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateTime = 0.0f;

    // Target location for current behavior
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    // Behavior-specific timer
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BehaviorTimer = 0.0f;

    // Alert level (0.0 = calm, 1.0 = maximum alert)
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlertLevel = 0.0f;

    // Last known threat position
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ThreatPosition = FVector::ZeroVector;

    // Time since last threat detected
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSinceThreat = 999.0f;
};

/**
 * Fragment for dinosaurs that belong to herds
 */
USTRUCT()
struct TRANSPERSONALGAME_API FCrowdSim_DinosaurHerdFragment_98F : public FMassFragment
{
    GENERATED_BODY()

    // Unique ID for this herd
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdID = 0;

    // Role in herd (0 = follower, 1 = leader, 2 = scout)
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdRole = 0;

    // Preferred distance from herd center
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredDistance = 500.0f;

    // Herd loyalty (how likely to stay with herd vs wander)
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HerdLoyalty = 0.8f;

    // Last known herd center position
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HerdCenter = FVector::ZeroVector;

    // Number of herd members last seen nearby
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NearbyHerdMembers = 0;
};

/**
 * Fragment for territory-based dinosaurs
 */
USTRUCT()
struct TRANSPERSONALGAME_API FCrowdSim_DinosaurTerritoryFragment : public FMassFragment
{
    GENERATED_BODY()

    // Center of territory
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TerritoryCenter = FVector::ZeroVector;

    // Territory radius
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 2000.0f;

    // How aggressively defends territory
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorialAggression = 0.5f;

    // Patrol points within territory
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PatrolPoints;

    // Current patrol target index
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentPatrolIndex = 0;
};

/**
 * Fragment for dinosaur daily routines
 */
USTRUCT()
struct TRANSPERSONALGAME_API FCrowdSim_DinosaurRoutineFragment : public FMassFragment
{
    GENERATED_BODY()

    // Preferred feeding time (0.0 = dawn, 0.5 = noon, 1.0 = midnight)
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredFeedingTime = 0.3f;

    // Preferred resting time
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredRestingTime = 0.9f;

    // Water source location
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WaterSource = FVector::ZeroVector;

    // Feeding area location
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FeedingArea = FVector::ZeroVector;

    // Resting area location
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RestingArea = FVector::ZeroVector;

    // Last feeding time
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastFeedingTime = 0.0f;

    // Last drinking time
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastDrinkingTime = 0.0f;
};

// Tags for different dinosaur types
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurHerbivoreTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurCarnivoreTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurSolitaryTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurHerdTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurTerritorialTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurFlyingTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurAquaticTag : public FMassTag
{
    GENERATED_BODY()
};