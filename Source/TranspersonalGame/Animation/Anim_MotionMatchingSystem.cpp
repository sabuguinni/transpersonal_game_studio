#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    // Initialize default values
    SpeedThreshold = 10.0f;
    RunThreshold = 300.0f;
    DirectionSmoothingSpeed = 10.0f;
    BlendSpeed = 5.0f;
    
    LastVelocity = FVector::ZeroVector;
    LastAcceleration = FVector::ZeroVector;
    StateTimer = 0.0f;
    LastGroundTime = 0.0f;
    bWasInAir = false;
}

void UAnim_MotionMatchingSystem::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Get character references
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
        
        if (MovementComponent)
        {
            UE_LOG(LogTemp, Log, TEXT("Motion Matching System initialized for character: %s"), 
                   *OwningCharacter->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No CharacterMovementComponent found on %s"), 
                   *OwningCharacter->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Motion Matching System: OwningActor is not a Character"));
    }

    // Initialize motion data
    MotionData = FAnim_MotionData();
    BlendParams = FAnim_BlendParameters();
}

void UAnim_MotionMatchingSystem::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }

    // Update motion data and blend parameters
    UpdateMotionData(DeltaTimeX);
    UpdateBlendParameters(DeltaTimeX);
    
    // Update state timer
    StateTimer += DeltaTimeX;
}

void UAnim_MotionMatchingSystem::UpdateMotionData(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }

    // Get current movement data
    FVector CurrentVelocity = MovementComponent->Velocity;
    FVector CurrentAcceleration = (CurrentVelocity - LastVelocity) / DeltaTime;
    
    // Update motion data
    MotionData.Velocity = CurrentVelocity;
    MotionData.Acceleration = CurrentAcceleration;
    
    // Calculate rotation velocity
    if (OwningCharacter)
    {
        FRotator CurrentRotation = OwningCharacter->GetActorRotation();
        static FRotator LastRotation = CurrentRotation;
        MotionData.RotationVelocity = (CurrentRotation - LastRotation) / DeltaTime;
        LastRotation = CurrentRotation;
    }

    // Update movement state
    EAnim_MovementState NewState = CalculateMovementState();
    if (ShouldTransitionToState(NewState))
    {
        MotionData.PreviousState = MotionData.CurrentState;
        MotionData.CurrentState = NewState;
        MotionData.StateTransitionTime = 0.0f;
        StateTimer = 0.0f;
    }
    else
    {
        MotionData.StateTransitionTime += DeltaTime;
    }

    // Store for next frame
    LastVelocity = CurrentVelocity;
    LastAcceleration = CurrentAcceleration;
}

EAnim_MovementState UAnim_MotionMatchingSystem::CalculateMovementState()
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }

    // Check if in air
    if (MovementComponent->IsFalling())
    {
        if (!bWasInAir)
        {
            bWasInAir = true;
            return EAnim_MovementState::Jumping;
        }
        return EAnim_MovementState::Falling;
    }
    else
    {
        // Just landed
        if (bWasInAir)
        {
            bWasInAir = false;
            LastGroundTime = 0.0f;
            return EAnim_MovementState::Landing;
        }
        
        LastGroundTime += GetWorld()->GetDeltaSeconds();
    }

    // Check if crouching
    if (MovementComponent->IsCrouching())
    {
        float Speed = MovementComponent->Velocity.Size2D();
        if (Speed > SpeedThreshold)
        {
            return EAnim_MovementState::Crouching;
        }
        return EAnim_MovementState::Crouching;
    }

    // Check movement speed
    float Speed = MovementComponent->Velocity.Size2D();
    
    if (Speed < SpeedThreshold)
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

void UAnim_MotionMatchingSystem::UpdateBlendParameters(float DeltaTime)
{
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }

    // Update speed
    float CurrentSpeed = MovementComponent->Velocity.Size2D();
    BlendParams.Speed = FMath::FInterpTo(BlendParams.Speed, CurrentSpeed, DeltaTime, BlendSpeed);

    // Update direction
    float CurrentDirection = CalculateDirection(MovementComponent->Velocity, OwningCharacter->GetActorRotation());
    BlendParams.Direction = FMath::FInterpTo(BlendParams.Direction, CurrentDirection, DeltaTime, DirectionSmoothingSpeed);

    // Update lean angle (based on acceleration)
    FVector LateralAcceleration = FVector::VectorPlaneProject(MotionData.Acceleration, FVector::UpVector);
    float LeanAmount = FMath::Clamp(LateralAcceleration.Size() / 1000.0f, -45.0f, 45.0f);
    BlendParams.LeanAngle = FMath::FInterpTo(BlendParams.LeanAngle, LeanAmount, DeltaTime, BlendSpeed);

    // Update boolean states
    BlendParams.bIsInAir = MovementComponent->IsFalling();
    BlendParams.bIsCrouching = MovementComponent->IsCrouching();

    // Update footstep timing
    BlendParams.TimeSinceLastFootstep += DeltaTime;

    // Smooth blend parameters
    SmoothBlendParameters(DeltaTime);
}

