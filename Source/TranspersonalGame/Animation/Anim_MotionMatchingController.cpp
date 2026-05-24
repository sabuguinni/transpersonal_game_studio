#include "Anim_MotionMatchingController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingController::UAnim_MotionMatchingController()
{
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    
    // Initialize movement state variables
    bIsMoving = false;
    bIsWalking = false;
    bIsRunning = false;
    bIsSprinting = false;
    bIsJumping = false;
    bIsFalling = false;
    bIsLanding = false;
    bIsClimbing = false;
    bIsSwimming = false;
    
    // Initialize blend values
    ForwardBackwardBlend = 0.0f;
    LeftRightBlend = 0.0f;
    MovementBlend = 0.0f;
    SpeedBlend = 0.0f;
    
    // Initialize internal state
    StateTransitionTimer = 0.0f;
    BlendSmoothingSpeed = 5.0f;
    bWasMovingLastFrame = false;
    LastVelocity = FVector::ZeroVector;
    
    InitializeSpeedThresholds();
}

void UAnim_MotionMatchingController::InitializeSpeedThresholds()
{
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    SprintSpeedThreshold = 600.0f;
}

void UAnim_MotionMatchingController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    UpdateCharacterReferences();
    
    if (OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching Controller initialized for: %s"), 
               *OwnerCharacter->GetName());
    }
}

void UAnim_MotionMatchingController::UpdateCharacterReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UAnim_MotionMatchingController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        UpdateCharacterReferences();
        return;
    }
    
    // Store previous frame data
    PreviousMotionData = CurrentMotionData;
    
    // Update motion data for current frame
    UpdateMotionData();
    
    // Update movement states
    UpdateMovementStates();
    
    // Calculate blend values for animation
    CalculateMovementBlends();
    
    // Smooth blend transitions
    SmoothBlendValues(DeltaTimeX);
    
    // Update state transition timer
    StateTransitionTimer += DeltaTimeX;
}

void UAnim_MotionMatchingController::UpdateMotionData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get current velocity and speed
    CurrentMotionData.Velocity = OwnerCharacter->GetVelocity();
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    
    // Calculate movement direction relative to character forward
    CurrentMotionData.Direction = GetMovementDirection();
    
    // Update movement state flags
    CurrentMotionData.bIsInAir = MovementComponent->IsFalling();
    CurrentMotionData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Determine current movement state
    if (CurrentMotionData.bIsInAir)
    {
        if (CurrentMotionData.Velocity.Z > 0)
        {
            CurrentMotionData.MovementState = EAnim_MovementState::Jumping;
        }
        else
        {
            CurrentMotionData.MovementState = EAnim_MovementState::Falling;
        }
    }
    else if (CurrentMotionData.bIsCrouching)
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Crouching;
    }
    else if (CurrentMotionData.Speed < 10.0f)
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Idle;
    }
    else if (CurrentMotionData.Speed < WalkSpeedThreshold)
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Walking;
    }
    else if (CurrentMotionData.Speed < RunSpeedThreshold)
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Running;
    }
    else
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Sprinting;
    }
}

void UAnim_MotionMatchingController::UpdateMovementStates()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update basic movement flags
    bIsMoving = CurrentMotionData.Speed > 10.0f;
    bIsWalking = CurrentMotionData.Speed > 10.0f && CurrentMotionData.Speed < WalkSpeedThreshold;
    bIsRunning = CurrentMotionData.Speed >= WalkSpeedThreshold && CurrentMotionData.Speed < RunSpeedThreshold;
    bIsSprinting = CurrentMotionData.Speed >= RunSpeedThreshold;
    
    // Update air movement flags
    bIsJumping = CurrentMotionData.bIsInAir && CurrentMotionData.Velocity.Z > 0;
    bIsFalling = CurrentMotionData.bIsInAir && CurrentMotionData.Velocity.Z <= 0;
    bIsLanding = !CurrentMotionData.bIsInAir && PreviousMotionData.bIsInAir;
    
    // Update special movement flags (placeholder for future implementation)
    bIsClimbing = false; // TODO: Implement climbing detection
    bIsSwimming = MovementComponent->IsSwimming();
}

