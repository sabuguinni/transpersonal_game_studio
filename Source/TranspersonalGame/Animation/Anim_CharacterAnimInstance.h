#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "../SharedTypes.h"
#include "Anim_CharacterAnimInstance.generated.h"

TRANSPERSONALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogAnimInstance, Log, All);

/**
 * Animation Instance for TranspersonalCharacter
 * Handles state machine logic for idle/walk/run/jump animations
 * Integrates with survival stats and environmental adaptation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UAnim_CharacterAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTimeX) override;

	// Animation state properties
	UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching;

	UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
	bool bIsRunning;

	// Survival state integration
	UPROPERTY(BlueprintReadOnly, Category = "Survival State", meta = (AllowPrivateAccess = "true"))
	float HealthPercentage;

	UPROPERTY(BlueprintReadOnly, Category = "Survival State", meta = (AllowPrivateAccess = "true"))
	float StaminaPercentage;

	UPROPERTY(BlueprintReadOnly, Category = "Survival State", meta = (AllowPrivateAccess = "true"))
	float FearLevel;

	UPROPERTY(BlueprintReadOnly, Category = "Survival State", meta = (AllowPrivateAccess = "true"))
	bool bIsInjured;

	UPROPERTY(BlueprintReadOnly, Category = "Survival State", meta = (AllowPrivateAccess = "true"))
	bool bIsExhausted;

	// Animation speed modifiers
	UPROPERTY(BlueprintReadOnly, Category = "Animation Modifiers", meta = (AllowPrivateAccess = "true"))
	float InjurySpeedModifier;

	UPROPERTY(BlueprintReadOnly, Category = "Animation Modifiers", meta = (AllowPrivateAccess = "true"))
	float FatigueSpeedModifier;

	UPROPERTY(BlueprintReadOnly, Category = "Animation Modifiers", meta = (AllowPrivateAccess = "true"))
	float FearSpeedModifier;

	// Animation thresholds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
	float WalkSpeedThreshold = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
	float RunSpeedThreshold = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
	float InjuryThreshold = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
	float ExhaustionThreshold = 0.2f;

protected:
	// Reference to the character
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	class ACharacter* Character;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	class UCharacterMovementComponent* CharacterMovement;

	// Update functions
	void UpdateMovementValues();
	void UpdateSurvivalStates();
	void UpdateAnimationModifiers();

	// Helper functions
	UFUNCTION(BlueprintCallable, Category = "Animation Helpers")
	float CalculateDirection(const FVector& Velocity, const FRotator& BaseRotation);

	UFUNCTION(BlueprintCallable, Category = "Animation Helpers")
	bool ShouldPlayInjuredAnimation() const;

	UFUNCTION(BlueprintCallable, Category = "Animation Helpers")
	bool ShouldPlayExhaustedAnimation() const;

	UFUNCTION(BlueprintCallable, Category = "Animation Helpers")
	float GetEffectiveAnimationSpeed() const;
};