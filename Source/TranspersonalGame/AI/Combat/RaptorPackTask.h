// RaptorPackTask.h
// Combat & Enemy AI Agent #12 — PROD_CYCLE_AUTO_20260626_002
// Raptor pack coordination: flanking AI, alpha-follower hierarchy, threat sharing
// Depends on: AIModule, BehaviorTreeModule, NavigationSystem
// All types prefixed with Combat_ to avoid collision with other agents

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "RaptorPackTask.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECombat_RaptorRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha"),        // Engages directly, triggers pack
    LeftFlank   UMETA(DisplayName = "Left Flank"),   // Circles left, attacks from side
    RightFlank  UMETA(DisplayName = "Right Flank"),  // Circles right, attacks from side
    Ambush      UMETA(DisplayName = "Ambush"),        // Holds position, attacks when target flees
};

UENUM(BlueprintType)
enum class ECombat_PackState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Stalking    UMETA(DisplayName = "Stalking"),     // Slow approach, no alert yet
    Flanking    UMETA(DisplayName = "Flanking"),     // Coordinated encirclement
    Attacking   UMETA(DisplayName = "Attacking"),    // Full assault
    Regrouping  UMETA(DisplayName = "Regrouping"),   // One member hurt, pack retreats briefly
    Scattered   UMETA(DisplayName = "Scattered"),    // Alpha killed, pack breaks
};

// ─── Shared Pack Data (lightweight struct, no UPROPERTY to avoid GC issues) ──

USTRUCT(BlueprintType)
struct FCombat_PackSharedData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    AActor* SharedTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    ECombat_PackState PackState = ECombat_PackState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bAlphaEngaged = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float LastPackAlertTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    int32 ActivePackSize = 3;
};

// ─── Task: Alpha Engage ───────────────────────────────────────────────────────
// Alpha raptor moves directly toward target and triggers flankers

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_RaptorAlphaEngageTask : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UCombat_RaptorAlphaEngageTask();

    // Blackboard key: target actor
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    FBlackboardKeySelector TargetActorKey;

    // Distance at which alpha commits to attack (cm)
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    float AttackRange = 200.0f;

    // Damage per bite
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    float BiteDamage = 45.0f;

    // Cooldown between bites (seconds)
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    float BiteCooldown = 1.8f;

    // Chase speed multiplier
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    float ChaseSpeedMultiplier = 1.6f;

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual uint16 GetInstanceMemorySize() const override;

private:
    float LastBiteTime = 0.0f;
};

// ─── Task: Flank Move ─────────────────────────────────────────────────────────
// Flanker raptor circles to a position offset from the target, then waits for alpha signal

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_RaptorFlankTask : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UCombat_RaptorFlankTask();

    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    FBlackboardKeySelector TargetActorKey;

    // Which side this flanker takes
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    ECombat_RaptorRole FlankRole = ECombat_RaptorRole::LeftFlank;

    // Flank offset distance from target (cm)
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    float FlankRadius = 600.0f;

    // Attack once alpha has engaged for this many seconds
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    float FlankAttackDelay = 2.0f;

    // Damage per flanking strike
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    float FlankDamage = 35.0f;

    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    float FlankSpeedMultiplier = 1.8f;

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual uint16 GetInstanceMemorySize() const override;

private:
    float AlphaEngageStartTime = 0.0f;
    bool bHasReachedFlankPos = false;
    FVector FlankDestination = FVector::ZeroVector;
};

// ─── Task: Regroup ────────────────────────────────────────────────────────────
// When alpha is killed or pack member takes heavy damage, pack retreats to regroup point

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_RaptorRegroupTask : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UCombat_RaptorRegroupTask();

    // Distance to retreat before regrouping
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    float RetreatDistance = 1200.0f;

    // Time to wait at regroup point before re-engaging
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    float RegroupWaitTime = 4.0f;

    // Retreat speed multiplier
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    float RetreatSpeedMultiplier = 2.0f;

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual uint16 GetInstanceMemorySize() const override;

private:
    float RetreatStartTime = 0.0f;
    FVector RetreatDestination = FVector::ZeroVector;
    bool bReachedRetreatPoint = false;
};

// ─── Service: Pack Threat Broadcast ──────────────────────────────────────────
// Runs on each raptor's BT; when one raptor detects a target, it broadcasts to all
// raptors within PackAlertRadius so the whole pack shares the same target

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_RaptorPackAlertService : public UBTService
{
    GENERATED_BODY()

public:
    UCombat_RaptorPackAlertService();

    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    FBlackboardKeySelector TargetActorKey;

    // Radius within which pack members share threat info (cm)
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    float PackAlertRadius = 3000.0f;

    // Tag used to identify pack members
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    FName PackTag = FName("RaptorPack");

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};

// ─── Decorator: Alpha Engaged Check ──────────────────────────────────────────
// Returns true if the alpha raptor in this pack has already engaged the target
// Used by flankers to time their attack

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_AlphaEngagedDecorator : public UBTDecorator
{
    GENERATED_BODY()

public:
    UCombat_AlphaEngagedDecorator();

    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    FBlackboardKeySelector AlphaEngagedKey;

    // Minimum time alpha must have been engaged before flankers attack
    UPROPERTY(EditAnywhere, Category = "Combat|Pack")
    float MinAlphaEngageTime = 1.5f;

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
