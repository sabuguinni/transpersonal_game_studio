#include "Anim_MovementAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MovementAnimInstance::UAnim_MovementAnimInstance()
{
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    bIsRunning = false;
    bIsCrouching = false;
    
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 375.0f;
    
    OwningCharacter = nullptr;
    CharacterMovement = nullptr;
}

void UAnim_MovementAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwningCharacter)
    {
        CharacterMovement = OwningCharacter->GetCharacterMovement();
    }
}

void UAnim_MovementAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    UpdateMovementVariables(DeltaTime);
    
    if (bIsInAir)
    {
        UpdateInAirState();
    }
    else
    {
        UpdateGroundedState();
    }
}

void UAnim_MovementAnimInstance::UpdateMovementVariables(float DeltaTime)
{
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = CharacterMovement->Velocity;
    Speed = Velocity.Size2D();
    
    // Calculate direction relative to character rotation
    if (Speed > 0.0f)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector RightVector = OwningCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
        
        float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        
        Direction = UKismetMathLibrary::DegAtan2(RightDot, ForwardDot);
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Update movement states
    bIsInAir = CharacterMovement->IsFalling();
    bIsAccelerating = CharacterMovement->GetCurrentAcceleration().SizeSquared() > 0.0f;
    bIsRunning = Speed > RunSpeedThreshold;
    bIsCrouching = CharacterMovement->IsCrouching();
}

void UAnim_MovementAnimInstance::UpdateGroundedState()
{
    // Additional grounded state logic can be added here
    // For example, checking for different movement modes
    // or updating blend space parameters
}

void UAnim_MovementAnimInstance::UpdateInAirState()
{
    // Additional in-air state logic can be added here
    // For example, checking fall velocity or jump state
}