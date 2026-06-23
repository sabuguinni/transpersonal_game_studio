#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

/**
 * Animation Instance for TranspersonalCharacter
 * Drives locomotion blend space, IK foot placement, and state transitions
 * for the prehistoric human survivor character.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UTranspersonalAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// ─── Locomotion Properties ───────────────────────────────────────────────

	/** Current movement speed (0 = idle, 300 = walk, 600 = run) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float Speed;

	/** Lateral movement direction (-1 left, 0 straight, 1 right) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float Direction;

	/** True when character is in the air */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsInAir;

	/** True when character is crouching */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsCrouching;

	/** True when character is sprinting */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsSprinting;

	/** True when character is moving */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsMoving;

	// ─── Survival State Properties ───────────────────────────────────────────

	/** Health percentage (0.0 - 1.0) — affects posture and movement quality */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float HealthPercent;

	/** Stamina percentage (0.0 - 1.0) — affects run speed and breathing */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float StaminaPercent;

	/** Fear level (0.0 - 1.0) — affects idle fidget frequency */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float FearLevel;

	/** True when character is injured (health < 0.3) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsInjured;

	/** True when character is exhausted (stamina < 0.2) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsExhausted;

	// ─── IK Foot Placement ───────────────────────────────────────────────────

	/** Left foot IK target location in world space */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	FVector LeftFootIKLocation;

	/** Right foot IK target location in world space */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	FVector RightFootIKLocation;

	/** Left foot IK rotation */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	FRotator LeftFootIKRotation;

	/** Right foot IK rotation */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	FRotator RightFootIKRotation;

	/** Pelvis offset for IK foot placement on uneven terrain */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	float PelvisOffset;

	/** True when IK foot placement is active (on ground) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	bool bEnableFootIK;

	// ─── Combat & Interaction ────────────────────────────────────────────────

	/** True when character is holding a weapon */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
	bool bIsArmed;

	/** True when character is attacking */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
	bool bIsAttacking;

	/** Upper body additive weight for aim offset */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
	float AimPitch;

	/** Aim yaw for upper body rotation */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
	float AimYaw;

	// ─── Blend Space Inputs ──────────────────────────────────────────────────

	/** Blend space X axis — forward/backward speed */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|BlendSpace")
	float BS_Speed;

	/** Blend space Y axis — strafe direction */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|BlendSpace")
	float BS_Direction;

protected:
	/** Update locomotion variables from character movement */
	void UpdateLocomotion(float DeltaSeconds);

	/** Update survival state variables from character stats */
	void UpdateSurvivalState();

	/** Update IK foot placement via line traces */
	void UpdateFootIK(float DeltaSeconds);

	/** Perform foot IK trace for a given socket */
	bool TraceFootIK(FName SocketName, FVector& OutLocation, FRotator& OutRotation);

private:
	/** Cached reference to owning character */
	UPROPERTY()
	class ATranspersonalCharacter* OwnerCharacter;

	/** Cached movement component */
	UPROPERTY()
	class UCharacterMovementComponent* MovementComponent;

	/** IK trace channel */
	static const float FootIKTraceDistance;

	/** Smooth pelvis offset interpolation speed */
	static const float PelvisInterpSpeed;
};
