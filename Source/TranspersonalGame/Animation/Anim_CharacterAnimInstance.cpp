#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    // Set default values
    SpeedSmoothingRate = 10.0f;
    DirectionSmoothingRate = 15.0f;
    LeanSmoothingRate = 8.0f;
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    SprintSpeedThreshold = 600.0f;
    JumpVelocityThreshold = 100.0f;
    FallVelocityThreshold = -100.0f;
    MinStateTime = 0.1f;
    
    // Initialize movement data
    MovementData = FAnim_MovementData();
    
    // Initialize smoothed values
    SmoothedSpeed = 0.0f;
    SmoothedDirection = 0.0f;
    SmoothedLeanAngle = 0.0f;
    
    // Initialize state tracking
    TimeInCurrentState = 0.0f;
    PreviousMovementState = EAnim_MovementState::Idle;
    PreviousVelocity = FVector::ZeroVector;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    
    if (OwnerCharacter)
    {
        // Get movement component
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                FString::Printf(TEXT("Animation Instance initialized for: %s"), *OwnerCharacter->GetName()));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("Failed to get character reference in animation instance"));
        }
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update movement data
    UpdateMovementData(DeltaTimeX);
    
    // Update movement state
    UpdateMovementState();
    
    // Update blend space values
    UpdateBlendSpaceValues();
    
    // Update state transition booleans
    bShouldEnterIdle = ShouldEnterIdleState();
    bShouldEnterMovement = ShouldEnterMovementState();
    bShouldEnterJump = ShouldEnterJumpState();
    bShouldEnterFall = ShouldEnterFallState();
    
    // Update time in current state
    if (MovementData.MovementState == PreviousMovementState)
    {
        TimeInCurrentState += DeltaTimeX;
    }
    else
    {
        TimeInCurrentState = 0.0f;
        PreviousMovementState = MovementData.MovementState;
    }
}

void UAnim_CharacterAnimInstance::UpdateMovementData(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get current velocity
    MovementData.Velocity = MovementComponent->Velocity;
    
    // Calculate speed (2D velocity magnitude)
    FVector Velocity2D = FVector(MovementData.Velocity.X, MovementData.Velocity.Y, 0.0f);
    MovementData.Speed = Velocity2D.Size();
    
    // Calculate direction relative to actor forward
    if (MovementData.Speed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity2D.GetSafeNormal();
        
        // Calculate angle between forward and velocity
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
        
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update air state
    MovementData.bIsInAir = MovementComponent->IsFalling();
    
    // Update moving state
    MovementData.bIsMoving = MovementData.Speed > WalkSpeedThreshold;
    
    // Update crouch state
    MovementData.bIsCrouched = MovementComponent->IsCrouching();
    
    // Calculate lean angle based on velocity change
    FVector VelocityDelta = MovementData.Velocity - PreviousVelocity;
    if (VelocityDelta.Size() > 1.0f)
    {
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float LeanInfluence = FVector::DotProduct(VelocityDelta, RightVector);
        MovementData.LeanAngle = FMath::Clamp(LeanInfluence * 0.1f, -45.0f, 45.0f);
    }
    else
    {
        MovementData.LeanAngle = 0.0f;
    }
    
    // Update aim offset from control rotation
    if (OwnerCharacter->GetController())
    {
        FRotator ControlRotation = OwnerCharacter->GetControlRotation();
        FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        
        FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);
        
        MovementData.AimPitch = FMath::Clamp(DeltaRotation.Pitch, -90.0f, 90.0f);
        MovementData.AimYaw = FMath::Clamp(DeltaRotation.Yaw, -180.0f, 180.0f);
        
        // Update aim offset properties for animation blueprint
        AimOffsetPitch = MovementData.AimPitch;
        AimOffsetYaw = MovementData.AimYaw;
    }
    
    // Store previous velocity for next frame
    PreviousVelocity = MovementData.Velocity;
}

void UAnim_CharacterAnimInstance::UpdateMovementState()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    EAnim_MovementState NewState = MovementData.MovementState;
    
    // Determine movement state based on current conditions
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > JumpVelocityThreshold)
        {
            NewState = EAnim_MovementState::Jumping;
        }
        else if (MovementData.Velocity.Z < FallVelocityThreshold)
        {
            NewState = EAnim_MovementState::Falling;
        }
    }
    else
    {
        // Ground movement states
        if (MovementData.bIsCrouched)
        {
            NewState = EAnim_MovementState::Crouching;
        }
        else if (MovementData.Speed < WalkSpeedThreshold)
        {
            NewState = EAnim_MovementState::Idle;
        }
        else if (MovementData.Speed < RunSpeedThreshold)
        {
            NewState = EAnim_MovementState::Walking;
        }
        else if (MovementData.Speed < SprintSpeedThreshold)
        {
            NewState = EAnim_MovementState::Running;
        }
        else
        {
            NewState = EAnim_MovementState::Sprinting;
        }
    }
    
    // Only change state if we've been in current state long enough
    if (NewState != MovementData.MovementState && TimeInCurrentState >= MinStateTime)
    {
        MovementData.MovementState = NewState;
        
        if (GEngine)
        {
            FString StateName = UEnum::GetValueAsString(MovementData.MovementState);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
                FString::Printf(TEXT("Animation State Changed: %s"), *StateName));
        }
    }
}

void UAnim_CharacterAnimInstance::UpdateBlendSpaceValues()
{
    if (!IsValid(this))
    {
        return;
    }
    
    float DeltaTime = GetDeltaSeconds();
    
    // Smooth speed value
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, MovementData.Speed, DeltaTime, SpeedSmoothingRate);
    
    // Smooth direction value
    SmoothedDirection = FMath::FInterpTo(SmoothedDirection, MovementData.Direction, DeltaTime, DirectionSmoothingRate);
    
    // Smooth lean angle
    SmoothedLeanAngle = FMath::FInterpTo(SmoothedLeanAngle, MovementData.LeanAngle, DeltaTime, LeanSmoothingRate);
    
    // Update movement data with smoothed values for blend spaces
    MovementData.Speed = SmoothedSpeed;
    MovementData.Direction = SmoothedDirection;
    MovementData.LeanAngle = SmoothedLeanAngle;
}

bool UAnim_CharacterAnimInstance::ShouldEnterIdleState() const
{
    return !MovementData.bIsInAir && 
           MovementData.Speed < WalkSpeedThreshold && 
           !MovementData.bIsCrouched &&
           TimeInCurrentState >= MinStateTime;
}

bool UAnim_CharacterAnimInstance::ShouldEnterMovementState() const
{
    return !MovementData.bIsInAir && 
           MovementData.Speed >= WalkSpeedThreshold && 
           !MovementData.bIsCrouched &&
           TimeInCurrentState >= MinStateTime;
}

bool UAnim_CharacterAnimInstance::ShouldEnterJumpState() const
{
    return MovementData.bIsInAir && 
           MovementData.Velocity.Z > JumpVelocityThreshold &&
           MovementData.MovementState != EAnim_MovementState::Jumping;
}

bool UAnim_CharacterAnimInstance::ShouldEnterFallState() const
{
    return MovementData.bIsInAir && 
           MovementData.Velocity.Z < FallVelocityThreshold &&
           MovementData.MovementState != EAnim_MovementState::Falling;
}