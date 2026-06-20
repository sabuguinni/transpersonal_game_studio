// DinosaurAIController.h
// Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260620_006
// Minimal AI controller for dinosaur pawns: Idle → Patrol → Attack state machine.
// Uses UE5 AIModule (no BehaviorTree asset required — pure C++ state machine).

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "DinosaurAIController.generated.h"

// Forward declarations — avoid cross-module header pulls
class ADinosaurBase;
class APawn;

/** Dinosaur AI states — simple FSM driving patrol and combat. */
UENUM(BlueprintType)
enum class ECore_DinoAIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Dead        UMETA(DisplayName = "Dead")
};

/**
 * ADinosaurAIController
 *
 * Minimal AI controller for all dinosaur species.
 * Implements a tick-driven FSM: Idle → Patrol → Chase → Attack.
 * No BehaviorTree asset dependency — fully self-contained C++.
 *
 * Usage: Set as AIControllerClass in ADinosaurBase (or child BP).
 */
UCLASS(ClassGroup = "DinosaurAI", meta = (DisplayName = "Dinosaur AI Controller"))
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

    // AAIController overrides
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void Tick(float DeltaTime) override;

    // ── State Machine ─────────────────────────────────────────────────────────

    /** Current AI state. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
    ECore_DinoAIState CurrentState;

    /** Force a state transition (callable from Blueprint or child classes). */
    UFUNCTION(BlueprintCallable, Category = "AI|State")
    void SetAIState(ECore_DinoAIState NewState);

    // ── Detection ─────────────────────────────────────────────────────────────

    /** Radius (cm) within which the dinosaur detects the player. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Detection")
    float DetectionRadius;

    /** Radius (cm) within which the dinosaur initiates a melee attack. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Detection")
    float AttackRadius;

    /** Half-angle (degrees) of the forward vision cone. 45 = 90° total FOV. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Detection")
    float VisionHalfAngleDeg;

    // ── Patrol ────────────────────────────────────────────────────────────────

    /** Radius (cm) around spawn point used to pick random patrol destinations. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    float PatrolRadius;

    /** Time (s) the dinosaur waits at a patrol point before moving to the next. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    float PatrolWaitTime;

    // ── Attack ────────────────────────────────────────────────────────────────

    /** Cooldown (s) between consecutive attacks. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
    float AttackCooldown;

    /** Damage dealt per attack hit. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
    float AttackDamage;

protected:
    // ── Internal helpers ──────────────────────────────────────────────────────

    /** Scan for the player pawn within DetectionRadius + VisionHalfAngleDeg. */
    APawn* FindPlayerInRange() const;

    /** Pick a random reachable point within PatrolRadius of SpawnLocation. */
    FVector PickPatrolDestination() const;

    /** Issue a move request to Destination. */
    void MoveToDestination(const FVector& Destination);

    /** Perform a melee attack on Target. */
    void PerformAttack(APawn* Target);

    // ── State handlers ────────────────────────────────────────────────────────
    void HandleIdle(float DeltaTime);
    void HandlePatrol(float DeltaTime);
    void HandleChase(float DeltaTime);
    void HandleAttack(float DeltaTime);
    void HandleFlee(float DeltaTime);

private:
    /** World location where this controller possessed its pawn. */
    FVector SpawnLocation;

    /** Cached reference to the controlled dinosaur pawn. */
    UPROPERTY()
    ADinosaurBase* ControlledDino;

    /** Current patrol destination. */
    FVector PatrolTarget;

    /** Accumulated idle/wait timer. */
    float IdleTimer;

    /** Accumulated attack cooldown timer. */
    float AttackTimer;

    /** Last known player location (used during chase). */
    FVector LastKnownPlayerLocation;

    /** Weak ref to current chase target. */
    UPROPERTY()
    APawn* ChaseTarget;
};
