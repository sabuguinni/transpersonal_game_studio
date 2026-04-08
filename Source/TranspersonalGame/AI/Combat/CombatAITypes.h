#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTags.h"
#include "CombatAITypes.generated.h"

UENUM(BlueprintType)
enum class ECombatAIState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Hunting,
    Attacking,
    Fleeing,
    Feeding,
    Resting,
    Territorial,
    PackHunting
};

UENUM(BlueprintType)
enum class ECombatThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Extreme
};

UENUM(BlueprintType)
enum class EDinosaurArchetype : uint8
{
    ApexPredator,      // T-Rex, Giganotosaurus
    PackHunter,        // Velociraptors, Deinonychus
    AmbushPredator,    // Carnotaurus, Baryonyx
    Herbivore_Large,   // Triceratops, Brontosaurus
    Herbivore_Small,   // Parasaurolophus, Gallimimus
    Scavenger,         // Compsognathus swarms
    Aquatic,           // Mosasaurus, Plesiosaur
    Flying             // Pteranodon, Quetzalcoatlus
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatAIStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StaminaMax = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearThreshold = 0.3f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatBehaviorData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FCombatAIStats BaseStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> BehaviorTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PackCoordinationRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PreferredPackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeIntimidated = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bTerritorial = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 2000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<AActor> LastKnownPlayerLocation;

    UPROPERTY(BlueprintReadWrite)
    FVector LastSeenPlayerPosition;

    UPROPERTY(BlueprintReadWrite)
    float TimeSinceLastSighting = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    ECombatThreatLevel PerceivedThreatLevel = ECombatThreatLevel::None;

    UPROPERTY(BlueprintReadWrite)
    TArray<TWeakObjectPtr<AActor>> KnownAllies;

    UPROPERTY(BlueprintReadWrite)
    TArray<TWeakObjectPtr<AActor>> KnownThreats;

    UPROPERTY(BlueprintReadWrite)
    bool bPlayerHasWeapon = false;

    UPROPERTY(BlueprintReadWrite)
    bool bPlayerNearFire = false;
};