#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SurvivorAnimInstance.generated.h"

/**
 * Animation Instance for the prehistoric human survivor character.
 * Drives locomotion blend space (idle/walk/run), jump states,
 * foot IK adaptation to terrain, and fear/stealth pose blending.
 *
 * Agent #10 — Animation Agent
 * Transpersonal Game Studio — Prehistoric Survival Game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API USurvivorAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	USurvivorAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// -------------------------------------------------------
	// LOCOMOTION PROPERTIES (read by AnimGraph blend space)
	// -------------------------------------------------------

	/** Current movement speed (0 = idle, 150 = walk, 375 = run) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
	float Speed;

	/** Lateral movement direction (-1 left, 0 forward, 1 right) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
	float Direction;

	/** True when character is in the air (jumping or falling) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	/** True when character is crouching (stealth mode) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching;

	/** True when character is sprinting */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting;

	/** True when character is moving (speed > threshold) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
	bool bIsMoving;

	// -------------------------------------------------------
	// SURVIVAL STATE PROPERTIES (affect pose blending)
	// -------------------------------------------------------

	/** Fear level 0-1 — affects hunched posture and head-look urgency */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
	float FearLevel;

	/** Stamina 0-1 — low stamina triggers exhaustion pose blend */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
	float StaminaNormalized;

	/** Health 0-1 — critical health triggers limping blend */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
	float HealthNormalized;

	/** True when character is carrying a weapon/tool */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
	bool bIsArmed;

	// -------------------------------------------------------
	// FOOT IK PROPERTIES
	// -------------------------------------------------------

	/** Left foot IK target offset from ground */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK", meta = (AllowPrivateAccess = "true"))
	FVector LeftFootIKOffset;

	/** Right foot IK target offset from ground */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK", meta = (AllowPrivateAccess = "true"))
	FVector RightFootIKOffset;

	/** Pelvis height adjustment for foot IK */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK", meta = (AllowPrivateAccess = "true"))
	float PelvisOffset;

	/** Enable foot IK terrain adaptation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK")
	bool bEnableFootIK;

	// -------------------------------------------------------
	// AIM OFFSET PROPERTIES
	// -------------------------------------------------------

	/** Pitch for aim offset (-90 to 90) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset", meta = (AllowPrivateAccess = "true"))
	float AimPitch;

	/** Yaw for aim offset (-90 to 90) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset", meta = (AllowPrivateAccess = "true"))
	float AimYaw;

	// -------------------------------------------------------
	// FUNCTIONS
	// -------------------------------------------------------

	/** Update foot IK traces against terrain */
	UFUNCTION(BlueprintCallable, Category = "Anim|FootIK")
	void UpdateFootIK(float DeltaSeconds);

	/** Get the locomotion blend space X axis value (speed) */
	UFUNCTION(BlueprintPure, Category = "Anim|Locomotion")
	float GetLocomotionSpeed() const { return Speed; }

	/** Get the locomotion blend space Y axis value (direction) */
	UFUNCTION(BlueprintPure, Category = "Anim|Locomotion")
	float GetLocomotionDirection() const { return Direction; }

private:
	/** Cached reference to owning character */
	UPROPERTY()
	class ACharacter* OwnerCharacter;

	/** Cached reference to movement component */
	UPROPERTY()
	class UCharacterMovementComponent* MovementComponent;

	/** Smooth foot IK offset — left foot */
	FVector LeftFootIKOffsetTarget;

	/** Smooth foot IK offset — right foot */
	FVector RightFootIKOffsetTarget;

	/** Trace foot position against terrain */
	bool TraceFootPosition(FName SocketName, FVector& OutIKOffset);

	/** Interpolation speed for foot IK smoothing */
	float FootIKInterpSpeed;
};
