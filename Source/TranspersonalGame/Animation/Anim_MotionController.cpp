#include "Anim_MotionController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionController::UAnim_MotionController()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default state
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentActionState = EAnim_ActionState::None;
    
    // Default thresholds
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 300.0f;
    SprintSpeedThreshold = 600.0f;
    StateTransitionSpeed = 10.0f;
    
    MovementComponent = nullptr;
    AnimInstance = nullptr;
}

void UAnim_MotionController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get movement component
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        MovementComponent = Character->GetCharacterMovement();
        
        // Get animation instance
        if (USkeletalMeshComponent* Mesh = Character->GetMesh())
        {
            AnimInstance = Mesh->GetAnimInstance();
        }
    }
}

void UAnim_MotionController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateMovementData();
    UpdateMovementState();
}

void UAnim_MotionController::UpdateMovementData()
{
    if (!MovementComponent)
        return;
    
    // Get current velocity and speed
    MovementData.Velocity = MovementComponent->Velocity;
    MovementData.Speed = MovementData.Velocity.Size();
    
    // Calculate movement direction relative to actor
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector RightVector = GetOwner()->GetActorRightVector();
    
    FVector NormalizedVelocity = MovementData.Velocity.GetSafeNormal();
    float ForwardDot = FVector::DotProduct(NormalizedVelocity, ForwardVector);
    float RightDot = FVector::DotProduct(NormalizedVelocity, RightVector);
    
    MovementData.Direction = FMath::Atan2(RightDot, ForwardDot) * (180.0f / PI);
    
    // Update movement flags
    MovementData.bIsMoving = MovementData.Speed > 10.0f;
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Get acceleration
    MovementData.Acceleration = MovementComponent->GetCurrentAcceleration();
}

void UAnim_MotionController::UpdateMovementState()
{
    EAnim_MovementState NewState = DetermineMovementState();
    
    if (NewState != CurrentMovementState)
    {
        TransitionToState(NewState);
    }
}

EAnim_MovementState UAnim_MotionController::DetermineMovementState()
{
    // Check for air states first
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > 0)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Check for crouching
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Check movement speed
    if (!MovementData.bIsMoving)
    {
        return EAnim_MovementState::Idle;
    }
    
    if (MovementData.Speed < WalkSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else if (MovementData.Speed < RunSpeedThreshold)
    {
        return EAnim_MovementState::Running;
    }
    else if (MovementData.Speed < SprintSpeedThreshold)
    {
        return EAnim_MovementState::Sprinting;
    }
    
    return EAnim_MovementState::Running;
}

void UAnim_MotionController::TransitionToState(EAnim_MovementState NewState)
{
    EAnim_MovementState PreviousState = CurrentMovementState;
    CurrentMovementState = NewState;
    
    // Handle special transitions
    if (PreviousState == EAnim_MovementState::Falling && 
        (NewState == EAnim_MovementState::Idle || NewState == EAnim_MovementState::Walking))
    {
        TriggerLanding();
    }
}

void UAnim_MotionController::SetActionState(EAnim_ActionState NewActionState)
{
    CurrentActionState = NewActionState;
}

void UAnim_MotionController::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (AnimInstance && Montage)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
    }
}

void UAnim_MotionController::StopMontage(UAnimMontage* Montage)
{
    if (AnimInstance && Montage)
    {
        AnimInstance->Montage_Stop(0.2f, Montage);
    }
}

bool UAnim_MotionController::IsPlayingMontage(UAnimMontage* Montage)
{
    if (AnimInstance && Montage)
    {
        return AnimInstance->Montage_IsPlaying(Montage);
    }
    return false;
}

void UAnim_MotionController::TriggerJump()
{
    if (JumpMontage)
    {
        PlayMontage(JumpMontage);
    }
}

void UAnim_MotionController::TriggerLanding()
{
    if (LandMontage)
    {
        PlayMontage(LandMontage);
    }
}

void UAnim_MotionController::TriggerAttack()
{
    if (AttackMontage)
    {
        SetActionState(EAnim_ActionState::Attacking);
        PlayMontage(AttackMontage);
    }
}

void UAnim_MotionController::TriggerCrafting()
{
    if (CraftingMontage)
    {
        SetActionState(EAnim_ActionState::Crafting);
        PlayMontage(CraftingMontage);
    }
}