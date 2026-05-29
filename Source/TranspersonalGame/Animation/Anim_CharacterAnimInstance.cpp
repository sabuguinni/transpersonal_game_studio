#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    CombatState = EAnim_CombatState::None;
    HealthPercentage = 1.0f;
    StaminaPercentage = 1.0f;
    FearLevel = 0.0f;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    UpdateMovementData();
    UpdateCombatState();
    UpdateSurvivalData();
}

void UAnim_CharacterAnimInstance::UpdateMovementData()
{
    if (!MovementComponent)
    {
        return;
    }

    // Get velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    MovementData.Speed = Velocity.Size2D();

    // Calculate movement direction relative to actor rotation
    if (MovementData.Speed > 0.0f && OwnerCharacter)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
        // Determine if moving left or right
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0.0f)
        {
            MovementData.Direction *= -1.0f;
        }
    }
    else
    {
        MovementData.Direction = 0.0f;
    }

    // Update movement state flags
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();

    // Calculate movement state
    MovementData.MovementState = CalculateMovementState();
}

EAnim_MovementState UAnim_CharacterAnimInstance::CalculateMovementState()
{
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

    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }

    if (MovementData.Speed > 300.0f) // Running threshold
    {
        return EAnim_MovementState::Running;
    }
    else if (MovementData.Speed > 10.0f) // Walking threshold
    {
        return EAnim_MovementState::Walking;
    }

    return EAnim_MovementState::Idle;
}

void UAnim_CharacterAnimInstance::UpdateCombatState()
{
    // For now, default to None - will be expanded when combat system is implemented
    CombatState = EAnim_CombatState::None;
    
    // TODO: Check for weapon equipped state
    // TODO: Check for attack input state
    // TODO: Check for blocking state
}

void UAnim_CharacterAnimInstance::UpdateSurvivalData()
{
    // Default survival values - will be connected to actual survival system
    HealthPercentage = 1.0f;
    StaminaPercentage = 1.0f;
    FearLevel = 0.0f;

    // TODO: Get actual health from character component
    // TODO: Get actual stamina from character component  
    // TODO: Calculate fear level based on nearby threats
}