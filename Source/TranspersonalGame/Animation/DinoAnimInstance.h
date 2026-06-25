#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoAnimInstance.generated.h"

/**
 * FAnim_DinoLocomotionState — locomotion state for dinosaur animation blending
 */
USTRUCT(BlueprintType)
struct FAnim_DinoLocomotionState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	bool bIsAlerted = false;

	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	float TurnRate = 0.0f;
};

/**
 * EAnim_DinoGait — gait states for dinosaur locomotion
 */
UENUM(BlueprintType)
enum class EAnim_DinoGait : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Walk        UMETA(DisplayName = "Walk"),
	Trot        UMETA(DisplayName = "Trot"),
	Run         UMETA(DisplayName = "Run"),
	Attack      UMETA(DisplayName = "Attack"),
	Roar        UMETA(DisplayName = "Roar"),
	Death       UMETA(DisplayName = "Death"),
};

/**
 * UDinoAnimInstance — Animation Instance for dinosaur characters.
 * Drives locomotion blending, attack triggers, and IK foot placement.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UDinoAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// --- Locomotion State ---

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
	float GroundSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
	EAnim_DinoGait CurrentGait = EAnim_DinoGait::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
	float MovementDirection = 0.0f;

	// --- Combat State ---

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsAlerted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsRoaring = false;

	// --- IK Foot Placement ---

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK", meta = (AllowPrivateAccess = "true"))
	FVector LeftFootIKLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK", meta = (AllowPrivateAccess = "true"))
	FVector RightFootIKLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK", meta = (AllowPrivateAccess = "true"))
	float BodyLeanAngle = 0.0f;

	// --- Speed Thresholds ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
	float WalkSpeedThreshold = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
	float TrotSpeedThreshold = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
	float RunSpeedThreshold = 700.0f;

	// --- Blueprint-callable triggers ---

	UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
	void TriggerAttack();

	UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
	void TriggerRoar();

	UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
	void ClearCombatState();

	UFUNCTION(BlueprintCallable, Category = "Animation|Locomotion")
	FAnim_DinoLocomotionState GetLocomotionState() const;

protected:
	void UpdateLocomotionGait();
	void UpdateFootIK(float DeltaSeconds);

private:
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn = nullptr;

	float FootIKInterpSpeed = 12.0f;
};
