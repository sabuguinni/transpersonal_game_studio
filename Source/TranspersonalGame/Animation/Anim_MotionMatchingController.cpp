#include "Anim_MotionMatchingController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingController::UAnim_MotionMatchingController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    TransitionThreshold = 0.1f;
    BlendTime = 0.3f;
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    
    StateTransitionTimer = 0.0f;
    bIsTransitioning = false;
    PendingState = EAnim_LocomotionState::Idle;
    
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    AnimInstance = nullptr;
}

void UAnim_MotionMatchingController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeReferences();
    ValidateAnimationAssets();
    
    // Initialize motion data
    UpdateMotionData();
    CurrentMotionData.CurrentState = EAnim_LocomotionState::Idle;
}

void UAnim_MotionMatchingController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Store previous frame data
    PreviousMotionData = CurrentMotionData;
    
    // Update current motion data
    UpdateMotionData();
    
    // Determine target state
    EAnim_LocomotionState TargetState = DetermineLocomotionState();
    
    // Handle state transitions
    if (bIsTransitioning)
    {
        StateTransitionTimer += DeltaTime;
        if (StateTransitionTimer >= BlendTime)
        {
            bIsTransitioning = false;
            CurrentMotionData.CurrentState = PendingState;
            StateTransitionTimer = 0.0f;
        }
    }
    else if (ShouldTransitionToState(TargetState))
    {
        TransitionToState(TargetState);
    }
}

void UAnim_MotionMatchingController::UpdateMotionData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and speed
    CurrentMotionData.Velocity = MovementComponent->Velocity;
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    
    // Calculate direction relative to character forward
    CurrentMotionData.Direction = CalculateDirectionAngle();
    
    // Update movement flags
    CurrentMotionData.bIsInAir = MovementComponent->IsFalling();
    CurrentMotionData.bIsCrouching = MovementComponent->IsCrouching();
}

EAnim_LocomotionState UAnim_MotionMatchingController::DetermineLocomotionState()
{
    if (!MovementComponent)
    {
        return EAnim_LocomotionState::Idle;
    }
    
    // Check for airborne states first
    if (CurrentMotionData.bIsInAir)
    {
        if (CurrentMotionData.Velocity.Z > 0.0f)
        {
            return EAnim_LocomotionState::Jumping;
        }
        else
        {
            return EAnim_LocomotionState::Falling;
        }
    }
    
    // Check for crouching
    if (CurrentMotionData.bIsCrouching)
    {
        return EAnim_LocomotionState::Crouching;
    }
    
    // Determine ground locomotion state based on speed
    if (CurrentMotionData.Speed < TransitionThreshold)
    {
        return EAnim_LocomotionState::Idle;
    }
    else if (CurrentMotionData.Speed < WalkSpeedThreshold)
    {
        return EAnim_LocomotionState::Walking;
    }
    else if (CurrentMotionData.Speed < RunSpeedThreshold)
    {
        return EAnim_LocomotionState::Running;
    }
    else
    {
        return EAnim_LocomotionState::Running; // Max speed is still running
    }
}

void UAnim_MotionMatchingController::TransitionToState(EAnim_LocomotionState NewState)
{
    if (NewState == CurrentMotionData.CurrentState)
    {
        return;
    }
    
    if (!IsValidTransition(CurrentMotionData.CurrentState, NewState))
    {
        return;
    }
    
    // Start transition
    bIsTransitioning = true;
    PendingState = NewState;
    StateTransitionTimer = 0.0f;
    BlendTime = GetTransitionDuration(CurrentMotionData.CurrentState, NewState);
    
    // Play appropriate montage
    PlayMontageForState(NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Motion Matching: Transitioning from %d to %d"), 
           (int32)CurrentMotionData.CurrentState, (int32)NewState);
}

