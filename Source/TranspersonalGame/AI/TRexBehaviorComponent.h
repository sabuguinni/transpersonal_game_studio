// TRexBehaviorComponent.h
// Agent #11 — NPC Behavior Agent | PROD_CYCLE_AUTO_20260629_007
// T-Rex behavioral state machine — patrol, alert, chase, attack, return, dead

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "TRexBehaviorComponent.generated.h"

// === GLOBAL SCOPE ENUMS (RULE 1 — must be outside UCLASS) ===

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle     UMETA(DisplayName = "Idle"),
    Patrol   UMETA(DisplayName = "Patrol"),
    Alert    UMETA(DisplayName = "Alert"),
    Chase    UMETA(DisplayName = "Chase"),
    Attack   UMETA(DisplayName = "Attack"),
    Return   UMETA(DisplayName = "Return"),
    Dead     UMETA(DisplayName = "Dead")
};

// === DELEGATES ===

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
    FNPC_TRexAnimStateDelegate,
    ENPC_TRexState, NewState,
    float, Speed,
    float, TurnRate,
    float, BlendWeight,
    bool, bIsAttacking
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FNPC_TRexAttackMontageDelegate,
    int32, AttackIndex
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FNPC_TRexRoarDelegate,
    FVector, RoarOrigin,
    float, RoarRadius
);

// === COMPONENT CLASS ===

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "T-Rex Behavior Component")
class TRANSPERSONALGAME_API UNPC_TRexBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TRexBehaviorComponent();

    // === BEHAVIORAL PARAMETERS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float ChaseDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Audio")
    float RoarRadius;

    // === STATE ===

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    ENPC_TRexState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    FVector PatrolOrigin;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    FVector CurrentPatrolTarget;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    APawn* TargetPawn;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    bool bCanAttack;

    // === DELEGATES ===

    UPROPERTY(BlueprintAssignable, Category = "TRex|Events")
    FNPC_TRexAnimStateDelegate OnTRexAnimStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "TRex|Events")
    FNPC_TRexAttackMontageDelegate OnTRexAttackMontage;

    UPROPERTY(BlueprintAssignable, Category = "TRex|Events")
    FNPC_TRexRoarDelegate OnTRexRoar;

    // === PUBLIC API ===

    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    void TransitionToState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "TRex|Events")
    void OnTRexDeath();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    FTimerHandle PatrolTimerHandle;
    FTimerHandle AlertTimerHandle;
    FTimerHandle AttackCooldownHandle;

    void ScanForPlayer();
    void UpdateBehaviorState(float DeltaTime);
    void ExecutePatrol(float DeltaTime);
    void ExecuteChase(float DeltaTime);
    void ExecuteAttack();
    void ExecuteReturn(float DeltaTime);
    void PickNewPatrolTarget();
    void BeginChase();
    void ResetAttackCooldown();
};
