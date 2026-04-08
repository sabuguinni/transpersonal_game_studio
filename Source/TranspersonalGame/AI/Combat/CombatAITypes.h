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
    ApexPredator    UMETA(DisplayName = "Apex Predator")
};

UENUM(BlueprintType)
enum class EDinosaurSize : uint8
{
    Tiny        UMETA(DisplayName = "Tiny"),      // < 1m
    Small       UMETA(DisplayName = "Small"),     // 1-3m
    Medium      UMETA(DisplayName = "Medium"),    // 3-8m
    Large       UMETA(DisplayName = "Large"),     // 8-15m
    Massive     UMETA(DisplayName = "Massive")    // > 15m
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
enum class EAttackPattern : uint8
{
    Ambush          UMETA(DisplayName = "Ambush"),
    DirectAssault   UMETA(DisplayName = "Direct Assault"),
    CircleAndStrike UMETA(DisplayName = "Circle and Strike"),
    PackHunt        UMETA(DisplayName = "Pack Hunt"),
    TerritorialChase UMETA(DisplayName = "Territorial Chase"),
    OpportunisticStrike UMETA(DisplayName = "Opportunistic Strike")
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
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SmellRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float StaminaMax = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float StaminaRegenRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FearThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 50.0f;

    FCombatStats()
    {
        Health = 100.0f;
        AttackDamage = 25.0f;
        AttackRange = 200.0f;
        MovementSpeed = 400.0f;
        DetectionRange = 1000.0f;
        HearingRange = 1500.0f;
        SmellRange = 800.0f;
        StaminaMax = 100.0f;
        StaminaRegenRate = 10.0f;
        FearThreshold = 30.0f;
        AggressionLevel = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurArchetype : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EDinosaurThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EDinosaurSize SizeCategory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    bool bCanBeDomesticated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    bool bIsPackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombatStats BaseCombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<EAttackPattern> PreferredAttackPatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FGameplayTagContainer BehaviorTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TerritorialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PreferredPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<FString> PreferredBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bNocturnal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bAquatic;

    FDinosaurArchetype()
    {
        SpeciesName = "Unknown Species";
        ThreatLevel = EDinosaurThreatLevel::Passive;
        SizeCategory = EDinosaurSize::Medium;
        bCanBeDomesticated = false;
        bIsPackHunter = false;
        TerritorialRadius = 500.0f;
        PreferredPackSize = 1.0f;
        bNocturnal = false;
        bAquatic = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TMap<AActor*, float> ThreatLevels;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TMap<AActor*, FVector> LastKnownLocations;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TMap<AActor*, float> LastSeenTime;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FVector> DangerousLocations;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FVector> SafeLocations;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float PlayerLastDamageTime;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    int32 PlayerEncounterCount;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    bool bPlayerIsKnownThreat;

    FCombatMemory()
    {
        PlayerLastDamageTime = -1.0f;
        PlayerEncounterCount = 0;
        bPlayerIsKnownThreat = false;
    }
};