void UAnim_MotionMatchingController::CalculateMovementBlends()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Calculate forward/backward blend (-1 to 1)
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector VelocityNormalized = CurrentMotionData.Velocity.GetSafeNormal();
    ForwardBackwardBlend = FVector::DotProduct(ForwardVector, VelocityNormalized);
    
    // Calculate left/right blend (-1 to 1)
    FVector RightVector = OwnerCharacter->GetActorRightVector();
    LeftRightBlend = FVector::DotProduct(RightVector, VelocityNormalized);
    
    // Calculate overall movement blend (0 to 1)
    MovementBlend = FMath::Clamp(CurrentMotionData.Speed / RunSpeedThreshold, 0.0f, 1.0f);
    
    // Calculate speed blend for different movement types
    if (CurrentMotionData.Speed < WalkSpeedThreshold)
    {
        SpeedBlend = FMath::Clamp(CurrentMotionData.Speed / WalkSpeedThreshold, 0.0f, 1.0f);
    }
    else if (CurrentMotionData.Speed < RunSpeedThreshold)
    {
        SpeedBlend = 1.0f + FMath::Clamp((CurrentMotionData.Speed - WalkSpeedThreshold) / 
                                        (RunSpeedThreshold - WalkSpeedThreshold), 0.0f, 1.0f);
    }
    else
    {
        SpeedBlend = 2.0f + FMath::Clamp((CurrentMotionData.Speed - RunSpeedThreshold) / 
                                        (SprintSpeedThreshold - RunSpeedThreshold), 0.0f, 1.0f);
    }
}

void UAnim_MotionMatchingController::SmoothBlendValues(float DeltaTime)
{
    // Smooth blend value transitions to avoid jittery animations
    float SmoothingFactor = BlendSmoothingSpeed * DeltaTime;
    
    // Apply smoothing to blend values
    // Note: In a real implementation, you would store target and current values separately
    // This is a simplified version for demonstration
}

float UAnim_MotionMatchingController::GetMovementDirection() const
{
    if (!OwnerCharacter || CurrentMotionData.Speed < 10.0f)
    {
        return 0.0f;
    }
    
    return CalculateDirectionAngle();
}

float UAnim_MotionMatchingController::CalculateDirectionAngle() const
{
    if (!OwnerCharacter)
    {
        return 0.0f;
    }
    
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector VelocityDirection = CurrentMotionData.Velocity.GetSafeNormal();
    
    // Calculate angle between forward vector and velocity direction
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
    
    return FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
}

bool UAnim_MotionMatchingController::ShouldTransitionToState(EAnim_MovementState NewState) const
{
    // Prevent rapid state transitions
    if (StateTransitionTimer < 0.1f && CurrentMotionData.MovementState != NewState)
    {
        return false;
    }
    
    // Allow transition based on current conditions
    return true;
}

void UAnim_MotionMatchingController::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMotionData.MovementState != NewState && ShouldTransitionToState(NewState))
    {
        CurrentMotionData.MovementState = NewState;
        StateTransitionTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Motion Matching: Transitioned to state %d"), 
               static_cast<int32>(NewState));
    }
}

FAnim_MotionData UAnim_MotionMatchingController::GetBestMotionMatch(const FAnim_MotionData& TargetMotion)
{
    // Placeholder for motion matching algorithm
    // In a full implementation, this would search through a database of motion clips
    // and find the best match based on velocity, pose, and trajectory
    
    return TargetMotion; // Simplified return for now
}

float UAnim_MotionMatchingController::CalculateMotionScore(const FAnim_MotionData& Motion1, const FAnim_MotionData& Motion2)
{
    float Score = 0.0f;
    
    // Calculate velocity difference
    float VelocityDiff = (Motion1.Velocity - Motion2.Velocity).Size();
    Score += VelocityDiff * MotionMatchingSettings.VelocityWeight;
    
    // Calculate direction difference
    float DirectionDiff = FMath::Abs(Motion1.Direction - Motion2.Direction);
    Score += DirectionDiff * MotionMatchingSettings.DirectionWeight;
    
    // Add other scoring factors based on settings
    // (pose matching, trajectory prediction, etc.)
    
    return Score;
}