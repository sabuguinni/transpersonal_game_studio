#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 300.0f;
    SprintSpeedThreshold = 500.0f;
    DirectionSmoothingSpeed = 10.0f;
    SpeedSmoothingSpeed = 8.0f;
    
    SmoothedSpeed = 0.0f;
    SmoothedDirection = 0.0f;
    PreviousVelocity = FVector::ZeroVector;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
    
    // Initialize motion data
    CurrentMotionData = FAnim_MotionData();
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerCharacter && MovementComponent)
    {
        UpdateMotionData();
    }
}

void UAnim_MotionMatchingSystem::UpdateMotionData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get current velocity and speed
    FVector CurrentVelocity = MovementComponent->Velocity;
    float CurrentSpeed = CurrentVelocity.Size();
    
    // Calculate acceleration
    FVector CurrentAcceleration = (CurrentVelocity - PreviousVelocity) / GetWorld()->GetDeltaSeconds();
    PreviousVelocity = CurrentVelocity;
    
    // Smooth speed and direction
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, CurrentSpeed, DeltaTime, SpeedSmoothingSpeed);
    
    // Calculate direction relative to character forward
    float CurrentDirection = 0.0f;
    if (CurrentSpeed > 10.0f) // Only calculate direction if moving
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector NormalizedVelocity = CurrentVelocity.GetSafeNormal();
        CurrentDirection = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, NormalizedVelocity)));
        
        // Determine if direction is left or right
        FVector CrossProduct = FVector::CrossProduct(ForwardVector, NormalizedVelocity);
        if (CrossProduct.Z < 0.0f)
        {
            CurrentDirection = -CurrentDirection;
        }
    }
    
    SmoothedDirection = FMath::FInterpTo(SmoothedDirection, CurrentDirection, DeltaTime, DirectionSmoothingSpeed);
    
    // Update motion data
    CurrentMotionData.Speed = SmoothedSpeed;
    CurrentMotionData.Direction = SmoothedDirection;
    CurrentMotionData.Velocity = CurrentVelocity;
    CurrentMotionData.Acceleration = CurrentAcceleration;
    CurrentMotionData.bIsMoving = CurrentSpeed > 10.0f;
    CurrentMotionData.bIsInAir = MovementComponent->IsFalling();
    CurrentMotionData.MovementState = CalculateMovementState();
    CurrentMotionData.LocomotionDirection = CalculateLocomotionDirection();
}

EAnim_MovementState UAnim_MotionMatchingSystem::CalculateMovementState()
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check if in air
    if (MovementComponent->IsFalling())
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
    if (MovementComponent->IsCrouching())
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Check if swimming
    if (MovementComponent->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    // Determine movement state based on speed
    float Speed = CurrentMotionData.Speed;
    
    if (Speed < 10.0f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (Speed < WalkSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else if (Speed < RunSpeedThreshold)
    {
        return EAnim_MovementState::Running;
    }
    else
    {
        return EAnim_MovementState::Sprinting;
    }
}

EAnim_LocomotionDirection UAnim_MotionMatchingSystem::CalculateLocomotionDirection()
{
    float Direction = CurrentMotionData.Direction;
    
    // Convert direction angle to locomotion direction enum
    if (FMath::Abs(Direction) <= 22.5f)
    {
        return EAnim_LocomotionDirection::Forward;
    }
    else if (FMath::Abs(Direction) >= 157.5f)
    {
        return EAnim_LocomotionDirection::Backward;
    }
    else if (Direction > 67.5f && Direction <= 112.5f)
    {
        return EAnim_LocomotionDirection::Right;
    }
    else if (Direction < -67.5f && Direction >= -112.5f)
    {
        return EAnim_LocomotionDirection::Left;
    }
    else if (Direction > 22.5f && Direction <= 67.5f)
    {
        return EAnim_LocomotionDirection::ForwardRight;
    }
    else if (Direction < -22.5f && Direction >= -67.5f)
    {
        return EAnim_LocomotionDirection::ForwardLeft;
    }
    else if (Direction > 112.5f && Direction <= 157.5f)
    {
        return EAnim_LocomotionDirection::BackwardRight;
    }
    else if (Direction < -112.5f && Direction >= -157.5f)
    {
        return EAnim_LocomotionDirection::BackwardLeft;
    }
    
    return EAnim_LocomotionDirection::Forward;
}

void UAnim_MotionMatchingSystem::SetBlendSpaceSettings(const FAnim_BlendSpaceSettings& NewSettings)
{
    BlendSpaceSettings = NewSettings;
}

UBlendSpace* UAnim_MotionMatchingSystem::GetCurrentBlendSpace() const
{
    switch (CurrentMotionData.MovementState)
    {
        case EAnim_MovementState::Idle:
            return BlendSpaceSettings.IdleBlendSpace;
        case EAnim_MovementState::Walking:
            return BlendSpaceSettings.WalkBlendSpace;
        case EAnim_MovementState::Running:
            return BlendSpaceSettings.RunBlendSpace;
        case EAnim_MovementState::Sprinting:
            return BlendSpaceSettings.SprintBlendSpace;
        default:
            return BlendSpaceSettings.IdleBlendSpace;
    }
}

float UAnim_MotionMatchingSystem::GetBlendSpaceX() const
{
    // Map speed to blend space X axis (0-1 range)
    float MaxSpeed = 0.0f;
    switch (CurrentMotionData.MovementState)
    {
        case EAnim_MovementState::Walking:
            MaxSpeed = WalkSpeedThreshold;
            break;
        case EAnim_MovementState::Running:
            MaxSpeed = RunSpeedThreshold;
            break;
        case EAnim_MovementState::Sprinting:
            MaxSpeed = SprintSpeedThreshold;
            break;
        default:
            MaxSpeed = 100.0f;
            break;
    }
    
    return FMath::Clamp(CurrentMotionData.Speed / MaxSpeed, 0.0f, 1.0f);
}

float UAnim_MotionMatchingSystem::GetBlendSpaceY() const
{
    // Map direction to blend space Y axis (-1 to 1 range)
    return FMath::Clamp(CurrentMotionData.Direction / 180.0f, -1.0f, 1.0f);
}