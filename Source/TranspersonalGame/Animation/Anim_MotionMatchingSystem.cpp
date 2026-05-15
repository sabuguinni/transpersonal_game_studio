#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    WalkThreshold = 150.0f;
    RunThreshold = 300.0f;
    SprintThreshold = 500.0f;
    MovingThreshold = 10.0f;
    StateTransitionDelay = 0.1f;
    LastStateChangeTime = 0.0f;
    
    // Initialize motion data
    CurrentMotionData = FAnim_MotionData();
    PreviousMotionData = FAnim_MotionData();
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeReferences();
    
    // Set initial state
    CurrentMotionData.MovementState = EAnim_MovementState::Idle;
    CurrentMotionData.LocomotionType = EAnim_LocomotionType::Ground;
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (IsValidForMotionMatching())
    {
        UpdateMotionData();
    }
}

void UAnim_MotionMatchingSystem::InitializeReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        if (!MovementComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("MotionMatchingSystem: No CharacterMovementComponent found on %s"), *OwnerCharacter->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MotionMatchingSystem: Component not attached to a Character"));
    }
}

void UAnim_MotionMatchingSystem::UpdateMotionData()
{
    if (!IsValidForMotionMatching())
    {
        return;
    }
    
    // Store previous data
    PreviousMotionData = CurrentMotionData;
    
    // Update basic motion data
    CurrentMotionData.Velocity = MovementComponent->Velocity;
    CurrentMotionData.Speed = GetGroundSpeed();
    CurrentMotionData.Direction = CalculateMovementDirection();
    CurrentMotionData.bIsMoving = CurrentMotionData.Speed > MovingThreshold;
    CurrentMotionData.bIsInAir = MovementComponent->IsFalling();
    CurrentMotionData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Update movement state
    UpdateMovementState();
    
    // Update locomotion type
    UpdateLocomotionType();
}

void UAnim_MotionMatchingSystem::UpdateMovementState()
{
    EAnim_MovementState NewState = DetermineMovementState();
    
    // Check if we can transition to the new state
    if (NewState != CurrentMotionData.MovementState && CanTransitionToState(NewState))
    {
        TransitionToState(NewState);
    }
}

void UAnim_MotionMatchingSystem::UpdateLocomotionType()
{
    if (CurrentMotionData.bIsInAir)
    {
        CurrentMotionData.LocomotionType = EAnim_LocomotionType::Air;
    }
    else if (MovementComponent && MovementComponent->IsSwimming())
    {
        CurrentMotionData.LocomotionType = EAnim_LocomotionType::Water;
    }
    else if (MovementComponent && MovementComponent->IsClimbing())
    {
        CurrentMotionData.LocomotionType = EAnim_LocomotionType::Climbing;
    }
    else
    {
        CurrentMotionData.LocomotionType = EAnim_LocomotionType::Ground;
    }
}

EAnim_MovementState UAnim_MotionMatchingSystem::DetermineMovementState() const
{
    if (!IsValidForMotionMatching())
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check for air states first
    if (CurrentMotionData.bIsInAir)
    {
        if (CurrentMotionData.Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Check for crouching
    if (CurrentMotionData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Check ground movement based on speed
    if (!CurrentMotionData.bIsMoving)
    {
        return EAnim_MovementState::Idle;
    }
    
    if (CurrentMotionData.Speed >= SprintThreshold)
    {
        return EAnim_MovementState::Sprinting;
    }
    else if (CurrentMotionData.Speed >= RunThreshold)
    {
        return EAnim_MovementState::Running;
    }
    else if (CurrentMotionData.Speed >= WalkThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    
    return EAnim_MovementState::Idle;
}

float UAnim_MotionMatchingSystem::CalculateMovementDirection() const
{
    if (!IsValidForMotionMatching() || !CurrentMotionData.bIsMoving)
    {
        return 0.0f;
    }
    
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector MovementDirection = GetMovementDirection();
    
    if (MovementDirection.IsNearlyZero())
    {
        return 0.0f;
    }
    
    // Calculate angle between forward vector and movement direction
    float DotProduct = FVector::DotProduct(ForwardVector, MovementDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, MovementDirection).Z;
    
    float Angle = FMath::Atan2(CrossProduct, DotProduct);
    return FMath::RadiansToDegrees(Angle);
}

void UAnim_MotionMatchingSystem::TransitionToState(EAnim_MovementState NewState)
{
    if (GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        // Update state and timestamp
        CurrentMotionData.MovementState = NewState;
        LastStateChangeTime = CurrentTime;
        
        // Log state transition for debugging
        UE_LOG(LogTemp, Log, TEXT("MotionMatching: Transitioned to state %d"), (int32)NewState);
    }
}

bool UAnim_MotionMatchingSystem::CanTransitionToState(EAnim_MovementState NewState) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check if enough time has passed since last transition
    if (CurrentTime - LastStateChangeTime < StateTransitionDelay)
    {
        return false;
    }
    
    // Add specific transition rules here if needed
    // For now, allow all transitions
    return true;
}

void UAnim_MotionMatchingSystem::ResetAnimationState()
{
    CurrentMotionData = FAnim_MotionData();
    PreviousMotionData = FAnim_MotionData();
    LastStateChangeTime = 0.0f;
    
    if (GetWorld())
    {
        LastStateChangeTime = GetWorld()->GetTimeSeconds();
    }
}

void UAnim_MotionMatchingSystem::SetBlendSpaceConfiguration(const FAnim_BlendSpaceConfig& Config)
{
    BlendSpaceConfig = Config;
}

bool UAnim_MotionMatchingSystem::IsValidForMotionMatching() const
{
    return OwnerCharacter && MovementComponent && MovementComponent->IsValidLowLevel();
}

float UAnim_MotionMatchingSystem::GetGroundSpeed() const
{
    if (!MovementComponent)
    {
        return 0.0f;
    }
    
    FVector HorizontalVelocity = MovementComponent->Velocity;
    HorizontalVelocity.Z = 0.0f;
    
    return HorizontalVelocity.Size();
}

FVector UAnim_MotionMatchingSystem::GetMovementDirection() const
{
    if (!MovementComponent)
    {
        return FVector::ZeroVector;
    }
    
    FVector HorizontalVelocity = MovementComponent->Velocity;
    HorizontalVelocity.Z = 0.0f;
    
    return HorizontalVelocity.GetSafeNormal();
}