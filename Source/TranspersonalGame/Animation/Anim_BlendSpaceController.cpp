#include "Anim_BlendSpaceController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_BlendSpaceController::UAnim_BlendSpaceController()
{
    // Initialize smoothing rates
    SpeedSmoothingRate = 10.0f;
    DirectionSmoothingRate = 15.0f;
    TurnRateSmoothingRate = 8.0f;

    // Initialize parameters
    Speed = 0.0f;
    Direction = 0.0f;
    SlopeAngle = 0.0f;
    TurnRate = 0.0f;
    bIsMoving = false;
    bIsInAir = false;
    bIsCrouching = false;

    // Initialize survival parameters
    HealthPercent = 100.0f;
    StaminaPercent = 100.0f;
    FearLevel = 0.0f;
    HungerLevel = 0.0f;
    ThirstLevel = 0.0f;

    // Initialize states
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentSurvivalState = EAnim_SurvivalState::Normal;

    // Initialize previous frame data
    PreviousVelocity = FVector::ZeroVector;
    PreviousRotation = FRotator::ZeroRotator;
    PreviousYaw = 0.0f;
}

void UAnim_BlendSpaceController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        // Initialize previous frame data
        PreviousVelocity = OwnerCharacter->GetVelocity();
        PreviousRotation = OwnerCharacter->GetActorRotation();
        PreviousYaw = PreviousRotation.Yaw;
    }
}

void UAnim_BlendSpaceController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Update movement data
    UpdateMovementData(DeltaTimeX);
    
    // Update states
    UpdateMovementState();
    UpdateSurvivalState();
}

void UAnim_BlendSpaceController::UpdateMovementData(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Get current velocity and movement data
    FVector CurrentVelocity = OwnerCharacter->GetVelocity();
    FVector Velocity2D = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);
    
    // Calculate speed (smooth interpolation)
    float TargetSpeed = Velocity2D.Size();
    Speed = FMath::FInterpTo(Speed, TargetSpeed, DeltaTime, SpeedSmoothingRate);
    
    // Update movement flags
    bIsMoving = Speed > 5.0f; // Threshold for considering movement
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Calculate direction relative to character forward
    CalculateDirection();
    
    // Calculate slope angle
    CalculateSlopeAngle();
    
    // Calculate turn rate
    CalculateTurnRate(DeltaTime);

    // Update movement data struct
    MovementData.Speed = Speed;
    MovementData.Direction = Direction;
    MovementData.SlopeAngle = SlopeAngle;
    MovementData.TurnRate = TurnRate;
    MovementData.bIsMoving = bIsMoving;
    MovementData.bIsInAir = bIsInAir;

    // Store current frame data for next frame
    PreviousVelocity = CurrentVelocity;
    PreviousRotation = OwnerCharacter->GetActorRotation();
    PreviousYaw = PreviousRotation.Yaw;
}

void UAnim_BlendSpaceController::CalculateDirection()
{
    if (!OwnerCharacter || !bIsMoving)
    {
        Direction = 0.0f;
        return;
    }

    // Get velocity in character's local space
    FVector CurrentVelocity = OwnerCharacter->GetVelocity();
    FVector Velocity2D = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);
    
    if (Velocity2D.SizeSquared() > 0.01f)
    {
        // Convert world velocity to local space
        FVector LocalVelocity = OwnerCharacter->GetActorTransform().InverseTransformVectorNoScale(Velocity2D);
        
        // Calculate angle from forward direction (-180 to 180)
        float TargetDirection = FMath::Atan2(LocalVelocity.Y, LocalVelocity.X) * (180.0f / PI);
        
        // Smooth interpolation
        Direction = FMath::FInterpTo(Direction, TargetDirection, GetWorld()->GetDeltaSeconds(), DirectionSmoothingRate);
    }
}

void UAnim_BlendSpaceController::CalculateSlopeAngle()
{
    if (!OwnerCharacter)
    {
        SlopeAngle = 0.0f;
        return;
    }

    // Perform line trace downward to get ground normal
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
    {
        // Calculate slope angle from ground normal
        FVector GroundNormal = HitResult.Normal;
        float DotProduct = FVector::DotProduct(GroundNormal, FVector::UpVector);
        SlopeAngle = FMath::Acos(DotProduct) * (180.0f / PI);
        
        // Determine if slope is upward or downward based on movement direction
        if (bIsMoving)
        {
            FVector MovementDirection = OwnerCharacter->GetVelocity().GetSafeNormal();
            FVector SlopeDirection = FVector::CrossProduct(GroundNormal, FVector::RightVector).GetSafeNormal();
            
            if (FVector::DotProduct(MovementDirection, SlopeDirection) < 0.0f)
            {
                SlopeAngle = -SlopeAngle; // Negative for downward slope
            }
        }
    }
    else
    {
        SlopeAngle = 0.0f;
    }
}

