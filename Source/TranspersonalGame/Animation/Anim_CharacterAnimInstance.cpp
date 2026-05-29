#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    OwningCharacter = nullptr;
    MovementComponent = nullptr;
    
    // Default animation thresholds
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    IdleThreshold = 10.0f;
    DirectionDeadZone = 15.0f;
    
    // State tracking
    bWasInAirLastFrame = false;
    TimeInCurrentState = 0.0f;
    PreviousMovementState = EAnim_MovementState::Idle;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
    
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Anim_CharacterAnimInstance: Initialized for character %s"), 
               *OwningCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Anim_CharacterAnimInstance: Failed to get character reference"));
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    UpdateMovementData(DeltaTimeX);
    UpdateMovementState();
    UpdateAimOffsets();
    
    // Update state timing
    if (PreviousMovementState != MovementData.MovementState)
    {
        TimeInCurrentState = 0.0f;
        PreviousMovementState = MovementData.MovementState;
    }
    else
    {
        TimeInCurrentState += DeltaTimeX;
    }
    
    // Track air state changes
    bWasInAirLastFrame = MovementData.bIsInAir;
}

void UAnim_CharacterAnimInstance::UpdateMovementData(float DeltaTime)
{
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and speed
    MovementData.Velocity = MovementComponent->Velocity;
    MovementData.GroundSpeed = MovementData.Velocity.Size2D();
    MovementData.Speed = MovementData.GroundSpeed;
    
    // Check air state
    MovementData.bIsInAir = MovementComponent->IsFalling();
    
    // Check crouching
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate direction
    CalculateDirection();
}

void UAnim_CharacterAnimInstance::UpdateMovementState()
{
    MovementData.MovementState = DetermineMovementState();
}

void UAnim_CharacterAnimInstance::CalculateDirection()
{
    if (!OwningCharacter || MovementData.Speed < IdleThreshold)
    {
        MovementData.Direction = 0.0f;
        return;
    }
    
    // Get character forward vector and velocity direction
    FVector ForwardVector = OwningCharacter->GetActorForwardVector();
    FVector VelocityDirection = MovementData.Velocity.GetSafeNormal2D();
    
    // Calculate angle between forward and velocity
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
    
    // Convert to angle in degrees
    float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));
    
    // Apply sign based on cross product
    if (CrossProduct < 0.0f)
    {
        Angle = -Angle;
    }
    
    // Apply dead zone
    if (FMath::Abs(Angle) < DirectionDeadZone)
    {
        MovementData.Direction = 0.0f;
    }
    else
    {
        MovementData.Direction = Angle;
    }
}

void UAnim_CharacterAnimInstance::UpdateAimOffsets()
{
    if (!OwningCharacter)
    {
        MovementData.AimYaw = 0.0f;
        MovementData.AimPitch = 0.0f;
        return;
    }
    
    // Get control rotation
    FRotator ControlRotation = OwningCharacter->GetControlRotation();
    FRotator ActorRotation = OwningCharacter->GetActorRotation();
    
    // Calculate aim offsets
    FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);
    
    MovementData.AimYaw = DeltaRotation.Yaw;
    MovementData.AimPitch = DeltaRotation.Pitch;
    
    // Clamp pitch to reasonable range
    MovementData.AimPitch = FMath::Clamp(MovementData.AimPitch, -90.0f, 90.0f);
}

EAnim_MovementState UAnim_CharacterAnimInstance::DetermineMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check for air states first
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Check for swimming
    if (MovementComponent->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    // Check for crouching
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Ground movement states
    if (MovementData.Speed <= IdleThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (MovementData.Speed <= RunSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}