bool UAnim_MotionMatchingSystem::ShouldTransitionToState(EAnim_MovementState NewState)
{
    // Prevent rapid state switching
    if (StateTimer < 0.1f && MotionData.CurrentState != EAnim_MovementState::Idle)
    {
        return false;
    }

    // Validate transition
    return ValidateStateTransition(MotionData.CurrentState, NewState);
}

void UAnim_MotionMatchingSystem::PlayActionMontage(UAnimMontage* Montage, float PlayRate)
{
    if (Montage && !IsPlayingActionMontage())
    {
        Montage_Play(Montage, PlayRate);
        UE_LOG(LogTemp, Log, TEXT("Playing action montage: %s"), *Montage->GetName());
    }
}

void UAnim_MotionMatchingSystem::StopActionMontage(float BlendOutTime)
{
    if (IsPlayingActionMontage())
    {
        Montage_Stop(BlendOutTime);
        UE_LOG(LogTemp, Log, TEXT("Stopping action montage"));
    }
}

bool UAnim_MotionMatchingSystem::IsPlayingActionMontage() const
{
    return IsAnyMontagePlaying();
}

bool UAnim_MotionMatchingSystem::IsMoving() const
{
    return BlendParams.Speed > SpeedThreshold;
}

bool UAnim_MotionMatchingSystem::IsRunning() const
{
    return BlendParams.Speed > RunThreshold;
}

bool UAnim_MotionMatchingSystem::IsInAir() const
{
    return BlendParams.bIsInAir;
}

bool UAnim_MotionMatchingSystem::IsCrouching() const
{
    return BlendParams.bIsCrouching;
}

float UAnim_MotionMatchingSystem::GetMovementSpeed() const
{
    return BlendParams.Speed;
}

float UAnim_MotionMatchingSystem::GetMovementDirection() const
{
    return BlendParams.Direction;
}

float UAnim_MotionMatchingSystem::CalculateDirection(const FVector& Velocity, const FRotator& Rotation)
{
    if (Velocity.Size2D() < SpeedThreshold)
    {
        return 0.0f;
    }

    // Calculate angle between forward vector and velocity
    FVector ForwardVector = Rotation.Vector();
    FVector VelocityNormalized = Velocity.GetSafeNormal2D();
    
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
    
    float Angle = FMath::Atan2(CrossProduct, DotProduct);
    return FMath::RadiansToDegrees(Angle);
}

void UAnim_MotionMatchingSystem::SmoothBlendParameters(float DeltaTime)
{
    // Additional smoothing for specific parameters
    static float LastSpeed = BlendParams.Speed;
    static float LastDirection = BlendParams.Direction;
    
    // Prevent jittery speed changes
    if (FMath::Abs(BlendParams.Speed - LastSpeed) < 5.0f)
    {
        BlendParams.Speed = FMath::FInterpTo(LastSpeed, BlendParams.Speed, DeltaTime, BlendSpeed * 2.0f);
    }
    
    // Prevent direction snapping
    if (FMath::Abs(BlendParams.Direction - LastDirection) > 180.0f)
    {
        // Handle wrapping around 180/-180 degrees
        if (BlendParams.Direction > LastDirection)
        {
            LastDirection += 360.0f;
        }
        else
        {
            LastDirection -= 360.0f;
        }
    }
    
    LastSpeed = BlendParams.Speed;
    LastDirection = BlendParams.Direction;
}

bool UAnim_MotionMatchingSystem::ValidateStateTransition(EAnim_MovementState FromState, EAnim_MovementState ToState)
{
    // Define valid state transitions
    switch (FromState)
    {
        case EAnim_MovementState::Idle:
            return true; // Can transition to any state from idle
            
        case EAnim_MovementState::Walking:
            return ToState != EAnim_MovementState::Landing; // Can't land while walking
            
        case EAnim_MovementState::Running:
            return ToState != EAnim_MovementState::Landing; // Can't land while running
            
        case EAnim_MovementState::Jumping:
            return ToState == EAnim_MovementState::Falling || ToState == EAnim_MovementState::Landing;
            
        case EAnim_MovementState::Falling:
            return ToState == EAnim_MovementState::Landing || ToState == EAnim_MovementState::Falling;
            
        case EAnim_MovementState::Landing:
            return ToState != EAnim_MovementState::Jumping && ToState != EAnim_MovementState::Falling;
            
        case EAnim_MovementState::Crouching:
            return ToState != EAnim_MovementState::Jumping; // Can't jump while crouching
            
        default:
            return true;
    }
}