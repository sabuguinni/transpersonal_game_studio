#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTags.h"
#include "CombatAITypes.generated.h"

/**
 * Enum defining different threat levels for dinosaurs
 */
UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    Passive         UMETA(DisplayName = "Passive"),           // Non-aggressive, flee on sight
    Territorial     UMETA(DisplayName = "Territorial"),       // Aggressive if player enters territory
    Hunter          UMETA(DisplayName = "Hunter"),            // Actively hunts player
    ApexPredator    UMETA(DisplayName = "Apex Predator")      // Dominant, relentless pursuit
};

/**
 * Enum defining combat behavioral archetypes
 */
UENUM(BlueprintType)
enum class ECombatArchetype : uint8
{
    Ambusher        UMETA(DisplayName = "Ambusher"),          // Stealth attacks, hit and run
    Berserker       UMETA(DisplayName = "Berserker"),         // Aggressive frontal assault
    Stalker         UMETA(DisplayName = "Stalker"),           // Patient tracking, psychological pressure
    PackHunter      UMETA(DisplayName = "Pack Hunter"),       // Coordinated group tactics
    Defender        UMETA(DisplayName = "Defender"),          // Protective, territorial
    Opportunist     UMETA(DisplayName = "Opportunist")        // Adapts tactics based on situation
};

/**
 * Enum defining current combat states
 */
UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Engaging        UMETA(DisplayName = "Engaging"),
    Retreating      UMETA(DisplayName = "Retreating"),
    Circling        UMETA(DisplayName = "Circling"),
    Ambushing       UMETA(DisplayName = "Ambushing")
};

/**
 * Struct containing individual dinosaur combat characteristics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurCombatProfile
{
    GENERATED_BODY()

    // Basic combat parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    EDinosaurThreatLevel ThreatLevel = EDinosaurThreatLevel::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    ECombatArchetype CombatArchetype = ECombatArchetype::Opportunist;

    // Detection and awareness
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float SightRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float SightAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "0.0", ClampMax = "3000.0"))
    float HearingRange = 800.0f;

    // Combat behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float Aggressiveness = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float Intelligence = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float Persistence = 5.0f;

    // Territory and movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float TerritoryRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float PreferredCombatDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float MovementSpeed = 400.0f;

    // Unique individual traits (procedurally generated)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Traits")
    FGameplayTagContainer IndividualTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Traits", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float SizeVariation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Traits", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float HealthVariation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Traits", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float DamageVariation = 1.0f;

    FDinosaurCombatProfile()
    {
        // Default constructor
    }
};

/**
 * Data table row for species-specific combat configurations
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurSpeciesCombatData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Data")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Data")
    FDinosaurCombatProfile BaseCombatProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Data")
    TArray<FGameplayTag> PossibleTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Data")
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Data")
    float DomesticationDifficulty = 5.0f;
};

/**
 * Struct for tracking combat memory and player interactions
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeSinceLastSighting = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float PlayerThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 NumberOfEncounters = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bPlayerHasWeapon = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bPlayerHasFire = false;

    FCombatMemoryEntry()
    {
        // Default constructor
    }
};