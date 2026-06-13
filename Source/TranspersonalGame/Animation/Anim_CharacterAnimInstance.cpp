#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    bIsCrouching = false;
    bIsRunning = false;
    Character = nullptr;
    CharacterMovement = nullptr;
    AccelerationThreshold = 10.0f;
    RunSpeedThreshold = 400.0f;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    Character = Cast<ACharacter>(TryGetPawnOwner());
    if (Character)
    {
        CharacterMovement = Character->GetCharacterMovement();
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!Character || !CharacterMovement)
    {
        return;
    }

    UpdateMovementState();
    UpdateAirborneState();
    UpdateCombatState();
}

void UAnim_CharacterAnimInstance::UpdateMovementState()
{
    if (!Character || !CharacterMovement)
    {
        return;
    }

    // Get current velocity
    Velocity = Character->GetVelocity();
    GroundSpeed = Velocity.Size2D();
    Speed = GroundSpeed;

    // Calculate direction relative to character rotation
    if (GroundSpeed > 0.0f)
    {
        FVector ForwardVector = Character->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        
        // Calculate angle between forward vector and velocity
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
        
        Direction = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
    }
    else
    {
        Direction = 0.0f;
    }

    // Check if accelerating
    FVector CurrentAcceleration = CharacterMovement->GetCurrentAcceleration();
    bIsAccelerating = CurrentAcceleration.Size() > AccelerationThreshold;

    // Check if running
    bIsRunning = GroundSpeed > RunSpeedThreshold;

    // Check if crouching
    bIsCrouching = CharacterMovement->IsCrouching();
}

void UAnim_CharacterAnimInstance::UpdateAirborneState()
{
    if (!CharacterMovement)
    {
        return;
    }

    bIsInAir = CharacterMovement->IsFalling();
}

void UAnim_CharacterAnimInstance::UpdateCombatState()
{
    // Combat state logic will be implemented by Combat AI Agent
    // Placeholder for combat stance, weapon state, etc.
}