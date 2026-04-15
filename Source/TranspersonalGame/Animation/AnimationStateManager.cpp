#include "AnimationStateManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace1D.h"
#include "Engine/Engine.h"

UAnimationStateManager::UAnimationStateManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default states
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentActionState = EAnim_ActionState::None;
    PreviousMovementState = EAnim_MovementState::Idle;
    
    // Initialize movement data
    MovementData = FAnim_MovementData();
    
    // Terrain adaptation settings
    bTerrainAdaptationEnabled = true;
    MaxGroundDistance = 200.0f;
    TerrainAdaptationSpeed = 5.0f;
    
    // Transition state
    StateTransitionTimer = 0.0f;
    bIsTransitioning = false;
}

void UAnimationStateManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get skeletal mesh component from owner
    if (AActor* Owner = GetOwner())
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!SkeletalMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("AnimationStateManager: No SkeletalMeshComponent found on owner %s"), *Owner->GetName());
        }
    }
    
    // Initialize default state transitions
    InitializeDefaultTransitions();
    
    UE_LOG(LogTemp, Log, TEXT("AnimationStateManager initialized for %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UAnimationStateManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update transition timer
    if (bIsTransitioning)
    {
        StateTransitionTimer -= DeltaTime;
        if (StateTransitionTimer <= 0.0f)
        {
            bIsTransitioning = false;
        }
    }
    
    // Update animation blending
    UpdateAnimationBlending(DeltaTime);
    
    // Auto-update movement state based on movement data
    if (!bIsTransitioning)
    {
        EAnim_MovementState NewState = CurrentMovementState;
        
        if (MovementData.bIsInAir)
        {
            if (MovementData.Velocity.Z > 50.0f)
            {
                NewState = EAnim_MovementState::Jumping;
            }
            else
            {
                NewState = EAnim_MovementState::Falling;
            }
        }
        else if (MovementData.bIsMoving)
        {
            if (MovementData.Speed > 600.0f)
            {
                NewState = EAnim_MovementState::Sprinting;
            }
            else if (MovementData.Speed > 300.0f)
            {
                NewState = EAnim_MovementState::Running;
            }
            else if (MovementData.Speed > 50.0f)
            {
                NewState = EAnim_MovementState::Walking;
            }
            else
            {
                NewState = EAnim_MovementState::Idle;
            }
        }
        else
        {
            NewState = EAnim_MovementState::Idle;
        }
        
        if (NewState != CurrentMovementState)
        {
            SetMovementState(NewState);
        }
    }
}

void UAnimationStateManager::SetMovementState(EAnim_MovementState NewState)
{
    if (NewState == CurrentMovementState || !CanTransitionToState(NewState))
    {
        return;
    }
    
    // Find transition data
    FAnim_StateTransition* Transition = nullptr;
    for (FAnim_StateTransition& Trans : StateTransitions)
    {
        if (Trans.FromState == CurrentMovementState && Trans.ToState == NewState)
        {
            Transition = &Trans;
            break;
        }
    }
    
    // Store previous state
    PreviousMovementState = CurrentMovementState;
    CurrentMovementState = NewState;
    
    // Set up transition
    if (Transition)
    {
        StateTransitionTimer = Transition->TransitionDuration;
        bIsTransitioning = StateTransitionTimer > 0.0f;
    }
    else
    {
        // Default transition
        StateTransitionTimer = 0.2f;
        bIsTransitioning = true;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AnimationStateManager: Transition from %d to %d"), 
           (int32)PreviousMovementState, (int32)CurrentMovementState);
}

void UAnimationStateManager::SetActionState(EAnim_ActionState NewState)
{
    if (NewState == CurrentActionState)
    {
        return;
    }
    
    // Stop current action montage if any
    if (CurrentActionState != EAnim_ActionState::None)
    {
        if (UAnimMontage** CurrentMontage = ActionMontages.Find(CurrentActionState))
        {
            if (*CurrentMontage)
            {
                StopMontage(*CurrentMontage);
            }
        }
    }
    
    CurrentActionState = NewState;
    
    // Play new action montage
    if (NewState != EAnim_ActionState::None)
    {
        if (UAnimMontage** NewMontage = ActionMontages.Find(NewState))
        {
            if (*NewMontage)
            {
                PlayMontage(*NewMontage);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("AnimationStateManager: Action state changed to %d"), (int32)CurrentActionState);
}

void UAnimationStateManager::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage || !SkeletalMeshComponent)
    {
        return;
    }
    
    if (UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance())
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
        UE_LOG(LogTemp, Log, TEXT("AnimationStateManager: Playing montage %s"), *Montage->GetName());
    }
}

