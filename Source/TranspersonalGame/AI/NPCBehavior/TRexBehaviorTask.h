// TRexBehaviorTask.h
// NPC Behavior Agent #11 — T-Rex Behavior Tree Task Nodes
// Patrol → Detect → Chase → Attack state machine via UE5 BehaviorTree

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "TRexBehaviorTask.generated.h"

// ─── Enums (global scope — UE5 rule) ───────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Roar        UMETA(DisplayName = "Roar"),
    Rest        UMETA(DisplayName = "Rest")
};

UENUM(BlueprintType)
enum class ENPC_TRexAlertLevel : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Curious     UMETA(DisplayName = "Curious"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Aggressive  UMETA(DisplayName = "Aggressive")
};

// ─── Patrol Task ────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_TRexPatrolTask : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_TRexPatrolTask();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    /** Radius around home location to patrol (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float PatrolRadius = 500000.0f; // 5000 units

    /** How long to wait at each patrol point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float WaitTimeAtPoint = 3.0f;

    /** Blackboard key for patrol destination */
    UPROPERTY(EditAnywhere, Category = "TRex|Patrol")
    FBlackboardKeySelector PatrolDestinationKey;

private:
    FVector GetRandomPatrolPoint(const FVector& HomeLocation, float Radius, UNavigationSystemV1* NavSys, AActor* Owner) const;
};

// ─── Chase Task ─────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_TRexChaseTask : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_TRexChaseTask();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    /** Movement speed multiplier during chase */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Chase")
    float ChaseSpeedMultiplier = 1.4f;

    /** Blackboard key for the target actor */
    UPROPERTY(EditAnywhere, Category = "TRex|Chase")
    FBlackboardKeySelector TargetActorKey;
};

// ─── Attack Task ─────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_TRexAttackTask : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_TRexAttackTask();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    /** Damage dealt per bite */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float BiteDamage = 85.0f;

    /** Attack range in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float AttackRange = 30000.0f; // 300 units

    /** Cooldown between attacks in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float AttackCooldown = 2.5f;

    /** Blackboard key for the target actor */
    UPROPERTY(EditAnywhere, Category = "TRex|Attack")
    FBlackboardKeySelector TargetActorKey;

private:
    float LastAttackTime = 0.0f;
};

// ─── Roar Task ───────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_TRexRoarTask : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_TRexRoarTask();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    /** Radius in which other NPCs hear the roar and scatter */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float RoarRadius = 800000.0f; // 8000 units

    /** Fear amount applied to player on roar */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float FearImpact = 25.0f;
};

// ─── Detect Decorator ────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_TRexDetectDecorator : public UBTDecorator
{
    GENERATED_BODY()

public:
    UNPC_TRexDetectDecorator();

    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

    /** Detection range in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float DetectionRange = 300000.0f; // 3000 units

    /** T-Rex has poor lateral vision — cone angle in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float VisionConeAngle = 60.0f;

    /** T-Rex detects movement — stationary players harder to detect */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    bool bMovementBasedDetection = true;

    /** Blackboard key to write detected target into */
    UPROPERTY(EditAnywhere, Category = "TRex|Detection")
    FBlackboardKeySelector TargetActorKey;
};
