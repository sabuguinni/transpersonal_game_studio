#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

/**
 * Animation Instance for the Transpersonal survival game character.
 * Drives locomotion blend space, jump states, and survival-state overlays.
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

	/** Ground speed used to drive the locomotion blend space (0 = idle, 600 = run) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float GroundSpeed;

	/** Lateral direction (-1 left, 0 forward, 1 right) for strafe blending */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float LateralDirection;

	/** True when the character has a non-zero velocity */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsMoving;

	/** True when the character is in the air (jumping or falling) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsInAir;

	/** True when the character is sprinting */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsSprinting;

	/** True when the character is crouching */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsCrouching;

	// ── Survival States ─────────────────────────────────────────────────────

	/** Normalised health (0-1). Drives injury overlay weight */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float HealthNormalized;

	/** Normalised stamina (0-1). Affects run animation speed multiplier */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float StaminaNormalized;

	/** Fear level (0-1). Drives alert/panic overlay */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float FearLevel;

	/** True when health < 0.3 — activates limping additive layer */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsInjured;

	/** True when fear > 0.7 — activates panic locomotion set */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsPanicking;

	// ── Combat / Tool States ─────────────────────────────────────────────────

	/** True when holding a weapon/tool — switches upper-body overlay */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
	bool bIsArmed;

	/** True during an attack montage */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
	bool bIsAttacking;

	// ── IK ──────────────────────────────────────────────────────────────────

	/** Foot IK alpha — blends procedural foot placement (0 = off, 1 = full) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	float FootIKAlpha;

	/** Left foot IK target in world space */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	FVector LeftFootIKTarget;

	/** Right foot IK target in world space */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	FVector RightFootIKTarget;

	/** Pelvis offset to keep body centred between foot IK targets */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	float PelvisOffset;

	// ── Aim Offset ──────────────────────────────────────────────────────────

	/** Aim pitch (-90 to 90) for upper-body aim offset */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
	float AimPitch;

	/** Aim yaw clamped to (-90, 90) for upper-body aim offset */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
	float AimYaw;

protected:
	/** Cached owning character */
	UPROPERTY()
	class ATranspersonalCharacter* OwnerCharacter;

private:
	/** Perform foot IK trace for one foot. Returns world hit location. */
	FVector TraceFootIK(const FName& FootSocketName, float& OutPelvisDelta) const;

	/** Smooth a float value toward a target using exponential decay */
	static float SmoothFloat(float Current, float Target, float Speed, float DeltaTime);
};
