#include "Anim_CharacterAnimationController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

UAnim_CharacterAnimationController::UAnim_CharacterAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default states
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentCombatState = EAnim_CombatState::Unarmed;
    CurrentSurvivalAction = EAnim_SurvivalAction::None;
    
    bIsInCombat = false;
    bIsInjured = false;
    InjuryLevel = 0.0f;
    
    CurrentSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsMoving = false;
    bIsGrounded = true;
    
    DefaultTransitionTime = 0.2f;
    bIsTransitioning = false;
    TransitionTimer = 0.0f;
    
    PreviousState = EAnim_MovementState::Idle;
    TargetState = EAnim_MovementState::Idle;
    
    CharacterMesh = nullptr;
    AnimInstance = nullptr;
}

void UAnim_CharacterAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get character mesh component
    if (AActor* Owner = GetOwner())
    {
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            CharacterMesh = Character->GetMesh();
            if (CharacterMesh)
            {
                AnimInstance = CharacterMesh->GetAnimInstance();
            }
        }
    }
    
    // Initialize default state transitions
    if (StateTransitions.Num() == 0)
    {
        // Add common state transitions
        FAnim_StateTransition IdleToWalk;
        IdleToWalk.FromState = EAnim_MovementState::Idle;
        IdleToWalk.ToState = EAnim_MovementState::Walking;
        IdleToWalk.TransitionDuration = 0.15f;
        StateTransitions.Add(IdleToWalk);
        
        FAnim_StateTransition WalkToRun;
        WalkToRun.FromState = EAnim_MovementState::Walking;
        WalkToRun.ToState = EAnim_MovementState::Running;
        WalkToRun.TransitionDuration = 0.2f;
        StateTransitions.Add(WalkToRun);
        
        FAnim_StateTransition RunToSprint;
        RunToSprint.FromState = EAnim_MovementState::Running;
        RunToSprint.ToState = EAnim_MovementState::Sprinting;
        RunToSprint.TransitionDuration = 0.25f;
        StateTransitions.Add(RunToSprint);
        
        FAnim_StateTransition AnyToJump;
        AnyToJump.FromState = EAnim_MovementState::Idle; // Will be handled specially
        AnyToJump.ToState = EAnim_MovementState::Jumping;
        AnyToJump.TransitionDuration = 0.1f;
        StateTransitions.Add(AnyToJump);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation Controller initialized for %s"), *GetOwner()->GetName());
}

void UAnim_CharacterAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateAnimationState(DeltaTime);
}

void UAnim_CharacterAnimationController::UpdateAnimationState(float DeltaTime)
{
    if (!CharacterMesh || !AnimInstance)
        return;
    
    // Update transition timer
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        if (TransitionTimer >= GetTransitionDuration(PreviousState, TargetState))
        {
            CurrentMovementState = TargetState;
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
        }
    }
    
    // Update movement data from character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            FVector Velocity = MovementComp->Velocity;
            CurrentSpeed = Velocity.Size();
            bIsMoving = CurrentSpeed > 1.0f;
            bIsGrounded = MovementComp->IsMovingOnGround();
            
            // Calculate movement direction relative to character forward
            if (bIsMoving)
            {
                FVector ForwardVector = Character->GetActorForwardVector();
                FVector RightVector = Character->GetActorRightVector();
                
                FVector NormalizedVelocity = Velocity.GetSafeNormal();
                MovementDirection = FMath::Atan2(FVector::DotProduct(NormalizedVelocity, RightVector), 
                                               FVector::DotProduct(NormalizedVelocity, ForwardVector));
                MovementDirection = FMath::RadiansToDegrees(MovementDirection);
            }
            
            // Update blend space data
            LocomotionBlendSpace.SpeedX = FVector::DotProduct(Velocity, Character->GetActorForwardVector());
            LocomotionBlendSpace.SpeedY = FVector::DotProduct(Velocity, Character->GetActorRightVector());
            LocomotionBlendSpace.Direction = MovementDirection;
        }
    }
    
    // Auto-update movement state based on speed
    if (!bIsTransitioning && CurrentSurvivalAction == EAnim_SurvivalAction::None)
    {
        EAnim_MovementState NewState = CurrentMovementState;
        
        if (!bIsGrounded)
        {
            if (CurrentMovementState != EAnim_MovementState::Jumping && CurrentMovementState != EAnim_MovementState::Falling)
            {
                NewState = EAnim_MovementState::Falling;
            }
        }
        else if (CurrentSpeed < 1.0f)
        {
            NewState = EAnim_MovementState::Idle;
        }
        else if (CurrentSpeed < 200.0f)
        {
            NewState = EAnim_MovementState::Walking;
        }
        else if (CurrentSpeed < 400.0f)
        {
            NewState = EAnim_MovementState::Running;
        }
        else
        {
            NewState = EAnim_MovementState::Sprinting;
        }
        
        if (NewState != CurrentMovementState)
        {
            SetMovementState(NewState);
        }
    }
}

void UAnim_CharacterAnimationController::SetMovementState(EAnim_MovementState NewState)
{
    if (NewState == CurrentMovementState || !CanTransitionTo(NewState))
        return;
    
    ProcessStateTransition(NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Movement state changed from %d to %d"), 
           (int32)CurrentMovementState, (int32)NewState);
}

