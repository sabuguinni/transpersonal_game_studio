#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimBlueprint.generated.h"

/**
 * UTranspersonalAnimBlueprint
 * C++ AnimInstance used as the parent class for the player character's
 * Animation Blueprint.  Exposes all state variables that the Blueprint's
 * AnimGraph reads via property access nodes.
 *
 * State machine layout (implemented in the AnimGraph):
 *   Locomotion SM  →  Idle / Walk / Run / Sprint
 *   Jump SM        →  Jump_Start / Jump_Loop / Jump_Land
 *   Upper-body layer (additive) for tool-use / attack overlays
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimBlueprint : public UAnimInstance
{
	GENERATED_BODY()

public:
	UTranspersonalAnimBlueprint();

	// ── UAnimInstance interface ─────────────────────────────────────────────
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// ── Locomotion state ────────────────────────────────────────────────────

	/** Current ground speed (cm/s).  Drives BlendSpace1D axis. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
		meta = (AllowPrivateAccess = "true"))
	float GroundSpeed = 0.f;

	/** Lateral direction relative to actor forward (-180…180°).
	 *  Used by a 2D BlendSpace for strafing. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
		meta = (AllowPrivateAccess = "true"))
	float MovementDirection = 0.f;

	/** True while the character movement component reports falling. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
		meta = (AllowPrivateAccess = "true"))
	bool bIsInAir = false;

	/** True when speed > WalkSpeedThreshold. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
		meta = (AllowPrivateAccess = "true"))
	bool bIsRunning = false;

	/** True when sprint input is held AND speed > RunSpeedThreshold. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
		meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting = false;

	/** True when the character is crouching. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
		meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching = false;

	// ── Jump / land ─────────────────────────────────────────────────────────

	/** Vertical velocity (Z component, cm/s).  Drives jump arc blend. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Jump",
		meta = (AllowPrivateAccess = "true"))
	float VerticalVelocity = 0.f;

	/** Seconds since the character left the ground. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Jump",
		meta = (AllowPrivateAccess = "true"))
	float TimeInAir = 0.f;

	/** True for one tick after landing (triggers Land montage). */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Jump",
		meta = (AllowPrivateAccess = "true"))
	bool bJustLanded = false;

	// ── Survival state ──────────────────────────────────────────────────────

	/** 0-1 fatigue factor — blends in exhausted locomotion poses. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival",
		meta = (AllowPrivateAccess = "true"))
	float FatigueAlpha = 0.f;

	/** 0-1 injury factor — blends in limping poses. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival",
		meta = (AllowPrivateAccess = "true"))
	float InjuryAlpha = 0.f;

	/** 0-1 fear factor — blends in crouched/tense poses. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival",
		meta = (AllowPrivateAccess = "true"))
	float FearAlpha = 0.f;

	// ── Upper-body overlay ──────────────────────────────────────────────────

	/** Which tool / weapon the character is currently holding.
	 *  0 = unarmed, 1 = spear, 2 = torch, 3 = rock */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|UpperBody",
		meta = (AllowPrivateAccess = "true"))
	int32 EquippedItemSlot = 0;

	/** True while an attack montage is playing. */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|UpperBody",
		meta = (AllowPrivateAccess = "true"))
	bool bIsAttacking = false;

	// ── Thresholds (designer-tunable) ───────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
	float WalkSpeedThreshold = 180.f;

	UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
	float RunSpeedThreshold  = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
	float SprintSpeedThreshold = 580.f;

private:
	// Cached references — set in NativeInitializeAnimation
	UPROPERTY()
	class ACharacter* OwnerCharacter = nullptr;

	UPROPERTY()
	class UCharacterMovementComponent* MovementComponent = nullptr;

	bool bWasInAir = false;
};
