#include "SurvivalCharacterAnimController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"

USurvivalCharacterAnimController::USurvivalCharacterAnimController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize states
    CurrentMovementState = EAnim_SurvivalMovementState::Idle;
    CurrentEmotionalState = EAnim_SurvivalEmotionalState::Calm;

    // Initialize timers
    StateTransitionTimer = 0.0f;
    EmotionalTransitionTimer = 0.0f;

    // Initialize component references
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MeshComponent = nullptr;

    // Initialize animation assets to null
    JumpMontage = nullptr;
    LandMontage = nullptr;
    FearReactionMontage = nullptr;
    LocomotionBlendSpace = nullptr;
    CrouchBlendSpace = nullptr;
}

void USurvivalCharacterAnimController::BeginPlay()
{
    Super::BeginPlay();

    // Get component references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
        
        UE_LOG(LogTemp, Log, TEXT("SurvivalCharacterAnimController: Successfully initialized for character %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SurvivalCharacterAnimController: Owner is not a Character!"));
    }
}

void USurvivalCharacterAnimController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Update timers
    StateTransitionTimer += DeltaTime;
    EmotionalTransitionTimer += DeltaTime;

    // Update animation data
    UpdateAnimationData();
}

void USurvivalCharacterAnimController::UpdateAnimationData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Update all data
    UpdateMovementData();
    UpdateSurvivalData();
    UpdateMovementState();
    UpdateEmotionalState();
}

void USurvivalCharacterAnimController::UpdateMovementData()
{
    if (!MovementComponent)
    {
        return;
    }

    // Get velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    MovementData.Speed = Velocity.Size();
    
    // Calculate movement direction relative to character forward
    if (MovementData.Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal();
        MovementData.Direction = FVector::DotProduct(ForwardVector, VelocityNormalized);
        MovementData.bIsMoving = true;
    }
    else
    {
        MovementData.Direction = 0.0f;
        MovementData.bIsMoving = false;
    }

    // Update movement flags
    MovementData.bIsFalling = MovementComponent->IsFalling();
    MovementData.bIsJumping = MovementComponent->IsMovingOnGround() == false && Velocity.Z > 0.0f;
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
}

void USurvivalCharacterAnimController::UpdateSurvivalData()
{
    // For now, use placeholder values
    // In a real implementation, these would come from survival stats component
    
    // Simulate fear based on movement speed (running = more fear)
    if (MovementData.Speed > 400.0f)
    {
        SurvivalData.FearLevel = FMath::Clamp(SurvivalData.FearLevel + 0.01f, 0.0f, 1.0f);
    }
    else
    {
        SurvivalData.FearLevel = FMath::Clamp(SurvivalData.FearLevel - 0.005f, 0.0f, 1.0f);
    }

    // Simulate stamina drain when moving fast
    if (MovementData.Speed > 300.0f)
    {
        SurvivalData.StaminaLevel = FMath::Clamp(SurvivalData.StaminaLevel - 0.002f, 0.0f, 1.0f);
    }
    else if (MovementData.Speed < 50.0f)
    {
        SurvivalData.StaminaLevel = FMath::Clamp(SurvivalData.StaminaLevel + 0.001f, 0.0f, 1.0f);
    }

    // Update derived flags
    SurvivalData.bIsExhausted = SurvivalData.StaminaLevel < 0.2f;
    SurvivalData.bIsInjured = SurvivalData.HealthLevel < 0.5f;
}

void USurvivalCharacterAnimController::UpdateMovementState()
{
    EAnim_SurvivalMovementState NewState = CurrentMovementState;

    if (MovementData.bIsFalling)
    {
        if (MovementData.bIsJumping)
        {
            NewState = EAnim_SurvivalMovementState::Jumping;
        }
        else
        {
            NewState = EAnim_SurvivalMovementState::Falling;
        }
    }
    else if (MovementData.bIsCrouching)
    {
        NewState = EAnim_SurvivalMovementState::Crouching;
    }
    else if (MovementData.bIsMoving)
    {
        if (MovementData.Speed > 300.0f)
        {
            NewState = EAnim_SurvivalMovementState::Running;
        }
        else
        {
            NewState = EAnim_SurvivalMovementState::Walking;
        }
    }
    else
    {
        NewState = EAnim_SurvivalMovementState::Idle;
    }

    // Only update if state changed
    if (NewState != CurrentMovementState)
    {
        SetMovementState(NewState);
    }
}

