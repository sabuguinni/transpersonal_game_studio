#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

/**
 * DinoSurvivorAnimInstance
 * AnimInstance for the prehistoric survivor player character.
 * Drives locomotion blend spaces, IK foot placement, survival state flags,
 * and aim offset for ranged/melee weapon aiming.
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

	/** Ground speed (cm/s) — drives walk/run blend space */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float Speed = 0.f;

	/** Lateral direction (-180..180) relative to actor forward */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float Direction = 0.f;

	/** Body lean angle during turns (-1..1) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float LeanAngle = 0.f;

	/** True when character is airborne */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsInAir = false;

	/** True when crouching */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsCrouching = false;

	/** True when sprinting (speed > SprintThreshold) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsSprinting = false;

	/** Speed above which sprint animations activate */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Locomotion")
	float SprintThreshold = 450.f;

	/** Walk speed threshold — below this plays idle blend */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Locomotion")
	float WalkThreshold = 10.f;

	// ── Foot IK ─────────────────────────────────────────────────────────────

	/** World-space IK target for left foot */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FVector LeftFootIKLocation = FVector::ZeroVector;

	/** World-space IK target for right foot */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FVector RightFootIKLocation = FVector::ZeroVector;

	/** Left foot IK alpha (0=off, 1=full IK) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	float LeftFootIKAlpha = 0.f;

	/** Right foot IK alpha */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	float RightFootIKAlpha = 0.f;

	/** Pelvis vertical offset driven by foot IK */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	float PelvisOffset = 0.f;

	/** Max trace distance for foot IK ground detection */
	UPROPERTY(EditDefaultsOnly, Category = "Anim|FootIK")
	float FootIKTraceDistance = 80.f;

	// ── Aim Offset ──────────────────────────────────────────────────────────

	/** Aim pitch (-90..90) for upper-body aim offset */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Aim")
	float AimPitch = 0.f;

	/** Aim yaw (-180..180) for upper-body aim offset */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Aim")
	float AimYaw = 0.f;

	// ── Survival State ───────────────────────────────────────────────────────

	/** Normalised stamina 0..1 — affects animation speed multiplier */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float StaminaNormalized = 1.f;

	/** True when stamina < 0.2 — triggers exhaustion blend */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsExhausted = false;

	/** Normalised fear 0..1 — drives fear tremor additive layer */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	float FearNormalized = 0.f;

	/** True when fear > 0.7 — triggers fear reaction pose */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsTerrified = false;

	/** True when character is injured (health < 0.3) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
	bool bIsInjured = false;

	// ── Interaction ─────────────────────────────────────────────────────────

	/** True when playing attack montage */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Interaction")
	bool bIsAttacking = false;

	/** True when gathering/crafting */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Interaction")
	bool bIsInteracting = false;

	/** True when carrying heavy object */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Interaction")
	bool bIsCarrying = false;

private:
	/** Cached owning pawn */
	UPROPERTY()
	class ACharacter* OwnerCharacter = nullptr;

	/** Previous speed for lean calculation */
	float PreviousSpeed = 0.f;

	/** Smoothed lean value */
	float SmoothedLean = 0.f;

	/** Perform a foot IK trace and return world hit location */
	FVector TraceFootIK(FName SocketName, bool& bHit) const;

	/** Update foot IK targets for both feet */
	void UpdateFootIK();

	/** Update locomotion variables from character movement */
	void UpdateLocomotion(float DeltaSeconds);

	/** Update survival state from character properties */
	void UpdateSurvivalState();

	/** Update aim offset from controller rotation */
	void UpdateAimOffset();
};
