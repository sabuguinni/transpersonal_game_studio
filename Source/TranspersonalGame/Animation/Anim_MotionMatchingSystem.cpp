#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize state
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentActionState = EAnim_ActionState::None;
    PreviousMovementState = EAnim_MovementState::Idle;
    PreviousActionState = EAnim_ActionState::None;

    // Initialize transition data
    bIsTransitioning = false;
    TransitionAlpha = 0.0f;
    TransitionTime = 0.0f;
    MaxTransitionTime = 0.2f;

    // Initialize motion thresholds
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    SprintSpeedThreshold = 600.0f;
    MovementStopThreshold = 10.0f;

    // Initialize cached references
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MeshComponent = nullptr;

    // Initialize animation assets to null
    MovementBlendSpace = nullptr;
    JumpMontage = nullptr;
    LandMontage = nullptr;
    GatherMontage = nullptr;
    CraftMontage = nullptr;

    StateChangeTime = 0.0f;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    CacheComponentReferences();
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (OwnerCharacter && MovementComponent)
    {
        UpdateMotionData(DeltaTime);
        UpdateTransition(DeltaTime);
    }
}

void UAnim_MotionMatchingSystem::UpdateMotionData(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }

    // Update basic motion data
    CurrentMotionData.Velocity = MovementComponent->Velocity;
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    CurrentMotionData.Acceleration = MovementComponent->GetCurrentAcceleration();
    CurrentMotionData.bIsInAir = MovementComponent->IsFalling();

    // Calculate movement direction relative to character forward
    if (CurrentMotionData.Speed > MovementStopThreshold)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = CurrentMotionData.Velocity.GetSafeNormal();
        CurrentMotionData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
        // Determine if moving left or right
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0.0f)
        {
            CurrentMotionData.Direction *= -1.0f;
        }

        CurrentMotionData.bIsMoving = true;
        CurrentMotionData.TimeSinceLastMovement = 0.0f;
    }
    else
    {
        CurrentMotionData.Direction = 0.0f;
        CurrentMotionData.bIsMoving = false;
        CurrentMotionData.TimeSinceLastMovement += DeltaTime;
    }

    // Update movement state based on motion data
    EAnim_MovementState NewMovementState = CalculateMovementState();
    if (NewMovementState != CurrentMovementState)
    {
        OnMovementStateChanged(NewMovementState);
    }
}

EAnim_MovementState UAnim_MotionMatchingSystem::CalculateMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }

    // Check if character is in air
    if (CurrentMotionData.bIsInAir)
    {
        if (CurrentMotionData.Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }

    // Check if character is crouching
    if (MovementComponent->IsCrouching())
    {
        if (CurrentMotionData.bIsMoving)
        {
            return EAnim_MovementState::Crawling;
        }
        else
        {
            return EAnim_MovementState::Crouching;
        }
    }

    // Determine movement state based on speed
    if (CurrentMotionData.Speed < MovementStopThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (CurrentMotionData.Speed < WalkSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else if (CurrentMotionData.Speed < RunSpeedThreshold)
    {
        return EAnim_MovementState::Running;
    }
    else
    {
        return EAnim_MovementState::Sprinting;
    }
}

void UAnim_MotionMatchingSystem::UpdateTransition(float DeltaTime)
{
    if (bIsTransitioning)
    {
        TransitionTime += DeltaTime;
        TransitionAlpha = FMath::Clamp(TransitionTime / MaxTransitionTime, 0.0f, 1.0f);

        if (TransitionAlpha >= 1.0f)
        {
            bIsTransitioning = false;
            TransitionAlpha = 0.0f;
            TransitionTime = 0.0f;
        }
    }
}

void UAnim_MotionMatchingSystem::OnMovementStateChanged(EAnim_MovementState NewState)
{
    PreviousMovementState = CurrentMovementState;
    CurrentMovementState = NewState;
    StateChangeTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    // Start transition
    bIsTransitioning = true;
    TransitionTime = 0.0f;
    TransitionAlpha = 0.0f;

    // Handle special state transitions
    if (NewState == EAnim_MovementState::Jumping && JumpMontage)
    {
        PlayActionMontage(JumpMontage);
    }
    else if (PreviousMovementState == EAnim_MovementState::Falling && 
             NewState != EAnim_MovementState::Falling && 
             NewState != EAnim_MovementState::Jumping && 
             LandMontage)
    {
        PlayActionMontage(LandMontage);
    }
}

void UAnim_MotionMatchingSystem::OnActionStateChanged(EAnim_ActionState NewState)
{
    PreviousActionState = CurrentActionState;
    CurrentActionState = NewState;

    // Play appropriate montage for action state
    switch (NewState)
    {
        case EAnim_ActionState::Gathering:
            if (GatherMontage)
            {
                PlayActionMontage(GatherMontage);
            }
            break;
        case EAnim_ActionState::Crafting:
            if (CraftMontage)
            {
                PlayActionMontage(CraftMontage);
            }
            break;
        default:
            break;
    }
}

void UAnim_MotionMatchingSystem::CacheComponentReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
    }
}

EAnim_MovementState UAnim_MotionMatchingSystem::GetCurrentMovementState() const
{
    return CurrentMovementState;
}

EAnim_ActionState UAnim_MotionMatchingSystem::GetCurrentActionState() const
{
    return CurrentActionState;
}

FAnim_MotionData UAnim_MotionMatchingSystem::GetMotionData() const
{
    return CurrentMotionData;
}

void UAnim_MotionMatchingSystem::SetActionState(EAnim_ActionState NewState)
{
    if (NewState != CurrentActionState)
    {
        OnActionStateChanged(NewState);
    }
}

bool UAnim_MotionMatchingSystem::IsTransitioning() const
{
    return bIsTransitioning;
}

float UAnim_MotionMatchingSystem::GetTransitionAlpha() const
{
    return TransitionAlpha;
}

void UAnim_MotionMatchingSystem::BlendToMovementState(EAnim_MovementState NewState, float BlendTime)
{
    if (NewState != CurrentMovementState)
    {
        MaxTransitionTime = BlendTime;
        OnMovementStateChanged(NewState);
    }
}

void UAnim_MotionMatchingSystem::PlayActionMontage(UAnimMontage* Montage, float PlayRate)
{
    if (Montage && MeshComponent)
    {
        UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(Montage, PlayRate);
        }
    }
}