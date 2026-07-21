#include "Anim_CharacterAnimBlueprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterAnimBlueprint::UAnim_CharacterAnimBlueprint()
{
    OwningCharacter = nullptr;
    MovementComponent = nullptr;
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 300.0f;
    JumpVelocityThreshold = 50.0f;
}

void UAnim_CharacterAnimBlueprint::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
    }
}

void UAnim_CharacterAnimBlueprint::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }

    // Update movement data
    FVector Velocity = MovementComponent->Velocity;
    MovementData.Speed = Velocity.Size();
    MovementData.Direction = UKismetMathLibrary::CalculateDirection(Velocity, OwningCharacter->GetActorRotation()).Yaw;
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    MovementData.bIsSwimming = MovementComponent->IsSwimming();

    UpdateMovementState();
}

void UAnim_CharacterAnimBlueprint::UpdateMovementState()
{
    MovementData.MovementState = CalculateMovementState();
}

EAnim_MovementState UAnim_CharacterAnimBlueprint::CalculateMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }

    if (MovementData.bIsSwimming)
    {
        return EAnim_MovementState::Swimming;
    }

    if (MovementData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > JumpVelocityThreshold)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }

    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
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

bool UAnim_CharacterAnimBlueprint::ShouldPlayIdleAnimation() const
{
    return MovementData.MovementState == EAnim_MovementState::Idle;
}

bool UAnim_CharacterAnimBlueprint::ShouldPlayWalkAnimation() const
{
    return MovementData.MovementState == EAnim_MovementState::Walking;
}

bool UAnim_CharacterAnimBlueprint::ShouldPlayRunAnimation() const
{
    return MovementData.MovementState == EAnim_MovementState::Running;
}

bool UAnim_CharacterAnimBlueprint::ShouldPlayJumpAnimation() const
{
    return MovementData.MovementState == EAnim_MovementState::Jumping || 
           MovementData.MovementState == EAnim_MovementState::Falling;
}