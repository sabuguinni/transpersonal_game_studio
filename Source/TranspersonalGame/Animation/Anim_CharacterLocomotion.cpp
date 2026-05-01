#include "Anim_CharacterLocomotion.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterLocomotion::UAnim_CharacterLocomotion()
{
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    bIsCrouching = false;
    OwningCharacter = nullptr;
    MovementComponent = nullptr;
    LastVelocity = FVector::ZeroVector;
}

void UAnim_CharacterLocomotion::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
        if (MovementComponent)
        {
            UE_LOG(LogTemp, Log, TEXT("CharacterLocomotion AnimInstance initialized for %s"), *OwningCharacter->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("CharacterLocomotion: No CharacterMovementComponent found"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterLocomotion: OwningActor is not a Character"));
    }
}

void UAnim_CharacterLocomotion::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    UpdateMovementProperties();
    UpdateAirState();
    UpdateAcceleration();
    
    // Store velocity for next frame acceleration calculation
    LastVelocity = MovementComponent->Velocity;
}

void UAnim_CharacterLocomotion::UpdateMovementProperties()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Calculate speed (horizontal velocity magnitude)
    FVector HorizontalVelocity = MovementComponent->Velocity;
    HorizontalVelocity.Z = 0.0f;
    Speed = HorizontalVelocity.Size();
    
    // Calculate direction relative to character forward
    if (Speed > 1.0f && OwningCharacter)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector VelocityDirection = HorizontalVelocity.GetSafeNormal();
        
        // Get angle between forward and velocity direction
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        FVector CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection);
        
        Direction = UKismetMathLibrary::DegAtan2(CrossProduct.Z, DotProduct);
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Update crouching state
    bIsCrouching = MovementComponent->IsCrouching();
}

void UAnim_CharacterLocomotion::UpdateAirState()
{
    if (!MovementComponent)
    {
        return;
    }
    
    bIsInAir = MovementComponent->IsFalling();
}

void UAnim_CharacterLocomotion::UpdateAcceleration()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Calculate acceleration based on velocity change
    FVector CurrentVelocity = MovementComponent->Velocity;
    FVector VelocityDelta = CurrentVelocity - LastVelocity;
    float AccelerationMagnitude = VelocityDelta.Size();
    
    bIsAccelerating = AccelerationMagnitude > AccelerationThreshold;
}