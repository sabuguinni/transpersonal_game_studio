#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BTDecorator.h"
#include "Combat_CombatAIController.h"
#include "Combat_CombatBehaviorTree.generated.h"

// Combat Task: Attack Target
UCLASS()
class TRANSPERSONALGAME_API UBTTask_Combat_AttackTarget : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_Combat_AttackTarget();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

protected:
    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackDuration = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    bool bRequireLineOfSight = true;
};

// Combat Task: Find Cover
UCLASS()
class TRANSPERSONALGAME_API UBTTask_Combat_FindCover : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_Combat_FindCover();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

protected:
    UPROPERTY(EditAnywhere, Category = "Combat")
    float CoverSearchRadius = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float MinCoverDistance = 200.0f;
};

// Combat Task: Circle Target
UCLASS()
class TRANSPERSONALGAME_API UBTTask_Combat_CircleTarget : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_Combat_CircleTarget();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

protected:
    UPROPERTY(EditAnywhere, Category = "Combat")
    float CircleRadius = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float CircleSpeed = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    bool bClockwise = true;
};

// Combat Service: Update Combat State
UCLASS()
class TRANSPERSONALGAME_API UBTService_Combat_UpdateCombatState : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_Combat_UpdateCombatState();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float UpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    bool bUpdateTargetLocation = true;
};

// Combat Service: Monitor Health
UCLASS()
class TRANSPERSONALGAME_API UBTService_Combat_MonitorHealth : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_Combat_MonitorHealth();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float LowHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float CriticalHealthThreshold = 0.15f;
};

// Combat Decorator: Can Attack
UCLASS()
class TRANSPERSONALGAME_API UBTDecorator_Combat_CanAttack : public UBTDecorator
{
    GENERATED_BODY()

public:
    UBTDecorator_Combat_CanAttack();

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
    virtual FString GetStaticDescription() const override;

    UPROPERTY(EditAnywhere, Category = "Combat")
    bool bCheckCooldown = true;

    UPROPERTY(EditAnywhere, Category = "Combat")
    bool bCheckRange = true;

    UPROPERTY(EditAnywhere, Category = "Combat")
    bool bCheckLineOfSight = false;
};

// Combat Decorator: Is In Combat Style
UCLASS()
class TRANSPERSONALGAME_API UBTDecorator_Combat_IsInCombatStyle : public UBTDecorator
{
    GENERATED_BODY()

public:
    UBTDecorator_Combat_IsInCombatStyle();

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
    virtual FString GetStaticDescription() const override;

    UPROPERTY(EditAnywhere, Category = "Combat")
    ECombat_CombatStyle RequiredCombatStyle = ECombat_CombatStyle::AmbushPredator;
};

// Combat Decorator: Target In Range
UCLASS()
class TRANSPERSONALGAME_API UBTDecorator_Combat_TargetInRange : public UBTDecorator
{
    GENERATED_BODY()

public:
    UBTDecorator_Combat_TargetInRange();

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
    virtual FString GetStaticDescription() const override;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float Range = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    bool bUseAttackRange = true;

    UPROPERTY(EditAnywhere, Category = "Combat")
    bool bUseDetectionRange = false;
};