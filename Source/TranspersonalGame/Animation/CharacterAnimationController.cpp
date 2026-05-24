#include "CharacterAnimationController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCharacterAnimationController::UCharacterAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default settings
    Settings.WalkSpeedThreshold = 150.0f;
    Settings.RunSpeedThreshold = 400.0f;
    Settings.JumpVelocityThreshold = 50.0f;
    Settings.FallVelocityThreshold = -200.0f;
    Settings.InjuredHealthThreshold = 0.3f;
    Settings.ExhaustedStaminaThreshold = 0.2f;
    Settings.FearfulThreshold = 0.5f;
    Settings.AnimationBlendSpeed = 5.0f;
    Settings.StateTransitionDelay = 0.1f;

    // Initialize animation data
    AnimationData.Speed = 0.0f;
    AnimationData.Direction = 0.0f;
    AnimationData.bIsInAir = false;
    AnimationData.bIsCrouching = false;
    AnimationData.bIsInjured = false;
    AnimationData.bIsExhausted = false;
    AnimationData.bIsFearful = false;
    AnimationData.HealthPercentage = 1.0f;
    AnimationData.StaminaPercentage = 1.0f;
    AnimationData.FearLevel = 0.0f;
    AnimationData.CurrentState = EAnim_CharacterState::Idle;
    AnimationData.MovementDirection = EAnim_MovementDirection::Forward;
}

void UCharacterAnimationController::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponents();
}

void UCharacterAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateAnimationState(DeltaTime);
}

void UCharacterAnimationController::InitializeComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterAnimationController: No owner found"));
        return;
    }

    // Find skeletal mesh component
    SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterAnimationController: No skeletal mesh component found on %s"), *Owner->GetName());
        return;
    }

    // Get animation instance
    AnimInstance = SkeletalMeshComponent->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterAnimationController: No animation instance found on %s"), *Owner->GetName());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("CharacterAnimationController: Successfully initialized on %s"), *Owner->GetName());
}

void UCharacterAnimationController::UpdateAnimationState(float DeltaTime)
{
    if (!SkeletalMeshComponent || !AnimInstance)
    {
        return;
    }

    // Update state transition timer
    if (StateTransitionTimer > 0.0f)
    {
        StateTransitionTimer -= DeltaTime;
        if (StateTransitionTimer <= 0.0f && PendingState != AnimationData.CurrentState)
        {
            SetCharacterState(PendingState);
        }
    }

    // Determine current state based on movement and survival stats
    DetermineCharacterState();
    
    // Update movement direction
    UpdateMovementDirection();
}

void UCharacterAnimationController::DetermineCharacterState()
{
    EAnim_CharacterState NewState = EAnim_CharacterState::Idle;

    // Priority order: Dead > Injured > Exhausted > Fearful > Movement states
    if (AnimationData.HealthPercentage <= 0.0f)
    {
        NewState = EAnim_CharacterState::Dead;
    }
    else if (AnimationData.bIsInjured || AnimationData.HealthPercentage <= Settings.InjuredHealthThreshold)
    {
        NewState = EAnim_CharacterState::Injured;
    }
    else if (AnimationData.bIsExhausted || AnimationData.StaminaPercentage <= Settings.ExhaustedStaminaThreshold)
    {
        NewState = EAnim_CharacterState::Exhausted;
    }
    else if (AnimationData.bIsFearful || AnimationData.FearLevel >= Settings.FearfulThreshold)
    {
        NewState = EAnim_CharacterState::Fearful;
    }
    else if (AnimationData.bIsInAir)
    {
        // Determine if jumping or falling based on velocity
        ACharacter* Character = Cast<ACharacter>(GetOwner());
        if (Character && Character->GetCharacterMovement())
        {
            float VerticalVelocity = Character->GetCharacterMovement()->Velocity.Z;
            if (VerticalVelocity > Settings.JumpVelocityThreshold)
            {
                NewState = EAnim_CharacterState::Jumping;
            }
            else if (VerticalVelocity < Settings.FallVelocityThreshold)
            {
                NewState = EAnim_CharacterState::Falling;
            }
            else
            {
                NewState = EAnim_CharacterState::Landing;
            }
        }
        else
        {
            NewState = EAnim_CharacterState::Falling;
        }
    }
    else if (AnimationData.bIsCrouching)
    {
        if (AnimationData.Speed > Settings.WalkSpeedThreshold * 0.5f)
        {
            NewState = EAnim_CharacterState::Crawling;
        }
        else
        {
            NewState = EAnim_CharacterState::Crouching;
        }
    }
    else if (AnimationData.Speed > Settings.RunSpeedThreshold)
    {
        NewState = EAnim_CharacterState::Running;
    }
    else if (AnimationData.Speed > Settings.WalkSpeedThreshold)
    {
        NewState = EAnim_CharacterState::Walking;
    }
    else
    {
        NewState = EAnim_CharacterState::Idle;
    }

    // Apply state transition with delay if needed
    if (NewState != AnimationData.CurrentState && CanTransitionToState(NewState))
    {
        if (Settings.StateTransitionDelay > 0.0f)
        {
            PendingState = NewState;
            StateTransitionTimer = Settings.StateTransitionDelay;
        }
        else
        {
            SetCharacterState(NewState);
        }
    }
}

