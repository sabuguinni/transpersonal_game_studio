#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "DinosaurBehaviorTask.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — global scope (RULE 1)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Graze       UMETA(DisplayName = "Graze"),
    Rest        UMETA(DisplayName = "Rest"),
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "T-Rex"),
    Velociraptor        UMETA(DisplayName = "Raptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs — global scope (RULE 1)
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_DinoMemoryEntry
{
    GENERATED_BODY()

    /** World location where the stimulus was detected */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector StimulusLocation = FVector::ZeroVector;

    /** Game time when the entry was recorded */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float RecordedTime = 0.f;

    /** Threat level 0-1 */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.f;

    /** True if the stimulus was the player */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsPlayer = false;
};

USTRUCT(BlueprintType)
struct FNPC_DinoPatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location = FVector::ZeroVector;

    /** Wait duration in seconds at this point */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitDuration = 3.f;
};

// ─────────────────────────────────────────────────────────────────────────────
// BT Task — Patrol to next waypoint
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "NPC|BehaviorTree", meta = (DisplayName = "BT Task: Dino Patrol"))
class TRANSPERSONALGAME_API UNPC_BTTask_DinoPatrol : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_DinoPatrol();

    /** Patrol radius around home location (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolRadius = 5000.f;

    /** Blackboard key for the patrol target location */
    UPROPERTY(EditAnywhere, Category = "NPC|Patrol")
    FBlackboardKeySelector PatrolTargetKey;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;
};

// ─────────────────────────────────────────────────────────────────────────────
// BT Task — Chase player
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "NPC|BehaviorTree", meta = (DisplayName = "BT Task: Dino Chase"))
class TRANSPERSONALGAME_API UNPC_BTTask_DinoChase : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_DinoChase();

    /** Chase speed multiplier applied to movement component */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Chase")
    float ChaseSpeedMultiplier = 1.8f;

    /** Blackboard key for the chase target actor */
    UPROPERTY(EditAnywhere, Category = "NPC|Chase")
    FBlackboardKeySelector ChaseTargetKey;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;
};

// ─────────────────────────────────────────────────────────────────────────────
// BT Task — Melee attack
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "NPC|BehaviorTree", meta = (DisplayName = "BT Task: Dino Attack"))
class TRANSPERSONALGAME_API UNPC_BTTask_DinoAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_DinoAttack();

    /** Attack range (cm) — abort if target is farther */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Attack")
    float AttackRange = 300.f;

    /** Base damage dealt per hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Attack")
    float BaseDamage = 40.f;

    /** Cooldown between attacks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Attack")
    float AttackCooldown = 2.f;

    /** Blackboard key for the attack target */
    UPROPERTY(EditAnywhere, Category = "NPC|Attack")
    FBlackboardKeySelector AttackTargetKey;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

private:
    float LastAttackTime = -999.f;
};

// ─────────────────────────────────────────────────────────────────────────────
// BT Task — Graze (herbivore idle behaviour)
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "NPC|BehaviorTree", meta = (DisplayName = "BT Task: Dino Graze"))
class TRANSPERSONALGAME_API UNPC_BTTask_DinoGraze : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_DinoGraze();

    /** Duration to graze at current location (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Graze")
    float GrazeDuration = 8.f;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

private:
    FTimerHandle GrazeTimerHandle;
};
