#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "DinosaurBehaviorTypes.generated.h"

/**
 * Enum defining the basic behavioral archetypes for dinosaurs
 */
UENUM(BlueprintType)
enum class EDinosaurArchetype : uint8
{
    // Apex predators - T-Rex, Allosaurus
    ApexPredator     UMETA(DisplayName = "Apex Predator"),
    
    // Pack hunters - Velociraptors, Deinonychus  
    PackHunter       UMETA(DisplayName = "Pack Hunter"),
    
    // Solitary hunters - Carnotaurus, Ceratosaurus
    SolitaryHunter   UMETA(DisplayName = "Solitary Hunter"),
    
    // Large herbivores - Triceratops, Brontosaurus
    LargeHerbivore   UMETA(DisplayName = "Large Herbivore"),
    
    // Herd herbivores - Parasaurolophus, Edmontosaurus
    HerdHerbivore    UMETA(DisplayName = "Herd Herbivore"),
    
    // Small herbivores (domesticable) - Compsognathus, Hypsilophodon
    SmallHerbivore   UMETA(DisplayName = "Small Herbivore"),
    
    // Scavengers - Ornithomimus, small theropods
    Scavenger        UMETA(DisplayName = "Scavenger"),
    
    // Ambush predators - Baryonyx, Suchomimus
    AmbushPredator   UMETA(DisplayName = "Ambush Predator")
};

/**
 * Current behavioral state of a dinosaur
 */
UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Mating          UMETA(DisplayName = "Mating"),
    Nesting         UMETA(DisplayName = "Nesting"),
    Migrating       UMETA(DisplayName = "Migrating"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Ambushing       UMETA(DisplayName = "Ambushing")
};

/**
 * Domestication levels for small herbivores
 */
UENUM(BlueprintType)
enum class EDomesticationLevel : uint8
{
    Wild            UMETA(DisplayName = "Wild"),
    Wary            UMETA(DisplayName = "Wary"),
    Curious         UMETA(DisplayName = "Curious"),
    Tolerant        UMETA(DisplayName = "Tolerant"),
    Friendly        UMETA(DisplayName = "Friendly"),
    Bonded          UMETA(DisplayName = "Bonded"),
    Domesticated    UMETA(DisplayName = "Domesticated")
};

/**
 * Individual personality traits that affect behavior
 */
USTRUCT(BlueprintType)
struct FDinosaurPersonality
{
    GENERATED_BODY()

    // Core personality traits (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;

    // Behavioral modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float ActivityLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float ReactionSpeed = 1.0f;

    FDinosaurPersonality()
    {
        // Generate random personality on construction
        Aggression = FMath::RandRange(0.0f, 1.0f);
        Curiosity = FMath::RandRange(0.0f, 1.0f);
        Fearfulness = FMath::RandRange(0.0f, 1.0f);
        Sociability = FMath::RandRange(0.0f, 1.0f);
        Territoriality = FMath::RandRange(0.0f, 1.0f);
        Intelligence = FMath::RandRange(0.3f, 1.0f);
        ActivityLevel = FMath::RandRange(0.7f, 1.5f);
        ReactionSpeed = FMath::RandRange(0.8f, 1.3f);
    }
};

/**
 * Memory entry for dinosaur interactions and observations
 */
USTRUCT(BlueprintType)
struct FDinosaurMemoryEntry
{
    GENERATED_BODY()

    // What was remembered
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag MemoryType;

    // Actor involved (if any)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<AActor> RelatedActor;

    // Location of the memory
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    // Emotional weight of the memory (-1.0 = very negative, 1.0 = very positive)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float EmotionalWeight = 0.0f;

    // When this memory was created
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;

    // How long this memory should persist
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 300.0f; // 5 minutes default

    // How important this memory is (affects retention)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Importance = 0.5f;

    FDinosaurMemoryEntry()
    {
        Timestamp = FPlatformTime::Seconds();
    }
};

/**
 * Daily routine schedule for dinosaurs
 */
USTRUCT(BlueprintType)
struct FDailyRoutineEntry
{
    GENERATED_BODY()

    // Time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TimeOfDay = 0.0f;

    // Preferred behavior at this time
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurBehaviorState PreferredBehavior = EDinosaurBehaviorState::Idle;

    // Priority of this routine (higher = more important)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Priority = 0.5f;

    // Duration of this behavior in hours
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "12.0"))
    float Duration = 1.0f;

    // Preferred location type for this behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag PreferredLocationTag;
};

/**
 * Data table row for dinosaur species behavior configuration
 */
USTRUCT(BlueprintType)
struct FDinosaurSpeciesData : public FTableRowBase
{
    GENERATED_BODY()

    // Species identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurArchetype Archetype = EDinosaurArchetype::LargeHerbivore;

    // Behavioral parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDailyRoutineEntry> DailyRoutine;

    // Detection ranges
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float SightRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "50.0", ClampMax = "2000.0"))
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "10.0", ClampMax = "500.0"))
    float SmellRange = 200.0f;

    // Social behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "50"))
    int32 PreferredGroupSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float TerritoryRadius = 1000.0f;

    // Domestication (only for small herbivores)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float DomesticationRate = 10.0f; // Points per positive interaction

    // Diet preferences
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> PreferredFoodTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> PreferredHabitatTags;

    // Threat assessment
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> NaturalEnemyTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> PreyTags;
};