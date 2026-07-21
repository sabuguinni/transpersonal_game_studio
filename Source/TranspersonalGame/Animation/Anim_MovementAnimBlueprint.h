#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Engine/Engine.h"
#include "Anim_MovementAnimBlueprint.generated.h"

/**
 * Animation Blueprint for TranspersonalCharacter movement states
 * Handles idle, walk, run, jump, and landing animations
 * Uses velocity-based state machine for smooth transitions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MovementAnimBlueprint : public UAnimInstance
{
	GENERATED_BODY()

public:
	UAnim_MovementAnimBlueprint();

	// Animation state variables
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching;

	// Animation thresholds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds")
	float WalkThreshold = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds")
	float RunThreshold = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds")
	float JumpThreshold = 50.0f;

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	// Cached references
	UPROPERTY()
	class APawn* OwningPawn;

	UPROPERTY()
	class UCharacterMovementComponent* CharacterMovementComponent;

	// Helper functions
	void UpdateMovementValues();
	void UpdateAirState();
	void UpdateAcceleration();
	void UpdateRunningState();
};