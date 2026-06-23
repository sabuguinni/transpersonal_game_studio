#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivalAnimInstance.generated.h"

/**
 * Animation Instance for the prehistoric human survivor character.
 * Drives locomotion blend space, IK foot placement, and survival state animations.
 * Agent #10 — Animation Agent
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UDinoSurvivalAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// ─── Locomotion ───────────────────────────────────────────────────────────

	/** Current ground speed (cm/s). Drives walk/run blend space. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float GroundSpeed = 0.0f;

	/** Direction of movement relative to character facing (-180 to 180). */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float MovementDirection = 0.0f;

	/** True when character is moving (speed > 10 cm/s). */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsMoving = false;

	/** True when character is in the air. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsInAir = false;

	/** True when character is crouching. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsCrouching = false;

	/** True when character is sprinting (speed > 450 cm/s). */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsSprinting = false;

	// ─── Survival States ──────────────────────────────────────────────────────

	/** Stamina (0-100). Low stamina affects movement animations. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	float Stamina = 100.0f;

	/** Fear level (0-100). High fear triggers alert/panic animations. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	float FearLevel = 0.0f;

	/** True when character is in combat stance (weapon drawn). */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	bool bIsInCombatStance = false;

	/** True when character is carrying a heavy item. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	bool bIsCarryingHeavyItem = false;

	// ─── IK Foot Placement ────────────────────────────────────────────────────

	/** Left foot IK target location in world space. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FVector LeftFootIKLocation = FVector::ZeroVector;

	/** Right foot IK target location in world space. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FVector RightFootIKLocation = FVector::ZeroVector;

	/** Left foot IK rotation. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FRotator LeftFootIKRotation = FRotator::ZeroRotator;

	/** Right foot IK rotation. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FRotator RightFootIKRotation = FRotator::ZeroRotator;

	/** Pelvis offset for foot IK (negative = lower pelvis on uneven terrain). */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	float PelvisOffset = 0.0f;

	// ─── Aim Offset ───────────────────────────────────────────────────────────

	/** Pitch of aim direction (-90 to 90). Used for upper body aim offset. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
	float AimPitch = 0.0f;

	/** Yaw of aim direction (-180 to 180). Used for upper body aim offset. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
	float AimYaw = 0.0f;

protected:
	/** Update locomotion variables from owning character. */
	void UpdateLocomotion(float DeltaSeconds);

	/** Update IK foot placement via line traces. */
	void UpdateFootIK(float DeltaSeconds);

	/** Update aim offset from controller rotation. */
	void UpdateAimOffset();

	/** Update survival state variables. */
	void UpdateSurvivalState();

private:
	/** Cached reference to owning character. */
	UPROPERTY()
	class ACharacter* OwnerCharacter = nullptr;

	/** Cached reference to character movement component. */
	UPROPERTY()
	class UCharacterMovementComponent* MovementComponent = nullptr;

	/** IK trace distance below foot. */
	static constexpr float IKTraceDistance = 55.0f;

	/** IK interpolation speed for smooth foot placement. */
	static constexpr float IKInterpSpeed = 15.0f;

	/** Cached left foot IK (interpolated). */
	FVector LeftFootIKCached = FVector::ZeroVector;

	/** Cached right foot IK (interpolated). */
	FVector RightFootIKCached = FVector::ZeroVector;
};
