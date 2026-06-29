#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TRexBehaviorComponent.generated.h"

// ── Global-scope enum (RULE 1: USTRUCT/UENUM at global scope only) ──────────
UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle      UMETA(DisplayName = "Idle"),
    Patrol    UMETA(DisplayName = "Patrol"),
    Alert     UMETA(DisplayName = "Alert"),
    Chase     UMETA(DisplayName = "Chase"),
    Attack    UMETA(DisplayName = "Attack"),
    Return    UMETA(DisplayName = "Return"),
    Dead      UMETA(DisplayName = "Dead")
};

// ── Delegate declarations ────────────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
    FNPC_TRexAnimStateDelegate,
    ENPC_TRexState, NewState,
    ENPC_TRexState, OldState,
    float, Speed,
    bool, bIsAttacking,
    bool, bIsRoaring
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FNPC_TRexAttackMontageDelegate,
    int32, AttackIndex
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FNPC_TRexRoarDelegate,
    FVector, Origin,
    float,   Radius
);

// ── Component ────────────────────────────────────────────────────────────────
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TRexBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TRexBehaviorComponent();

    // ── Tuning ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float PatrolRadius = 5000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float DetectionRange = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float AttackRange = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float AttackDamage = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float RoarRadius = 6000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float PatrolSpeed = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChaseSpeed = 700.f;

    // ── State ────────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State",
              meta = (AllowPrivateAccess = "true"))
    ENPC_TRexState CurrentState = ENPC_TRexState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State",
              meta = (AllowPrivateAccess = "true"))
    AActor* TargetActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State",
              meta = (AllowPrivateAccess = "true"))
    FVector HomeLocation = FVector::ZeroVector;

    // ── Delegates ────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "TRex|Events")
    FNPC_TRexAnimStateDelegate OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "TRex|Events")
    FNPC_TRexAttackMontageDelegate OnAttackMontage;

    UPROPERTY(BlueprintAssignable, Category = "TRex|Events")
    FNPC_TRexRoarDelegate OnRoar;

    // ── Public API ───────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void SetState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void TriggerRoar();

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void TriggerAttack(int32 AttackIndex = 0);

    UFUNCTION(BlueprintPure, Category = "TRex|Behavior")
    bool IsPlayerInDetectionRange() const;

    UFUNCTION(BlueprintPure, Category = "TRex|Behavior")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "TRex|Behavior")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintPure, Category = "TRex|Behavior")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    float TimeSinceLastAttack = 0.f;
    FVector CurrentPatrolTarget = FVector::ZeroVector;
    bool   bPatrolTargetReached = true;

    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickReturn(float DeltaTime);

    void PickNewPatrolTarget();
    AActor* FindNearestPlayer() const;
};
