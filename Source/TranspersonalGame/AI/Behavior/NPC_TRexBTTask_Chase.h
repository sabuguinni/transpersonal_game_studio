#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "NPC_TRexBTTask_Chase.generated.h"

/**
 * Combat AI — Chase Task
 * Agent #12: Combat & Enemy AI
 *
 * BT Task: T-Rex chases the player when within detection range.
 * - Reads TargetActorKey from Blackboard (set by perception/EQS)
 * - Uses MoveToActor at full sprint speed
 * - Transitions to Attack when within AttackRange
 * - Abandons chase if player escapes beyond AbandonRange
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_TRexBTTask_Chase : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_TRexBTTask_Chase();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    /** Blackboard key for the target actor (player) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Blackboard")
    FBlackboardKeySelector TargetActorKey;

    /** Distance at which chase transitions to attack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Chase", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
    float AttackRange;

    /** Distance at which T-Rex gives up the chase */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Chase", meta = (ClampMin = "1000.0", ClampMax = "10000.0"))
    float AbandonRange;

    /** Movement speed multiplier during chase (1.0 = normal, 1.5 = sprint) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Chase", meta = (ClampMin = "0.5", ClampMax = "3.0"))
    float ChaseSpeedMultiplier;

    /** Acceptance radius for MoveToActor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Chase", meta = (ClampMin = "50.0", ClampMax = "500.0"))
    float AcceptanceRadius;

private:
    /** Cached original max walk speed — restored when chase ends */
    float OriginalMaxWalkSpeed;

    /** Whether we have an active move request */
    bool bMoveRequestActive;

    /** Apply sprint speed to the pawn's movement component */
    void ApplyChaseSpeed(APawn* ControlledPawn);

    /** Restore original movement speed */
    void RestoreOriginalSpeed(APawn* ControlledPawn);
};