void USurvivalCharacterAnimController::UpdateEmotionalState()
{
    EAnim_SurvivalEmotionalState NewState = CurrentEmotionalState;

    // Determine emotional state based on survival data
    if (SurvivalData.FearLevel > 0.8f)
    {
        NewState = EAnim_SurvivalEmotionalState::Panicked;
    }
    else if (SurvivalData.FearLevel > 0.4f)
    {
        NewState = EAnim_SurvivalEmotionalState::Fearful;
    }
    else if (SurvivalData.bIsExhausted)
    {
        NewState = EAnim_SurvivalEmotionalState::Exhausted;
    }
    else if (SurvivalData.bIsInjured)
    {
        NewState = EAnim_SurvivalEmotionalState::Injured;
    }
    else if (SurvivalData.FearLevel > 0.1f || MovementData.Speed > 200.0f)
    {
        NewState = EAnim_SurvivalEmotionalState::Alert;
    }
    else
    {
        NewState = EAnim_SurvivalEmotionalState::Calm;
    }

    // Only update if state changed and enough time has passed
    if (NewState != CurrentEmotionalState && EmotionalTransitionTimer > 0.5f)
    {
        SetEmotionalState(NewState);
    }
}

void USurvivalCharacterAnimController::SetMovementState(EAnim_SurvivalMovementState NewState)
{
    if (NewState != CurrentMovementState)
    {
        EAnim_SurvivalMovementState PreviousState = CurrentMovementState;
        CurrentMovementState = NewState;
        StateTransitionTimer = 0.0f;

        UE_LOG(LogTemp, Log, TEXT("Movement state changed from %d to %d"), 
               (int32)PreviousState, (int32)CurrentMovementState);

        // Handle special state transitions
        if (PreviousState == EAnim_SurvivalMovementState::Falling && 
            NewState != EAnim_SurvivalMovementState::Falling)
        {
            TriggerLandAnimation();
        }
    }
}

void USurvivalCharacterAnimController::SetEmotionalState(EAnim_SurvivalEmotionalState NewState)
{
    if (NewState != CurrentEmotionalState)
    {
        EAnim_SurvivalEmotionalState PreviousState = CurrentEmotionalState;
        CurrentEmotionalState = NewState;
        EmotionalTransitionTimer = 0.0f;

        UE_LOG(LogTemp, Log, TEXT("Emotional state changed from %d to %d"), 
               (int32)PreviousState, (int32)CurrentEmotionalState);

        // Trigger fear reaction if entering fearful state
        if (NewState == EAnim_SurvivalEmotionalState::Fearful || 
            NewState == EAnim_SurvivalEmotionalState::Panicked)
        {
            TriggerFearReaction();
        }
    }
}

void USurvivalCharacterAnimController::TriggerJumpAnimation()
{
    if (MeshComponent && JumpMontage)
    {
        UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
        if (AnimInstance && !AnimInstance->Montage_IsPlaying(JumpMontage))
        {
            AnimInstance->Montage_Play(JumpMontage);
            UE_LOG(LogTemp, Log, TEXT("Triggered jump animation"));
        }
    }
}

void USurvivalCharacterAnimController::TriggerLandAnimation()
{
    if (MeshComponent && LandMontage)
    {
        UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
        if (AnimInstance && !AnimInstance->Montage_IsPlaying(LandMontage))
        {
            AnimInstance->Montage_Play(LandMontage);
            UE_LOG(LogTemp, Log, TEXT("Triggered land animation"));
        }
    }
}

void USurvivalCharacterAnimController::TriggerFearReaction()
{
    if (MeshComponent && FearReactionMontage)
    {
        UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
        if (AnimInstance && !AnimInstance->Montage_IsPlaying(FearReactionMontage))
        {
            AnimInstance->Montage_Play(FearReactionMontage);
            UE_LOG(LogTemp, Log, TEXT("Triggered fear reaction animation"));
        }
    }
}