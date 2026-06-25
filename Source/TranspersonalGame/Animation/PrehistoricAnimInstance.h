// PrehistoricAnimInstance.h
// Animation Agent #10 — Transpersonal Game Studio
// AnimInstance for the prehistoric survivor character.
// Exposes locomotion state variables consumed by the Animation Blueprint.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PrehistoricAnimInstance.generated.h"

class ACharacter;

/**
 * UPrehistoricAnimInstance
 *
 * Drives the survivor character's animation state machine.
 * Reads velocity, movement flags and survival stats each tick,
 * then exposes clean float/bool properties for the AnimBlueprint
 * to blend between Idle, Walk, Run, Crouch, Jump, Attack and Death states.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPrehistoricAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPrehistoricAnimInstance();

    // ── UAnimInstance interface ─────────────────────────────────────────────
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Called by the character when it lands after a jump ─────────────────
    UFUNCTION(BlueprintCallable, Category = "Animation|Events")
    void OnLanded();

    // ── State setters (called by Character/Combat components) ───────────────
    UFUNCTION(BlueprintCallable, Category = "Animation|State")
    void SetAttacking(bool bAttacking);

    UFUNCTION(BlueprintCallable, Category = "Animation|State")
    void SetDead(bool bDead);

    /** 0=calm, 1=max fear — affects trembling/crouching blend weight */
    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void SetFearIntensity(float Intensity);

    /** 0=exhausted, 1=full stamina — scales animation playback speed */
    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void SetStaminaRatio(float Ratio);

    /** Returns animation playback speed multiplier (0.75–1.0) */
    UFUNCTION(BlueprintPure, Category = "Animation|Survival")
    float GetAnimSpeedScale() const;

    // ── Locomotion state (read by AnimBlueprint) ────────────────────────────

    /** Ground speed (cm/s, XY plane only) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed;

    /** Strafe direction angle relative to actor forward (-180..180) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction;

    /** True while the character is airborne */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir;

    /** True while the character is crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching;

    /** True when speed > 400 and on ground (sprint threshold) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting;

    /** Set externally by the combat system */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsAttacking;

    /** Set externally when health reaches zero */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsDead;

    /** Lateral lean angle driven by strafe direction (-15..15 degrees) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float LeanAngle;

    /** Control rotation pitch for aim offset (-90..90) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float AimPitch;

    /** 1.0 on landing, fades to 0 — drives landing recovery blend */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float LandedBlendAlpha;

    /** 0=calm, 1=max fear — drives trembling/cowering blend */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearIntensity;

    /** 0=exhausted, 1=full — scales animation speed */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaRatio;

private:
    UPROPERTY()
    ACharacter* OwnerCharacter;
};
