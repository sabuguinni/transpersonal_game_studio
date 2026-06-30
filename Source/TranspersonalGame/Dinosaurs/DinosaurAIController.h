// DinosaurAIController.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// AI Controller for all dinosaur species. Drives the EDinosaurBehavior state machine
// defined in DinosaurBase via Behavior Trees + EQS queries.
// Depends on: DinosaurBase.h, AIController (Engine)

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "DinosaurBase.h"
#include "DinosaurAIController.generated.h"

// ─── Blackboard Key Names ────────────────────────────────────────────────────
// These string literals must match the BB_DinosaurBase Blackboard asset keys.

/** Blackboard key: target actor (player or prey) */
#define BB_KEY_TARGET_ACTOR      TEXT("TargetActor")
/** Blackboard key: patrol destination vector */
#define BB_KEY_PATROL_DEST       TEXT("PatrolDestination")
/** Blackboard key: home/territory center vector */
#define BB_KEY_HOME_LOCATION     TEXT("HomeLocation")
/** Blackboard key: current behavior state (int32 cast of EDinosaurBehavior) */
#define BB_KEY_BEHAVIOR_STATE    TEXT("BehaviorState")
/** Blackboard key: is target visible (bool) */
#define BB_KEY_TARGET_VISIBLE    TEXT("bTargetVisible")
/** Blackboard key: stamina value (float) */
#define BB_KEY_STAMINA           TEXT("Stamina")

// ─── Forward Declarations ────────────────────────────────────────────────────
class ADinosaurBase;
class UBehaviorTreeComponent;

/**
 * ADinosaurAIController
 *
 * Drives all dinosaur species via a shared Behavior Tree (BT_DinosaurBase).
 * Perception events from UAIPerceptionComponent update the Blackboard, which
 * in turn drives BT task selection (patrol, chase, attack, flee, return).
 *
 * Species-specific overrides are handled by subclassing this controller
 * (e.g., ATRexAIController, ARaptorAIController) and overriding
 * SelectPatrolDestination() or AdjustCombatBehavior().
 */
UCLASS(ClassGroup = "DinosaurAI", meta = (DisplayName = "Dinosaur AI Controller"))
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // ─── AAIController Overrides ─────────────────────────────────────────────

    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Behavior Tree ───────────────────────────────────────────────────────

    /** The shared Behavior Tree asset for all dinosaurs. Set per-species in BP. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|BehaviorTree")
    UBehaviorTree* DinosaurBehaviorTree;

    /** Starts the Behavior Tree on the possessed pawn. */
    UFUNCTION(BlueprintCallable, Category = "AI|BehaviorTree")
    void StartBehaviorTree();

    /** Stops the Behavior Tree and clears the Blackboard. */
    UFUNCTION(BlueprintCallable, Category = "AI|BehaviorTree")
    void StopBehaviorTree();

    // ─── Blackboard Interface ────────────────────────────────────────────────

    /** Sets the current behavior state on the Blackboard. */
    UFUNCTION(BlueprintCallable, Category = "AI|Blackboard")
    void SetBehaviorState(EDinosaurBehavior NewState);

    /** Gets the current behavior state from the Blackboard. */
    UFUNCTION(BlueprintPure, Category = "AI|Blackboard")
    EDinosaurBehavior GetBehaviorState() const;

    /** Sets the target actor (player/prey) on the Blackboard. */
    UFUNCTION(BlueprintCallable, Category = "AI|Blackboard")
    void SetTargetActor(AActor* Target);

    /** Gets the current target actor from the Blackboard. */
    UFUNCTION(BlueprintPure, Category = "AI|Blackboard")
    AActor* GetTargetActor() const;

    /** Sets the patrol destination on the Blackboard. */
    UFUNCTION(BlueprintCallable, Category = "AI|Blackboard")
    void SetPatrolDestination(FVector Destination);

    // ─── Perception ──────────────────────────────────────────────────────────

    /** Called when the AI perceives actors (sight/hearing). */
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    /** Returns true if the given actor is currently seen. */
    UFUNCTION(BlueprintPure, Category = "AI|Perception")
    bool CanSeeActor(AActor* Actor) const;

    // ─── Navigation ──────────────────────────────────────────────────────────

    /**
     * Selects a random patrol destination within PatrolRadius of the territory center.
     * Subclasses can override to implement species-specific patrol patterns.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "AI|Navigation")
    FVector SelectPatrolDestination();
    virtual FVector SelectPatrolDestination_Implementation();

    /** Moves the pawn toward a target location using the NavMesh. */
    UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
    void MoveToLocation_Safe(FVector TargetLocation, float AcceptanceRadius = 100.0f);

    /** Moves the pawn toward a target actor using the NavMesh. */
    UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
    void MoveToActor_Safe(AActor* TargetActor, float AcceptanceRadius = 200.0f);

    // ─── Combat ──────────────────────────────────────────────────────────────

    /**
     * Called when the pawn should engage in combat.
     * Subclasses override to implement species-specific attack patterns.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "AI|Combat")
    void AdjustCombatBehavior();
    virtual void AdjustCombatBehavior_Implementation();

    /** Returns true if the target is within melee attack range. */
    UFUNCTION(BlueprintPure, Category = "AI|Combat")
    bool IsTargetInAttackRange() const;

    // ─── State Machine ───────────────────────────────────────────────────────

    /** Transitions the dinosaur to a new behavior state. Updates BB + DinosaurBase. */
    UFUNCTION(BlueprintCallable, Category = "AI|StateMachine")
    void TransitionToState(EDinosaurBehavior NewState);

    /** Evaluates current conditions and selects the appropriate state. */
    UFUNCTION(BlueprintCallable, Category = "AI|StateMachine")
    void EvaluateState();

protected:
    // ─── Internal References ─────────────────────────────────────────────────

    /** Cached reference to the possessed DinosaurBase pawn. */
    UPROPERTY(BlueprintReadOnly, Category = "AI|Internal",
              meta = (AllowPrivateAccess = "true"))
    ADinosaurBase* ControlledDinosaur;

    /** BehaviorTree runtime component. */
    UPROPERTY(BlueprintReadOnly, Category = "AI|Internal",
              meta = (AllowPrivateAccess = "true"))
    UBehaviorTreeComponent* BehaviorTreeComp;

    // ─── Configuration ───────────────────────────────────────────────────────

    /** Radius around territory center for patrol waypoints. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Config")
    float PatrolRadius;

    /** Distance from territory center that triggers a Return-to-Home state. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Config")
    float MaxRoamDistance;

    /** Minimum stamina (0-100) required to initiate a chase. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Config")
    float MinStaminaToChase;

    /** Stamina threshold below which the dinosaur flees combat. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Config")
    float FleeStaminaThreshold;

    /** How often (seconds) EvaluateState() is called from Tick. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Config")
    float StateEvaluationInterval;

private:
    /** Accumulator for StateEvaluationInterval. */
    float StateEvalTimer;

    /** Last known location of the target (used when target is lost). */
    FVector LastKnownTargetLocation;

    /** Whether the controller has a valid BT running. */
    bool bBehaviorTreeRunning;
};
