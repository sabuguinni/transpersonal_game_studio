#include "PrimitiveAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UPrimitiveAnimationController::UPrimitiveAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default thresholds
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    IdleSpeedThreshold = 5.0f;
    
    // Initialize states
    CurrentMovementState = EAnim_MovementState::Idle;
    PreviousMovementState = EAnim_MovementState::Idle;
    CurrentActionState = EAnim_ActionState::None;
    
    StateChangeTimer = 0.0f;
    bIsTransitioning = false;
}

void UPrimitiveAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get component references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
        
        if (MeshComponent)
        {
            AnimInstance = MeshComponent->GetAnimInstance();
        }
    }
    
    // Initialize state transitions
    InitializeStateTransitions();
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController initialized for %s"), 
           OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));
}

void UPrimitiveAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update movement data every frame
    UpdateMovementData();
    
    // Update animation state based on movement
    UpdateAnimationState();
    
    // Handle state transition timing
    if (bIsTransitioning)
    {
        StateChangeTimer += DeltaTime;
        if (StateChangeTimer >= 0.2f) // Default transition time
        {
            bIsTransitioning = false;
            StateChangeTimer = 0.0f;
        }
    }
}

void UPrimitiveAnimationController::UpdateMovementData()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Get current velocity
    FVector CurrentVelocity = MovementComponent->Velocity;
    MovementData.Velocity = CurrentVelocity;
    
    // Calculate speed (magnitude of horizontal velocity)
    FVector HorizontalVelocity = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);
    MovementData.Speed = HorizontalVelocity.Size();
    MovementData.GroundSpeed = MovementData.Speed;
    
    // Calculate movement direction relative to character forward
    if (OwnerCharacter && MovementData.Speed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector NormalizedVelocity = HorizontalVelocity.GetSafeNormal();
        
        float DotProduct = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float CrossProduct = FVector::CrossProduct(ForwardVector, NormalizedVelocity).Z;
        
        MovementData.Direction = UKismetMathLibrary::DegAtan2(CrossProduct, DotProduct);
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update movement flags
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
}

void UPrimitiveAnimationController::UpdateAnimationState()
{
    EAnim_MovementState NewState = CurrentMovementState;
    
    // Determine new state based on movement data
    if (MovementData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
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
    else if (MovementData.Speed <= IdleSpeedThreshold)
    {
        NewState = EAnim_MovementState::Idle;
    }
    else if (MovementData.Speed <= WalkSpeedThreshold)
    {
        NewState = EAnim_MovementState::Walking;
    }
    else if (MovementData.Speed > RunSpeedThreshold)
    {
        NewState = EAnim_MovementState::Running;
    }
    else
    {
        NewState = EAnim_MovementState::Walking;
    }
    
    // Apply state change if different and valid
    if (NewState != CurrentMovementState && CanTransitionTo(NewState))
    {
        SetMovementState(NewState);
    }
}

void UPrimitiveAnimationController::SetMovementState(EAnim_MovementState NewState)
{
    if (NewState == CurrentMovementState)
    {
        return;
    }
    
    if (!CanTransitionTo(NewState))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid state transition from %d to %d"), 
               (int32)CurrentMovementState, (int32)NewState);
        return;
    }
    
    PreviousMovementState = CurrentMovementState;
    CurrentMovementState = NewState;
    bIsTransitioning = true;
    StateChangeTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Animation state changed: %d -> %d"), 
           (int32)PreviousMovementState, (int32)CurrentMovementState);
}

void UPrimitiveAnimationController::SetActionState(EAnim_ActionState NewState)
{
    if (NewState != CurrentActionState)
    {
        CurrentActionState = NewState;
        UE_LOG(LogTemp, Log, TEXT("Action state changed to: %d"), (int32)CurrentActionState);
    }
}

void UPrimitiveAnimationController::UpdateMovementState()
{
    UpdateAnimationState();
}

void UPrimitiveAnimationController::TriggerJump()
{
    if (CanTransitionTo(EAnim_MovementState::Jumping))
    {
        SetMovementState(EAnim_MovementState::Jumping);
        UE_LOG(LogTemp, Log, TEXT("Jump animation triggered"));
    }
}

void UPrimitiveAnimationController::TriggerLand()
{
    if (CurrentMovementState == EAnim_MovementState::Falling || 
        CurrentMovementState == EAnim_MovementState::Jumping)
    {
        // Determine landing state based on current speed
        EAnim_MovementState LandingState = MovementData.Speed > IdleSpeedThreshold ? 
            EAnim_MovementState::Walking : EAnim_MovementState::Idle;
        
        SetMovementState(LandingState);
        UE_LOG(LogTemp, Log, TEXT("Land animation triggered, transitioning to state: %d"), (int32)LandingState);
    }
}

void UPrimitiveAnimationController::TriggerGatherAction()
{
    SetActionState(EAnim_ActionState::Gathering);
    UE_LOG(LogTemp, Log, TEXT("Gather action triggered"));
}

void UPrimitiveAnimationController::TriggerCombatAction()
{
    SetActionState(EAnim_ActionState::Combat);
    UE_LOG(LogTemp, Log, TEXT("Combat action triggered"));
}

void UPrimitiveAnimationController::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!AnimInstance || !Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play montage - missing AnimInstance or Montage"));
        return;
    }
    
    float Duration = AnimInstance->Montage_Play(Montage, PlayRate);
    if (Duration > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Playing montage: %s (Duration: %f)"), *Montage->GetName(), Duration);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to play montage: %s"), *Montage->GetName());
    }
}

