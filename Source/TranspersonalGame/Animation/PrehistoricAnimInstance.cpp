#include "PrehistoricAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UPrehistoricAnimInstance::UPrehistoricAnimInstance()
{
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentActionState = EAnim_ActionState::None;
    WalkRunBlend = 0.0f;
    FearIntensity = 0.0f;
    FatigueLevel = 0.0f;
    InjuryLevel = 0.0f;
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UPrehistoricAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Get character and movement component references
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }

    // Initialize default values
    MovementData = FAnim_MovementData();
    SurvivalData = FAnim_SurvivalData();
}

void UPrehistoricAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Update all animation data
    UpdateMovementData();
    UpdateSurvivalData();
    UpdateMovementState();
    UpdateActionState();
    UpdateBlendingParameters();
}

void UPrehistoricAnimInstance::UpdateMovementData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Get velocity and speed
    MovementData.Velocity = MovementComponent->Velocity;
    MovementData.Speed = MovementData.Velocity.Size();
    MovementData.GroundSpeed = MovementData.Velocity.Size2D();

    // Calculate movement direction relative to character rotation
    if (MovementData.GroundSpeed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = MovementData.Velocity.GetSafeNormal2D();
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

    // Update air and crouch states
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
}

void UPrehistoricAnimInstance::UpdateSurvivalData()
{
    // In a real implementation, this would get data from a survival component
    // For now, we'll simulate some basic values
    
    // Simulate stamina drain during movement
    if (MovementData.GroundSpeed > 400.0f) // Running
    {
        SurvivalData.Stamina = FMath::Max(0.0f, SurvivalData.Stamina - 0.1f);
    }
    else if (MovementData.GroundSpeed > 0.1f) // Walking
    {
        SurvivalData.Stamina = FMath::Min(100.0f, SurvivalData.Stamina + 0.05f);
    }
    else // Idle
    {
        SurvivalData.Stamina = FMath::Min(100.0f, SurvivalData.Stamina + 0.1f);
    }

    // Simulate fear based on proximity to threats (placeholder)
    SurvivalData.Fear = FMath::Max(0.0f, SurvivalData.Fear - 0.5f);

    // Health regeneration when not in combat
    if (CurrentActionState != EAnim_ActionState::Combat)
    {
        SurvivalData.Health = FMath::Min(100.0f, SurvivalData.Health + 0.01f);
    }
}

void UPrehistoricAnimInstance::UpdateMovementState()
{
    EAnim_MovementState NewState = CurrentMovementState;

    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > 0.1f)
        {
            NewState = EAnim_MovementState::Jumping;
        }
        else
        {
            NewState = EAnim_MovementState::Falling;
        }
    }
    else if (MovementData.bIsCrouching)
    {
        NewState = EAnim_MovementState::Crouching;
    }
    else if (MovementData.GroundSpeed > 400.0f)
    {
        NewState = EAnim_MovementState::Running;
    }
    else if (MovementData.GroundSpeed > 50.0f)
    {
        NewState = EAnim_MovementState::Walking;
    }
    else
    {
        NewState = EAnim_MovementState::Idle;
    }

    CurrentMovementState = NewState;
}

void UPrehistoricAnimInstance::UpdateActionState()
{
    // Action states are typically set by external systems
    // This function handles automatic transitions or timeouts
    
    // Auto-return to None state after certain actions complete
    // This would be expanded with proper action duration tracking
}

void UPrehistoricAnimInstance::UpdateBlendingParameters()
{
    // Walk/Run blend based on speed
    float MaxWalkSpeed = 300.0f;
    float MaxRunSpeed = 600.0f;
    
    if (MovementData.GroundSpeed <= MaxWalkSpeed)
    {
        WalkRunBlend = MovementData.GroundSpeed / MaxWalkSpeed;
    }
    else
    {
        WalkRunBlend = 1.0f + ((MovementData.GroundSpeed - MaxWalkSpeed) / (MaxRunSpeed - MaxWalkSpeed));
    }
    WalkRunBlend = FMath::Clamp(WalkRunBlend, 0.0f, 2.0f);

    // Fear intensity based on fear level
    FearIntensity = SurvivalData.Fear / 100.0f;

    // Fatigue level based on stamina
    FatigueLevel = 1.0f - (SurvivalData.Stamina / 100.0f);

    // Injury level based on health
    InjuryLevel = 1.0f - (SurvivalData.Health / 100.0f);
}

bool UPrehistoricAnimInstance::ShouldTransitionToRunning() const
{
    return MovementData.GroundSpeed > 400.0f && !MovementData.bIsInAir && !MovementData.bIsCrouching;
}

bool UPrehistoricAnimInstance::ShouldTransitionToWalking() const
{
    return MovementData.GroundSpeed > 50.0f && MovementData.GroundSpeed <= 400.0f && !MovementData.bIsInAir && !MovementData.bIsCrouching;
}

bool UPrehistoricAnimInstance::ShouldTransitionToIdle() const
{
    return MovementData.GroundSpeed <= 50.0f && !MovementData.bIsInAir && !MovementData.bIsCrouching;
}

bool UPrehistoricAnimInstance::ShouldTransitionToJumping() const
{
    return MovementData.bIsInAir && MovementData.Velocity.Z > 0.1f;
}

bool UPrehistoricAnimInstance::ShouldTransitionToFalling() const
{
    return MovementData.bIsInAir && MovementData.Velocity.Z <= 0.1f;
}

void UPrehistoricAnimInstance::TriggerGatheringAnimation()
{
    SetActionState(EAnim_ActionState::Gathering);
}

void UPrehistoricAnimInstance::TriggerCraftingAnimation()
{
    SetActionState(EAnim_ActionState::Crafting);
}

void UPrehistoricAnimInstance::TriggerCombatAnimation()
{
    SetActionState(EAnim_ActionState::Combat);
}

void UPrehistoricAnimInstance::TriggerEatingAnimation()
{
    SetActionState(EAnim_ActionState::Eating);
}

void UPrehistoricAnimInstance::TriggerDrinkingAnimation()
{
    SetActionState(EAnim_ActionState::Drinking);
}

void UPrehistoricAnimInstance::SetActionState(EAnim_ActionState NewActionState)
{
    CurrentActionState = NewActionState;
}

bool UPrehistoricAnimInstance::IsPerformingAction() const
{
    return CurrentActionState != EAnim_ActionState::None;
}

float UPrehistoricAnimInstance::GetMovementSpeedRatio() const
{
    if (!MovementComponent)
    {
        return 0.0f;
    }

    float MaxSpeed = MovementComponent->GetMaxSpeed();
    if (MaxSpeed > 0.0f)
    {
        return MovementData.GroundSpeed / MaxSpeed;
    }

    return 0.0f;
}

bool UPrehistoricAnimInstance::ShouldPlayFearAnimation() const
{
    return SurvivalData.Fear > 50.0f;
}

bool UPrehistoricAnimInstance::ShouldPlayFatigueAnimation() const
{
    return SurvivalData.Stamina < 20.0f;
}