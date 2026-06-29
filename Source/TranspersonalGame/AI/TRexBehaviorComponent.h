// TRexBehaviorComponent.h
// Agent #11 — NPC Behavior Agent
// CYCLE: PROD_CYCLE_AUTO_20260629_006
//
// T-Rex behavioral brain component.
// Attach to any T-Rex pawn to give it:
//   - 5000-unit patrol territory
//   - 3000-unit player detection radius
//   - 300-unit attack range
//   - Persistent threat memory (30s decay)
//   - Delegates for animation coupling with Agent #10

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NavigationSystem.h"
#include "TRexBehaviorComponent.generated.h"

// ============================================================
// State enum — global scope (UHT requirement)
// ============================================================
UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle    UMETA(DisplayName = "Idle"),
    Patrol  UMETA(DisplayName = "Patrol"),
    Alert   UMETA(DisplayName = "Alert"),
    Chase   UMETA(DisplayName = "Chase"),
    Attack  UMETA(DisplayName = "Attack"),
    Return  UMETA(DisplayName = "Return to Territory"),
    Dead    UMETA(DisplayName = "Dead")
};

// ============================================================
// Delegates — for animation coupling (Agent #10 API)
// ============================================================
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FNPC_TRexAnimStateDelegate,
    bool, bAttacking,
    bool, bEating,
    bool, bRoaring,
    bool, bAlert,
    bool, bSleeping);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNPC_TRexAttackMontageDelegate,
    int32, AttackTypeIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNPC_TRexRoarDelegate,
    FVector, RoarOrigin,
    float, RoarRadius);

// ============================================================
// Component class
// ============================================================
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent), DisplayName = "T-Rex Behavior Component")
class TRANSPERSONALGAME_API UNPC_TRexBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TRexBehaviorComponent();

    // --------------------------------------------------------
    // Configuration
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Territory")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float ChaseRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Memory")
    float ThreatMemoryDecayTime;

    // --------------------------------------------------------
    // Runtime state (read-only in Blueprint)
    // --------------------------------------------------------
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    ENPC_TRexState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Memory")
    bool bPlayerInMemory;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Memory")
    FVector LastKnownPlayerLocation;

    // --------------------------------------------------------
    // Delegates — bind in Blueprint or DinosaurAnimInstance
    // --------------------------------------------------------
    UPROPERTY(BlueprintAssignable, Category = "TRex|Animation")
    FNPC_TRexAnimStateDelegate OnAnimStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "TRex|Animation")
    FNPC_TRexAttackMontageDelegate OnAttackMontageRequested;

    UPROPERTY(BlueprintAssignable, Category = "TRex|Social")
    FNPC_TRexRoarDelegate OnRoarBroadcast;

    // --------------------------------------------------------
    // External API — called by other agents
    // --------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "TRex|AI")
    void NotifyPlayerDetectedByOtherDino(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "TRex|AI")
    void ForceReturnToTerritory();

    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    bool IsHostile() const { return CurrentState == ENPC_TRexState::Chase || CurrentState == ENPC_TRexState::Attack; }

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Cached references
    UPROPERTY()
    APawn* OwnerPawn;

    UPROPERTY()
    class AAIController* OwnerAIController;

    UPROPERTY()
    class UCharacterMovementComponent* OwnerMovement;

    // Patrol state
    FVector PatrolOrigin;
    FVector CurrentPatrolTarget;
    bool bHasPatrolOrigin;
    bool bHasPatrolTarget;

    // Timing
    float LastAttackTime;
    float LastThreatTime;
    float IdleTimer;
    bool bJustEnteredChase;

    // Internal methods
    void SensePlayer();
    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickReturn(float DeltaTime);
    void TransitionToState(ENPC_TRexState NewState);
    void UpdateAnimationState();
    void PickNewPatrolTarget();
    void ExecuteBiteAttack(class ACharacter* Target);
    void BroadcastRoar();
};
