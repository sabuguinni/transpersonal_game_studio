#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

/**
 * Animation Instance for TranspersonalCharacter.
 * Drives idle/walk/run/sprint/jump/crouch blend spaces via character velocity
 * and movement state. Designed for Motion Matching readiness with foot IK.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UTranspersonalAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// ── Locomotion State ──
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float LateralSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsMoving;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsSprinting;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsCrouching;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsFalling;

	// ── Survival State (drives animation urgency) ──
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float StaminaNormalized;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float FearLevel;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsExhausted;

	// ── Direction for blend space ──
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Direction")
	float MovementDirection;

	// ── Foot IK ──
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	FVector LeftFootIKLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	FVector RightFootIKLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	float LeftFootIKAlpha;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	float RightFootIKAlpha;

	// ── Blend Weights ──
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
	float WalkRunBlend;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
	float SprintBlend;

private:
	UPROPERTY()
	class ACharacter* OwnerCharacter;

	void UpdateLocomotionState(float DeltaSeconds);
	void UpdateFootIK(float DeltaSeconds);
	void UpdateSurvivalState(float DeltaSeconds);
	float CalculateMovementDirection() const;
	bool TraceFootIK(const FName& SocketName, FVector& OutLocation, float& OutAlpha) const;
};
