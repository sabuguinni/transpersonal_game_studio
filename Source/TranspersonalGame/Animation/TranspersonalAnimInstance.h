#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

/**
 * Animation Instance for the Transpersonal Game player character.
 * Drives locomotion state machine: Idle, Walk, Run, Jump, Fall, Land.
 * Uses foot IK to adapt to terrain surface normals.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UTranspersonalAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// ── Locomotion State ────────────────────────────────────────────────────

	/** Current movement speed (cm/s) — drives blend space axis */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float Speed = 0.0f;

	/** Lateral movement direction (-180 to 180) for strafe blending */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float Direction = 0.0f;

	/** True when character is in the air */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsInAir = false;

	/** True when character is accelerating */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsAccelerating = false;

	/** True when character is crouching */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsCrouching = false;

	/** True when character is sprinting (speed > RunThreshold) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsSprinting = false;

	// ── Survival State ──────────────────────────────────────────────────────

	/** Stamina 0-100. Low stamina affects movement animation weight */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float Stamina = 100.0f;

	/** Fear level 0-100. High fear triggers trembling additive layer */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float Fear = 0.0f;

	/** Health 0-100. Low health triggers limping blend */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float Health = 100.0f;

	// ── Foot IK ─────────────────────────────────────────────────────────────

	/** Left foot IK target location in world space */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	FVector LeftFootIKLocation = FVector::ZeroVector;

	/** Right foot IK target location in world space */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	FVector RightFootIKLocation = FVector::ZeroVector;

	/** Left foot IK alpha (0=off, 1=full IK) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	float LeftFootIKAlpha = 0.0f;

	/** Right foot IK alpha (0=off, 1=full IK) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	float RightFootIKAlpha = 0.0f;

	// ── Thresholds ──────────────────────────────────────────────────────────

	/** Speed above which character transitions to Run animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
	float RunThreshold = 300.0f;

	/** Speed above which character transitions to Sprint animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
	float SprintThreshold = 500.0f;

	/** Trace distance for foot IK ground detection */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
	float FootIKTraceDistance = 50.0f;

protected:
	/** Cached reference to the owning character */
	UPROPERTY()
	class ACharacter* OwnerCharacter = nullptr;

	/** Cached reference to the character movement component */
	UPROPERTY()
	class UCharacterMovementComponent* MovementComponent = nullptr;

	/** Update foot IK targets via line traces to ground */
	void UpdateFootIK(float DeltaSeconds);

	/** Perform a single foot IK trace and return the hit location */
	bool TraceFootIK(FName SocketName, FVector& OutLocation);

	/** Interpolate foot IK alpha smoothly */
	float InterpFootIKAlpha(float Current, float Target, float DeltaSeconds, float Speed = 10.0f);
};
