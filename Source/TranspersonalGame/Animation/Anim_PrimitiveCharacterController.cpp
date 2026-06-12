#include "Anim_PrimitiveCharacterController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_PrimitiveCharacterController::UAnim_PrimitiveCharacterController()
{
    OwningCharacter = nullptr;
    CharacterMovement = nullptr;
    
    // Initialize animation assets to nullptr
    LocomotionBlendSpace = nullptr;
    CraftingMontage = nullptr;
    GatheringMontage = nullptr;
    HuntingMontage = nullptr;
    AttackMontage = nullptr;
    DefendMontage = nullptr;
    InjuredMontage = nullptr;
    FearMontage = nullptr;
}

void UAnim_PrimitiveCharacterController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        CharacterMovement = OwningCharacter->GetCharacterMovement();
    }
}

void UAnim_PrimitiveCharacterController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    UpdateMovementData();
    UpdateActionData();
    UpdateSurvivalData();
}

void UAnim_PrimitiveCharacterController::UpdateMovementData()
{
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = CharacterMovement->Velocity;
    AnimData.Speed = Velocity.Size();
    
    // Calculate direction relative to character forward
    if (AnimData.Speed > 0.1f)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector RightVector = OwningCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        
        float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        
        AnimData.Direction = UKismetMathLibrary::DegAtan2(RightDot, ForwardDot);
    }
    else
    {
        AnimData.Direction = 0.0f;
    }
    
    // Update movement state flags
    AnimData.bIsInAir = CharacterMovement->IsFalling();
    AnimData.bIsCrouching = CharacterMovement->IsCrouching();
    
    // Calculate movement state
    AnimData.MovementState = CalculateMovementState();
}

void UAnim_PrimitiveCharacterController::UpdateActionData()
{
    // Action state is set externally via SetActionState
    // This function can be extended to handle automatic action detection
}

void UAnim_PrimitiveCharacterController::UpdateSurvivalData()
{
    // Get survival stats from character
    // This would integrate with the survival system
    // For now, use placeholder values
    AnimData.HealthPercent = 1.0f;
    AnimData.StaminaPercent = 1.0f;
    AnimData.FearLevel = 0.0f;
    
    // TODO: Integrate with actual survival stats from TranspersonalCharacter
}

EAnim_MovementState UAnim_PrimitiveCharacterController::CalculateMovementState()
{
    if (AnimData.bIsInAir)
    {
        if (CharacterMovement->Velocity.Z > 0)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    if (AnimData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    if (AnimData.ActionState == EAnim_ActionState::Hunting)
    {
        return EAnim_MovementState::Combat;
    }
    
    if (AnimData.HealthPercent < 0.3f)
    {
        return EAnim_MovementState::Injured;
    }
    
    // Speed-based movement states
    if (AnimData.Speed < 5.0f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (AnimData.Speed < 200.0f)
    {
        return EAnim_MovementState::Walking;
    }
    else if (AnimData.Speed < 400.0f)
    {
        return EAnim_MovementState::Running;
    }
    else
    {
        return EAnim_MovementState::Sprinting;
    }
}

void UAnim_PrimitiveCharacterController::PlayActionMontage(EAnim_ActionState ActionType)
{
    UAnimMontage* MontageToPlay = nullptr;
    
    switch (ActionType)
    {
        case EAnim_ActionState::Crafting:
            MontageToPlay = CraftingMontage;
            break;
        case EAnim_ActionState::Gathering:
            MontageToPlay = GatheringMontage;
            break;
        case EAnim_ActionState::Hunting:
            MontageToPlay = HuntingMontage;
            break;
        default:
            break;
    }
    
    if (MontageToPlay && !IsPlayingActionMontage())
    {
        Montage_Play(MontageToPlay);
        SetActionState(ActionType);
    }
}

void UAnim_PrimitiveCharacterController::StopActionMontage()
{
    if (IsPlayingActionMontage())
    {
        Montage_Stop(0.2f);
        SetActionState(EAnim_ActionState::None);
    }
}

void UAnim_PrimitiveCharacterController::SetMovementState(EAnim_MovementState NewState)
{
    AnimData.MovementState = NewState;
}

void UAnim_PrimitiveCharacterController::SetActionState(EAnim_ActionState NewState)
{
    AnimData.ActionState = NewState;
}

bool UAnim_PrimitiveCharacterController::IsPlayingActionMontage() const
{
    return IsAnyMontagePlaying();
}