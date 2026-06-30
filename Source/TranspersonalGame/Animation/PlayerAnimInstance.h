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
	Sneak       UMETA(DisplayName = "Sneak"),
	InAir       UMETA(DisplayName = "InAir"),
	Land        UMETA(DisplayName = "Land"),
	Climb       UMETA(DisplayName = "Climb")
};

UENUM(BlueprintType)
enum class EAnim_WeaponState : uint8
{
	Unarmed     UMETA(DisplayName = "Unarmed"),
	SpearReady  UMETA(DisplayName = "SpearReady"),
	SpearThrow  UMETA(DisplayName = "SpearThrow"),
	BowReady    UMETA(DisplayName = "BowReady"),
	BowDraw     UMETA(DisplayName = "BowDraw")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPlayerAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// === LOCOMOTION STATE ===
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	EAnim_LocomotionState LocomotionState;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	EAnim_WeaponState WeaponState;

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
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsCrouching;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsSprinting;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsClimbing;

	// === IK ===
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FVector LeftFootIKLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FVector RightFootIKLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	float LeftFootIKAlpha;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	float RightFootIKAlpha;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FRotator LeftFootIKRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FRotator RightFootIKRotation;

	// === AIM OFFSET ===
	UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
	float AimPitch;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
	float AimYaw;

	// === SURVIVAL STATES ===
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	float ExhaustionAlpha;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	float InjuryAlpha;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	bool bIsExhausted;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
	bool bIsInjured;

	// === THRESHOLDS ===
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
	float WalkSpeedThreshold;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
	float RunSpeedThreshold;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
	float SprintSpeedThreshold;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
	float SpeedSmoothingRate;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
	float IKTraceDistance;

private:
	void UpdateLocomotionState();
	void UpdateIK(float DeltaSeconds);
	void UpdateAimOffset();
	void UpdateSurvivalStates();
	void PerformFootIKTrace(FName SocketName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha);

	UPROPERTY()
	TObjectPtr<class ACharacter> OwnerCharacter;

	float LeanSmoothed;
	float PreviousSpeed;
};
