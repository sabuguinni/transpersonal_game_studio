#include "Anim_BlendSpaceController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_BlendSpaceController::UAnim_BlendSpaceController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize smoothing rates
    SpeedSmoothingRate = 5.0f;
    DirectionSmoothingRate = 8.0f;
    LeanSmoothingRate = 3.0f;
    
    // Initialize thresholds
    MovementThreshold = 10.0f;
    RunningThreshold = 300.0f;
    
    // Initialize cached values
    PreviousSpeed = 0.0f;
    PreviousDirection = 0.0f;
    PreviousLean = 0.0f;
    
    OwnerCharacter = nullptr;
}

void UAnim_BlendSpaceController::BeginPlay()
{
    Super::BeginPlay();
    CacheOwnerCharacter();
}

void UAnim_BlendSpaceController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter)
    {
        CacheOwnerCharacter();
        return;
    }
    
    UpdateLocomotionFromCharacter();
    SmoothAllParameters(DeltaTime);
}

void UAnim_BlendSpaceController::UpdateBlendSpaceParameters(float Speed, float Direction, float Lean)
{
    BlendSpaceData.Speed = Speed;
    BlendSpaceData.Direction = Direction;
    BlendSpaceData.Lean = Lean;
}

void UAnim_BlendSpaceController::SetMovementBlendSpace(UBlendSpace* NewBlendSpace)
{
    BlendSpaceData.MovementBlendSpace = NewBlendSpace;
}

void UAnim_BlendSpaceController::SetSpeedBlendSpace(UBlendSpace1D* NewBlendSpace)
{
    BlendSpaceData.SpeedBlendSpace = NewBlendSpace;
}

FAnim_LocomotionParameters UAnim_BlendSpaceController::CalculateLocomotionParameters(const FVector& Velocity, const FRotator& ActorRotation)
{
    FAnim_LocomotionParameters Params;
    
    // Calculate velocity magnitude
    Params.Velocity = Velocity.Size();
    
    // Determine movement states
    Params.bIsMoving = Params.Velocity > MovementThreshold;
    Params.bIsRunning = Params.Velocity > RunningThreshold;
    Params.bIsInAir = false; // Will be set by character movement component
    
    // Calculate direction angle
    Params.DirectionAngle = CalculateDirectionAngle(Velocity, ActorRotation);
    
    // Calculate lean amount based on velocity change
    Params.LeanAmount = CalculateLeanAmount(Velocity, GetWorld()->GetDeltaSeconds());
    
    return Params;
}

float UAnim_BlendSpaceController::CalculateDirectionAngle(const FVector& Velocity, const FRotator& ActorRotation)
{
    if (Velocity.SizeSquared() < MovementThreshold * MovementThreshold)
    {
        return 0.0f;
    }
    
    // Get velocity direction in world space
    FVector VelocityDirection = Velocity.GetSafeNormal();
    
    // Get actor forward vector
    FVector ActorForward = ActorRotation.Vector();
    
    // Calculate angle between velocity and forward vector
    float DotProduct = FVector::DotProduct(VelocityDirection, ActorForward);
    float CrossProduct = FVector::CrossProduct(VelocityDirection, ActorForward).Z;
    
    // Convert to angle in degrees
    float Angle = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
    
    // Normalize to -180 to 180 range
    return FMath::ClampAngle(Angle, -180.0f, 180.0f);
}

float UAnim_BlendSpaceController::CalculateLeanAmount(const FVector& Velocity, float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return 0.0f;
    }
    
    // Calculate acceleration based on velocity change
    static FVector PreviousVelocity = FVector::ZeroVector;
    FVector Acceleration = (Velocity - PreviousVelocity) / DeltaTime;
    PreviousVelocity = Velocity;
    
    // Calculate lean based on lateral acceleration
    float LateralAcceleration = FVector::CrossProduct(Acceleration, FVector::UpVector).Size();
    
    // Scale and clamp lean amount
    float LeanAmount = LateralAcceleration / 1000.0f; // Scale factor
    return FMath::Clamp(LeanAmount, -1.0f, 1.0f);
}

void UAnim_BlendSpaceController::EvaluateBlendSpace(UBlendSpace* BlendSpace, float XValue, float YValue, TArray<FBlendSampleData>& OutSampleData)
{
    if (!BlendSpace)
    {
        OutSampleData.Empty();
        return;
    }
    
    // Create blend input
    FVector BlendInput(XValue, YValue, 0.0f);
    
    // Evaluate blend space
    BlendSpace->GetSamplesFromBlendInput(BlendInput, OutSampleData);
}

void UAnim_BlendSpaceController::EvaluateBlendSpace1D(UBlendSpace1D* BlendSpace, float Value, TArray<FBlendSampleData>& OutSampleData)
{
    if (!BlendSpace)
    {
        OutSampleData.Empty();
        return;
    }
    
    // Create blend input for 1D blend space
    FVector BlendInput(Value, 0.0f, 0.0f);
    
    // Evaluate 1D blend space
    BlendSpace->GetSamplesFromBlendInput(BlendInput, OutSampleData);
}

float UAnim_BlendSpaceController::SmoothParameter(float CurrentValue, float TargetValue, float SmoothingSpeed, float DeltaTime)
{
    if (SmoothingSpeed <= 0.0f)
    {
        return TargetValue;
    }
    
    return FMath::FInterpTo(CurrentValue, TargetValue, DeltaTime, SmoothingSpeed);
}

void UAnim_BlendSpaceController::SmoothAllParameters(float DeltaTime)
{
    // Smooth speed
    float TargetSpeed = LocomotionParams.Velocity;
    BlendSpaceData.Speed = SmoothParameter(BlendSpaceData.Speed, TargetSpeed, SpeedSmoothingRate, DeltaTime);
    PreviousSpeed = BlendSpaceData.Speed;
    
    // Smooth direction
    float TargetDirection = LocomotionParams.DirectionAngle;
    BlendSpaceData.Direction = SmoothParameter(BlendSpaceData.Direction, TargetDirection, DirectionSmoothingRate, DeltaTime);
    PreviousDirection = BlendSpaceData.Direction;
    
    // Smooth lean
    float TargetLean = LocomotionParams.LeanAmount;
    BlendSpaceData.Lean = SmoothParameter(BlendSpaceData.Lean, TargetLean, LeanSmoothingRate, DeltaTime);
    PreviousLean = BlendSpaceData.Lean;
}

void UAnim_BlendSpaceController::CacheOwnerCharacter()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UAnim_BlendSpaceController::UpdateLocomotionFromCharacter()
{
    if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    
    // Get current velocity and rotation
    FVector CurrentVelocity = MovementComp->Velocity;
    FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
    
    // Update locomotion parameters
    LocomotionParams = CalculateLocomotionParameters(CurrentVelocity, CurrentRotation);
    
    // Update air state from movement component
    LocomotionParams.bIsInAir = MovementComp->IsFalling();
    
    // Update blend space parameters with raw values
    UpdateBlendSpaceParameters(LocomotionParams.Velocity, LocomotionParams.DirectionAngle, LocomotionParams.LeanAmount);
}