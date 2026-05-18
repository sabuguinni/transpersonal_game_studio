#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    // Initialize default values
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 300.0f;
    SprintSpeedThreshold = 500.0f;
    JumpingThreshold = 200.0f;
    LandingThreshold = -200.0f;
    StateTransitionSpeed = 5.0f;
    SpeedSmoothingRate = 10.0f;
    
    // Initialize internal state
    PreviousMovementState = EAnim_MovementState::Idle;
    SmoothedSpeed = 0.0f;
    StateTransitionAlpha = 0.0f;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character references
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        CharacterMovement = OwningCharacter->GetCharacterMovement();
        
        UE_LOG(LogTemp, Log, TEXT("Animation Instance initialized for character: %s"), 
               *OwningCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Instance failed to get owning character"));
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Update all movement data
    UpdateMovementData(DeltaTime);
    UpdateMovementState();
    UpdateGroundDistance();
}

void UAnim_CharacterAnimInstance::UpdateMovementData(float DeltaTime)
{
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get current velocity and speed
    MovementData.Velocity = CharacterMovement->Velocity;
    float CurrentSpeed = MovementData.Velocity.Size();
    
    // Smooth the speed for better animation transitions
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, CurrentSpeed, DeltaTime, SpeedSmoothingRate);
    MovementData.Speed = SmoothedSpeed;
    
    // Calculate direction relative to character forward
    MovementData.Direction = CalculateDirection();
    
    // Update movement flags
    MovementData.bIsInAir = CharacterMovement->IsFalling();
    MovementData.bIsCrouching = CharacterMovement->IsCrouching();
    MovementData.bIsAccelerating = CharacterMovement->GetCurrentAcceleration().SizeSquared() > 0.0f;
}

void UAnim_CharacterAnimInstance::UpdateMovementState()
{
    EAnim_MovementState NewState = CalculateMovementState();
    
    // Handle state transitions with smoothing
    if (NewState != MovementData.MovementState)
    {
        PreviousMovementState = MovementData.MovementState;
        MovementData.MovementState = NewState;
        StateTransitionAlpha = 0.0f;
        
        // Log state changes for debugging
        UE_LOG(LogTemp, Log, TEXT("Animation state changed from %d to %d"), 
               (int32)PreviousMovementState, (int32)NewState);
    }
    
    // Update transition alpha
    StateTransitionAlpha = FMath::FInterpTo(StateTransitionAlpha, 1.0f, 
                                          GetWorld()->GetDeltaSeconds(), StateTransitionSpeed);
}

void UAnim_CharacterAnimInstance::UpdateGroundDistance()
{
    if (!OwningCharacter)
    {
        MovementData.GroundDistance = 0.0f;
        return;
    }
    
    // Perform line trace to ground
    FVector StartLocation = OwningCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, 1000.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        MovementData.GroundDistance = FVector::Dist(StartLocation, HitResult.Location);
    }
    else
    {
        MovementData.GroundDistance = 1000.0f; // Max trace distance
    }
}

EAnim_MovementState UAnim_CharacterAnimInstance::CalculateMovementState() const
{
    if (!CharacterMovement)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check air states first
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > JumpingThreshold)
        {
            return EAnim_MovementState::Jumping;
        }
        else if (MovementData.Velocity.Z < LandingThreshold)
        {
            return EAnim_MovementState::Falling;
        }
        else
        {
            return EAnim_MovementState::Falling; // Default air state
        }
    }
    
    // Check crouching
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Check swimming
    if (CharacterMovement->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    // Ground movement states based on speed
    float Speed = MovementData.Speed;
    
    if (Speed < 10.0f) // Almost stationary
    {
        return EAnim_MovementState::Idle;
    }
    else if (Speed < WalkSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else if (Speed < RunSpeedThreshold)
    {
        return EAnim_MovementState::Running;
    }
    else if (Speed < SprintSpeedThreshold)
    {
        return EAnim_MovementState::Sprinting;
    }
    else
    {
        return EAnim_MovementState::Sprinting; // Max speed state
    }
}

float UAnim_CharacterAnimInstance::CalculateDirection() const
{
    if (!OwningCharacter || MovementData.Speed < 10.0f)
    {
        return 0.0f;
    }
    
    // Get character forward vector
    FVector ForwardVector = OwningCharacter->GetActorForwardVector();
    
    // Get movement direction (normalized velocity)
    FVector MovementDirection = MovementData.Velocity.GetSafeNormal();
    
    // Calculate angle between forward and movement direction
    float DotProduct = FVector::DotProduct(ForwardVector, MovementDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, MovementDirection).Z;
    
    // Convert to angle in degrees
    float Angle = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
    
    return Angle;
}