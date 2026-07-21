#include "Anim_PlayerLocomotionBlueprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_PlayerLocomotionBlueprint::UAnim_PlayerLocomotionBlueprint()
{
    // Set default values
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    DirectionDeadZone = 5.0f;
    LeanInterpSpeed = 5.0f;
    StateTransitionSpeed = 10.0f;
    VelocityInterpSpeed = 6.0f;
    
    PreviousState = EAnim_LocomotionState::Idle;
    StateTransitionTime = 0.0f;
    PreviousVelocity = FVector::ZeroVector;
    CurrentLeanTarget = 0.0f;
}

void UAnim_PlayerLocomotionBlueprint::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    InitializeReferences();
    
    // Initialize locomotion data
    LocomotionData = FAnim_LocomotionData();
    PreviousState = EAnim_LocomotionState::Idle;
    StateTransitionTime = 0.0f;
}

void UAnim_PlayerLocomotionBlueprint::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !MovementComponent)
    {
        InitializeReferences();
        return;
    }
    
    // Update movement data
    UpdateMovementData(DeltaTimeX);
    
    // Update locomotion state
    UpdateLocomotionState();
    
    // Update movement direction
    UpdateMovementDirection();
    
    // Update lean angle for realistic movement
    UpdateLeanAngle(DeltaTimeX);
    
    // Smooth state transitions
    SmoothStateTransitions(DeltaTimeX);
}

void UAnim_PlayerLocomotionBlueprint::InitializeReferences()
{
    if (!OwningCharacter)
    {
        OwningCharacter = Cast<ACharacter>(GetOwningActor());
    }
    
    if (OwningCharacter && !MovementComponent)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
    }
}

void UAnim_PlayerLocomotionBlueprint::UpdateMovementData(float DeltaTime)
{
    if (!MovementComponent)
        return;
    
    // Get current velocity
    FVector CurrentVelocity = MovementComponent->Velocity;
    
    // Smooth velocity for animation
    LocomotionData.Velocity = FMath::VInterpTo(LocomotionData.Velocity, CurrentVelocity, DeltaTime, VelocityInterpSpeed);
    
    // Calculate speed (2D only for locomotion)
    FVector HorizontalVelocity = FVector(LocomotionData.Velocity.X, LocomotionData.Velocity.Y, 0.0f);
    LocomotionData.Speed = HorizontalVelocity.Size();
    
    // Calculate acceleration
    LocomotionData.Acceleration = (CurrentVelocity - PreviousVelocity) / DeltaTime;
    PreviousVelocity = CurrentVelocity;
    
    // Update air state
    LocomotionData.bIsInAir = MovementComponent->IsFalling();
    
    // Update crouch state
    LocomotionData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate direction relative to character forward
    if (OwningCharacter && LocomotionData.Speed > DirectionDeadZone)
    {
        LocomotionData.Direction = CalculateDirectionAngle();
    }
    else
    {
        LocomotionData.Direction = 0.0f;
    }
}

void UAnim_PlayerLocomotionBlueprint::UpdateLocomotionState()
{
    EAnim_LocomotionState NewState = LocomotionData.CurrentState;
    
    // Determine new state based on conditions
    if (ShouldEnterJumpState())
    {
        NewState = EAnim_LocomotionState::Jumping;
    }
    else if (LocomotionData.bIsInAir)
    {
        NewState = EAnim_LocomotionState::Falling;
    }
    else if (ShouldEnterCrouchState())
    {
        NewState = EAnim_LocomotionState::Crouching;
    }
    else if (ShouldEnterRunState())
    {
        NewState = EAnim_LocomotionState::Running;
    }
    else if (ShouldEnterWalkState())
    {
        NewState = EAnim_LocomotionState::Walking;
    }
    else if (ShouldEnterIdleState())
    {
        NewState = EAnim_LocomotionState::Idle;
    }
    
    // Update state if changed
    if (NewState != LocomotionData.CurrentState)
    {
        PreviousState = LocomotionData.CurrentState;
        LocomotionData.CurrentState = NewState;
        StateTransitionTime = 0.0f;
    }
}

void UAnim_PlayerLocomotionBlueprint::UpdateMovementDirection()
{
    if (LocomotionData.Speed <= DirectionDeadZone)
    {
        LocomotionData.MovementDirection = EAnim_MovementDirection::Forward;
        return;
    }
    
    float DirectionAngle = LocomotionData.Direction;
    LocomotionData.MovementDirection = DetermineMovementDirection(DirectionAngle);
}