void UPrimitiveAnimationController::StopMontage(UAnimMontage* Montage)
{
    if (!AnimInstance)
    {
        return;
    }
    
    if (Montage)
    {
        AnimInstance->Montage_Stop(0.2f, Montage);
        UE_LOG(LogTemp, Log, TEXT("Stopped specific montage: %s"), *Montage->GetName());
    }
    else
    {
        AnimInstance->StopAllMontages(0.2f);
        UE_LOG(LogTemp, Log, TEXT("Stopped all montages"));
    }
}

bool UPrimitiveAnimationController::CanTransitionTo(EAnim_MovementState NewState) const
{
    // Check if we're in a transition period
    if (bIsTransitioning && StateChangeTimer < 0.1f)
    {
        return false;
    }
    
    // Validate transition using rules
    return ValidateStateTransition(CurrentMovementState, NewState);
}

void UPrimitiveAnimationController::AddStateTransition(const FAnim_StateTransition& Transition)
{
    StateTransitions.Add(Transition);
    UE_LOG(LogTemp, Log, TEXT("Added state transition: %d -> %d"), 
           (int32)Transition.FromState, (int32)Transition.ToState);
}

void UPrimitiveAnimationController::InitializeStateTransitions()
{
    StateTransitions.Empty();
    
    // Define basic state transitions
    FAnim_StateTransition IdleToWalk;
    IdleToWalk.FromState = EAnim_MovementState::Idle;
    IdleToWalk.ToState = EAnim_MovementState::Walking;
    IdleToWalk.TransitionDuration = 0.2f;
    IdleToWalk.bCanInterrupt = true;
    StateTransitions.Add(IdleToWalk);
    
    FAnim_StateTransition WalkToRun;
    WalkToRun.FromState = EAnim_MovementState::Walking;
    WalkToRun.ToState = EAnim_MovementState::Running;
    WalkToRun.TransitionDuration = 0.3f;
    WalkToRun.bCanInterrupt = true;
    StateTransitions.Add(WalkToRun);
    
    FAnim_StateTransition RunToWalk;
    RunToWalk.FromState = EAnim_MovementState::Running;
    RunToWalk.ToState = EAnim_MovementState::Walking;
    RunToWalk.TransitionDuration = 0.2f;
    RunToWalk.bCanInterrupt = true;
    StateTransitions.Add(RunToWalk);
    
    FAnim_StateTransition WalkToIdle;
    WalkToIdle.FromState = EAnim_MovementState::Walking;
    WalkToIdle.ToState = EAnim_MovementState::Idle;
    WalkToIdle.TransitionDuration = 0.2f;
    WalkToIdle.bCanInterrupt = true;
    StateTransitions.Add(WalkToIdle);
    
    // Jump transitions
    FAnim_StateTransition AnyToJump;
    AnyToJump.ToState = EAnim_MovementState::Jumping;
    AnyToJump.TransitionDuration = 0.1f;
    AnyToJump.bCanInterrupt = false;
    StateTransitions.Add(AnyToJump);
    
    FAnim_StateTransition JumpToFall;
    JumpToFall.FromState = EAnim_MovementState::Jumping;
    JumpToFall.ToState = EAnim_MovementState::Falling;
    JumpToFall.TransitionDuration = 0.1f;
    JumpToFall.bCanInterrupt = false;
    StateTransitions.Add(JumpToFall);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d state transitions"), StateTransitions.Num());
}

bool UPrimitiveAnimationController::ValidateStateTransition(EAnim_MovementState FromState, EAnim_MovementState ToState) const
{
    // Always allow transitions to the same state
    if (FromState == ToState)
    {
        return true;
    }
    
    // Check if we have a specific rule for this transition
    for (const FAnim_StateTransition& Transition : StateTransitions)
    {
        if (Transition.FromState == FromState && Transition.ToState == ToState)
        {
            return true;
        }
        
        // Special case: "any state" transitions (FromState not specified means any)
        if (Transition.ToState == ToState && 
            (Transition.FromState == EAnim_MovementState::Idle && ToState == EAnim_MovementState::Jumping))
        {
            return true;
        }
    }
    
    // Default allowed transitions (fallback)
    switch (FromState)
    {
        case EAnim_MovementState::Idle:
            return ToState == EAnim_MovementState::Walking || 
                   ToState == EAnim_MovementState::Jumping ||
                   ToState == EAnim_MovementState::Crouching;
            
        case EAnim_MovementState::Walking:
            return ToState == EAnim_MovementState::Idle || 
                   ToState == EAnim_MovementState::Running ||
                   ToState == EAnim_MovementState::Jumping ||
                   ToState == EAnim_MovementState::Crouching;
            
        case EAnim_MovementState::Running:
            return ToState == EAnim_MovementState::Walking || 
                   ToState == EAnim_MovementState::Jumping ||
                   ToState == EAnim_MovementState::Falling;
            
        case EAnim_MovementState::Jumping:
            return ToState == EAnim_MovementState::Falling;
            
        case EAnim_MovementState::Falling:
            return ToState == EAnim_MovementState::Idle || 
                   ToState == EAnim_MovementState::Walking ||
                   ToState == EAnim_MovementState::Running;
            
        case EAnim_MovementState::Crouching:
            return ToState == EAnim_MovementState::Idle || 
                   ToState == EAnim_MovementState::Walking;
            
        default:
            return true; // Allow unknown transitions
    }
}