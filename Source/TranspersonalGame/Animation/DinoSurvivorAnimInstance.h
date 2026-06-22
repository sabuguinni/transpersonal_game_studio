// DinoSurvivorAnimInstance.h
// Animation Agent #10 — Transpersonal Game Studio
// AnimInstance for the prehistoric survivor character: locomotion, foot IK, aim offset, survival blending

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

// ─── ENUMS (global scope, Anim_ prefix) ──────────────────────────────────────

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Walk        UMETA(DisplayName = "Walk"),
	Run         UMETA(DisplayName = "Run"),
	Sprint      UMETA(DisplayName = "Sprint"),
	Crouch      UMETA(DisplayName = "Crouch"),
	CrouchWalk  UMETA(DisplayName = "CrouchWalk"),
	Jump        UMETA(DisplayName = "Jump"),
	Fall        UMETA(DisplayName = "Fall"),
	Land        UMETA(DisplayName = "Land")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalPosture : uint8
{
	Upright    UMETA(DisplayName = "Upright"),
	Alert      UMETA(DisplayName = "Alert"),
	Terrified  UMETA(DisplayName = "Terrified"),
	Exhausted  UMETA(DisplayName = "Exhausted"),
	Wounded    UMETA(DisplayName = "Wounded")
};

// ─── STRUCTS (global scope, Anim_ prefix) ────────────────────────────────────

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
	FVector LeftFootLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
	FVector RightFootLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
	FRotator LeftFootRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
	FRotator RightFootRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
	float PelvisOffset = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
	bool bLeftFootGrounded = true;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
	bool bRightFootGrounded = true;
};

USTRUCT(BlueprintType)
struct FAnim_AimOffsetData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
	float AimPitch = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
	float AimYaw = 0.0f;
};

// ─── MAIN CLASS ──────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UDinoSurvivorAnimInstance();

	// UAnimInstance overrides
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// ─── LOCOMOTION STATE ─────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	EAnim_LocomotionState LocomotionState;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	EAnim_SurvivalPosture SurvivalPosture;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float LeanAngle;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsCrouching;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsSprinting;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsMoving;

	// ─── SURVIVAL STATS ───────────────────────────────────────────────────
	UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
	float HealthNormalized;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
	float StaminaNormalized;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
	float FearNormalized;

	// ─── THRESHOLDS ───────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
	float WalkSpeedThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
	float RunSpeedThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
	float SprintSpeedThreshold;

	// ─── FOOT IK ──────────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
	FAnim_FootIKData FootIKData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
	bool bFootIKEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
	float FootIKTraceDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
	float FootIKInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
	float PelvisAdjustmentSpeed;

	// ─── AIM OFFSET ───────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
	FAnim_AimOffsetData AimOffsetData;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
	float AimPitch;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
	float AimYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|AimOffset")
	float AimOffsetInterpSpeed;

	// ─── BLEND WEIGHTS ────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Blending")
	float UpperBodyBlendWeight;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Blending")
	float SurvivalBlendWeight;

	// ─── PUBLIC API ───────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
	void SetSurvivalStats(float Health, float Stamina, float Fear);

	UFUNCTION(BlueprintCallable, Category = "Animation|Locomotion")
	void SetSprintingState(bool bSprinting);

	UFUNCTION(BlueprintPure, Category = "Animation|Locomotion")
	EAnim_LocomotionState GetLocomotionState() const;

	UFUNCTION(BlueprintPure, Category = "Animation|FootIK")
	FAnim_FootIKData GetFootIKData() const;

private:
	// Cached references
	UPROPERTY()
	ACharacter* OwnerCharacter;

	UPROPERTY()
	UCharacterMovementComponent* MovementComponent;

	// Internal update methods
	void UpdateLocomotionData(float DeltaSeconds);
	void UpdateLocomotionState();
	void UpdateSurvivalPosture();
	void UpdateFootIK(float DeltaSeconds);
	void TraceFootIK(const FName& BoneName, FVector& OutLocation, FRotator& OutRotation, bool& bGrounded);
	void UpdateAimOffset(float DeltaSeconds);
	void UpdateSurvivalBlending(float DeltaSeconds);
};
