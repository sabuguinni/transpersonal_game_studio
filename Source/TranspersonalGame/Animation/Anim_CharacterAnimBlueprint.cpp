#include "Anim_CharacterAnimBlueprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterAnimBlueprint::UAnim_CharacterAnimBlueprint()
{
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 300.0f;
    JumpVelocityThreshold = 100.0f;
    FallVelocityThreshold = -100.0f;
}

void UAnim_CharacterAnimBlueprint::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    UpdateCharacterReferences();
}

void UAnim_CharacterAnimBlueprint::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        UpdateCharacterReferences();
        return;
    }

    UpdateMovementData();
    UpdateMovementState();
    CalculateDirection();
}

void UAnim_CharacterAnimBlueprint::UpdateCharacterReferences()
{
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UAnim_CharacterAnimBlueprint::UpdateMovementData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Update basic movement data
    FVector Velocity = MovementComponent->Velocity;
    MovementData.Speed = Velocity.Size2D();
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    MovementData.bIsSwimming = MovementComponent->IsSwimming();
}

void UAnim_CharacterAnimBlueprint::UpdateMovementState()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        MovementData.MovementState = EAnim_MovementState::Idle;
        return;
    }

    // Priority order: Air states -> Ground states
    if (ShouldEnterJumpState())
    {
        MovementData.MovementState = EAnim_MovementState::Jumping;
    }
    else if (ShouldEnterFallState())
    {
        MovementData.MovementState = EAnim_MovementState::Falling;
    }
    else if (MovementData.bIsSwimming)
    {
        MovementData.MovementState = EAnim_MovementState::Swimming;
    }
    else if (MovementData.bIsCrouching)
    {
        MovementData.MovementState = EAnim_MovementState::Crouching;
    }
    else if (ShouldEnterRunState())
    {
        MovementData.MovementState = EAnim_MovementState::Running;
    }
    else if (ShouldEnterWalkState())
    {
        MovementData.MovementState = EAnim_MovementState::Walking;
    }
    else
    {
        MovementData.MovementState = EAnim_MovementState::Idle;
    }
}

void UAnim_CharacterAnimBlueprint::CalculateDirection()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        MovementData.Direction = 0.0f;
        return;
    }

    FVector Velocity = MovementComponent->Velocity;
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    
    if (Velocity.Size2D() > 0.1f)
    {
        FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
        float DotProduct = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float CrossProduct = FVector::CrossProduct(ForwardVector, NormalizedVelocity).Z;
        
        MovementData.Direction = UKismetMathLibrary::DegAtan2(CrossProduct, DotProduct);
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
}

bool UAnim_CharacterAnimBlueprint::ShouldEnterIdleState() const
{
    return MovementData.Speed < WalkSpeedThreshold && !MovementData.bIsInAir;
}

bool UAnim_CharacterAnimBlueprint::ShouldEnterWalkState() const
{
    return MovementData.Speed >= WalkSpeedThreshold && 
           MovementData.Speed < RunSpeedThreshold && 
           !MovementData.bIsInAir;
}

bool UAnim_CharacterAnimBlueprint::ShouldEnterRunState() const
{
    return MovementData.Speed >= RunSpeedThreshold && !MovementData.bIsInAir;
}

bool UAnim_CharacterAnimBlueprint::ShouldEnterJumpState() const
{
    if (!MovementComponent)
    {
        return false;
    }
    
    return MovementData.bIsInAir && MovementComponent->Velocity.Z > JumpVelocityThreshold;
}

bool UAnim_CharacterAnimBlueprint::ShouldEnterFallState() const
{
    if (!MovementComponent)
    {
        return false;
    }
    
    return MovementData.bIsInAir && MovementComponent->Velocity.Z < FallVelocityThreshold;
}