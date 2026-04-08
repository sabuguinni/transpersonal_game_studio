#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "DinosaurBehaviorTypes.generated.h"

/**
 * Core behavior types and enums for the dinosaur AI system
 * Based on the principle that each dinosaur lives its own life independent of the player
 */

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // Herbivores - Small (Domesticable)
    Compsognathus     UMETA(DisplayName = "Compsognathus"),
    Dryosaurus        UMETA(DisplayName = "Dryosaurus"),
    Hypsilophodon     UMETA(DisplayName = "Hypsilophodon"),
    
    // Herbivores - Medium
    Parasaurolophus   UMETA(DisplayName = "Parasaurolophus"),
    Triceratops       UMETA(DisplayName = "Triceratops"),
    Stegosaurus       UMETA(DisplayName = "Stegosaurus"),
    
    // Herbivores - Large
    Brachiosaurus     UMETA(DisplayName = "Brachiosaurus"),
    Diplodocus        UMETA(DisplayName = "Diplodocus"),
    
    // Carnivores - Small
    Velociraptor      UMETA(DisplayName = "Velociraptor"),
    Dilophosaurus     UMETA(DisplayName = "Dilophosaurus"),
    
    // Carnivores - Medium
    Allosaurus        UMETA(DisplayName = "Allosaurus"),
    Carnotaurus       UMETA(DisplayName = "Carnotaurus"),
    
    // Carnivores - Large (Apex Predators)
    TyrannosaurusRex  UMETA(DisplayName = "Tyrannosaurus Rex"),
    Spinosaurus       UMETA(DisplayName = "Spinosaurus"),
    
    MAX_SPECIES       UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDinosaurSize : uint8
{
    Small       UMETA(DisplayName = "Small (0.5-2m)"),
    Medium      UMETA(DisplayName = "Medium (2-6m)"),
    Large       UMETA(DisplayName = "Large (6-12m)"),
    Massive     UMETA(DisplayName = "Massive (12m+)")
};

UENUM(BlueprintType)
enum class EDinosaurDiet : uint8
{
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
    Piscivore   UMETA(DisplayName = "Piscivore")
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    // Basic States
    Idle            UMETA(DisplayName = "Idle"),
    Wandering       UMETA(DisplayName = "Wandering"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Resting         UMETA(DisplayName = "Resting"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    
    // Social States
    Socializing     UMETA(DisplayName = "Socializing"),
    Mating          UMETA(DisplayName = "Mating"),
    Parenting       UMETA(DisplayName = "Parenting"),
    
    // Alert States
    Alert           UMETA(DisplayName = "Alert"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Hiding          UMETA(DisplayName = "Hiding"),
    
    // Aggressive States
    Hunting         UMETA(DisplayName = "Hunting"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Territorial     UMETA(DisplayName = "Territorial"),
    
    // Player Interaction States
    CuriousAboutPlayer    UMETA(DisplayName = "Curious About Player"),
    AvoidingPlayer        UMETA(DisplayName = "Avoiding Player"),
    TrustingPlayer        UMETA(DisplayName = "Trusting Player"),
    BondedWithPlayer      UMETA(DisplayName = "Bonded With Player"),
    
    // Special States
    Migrating       UMETA(DisplayName = "Migrating"),
    Injured         UMETA(DisplayName = "Injured"),
    Dying           UMETA(DisplayName = "Dying"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EDinosaurPersonality : uint8
{
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Timid           UMETA(DisplayName = "Timid"),
    Curious         UMETA(DisplayName = "Curious"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Social          UMETA(DisplayName = "Social"),
    Solitary        UMETA(DisplayName = "Solitary"),
    Protective      UMETA(DisplayName = "Protective"),
    Opportunistic   UMETA(DisplayName = "Opportunistic")
};

UENUM(BlueprintType)
enum class EDomesticationStage : uint8
{
    Wild            UMETA(DisplayName = "Wild"),
    Wary            UMETA(DisplayName = "Wary"),
    Curious         UMETA(DisplayName = "Curious"),
    Accepting       UMETA(DisplayName = "Accepting"),
    Trusting        UMETA(DisplayName = "Trusting"),
    Bonded          UMETA(DisplayName = "Bonded"),
    Domesticated    UMETA(DisplayName = "Domesticated")
};

USTRUCT(BlueprintType)
struct FDinosaurNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Safety = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Comfort = 100.0f;

    FDinosaurNeeds()
    {
        Hunger = 100.0f;
        Thirst = 100.0f;
        Energy = 100.0f;
        Social = 50.0f;
        Safety = 100.0f;
        Comfort = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    // Locations
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> FoodSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> WaterSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> SafeSpots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> DangerousAreas;

    // Entities
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TWeakObjectPtr<AActor>> KnownPredators;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TWeakObjectPtr<AActor>> KnownPrey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TWeakObjectPtr<AActor>> PackMembers;

    // Player relationship
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<APawn> PlayerReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerTrustLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastPlayerInteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PositivePlayerInteractions = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NegativePlayerInteractions = 0;
};

USTRUCT(BlueprintType)
struct FDinosaurPhysicalTraits
{
    GENERATED_BODY()

    // Size variations (multipliers from base)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float SizeMultiplier = 1.0f;

    // Color variations
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PrimaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SecondaryColor = FLinearColor::Gray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AccentColor = FLinearColor::Black;

    // Pattern variations
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PatternVariant = 0;

    // Physical features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float HornSizeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float TailLengthMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float NeckLengthMultiplier = 1.0f;

    // Scars and markings
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> ScarLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasLimp = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMissingTooth = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bBrokenHorn = false;
};

USTRUCT(BlueprintType)
struct FDinosaurSpeciesData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSize Size;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurDiet Diet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsApexPredator = false;

    // Base stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDamage = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeThreshold = 30.0f;

    // Behavior modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CuriosityLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialNeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorialRadius = 2000.0f;

    // Daily routine timing (hours 0-24)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredWakeTime = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredSleepTime = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredFeedingTime = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredDrinkingTime = 8.0f;
};