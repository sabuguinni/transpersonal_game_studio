#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogAnimInstance);

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
	// Initialize default values
	Speed = 0.0f;
	Direction = 0.0f;
	bIsInAir = false;
	bIsAccelerating = false;
	bIsCrouching = false;
	bIsRunning = false;
	
	HealthPercentage = 1.0f;
	StaminaPercentage = 1.0f;
	FearLevel = 0.0f;
	bIsInjured = false;
	bIsExhausted = false;
	
	InjurySpeedModifier = 1.0f;
	FatigueSpeedModifier = 1.0f;
	FearSpeedModifier = 1.0f;
	
	Character = nullptr;
	CharacterMovement = nullptr;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	// Get character reference
	Character = Cast<ACharacter>(GetOwningActor());
	if (Character)
	{
		CharacterMovement = Character->GetCharacterMovement();
		UE_LOG(LogAnimInstance, Log, TEXT("Animation Instance initialized for character: %s"), *Character->GetName());
	}
	else
	{
		UE_LOG(LogAnimInstance, Warning, TEXT("Failed to get character reference in animation instance"));
	}
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
	Super::NativeUpdateAnimation(DeltaTimeX);
	
	if (!Character || !CharacterMovement)
	{
		return;
	}
	
	// Update all animation values
	UpdateMovementValues();
	UpdateSurvivalStates();
	UpdateAnimationModifiers();
}

void UAnim_CharacterAnimInstance::UpdateMovementValues()
{
	if (!Character || !CharacterMovement)
	{
		return;
	}
	
	// Get velocity and calculate speed
	FVector Velocity = CharacterMovement->Velocity;
	Speed = Velocity.Size();
	
	// Calculate direction relative to character rotation
	Direction = CalculateDirection(Velocity, Character->GetActorRotation());
	
	// Update movement states
	bIsInAir = CharacterMovement->IsFalling();
	bIsAccelerating = CharacterMovement->GetCurrentAcceleration().Size() > 0.0f;
	bIsCrouching = CharacterMovement->IsCrouching();
	
	// Determine if running based on speed threshold
	bIsRunning = Speed > RunSpeedThreshold;
	
	UE_LOG(LogAnimInstance, VeryVerbose, TEXT("Movement Update - Speed: %.2f, Direction: %.2f, InAir: %s, Running: %s"), 
		Speed, Direction, bIsInAir ? TEXT("true") : TEXT("false"), bIsRunning ? TEXT("true") : TEXT("false"));
}

void UAnim_CharacterAnimInstance::UpdateSurvivalStates()
{
	// This would integrate with the actual survival system
	// For now, using placeholder logic
	
	// Check for injury state
	bIsInjured = HealthPercentage < InjuryThreshold;
	
	// Check for exhaustion state
	bIsExhausted = StaminaPercentage < ExhaustionThreshold;
	
	// Fear level affects animation intensity
	if (FearLevel > 0.7f)
	{
		// High fear - tense, quick movements
		FearSpeedModifier = 1.3f;
	}
	else if (FearLevel > 0.4f)
	{
		// Moderate fear - slightly faster
		FearSpeedModifier = 1.1f;
	}
	else
	{
		// Normal state
		FearSpeedModifier = 1.0f;
	}
}

void UAnim_CharacterAnimInstance::UpdateAnimationModifiers()
{
	// Injury affects movement speed and style
	if (bIsInjured)
	{
		InjurySpeedModifier = FMath::Lerp(0.5f, 1.0f, HealthPercentage);
	}
	else
	{
		InjurySpeedModifier = 1.0f;
	}
	
	// Fatigue affects animation speed
	if (bIsExhausted)
	{
		FatigueSpeedModifier = FMath::Lerp(0.7f, 1.0f, StaminaPercentage);
	}
	else
	{
		FatigueSpeedModifier = 1.0f;
	}
}

float UAnim_CharacterAnimInstance::CalculateDirection(const FVector& Velocity, const FRotator& BaseRotation)
{
	if (Velocity.Size() < 0.1f)
	{
		return 0.0f;
	}
	
	// Calculate the direction relative to the character's forward vector
	FVector ForwardVector = BaseRotation.Vector();
	FVector RightVector = FVector::CrossProduct(ForwardVector, FVector::UpVector);
	
	FVector NormalizedVelocity = Velocity.GetSafeNormal();
	
	float ForwardAmount = FVector::DotProduct(NormalizedVelocity, ForwardVector);
	float RightAmount = FVector::DotProduct(NormalizedVelocity, RightVector);
	
	return FMath::Atan2(RightAmount, ForwardAmount) * (180.0f / PI);
}

bool UAnim_CharacterAnimInstance::ShouldPlayInjuredAnimation() const
{
	return bIsInjured && HealthPercentage < 0.7f;
}

bool UAnim_CharacterAnimInstance::ShouldPlayExhaustedAnimation() const
{
	return bIsExhausted && StaminaPercentage < 0.3f;
}

float UAnim_CharacterAnimInstance::GetEffectiveAnimationSpeed() const
{
	return InjurySpeedModifier * FatigueSpeedModifier * FearSpeedModifier;
}