void UCharacterAnimationController::UpdateMovementDirection()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character || !Character->GetCharacterMovement())
    {
        return;
    }

    FVector Velocity = Character->GetCharacterMovement()->Velocity;
    FVector ForwardVector = Character->GetActorForwardVector();
    FVector RightVector = Character->GetActorRightVector();

    // Calculate direction relative to character's forward
    float ForwardDot = FVector::DotProduct(Velocity.GetSafeNormal(), ForwardVector);
    float RightDot = FVector::DotProduct(Velocity.GetSafeNormal(), RightVector);

    // Store direction as angle for blend spaces
    AnimationData.Direction = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));

    // Determine discrete movement direction
    const float DirectionThreshold = 0.3f;
    
    if (FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
    {
        if (ForwardDot > DirectionThreshold)
        {
            AnimationData.MovementDirection = EAnim_MovementDirection::Forward;
        }
        else if (ForwardDot < -DirectionThreshold)
        {
            AnimationData.MovementDirection = EAnim_MovementDirection::Backward;
        }
    }
    else
    {
        if (RightDot > DirectionThreshold)
        {
            AnimationData.MovementDirection = EAnim_MovementDirection::Right;
        }
        else if (RightDot < -DirectionThreshold)
        {
            AnimationData.MovementDirection = EAnim_MovementDirection::Left;
        }
    }

    // Handle diagonal movement
    if (FMath::Abs(ForwardDot) > DirectionThreshold && FMath::Abs(RightDot) > DirectionThreshold)
    {
        if (ForwardDot > 0 && RightDot > 0)
        {
            AnimationData.MovementDirection = EAnim_MovementDirection::ForwardRight;
        }
        else if (ForwardDot > 0 && RightDot < 0)
        {
            AnimationData.MovementDirection = EAnim_MovementDirection::ForwardLeft;
        }
        else if (ForwardDot < 0 && RightDot > 0)
        {
            AnimationData.MovementDirection = EAnim_MovementDirection::BackwardRight;
        }
        else if (ForwardDot < 0 && RightDot < 0)
        {
            AnimationData.MovementDirection = EAnim_MovementDirection::BackwardLeft;
        }
    }
}

bool UCharacterAnimationController::CanTransitionToState(EAnim_CharacterState NewState) const
{
    // Dead state cannot transition to anything
    if (AnimationData.CurrentState == EAnim_CharacterState::Dead)
    {
        return false;
    }

    // Some states have restrictions
    switch (NewState)
    {
        case EAnim_CharacterState::Landing:
            return AnimationData.CurrentState == EAnim_CharacterState::Falling || 
                   AnimationData.CurrentState == EAnim_CharacterState::Jumping;
        
        case EAnim_CharacterState::Jumping:
            return AnimationData.CurrentState != EAnim_CharacterState::Falling &&
                   AnimationData.CurrentState != EAnim_CharacterState::Landing;
        
        default:
            return true;
    }
}

void UCharacterAnimationController::SetCharacterState(EAnim_CharacterState NewState)
{
    if (NewState == AnimationData.CurrentState)
    {
        return;
    }

    EAnim_CharacterState OldState = AnimationData.CurrentState;
    AnimationData.CurrentState = NewState;
    
    OnStateChanged(OldState, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("CharacterAnimationController: State changed from %d to %d"), 
           (int32)OldState, (int32)NewState);
}

void UCharacterAnimationController::OnStateChanged(EAnim_CharacterState OldState, EAnim_CharacterState NewState)
{
    // Handle state-specific logic here
    switch (NewState)
    {
        case EAnim_CharacterState::Dead:
            // Stop all montages when dead
            StopMontage();
            break;
            
        case EAnim_CharacterState::Injured:
            // Could trigger injury reaction montage
            break;
            
        case EAnim_CharacterState::Fearful:
            // Could trigger fear reaction
            break;
            
        default:
            break;
    }
}

void UCharacterAnimationController::UpdateMovementData(float Speed, float Direction, bool bInAir)
{
    AnimationData.Speed = Speed;
    AnimationData.Direction = Direction;
    AnimationData.bIsInAir = bInAir;
}

void UCharacterAnimationController::UpdateSurvivalStats(float Health, float Stamina, float Fear)
{
    AnimationData.HealthPercentage = FMath::Clamp(Health, 0.0f, 1.0f);
    AnimationData.StaminaPercentage = FMath::Clamp(Stamina, 0.0f, 1.0f);
    AnimationData.FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);

    // Update boolean flags based on thresholds
    AnimationData.bIsInjured = AnimationData.HealthPercentage <= Settings.InjuredHealthThreshold;
    AnimationData.bIsExhausted = AnimationData.StaminaPercentage <= Settings.ExhaustedStaminaThreshold;
    AnimationData.bIsFearful = AnimationData.FearLevel >= Settings.FearfulThreshold;
}

void UCharacterAnimationController::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!AnimInstance || !Montage)
    {
        return;
    }

    AnimInstance->Montage_Play(Montage, PlayRate);
}

void UCharacterAnimationController::StopMontage(UAnimMontage* Montage)
{
    if (!AnimInstance)
    {
        return;
    }

    if (Montage)
    {
        AnimInstance->Montage_Stop(0.2f, Montage);
    }
    else
    {
        AnimInstance->StopAllMontages(0.2f);
    }
}

bool UCharacterAnimationController::IsPlayingMontage(UAnimMontage* Montage) const
{
    if (!AnimInstance)
    {
        return false;
    }

    if (Montage)
    {
        return AnimInstance->Montage_IsPlaying(Montage);
    }
    else
    {
        return AnimInstance->IsAnyMontagePlaying();
    }
}