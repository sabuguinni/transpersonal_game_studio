#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CombatAITypes.generated.h"

// Enum para tipos de comportamento de combate
UENUM(BlueprintType)
enum class ECombatBehaviorType : uint8
{
    Passive         UMETA(DisplayName = "Passive"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Predator        UMETA(DisplayName = "Predator"),
    PackHunter      UMETA(DisplayName = "Pack Hunter"),
    Ambush          UMETA(DisplayName = "Ambush"),
    Scavenger       UMETA(DisplayName = "Scavenger")
};

// Enum para estados de combate
UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Engaging        UMETA(DisplayName = "Engaging"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Retreating      UMETA(DisplayName = "Retreating"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Resting         UMETA(DisplayName = "Resting")
};

// Enum para tipos de ataque
UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    Tail            UMETA(DisplayName = "Tail Whip"),
    Charge          UMETA(DisplayName = "Charge"),
    Stomp           UMETA(DisplayName = "Stomp"),
    Spit            UMETA(DisplayName = "Spit"),
    Roar            UMETA(DisplayName = "Intimidating Roar"),
    Grapple         UMETA(DisplayName = "Grapple")
};

// Struct para configuração de ataque
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatAttackConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAttackType AttackType = EAttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Range = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindupTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecoveryTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresLineOfSight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer AttackTags;
};

// Struct para configuração de percepção
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatPerceptionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SightAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HearingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SmellRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LoseTargetTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanSenseWhenDamaged = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageAlertRadius = 1000.0f;
};

// Struct para configuração de combate
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatAIConfig : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECombatBehaviorType BehaviorType = ECombatBehaviorType::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FCombatPerceptionConfig PerceptionConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FCombatAttackConfig> AvailableAttacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredCombatDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RetreatHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanCallForHelp = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HelpCallRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer CombatTags;
};

// Struct para dados de combate em runtime
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatRuntimeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    ECombatState CurrentState = ECombatState::Idle;

    UPROPERTY(BlueprintReadWrite)
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float TimeSinceLastTargetSeen = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float CurrentAggression = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float CurrentFear = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bInCombat = false;

    UPROPERTY(BlueprintReadWrite)
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    TArray<AActor*> AlliesInRange;

    UPROPERTY(BlueprintReadWrite)
    TArray<AActor*> EnemiesInRange;
};