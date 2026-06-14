#include "Anim_CharacterStateManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

UAnim_CharacterStateManager::UAnim_CharacterStateManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    CurrentState = EAnim_CharacterState::Idle;
    PreviousState = EAnim_CharacterState::Idle;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    TransitionTimer = 0.0f;
    CurrentTransitionDuration = 0.2f;

    MovementSpeed = 0.0f;
    MovementDirection = FVector::ZeroVector;
    bGrounded = true;
    bFalling = false;
}

void UAnim_CharacterStateManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultTransitions();
}

void UAnim_CharacterStateManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateTransition(DeltaTime);

    // Auto-determine state based on movement if not in transition
    if (!bIsTransitioning)
    {
        EAnim_CharacterState NewState = DetermineStateFromMovement();
        if (NewState != CurrentState)
        {
            SetCharacterState(NewState);
        }
    }
}

void UAnim_CharacterStateManager::SetCharacterState(EAnim_CharacterState NewState, bool bForceTransition)
{
    if (NewState == CurrentState && !bForceTransition)
    {
        return;
    }

    if (!bForceTransition && !CanTransitionTo(NewState))
    {
        return;
    }

    PreviousState = CurrentState;
    CurrentState = NewState;

    // Find transition duration
    CurrentTransitionDuration = 0.2f; // Default
    for (const FAnim_StateTransition& Transition : StateTransitions)
    {
        if (Transition.FromState == PreviousState && Transition.ToState == CurrentState)
        {
            CurrentTransitionDuration = Transition.TransitionDuration;
            break;
        }
    }

    bIsTransitioning = true;
    TransitionTimer = 0.0f;
    TransitionProgress = 0.0f;
}

bool UAnim_CharacterStateManager::IsMoving() const
{
    return CurrentState == EAnim_CharacterState::Walking || 
           CurrentState == EAnim_CharacterState::Running ||
           CurrentState == EAnim_CharacterState::Climbing ||
           CurrentState == EAnim_CharacterState::Swimming;
}

bool UAnim_CharacterStateManager::IsGrounded() const
{
    return bGrounded && CurrentState != EAnim_CharacterState::Jumping && 
           CurrentState != EAnim_CharacterState::Falling;
}

bool UAnim_CharacterStateManager::IsInCombat() const
{
    return CurrentState == EAnim_CharacterState::Combat;
}

bool UAnim_CharacterStateManager::CanTransitionTo(EAnim_CharacterState TargetState) const
{
    if (bIsTransitioning)
    {
        // Check if current transition can be interrupted
        for (const FAnim_StateTransition& Transition : StateTransitions)
        {
            if (Transition.FromState == PreviousState && Transition.ToState == CurrentState)
            {
                return Transition.bCanInterrupt;
            }
        }
        return false;
    }

    return CanTransitionFromTo(CurrentState, TargetState);
}

void UAnim_CharacterStateManager::UpdateMovementData(float Speed, FVector Velocity, bool bIsGrounded, bool bIsFalling)
{
    MovementSpeed = Speed;
    MovementDirection = Velocity.GetSafeNormal();
    bGrounded = bIsGrounded;
    bFalling = bIsFalling;
}

void UAnim_CharacterStateManager::InitializeDefaultTransitions()
{
    StateTransitions.Empty();

    // Idle transitions
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Idle, EAnim_CharacterState::Walking, 0.2f, true});
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Idle, EAnim_CharacterState::Running, 0.3f, true});
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Idle, EAnim_CharacterState::Jumping, 0.1f, true});
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Idle, EAnim_CharacterState::Crouching, 0.3f, true});

    // Walking transitions
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Walking, EAnim_CharacterState::Idle, 0.2f, true});
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Walking, EAnim_CharacterState::Running, 0.2f, true});
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Walking, EAnim_CharacterState::Jumping, 0.1f, true});

    // Running transitions
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Running, EAnim_CharacterState::Walking, 0.2f, true});
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Running, EAnim_CharacterState::Idle, 0.3f, true});
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Running, EAnim_CharacterState::Jumping, 0.1f, true});

    // Jumping/Falling transitions
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Jumping, EAnim_CharacterState::Falling, 0.1f, false});
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Falling, EAnim_CharacterState::Landing, 0.1f, false});
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Landing, EAnim_CharacterState::Idle, 0.3f, false});

    // Combat transitions
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Combat, EAnim_CharacterState::Idle, 0.4f, true});
    StateTransitions.Add(FAnim_StateTransition{EAnim_CharacterState::Idle, EAnim_CharacterState::Combat, 0.2f, true});
}

bool UAnim_CharacterStateManager::CanTransitionFromTo(EAnim_CharacterState From, EAnim_CharacterState To) const
{
    for (const FAnim_StateTransition& Transition : StateTransitions)
    {
        if (Transition.FromState == From && Transition.ToState == To)
        {
            return true;
        }
    }
    return false;
}

void UAnim_CharacterStateManager::UpdateTransition(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }

    TransitionTimer += DeltaTime;
    TransitionProgress = FMath::Clamp(TransitionTimer / CurrentTransitionDuration, 0.0f, 1.0f);

    if (TransitionProgress >= 1.0f)
    {
        bIsTransitioning = false;
        TransitionTimer = 0.0f;
        TransitionProgress = 0.0f;
    }
}

EAnim_CharacterState UAnim_CharacterStateManager::DetermineStateFromMovement() const
{
    if (bFalling)
    {
        return EAnim_CharacterState::Falling;
    }

    if (!bGrounded)
    {
        return EAnim_CharacterState::Jumping;
    }

    if (MovementSpeed < 1.0f)
    {
        return EAnim_CharacterState::Idle;
    }
    else if (MovementSpeed < 300.0f)
    {
        return EAnim_CharacterState::Walking;
    }
    else
    {
        return EAnim_CharacterState::Running;
    }
}