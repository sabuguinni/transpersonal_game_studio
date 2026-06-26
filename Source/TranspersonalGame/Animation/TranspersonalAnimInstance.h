#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Walk		UMETA(DisplayName = "Walk"),
	Run			UMETA(DisplayName = "Run"),
	Sprint		UMETA(DisplayName = "Sprint"),
	Jump		UMETA(DisplayName = "Jump"),
	Fall		UMETA(DisplayName = "Fall"),
	Land		UMETA(DisplayName = "Land"),
	Crouch		UMETA(DisplayName = "Crouch"),
	Climb		UMETA(DisplayName = "Climb"),
	Swim		UMETA(DisplayName = "Swim")
};

UENUM(BlueprintType)
enum class EAnim_StanceState : uint8
{
	Upright		UMETA(DisplayName = "Upright"),
	Crouched	UMETA(DisplayName = "Crouched"),
	Prone		UMETA(DisplayName = "Prone")
};

USTRUCT(BlueprintType)
struct FAnim_LocomotionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
	float Pitch = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
	bool bIsCrouching = false;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
	bool bIsAccelerating = false;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
	float LeanAmount = 0.f;
};

USTRUCT(BlueprintType)
struct FAnim_IKData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
	FVector LeftFootLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
	FVector RightFootLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
	FRotator LeftFootRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
	FRotator RightFootRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
	float LeftFootAlpha = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
	float RightFootAlpha = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
	float PelvisOffset = 0.f;
};

USTRUCT(BlueprintType)
struct FAnim_SurvivalData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
	float StaminaNormalized = 1.f;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
	float HealthNormalized = 1.f;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
	float FearNormalized = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
	bool bIsExhausted = false;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
	bool bIsInjured = false;
};

/**
 * Main Animation Instance for the TranspersonalCharacter.
 * Drives locomotion blend spaces, IK, and survival state animations.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UTranspersonalAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	// ─── Locomotion ───────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
		meta = (AllowPrivateAccess = "true"))
	FAnim_LocomotionData LocomotionData;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
		meta = (AllowPrivateAccess = "true"))
	EAnim_MovementState MovementState;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
		meta = (AllowPrivateAccess = "true"))
	EAnim_StanceState StanceState;

	// ─── IK ──────────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK",
		meta = (AllowPrivateAccess = "true"))
	FAnim_IKData IKData;

	// ─── Survival ─────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
		meta = (AllowPrivateAccess = "true"))
	FAnim_SurvivalData SurvivalData;

	// ─── Blend weights ────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend",
		meta = (AllowPrivateAccess = "true"))
	float GroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend",
		meta = (AllowPrivateAccess = "true"))
	bool bShouldMove = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend",
		meta = (AllowPrivateAccess = "true"))
	bool bIsFalling = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend",
		meta = (AllowPrivateAccess = "true"))
	float WalkRunAlpha = 0.f;

	// ─── Aim offset ───────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim",
		meta = (AllowPrivateAccess = "true"))
	float AimYaw = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim",
		meta = (AllowPrivateAccess = "true"))
	float AimPitch = 0.f;

	// ─── Functions ────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void UpdateLocomotionData();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void UpdateIKData();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void UpdateSurvivalData();

	UFUNCTION(BlueprintPure, Category = "Animation")
	EAnim_MovementState GetMovementState() const { return MovementState; }

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool IsExhausted() const { return SurvivalData.bIsExhausted; }

private:
	void UpdateMovementState();
	void PerformFootIKTrace(bool bIsLeftFoot, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha);

	UPROPERTY()
	TObjectPtr<class ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<class UCharacterMovementComponent> MovementComponent;

	// IK trace settings
	float IKTraceDistance = 50.f;
	float IKInterpSpeed = 15.f;
	float PelvisInterpSpeed = 10.f;

	// Lean smoothing
	float PreviousYaw = 0.f;
	float LeanInterpSpeed = 5.f;
};
