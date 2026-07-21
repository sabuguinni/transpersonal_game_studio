#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SurvivorAnimInstance.generated.h"

/**
 * USurvivorAnimInstance
 * Animation instance for the prehistoric survivor player character.
 * Drives locomotion blend space (idle/walk/run), jump states,
 * crouching, and foot IK placement on uneven terrain.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API USurvivorAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	USurvivorAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// ── Locomotion ──────────────────────────────────────────────────────────

	/** Current movement speed (cm/s). Drives the locomotion blend space. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float Speed;

	/** Lateral strafe direction (-1 left, 0 forward, 1 right). */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float Direction;

	/** True when the character is moving (Speed > IdleThreshold). */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsMoving;

	/** True when the character is sprinting (Speed > SprintThreshold). */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsSprinting;

	/** True when the character is crouching. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsCrouching;

	// ── Airborne ────────────────────────────────────────────────────────────

	/** True when the character is in the air (jumping or falling). */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Airborne")
	bool bIsInAir;

	/** Vertical velocity (Z component). Positive = rising, negative = falling. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Airborne")
	float VerticalVelocity;

	// ── Survival State ──────────────────────────────────────────────────────

	/** Stamina ratio [0..1]. Affects animation speed and posture. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float StaminaRatio;

	/** Fear level [0..1]. Triggers fear animations (trembling, cowering). */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float FearLevel;

	/** True when health is critically low (<20%). Triggers limping. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsInjured;

	// ── Foot IK ─────────────────────────────────────────────────────────────

	/** Left foot IK target location in component space. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FVector LeftFootIKLocation;

	/** Right foot IK target location in component space. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FVector RightFootIKLocation;

	/** IK alpha blend [0..1]. 0 = no IK, 1 = full IK. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	float FootIKAlpha;

	// ── Thresholds (tunable in Blueprint) ───────────────────────────────────

	/** Speed below which the character is considered idle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Config")
	float IdleThreshold;

	/** Speed above which the character is considered sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Config")
	float SprintThreshold;

	/** IK trace distance below the foot bone (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Config")
	float FootIKTraceDistance;

protected:
	/** Updates locomotion variables from the owning character's movement component. */
	void UpdateLocomotion();

	/** Updates survival state variables from the owning character's stats. */
	void UpdateSurvivalState();

	/** Performs two line traces (left/right foot) and updates IK target locations. */
	void UpdateFootIK(float DeltaSeconds);

private:
	/** Cached reference to the owning ACharacter. Set in NativeInitializeAnimation. */
	UPROPERTY()
	class ACharacter* OwnerCharacter;

	/** Cached reference to the owning character's movement component. */
	UPROPERTY()
	class UCharacterMovementComponent* MovementComponent;

	/** Smooth IK alpha interpolation speed. */
	float IKInterpSpeed;
};
