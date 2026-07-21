#include "Anim_MovementAnimBlueprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MovementAnimBlueprint::UAnim_MovementAnimBlueprint()
{
	// Initialize default values
	Speed = 0.0f;
	Direction = 0.0f;
	bIsInAir = false;
	bIsAccelerating = false;
	bIsRunning = false;
	bIsCrouching = false;
	
	WalkThreshold = 100.0f;
	RunThreshold = 300.0f;
	JumpThreshold = 50.0f;
	
	OwningPawn = nullptr;
	CharacterMovementComponent = nullptr;
}

void UAnim_MovementAnimBlueprint::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	// Cache the owning pawn and movement component
	OwningPawn = TryGetPawnOwner();
	if (OwningPawn)
	{
		CharacterMovementComponent = OwningPawn->FindComponentByClass<UCharacterMovementComponent>();
	}
}

void UAnim_MovementAnimBlueprint::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	
	if (!OwningPawn || !CharacterMovementComponent)
	{
		return;
	}
	
	UpdateMovementValues();
	UpdateAirState();
	UpdateAcceleration();
	UpdateRunningState();
}

void UAnim_MovementAnimBlueprint::UpdateMovementValues()
{
	if (!OwningPawn || !CharacterMovementComponent)
	{
		return;
	}
	
	// Get velocity and calculate speed
	FVector Velocity = OwningPawn->GetVelocity();
	Speed = Velocity.Size2D();
	
	// Calculate direction relative to actor rotation
	if (Speed > 0.0f)
	{
		FVector ForwardVector = OwningPawn->GetActorForwardVector();
		FVector RightVector = OwningPawn->GetActorRightVector();
		
		FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
		
		float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
		float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
		
		Direction = UKismetMathLibrary::DegAtan2(RightDot, ForwardDot);
	}
	else
	{
		Direction = 0.0f;
	}
}

void UAnim_MovementAnimBlueprint::UpdateAirState()
{
	if (!CharacterMovementComponent)
	{
		return;
	}
	
	bIsInAir = CharacterMovementComponent->IsFalling();
}

void UAnim_MovementAnimBlueprint::UpdateAcceleration()
{
	if (!OwningPawn || !CharacterMovementComponent)
	{
		return;
	}
	
	FVector Acceleration = CharacterMovementComponent->GetCurrentAcceleration();
	bIsAccelerating = Acceleration.SizeSquared() > 0.0f;
}

void UAnim_MovementAnimBlueprint::UpdateRunningState()
{
	if (!CharacterMovementComponent)
	{
		return;
	}
	
	// Consider running if speed exceeds run threshold and character is accelerating
	bIsRunning = (Speed > RunThreshold) && bIsAccelerating && !bIsInAir;
	
	// Update crouching state
	bIsCrouching = CharacterMovementComponent->IsCrouching();
}