#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

/**
 * Animation Instance for the Transpersonal prehistoric survival character.
 * Drives locomotion blend space, jump states, and survival-state driven poses.
 * Agent #10 — Animation Agent
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────

    /** Ground speed used to drive the locomotion blend space (0 = idle, >0 = moving) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float GroundSpeed;

    /** Lateral strafe direction (-1 left, 0 forward, 1 right) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float StrafeDirection;

    /** True when the character is accelerating (used to trigger start animations) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsAccelerating;

    /** True when the character velocity exceeds the run threshold */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsRunning;

    /** True when the character is crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching;

    // ── Airborne ────────────────────────────────────────────────────────────

    /** True when the character is in the air */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Airborne")
    bool bIsInAir;

    /** Vertical velocity — positive = ascending, negative = falling */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Airborne")
    float VerticalVelocity;

    // ── Survival States ─────────────────────────────────────────────────────

    /** Stamina ratio [0..1] — affects movement animation intensity */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float StaminaRatio;

    /** Fear level [0..1] — drives fear-based pose blending */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float FearLevel;

    /** True when the character is severely wounded (health < 25%) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsWounded;

    // ── Combat ──────────────────────────────────────────────────────────────

    /** True when the character is in combat stance */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsInCombat;

    /** True when a melee attack montage should be triggered */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsAttacking;

    // ── Thresholds ──────────────────────────────────────────────────────────

    /** Speed above which the character transitions to run animation */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float RunSpeedThreshold;

    /** Speed below which the character is considered idle */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float IdleSpeedThreshold;

private:
    /** Cached reference to the owning character */
    UPROPERTY()
    class ATranspersonalCharacter* OwnerCharacter;

    /** Cached movement component */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    /** Update locomotion variables from movement component */
    void UpdateLocomotion(float DeltaSeconds);

    /** Update survival state variables from character stats */
    void UpdateSurvivalStates();
};
