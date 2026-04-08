#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CombatAITypes.generated.h"

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    Passive         UMETA(DisplayName = "Passive - Herbivore"),
    Defensive       UMETA(DisplayName = "Defensive - Will defend if threatened"),
    Territorial     UMETA(DisplayName = "Territorial - Attacks if too close"),
    Aggressive      UMETA(DisplayName = "Aggressive - Actively hunts"),
    ApexPredator    UMETA(DisplayName = "Apex Predator - Dominates territory")
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Resting         UMETA(DisplayName = "Resting")
};

UENUM(BlueprintType)
enum class EDinosaurSize : uint8
{
    Tiny        UMETA(DisplayName = "Tiny (< 1m)"),
    Small       UMETA(DisplayName = "Small (1-3m)"),
    Medium      UMETA(DisplayName = "Medium (3-6m)"),
    Large       UMETA(DisplayName = "Large (6-12m)"),
    Massive     UMETA(DisplayName = "Massive (> 12m)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float StaminaMax = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float StaminaRegenRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FearThreshold = 30.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float SizeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FLinearColor PrimaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FLinearColor SecondaryColor = FLinearColor::Gray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float AggressionModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float SpeedModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    int32 UniqueID = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurArchetype : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    EDinosaurThreatLevel ThreatLevel = EDinosaurThreatLevel::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    EDinosaurSize Size = EDinosaurSize::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombatStats BaseStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FGameplayTagContainer BehaviorTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TSoftObjectPtr<class UBehaviorTree> BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TSoftObjectPtr<class UBlackboardAsset> Blackboard;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatEncounter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    FString EncounterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    TArray<FName> ParticipatingSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float TensionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float DifficultyScaling = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    bool bPlayerIsTarget = false;
};