#include "Anim_MotionMatchingController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_MotionMatchingController::UAnim_MotionMatchingController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize motion data
    CurrentMotionData = FAnim_MotionData();
    PreviousMotionData = FAnim_MotionData();
    
    // Set default thresholds
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    IdleSpeedThreshold = 10.0f;
    StateTransitionDelay = 0.1f;
    MotionSmoothingFactor = 5.0f;
}

void UAnim_MotionMatchingController::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponentReferences();
    
    if (OwnerCharacter)
    {
        UE_LOG(LogTemp, Log, TEXT("Motion Matching Controller initialized for: %s"), *OwnerCharacter->GetName());
    }
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
    
    // Smooth motion data for better transitions
    SmoothMotionData(DeltaTime);
    
    // Determine and transition to new state if needed
    EAnim_MotionState NewState = DetermineMotionState();
    if (NewState != CurrentMotionData.CurrentState)
    {
        TransitionToState(NewState);
    }
    
    // Process pending state transitions
    ProcessStateTransition(DeltaTime);
}

void UAnim_MotionMatchingController::UpdateMotionData()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Get velocity and speed
    FVector CurrentVelocity = MovementComponent->Velocity;
    CurrentMotionData.Velocity = CurrentVelocity;
    CurrentMotionData.Speed = CurrentVelocity.Size();
    
    // Calculate movement direction relative to character forward
    if (OwnerCharacter && CurrentMotionData.Speed > IdleSpeedThreshold)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = CurrentVelocity.GetSafeNormal();
        
        float ForwardDot = FVector::DotProduct(NormalizedVelocity, ForwardVector);
        float RightDot = FVector::DotProduct(NormalizedVelocity, RightVector);
        
        CurrentMotionData.Direction = UKismetMathLibrary::Atan2(RightDot, ForwardDot) * (180.0f / PI);
        CurrentMotionData.bIsMoving = true;
    }
    else
    {
        CurrentMotionData.Direction = 0.0f;
        CurrentMotionData.bIsMoving = false;
    }
    
    // Check ground state
    CurrentMotionData.bIsOnGround = MovementComponent->IsMovingOnGround();
}

EAnim_MotionState UAnim_MotionMatchingController::DetermineMotionState()
{
    if (!MovementComponent)
    {
        return EAnim_MotionState::Idle;
    }
    
    // Check if falling or jumping
    if (!CurrentMotionData.bIsOnGround)
    {
        if (CurrentMotionData.Velocity.Z > 50.0f)
        {
            return EAnim_MotionState::Jumping;
        }
        else if (CurrentMotionData.Velocity.Z < -50.0f)
        {
            return EAnim_MotionState::Falling;
        }
    }
    
    // Check if just landed
    if (CurrentMotionData.bIsOnGround && !PreviousMotionData.bIsOnGround)
    {
        return EAnim_MotionState::Landing;
    }
    
    // Check movement states based on speed
    if (CurrentMotionData.Speed <= IdleSpeedThreshold)
    {
        return EAnim_MotionState::Idle;
    }
    else if (CurrentMotionData.Speed <= WalkSpeedThreshold)
    {
        return EAnim_MotionState::Walking;
    }
    else if (CurrentMotionData.Speed <= RunSpeedThreshold)
    {
        return EAnim_MotionState::Running;
    }
    else
    {
        return EAnim_MotionState::Running; // Max speed is still running
    }
}

void UAnim_MotionMatchingController::TransitionToState(EAnim_MotionState NewState)
{
    if (ShouldTransitionToState(NewState))
    {
        PendingState = NewState;
        bHasPendingTransition = true;
        StateTransitionTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Motion state transition queued: %d -> %d"), 
               (int32)CurrentMotionData.CurrentState, (int32)NewState);
    }
}

void UAnim_MotionMatchingController::SetMotionState(EAnim_MotionState NewState)
{
    CurrentMotionData.CurrentState = NewState;
    bHasPendingTransition = false;
    
    UE_LOG(LogTemp, Log, TEXT("Motion state set to: %d"), (int32)NewState);
}

float UAnim_MotionMatchingController::GetMovementSpeed() const
{
    return CurrentMotionData.Speed;
}

float UAnim_MotionMatchingController::GetMovementDirection() const
{
    return CurrentMotionData.Direction;
}

void UAnim_MotionMatchingController::CacheComponentReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
    }
}

void UAnim_MotionMatchingController::SmoothMotionData(float DeltaTime)
{
    if (MotionSmoothingFactor <= 0.0f)
    {
        return;
    }
    
    float SmoothingAlpha = FMath::Clamp(DeltaTime * MotionSmoothingFactor, 0.0f, 1.0f);
    
    // Smooth speed
    CurrentMotionData.Speed = FMath::FInterpTo(
        PreviousMotionData.Speed, 
        CurrentMotionData.Speed, 
        DeltaTime, 
        MotionSmoothingFactor
    );
    
    // Smooth direction (handle wrapping)
    float DirectionDiff = CurrentMotionData.Direction - PreviousMotionData.Direction;
    if (DirectionDiff > 180.0f)
    {
        DirectionDiff -= 360.0f;
    }
    else if (DirectionDiff < -180.0f)
    {
        DirectionDiff += 360.0f;
    }
    
    CurrentMotionData.Direction = PreviousMotionData.Direction + (DirectionDiff * SmoothingAlpha);
    
    // Normalize direction
    while (CurrentMotionData.Direction > 180.0f)
    {
        CurrentMotionData.Direction -= 360.0f;
    }
    while (CurrentMotionData.Direction < -180.0f)
    {
        CurrentMotionData.Direction += 360.0f;
    }
}

bool UAnim_MotionMatchingController::ShouldTransitionToState(EAnim_MotionState NewState)
{
    // Always allow transitions to/from air states
    if (NewState == EAnim_MotionState::Jumping || 
        NewState == EAnim_MotionState::Falling || 
        NewState == EAnim_MotionState::Landing ||
        CurrentMotionData.CurrentState == EAnim_MotionState::Jumping ||
        CurrentMotionData.CurrentState == EAnim_MotionState::Falling)
    {
        return true;
    }
    
    // Prevent rapid state switching for ground movement
    if (bHasPendingTransition && StateTransitionTimer < StateTransitionDelay)
    {
        return false;
    }
    
    return true;
}

void UAnim_MotionMatchingController::ProcessStateTransition(float DeltaTime)
{
    if (!bHasPendingTransition)
    {
        return;
    }
    
    StateTransitionTimer += DeltaTime;
    
    if (StateTransitionTimer >= StateTransitionDelay)
    {
        SetMotionState(PendingState);
    }
}