void UAnimationStateManager::StopMontage(UAnimMontage* Montage)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    if (UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance())
    {
        if (Montage)
        {
            AnimInstance->Montage_Stop(0.2f, Montage);
        }
        else
        {
            AnimInstance->StopAllMontages(0.2f);
        }
        UE_LOG(LogTemp, Log, TEXT("AnimationStateManager: Stopped montage"));
    }
}

bool UAnimationStateManager::IsPlayingMontage() const
{
    if (!SkeletalMeshComponent)
    {
        return false;
    }
    
    if (UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance())
    {
        return AnimInstance->IsAnyMontagePlaying();
    }
    
    return false;
}

void UAnimationStateManager::UpdateMovementData(float Speed, float Direction, bool bInAir, const FVector& Velocity)
{
    MovementData.Speed = Speed;
    MovementData.Direction = Direction;
    MovementData.bIsInAir = bInAir;
    MovementData.bIsMoving = Speed > 10.0f;
    MovementData.Velocity = Velocity;
}

void UAnimationStateManager::SetGroundDistance(float Distance)
{
    MovementData.GroundDistance = Distance;
}

void UAnimationStateManager::EnableTerrainAdaptation(bool bEnable)
{
    bTerrainAdaptationEnabled = bEnable;
}

bool UAnimationStateManager::CanTransitionToState(EAnim_MovementState NewState) const
{
    // Check if we're in a transition that can't be interrupted
    if (bIsTransitioning)
    {
        for (const FAnim_StateTransition& Trans : StateTransitions)
        {
            if (Trans.FromState == PreviousMovementState && Trans.ToState == CurrentMovementState)
            {
                return Trans.bCanInterrupt;
            }
        }
        return false; // Default to non-interruptible if no transition found
    }
    
    return true;
}

void UAnimationStateManager::InitializeDefaultTransitions()
{
    StateTransitions.Empty();
    
    // Define common transitions
    TArray<FAnim_StateTransition> DefaultTransitions = {
        // From Idle
        {EAnim_MovementState::Idle, EAnim_MovementState::Walking, 0.2f, true},
        {EAnim_MovementState::Idle, EAnim_MovementState::Running, 0.3f, true},
        {EAnim_MovementState::Idle, EAnim_MovementState::Jumping, 0.1f, false},
        {EAnim_MovementState::Idle, EAnim_MovementState::Crouching, 0.3f, true},
        
        // From Walking
        {EAnim_MovementState::Walking, EAnim_MovementState::Idle, 0.2f, true},
        {EAnim_MovementState::Walking, EAnim_MovementState::Running, 0.2f, true},
        {EAnim_MovementState::Walking, EAnim_MovementState::Jumping, 0.1f, false},
        
        // From Running
        {EAnim_MovementState::Running, EAnim_MovementState::Walking, 0.2f, true},
        {EAnim_MovementState::Running, EAnim_MovementState::Sprinting, 0.2f, true},
        {EAnim_MovementState::Running, EAnim_MovementState::Jumping, 0.1f, false},
        
        // From Jumping
        {EAnim_MovementState::Jumping, EAnim_MovementState::Falling, 0.1f, false},
        
        // From Falling
        {EAnim_MovementState::Falling, EAnim_MovementState::Landing, 0.1f, false},
        
        // From Landing
        {EAnim_MovementState::Landing, EAnim_MovementState::Idle, 0.3f, false},
        {EAnim_MovementState::Landing, EAnim_MovementState::Walking, 0.3f, false},
    };
    
    StateTransitions = DefaultTransitions;
    
    UE_LOG(LogTemp, Log, TEXT("AnimationStateManager: Initialized %d default state transitions"), StateTransitions.Num());
}

void UAnimationStateManager::UpdateAnimationBlending(float DeltaTime)
{
    if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetAnimInstance())
    {
        return;
    }
    
    // This would typically update blend space parameters
    // For now, we'll just log the current state for debugging
    if (GEngine && GetWorld() && GetWorld()->GetTimeSeconds() - GetWorld()->GetDeltaSeconds() < 1.0f)
    {
        // Only log occasionally to avoid spam
        static float LastLogTime = 0.0f;
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastLogTime > 2.0f)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("AnimationStateManager: Current state %d, Speed %.1f, InAir %s"), 
                   (int32)CurrentMovementState, MovementData.Speed, MovementData.bIsInAir ? TEXT("Yes") : TEXT("No"));
            LastLogTime = CurrentTime;
        }
    }
}