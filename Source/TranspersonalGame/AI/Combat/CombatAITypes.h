#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CombatAITypes.generated.h"

// Forward Declarations
class UBehaviorTree;
class UBlackboardComponent;

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
enum class EAttackType : uint8
{
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    Tail            UMETA(DisplayName = "Tail Whip"),
    Charge          UMETA(DisplayName = "Charge"),
    Stomp           UMETA(DisplayName = "Stomp"),
    Roar            UMETA(DisplayName = "Intimidating Roar"),
    Spit            UMETA(DisplayName = "Spit Attack"),
    Pounce          UMETA(DisplayName = "Pounce")
};

USTRUCT(BlueprintType)
struct FDinosaurCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float HearingRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float SmellRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float TerritoryRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float StaminaMax = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float StaminaRegenRate = 10.0f;

    FDinosaurCombatStats()
    {
        Health = 100.0f;
        AttackDamage = 25.0f;
        AttackRange = 200.0f;
        MovementSpeed = 300.0f;
        DetectionRange = 1500.0f;
        HearingRange = 2000.0f;
        SmellRange = 1000.0f;
        TerritoryRadius = 3000.0f;
        AttackCooldown = 2.0f;
        StaminaMax = 100.0f;
        StaminaRegenRate = 10.0f;
    }
};

USTRUCT(BlueprintType)
struct FAttackPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    EAttackType AttackType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    float Range;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    float Cooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    float StaminaCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    float WindupTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    float RecoveryTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    bool bRequiresLineOfSight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Pattern")
    FGameplayTagContainer RequiredTags;

    FAttackPattern()
    {
        AttackType = EAttackType::Bite;
        Damage = 25.0f;
        Range = 200.0f;
        Cooldown = 2.0f;
        StaminaCost = 20.0f;
        WindupTime = 0.5f;
        RecoveryTime = 1.0f;
        bRequiresLineOfSight = true;
    }
};

USTRUCT(BlueprintType)
struct FDinosaurCombatProfile : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FString DinosaurName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    EDinosaurThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FGameplayTagContainer DinosaurTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    FDinosaurCombatStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Patterns")
    TArray<FAttackPattern> AttackPatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TSoftObjectPtr<UBehaviorTree> CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CautiousLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TerritorialLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bPackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bAmbushPredator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bScavenger;

    FDinosaurCombatProfile()
    {
        DinosaurName = TEXT("Unknown Dinosaur");
        ThreatLevel = EDinosaurThreatLevel::Passive;
        AggressionLevel = 0.5f;
        CautiousLevel = 0.5f;
        TerritorialLevel = 0.5f;
        bPackHunter = false;
        bAmbushPredator = false;
        bScavenger = false;
    }
};

USTRUCT(BlueprintType)
struct FCombatMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* LastKnownTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownTargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeSinceLastSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeSinceLastAttack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 AttackAttempts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bPlayerEscapedBefore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 CurrentPatrolIndex;

    FCombatMemory()
    {
        LastKnownTarget = nullptr;
        LastKnownTargetLocation = FVector::ZeroVector;
        TimeSinceLastSeen = 0.0f;
        TimeSinceLastAttack = 0.0f;
        AttackAttempts = 0;
        bPlayerEscapedBefore = false;
        CurrentPatrolIndex = 0;
    }
};