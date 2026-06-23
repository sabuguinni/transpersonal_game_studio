#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

/**
 * Animation instance for the prehistoric survivor player character.
 * Drives locomotion blend space (idle/walk/run), jump, crouch, and combat states.
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

	/** Ground speed (cm/s) — drives the locomotion blend space X axis */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
	float GroundSpeed;

	/** Direction offset (-180..180) relative to actor forward — blend space Y axis */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
	float MovementDirection;

	/** True when the character velocity has a meaningful horizontal component */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
	bool bIsMoving;

	/** True when the character is in the air (falling or jumping) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	/** True when the character is crouching */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching;

	/** True when the character is sprinting (speed > SprintThreshold) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting;

	// ── Survival State ───────────────────────────────────────────────────────

	/** 0-100 stamina — affects animation speed scale when low */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
	float Stamina;

	/** 0-100 health — triggers limping additive layer below LimpThreshold */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
	float Health;

	/** True when health < LimpHealthThreshold — activates limp additive */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
	bool bIsLimping;

	/** Fear level 0-100 — drives tremor additive at high values */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
	float FearLevel;

	// ── Combat ───────────────────────────────────────────────────────────────

	/** True when the character is holding a weapon (spear, club, etc.) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsArmed;

	/** True during an active attack montage */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsAttacking;

	// ── Thresholds ───────────────────────────────────────────────────────────

	/** Speed above which bIsSprinting becomes true (cm/s) */
	UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
	float SprintThreshold;

	/** Health below which bIsLimping becomes true */
	UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
	float LimpHealthThreshold;

	/** Minimum speed to consider the character as moving */
	UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
	float MinMoveSpeed;

private:
	/** Cached reference to the owning character movement component */
	UPROPERTY()
	class UCharacterMovementComponent* CachedMovementComp;

	/** Cached reference to the owning TranspersonalCharacter */
	UPROPERTY()
	class ATranspersonalCharacter* CachedCharacter;
};
