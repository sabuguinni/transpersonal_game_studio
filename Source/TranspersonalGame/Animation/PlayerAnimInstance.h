#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Walk        UMETA(DisplayName = "Walk"),
	Run         UMETA(DisplayName = "Run"),
	Sprint      UMETA(DisplayName = "Sprint"),
	Crouch      UMETA(DisplayName = "Crouch"),
	InAir       UMETA(DisplayName = "InAir"),
	Land        UMETA(DisplayName = "Land"),
	Climb       UMETA(DisplayName = "Climb"),
	Sneak       UMETA(DisplayName = "Sneak")
};

UENUM(BlueprintType)
enum class EAnim_WeaponState : uint8
{
	Unarmed     UMETA(DisplayName = "Unarmed"),
	Spear       UMETA(DisplayName = "Spear"),
	Club        UMETA(DisplayName = "Club"),
	Bow         UMETA(DisplayName = "Bow"),
	Torch       UMETA(DisplayName = "Torch")
};

/**
 * UPlayerAnimInstance
 * Animation Instance for the TranspersonalCharacter (prehistoric human survivor).
 * Drives locomotion state machine, IK foot placement, weapon pose blending.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPlayerAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativePostEvaluateAnimation() override;

	// ── Locomotion ────────────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float SmoothedSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float LeanAmount;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float VerticalVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsMoving;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsSprinting;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsCrouching;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsSneaking;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsClimbing;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bJustLanded;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	EAnim_LocomotionState LocomotionState;

	// ── Survival / Condition ──────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	float HealthNormalized;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	float StaminaNormalized;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	float FearLevel;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	bool bIsExhausted;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	bool bIsInjured;

	// ── Weapon / Combat ───────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
	EAnim_WeaponState WeaponState;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
	bool bIsAttacking;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
	float AimPitch;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
	float AimYaw;

	// ── Foot IK ───────────────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FVector LeftFootIKLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FVector RightFootIKLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FRotator LeftFootIKRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FRotator RightFootIKRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	float PelvisOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	bool bEnableFootIK;

	// ── Blend Weights ─────────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
	float AdditiveExhaustionWeight;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
	float AdditiveFearWeight;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
	float AdditiveInjuryWeight;

	// ── Tuning ────────────────────────────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
	float SpeedSmoothingRate;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
	float LeanSmoothingRate;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
	float FootIKTraceLength;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
	float FootIKInterpSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
	float WalkSpeedThreshold;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
	float RunSpeedThreshold;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
	float SprintSpeedThreshold;

private:
	void UpdateLocomotionData(float DeltaSeconds);
	void UpdateSurvivalData();
	void UpdateCombatData();
	void UpdateFootIK(float DeltaSeconds);
	void UpdateAdditiveWeights(float DeltaSeconds);
	EAnim_LocomotionState DetermineLocomotionState() const;

	UPROPERTY()
	class ACharacter* OwnerCharacter;

	FVector PreviousVelocity;
	float LandingTimer;
	FVector LeftFootIKTarget;
	FVector RightFootIKTarget;
};
