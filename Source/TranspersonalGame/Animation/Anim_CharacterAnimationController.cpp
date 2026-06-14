#include "Anim_CharacterAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_CharacterAnimationController::UAnim_CharacterAnimationController()
{
    OwningCharacter = nullptr;
    CharacterMovement = nullptr;
    
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 300.0f;
    DirectionDeadZone = 5.0f;
    
    IdleAnimation = nullptr;
    WalkAnimation = nullptr;
    RunAnimation = nullptr;
    JumpStartAnimation = nullptr;
    JumpLoopAnimation = nullptr;
    JumpEndAnimation = nullptr;
}

void UAnim_CharacterAnimationController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        CharacterMovement = OwningCharacter->GetCharacterMovement();
    }
    
    SetAnimationSequences();
}

void UAnim_CharacterAnimationController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    UpdateMovementData();
    UpdateMovementState();
}

void UAnim_CharacterAnimationController::UpdateMovementData()
{
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = CharacterMovement->Velocity;
    MovementData.Speed = Velocity.Size2D();
    
    // Calculate direction relative to character forward
    CalculateDirection();
    
    // Update movement flags
    MovementData.bIsInAir = CharacterMovement->IsFalling();
    MovementData.bIsCrouching = CharacterMovement->IsCrouching();
    MovementData.bIsAccelerating = CharacterMovement->GetCurrentAcceleration().Size2D() > 0.0f;
    
    // Determine movement state
    MovementData.MovementState = DetermineMovementState();
}

void UAnim_CharacterAnimationController::CalculateDirection()
{
    if (!OwningCharacter || !CharacterMovement)
    {
        MovementData.Direction = 0.0f;
        return;
    }
    
    FVector Velocity = CharacterMovement->Velocity;
    FVector ForwardVector = OwningCharacter->GetActorForwardVector();
    
    if (Velocity.Size2D() > DirectionDeadZone)
    {
        FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
        float DotProduct = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        FVector CrossProduct = FVector::CrossProduct(ForwardVector, NormalizedVelocity);
        
        MovementData.Direction = UKismetMathLibrary::RadiansToDegrees(FMath::Atan2(CrossProduct.Z, DotProduct));
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
}

EAnim_MovementState UAnim_CharacterAnimationController::DetermineMovementState() const
{
    if (MovementData.bIsInAir)
    {
        if (CharacterMovement && CharacterMovement->Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        return EAnim_MovementState::Falling;
    }
    
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    if (CharacterMovement && CharacterMovement->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    if (MovementData.Speed > RunSpeedThreshold)
    {
        return EAnim_MovementState::Running;
    }
    else if (MovementData.Speed > WalkSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    
    return EAnim_MovementState::Idle;
}

void UAnim_CharacterAnimationController::UpdateMovementState()
{
    // This function can be used to trigger animation montages or state changes
    // based on the current movement state
    
    switch (MovementData.MovementState)
    {
        case EAnim_MovementState::Idle:
            // Handle idle state logic
            break;
        case EAnim_MovementState::Walking:
            // Handle walking state logic
            break;
        case EAnim_MovementState::Running:
            // Handle running state logic
            break;
        case EAnim_MovementState::Jumping:
            // Handle jumping state logic
            break;
        case EAnim_MovementState::Falling:
            // Handle falling state logic
            break;
        case EAnim_MovementState::Crouching:
            // Handle crouching state logic
            break;
        case EAnim_MovementState::Swimming:
            // Handle swimming state logic
            break;
    }
}

void UAnim_CharacterAnimationController::SetAnimationSequences()
{
    // Load default animation sequences
    // These would typically be set in Blueprint or loaded from content
    
    // For now, we'll leave these as nullptr and they can be set in Blueprint
    // or through the editor
}

bool UAnim_CharacterAnimationController::ShouldEnterIdleState() const
{
    return MovementData.MovementState == EAnim_MovementState::Idle;
}

bool UAnim_CharacterAnimationController::ShouldEnterWalkState() const
{
    return MovementData.MovementState == EAnim_MovementState::Walking;
}

bool UAnim_CharacterAnimationController::ShouldEnterRunState() const
{
    return MovementData.MovementState == EAnim_MovementState::Running;
}

bool UAnim_CharacterAnimationController::ShouldEnterJumpState() const
{
    return MovementData.MovementState == EAnim_MovementState::Jumping || 
           MovementData.MovementState == EAnim_MovementState::Falling;
}