#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTags.h"
#include "CombatAITypes.generated.h"

UENUM(BlueprintType)
enum class EDinosaurArchetype : uint8
{
    None            UMETA(DisplayName = "None"),
    ApexPredator    UMETA(DisplayName = "Apex Predator"),      // T-Rex, Giganotosaurus
    PackHunter      UMETA(DisplayName = "Pack Hunter"),        // Velociraptor, Utahraptor
    AmbushPredator  UMETA(DisplayName = "Ambush Predator"),    // Carnotaurus, Baryonyx
    TerritorialHerbivore UMETA(DisplayName = "Territorial Herbivore"), // Triceratops, Ankylosaurus
    HerdHerbivore   UMETA(DisplayName = "Herd Herbivore"),     // Parasaurolophus, Brachiosaurus
    SmallHerbivore  UMETA(DisplayName = "Small Herbivore"),    // Gallimimus, Compsognathus
    AerialPredator  UMETA(DisplayName = "Aerial Predator"),    // Pteranodon, Quetzalcoatlus
    AquaticPredator UMETA(DisplayName = "Aquatic Predator")    // Mosasaurus, Plesiosaur
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Passive         UMETA(DisplayName = "Passive"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Resting         UMETA(DisplayName = "Resting")
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
enum class EHuntingStrategy : uint8
{
    Direct      UMETA(DisplayName = "Direct Assault"),
    Ambush      UMETA(DisplayName = "Ambush"),
    Pack        UMETA(DisplayName = "Pack Coordination"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Territorial UMETA(DisplayName = "Territorial Defense")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Courage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Intelligence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float TerritorialRadius = 1000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float SizeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float SpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float HealthMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float AggressionModifier = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FLinearColor ColorTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    TArray<FString> PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FString UniqueIdentifier;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurArchetypeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    EDinosaurArchetype Archetype = EDinosaurArchetype::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString ArchetypeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombatStats BaseCombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EHuntingStrategy PreferredStrategy = EHuntingStrategy::Direct;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<EHuntingStrategy> AlternativeStrategies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsTerritorial = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsNocturnal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PackSize = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
    FGameplayTagContainer DinosaurTags;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChanged, ECombatState, OldState, ECombatState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnThreatLevelChanged, EThreatLevel, OldLevel, EThreatLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetAcquired, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetLost, AActor*, Target);