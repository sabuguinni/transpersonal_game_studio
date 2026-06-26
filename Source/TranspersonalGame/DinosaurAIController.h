// DinosaurAIController.h — AI Controller for dinosaur pawns
// Handles navigation, perception, and behavior state transitions
// Agent #3 — Core Systems Programmer

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "DinosaurBase.h"
#include "DinosaurAIController.generated.h"

// Forward declarations
class UBehaviorTree;
class UBlackboardComponent;
class UAIPerceptionComponent;

/**
 * ADinosaurAIController
 *
 * AI Controller for all dinosaur species. Manages:
 * - Perception (sight + hearing) via AIPerceptionComponent
 * - Biome-aware roaming using random patrol points
 * - Threat response: idle → alert → hunt → flee state transitions
 * - Navigation via UE5 NavMesh (recast)
 *
 * Designed to work with ADinosaurBase without requiring a full Behavior Tree asset —
 * falls back to code-driven tick logic if no BT asset is assigned.
 */
UCLASS(ClassGroup = "DinosaurAI", meta = (DisplayName = "Dinosaur AI Controller"))
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

    // ─── Overrides ────────────────────────────────────────────────────────────

    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Perception ───────────────────────────────────────────────────────────

    /** Called by AIPerception when a stimulus is detected or lost */
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ─── Navigation ───────────────────────────────────────────────────────────

    /** Pick a random patrol point within PatrolRadius of the home location */
    UFUNCTION(BlueprintCallable, Category = "DinosaurAI|Navigation")
    FVector GetRandomPatrolPoint() const;

    /** Move to the next patrol point */
    UFUNCTION(BlueprintCallable, Category = "DinosaurAI|Navigation")
    void MoveToNextPatrolPoint();

    /** Chase the current threat target */
    UFUNCTION(BlueprintCallable, Category = "DinosaurAI|Navigation")
    void ChaseTarget(AActor* Target);

    /** Flee away from the threat target */
    UFUNCTION(BlueprintCallable, Category = "DinosaurAI|Navigation")
    void FleeFromTarget(AActor* Target);

    // ─── State ────────────────────────────────────────────────────────────────

    /** Force a behavior state change (called by DinosaurBase or external systems) */
    UFUNCTION(BlueprintCallable, Category = "DinosaurAI|State")
    void SetBehaviorState(ECore_DinosaurBehaviorState NewState);

    /** Current behavior state mirrored from the pawn */
    UFUNCTION(BlueprintPure, Category = "DinosaurAI|State")
    ECore_DinosaurBehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    // ─── Properties ───────────────────────────────────────────────────────────

    /** Radius within which this dinosaur patrols around its home location */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinosaurAI|Patrol",
        meta = (ClampMin = "200.0", ClampMax = "10000.0"))
    float PatrolRadius = 2000.0f;

    /** How long (seconds) the dinosaur waits at a patrol point before moving */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinosaurAI|Patrol",
        meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float PatrolWaitTime = 5.0f;

    /** Sight radius for AIPerceptionComponent */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinosaurAI|Perception",
        meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float SightRadius = 1500.0f;

    /** Peripheral vision half-angle (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinosaurAI|Perception",
        meta = (ClampMin = "10.0", ClampMax = "180.0"))
    float SightAngle = 90.0f;

    /** Hearing range radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinosaurAI|Perception",
        meta = (ClampMin = "100.0", ClampMax = "3000.0"))
    float HearingRange = 800.0f;

    /** Optional Behavior Tree asset — if null, uses code-driven tick logic */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinosaurAI|BehaviorTree")
    TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

protected:
    // ─── Internal state ───────────────────────────────────────────────────────

    /** Cached reference to the controlled dinosaur pawn */
    UPROPERTY(Transient)
    TObjectPtr<ADinosaurBase> ControlledDino;

    /** Current behavior state */
    UPROPERTY(Transient, BlueprintReadOnly, Category = "DinosaurAI|State",
        meta = (AllowPrivateAccess = "true"))
    ECore_DinosaurBehaviorState CurrentBehaviorState;

    /** Home location — set when the pawn is possessed */
    UPROPERTY(Transient)
    FVector HomeLocation;

    /** Current threat target (player or another actor) */
    UPROPERTY(Transient)
    TObjectPtr<AActor> ThreatTarget;

    /** Timer accumulator for patrol wait */
    float PatrolWaitTimer = 0.0f;

    /** Whether we are currently waiting at a patrol point */
    bool bWaitingAtPatrolPoint = false;

    /** Perception component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DinosaurAI|Perception",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAIPerceptionComponent> PerceptionComp;

    /** Sight sense config */
    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    /** Hearing sense config */
    UPROPERTY()
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

private:
    /** Tick sub-routines per state */
    void TickIdle(float DeltaTime);
    void TickPatrolling(float DeltaTime);
    void TickAlerted(float DeltaTime);
    void TickHunting(float DeltaTime);
    void TickFleeing(float DeltaTime);
    void TickResting(float DeltaTime);
};
