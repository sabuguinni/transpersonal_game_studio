#include "Anim_BlendSpaceController.h"
#include "Anim_StateManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_BlendSpaceController::UAnim_BlendSpaceController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize blend data
    CurrentBlendData = FAnim_BlendSpaceData();
    TargetBlendData = FAnim_BlendSpaceData();
    Settings = FAnim_BlendSpaceSettings();
    
    // Initialize previous frame data
    PreviousVelocity = FVector::ZeroVector;
    PreviousRotation = FRotator::ZeroRotator;
    PreviousSpeed = 0.0f;
    
    bSmoothingEnabled = true;
}

void UAnim_BlendSpaceController::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponentReferences();
    
    if (OwnerCharacter)
    {
        PreviousRotation = OwnerCharacter->GetActorRotation();
    }
}

void UAnim_BlendSpaceController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Process any active overrides
    ProcessOverrides();
    
    // Update target blend data based on current movement
    UpdateTargetBlendData();
    
    // Smooth towards target values
    if (bSmoothingEnabled)
    {
        SmoothBlendData(DeltaTime);
    }
    else
    {
        CurrentBlendData = TargetBlendData;
    }
    
    // Store current frame data for next frame calculations
    PreviousVelocity = MovementComponent->Velocity;
    PreviousRotation = OwnerCharacter->GetActorRotation();
    PreviousSpeed = CurrentBlendData.Speed;
}

void UAnim_BlendSpaceController::CacheComponentReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        StateManager = OwnerCharacter->FindComponentByClass<UAnim_StateManager>();
    }
}

void UAnim_BlendSpaceController::UpdateTargetBlendData()
{
    UpdateSpeedAndDirection();
    UpdateLeanAndTurn(GetWorld()->GetDeltaSeconds());
    UpdateSlope();
}

void UAnim_BlendSpaceController::UpdateSpeedAndDirection()
{
    if (!MovementComponent)
    {
        return;
    }
    
    FVector Velocity = MovementComponent->Velocity;
    FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    
    // Update speed (unless overridden)
    if (!bSpeedOverride)
    {
        TargetBlendData.Speed = Velocity.Size();
    }
    
    // Update direction (unless overridden)
    if (!bDirectionOverride)
    {
        TargetBlendData.Direction = CalculateDirectionFromVelocity(Velocity, ActorRotation);
    }
}

float UAnim_BlendSpaceController::CalculateDirectionFromVelocity(const FVector& Velocity, const FRotator& ActorRotation) const
{
    if (Velocity.SizeSquared() < 1.0f)
    {
        return 0.0f; // No movement, no direction
    }
    
    // Convert velocity to local space
    FVector LocalVelocity = ActorRotation.UnrotateVector(Velocity);
    
    // Calculate angle from forward direction
    float Angle = FMath::Atan2(LocalVelocity.Y, LocalVelocity.X);
    
    // Convert to degrees and normalize to -180 to 180 range
    float DirectionDegrees = FMath::RadiansToDegrees(Angle);
    
    return DirectionDegrees;
}

void UAnim_BlendSpaceController::UpdateLeanAndTurn(float DeltaTime)
{
    if (!MovementComponent || DeltaTime <= 0.0f)
    {
        return;
    }
    
    // Calculate lean from acceleration
    FVector Acceleration = (MovementComponent->Velocity - PreviousVelocity) / DeltaTime;
    TargetBlendData.Lean = CalculateLeanFromAcceleration(Acceleration);
    
    // Calculate turn rate
    FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
    TargetBlendData.Turn = CalculateTurnRate(CurrentRotation, PreviousRotation, DeltaTime);
}

float UAnim_BlendSpaceController::CalculateLeanFromAcceleration(const FVector& Acceleration) const
{
    if (Acceleration.SizeSquared() < 1.0f)
    {
        return 0.0f;
    }
    
    // Convert acceleration to local space
    FVector LocalAcceleration = OwnerCharacter->GetActorRotation().UnrotateVector(Acceleration);
    
    // Calculate lean angle from lateral acceleration
    float LateralAcceleration = LocalAcceleration.Y;
    float LeanAngle = FMath::Atan2(LateralAcceleration, 980.0f); // Gravity as reference
    
    // Convert to degrees and clamp
    float LeanDegrees = FMath::RadiansToDegrees(LeanAngle);
    return FMath::Clamp(LeanDegrees, -Settings.MaxLeanAngle, Settings.MaxLeanAngle);
}

