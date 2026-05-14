#include "Anim_StateManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_StateManager::UAnim_StateManager()
{
    // Initialize default values
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentEmotionalState = EAnim_EmotionalState::Calm;
    
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsAccelerating = false;
    
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    
    // Animation thresholds
    IdleThreshold = 10.0f;
    WalkThreshold = 150.0f;
    RunThreshold = 300.0f;
    
    // Play rates
    IdlePlayRate = 1.0f;
    WalkPlayRate = 1.0f;
    RunPlayRate = 1.0f;
    
    // State tracking
    StateTimer = 0.0f;
    PreviousMovementState = EAnim_MovementState::Idle;
    TransitionTimer = 0.0f;
    bIsTransitioning = false;
    
    // Setup default transitions
    StateTransitions.Empty();
    
    // Idle transitions
    FAnim_StateTransition IdleToWalk;
    IdleToWalk.FromState = EAnim_MovementState::Idle;
    IdleToWalk.ToState = EAnim_MovementState::Walking;
    IdleToWalk.BlendTime = 0.2f;
    IdleToWalk.bRequiresGrounded = true;
    StateTransitions.Add(IdleToWalk);
    
    // Walk transitions
    FAnim_StateTransition WalkToRun;
    WalkToRun.FromState = EAnim_MovementState::Walking;
    WalkToRun.ToState = EAnim_MovementState::Running;
    WalkToRun.BlendTime = 0.3f;
    WalkToRun.bRequiresGrounded = true;
    StateTransitions.Add(WalkToRun);
    
    FAnim_StateTransition WalkToIdle;
    WalkToIdle.FromState = EAnim_MovementState::Walking;
    WalkToIdle.ToState = EAnim_MovementState::Idle;
    WalkToIdle.BlendTime = 0.2f;
    WalkToIdle.bRequiresGrounded = true;
    StateTransitions.Add(WalkToIdle);
    
    // Run transitions
    FAnim_StateTransition RunToWalk;
    RunToWalk.FromState = EAnim_MovementState::Running;
    RunToWalk.ToState = EAnim_MovementState::Walking;
    RunToWalk.BlendTime = 0.2f;
    RunToWalk.bRequiresGrounded = true;
    StateTransitions.Add(RunToWalk);
    
    // Jump transitions
    FAnim_StateTransition GroundToJump;
    GroundToJump.FromState = EAnim_MovementState::Idle;
    GroundToJump.ToState = EAnim_MovementState::Jumping;
    GroundToJump.BlendTime = 0.1f;
    GroundToJump.bRequiresGrounded = false;
    StateTransitions.Add(GroundToJump);
    
    FAnim_StateTransition JumpToFall;
    JumpToFall.FromState = EAnim_MovementState::Jumping;
    JumpToFall.ToState = EAnim_MovementState::Falling;
    JumpToFall.BlendTime = 0.1f;
    JumpToFall.bRequiresGrounded = false;
    StateTransitions.Add(JumpToFall);
}

void UAnim_StateManager::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UAnim_StateManager::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Calculate movement parameters
    CalculateMovementParameters();
    
    // Update animation states
    UpdateMovementState(DeltaTime);
    UpdateEmotionalState(DeltaTime);
    
    // Update timers
    UpdateStateTimer(DeltaTime);
}

void UAnim_StateManager::CalculateMovementParameters()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();
    
    // Calculate direction relative to actor forward
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector RightVector = OwnerCharacter->GetActorRightVector();
    
    FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
    
    float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
    float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
    
    Direction = FMath::Atan2(RightDot, ForwardDot) * (180.0f / PI);
    
    // Update movement flags
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;
    
    // Calculate play rates based on speed
    if (Speed > 0.1f)
    {
        WalkPlayRate = FMath::Clamp(Speed / WalkThreshold, 0.5f, 2.0f);
        RunPlayRate = FMath::Clamp(Speed / RunThreshold, 0.5f, 2.0f);
    }
    else
    {
        WalkPlayRate = 1.0f;
        RunPlayRate = 1.0f;
    }
}

void UAnim_StateManager::UpdateMovementState(float DeltaTime)
{
    EAnim_MovementState NewState = DetermineMovementState();
    
    if (NewState != CurrentMovementState && CanTransitionTo(NewState))
    {
        HandleStateTransition(NewState);
    }
}