void UAnim_BlendSpaceController::CalculateTurnRate(float DeltaTime)
{
    if (!OwnerCharacter || DeltaTime <= 0.0f)
    {
        TurnRate = 0.0f;
        return;
    }

    // Calculate yaw difference
    float CurrentYaw = OwnerCharacter->GetActorRotation().Yaw;
    float YawDifference = CurrentYaw - PreviousYaw;
    
    // Handle wrap-around
    if (YawDifference > 180.0f)
    {
        YawDifference -= 360.0f;
    }
    else if (YawDifference < -180.0f)
    {
        YawDifference += 360.0f;
    }
    
    // Calculate turn rate (degrees per second)
    float TargetTurnRate = YawDifference / DeltaTime;
    
    // Smooth interpolation
    TurnRate = FMath::FInterpTo(TurnRate, TargetTurnRate, DeltaTime, TurnRateSmoothingRate);
}

void UAnim_BlendSpaceController::UpdateMovementState()
{
    if (!MovementComponent)
    {
        return;
    }

    // Determine movement state based on speed and movement flags
    if (bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            CurrentMovementState = EAnim_MovementState::Jumping;
        }
        else
        {
            CurrentMovementState = EAnim_MovementState::Falling;
        }
    }
    else if (bIsCrouching)
    {
        CurrentMovementState = EAnim_MovementState::Crouching;
    }
    else if (bIsMoving)
    {
        // Determine walk/run/sprint based on speed
        float MaxWalkSpeed = MovementComponent->MaxWalkSpeed;
        
        if (Speed < MaxWalkSpeed * 0.3f)
        {
            CurrentMovementState = EAnim_MovementState::Walking;
        }
        else if (Speed < MaxWalkSpeed * 0.8f)
        {
            CurrentMovementState = EAnim_MovementState::Running;
        }
        else
        {
            CurrentMovementState = EAnim_MovementState::Sprinting;
        }
    }
    else
    {
        CurrentMovementState = EAnim_MovementState::Idle;
    }
}

void UAnim_BlendSpaceController::UpdateSurvivalState()
{
    // Update survival state based on character stats
    // This would typically read from a survival component or game state
    
    // For now, use simple thresholds
    if (HealthPercent < 25.0f)
    {
        CurrentSurvivalState = EAnim_SurvivalState::Injured;
    }
    else if (StaminaPercent < 20.0f)
    {
        CurrentSurvivalState = EAnim_SurvivalState::Tired;
    }
    else if (FearLevel > 70.0f)
    {
        CurrentSurvivalState = EAnim_SurvivalState::Afraid;
    }
    else if (HungerLevel > 80.0f)
    {
        CurrentSurvivalState = EAnim_SurvivalState::Hungry;
    }
    else if (ThirstLevel > 80.0f)
    {
        CurrentSurvivalState = EAnim_SurvivalState::Thirsty;
    }
    else if (FearLevel > 30.0f)
    {
        CurrentSurvivalState = EAnim_SurvivalState::Cautious;
    }
    else
    {
        CurrentSurvivalState = EAnim_SurvivalState::Normal;
    }
}

void UAnim_BlendSpaceController::SetMovementState(EAnim_MovementState NewState)
{
    CurrentMovementState = NewState;
}

void UAnim_BlendSpaceController::SetSurvivalState(EAnim_SurvivalState NewState)
{
    CurrentSurvivalState = NewState;
}

bool UAnim_BlendSpaceController::ShouldPlayIdleAnimation() const
{
    return CurrentMovementState == EAnim_MovementState::Idle && !bIsInAir;
}

bool UAnim_BlendSpaceController::ShouldPlayMovementAnimation() const
{
    return bIsMoving && !bIsInAir;
}

bool UAnim_BlendSpaceController::ShouldPlayTurnInPlaceAnimation() const
{
    return !bIsMoving && FMath::Abs(TurnRate) > 45.0f; // Turn rate threshold
}