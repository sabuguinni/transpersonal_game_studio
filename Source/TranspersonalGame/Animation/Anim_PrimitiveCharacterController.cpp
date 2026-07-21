#include "Anim_PrimitiveCharacterController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_PrimitiveCharacterController::UAnim_PrimitiveCharacterController()
{
    OwningCharacter = nullptr;
    CharacterMovement = nullptr;
    
    // Set default movement thresholds
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    
    // Initialize animation montages to nullptr
    GatheringMontage = nullptr;
    CraftingMontage = nullptr;
    HuntingMontage = nullptr;
    BuildingMontage = nullptr;
    EatingMontage = nullptr;
    DrinkingMontage = nullptr;
}

void UAnim_PrimitiveCharacterController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get the owning character
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    
    if (OwningCharacter)
    {
        // Get the character movement component
        CharacterMovement = OwningCharacter->GetCharacterMovement();
    }
}

void UAnim_PrimitiveCharacterController::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Update animation data
    UpdateMovementData();
    UpdateActionState();
}

void UAnim_PrimitiveCharacterController::UpdateMovementData()
{
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = CharacterMovement->Velocity;
    AnimationData.Speed = Velocity.Size();
    
    // Calculate direction relative to character forward
    if (AnimationData.Speed > 0.0f)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector RightVector = OwningCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        
        float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        
        AnimationData.Direction = UKismetMathLibrary::DegAtan2(RightDot, ForwardDot);
    }
    else
    {
        AnimationData.Direction = 0.0f;
    }
    
    // Update air state
    AnimationData.bIsInAir = CharacterMovement->IsFalling();
    
    // Update crouching state
    AnimationData.bIsCrouching = CharacterMovement->IsCrouching();
    
    // Calculate movement state
    AnimationData.MovementState = CalculateMovementState();
}

void UAnim_PrimitiveCharacterController::UpdateActionState()
{
    // This would be updated based on player input or game state
    // For now, we maintain the current action state
    // In a full implementation, this would check for active survival actions
}

EAnim_MovementState UAnim_PrimitiveCharacterController::CalculateMovementState()
{
    if (!CharacterMovement)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check if in air first
    if (AnimationData.bIsInAir)
    {
        if (CharacterMovement->Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Check if crouching
    if (AnimationData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Check if in combat (would be set by combat system)
    if (AnimationData.bIsInCombat)
    {
        return EAnim_MovementState::Combat;
    }
    
    // Determine movement state based on speed
    if (AnimationData.Speed < 10.0f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (AnimationData.Speed < RunSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

void UAnim_PrimitiveCharacterController::PlaySurvivalAction(EAnim_ActionState ActionType)
{
    UAnimMontage* MontageToPlay = nullptr;
    
    switch (ActionType)
    {
        case EAnim_ActionState::Gathering:
            MontageToPlay = GatheringMontage;
            break;
        case EAnim_ActionState::Crafting:
            MontageToPlay = CraftingMontage;
            break;
        case EAnim_ActionState::Hunting:
            MontageToPlay = HuntingMontage;
            break;
        case EAnim_ActionState::Building:
            MontageToPlay = BuildingMontage;
            break;
        case EAnim_ActionState::Eating:
            MontageToPlay = EatingMontage;
            break;
        case EAnim_ActionState::Drinking:
            MontageToPlay = DrinkingMontage;
            break;
        default:
            return;
    }
    
    if (MontageToPlay)
    {
        Montage_Play(MontageToPlay);
        AnimationData.ActionState = ActionType;
    }
}

void UAnim_PrimitiveCharacterController::StopCurrentAction()
{
    if (IsAnyMontagePlaying())
    {
        Montage_Stop(0.2f);
    }
    
    AnimationData.ActionState = EAnim_ActionState::None;
}