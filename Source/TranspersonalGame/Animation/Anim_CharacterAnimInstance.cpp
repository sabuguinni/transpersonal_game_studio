#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    OwningCharacter = nullptr;
    MovementComponent = nullptr;
    
    GroundSpeed = 0.0f;
    MovementDirection = 0.0f;
    bShouldMove = false;
    bIsFalling = false;
    bIsJumping = false;
    
    // Set default animation thresholds
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 375.0f;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                TEXT("Animation Instance initialized for TranspersonalCharacter"));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("Failed to get character reference in Animation Instance"));
        }
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update locomotion data
    UpdateLocomotionData();
    
    // Update animation variables for Blueprint
    GroundSpeed = LocomotionData.Speed;
    MovementDirection = LocomotionData.Direction;
    bShouldMove = GroundSpeed > 3.0f && !MovementComponent->GetCurrentAcceleration().Equals(FVector::ZeroVector, 0.1f);
    bIsFalling = LocomotionData.bIsInAir;
    bIsJumping = bIsFalling && MovementComponent->Velocity.Z > 0.0f;
}

void UAnim_CharacterAnimInstance::UpdateLocomotionData()
{
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get movement velocity
    FVector Velocity = MovementComponent->Velocity;
    LocomotionData.Speed = Velocity.Size2D();
    
    // Calculate movement direction relative to character rotation
    LocomotionData.Direction = CalculateDirection();
    
    // Update movement state flags
    LocomotionData.bIsInAir = MovementComponent->IsFalling();
    LocomotionData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Determine current movement state
    LocomotionData.MovementState = CalculateMovementState();
}

EAnim_MovementState UAnim_CharacterAnimInstance::CalculateMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check if character is in air
    if (LocomotionData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Check if crouching
    if (LocomotionData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Determine locomotion state based on speed
    if (LocomotionData.Speed < 3.0f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (LocomotionData.Speed < RunSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

float UAnim_CharacterAnimInstance::CalculateDirection() const
{
    if (!OwningCharacter || !MovementComponent)
    {
        return 0.0f;
    }
    
    // Get movement direction relative to character forward
    FVector Velocity = MovementComponent->Velocity;
    FVector ForwardVector = OwningCharacter->GetActorForwardVector();
    FVector RightVector = OwningCharacter->GetActorRightVector();
    
    // Normalize velocity for direction calculation
    FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
    
    // Calculate angle between forward vector and velocity
    float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
    float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
    
    // Convert to angle in degrees (-180 to 180)
    float Direction = UKismetMathLibrary::DegAtan2(RightDot, ForwardDot);
    
    return Direction;
}