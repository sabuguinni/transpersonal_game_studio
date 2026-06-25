#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "RaptorPackBehaviorTree.generated.h"

// ============================================================
// Raptor Pack Combat AI — Agent #12 Combat & Enemy AI
// Three-raptor flanking coordination system
// ============================================================

UENUM(BlueprintType)
enum class ECombat_RaptorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Flanking        UMETA(DisplayName = "Flanking"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Retreating      UMETA(DisplayName = "Retreating"),
    PackCoordinate  UMETA(DisplayName = "PackCoordinate")
};

UENUM(BlueprintType)
enum class ECombat_RaptorRole : uint8
{
    Alpha           UMETA(DisplayName = "Alpha"),
    FlankLeft       UMETA(DisplayName = "FlankLeft"),
    FlankRight      UMETA(DisplayName = "FlankRight")
};

// ============================================================
// Raptor Pack Coordinator — shared state between 3 raptors
// ============================================================
USTRUCT(BlueprintType)
struct FCombat_RaptorPackState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    AActor* TargetActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    bool bPackEngaged = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    bool bAlphaAttacking = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    float LastPackAttackTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    int32 ActiveRaptorCount = 3;
};

// ============================================================
// BT Task: Raptor Patrol
// ============================================================
UCLASS()
class TRANSPERSONALGAME_API UCombat_BTTask_RaptorPatrol : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UCombat_BTTask_RaptorPatrol();

    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float PatrolRadius = 2500.0f;

    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float MinStepDistance = 500.0f;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

// ============================================================
// BT Task: Raptor Flank — moves to flanking position relative to target
// ============================================================
UCLASS()
class TRANSPERSONALGAME_API UCombat_BTTask_RaptorFlank : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UCombat_BTTask_RaptorFlank();

    /** Angle offset from target's forward vector (degrees). Alpha=0, FlankL=90, FlankR=-90 */
    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float FlankAngleDegrees = 90.0f;

    /** Distance from target to maintain flanking position */
    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float FlankDistance = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    ECombat_RaptorRole RaptorRole = ECombat_RaptorRole::Alpha;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

// ============================================================
// BT Task: Raptor Attack — quick slash, low damage, high frequency
// ============================================================
UCLASS()
class TRANSPERSONALGAME_API UCombat_BTTask_RaptorAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UCombat_BTTask_RaptorAttack();

    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float SlashDamage = 25.0f;

    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float AttackRange = 180.0f;

    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float AttackCooldown = 1.2f;

    /** Only flankers attack when alpha is distracting */
    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    bool bRequiresAlphaDistraction = false;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
    float LastAttackTime = 0.0f;
};

// ============================================================
// BT Task: Raptor Chase — 2.2× speed multiplier
// ============================================================
UCLASS()
class TRANSPERSONALGAME_API UCombat_BTTask_RaptorChase : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UCombat_BTTask_RaptorChase();

    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float ChaseSpeedMultiplier = 2.2f;

    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float GiveUpDistance = 4000.0f;

    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float AcceptanceRadius = 200.0f;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

// ============================================================
// BT Decorator: Raptor Sense Player — sight + sound detection
// ============================================================
UCLASS()
class TRANSPERSONALGAME_API UCombat_BTDecorator_RaptorSensePlayer : public UBTDecorator
{
    GENERATED_BODY()

public:
    UCombat_BTDecorator_RaptorSensePlayer();

    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float SightAngleDegrees = 120.0f;

    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float SoundDetectionRadius = 1200.0f;

    /** Velocity threshold above which raptors hear the player (cm/s) */
    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float SoundVelocityThreshold = 80.0f;

    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};

// ============================================================
// BT Task: Raptor Retreat — pack retreats if alpha is downed
// ============================================================
UCLASS()
class TRANSPERSONALGAME_API UCombat_BTTask_RaptorRetreat : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UCombat_BTTask_RaptorRetreat();

    UPROPERTY(EditAnywhere, Category = "Combat|Raptor")
    float RetreatDistance = 3000.0f;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
