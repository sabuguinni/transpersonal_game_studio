#pragma once

#include "CoreMinimal.h"
#include "NPCBehaviorSystem.h"
#include "DinosaurArchetypes.generated.h"

/**
 * Dinosaur size categories affecting behavior
 */
UENUM(BlueprintType)
enum class EDinosaurSize : uint8
{
    Tiny        UMETA(DisplayName = "Tiny (< 1m)"),
    Small       UMETA(DisplayName = "Small (1-3m)"),
    Medium      UMETA(DisplayName = "Medium (3-8m)"),
    Large       UMETA(DisplayName = "Large (8-15m)"),
    Massive     UMETA(DisplayName = "Massive (> 15m)")
};

/**
 * Dinosaur diet types
 */
UENUM(BlueprintType)
enum class EDinosaurDiet : uint8
{
    Herbivore       UMETA(DisplayName = "Herbivore"),
    Carnivore       UMETA(DisplayName = "Carnivore"),
    Omnivore        UMETA(DisplayName = "Omnivore"),
    Piscivore       UMETA(DisplayName = "Piscivore (Fish-eater)"),
    Insectivore     UMETA(DisplayName = "Insectivore")
};

/**
 * Social behavior patterns
 */
UENUM(BlueprintType)
enum class ESocialPattern : uint8
{
    Solitary        UMETA(DisplayName = "Solitary"),
    Pair            UMETA(DisplayName = "Pair Bonded"),
    SmallGroup      UMETA(DisplayName = "Small Group (3-8)"),
    Herd            UMETA(DisplayName = "Herd (9-30)"),
    MegaHerd        UMETA(DisplayName = "Mega Herd (30+)")
};

/**
 * Predefined dinosaur archetype configurations
 */
USTRUCT(BlueprintType)
struct FDinosaurArchetype
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ArchetypeName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSize Size = EDinosaurSize::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurDiet Diet = EDinosaurDiet::Herbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESocialPattern SocialBehavior = ESocialPattern::Solitary;

    // Personality template for this archetype
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FNPCPersonality BasePersonality;

    // Behavioral preferences
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeThreshold = 0.3f; // Safety level that triggers fleeing

    // Preferred activities and timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDailyRoutineEntry> DefaultRoutine;

    FDinosaurArchetype()
    {
        ArchetypeName = TEXT("Generic Dinosaur");
        Size = EDinosaurSize::Medium;
        Diet = EDinosaurDiet::Herbivore;
        SocialBehavior = ESocialPattern::Solitary;
        BasePersonality = FNPCPersonality();
        bCanBeDomesticated = false;
        TerritoryRadius = 1000.0f;
        DetectionRange = 500.0f;
        FleeThreshold = 0.3f;
    }
};

/**
 * Dinosaur Archetype Library
 * Contains predefined behavioral templates for different dinosaur species
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurArchetypeLibrary : public UObject
{
    GENERATED_BODY()

public:
    UDinosaurArchetypeLibrary();

    // Predefined archetypes
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Archetypes")
    TMap<FString, FDinosaurArchetype> Archetypes;

    // Get archetype by name
    UFUNCTION(BlueprintCallable, Category = "Archetypes")
    FDinosaurArchetype GetArchetype(const FString& ArchetypeName);

    // Create personality variation from archetype
    UFUNCTION(BlueprintCallable, Category = "Archetypes")
    FNPCPersonality CreatePersonalityVariation(const FDinosaurArchetype& Archetype, float VariationAmount = 0.2f);

    // Initialize default archetypes
    UFUNCTION(BlueprintCallable, Category = "Archetypes")
    void InitializeDefaultArchetypes();

private:
    void CreateSmallHerbivoreArchetype();
    void CreateLargeCarnivoreArchetype();
    void CreatePackHunterArchetype();
    void CreateGentleGiantArchetype();
    void CreateSkittishPreyArchetype();
    void CreateApexPredatorArchetype();
    void CreateSocialHerbivoreArchetype();
    void CreateSolitaryScavengerArchetype();
};