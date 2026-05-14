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
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize motion data
    CurrentMotionData = FAnim_MotionData();
    PreviousMotionData = FAnim_MotionData();
    
    // Initialize state tracking
    StateChangeTime = 0.0f;
    PreviousState = EAnim_MotionState::Idle;
    
    // Set default thresholds
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    JumpVelocityThreshold = 100.0f;
    
    SpeedSmoothingRate = 10.0f;
    DirectionSmoothingRate = 15.0f;
}

void UAnim_MotionMatchingController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get character and movement component references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Motion Matching Controller initialized for character: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Motion Matching Controller: Owner is not a Character!"));
    }
}

void UAnim_MotionMatchingController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Store previous data
    PreviousMotionData = CurrentMotionData;
    
    // Update motion data
    UpdateMotionData();
    
    // Smooth the data
    SmoothMotionData(DeltaTime);
    
    // Determine new state
    EAnim_MotionState NewState = DetermineMotionState();
    
    // Check for state changes
    if (NewState != CurrentMotionData.CurrentState)
    {
        OnStateChanged(CurrentMotionData.CurrentState, NewState);
        SetAnimationState(NewState);
    }
    
    // Update state change time
    StateChangeTime += DeltaTime;
}

void UAnim_MotionMatchingController::UpdateMotionData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and speed
    FVector CurrentVelocity = MovementComponent->Velocity;
    CurrentMotionData.Velocity = CurrentVelocity;
    CurrentMotionData.Speed = CurrentVelocity.Size();
    
    // Calculate movement direction relative to character forward
    CurrentMotionData.Direction = CalculateMovementDirection();
    
    // Update movement state flags
    CurrentMotionData.bIsInAir = MovementComponent->IsFalling();
    CurrentMotionData.bIsCrouching = MovementComponent->IsCrouching();
}

EAnim_MotionState UAnim_MotionMatchingController::DetermineMotionState()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return EAnim_MotionState::Idle;
    }
    
    // Check for airborne states first
    if (CurrentMotionData.bIsInAir)
    {
        if (CurrentMotionData.Velocity.Z > JumpVelocityThreshold)
        {
            return EAnim_MotionState::Jumping;
        }
        else if (CurrentMotionData.Velocity.Z < -JumpVelocityThreshold)
        {
            return EAnim_MotionState::Falling;
        }
        else
        {
            return EAnim_MotionState::Falling; // Default airborne state
        }
    }
    
    // Check for landing (just landed)
    if (PreviousMotionData.bIsInAir && !CurrentMotionData.bIsInAir && StateChangeTime < 0.5f)
    {
        return EAnim_MotionState::Landing;
    }
    
    // Check for crouching
    if (CurrentMotionData.bIsCrouching)
    {
        return EAnim_MotionState::Crouching;
    }
    
    // Ground movement states based on speed
    if (CurrentMotionData.Speed < WalkSpeedThreshold)
    {
        return EAnim_MotionState::Idle;
    }
    else if (CurrentMotionData.Speed < RunSpeedThreshold)
    {
        return EAnim_MotionState::Walking;
    }
    else
    {
        return EAnim_MotionState::Running;
    }
}

float UAnim_MotionMatchingController::CalculateMovementDirection()
{
    if (!OwnerCharacter || CurrentMotionData.Speed < 1.0f)
    {
        return 0.0f;
    }
    
    // Get character forward vector
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    
    // Get horizontal velocity (ignore Z component)
    FVector HorizontalVelocity = FVector(CurrentMotionData.Velocity.X, CurrentMotionData.Velocity.Y, 0.0f);
    HorizontalVelocity.Normalize();
    
    // Calculate angle between forward and velocity
    float DotProduct = FVector::DotProduct(ForwardVector, HorizontalVelocity);
    float CrossProduct = FVector::CrossProduct(ForwardVector, HorizontalVelocity).Z;
    
    // Convert to angle (-180 to 180)
    float Angle = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
    
    return Angle;
}

void UAnim_MotionMatchingController::SetAnimationState(EAnim_MotionState NewState)
{
    PreviousState = CurrentMotionData.CurrentState;
    CurrentMotionData.CurrentState = NewState;
    StateChangeTime = 0.0f;
    
    // Log state changes for debugging
    UE_LOG(LogTemp, Log, TEXT("Animation state changed from %d to %d"), (int32)PreviousState, (int32)NewState);
}

void UAnim_MotionMatchingController::SmoothMotionData(float DeltaTime)
{
    // Smooth speed
    CurrentMotionData.Speed = FMath::FInterpTo(
        PreviousMotionData.Speed,
        CurrentMotionData.Speed,
        DeltaTime,
        SpeedSmoothingRate
    );
    
    // Smooth direction
    CurrentMotionData.Direction = FMath::FInterpTo(
        PreviousMotionData.Direction,
        CurrentMotionData.Direction,
        DeltaTime,
        DirectionSmoothingRate
    );
}

bool UAnim_MotionMatchingController::HasStateChanged()
{
    return CurrentMotionData.CurrentState != PreviousState;
}

void UAnim_MotionMatchingController::OnStateChanged(EAnim_MotionState OldState, EAnim_MotionState NewState)
{
    // Handle specific state transitions
    switch (NewState)
    {
        case EAnim_MotionState::Jumping:
            UE_LOG(LogTemp, Log, TEXT("Character started jumping"));
            break;
            
        case EAnim_MotionState::Landing:
            UE_LOG(LogTemp, Log, TEXT("Character landed"));
            break;
            
        case EAnim_MotionState::Running:
            if (OldState == EAnim_MotionState::Walking)
            {
                UE_LOG(LogTemp, Log, TEXT("Character started running"));
            }
            break;
            
        case EAnim_MotionState::Walking:
            if (OldState == EAnim_MotionState::Idle)
            {
                UE_LOG(LogTemp, Log, TEXT("Character started walking"));
            }
            break;
            
        case EAnim_MotionState::Idle:
            UE_LOG(LogTemp, Log, TEXT("Character stopped moving"));
            break;
            
        default:
            break;
    }
}