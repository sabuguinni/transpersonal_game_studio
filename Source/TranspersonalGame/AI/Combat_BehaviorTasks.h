#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BTDecorator.h"
#include "../SharedTypes.h"
#include "Combat_BehaviorTasks.generated.h"

// Task: Execute Attack
UCLASS()
class TRANSPERSONALGAME_API UBTTask_Combat_ExecuteAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_Combat_ExecuteAttack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;
};

// Task: Find Cover
UCLASS()
class TRANSPERSONALGAME_API UBTTask_Combat_FindCover : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_Combat_FindCover();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SearchRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FBlackboardKeySelector CoverLocationKey;
};

// Task: Coordinate Pack Attack
UCLASS()
class TRANSPERSONALGAME_API UBTTask_Combat_CoordinatePackAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_Combat_CoordinatePackAttack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CoordinationRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 MinPackSize = 2;
};

// Task: Territorial Patrol
UCLASS()
class TRANSPERSONALGAME_API UBTTask_Combat_TerritorialPatrol : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_Combat_TerritorialPatrol();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TerritoryRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FBlackboardKeySelector PatrolLocationKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PatrolSpeed = 200.0f;
};

// Service: Update Combat State
UCLASS()
class TRANSPERSONALGAME_API UBTService_Combat_UpdateCombatState : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_Combat_UpdateCombatState();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FBlackboardKeySelector TargetActorKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FBlackboardKeySelector DistanceToTargetKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FBlackboardKeySelector AIStateKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FBlackboardKeySelector HealthPercentageKey;
};

// Service: Pack Coordination
UCLASS()
class TRANSPERSONALGAME_API UBTService_Combat_PackCoordination : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_Combat_PackCoordination();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FBlackboardKeySelector PackLeaderKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FBlackboardKeySelector PackSizeKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FBlackboardKeySelector PackFormationKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PackCoordinationRadius = 2500.0f;
};

// Decorator: In Combat Range
UCLASS()
class TRANSPERSONALGAME_API UBTDecorator_Combat_InCombatRange : public UBTDecorator
{
    GENERATED_BODY()

public:
    UBTDecorator_Combat_InCombatRange();

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FBlackboardKeySelector TargetActorKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bInvertCondition = false;
};

// Decorator: Has Pack Support
UCLASS()
class TRANSPERSONALGAME_API UBTDecorator_Combat_HasPackSupport : public UBTDecorator
{
    GENERATED_BODY()

public:
    UBTDecorator_Combat_HasPackSupport();

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FBlackboardKeySelector PackSizeKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 MinPackSize = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PackRadius = 2000.0f;
};

// Decorator: Health Threshold
UCLASS()
class TRANSPERSONALGAME_API UBTDecorator_Combat_HealthThreshold : public UBTDecorator
{
    GENERATED_BODY()

public:
    UBTDecorator_Combat_HealthThreshold();

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FBlackboardKeySelector HealthPercentageKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float HealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCheckBelow = true; // true = below threshold, false = above threshold
};