void UAnim_CharacterAnimationController::SetCombatState(EAnim_CombatState NewState)
{
    if (NewState == CurrentCombatState)
        return;
    
    CurrentCombatState = NewState;
    bIsInCombat = (NewState != EAnim_CombatState::Unarmed);
    
    // Play combat montage if available
    if (CombatMontages.Contains(NewState))
    {
        PlayCombatMontage(NewState);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Combat state changed to %d"), (int32)NewState);
}

void UAnim_CharacterAnimationController::PlaySurvivalAction(EAnim_SurvivalAction Action)
{
    if (Action == EAnim_SurvivalAction::None)
    {
        CurrentSurvivalAction = Action;
        return;
    }
    
    if (SurvivalActionMontages.Contains(Action))
    {
        UAnimMontage* Montage = SurvivalActionMontages[Action];
        if (Montage && AnimInstance)
        {
            AnimInstance->Montage_Play(Montage);
            CurrentSurvivalAction = Action;
            
            UE_LOG(LogTemp, Log, TEXT("Playing survival action montage: %d"), (int32)Action);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No montage found for survival action: %d"), (int32)Action);
    }
}

void UAnim_CharacterAnimationController::PlayCombatMontage(EAnim_CombatState CombatState)
{
    if (CombatMontages.Contains(CombatState))
    {
        UAnimMontage* Montage = CombatMontages[CombatState];
        if (Montage && AnimInstance)
        {
            AnimInstance->Montage_Play(Montage);
            UE_LOG(LogTemp, Log, TEXT("Playing combat montage: %d"), (int32)CombatState);
        }
    }
}

void UAnim_CharacterAnimationController::TriggerJump()
{
    if (JumpMontage && AnimInstance && bIsGrounded)
    {
        AnimInstance->Montage_Play(JumpMontage);
        SetMovementState(EAnim_MovementState::Jumping);
        UE_LOG(LogTemp, Log, TEXT("Jump animation triggered"));
    }
}

void UAnim_CharacterAnimationController::TriggerLanding()
{
    if (LandMontage && AnimInstance)
    {
        AnimInstance->Montage_Play(LandMontage);
        UE_LOG(LogTemp, Log, TEXT("Landing animation triggered"));
    }
}

void UAnim_CharacterAnimationController::TriggerDeath()
{
    if (DeathMontage && AnimInstance)
    {
        AnimInstance->Montage_Play(DeathMontage);
        SetMovementState(EAnim_MovementState::Idle);
        SetCombatState(EAnim_CombatState::Dead);
        UE_LOG(LogTemp, Log, TEXT("Death animation triggered"));
    }
}

void UAnim_CharacterAnimationController::UpdateMovementData(float Speed, float Direction, bool IsGrounded)
{
    CurrentSpeed = Speed;
    MovementDirection = Direction;
    bIsGrounded = IsGrounded;
    bIsMoving = Speed > 1.0f;
}

void UAnim_CharacterAnimationController::SetInjuryState(bool Injured, float Level)
{
    bIsInjured = Injured;
    InjuryLevel = FMath::Clamp(Level, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Injury state set: %s, Level: %f"), 
           Injured ? TEXT("True") : TEXT("False"), InjuryLevel);
}

bool UAnim_CharacterAnimationController::IsPlayingMontage() const
{
    if (AnimInstance)
    {
        return AnimInstance->IsAnyMontagePlaying();
    }
    return false;
}

float UAnim_CharacterAnimationController::GetCurrentMontagePosition() const
{
    if (AnimInstance && IsPlayingMontage())
    {
        return AnimInstance->Montage_GetPosition(AnimInstance->GetCurrentActiveMontage());
    }
    return 0.0f;
}

void UAnim_CharacterAnimationController::StopCurrentMontage()
{
    if (AnimInstance && IsPlayingMontage())
    {
        AnimInstance->Montage_Stop(0.2f);
        CurrentSurvivalAction = EAnim_SurvivalAction::None;
        UE_LOG(LogTemp, Log, TEXT("Current montage stopped"));
    }
}

void UAnim_CharacterAnimationController::ProcessStateTransition(EAnim_MovementState NewState)
{
    if (bIsTransitioning)
        return;
    
    PreviousState = CurrentMovementState;
    TargetState = NewState;
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
    
    // Find and play transition montage if available
    for (const FAnim_StateTransition& Transition : StateTransitions)
    {
        if (Transition.FromState == PreviousState && Transition.ToState == TargetState)
        {
            if (Transition.TransitionMontage && AnimInstance)
            {
                AnimInstance->Montage_Play(Transition.TransitionMontage);
            }
            break;
        }
    }
}

bool UAnim_CharacterAnimationController::CanTransitionTo(EAnim_MovementState NewState) const
{
    // Always allow transition to idle or falling
    if (NewState == EAnim_MovementState::Idle || NewState == EAnim_MovementState::Falling)
        return true;
    
    // Don't allow transitions while playing survival actions
    if (CurrentSurvivalAction != EAnim_SurvivalAction::None)
        return false;
    
    // Don't allow transitions while in death state
    if (CurrentCombatState == EAnim_CombatState::Dead)
        return false;
    
    // Check for valid transition rules
    switch (CurrentMovementState)
    {
        case EAnim_MovementState::Jumping:
        case EAnim_MovementState::Falling:
            return (NewState == EAnim_MovementState::Landing || NewState == EAnim_MovementState::Idle);
        
        case EAnim_MovementState::Landing:
            return (NewState == EAnim_MovementState::Idle || NewState == EAnim_MovementState::Walking);
        
        default:
            return true;
    }
}

float UAnim_CharacterAnimationController::GetTransitionDuration(EAnim_MovementState FromState, EAnim_MovementState ToState) const
{
    for (const FAnim_StateTransition& Transition : StateTransitions)
    {
        if (Transition.FromState == FromState && Transition.ToState == ToState)
        {
            return Transition.TransitionDuration;
        }
    }
    return DefaultTransitionTime;
}