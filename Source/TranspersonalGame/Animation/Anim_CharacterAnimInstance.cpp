#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
	Speed = 0.0f;
	bIsInAir = false;
	bIsAccelerating = false;
	bIsRunning = false;
	Direction = 0.0f;
	
	WalkThreshold = 50.0f;
	RunThreshold = 300.0f;
	
	Character = nullptr;
	CharacterMovement = nullptr;
	
	IdleAnimation = nullptr;
	WalkAnimation = nullptr;
	RunAnimation = nullptr;
	JumpAnimation = nullptr;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	// Get reference to owning character
	Character = Cast<ACharacter>(TryGetPawnOwner());
	if (Character)
	{
		CharacterMovement = Character->GetCharacterMovement();
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
				FString::Printf(TEXT("Animation Instance initialized for: %s"), *Character->GetName()));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
				TEXT("Failed to get Character reference in AnimInstance"));
		}
	}
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	
	if (Character && CharacterMovement)
	{
		UpdateAnimationProperties(DeltaTime);
		SetAnimationState();
	}
}

void UAnim_CharacterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (!Character || !CharacterMovement)
		return;
	
	// Get velocity and calculate speed
	FVector Velocity = CharacterMovement->Velocity;
	Speed = Velocity.Size();
	
	// Check if character is in air
	bIsInAir = CharacterMovement->IsFalling();
	
	// Check if character is accelerating
	FVector Acceleration = CharacterMovement->GetCurrentAcceleration();
	bIsAccelerating = Acceleration.Size() > 0.0f;
	
	// Check if character is running (above run threshold)
	bIsRunning = Speed > RunThreshold;
	
	// Calculate movement direction relative to character rotation
	if (Speed > 0.0f)
	{
		FVector ForwardVector = Character->GetActorForwardVector();
		FVector VelocityNormalized = Velocity.GetSafeNormal();
		
		// Get angle between forward vector and velocity
		float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
		Direction = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
		
		// Check if moving backwards
		FVector RightVector = Character->GetActorRightVector();
		float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
		if (RightDot < 0.0f)
		{
			Direction *= -1.0f;
		}
	}
	else
	{
		Direction = 0.0f;
	}
}

void UAnim_CharacterAnimInstance::SetAnimationState()
{
	// Animation state logic is handled by the Animation Blueprint
	// This function can be used for additional state management if needed
	
	// Debug output for animation states
	if (GEngine && Character)
	{
		FString StateInfo = FString::Printf(TEXT("Speed: %.1f | InAir: %s | Accelerating: %s | Running: %s"), 
			Speed, 
			bIsInAir ? TEXT("Yes") : TEXT("No"),
			bIsAccelerating ? TEXT("Yes") : TEXT("No"),
			bIsRunning ? TEXT("Yes") : TEXT("No"));
			
		GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow, StateInfo);
	}
}

void UAnim_CharacterAnimInstance::PlayJumpMontage()
{
	if (JumpAnimation)
	{
		// Create a montage from the jump animation if needed
		// For now, just log that jump was triggered
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Jump animation triggered"));
		}
	}
}

void UAnim_CharacterAnimInstance::PlayLandMontage()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Land animation triggered"));
	}
}

void UAnim_CharacterAnimInstance::StopAllMontages()
{
	// Stop all currently playing montages
	Montage_Stop(0.2f);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("All animation montages stopped"));
	}
}