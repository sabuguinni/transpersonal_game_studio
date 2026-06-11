#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_CharacterController.generated.h"

/**
 * Animation controller for TranspersonalCharacter
 * Handles basic locomotion states: idle, walk, run, jump, fall, land
 * Provides smooth transitions between animation states based on character movement
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterController : public UAnimInstance
{
	GENERATED_BODY()

public:
	UAnim_CharacterController();

protected:
	// Animation state variables
	UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
	bool bIsWalking;

	UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
	bool bIsIdle;

	// Movement thresholds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds")
	float WalkSpeedThreshold = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds")
	float RunSpeedThreshold = 300.0f;

	// Character reference
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	class ACharacter* Character;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	class UCharacterMovementComponent* CharacterMovement;

public:
	// AnimInstance overrides
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	// Animation state functions
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void UpdateAnimationState();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void UpdateMovementValues();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool ShouldPlayIdleAnimation() const;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool ShouldPlayWalkAnimation() const;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool ShouldPlayRunAnimation() const;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool ShouldPlayJumpAnimation() const;

	// Getters for animation blueprints
	UFUNCTION(BlueprintPure, Category = "Animation State")
	float GetSpeed() const { return Speed; }

	UFUNCTION(BlueprintPure, Category = "Animation State")
	bool GetIsInAir() const { return bIsInAir; }

	UFUNCTION(BlueprintPure, Category = "Animation State")
	bool GetIsAccelerating() const { return bIsAccelerating; }

	UFUNCTION(BlueprintPure, Category = "Animation State")
	float GetDirection() const { return Direction; }

	UFUNCTION(BlueprintPure, Category = "Animation State")
	bool GetIsWalking() const { return bIsWalking; }

	UFUNCTION(BlueprintPure, Category = "Animation State")
	bool GetIsRunning() const { return bIsRunning; }

	UFUNCTION(BlueprintPure, Category = "Animation State")
	bool GetIsIdle() const { return bIsIdle; }
};