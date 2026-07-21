#include "Anim_PrimitiveAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_PrimitiveAnimInstance::UAnim_PrimitiveAnimInstance()
{
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    bIsCrouching = false;
    MovementState = EAnim_MovementState::Idle;
    OwningCharacter = nullptr;
    CharacterMovement = nullptr;
    
    // Set default thresholds
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 400.0f;
}

void UAnim_PrimitiveAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get the owning character
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        CharacterMovement = OwningCharacter->GetCharacterMovement();
    }
}

void UAnim_PrimitiveAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    UpdateMovementValues();
    UpdateMovementState();
}

void UAnim_PrimitiveAnimInstance::UpdateMovementValues()
{
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = CharacterMovement->Velocity;
    Speed = Velocity.Size();
    
    // Calculate direction relative to character forward
    if (Speed > 0.0f)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector RightVector = OwningCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        
        float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        
        Direction = UKismetMathLibrary::DegAtan2(RightDot, ForwardDot);
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Check if character is in air
    bIsInAir = CharacterMovement->IsFalling();
    
    // Check if character is accelerating
    bIsAccelerating = CharacterMovement->GetCurrentAcceleration().Size() > 0.0f;
    
    // Check if character is crouching
    bIsCrouching = CharacterMovement->IsCrouching();
}

void UAnim_PrimitiveAnimInstance::UpdateMovementState()
{
    if (bIsInAir)
    {
        if (CharacterMovement->Velocity.Z > 0.0f)
        {
            MovementState = EAnim_MovementState::Jumping;
        }
        else
        {
            MovementState = EAnim_MovementState::Falling;
        }
    }
    else if (bIsCrouching)
    {
        MovementState = EAnim_MovementState::Crouching;
    }
    else if (Speed < WalkSpeedThreshold)
    {
        MovementState = EAnim_MovementState::Idle;
    }
    else if (Speed < RunSpeedThreshold)
    {
        MovementState = EAnim_MovementState::Walking;
    }
    else
    {
        MovementState = EAnim_MovementState::Running;
    }
}