void UAnim_MotionMatchingController::PlayMontageForState(EAnim_LocomotionState State)
{
    if (!AnimInstance)
    {
        return;
    }
    
    UAnimMontage* MontageToPlay = nullptr;
    
    switch (State)
    {
        case EAnim_LocomotionState::Idle:
            MontageToPlay = MontageSet.IdleMontage;
            break;
        case EAnim_LocomotionState::Walking:
            MontageToPlay = MontageSet.WalkMontage;
            break;
        case EAnim_LocomotionState::Running:
            MontageToPlay = MontageSet.RunMontage;
            break;
        case EAnim_LocomotionState::Jumping:
        case EAnim_LocomotionState::Falling:
        case EAnim_LocomotionState::Landing:
            MontageToPlay = MontageSet.JumpMontage;
            break;
        default:
            break;
    }
    
    if (MontageToPlay && !AnimInstance->Montage_IsPlaying(MontageToPlay))
    {
        AnimInstance->Montage_Play(MontageToPlay, 1.0f);
    }
}

float UAnim_MotionMatchingController::CalculateDirectionAngle()
{
    if (!OwnerCharacter || CurrentMotionData.Speed < TransitionThreshold)
    {
        return 0.0f;
    }
    
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector VelocityDirection = CurrentMotionData.Velocity.GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
    
    return UKismetMathLibrary::Atan2(CrossProduct, DotProduct) * 180.0f / PI;
}

bool UAnim_MotionMatchingController::ShouldTransitionToState(EAnim_LocomotionState TargetState)
{
    if (TargetState == CurrentMotionData.CurrentState)
    {
        return false;
    }
    
    if (bIsTransitioning)
    {
        return false;
    }
    
    // Add hysteresis to prevent rapid state changes
    float SpeedDifference = FMath::Abs(CurrentMotionData.Speed - PreviousMotionData.Speed);
    if (SpeedDifference > TransitionThreshold * 2.0f)
    {
        return true;
    }
    
    // Check if we've been in current state long enough
    static float MinStateTime = 0.2f;
    if (StateTransitionTimer < MinStateTime)
    {
        return false;
    }
    
    return IsValidTransition(CurrentMotionData.CurrentState, TargetState);
}

bool UAnim_MotionMatchingController::IsInTransition() const
{
    return bIsTransitioning;
}

void UAnim_MotionMatchingController::SetMontageSet(const FAnim_MontageSet& NewMontageSet)
{
    MontageSet = NewMontageSet;
    ValidateAnimationAssets();
}

void UAnim_MotionMatchingController::InitializeWithCharacter(ACharacter* Character)
{
    OwnerCharacter = Character;
    InitializeReferences();
}

void UAnim_MotionMatchingController::InitializeReferences()
{
    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(GetOwner());
    }
    
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        if (USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh())
        {
            AnimInstance = MeshComp->GetAnimInstance();
        }
    }
}

void UAnim_MotionMatchingController::ValidateAnimationAssets()
{
    if (!MontageSet.IdleMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching: Idle montage not assigned"));
    }
    
    if (!MontageSet.WalkMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching: Walk montage not assigned"));
    }
    
    if (!MontageSet.RunMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching: Run montage not assigned"));
    }
    
    if (!MontageSet.JumpMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching: Jump montage not assigned"));
    }
    
    if (!MontageSet.LocomotionBlendSpace)
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching: Locomotion blend space not assigned"));
    }
}

bool UAnim_MotionMatchingController::IsValidTransition(EAnim_LocomotionState FromState, EAnim_LocomotionState ToState)
{
    // All transitions are valid for now - can be refined later
    return true;
}

float UAnim_MotionMatchingController::GetTransitionDuration(EAnim_LocomotionState FromState, EAnim_LocomotionState ToState)
{
    // Quick transitions for airborne states
    if (ToState == EAnim_LocomotionState::Jumping || ToState == EAnim_LocomotionState::Falling)
    {
        return 0.1f;
    }
    
    // Medium transitions for ground locomotion
    if ((FromState == EAnim_LocomotionState::Idle && ToState == EAnim_LocomotionState::Walking) ||
        (FromState == EAnim_LocomotionState::Walking && ToState == EAnim_LocomotionState::Idle))
    {
        return 0.2f;
    }
    
    // Longer transitions for speed changes
    if ((FromState == EAnim_LocomotionState::Walking && ToState == EAnim_LocomotionState::Running) ||
        (FromState == EAnim_LocomotionState::Running && ToState == EAnim_LocomotionState::Walking))
    {
        return 0.3f;
    }
    
    // Default blend time
    return BlendTime;
}