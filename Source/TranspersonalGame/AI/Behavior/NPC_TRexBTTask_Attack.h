#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NPC_TRexBTTask_Attack.generated.h"

/**
 * Combat AI — Attack Task
 * Agent #12: Combat & Enemy AI
 *
 * BT Task: T-Rex executes an attack on the player.
 * - Applies damage via UGameplayStatics::ApplyDamage()
 * - Triggers attack animation via DinosaurAnimInstance
 * - Enforces attack cooldown to prevent spam
 * - Knocks back the player on hit
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_TRexBTTask_Attack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_TRexBTTask_Attack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    /** Blackboard key for the target actor (player) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Blackboard")
    FBlackboardKeySelector TargetActorKey;

    /** Damage dealt per attack hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack", meta = (ClampMin = "1.0", ClampMax = "500.0"))
    float AttackDamage;

    /** Range within which the attack connects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack", meta = (ClampMin = "50.0", ClampMax = "800.0"))
    float AttackReach;

    /** Knockback impulse applied to the player on hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float KnockbackImpulse;

    /** Minimum time between consecutive attacks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack", meta = (ClampMin = "0.5", ClampMax = "10.0"))
    float AttackCooldown;

    /** Damage type class applied to the player */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    TSubclassOf<UDamageType> DamageTypeClass;

private:
    /** World time of the last successful attack */
    float LastAttackTime;

    /** Check if the target is within AttackReach */
    bool IsTargetInRange(APawn* AttackerPawn, AActor* TargetActor) const;

    /** Apply knockback impulse to the target */
    void ApplyKnockback(APawn* AttackerPawn, AActor* TargetActor) const;
};
