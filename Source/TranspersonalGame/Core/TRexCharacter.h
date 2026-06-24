// TRexCharacter.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Tyrannosaurus Rex — apex predator subclass of ADinosaurBase
// UE5.5 compliant — TRANSPERSONALGAME_API exported

#pragma once

#include "CoreMinimal.h"
#include "Core/DinosaurBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter
 *
 * Tyrannosaurus Rex — the apex predator of the prehistoric survival game.
 * Extends ADinosaurBase with T-Rex specific stats:
 *   - Capsule: radius 120, half-height 280 (large bipedal predator)
 *   - Walk speed: 350 cm/s  |  Run speed: 900 cm/s
 *   - Bite damage: 150 (one-shot kills most prey)
 *   - Detection radius: 3000 cm (large sensory cone)
 *   - Roar ability: stuns nearby prey for 2 seconds
 *
 * AI: Uses BTT_TRex BehaviorTree (patrol → detect → chase → attack).
 * Mesh: /Game/Dinosaur_Pack/Trex/Mesh/SKM_Trex_Skin (scale 3.0)
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    // ── Roar ability ─────────────────────────────────────────────────────────

    /** Roar — stuns nearby prey within RoarRadius for RoarStunDuration seconds */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformRoar();

    /** Radius within which the roar stuns prey (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarRadius = 1500.0f;

    /** Duration prey is stunned after a roar (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarStunDuration = 2.0f;

    /** Cooldown between roars (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarCooldown = 30.0f;

    // ── Detection ────────────────────────────────────────────────────────────

    /** Detection radius — T-Rex has excellent vision and smell (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|AI")
    float DetectionRadius = 3000.0f;

    /** Field of view for visual detection (degrees, forward cone) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|AI")
    float DetectionFOV = 120.0f;

    // ── BehaviorTree ─────────────────────────────────────────────────────────

    /** BehaviorTree asset for T-Rex AI (patrol → detect → chase → attack) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|AI")
    TObjectPtr<UBehaviorTree> TRexBehaviorTree;

    // ── Stomp ─────────────────────────────────────────────────────────────────

    /** Ground stomp — area damage when T-Rex lands a heavy step */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformStomp();

    /** Stomp radius (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius = 400.0f;

    /** Stomp damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamage = 80.0f;

protected:
    virtual void BeginPlay() override;

    /** Blueprint event — fired when T-Rex roars */
    UFUNCTION(BlueprintImplementableEvent, Category = "TRex|Events")
    void OnTRexRoar();

    /** Blueprint event — fired when T-Rex stomps */
    UFUNCTION(BlueprintImplementableEvent, Category = "TRex|Events")
    void OnTRexStomp();

private:
    /** Timestamp of last roar — used for cooldown check */
    float LastRoarTime = -999.0f;

    /** Whether the roar cooldown has elapsed */
    bool CanRoar() const;
};
