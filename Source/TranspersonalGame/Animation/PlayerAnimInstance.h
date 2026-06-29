#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 * UPlayerAnimInstance
 * Animation instance for the TranspersonalCharacter (player).
 * Drives locomotion blend spaces, survival state transitions,
 * foot IK, and lean/tilt based on movement physics.
 *
 * Agent #10 — Animation Agent | Transpersonal Game Studio
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPlayerAnimInstance();

	// ─── UAnimInstance Interface ───────────────────────────────────────
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativePostEvaluateAnimation() override;

	// ─── Locomotion ────────────────────────────────────────────────────
	/** World-space speed (cm/s) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float Speed;

	/** Exponentially-smoothed speed for blend space input */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float SmoothedSpeed;

	/** Strafe direction angle (-180..180) relative to actor forward */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float Direction;

	/** True when horizontal speed > 10 cm/s */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsMoving;

	/** True when sprint input is held and speed > walk threshold */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsSprinting;

	/** True when crouch is active */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsCrouching;

	/** True when character is airborne */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsInAir;

	/** Vertical velocity (positive = rising, negative = falling) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float VerticalVelocity;

	/** Lean amount for banking turns (-1..1, left..right) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float LeanAmount;

	/** Pitch tilt for slopes (-1..1) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float PitchTilt;

	// ─── Survival States ───────────────────────────────────────────────
	/** Sneaking (low-profile movement) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	bool bIsSneaking;

	/** Climbing a surface */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	bool bIsClimbing;

	/** Fear level 0-1 (affects posture and movement blend) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	float FearLevel;

	/** Stamina level 0-1 (affects breathing and movement quality) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	float StaminaLevel;

	/** Health level 0-1 (low health = limping posture) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	float HealthLevel;

	/** True when carrying a heavy object */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	bool bIsCarrying;

	/** True when aiming/throwing a weapon */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	bool bIsAiming;

	// ─── Foot IK ───────────────────────────────────────────────────────
	/** Left foot IK target offset in component space */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
	FVector LeftFootIKOffset;

	/** Right foot IK target offset in component space */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
	FVector RightFootIKOffset;

	/** Left foot IK alpha (0=off, 1=full IK) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
	float LeftFootIKAlpha;

	/** Right foot IK alpha (0=off, 1=full IK) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
	float RightFootIKAlpha;

	/** Pelvis vertical offset to keep feet on ground */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
	float PelvisOffset;

	// ─── Blend Space Inputs ────────────────────────────────────────────
	/** Input X for locomotion blend space (direction -180..180) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace")
	float BlendSpaceX;

	/** Input Y for locomotion blend space (speed 0..600) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace")
	float BlendSpaceY;

	// ─── Jump / Land ───────────────────────────────────────────────────
	/** Time since leaving the ground (for jump arc blend) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Jump")
	float TimeInAir;

	/** True during the first 0.15s after landing */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Jump")
	bool bJustLanded;

	/** Landing impact speed (used to select hard/soft land anim) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Jump")
	float LandingImpactSpeed;

	// ─── Tuning ────────────────────────────────────────────────────────
	/** Speed smoothing factor (higher = faster response) */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Tuning")
	float SpeedSmoothingRate;

	/** Lean smoothing factor */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Tuning")
	float LeanSmoothingRate;

	/** Foot IK trace distance below foot bone */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|FootIK")
	float FootIKTraceDistance;

	/** Foot IK trace distance above foot bone */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|FootIK")
	float FootIKTraceUpDistance;

private:
	// ─── Internal helpers ──────────────────────────────────────────────
	void UpdateLocomotion(float DeltaSeconds);
	void UpdateSurvivalStates(float DeltaSeconds);
	void UpdateFootIK(float DeltaSeconds);
	void UpdateJumpState(float DeltaSeconds);
	void SolveFootIK(FName FootBoneName, FVector& OutIKOffset, float& OutIKAlpha, float DeltaSeconds);

	/** Cached owning character */
	UPROPERTY()
	class ACharacter* OwnerCharacter;

	/** Cached movement component */
	UPROPERTY()
	class UCharacterMovementComponent* MovementComponent;

	/** Previous frame velocity for lean calculation */
	FVector PreviousVelocity;

	/** Accumulated time in air */
	float AirTime;

	/** Landing reset timer */
	float LandedTimer;

	/** Previous bIsInAir for landing detection */
	bool bWasInAir;
};