void UAnim_PlayerLocomotionBlueprint::UpdateLeanAngle(float DeltaTime)
{
    if (!OwningCharacter || LocomotionData.Speed <= DirectionDeadZone)
    {
        CurrentLeanTarget = 0.0f;
    }
    else
    {
        // Calculate lean based on acceleration and turning
        FVector HorizontalAccel = FVector(LocomotionData.Acceleration.X, LocomotionData.Acceleration.Y, 0.0f);
        float AccelMagnitude = HorizontalAccel.Size();
        
        // Get the cross product to determine lean direction
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector RightVector = OwningCharacter->GetActorRightVector();
        
        float LateralAccel = FVector::DotProduct(HorizontalAccel, RightVector);
        CurrentLeanTarget = FMath::Clamp(LateralAccel * 0.1f, -30.0f, 30.0f);
    }
    
    // Smooth lean angle interpolation
    LocomotionData.LeanAngle = FMath::FInterpTo(LocomotionData.LeanAngle, CurrentLeanTarget, DeltaTime, LeanInterpSpeed);
}

void UAnim_PlayerLocomotionBlueprint::SmoothStateTransitions(float DeltaTime)
{
    StateTransitionTime += DeltaTime;
    
    // Add any additional smoothing logic here if needed
    // This can be used for blend weights in Animation Blueprints
}

float UAnim_PlayerLocomotionBlueprint::CalculateDirectionAngle() const
{
    if (!OwningCharacter || LocomotionData.Speed <= DirectionDeadZone)
        return 0.0f;
    
    FVector ForwardVector = OwningCharacter->GetActorForwardVector();
    FVector VelocityDirection = LocomotionData.Velocity.GetSafeNormal();
    
    // Remove Z component for 2D calculation
    ForwardVector.Z = 0.0f;
    VelocityDirection.Z = 0.0f;
    
    ForwardVector.Normalize();
    VelocityDirection.Normalize();
    
    // Calculate angle between forward and velocity
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
    
    float Angle = FMath::Atan2(CrossProduct, DotProduct);
    return FMath::RadiansToDegrees(Angle);
}

EAnim_MovementDirection UAnim_PlayerLocomotionBlueprint::DetermineMovementDirection(float DirectionAngle) const
{
    float AbsAngle = FMath::Abs(DirectionAngle);
    
    // Forward movement
    if (AbsAngle <= 22.5f)
    {
        return EAnim_MovementDirection::Forward;
    }
    // Backward movement
    else if (AbsAngle >= 157.5f)
    {
        return EAnim_MovementDirection::Backward;
    }
    // Side and diagonal movements
    else if (DirectionAngle > 0.0f)  // Right side
    {
        if (DirectionAngle <= 67.5f)
            return EAnim_MovementDirection::ForwardRight;
        else if (DirectionAngle <= 112.5f)
            return EAnim_MovementDirection::Right;
        else
            return EAnim_MovementDirection::BackwardRight;
    }
    else  // Left side
    {
        if (DirectionAngle >= -67.5f)
            return EAnim_MovementDirection::ForwardLeft;
        else if (DirectionAngle >= -112.5f)
            return EAnim_MovementDirection::Left;
        else
            return EAnim_MovementDirection::BackwardLeft;
    }
}

bool UAnim_PlayerLocomotionBlueprint::ShouldEnterIdleState() const
{
    return LocomotionData.Speed < WalkSpeedThreshold && !LocomotionData.bIsInAir && !LocomotionData.bIsCrouching;
}

bool UAnim_PlayerLocomotionBlueprint::ShouldEnterWalkState() const
{
    return LocomotionData.Speed >= WalkSpeedThreshold && 
           LocomotionData.Speed < RunSpeedThreshold && 
           !LocomotionData.bIsInAir && 
           !LocomotionData.bIsCrouching;
}

bool UAnim_PlayerLocomotionBlueprint::ShouldEnterRunState() const
{
    return LocomotionData.Speed >= RunSpeedThreshold && 
           !LocomotionData.bIsInAir && 
           !LocomotionData.bIsCrouching;
}

bool UAnim_PlayerLocomotionBlueprint::ShouldEnterJumpState() const
{
    return MovementComponent && 
           MovementComponent->IsFalling() && 
           LocomotionData.Velocity.Z > 0.0f;
}

bool UAnim_PlayerLocomotionBlueprint::ShouldEnterCrouchState() const
{
    return LocomotionData.bIsCrouching && !LocomotionData.bIsInAir;
}

bool UAnim_PlayerLocomotionBlueprint::IsMovingForward() const
{
    return LocomotionData.MovementDirection == EAnim_MovementDirection::Forward ||
           LocomotionData.MovementDirection == EAnim_MovementDirection::ForwardLeft ||
           LocomotionData.MovementDirection == EAnim_MovementDirection::ForwardRight;
}

bool UAnim_PlayerLocomotionBlueprint::IsMovingBackward() const
{
    return LocomotionData.MovementDirection == EAnim_MovementDirection::Backward ||
           LocomotionData.MovementDirection == EAnim_MovementDirection::BackwardLeft ||
           LocomotionData.MovementDirection == EAnim_MovementDirection::BackwardRight;
}

bool UAnim_PlayerLocomotionBlueprint::IsMovingSideways() const
{
    return LocomotionData.MovementDirection == EAnim_MovementDirection::Left ||
           LocomotionData.MovementDirection == EAnim_MovementDirection::Right;
}