#include "Anim_CharacterAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"

UAnim_CharacterAnimationController::UAnim_CharacterAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    MovementState = FAnim_MovementState();
    ActionState = FAnim_ActionState();
    
    OwnerCharacter = nullptr;
    AnimInstance = nullptr;
    MovementBlendSpace = nullptr;
    JumpMontage = nullptr;
    AttackMontage = nullptr;
    InteractionMontage = nullptr;
}

void UAnim_CharacterAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner character
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationController: Owner is not a Character!"));
        return;
    }
    
    // Get animation instance
    if (OwnerCharacter->GetMesh())
    {
        AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
        if (!AnimInstance)
        {
            UE_LOG(LogTemp, Warning, TEXT("AnimationController: No AnimInstance found!"));
        }
    }
    
    InitializeAnimationAssets();
}

void UAnim_CharacterAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !AnimInstance)
    {
        return;
    }
    
    UpdateAnimationState(DeltaTime);
    HandleMovementTransitions();
    HandleActionTransitions();
}

void UAnim_CharacterAnimationController::UpdateMovementAnimation(float Speed, float Direction, bool bInAir)
{
    MovementState.Speed = Speed;
    MovementState.Direction = Direction;
    MovementState.bIsInAir = bInAir;
    
    // Determine movement mode based on speed
    if (Speed < 50.0f)
    {
        MovementState.MovementMode = EAnim_MovementMode::Idle;
    }
    else if (Speed < 300.0f)
    {
        MovementState.MovementMode = EAnim_MovementMode::Walking;
    }
    else
    {
        MovementState.MovementMode = EAnim_MovementMode::Running;
    }
}

void UAnim_CharacterAnimationController::SetMovementMode(EAnim_MovementMode NewMode)
{
    if (MovementState.MovementMode != NewMode)
    {
        MovementState.MovementMode = NewMode;
        UE_LOG(LogTemp, Log, TEXT("AnimationController: Movement mode changed to %d"), (int32)NewMode);
    }
}

void UAnim_CharacterAnimationController::TriggerJumpAnimation()
{
    if (JumpMontage && AnimInstance)
    {
        AnimInstance->Montage_Play(JumpMontage, 1.0f);
        MovementState.bIsInAir = true;
        UE_LOG(LogTemp, Log, TEXT("AnimationController: Jump animation triggered"));
    }
}

void UAnim_CharacterAnimationController::TriggerLandingAnimation()
{
    MovementState.bIsInAir = false;
    UE_LOG(LogTemp, Log, TEXT("AnimationController: Landing animation triggered"));
}

void UAnim_CharacterAnimationController::PlayAttackAnimation(EAnim_AttackType AttackType)
{
    if (AttackMontage && AnimInstance)
    {
        ActionState.bIsAttacking = true;
        ActionState.CurrentAction = EAnim_ActionType::Combat;
        
        AnimInstance->Montage_Play(AttackMontage, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("AnimationController: Attack animation played - Type %d"), (int32)AttackType);
    }
}

void UAnim_CharacterAnimationController::PlayInteractionAnimation(EAnim_InteractionType InteractionType)
{
    if (InteractionMontage && AnimInstance)
    {
        ActionState.bIsInteracting = true;
        ActionState.CurrentAction = EAnim_ActionType::Interaction;
        
        AnimInstance->Montage_Play(InteractionMontage, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("AnimationController: Interaction animation played - Type %d"), (int32)InteractionType);
    }
}

void UAnim_CharacterAnimationController::PlayEmoteAnimation(EAnim_EmoteType EmoteType)
{
    if (AnimInstance)
    {
        ActionState.CurrentAction = EAnim_ActionType::Emote;
        UE_LOG(LogTemp, Log, TEXT("AnimationController: Emote animation played - Type %d"), (int32)EmoteType);
    }
}

bool UAnim_CharacterAnimationController::IsPlayingMontage() const
{
    if (AnimInstance)
    {
        return AnimInstance->IsAnyMontagePlaying();
    }
    return false;
}

void UAnim_CharacterAnimationController::InitializeAnimationAssets()
{
    // Try to load default animation assets
    // In a real project, these would be set in Blueprint or loaded from data assets
    
    UE_LOG(LogTemp, Log, TEXT("AnimationController: Initializing animation assets"));
    
    // Log available assets for debugging
    if (MovementBlendSpace)
    {
        UE_LOG(LogTemp, Log, TEXT("AnimationController: Movement BlendSpace assigned"));
    }
    
    if (JumpMontage)
    {
        UE_LOG(LogTemp, Log, TEXT("AnimationController: Jump Montage assigned"));
    }
    
    if (AttackMontage)
    {
        UE_LOG(LogTemp, Log, TEXT("AnimationController: Attack Montage assigned"));
    }
    
    if (InteractionMontage)
    {
        UE_LOG(LogTemp, Log, TEXT("AnimationController: Interaction Montage assigned"));
    }
}

void UAnim_CharacterAnimationController::UpdateAnimationState(float DeltaTime)
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Update movement state from character movement component
    if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
    {
        FVector Velocity = MovementComp->Velocity;
        float CurrentSpeed = Velocity.Size2D();
        
        // Calculate direction relative to character forward
        FVector Forward = OwnerCharacter->GetActorForwardVector();
        FVector Right = OwnerCharacter->GetActorRightVector();
        
        float ForwardDot = FVector::DotProduct(Velocity.GetSafeNormal2D(), Forward);
        float RightDot = FVector::DotProduct(Velocity.GetSafeNormal2D(), Right);
        
        float Direction = FMath::Atan2(RightDot, ForwardDot) * (180.0f / PI);
        
        bool bInAir = MovementComp->IsFalling();
        bool bCrouching = MovementComp->IsCrouching();
        
        UpdateMovementAnimation(CurrentSpeed, Direction, bInAir);
        MovementState.bIsCrouching = bCrouching;
    }
}

void UAnim_CharacterAnimationController::HandleMovementTransitions()
{
    // Handle smooth transitions between movement states
    // This would typically involve blend space updates or state machine transitions
    
    static EAnim_MovementMode LastMode = EAnim_MovementMode::Idle;
    
    if (MovementState.MovementMode != LastMode)
    {
        // Log transition for debugging
        UE_LOG(LogTemp, VeryVerbose, TEXT("AnimationController: Movement transition from %d to %d"), 
               (int32)LastMode, (int32)MovementState.MovementMode);
        
        LastMode = MovementState.MovementMode;
    }
}

void UAnim_CharacterAnimationController::HandleActionTransitions()
{
    // Handle action state transitions and montage completion
    
    if (ActionState.bIsAttacking && !IsPlayingMontage())
    {
        ActionState.bIsAttacking = false;
        ActionState.CurrentAction = EAnim_ActionType::None;
        UE_LOG(LogTemp, VeryVerbose, TEXT("AnimationController: Attack action completed"));
    }
    
    if (ActionState.bIsInteracting && !IsPlayingMontage())
    {
        ActionState.bIsInteracting = false;
        ActionState.CurrentAction = EAnim_ActionType::None;
        UE_LOG(LogTemp, VeryVerbose, TEXT("AnimationController: Interaction action completed"));
    }
}