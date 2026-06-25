#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "TRexBehaviorTree.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting")
};

// ============================================================
// BTTask: T-Rex Patrol — move to random point in patrol radius
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "NPC BT Task: TRex Patrol"))
class TRANSPERSONALGAME_API UNPC_BTTask_TRexPatrol : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_TRexPatrol();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    /** Radius around home location within which the T-Rex patrols */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Patrol")
    float PatrolRadius = 5000.0f;

    /** Minimum distance to move per patrol step */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Patrol")
    float MinPatrolStepDistance = 800.0f;

    /** Blackboard key for the patrol destination */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Patrol")
    FBlackboardKeySelector PatrolDestinationKey;

    /** Blackboard key for the home/spawn location */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Patrol")
    FBlackboardKeySelector HomeLocationKey;
};

// ============================================================
// BTTask: T-Rex Chase — pursue the player at full sprint speed
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "NPC BT Task: TRex Chase Player"))
class TRANSPERSONALGAME_API UNPC_BTTask_TRexChasePlayer : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_TRexChasePlayer();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    /** Chase speed multiplier applied to CharacterMovement MaxWalkSpeed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Chase")
    float ChaseSpeedMultiplier = 1.8f;

    /** Distance at which T-Rex gives up the chase */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Chase")
    float GiveUpDistance = 6000.0f;

    /** Blackboard key for the player/target actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Chase")
    FBlackboardKeySelector TargetActorKey;

    /** Blackboard key for last known player location */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Chase")
    FBlackboardKeySelector LastKnownPlayerLocationKey;
};

// ============================================================
// BTTask: T-Rex Attack — deal damage when within attack range
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "NPC BT Task: TRex Attack"))
class TRANSPERSONALGAME_API UNPC_BTTask_TRexAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_TRexAttack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    /** Damage dealt per bite */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Attack")
    float BiteDamage = 85.0f;

    /** Attack range — must be within this distance to bite */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Attack")
    float AttackRange = 350.0f;

    /** Cooldown between attacks in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Attack")
    float AttackCooldown = 2.5f;

    /** Blackboard key for the target actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Attack")
    FBlackboardKeySelector TargetActorKey;

private:
    float LastAttackTime = 0.0f;
};

// ============================================================
// BTDecorator: T-Rex Sense Player — detects player by sight/sound
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "NPC BT Decorator: TRex Sense Player"))
class TRANSPERSONALGAME_API UNPC_BTDecorator_TRexSensePlayer : public UBTDecorator
{
    GENERATED_BODY()

public:
    UNPC_BTDecorator_TRexSensePlayer();

    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
    virtual FString GetStaticDescription() const override;

    /** Detection radius for sight */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Sense")
    float SightRadius = 3000.0f;

    /** Detection radius for sound (player running/sprinting) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Sense")
    float SoundRadius = 1500.0f;

    /** Half-angle of the T-Rex forward vision cone in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Sense")
    float VisionConeHalfAngle = 70.0f;

    /** Blackboard key to write the detected target into */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Sense")
    FBlackboardKeySelector TargetActorKey;
};
