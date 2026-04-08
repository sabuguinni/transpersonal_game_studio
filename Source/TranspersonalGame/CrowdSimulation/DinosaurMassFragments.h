#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "DinosaurCrowdSubsystem.h"
#include "DinosaurMassFragments.generated.h"

/**
 * Mass Entity fragments specific to dinosaur crowd simulation
 * These fragments define the data components for individual dinosaur entities
 */

// Core dinosaur identity and behavior
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    // Species identification
    UPROPERTY(EditAnywhere)
    EDinosaurSpecies Species = EDinosaurSpecies::Triceratops;

    // Individual variation within species
    UPROPERTY(EditAnywhere)
    float SizeVariation = 1.0f; // 0.8 to 1.2 multiplier

    UPROPERTY(EditAnywhere)
    FLinearColor ColorVariation = FLinearColor::White;

    // Unique physical traits for recognition
    UPROPERTY(EditAnywhere)
    int32 UniqueTraitSeed = 0; // Used for procedural trait generation

    UPROPERTY(EditAnywhere)
    float AggressionLevel = 0.5f; // 0.0 = peaceful, 1.0 = highly aggressive

    UPROPERTY(EditAnywhere)
    float FearLevel = 0.5f; // 0.0 = fearless, 1.0 = very skittish
};

// Dinosaur behavioral state
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    EDinosaurBehaviorState CurrentBehavior = EDinosaurBehaviorState::Grazing;

    UPROPERTY(EditAnywhere)
    float BehaviorTimer = 0.0f; // Time in current behavior

    UPROPERTY(EditAnywhere)
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere)
    float MovementSpeed = 200.0f; // cm/s

    UPROPERTY(EditAnywhere)
    bool bInHerd = false;

    UPROPERTY(EditAnywhere)
    FMassEntityHandle HerdLeader; // Reference to herd leader entity

    UPROPERTY(EditAnywhere)
    int32 HerdID = -1; // Unique identifier for the herd
};

// Dinosaur needs and status
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurNeedsFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    float Hunger = 0.5f; // 0.0 = full, 1.0 = starving

    UPROPERTY(EditAnywhere)
    float Thirst = 0.5f; // 0.0 = hydrated, 1.0 = dehydrated

    UPROPERTY(EditAnywhere)
    float Energy = 1.0f; // 0.0 = exhausted, 1.0 = full energy

    UPROPERTY(EditAnywhere)
    float Health = 1.0f; // 0.0 = dead, 1.0 = full health

    UPROPERTY(EditAnywhere)
    float Stress = 0.0f; // 0.0 = calm, 1.0 = panicked

    // Time-based needs decay
    UPROPERTY(EditAnywhere)
    float HungerDecayRate = 0.1f; // Per minute

    UPROPERTY(EditAnywhere)
    float ThirstDecayRate = 0.15f; // Per minute

    UPROPERTY(EditAnywhere)
    float EnergyDecayRate = 0.05f; // Per minute
};

// Dinosaur memory and awareness
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurMemoryFragment : public FMassFragment
{
    GENERATED_BODY()

    // Known locations
    UPROPERTY(EditAnywhere)
    TArray<FVector> KnownWaterSources;

    UPROPERTY(EditAnywhere)
    TArray<FVector> KnownFoodSources;

    UPROPERTY(EditAnywhere)
    TArray<FVector> KnownDangerZones;

    // Recent experiences
    UPROPERTY(EditAnywhere)
    float LastThreatTime = -1.0f;

    UPROPERTY(EditAnywhere)
    FVector LastThreatLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere)
    float LastFeedTime = -1.0f;

    UPROPERTY(EditAnywhere)
    float LastDrinkTime = -1.0f;

    // Memory capacity (affects how many locations can be remembered)
    UPROPERTY(EditAnywhere)
    int32 MemoryCapacity = 10;
};

// Dinosaur social relationships
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurSocialFragment : public FMassFragment
{
    GENERATED_BODY()

    // Social bonds with other dinosaurs
    UPROPERTY(EditAnywhere)
    TArray<FMassEntityHandle> KnownAllies;

    UPROPERTY(EditAnywhere)
    TArray<FMassEntityHandle> KnownEnemies;

    // Social status within herd
    UPROPERTY(EditAnywhere)
    float DominanceRank = 0.5f; // 0.0 = submissive, 1.0 = alpha

    UPROPERTY(EditAnywhere)
    bool bIsHerdLeader = false;

    UPROPERTY(EditAnywhere)
    int32 FollowerCount = 0;

    // Domestication progress (for herbivores only)
    UPROPERTY(EditAnywhere)
    float DomesticationLevel = 0.0f; // 0.0 = wild, 1.0 = fully domesticated

    UPROPERTY(EditAnywhere)
    bool bPlayerInteracted = false;

    UPROPERTY(EditAnywhere)
    float TrustLevel = 0.0f; // Trust towards player
};

// Zone occupation and territory
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurZoneFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    int32 CurrentZoneID = -1;

    UPROPERTY(EditAnywhere)
    EDinosaurZoneType PreferredZoneType = EDinosaurZoneType::Grazing;

    UPROPERTY(EditAnywhere)
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere)
    float TerritoryRadius = 500.0f;

    UPROPERTY(EditAnywhere)
    bool bDefendingTerritory = false;

    UPROPERTY(EditAnywhere)
    float ZoneOccupancyTime = 0.0f; // How long in current zone
};

// Supporting enums for dinosaur simulation
UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // Herbivores - Large
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    
    // Herbivores - Medium
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Iguanodon       UMETA(DisplayName = "Iguanodon"),
    
    // Herbivores - Small
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Hypsilophodon   UMETA(DisplayName = "Hypsilophodon"),
    
    // Carnivores - Large
    TyrannosaurusRex UMETA(DisplayName = "T-Rex"),
    Allosaurus      UMETA(DisplayName = "Allosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    
    // Carnivores - Medium
    Carnotaurus     UMETA(DisplayName = "Carnotaurus"),
    Ceratosaurus    UMETA(DisplayName = "Ceratosaurus"),
    
    // Carnivores - Small
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Compsognathus   UMETA(DisplayName = "Compsognathus"),
    Deinonychus     UMETA(DisplayName = "Deinonychus")
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Grazing         UMETA(DisplayName = "Grazing"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Wandering       UMETA(DisplayName = "Wandering"),
    Following       UMETA(DisplayName = "Following Herd"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Fighting        UMETA(DisplayName = "Fighting"),
    Mating          UMETA(DisplayName = "Mating"),
    Nesting         UMETA(DisplayName = "Nesting"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Territorial     UMETA(DisplayName = "Defending Territory")
};