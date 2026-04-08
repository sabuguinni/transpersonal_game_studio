#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CombatAITypes.generated.h"

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    Passive         UMETA(DisplayName = "Passive"),
    Defensive       UMETA(DisplayName = "Defensive"), 
    Territorial     UMETA(DisplayName = "Territorial"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Apex            UMETA(DisplayName = "Apex Predator")
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Investigate     UMETA(DisplayName = "Investigate"),
    Stalk           UMETA(DisplayName = "Stalk"),
    Hunt            UMETA(DisplayName = "Hunt"),
    Attack          UMETA(DisplayName = "Attack"),
    Flee            UMETA(DisplayName = "Flee"),
    Wounded         UMETA(DisplayName = "Wounded"),
    Feeding         UMETA(DisplayName = "Feeding")
};

UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    Tail            UMETA(DisplayName = "Tail Whip"),
    Charge          UMETA(DisplayName = "Charge"),
    Stomp           UMETA(DisplayName = "Stomp"),
    Ambush          UMETA(DisplayName = "Ambush")
};

UENUM(BlueprintType)
enum class EHuntingStrategy : uint8
{
    Solitary        UMETA(DisplayName = "Solitary Hunter"),
    Pack            UMETA(DisplayName = "Pack Hunter"),
    Ambush          UMETA(DisplayName = "Ambush Predator"),
    Persistence     UMETA(DisplayName = "Persistence Hunter"),
    Opportunistic   UMETA(DisplayName = "Opportunistic")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float HearingRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SmellRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Aggression = 0.5f; // 0.0 = Passive, 1.0 = Extremely Aggressive

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Intelligence = 0.5f; // Affects tactical decisions

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Courage = 0.5f; // Affects flee threshold
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FString IndividualName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Territoriality = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float SocialTendency = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Patience = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Adaptability = 0.5f;

    // Physical variations for uniqueness
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float SizeVariation = 1.0f; // 0.8 to 1.2 typically

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    FLinearColor ColorVariation = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    TArray<FString> UniqueFeatures; // "Scar on left eye", "Broken horn", etc.
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TMap<AActor*, float> ThreatLevels; // Actor -> Threat Level (0.0 to 1.0)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TMap<FVector, float> DangerousLocations; // Location -> Danger Level

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> SafeLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastPlayerEncounterTime = -1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 PlayerEncounterCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bHasBeenWoundedByPlayer = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurCombatData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    EDinosaurThreatLevel ThreatLevel = EDinosaurThreatLevel::Defensive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    EHuntingStrategy HuntingStrategy = EHuntingStrategy::Solitary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombatStats BaseStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<EAttackType> AvailableAttacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeHealthThreshold = 0.3f; // Flee when health drops below this

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CallForHelpThreshold = 0.5f; // Call pack when health drops below this

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DomesticationDifficulty = 1.0f; // Higher = harder to domesticate

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
    FGameplayTagContainer CombatTags;
};