float UAnim_BlendSpaceController::CalculateTurnRate(const FRotator& CurrentRotation, const FRotator& PreviousRotation, float DeltaTime) const
{
    if (DeltaTime <= 0.0f)
    {
        return 0.0f;
    }
    
    // Calculate yaw difference
    float YawDifference = CurrentRotation.Yaw - PreviousRotation.Yaw;
    
    // Normalize to -180 to 180 range
    while (YawDifference > 180.0f) YawDifference -= 360.0f;
    while (YawDifference < -180.0f) YawDifference += 360.0f;
    
    // Calculate turn rate in degrees per second
    float TurnRate = YawDifference / DeltaTime;
    
    // Clamp to maximum turn rate
    return FMath::Clamp(TurnRate, -Settings.MaxTurnRate, Settings.MaxTurnRate);
}

void UAnim_BlendSpaceController::UpdateSlope()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Trace forward to detect slope
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector EndLocation = StartLocation + (ForwardVector * Settings.SlopeDetectionDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        // Calculate slope angle from hit normal
        FVector GroundNormal = HitResult.Normal;
        float SlopeAngle = FMath::Acos(FVector::DotProduct(GroundNormal, FVector::UpVector));
        TargetBlendData.Slope = FMath::RadiansToDegrees(SlopeAngle);
    }
    else
    {
        TargetBlendData.Slope = 0.0f;
    }
}

void UAnim_BlendSpaceController::SmoothBlendData(float DeltaTime)
{
    // Smooth each parameter with its own rate
    CurrentBlendData.Speed = SmoothDamp(CurrentBlendData.Speed, TargetBlendData.Speed, Settings.SpeedSmoothingRate, DeltaTime);
    CurrentBlendData.Direction = SmoothDamp(CurrentBlendData.Direction, TargetBlendData.Direction, Settings.DirectionSmoothingRate, DeltaTime);
    CurrentBlendData.Lean = SmoothDamp(CurrentBlendData.Lean, TargetBlendData.Lean, Settings.LeanSmoothingRate, DeltaTime);
    CurrentBlendData.Turn = SmoothDamp(CurrentBlendData.Turn, TargetBlendData.Turn, Settings.TurnSmoothingRate, DeltaTime);
    CurrentBlendData.Slope = SmoothDamp(CurrentBlendData.Slope, TargetBlendData.Slope, Settings.SpeedSmoothingRate, DeltaTime);
}

float UAnim_BlendSpaceController::SmoothDamp(float Current, float Target, float SmoothTime, float DeltaTime) const
{
    if (SmoothTime <= 0.0f || DeltaTime <= 0.0f)
    {
        return Target;
    }
    
    float Alpha = FMath::Clamp(DeltaTime / SmoothTime, 0.0f, 1.0f);
    return FMath::Lerp(Current, Target, Alpha);
}

void UAnim_BlendSpaceController::ProcessOverrides()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check speed override
    if (bSpeedOverride)
    {
        if (SpeedOverrideEndTime > 0.0f && CurrentTime >= SpeedOverrideEndTime)
        {
            bSpeedOverride = false;
            SpeedOverrideEndTime = 0.0f;
        }
        else
        {
            TargetBlendData.Speed = SpeedOverrideValue;
        }
    }
    
    // Check direction override
    if (bDirectionOverride)
    {
        if (DirectionOverrideEndTime > 0.0f && CurrentTime >= DirectionOverrideEndTime)
        {
            bDirectionOverride = false;
            DirectionOverrideEndTime = 0.0f;
        }
        else
        {
            TargetBlendData.Direction = DirectionOverrideValue;
        }
    }
}

void UAnim_BlendSpaceController::SetSpeedOverride(float Speed, float Duration)
{
    bSpeedOverride = true;
    SpeedOverrideValue = Speed;
    
    if (Duration > 0.0f)
    {
        SpeedOverrideEndTime = GetWorld()->GetTimeSeconds() + Duration;
    }
    else
    {
        SpeedOverrideEndTime = 0.0f; // Permanent until cleared
    }
}

void UAnim_BlendSpaceController::SetDirectionOverride(float Direction, float Duration)
{
    bDirectionOverride = true;
    DirectionOverrideValue = Direction;
    
    if (Duration > 0.0f)
    {
        DirectionOverrideEndTime = GetWorld()->GetTimeSeconds() + Duration;
    }
    else
    {
        DirectionOverrideEndTime = 0.0f; // Permanent until cleared
    }
}

void UAnim_BlendSpaceController::ClearOverrides()
{
    bSpeedOverride = false;
    bDirectionOverride = false;
    SpeedOverrideEndTime = 0.0f;
    DirectionOverrideEndTime = 0.0f;
}

void UAnim_BlendSpaceController::SetBlendSpaceSettings(const FAnim_BlendSpaceSettings& NewSettings)
{
    Settings = NewSettings;
}

void UAnim_BlendSpaceController::ResetBlendData()
{
    CurrentBlendData = FAnim_BlendSpaceData();
    TargetBlendData = FAnim_BlendSpaceData();
}

void UAnim_BlendSpaceController::SetSmoothingEnabled(bool bEnabled)
{
    bSmoothingEnabled = bEnabled;
}