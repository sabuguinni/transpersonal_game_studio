#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "DinosaurBehaviorTask.generated.h"

// ============================================================
// Enums — NPC_ prefix to avoid collision with other agents
// ============================================================

UENUM(BlueprintType)
enum class ENPC_DinoAIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Eat         UMETA(DisplayName = "Eat"),
    Rest        UMETA(DisplayName = "Rest")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Generic         UMETA(DisplayName = "Generic")
};

// ============================================================
// Shared data struct for blackboard-driven behavior
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_DinoBlackboardData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    ENPC_DinoAIState CurrentState = ENPC_DinoAIState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    FVector PatrolOrigin = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    float FleeHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    bool bTargetInSight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    bool bIsHungry = false;
};

// ============================================================
// BTTask: Patrol — move to random point within patrol radius
// ============================================================

UCLASS(ClassGroup = "NPC|AI", meta = (DisplayName = "NPC Dino Patrol Task"))
class TRANSPERSONALGAME_API UNPC_BTTask_DinoPatrol : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_DinoPatrol();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    UPROPERTY(EditAnywhere, Category = "NPC|Patrol")
    FBlackboardKeySelector PatrolOriginKey;

    UPROPERTY(EditAnywhere, Category = "NPC|Patrol")
    FBlackboardKeySelector PatrolRadiusKey;

    UPROPERTY(EditAnywhere, Category = "NPC|Patrol")
    float MinPatrolDistance = 500.0f;
};

// ============================================================
// BTTask: Chase — move toward detected target
// ============================================================

UCLASS(ClassGroup = "NPC|AI", meta = (DisplayName = "NPC Dino Chase Task"))
class TRANSPERSONALGAME_API UNPC_BTTask_DinoChase : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_DinoChase();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    UPROPERTY(EditAnywhere, Category = "NPC|Chase")
    FBlackboardKeySelector TargetActorKey;

    UPROPERTY(EditAnywhere, Category = "NPC|Chase")
    float AcceptanceRadius = 280.0f;
};

// ============================================================
// BTTask: Attack — deal damage to target in melee range
// ============================================================

UCLASS(ClassGroup = "NPC|AI", meta = (DisplayName = "NPC Dino Attack Task"))
class TRANSPERSONALGAME_API UNPC_BTTask_DinoAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_DinoAttack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    UPROPERTY(EditAnywhere, Category = "NPC|Attack")
    FBlackboardKeySelector TargetActorKey;

    UPROPERTY(EditAnywhere, Category = "NPC|Attack")
    float BaseDamage = 50.0f;

    UPROPERTY(EditAnywhere, Category = "NPC|Attack")
    float AttackCooldown = 2.0f;

private:
    float LastAttackTime = -999.0f;
};

// ============================================================
// BTTask: Roar — intimidation behavior, stagger nearby prey
// ============================================================

UCLASS(ClassGroup = "NPC|AI", meta = (DisplayName = "NPC Dino Roar Task"))
class TRANSPERSONALGAME_API UNPC_BTTask_DinoRoar : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_DinoRoar();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    UPROPERTY(EditAnywhere, Category = "NPC|Roar")
    float RoarRadius = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "NPC|Roar")
    float FearInflicted = 30.0f;
};
