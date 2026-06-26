// DinosaurAIController.h — Core Systems Programmer #03 — Cycle 009
// Prehistoric survival game: AI controller for all dinosaur pawns.
// Drives navigation, perception, and biome-aware roaming via UE5 AIModule.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DinosaurBase.h"
#include "DinosaurAIController.generated.h"

// Forward declarations
class UNavigationSystemV1;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;

/**
 * ADinosaurAIController — AI Controller for all dinosaur pawns.
 *
 * Responsibilities:
 * - Biome-aware roaming: picks patrol points within suitable biome zones
 * - Threat detection: uses UAIPerceptionComponent (sight + hearing)
 * - State-driven navigation: moves pawn based on ADinosaurBase::CurrentState
 * - Attack execution: moves into melee range and triggers ApplyMeleeDamage()
 *
 * Usage: Set as AIControllerClass on any ADinosaurBase Blueprint subclass.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

    // ── Lifecycle ──────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ── Navigation ─────────────────────────────────────────────────────────────

    /** Pick a random reachable point within RoamRadius and move toward it. */
    UFUNCTION(BlueprintCallable, Category = "DinoAI|Navigation")
    void StartRoaming();

    /** Move directly toward the current threat target. */
    UFUNCTION(BlueprintCallable, Category = "DinoAI|Navigation")
    void MoveToTarget(AActor* Target);

    /** Move away from the threat (flee behavior). */
    UFUNCTION(BlueprintCallable, Category = "DinoAI|Navigation")
    void FleeFromTarget(AActor* Threat);

    /** Stop all movement immediately. */
    UFUNCTION(BlueprintCallable, Category = "DinoAI|Navigation")
    void StopMovement();

    // ── Perception ─────────────────────────────────────────────────────────────

    /** Called when AI perception detects/loses an actor. */
    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ── State Queries ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category = "DinoAI|State")
    bool HasReachedDestination() const;

    UFUNCTION(BlueprintPure, Category = "DinoAI|State")
    AActor* GetCurrentTarget() const { return CurrentThreat; }

    UFUNCTION(BlueprintPure, Category = "DinoAI|State")
    bool IsInAttackRange() const;

    // ── Config ─────────────────────────────────────────────────────────────────

    /** Radius within which the dino will pick roam destinations. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinoAI|Config")
    float RoamRadius = 2000.f;

    /** How long (seconds) to wait at a roam destination before picking a new one. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinoAI|Config")
    float RoamWaitTime = 3.0f;

    /** Minimum distance to consider "arrived" at a roam point. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinoAI|Config")
    float AcceptanceRadius = 150.f;

    /** How often (seconds) the AI re-evaluates its state. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DinoAI|Config")
    float AITickInterval = 0.5f;

protected:
    // ── Components ─────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DinoAI|Components",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DinoAI|Components",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DinoAI|Components",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig;

    // ── Internal State ─────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DinoAI|State",
        meta = (AllowPrivateAccess = "true"))
    AActor* CurrentThreat = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DinoAI|State",
        meta = (AllowPrivateAccess = "true"))
    FVector CurrentRoamDestination = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DinoAI|State",
        meta = (AllowPrivateAccess = "true"))
    float RoamWaitTimer = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DinoAI|State",
        meta = (AllowPrivateAccess = "true"))
    float AITickTimer = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DinoAI|State",
        meta = (AllowPrivateAccess = "true"))
    bool bIsWaitingAtDestination = false;

    // ── Internal Helpers ───────────────────────────────────────────────────────
    ADinosaurBase* GetDinoPawn() const;
    void EvaluateAndExecuteState(float DeltaTime);
    FVector PickRoamDestination() const;
    void SetupPerceptionSystem();
};
