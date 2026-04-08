#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CombatAITypes.generated.h"

UENUM(BlueprintType)
enum class EDinosaurType : uint8
{
    None            UMETA(DisplayName = "None"),
    SmallHerbivore  UMETA(DisplayName = "Small Herbivore"),
    LargeHerbivore  UMETA(DisplayName = "Large Herbivore"),
    SmallCarnivore  UMETA(DisplayName = "Small Carnivore"),
    MediumCarnivore UMETA(DisplayName = "Medium Carnivore"),
    ApexPredator    UMETA(DisplayName = "Apex Predator")
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Passive         UMETA(DisplayName = "Passive"),
    Alert           UMETA(DisplayName = "Alert"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Territorial     UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Extreme     UMETA(DisplayName = "Extreme Threat")
};

UENUM(BlueprintType)
enum class EHuntingBehavior : uint8
{
    Ambush      UMETA(DisplayName = "Ambush Predator"),
    Pack        UMETA(DisplayName = "Pack Hunter"),
    Solo        UMETA(DisplayName = "Solo Hunter"),
    Opportunist UMETA(DisplayName = "Opportunistic"),
    Scavenger   UMETA(DisplayName = "Scavenger")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float HearingRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float SmellRange = 1000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurPersonality
{
    GENERATED_BODY()

    // Aggressiveness: 0.0 = Peaceful, 1.0 = Extremely Aggressive
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness = 0.5f;

    // Curiosity: 0.0 = Avoids everything, 1.0 = Investigates everything
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.3f;

    // Fear Response: 0.0 = Fearless, 1.0 = Extremely Skittish
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearResponse = 0.4f;

    // Territorial: 0.0 = Non-territorial, 1.0 = Highly territorial
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territorial = 0.6f;

    // Social: 0.0 = Solitary, 1.0 = Highly social
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Social = 0.2f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    EThreatLevel ThreatLevel = EThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    float ThreatDistance = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    bool bHasLineOfSight = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurArchetype : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    EDinosaurType DinosaurType = EDinosaurType::SmallHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    EHuntingBehavior HuntingBehavior = EHuntingBehavior::Solo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombatStats BaseCombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FDinosaurPersonality BasePersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FGameplayTag> BehaviorTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float DomesticationDifficulty = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TSoftObjectPtr<class UBehaviorTree> BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TSoftObjectPtr<class UBlackboardAsset> Blackboard;
};