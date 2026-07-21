#include "Anim_CharacterController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterController::UAnim_CharacterController()
{
	// Initialize animation state variables
	Speed = 0.0f;
	bIsInAir = false;
	bIsAccelerating = false;
	Direction = 0.0f;
	bIsWalking = false;
	bIsRunning = false;
	bIsIdle = true;
	
	// Set default thresholds
	WalkSpeedThreshold = 150.0f;
	RunSpeedThreshold = 300.0f;
	
	Character = nullptr;
	CharacterMovement = nullptr;
}

void UAnim_CharacterController::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	// Get character reference
	Character = Cast<ACharacter>(GetOwningActor());
	if (Character)
	{
		CharacterMovement = Character->GetCharacterMovement();
		UE_LOG(LogTemp, Log, TEXT("Animation Controller initialized for character: %s"), *Character->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Animation Controller: Failed to get character reference"));
	}
}

void UAnim_CharacterController::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	
	if (Character && CharacterMovement)
	{
		UpdateMovementValues();
		UpdateAnimationState();
	}
}

void UAnim_CharacterController::UpdateMovementValues()
{
	if (!Character || !CharacterMovement)
	{
		return;
	}
	
	// Get current velocity and calculate speed
	FVector Velocity = Character->GetVelocity();
	Speed = Velocity.Size2D(); // Use 2D speed (ignore Z component for ground movement)
	
	// Check if character is in air
	bIsInAir = CharacterMovement->IsFalling();
	
	// Check if character is accelerating
	FVector Acceleration = CharacterMovement->GetCurrentAcceleration();
	bIsAccelerating = Acceleration.SizeSquared() > 0.0f;
	
	// Calculate movement direction relative to character rotation
	if (Speed > 0.0f)
	{
		FVector ForwardVector = Character->GetActorForwardVector();
		FVector VelocityNormalized = Velocity.GetSafeNormal2D();
		
		// Calculate dot product to get forward/backward movement
		float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
		
		// Calculate cross product to get left/right movement
		FVector CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized);
		
		// Direction: positive = forward, negative = backward
		Direction = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct.Z, DotProduct));
	}
	else
	{
		Direction = 0.0f;
	}
}

void UAnim_CharacterController::UpdateAnimationState()
{
	// Reset all states
	bIsIdle = false;
	bIsWalking = false;
	bIsRunning = false;
	
	// Don't update ground movement states if in air
	if (bIsInAir)
	{
		return;
	}
	
	// Determine animation state based on speed
	if (Speed < 10.0f) // Small threshold to account for floating point precision
	{
		bIsIdle = true;
	}
	else if (Speed < RunSpeedThreshold)
	{
		bIsWalking = true;
	}
	else
	{
		bIsRunning = true;
	}
}

bool UAnim_CharacterController::ShouldPlayIdleAnimation() const
{
	return bIsIdle && !bIsInAir;
}

bool UAnim_CharacterController::ShouldPlayWalkAnimation() const
{
	return bIsWalking && !bIsInAir;
}

bool UAnim_CharacterController::ShouldPlayRunAnimation() const
{
	return bIsRunning && !bIsInAir;
}

bool UAnim_CharacterController::ShouldPlayJumpAnimation() const
{
	return bIsInAir;
}