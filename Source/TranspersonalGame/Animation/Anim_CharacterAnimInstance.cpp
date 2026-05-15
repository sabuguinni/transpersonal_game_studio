#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
    : OwningCharacter(nullptr)
    , MovementComponent(nullptr)
    , IdleAnimation(nullptr)
    , WalkAnimation(nullptr)
    , RunAnimation(nullptr)
    , JumpStartAnimation(nullptr)
    , JumpLoopAnimation(nullptr)
    , JumpEndAnimation(nullptr)
    , CrouchIdleAnimation(nullptr)
    , CrouchWalkAnimation(nullptr)
    , WalkRunBlendAlpha(0.0f)
    , IdleToMovementBlendAlpha(0.0f)
    , PreviousMovementState(EAnim_MovementState::Idle)
    , StateTransitionTime(0.0f)
    , MaxWalkSpeed(300.0f)
    , MaxRunSpeed(600.0f)
{
    // Initialize movement data
    MovementData = FAnim_MovementData();
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Get character reference
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
        
        if (MovementComponent)
        {
            MaxWalkSpeed = MovementComponent->MaxWalkSpeed;
            MaxRunSpeed = MovementComponent->MaxWalkSpeed * 2.0f; // Assume run is 2x walk speed
        }
    }

    // Initialize state
    PreviousMovementState = EAnim_MovementState::Idle;
    StateTransitionTime = 0.0f;
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }

    // Update movement data
    UpdateMovementData();
    
    // Update state machine
    UpdateMovementState();
    
    // Update blending parameters
    UpdateBlendingParameters();
    
    // Track state transition time
    StateTransitionTime += DeltaTimeX;
}

void UAnim_CharacterAnimInstance::UpdateMovementData()
{
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }

    // Get velocity and speed
    FVector Velocity = MovementComponent->Velocity;
    MovementData.Speed = Velocity.Size();
    
    // Calculate direction relative to character forward
    CalculateDirection();
    
    // Update movement flags
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    MovementData.bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;
}

void UAnim_CharacterAnimInstance::CalculateDirection()
{
    if (!OwningCharacter || !MovementComponent)
    {
        MovementData.Direction = 0.0f;
        return;
    }

    FVector Velocity = MovementComponent->Velocity;
    if (Velocity.SizeSquared() < 1.0f)
    {
        MovementData.Direction = 0.0f;
        return;
    }

    // Get character forward vector (ignore Z component for direction calculation)
    FVector ForwardVector = OwningCharacter->GetActorForwardVector();
    ForwardVector.Z = 0.0f;
    ForwardVector.Normalize();

    // Get velocity direction (ignore Z component)
    FVector VelocityDirection = Velocity;
    VelocityDirection.Z = 0.0f;
    VelocityDirection.Normalize();

    // Calculate angle between forward and velocity
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;

    // Convert to angle in degrees (-180 to 180)
    MovementData.Direction = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
}

void UAnim_CharacterAnimInstance::UpdateMovementState()
{
    EAnim_MovementState NewState = DetermineMovementState();
    
    if (NewState != MovementData.MovementState)
    {
        PreviousMovementState = MovementData.MovementState;
        MovementData.MovementState = NewState;
        StateTransitionTime = 0.0f;
    }
}

EAnim_MovementState UAnim_CharacterAnimInstance::DetermineMovementState() const
{
    if (!OwningCharacter || !MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }

    // Check for air states first
    if (MovementData.bIsInAir)
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

    // Check for swimming
    if (MovementComponent->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }

    // Check for crouching
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }

    // Check movement speed
    if (MovementData.Speed < 10.0f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (MovementData.Speed < MaxWalkSpeed * 0.8f)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

void UAnim_CharacterAnimInstance::UpdateBlendingParameters()
{
    // Update walk/run blend alpha
    if (MovementData.Speed > 0.0f)
    {
        float NormalizedSpeed = FMath::Clamp(MovementData.Speed / MaxRunSpeed, 0.0f, 1.0f);
        WalkRunBlendAlpha = FMath::FInterpTo(WalkRunBlendAlpha, NormalizedSpeed, GetWorld()->GetDeltaSeconds(), 5.0f);
    }
    else
    {
        WalkRunBlendAlpha = FMath::FInterpTo(WalkRunBlendAlpha, 0.0f, GetWorld()->GetDeltaSeconds(), 10.0f);
    }

    // Update idle to movement blend alpha
    bool bShouldMove = MovementData.MovementState == EAnim_MovementState::Walking || 
                      MovementData.MovementState == EAnim_MovementState::Running;
    
    float TargetAlpha = bShouldMove ? 1.0f : 0.0f;
    IdleToMovementBlendAlpha = FMath::FInterpTo(IdleToMovementBlendAlpha, TargetAlpha, GetWorld()->GetDeltaSeconds(), 8.0f);
}

bool UAnim_CharacterAnimInstance::ShouldEnterIdleState() const
{
    return MovementData.MovementState == EAnim_MovementState::Idle;
}

bool UAnim_CharacterAnimInstance::ShouldEnterMovementState() const
{
    return MovementData.MovementState == EAnim_MovementState::Walking || 
           MovementData.MovementState == EAnim_MovementState::Running;
}

bool UAnim_CharacterAnimInstance::ShouldEnterJumpState() const
{
    return MovementData.MovementState == EAnim_MovementState::Jumping;
}

bool UAnim_CharacterAnimInstance::ShouldEnterFallingState() const
{
    return MovementData.MovementState == EAnim_MovementState::Falling;
}