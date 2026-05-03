#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    OwningCharacter = nullptr;
    CharacterMovement = nullptr;
    IdleMontage = nullptr;
    LocomotionBlendSpace = nullptr;
    JumpMontage = nullptr;
    LandMontage = nullptr;
    
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 300.0f;
    SprintSpeedThreshold = 500.0f;
    DirectionSmoothingSpeed = 10.0f;
}

void UAnim_MotionMatchingSystem::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        CharacterMovement = OwningCharacter->GetCharacterMovement();
    }
    
    // Initialize motion data
    MotionData = FAnim_MotionData();
}

void UAnim_MotionMatchingSystem::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    UpdateMotionData();
}

void UAnim_MotionMatchingSystem::UpdateMotionData()
{
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get current velocity and speed
    const FVector Velocity = CharacterMovement->Velocity;
    MotionData.Speed = Velocity.Size();
    
    // Calculate if accelerating
    MotionData.bIsAccelerating = CharacterMovement->GetCurrentAcceleration().SizeSquared() > 0.0f;
    
    // Check if in air
    MotionData.bIsInAir = CharacterMovement->IsFalling();
    
    // Check if crouching
    MotionData.bIsCrouching = CharacterMovement->IsCrouching();
    
    // Calculate movement direction
    CalculateMovementDirection();
    
    // Calculate locomotion state
    CalculateLocomotionState();
}

void UAnim_MotionMatchingSystem::CalculateLocomotionState()
{
    if (!CharacterMovement)
    {
        return;
    }
    
    // Check if in air first
    if (MotionData.bIsInAir)
    {
        if (CharacterMovement->Velocity.Z > 0.0f)
        {
            MotionData.LocomotionState = EAnim_LocomotionState::Jumping;
        }
        else
        {
            MotionData.LocomotionState = EAnim_LocomotionState::Falling;
        }
        return;
    }
    
    // Check if crouching
    if (MotionData.bIsCrouching)
    {
        MotionData.LocomotionState = EAnim_LocomotionState::Crouching;
        return;
    }
    
    // Determine locomotion state based on speed
    if (MotionData.Speed < 10.0f)
    {
        MotionData.LocomotionState = EAnim_LocomotionState::Idle;
    }
    else if (MotionData.Speed < WalkSpeedThreshold)
    {
        MotionData.LocomotionState = EAnim_LocomotionState::Walking;
    }
    else if (MotionData.Speed < RunSpeedThreshold)
    {
        MotionData.LocomotionState = EAnim_LocomotionState::Running;
    }
    else
    {
        MotionData.LocomotionState = EAnim_LocomotionState::Sprinting;
    }
}

void UAnim_MotionMatchingSystem::CalculateMovementDirection()
{
    if (!OwningCharacter)
    {
        return;
    }
    
    const FVector Velocity = OwningCharacter->GetVelocity();
    const FRotator Rotation = OwningCharacter->GetActorRotation();
    
    MotionData.Direction = CalculateDirection(Velocity, Rotation);
    
    // Determine movement direction enum based on angle
    float AbsDirection = FMath::Abs(MotionData.Direction);
    
    if (AbsDirection < 22.5f)
    {
        MotionData.MovementDirection = EAnim_MovementDirection::Forward;
    }
    else if (AbsDirection > 157.5f)
    {
        MotionData.MovementDirection = EAnim_MovementDirection::Backward;
    }
    else if (MotionData.Direction > 0.0f)
    {
        if (MotionData.Direction < 67.5f)
        {
            MotionData.MovementDirection = EAnim_MovementDirection::ForwardRight;
        }
        else if (MotionData.Direction < 112.5f)
        {
            MotionData.MovementDirection = EAnim_MovementDirection::Right;
        }
        else
        {
            MotionData.MovementDirection = EAnim_MovementDirection::BackwardRight;
        }
    }
    else
    {
        if (MotionData.Direction > -67.5f)
        {
            MotionData.MovementDirection = EAnim_MovementDirection::ForwardLeft;
        }
        else if (MotionData.Direction > -112.5f)
        {
            MotionData.MovementDirection = EAnim_MovementDirection::Left;
        }
        else
        {
            MotionData.MovementDirection = EAnim_MovementDirection::BackwardLeft;
        }
    }
}

float UAnim_MotionMatchingSystem::CalculateDirection(const FVector& Velocity, const FRotator& Rotation) const
{
    if (Velocity.SizeSquared() < 1.0f)
    {
        return 0.0f;
    }
    
    const FVector ForwardVector = UKismetMathLibrary::GetForwardVector(Rotation);
    const FVector RightVector = UKismetMathLibrary::GetRightVector(Rotation);
    
    const FVector NormalizedVelocity = Velocity.GetSafeNormal();
    
    const float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
    const float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
    
    return FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
}

void UAnim_MotionMatchingSystem::SetLocomotionState(EAnim_LocomotionState NewState)
{
    MotionData.LocomotionState = NewState;
}

EAnim_LocomotionState UAnim_MotionMatchingSystem::GetCurrentLocomotionState() const
{
    return MotionData.LocomotionState;
}

float UAnim_MotionMatchingSystem::GetMovementSpeed() const
{
    return MotionData.Speed;
}

bool UAnim_MotionMatchingSystem::IsMoving() const
{
    return MotionData.Speed > 10.0f;
}