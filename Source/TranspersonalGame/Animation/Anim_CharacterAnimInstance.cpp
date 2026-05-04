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
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Get character reference
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    UpdateMovementVariables();
    UpdateStateVariables();
}

void UAnim_CharacterAnimInstance::UpdateMovementVariables()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Get velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size();

    // Calculate direction relative to character rotation
    if (Speed > 0.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        
        float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        
        Direction = UKismetMathLibrary::DegAtan2(RightDot, ForwardDot);
    }
    else
    {
        Direction = 0.0f;
    }

    // Check if accelerating
    FVector Acceleration = MovementComponent->GetCurrentAcceleration();
    bIsAccelerating = Acceleration.SizeSquared() > 0.0f;
}

void UAnim_CharacterAnimInstance::UpdateStateVariables()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Check if in air
    bIsInAir = MovementComponent->IsFalling();

    // Check if crouching
    bIsCrouching = MovementComponent->IsCrouching();

    // Check if running (speed threshold)
    const float RunThreshold = 300.0f;
    bIsRunning = Speed > RunThreshold && !bIsInAir;
}