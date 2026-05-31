#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterAnimInstance.generated.h"

/**
 * Animation Instance for TranspersonalCharacter
 * Handles state machine for idle, walk, run, jump animations
 * Based on character movement velocity and state
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UAnim_CharacterAnimInstance();

protected:
	// Animation state variables
	UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	float Direction;

	// Animation thresholds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds")
	float WalkThreshold = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds")
	float RunThreshold = 300.0f;

	// Animation assets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
	class UAnimSequence* IdleAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
	class UAnimSequence* WalkAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
	class UAnimSequence* RunAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
	class UAnimSequence* JumpAnimation;

	// Reference to owning character
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	class ACharacter* Character;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	class UCharacterMovementComponent* CharacterMovement;

public:
	// Animation Blueprint interface
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	// Animation state functions
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void UpdateAnimationProperties(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetAnimationState();

	// Animation montage functions
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayJumpMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayLandMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void StopAllMontages();

	// Getters for animation properties
	UFUNCTION(BlueprintPure, Category = "Animation")
	float GetSpeed() const { return Speed; }

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool GetIsInAir() const { return bIsInAir; }

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool GetIsAccelerating() const { return bIsAccelerating; }

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool GetIsRunning() const { return bIsRunning; }

	UFUNCTION(BlueprintPure, Category = "Animation")
	float GetDirection() const { return Direction; }
};