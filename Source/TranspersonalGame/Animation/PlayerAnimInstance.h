#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 * UPlayerAnimInstance
 * Animation instance for the prehistoric human player character.
 * Drives locomotion blend spaces, survival state transitions,
 * combat poses, and foot IK for terrain adaptation.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativePostEvaluateAnimation() override;

    // ─── Locomotion ───────────────────────────────────────────────────────────

    /** Current ground speed (cm/s) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float Speed;

    /** Exponentially smoothed speed for blend space input */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float SmoothedSpeed;

    /** Strafe direction angle (-180 to 180) relative to movement direction */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float Direction;

    /** True when character velocity magnitude > 10 cm/s */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsMoving;

    /** True when sprinting (shift held, stamina > 0) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting;

    /** True when crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    /** True when character is airborne */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    /** Lateral lean amount for banking turns (-1 left, +1 right) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float LeanAmount;

    /** Vertical velocity for jump/fall blend */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float VerticalVelocity;

    // ─── Survival States ──────────────────────────────────────────────────────

    /** True when sneaking (low stance, slow movement) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsSneaking;

    /** True when climbing a surface */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsClimbing;

    /** Fear level 0-1 — affects breathing, head movement, posture */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float FearLevel;

    /** Stamina 0-1 — affects run posture, breathing intensity */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float StaminaLevel;

    /** Hunger 0-1 — affects idle posture (hunched when starving) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float HungerLevel;

    /** True when character is exhausted (stamina < 0.1) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsExhausted;

    // ─── Combat ───────────────────────────────────────────────────────────────

    /** True when in combat stance */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsInCombat;

    /** True when attacking (melee swing active) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking;

    /** True when blocking/guarding */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsBlocking;

    /** True when throwing a projectile */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsThrowing;

    /** True when character is hit (triggers hit reaction montage) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsHit;

    /** True when character is dead */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsDead;

    /** Equipped weapon type index (0=none, 1=spear, 2=club, 3=bow) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    int32 WeaponTypeIndex;

    // ─── Interaction ──────────────────────────────────────────────────────────

    /** True when gathering resources (picking up, harvesting) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Interaction", meta = (AllowPrivateAccess = "true"))
    bool bIsGathering;

    /** True when crafting at a workstation */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Interaction", meta = (AllowPrivateAccess = "true"))
    bool bIsCrafting;

    /** True when swimming */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Interaction", meta = (AllowPrivateAccess = "true"))
    bool bIsSwimming;

    // ─── Foot IK ──────────────────────────────────────────────────────────────

    /** Left foot IK target offset from default bone position */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    FVector LeftFootIKOffset;

    /** Right foot IK target offset from default bone position */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    FVector RightFootIKOffset;

    /** Pelvis vertical offset to keep feet planted on uneven terrain */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    float PelvisOffset;

    /** Left foot IK alpha (0=off, 1=full IK) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    float LeftFootIKAlpha;

    /** Right foot IK alpha (0=off, 1=full IK) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    float RightFootIKAlpha;

    // ─── AimOffset ────────────────────────────────────────────────────────────

    /** Aim pitch for upper body aim offset (-90 to 90) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset", meta = (AllowPrivateAccess = "true"))
    float AimPitch;

    /** Aim yaw for upper body aim offset (-90 to 90) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset", meta = (AllowPrivateAccess = "true"))
    float AimYaw;

    // ─── Blueprint callable utilities ─────────────────────────────────────────

    /** Play a one-shot montage by name (e.g., "Attack_Spear", "HitReact_Front") */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayMontageByName(const FName& MontageName, float PlayRate = 1.0f);

    /** Stop the currently active montage */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopActiveMontage(float BlendOutTime = 0.25f);

    /** Returns true if any montage is currently playing */
    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsAnyMontageActive() const;

private:
    /** Cached reference to the owning character */
    UPROPERTY()
    class ATranspersonalCharacter* OwnerCharacter;

    /** Cached movement component */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    /** Speed smoothing factor (higher = snappier) */
    float SpeedSmoothingAlpha;

    /** Lean smoothing factor */
    float LeanSmoothingAlpha;

    /** Previous frame velocity for lean calculation */
    FVector PreviousVelocity;

    /** Foot IK trace channel */
    TEnumAsByte<ECollisionChannel> FootIKTraceChannel;

    /** Foot IK trace half-height (cm) */
    float FootIKTraceHalfHeight;

    /** Maximum pelvis correction distance (cm) */
    float MaxPelvisCorrection;

    /** Internal: update locomotion variables from movement component */
    void UpdateLocomotion(float DeltaSeconds);

    /** Internal: update survival state variables from character */
    void UpdateSurvivalStates(float DeltaSeconds);

    /** Internal: update combat variables from character */
    void UpdateCombatStates(float DeltaSeconds);

    /** Internal: perform foot IK raycasts and update offsets */
    void UpdateFootIK(float DeltaSeconds);

    /** Internal: update aim offset from control rotation */
    void UpdateAimOffset(float DeltaSeconds);

    /** Internal: smooth a float value toward target */
    float SmoothFloat(float Current, float Target, float Alpha, float DeltaSeconds) const;
};