EAnim_MovementState UAnim_StateManager::DetermineMovementState()
{
    // Handle airborne states first
    if (bIsInAir)
    {
        if (MovementComponent && MovementComponent->Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Handle crouching
    if (bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Handle ground movement based on speed
    if (Speed < IdleThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (Speed < RunThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

bool UAnim_StateManager::CanTransitionTo(EAnim_MovementState NewState)
{
    // Find transition rule
    for (const FAnim_StateTransition& Transition : StateTransitions)
    {
        if (Transition.FromState == CurrentMovementState && Transition.ToState == NewState)
        {
            // Check if grounded requirement is met
            if (Transition.bRequiresGrounded && bIsInAir)
            {
                return false;
            }
            return true;
        }
    }
    
    // Allow any transition if no specific rule found (fallback)
    return true;
}

void UAnim_StateManager::HandleStateTransition(EAnim_MovementState NewState)
{
    PreviousMovementState = CurrentMovementState;
    CurrentMovementState = NewState;
    StateTimer = 0.0f;
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
    
    // Log state change for debugging
    UE_LOG(LogTemp, Log, TEXT("Animation State Changed: %d -> %d"), 
           (int32)PreviousMovementState, (int32)CurrentMovementState);
}

void UAnim_StateManager::UpdateEmotionalState(float DeltaTime)
{
    // Basic emotional state logic based on movement and environment
    // This can be expanded with more complex AI and survival mechanics
    
    if (Speed > RunThreshold && bIsAccelerating)
    {
        // Running fast might indicate urgency or fear
        if (CurrentEmotionalState != EAnim_EmotionalState::Alert)
        {
            CurrentEmotionalState = EAnim_EmotionalState::Alert;
        }
    }
    else if (Speed < IdleThreshold && StateTimer > 5.0f)
    {
        // Standing still for a while - relaxed
        if (CurrentEmotionalState != EAnim_EmotionalState::Relaxed)
        {
            CurrentEmotionalState = EAnim_EmotionalState::Relaxed;
        }
    }
    else
    {
        // Default calm state
        if (CurrentEmotionalState != EAnim_EmotionalState::Calm)
        {
            CurrentEmotionalState = EAnim_EmotionalState::Calm;
        }
    }
}

void UAnim_StateManager::UpdateStateTimer(float DeltaTime)
{
    StateTimer += DeltaTime;
    
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        
        // Find transition blend time
        float BlendTime = 0.2f; // Default
        for (const FAnim_StateTransition& Transition : StateTransitions)
        {
            if (Transition.FromState == PreviousMovementState && 
                Transition.ToState == CurrentMovementState)
            {
                BlendTime = Transition.BlendTime;
                break;
            }
        }
        
        if (TransitionTimer >= BlendTime)
        {
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
        }
    }
}

void UAnim_StateManager::ForceStateTransition(EAnim_MovementState NewState)
{
    HandleStateTransition(NewState);
}

float UAnim_StateManager::GetStateBlendWeight(EAnim_MovementState State) const
{
    if (State == CurrentMovementState)
    {
        if (bIsTransitioning)
        {
            // Calculate blend weight during transition
            float BlendTime = 0.2f;
            for (const FAnim_StateTransition& Transition : StateTransitions)
            {
                if (Transition.FromState == PreviousMovementState && 
                    Transition.ToState == CurrentMovementState)
                {
                    BlendTime = Transition.BlendTime;
                    break;
                }
            }
            
            float Alpha = FMath::Clamp(TransitionTimer / BlendTime, 0.0f, 1.0f);
            return Alpha;
        }
        else
        {
            return 1.0f;
        }
    }
    else if (State == PreviousMovementState && bIsTransitioning)
    {
        // Calculate inverse blend weight for previous state
        float BlendTime = 0.2f;
        for (const FAnim_StateTransition& Transition : StateTransitions)
        {
            if (Transition.FromState == PreviousMovementState && 
                Transition.ToState == CurrentMovementState)
            {
                BlendTime = Transition.BlendTime;
                break;
            }
        }
        
        float Alpha = FMath::Clamp(TransitionTimer / BlendTime, 0.0f, 1.0f);
        return 1.0f - Alpha;
    }
    
    return 0.0f;
}