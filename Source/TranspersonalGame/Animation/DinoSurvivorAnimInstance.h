#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

/**
 * Animation Instance for the prehistoric human survivor character.
 * Drives locomotion blend spaces, foot IK, aim offset, and survival state flags.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UDinoSurvivorAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// ── Locomotion ──────────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float LeanAngle;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsCrouching;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsSprinting;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsMoving;

	// ── Foot IK ─────────────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FVector LeftFootIKLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FVector RightFootIKLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FRotator LeftFootIKRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FRotator RightFootIKRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	float PelvisOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	float LeftFootIKAlpha;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	float RightFootIKAlpha;

	// ── Aim Offset ──────────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
	float AimPitch;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
	float AimYaw;

	// ── Survival State ──────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float StaminaNormalized;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float FearNormalized;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsExhausted;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsInjured;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsFleeing;

	// ── Smooth interpolation targets ────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Internal")
	float SmoothedSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Internal")
	float SmoothedDirection;

private:
	void UpdateLocomotion(float DeltaSeconds);
	void UpdateFootIK(float DeltaSeconds);
	void UpdateAimOffset(float DeltaSeconds);
	void UpdateSurvivalState(float DeltaSeconds);

	FVector TraceFootIK(FName SocketName, FVector& OutHitNormal);

	UPROPERTY()
	class ACharacter* OwnerCharacter;

	UPROPERTY()
	class UCharacterMovementComponent* MovementComponent;

	float LeanAngleVelocity;
	float SpeedInterpVelocity;
	float DirectionInterpVelocity;
};
