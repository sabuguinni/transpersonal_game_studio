#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UDinoSurvivorAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// --- Locomotion ---
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

	// --- Aim Offset ---
	UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
	float AimPitch;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
	float AimYaw;

	// --- Foot IK ---
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FVector LeftFootIKLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FVector RightFootIKLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FRotator LeftFootIKRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FRotator RightFootIKRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	float LeftFootIKAlpha;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	float RightFootIKAlpha;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FVector PelvisOffset;

	// --- Survival State ---
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float StaminaNormalized;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float FearNormalized;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsExhausted;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsInjured;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsDead;

private:
	void UpdateLocomotion(float DeltaSeconds);
	void UpdateFootIK(float DeltaSeconds);
	void UpdateAimOffset(float DeltaSeconds);
	void UpdateSurvivalState(float DeltaSeconds);

	float LeanAngleSmoothed;
	float SpeedSmoothed;
};
