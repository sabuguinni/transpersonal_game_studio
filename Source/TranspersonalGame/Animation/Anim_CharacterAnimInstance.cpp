#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    // Initialize default values
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentCombatState = EAnim_CombatState::Unarmed;
    
    WalkRunBlend = 0.0f;
    TurnInPlaceAngle = 0.0f;
    bShouldTurnInPlace = false;
    LeanAmount = 0.0f;
    AimPitch = 0.0f;
    AimYaw = 0.0f;
    
    // Animation thresholds
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 375.0f;
    TurnInPlaceThreshold = 45.0f;
    IdleTimeBeforeTurnInPlace = 2.0f;
    
    // Survival states
    bIsHungry = false;
    bIsThirsty = false;
    bIsTired = false;
    bIsAfraid = false;
    FearLevel = 0.0f;
    
    // Internal state
    IdleTime = 0.0f;
    LastMovementTime = 0.0f;
    LastFrameRotation = FRotator::ZeroRotator;
    
    OwnerCharacter = nullptr;
    CharacterMovement = nullptr;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character references
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        CharacterMovement = OwnerCharacter->GetCharacterMovement();
        LastFrameRotation = OwnerCharacter->GetActorRotation();
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Update all animation data
    UpdateMovementData();
    UpdateMovementState();
    UpdateCombatState();
    UpdateAnimationParameters();
    UpdateSurvivalStates();
    UpdateTurnInPlace(DeltaTimeX);
}

void UAnim_CharacterAnimInstance::UpdateMovementData()
{
    if (!OwnerCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get basic movement data
    MovementData.Velocity = CharacterMovement->Velocity;
    MovementData.GroundSpeed = MovementData.Velocity.Size2D();
    MovementData.Speed = MovementData.GroundSpeed;
    MovementData.Direction = CalculateDirection();
    
    // Movement state flags
    MovementData.bIsInAir = CharacterMovement->IsFalling();
    MovementData.bIsCrouching = CharacterMovement->IsCrouching();
    MovementData.bIsAccelerating = CharacterMovement->GetCurrentAcceleration().SizeSquared() > 0.0f;
}

void UAnim_CharacterAnimInstance::UpdateMovementState()
{
    CurrentMovementState = CalculateMovementState();
}

void UAnim_CharacterAnimInstance::UpdateCombatState()
{
    // For now, default to unarmed
    // This will be expanded when weapon system is implemented
    CurrentCombatState = EAnim_CombatState::Unarmed;
}

void UAnim_CharacterAnimInstance::UpdateAnimationParameters()
{
    // Calculate walk/run blend
    WalkRunBlend = CalculateWalkRunBlend();
    
    // Calculate lean amount based on acceleration
    if (CharacterMovement && MovementData.bIsAccelerating)
    {
        FVector Acceleration = CharacterMovement->GetCurrentAcceleration();
        FVector LocalAcceleration = OwnerCharacter->GetActorTransform().InverseTransformVectorNoScale(Acceleration);
        LeanAmount = FMath::Clamp(LocalAcceleration.Y / CharacterMovement->GetMaxAcceleration(), -1.0f, 1.0f);
    }
    else
    {
        LeanAmount = FMath::FInterpTo(LeanAmount, 0.0f, GetWorld()->GetDeltaSeconds(), 5.0f);
    }
    
    // Update aim offsets (placeholder for now)
    AimPitch = 0.0f;
    AimYaw = 0.0f;
}

void UAnim_CharacterAnimInstance::UpdateSurvivalStates()
{
    // This will be connected to the survival system later
    // For now, use placeholder logic based on movement
    
    // Simulate fatigue based on movement
    if (MovementData.Speed > RunSpeedThreshold)
    {
        bIsTired = true;
    }
    else if (MovementData.Speed < WalkSpeedThreshold)
    {
        bIsTired = false;
    }
    
    // Placeholder fear system
    FearLevel = FMath::Clamp(FearLevel - GetWorld()->GetDeltaSeconds() * 0.1f, 0.0f, 1.0f);
    bIsAfraid = FearLevel > 0.3f;
}

void UAnim_CharacterAnimInstance::UpdateTurnInPlace(float DeltaTime)
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Track idle time
    if (MovementData.Speed < 1.0f)
    {
        IdleTime += DeltaTime;
    }
    else
    {
        IdleTime = 0.0f;
        LastMovementTime = GetWorld()->GetTimeSeconds();
    }
    
    // Calculate turn in place angle
    FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
    float DeltaYaw = FMath::FindDeltaAngleDegrees(LastFrameRotation.Yaw, CurrentRotation.Yaw);
    
    if (FMath::Abs(DeltaYaw) > 0.1f)
    {
        TurnInPlaceAngle += DeltaYaw;
        TurnInPlaceAngle = FMath::ClampAngle(TurnInPlaceAngle, -180.0f, 180.0f);
    }
    
    // Determine if we should turn in place
    bShouldTurnInPlace = ShouldTurnInPlace();
    
    // Reset turn angle if we've completed the turn
    if (bShouldTurnInPlace && FMath::Abs(TurnInPlaceAngle) < 5.0f)
    {
        TurnInPlaceAngle = 0.0f;
        bShouldTurnInPlace = false;
    }
    
    LastFrameRotation = CurrentRotation;
}

EAnim_MovementState UAnim_CharacterAnimInstance::CalculateMovementState() const
{
    if (!CharacterMovement)
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
    if (CharacterMovement->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    // Check for crouching
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Ground movement states
    if (MovementData.Speed > RunSpeedThreshold)
    {
        return EAnim_MovementState::Running;
    }
    else if (MovementData.Speed > WalkSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    
    return EAnim_MovementState::Idle;
}

float UAnim_CharacterAnimInstance::CalculateDirection() const
{
    if (!OwnerCharacter || MovementData.Speed < 1.0f)
    {
        return 0.0f;
    }
    
    // Calculate direction relative to actor's forward vector
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector VelocityNormalized = MovementData.Velocity.GetSafeNormal2D();
    
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
    
    return FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
}

float UAnim_CharacterAnimInstance::CalculateWalkRunBlend() const
{
    if (MovementData.Speed <= WalkSpeedThreshold)
    {
        return 0.0f; // Pure walk
    }
    else if (MovementData.Speed >= RunSpeedThreshold)
    {
        return 1.0f; // Pure run
    }
    
    // Blend between walk and run
    float BlendRange = RunSpeedThreshold - WalkSpeedThreshold;
    float SpeedInRange = MovementData.Speed - WalkSpeedThreshold;
    return FMath::Clamp(SpeedInRange / BlendRange, 0.0f, 1.0f);
}

bool UAnim_CharacterAnimInstance::ShouldTurnInPlace() const
{
    // Turn in place if we've been idle long enough and have significant rotation
    return (IdleTime >= IdleTimeBeforeTurnInPlace && 
            FMath::Abs(TurnInPlaceAngle) >= TurnInPlaceThreshold);
}