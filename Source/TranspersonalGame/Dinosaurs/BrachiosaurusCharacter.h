// BrachiosaurusCharacter.h
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260624_002
// Brachiosaurus — large herbivore subclass of ADinosaurBase
// Species: Brachiosaurus | MaxHealth: 3000 | AttackDamage: 20 (tail swipe) | MaxWalkSpeed: 300

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "BrachiosaurusCharacter.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABrachiosaurusCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ABrachiosaurusCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Tail Swipe Attack ──────────────────────────────────────────────────
    /** Perform a wide-arc tail swipe dealing AoE damage to nearby threats */
    UFUNCTION(BlueprintCallable, Category = "Brachiosaurus|Combat")
    void PerformTailSwipe();

    // ── Passive Flee Behaviour ─────────────────────────────────────────────
    /** Evaluate nearby threats and trigger flee if within FleeRadius */
    UFUNCTION(BlueprintCallable, Category = "Brachiosaurus|Behaviour")
    void EvaluateFleeResponse();

    /** Stomp the ground, creating a shockwave that knocks back small creatures */
    UFUNCTION(BlueprintCallable, Category = "Brachiosaurus|Combat")
    void PerformGroundStomp();

    /** Emit a low-frequency rumble to warn herd members of danger */
    UFUNCTION(BlueprintCallable, Category = "Brachiosaurus|Social")
    void EmitHerdWarning();

    // ── Properties ────────────────────────────────────────────────────────
    /** Radius within which the Brachiosaurus detects threats and begins fleeing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachiosaurus|Behaviour")
    float FleeRadius;

    /** Radius of the tail swipe AoE damage zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachiosaurus|Combat")
    float TailSwipeRadius;

    /** Knockback impulse applied by ground stomp */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachiosaurus|Combat")
    float StompKnockbackImpulse;

    /** Minimum time between tail swipe attacks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachiosaurus|Combat")
    float TailSwipeCooldown;

    /** Whether this Brachiosaurus is currently fleeing */
    UPROPERTY(BlueprintReadOnly, Category = "Brachiosaurus|State")
    bool bIsFleeing;

    /** Current flee target location */
    UPROPERTY(BlueprintReadOnly, Category = "Brachiosaurus|State")
    FVector FleeTargetLocation;

    /** Nearby herd members (other Brachiosaurus within DetectionRange) */
    UPROPERTY(BlueprintReadOnly, Category = "Brachiosaurus|Social")
    TArray<ABrachiosaurusCharacter*> HerdMembers;

protected:
    /** Timer handle for periodic flee evaluation */
    FTimerHandle FleeEvaluationTimer;

    /** Timer handle for tail swipe cooldown */
    FTimerHandle TailSwipeCooldownTimer;

    /** Whether the tail swipe is currently on cooldown */
    bool bTailSwipeReady;

    /** Scan for nearby herd members and populate HerdMembers array */
    void UpdateHerdMembers();

    /** Broadcast flee direction to all herd members */
    void CoordinateHerdFlee(const FVector& ThreatLocation);
};
