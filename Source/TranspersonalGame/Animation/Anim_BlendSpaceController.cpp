#include "Anim_BlendSpaceController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_BlendSpaceController::UAnim_BlendSpaceController()
{
    // Initialize default values
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    GroundSpeed = 0.0f;
    bShouldMove = false;
    SmoothedSpeed = 0.0f;
    SmoothedDirection = 0.0f;
    SmoothedLeanAngle = 0.0f;
    
    // Initialize blend space data
    BlendSpaceData = FAnim_BlendSpaceData();
    BlendSpaceSettings = FAnim_BlendSpaceSettings();
}

void UAnim_BlendSpaceController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    UpdateCharacterReferences();
    
    if (OwnerCharacter)
    {
        UE_LOG(LogTemp, Log, TEXT("Anim_BlendSpaceController: Initialized for character %s"), 
               *OwnerCharacter->GetName());
    }
}

void UAnim_BlendSpaceController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        UpdateCharacterReferences();
        return;
    }
    
    UpdateMovementData();
    UpdateBlendSpaceValues(DeltaTimeX);
    SmoothValues(DeltaTimeX);
}

void UAnim_BlendSpaceController::UpdateCharacterReferences()
{
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UAnim_BlendSpaceController::UpdateMovementData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get current movement data
    Velocity = OwnerCharacter->GetVelocity();
    Acceleration = MovementComponent->GetCurrentAcceleration();
    GroundSpeed = Velocity.Size2D();
    bShouldMove = ShouldCharacterMove();
    
    // Update movement state flags
    BlendSpaceData.bIsMoving = bShouldMove && GroundSpeed > BlendSpaceSettings.MinMovementSpeed;
    BlendSpaceData.bIsFalling = MovementComponent->IsFalling();
    BlendSpaceData.bIsJumping = MovementComponent->IsMovingOnGround() == false && Velocity.Z > 0.0f;
}

void UAnim_BlendSpaceController::UpdateBlendSpaceValues(float DeltaTime)
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    CalculateSpeedAndDirection();
    CalculateLeanAngle();
}

void UAnim_BlendSpaceController::CalculateSpeedAndDirection()
{
    if (!OwnerCharacter)
    {
        BlendSpaceData.Speed = 0.0f;
        BlendSpaceData.Direction = 0.0f;
        return;
    }
    
    // Calculate speed (normalized to character's max walk speed)
    float MaxWalkSpeed = MovementComponent ? MovementComponent->MaxWalkSpeed : 600.0f;
    BlendSpaceData.Speed = FMath::Clamp(GroundSpeed / MaxWalkSpeed, 0.0f, 1.0f);
    
    // Calculate direction relative to character's forward vector
    if (BlendSpaceData.bIsMoving && GroundSpeed > BlendSpaceSettings.MinMovementSpeed)
    {
        BlendSpaceData.Direction = CalculateDirectionFromVelocity();
    }
    else
    {
        BlendSpaceData.Direction = 0.0f;
    }
}

void UAnim_BlendSpaceController::CalculateLeanAngle()
{
    if (!OwnerCharacter || !BlendSpaceData.bIsMoving)
    {
        BlendSpaceData.LeanAngle = 0.0f;
        return;
    }
    
    BlendSpaceData.LeanAngle = CalculateLeanFromAcceleration();
}

float UAnim_BlendSpaceController::CalculateDirectionFromVelocity() const
{
    if (!OwnerCharacter || GroundSpeed < BlendSpaceSettings.MinMovementSpeed)
    {
        return 0.0f;
    }
    
    // Get character's forward and right vectors
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector RightVector = OwnerCharacter->GetActorRightVector();
    
    // Normalize velocity to 2D
    FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
    
    // Calculate dot products to determine direction
    float ForwardDot = FVector::DotProduct(NormalizedVelocity, ForwardVector);
    float RightDot = FVector::DotProduct(NormalizedVelocity, RightVector);
    
    // Convert to angle in degrees (-180 to 180)
    float Direction = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
    
    return Direction;
}

float UAnim_BlendSpaceController::CalculateLeanFromAcceleration() const
{
    if (!OwnerCharacter || !BlendSpaceSettings.bUseAcceleration)
    {
        return 0.0f;
    }
    
    // Get character's right vector
    FVector RightVector = OwnerCharacter->GetActorRightVector();
    
    // Calculate lean based on acceleration in right direction
    float RightAcceleration = FVector::DotProduct(Acceleration, RightVector);
    
    // Normalize and clamp to max lean angle
    float LeanAngle = (RightAcceleration / 1000.0f) * BlendSpaceSettings.MaxLeanAngle;
    LeanAngle = FMath::Clamp(LeanAngle, -BlendSpaceSettings.MaxLeanAngle, BlendSpaceSettings.MaxLeanAngle);
    
    return LeanAngle;
}

void UAnim_BlendSpaceController::SmoothValues(float DeltaTime)
{
    // Smooth speed
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, BlendSpaceData.Speed, 
                                   DeltaTime, BlendSpaceSettings.SpeedSmoothingRate);
    
    // Smooth direction
    SmoothedDirection = FMath::FInterpTo(SmoothedDirection, BlendSpaceData.Direction, 
                                       DeltaTime, BlendSpaceSettings.DirectionSmoothingRate);
    
    // Smooth lean angle
    SmoothedLeanAngle = FMath::FInterpTo(SmoothedLeanAngle, BlendSpaceData.LeanAngle, 
                                       DeltaTime, BlendSpaceSettings.LeanSmoothingRate);
    
    // Update blend space data with smoothed values
    BlendSpaceData.Speed = SmoothedSpeed;
    BlendSpaceData.Direction = SmoothedDirection;
    BlendSpaceData.LeanAngle = SmoothedLeanAngle;
}

bool UAnim_BlendSpaceController::ShouldCharacterMove() const
{
    if (!MovementComponent)
    {
        return false;
    }
    
    // Character should move if:
    // 1. Has input acceleration
    // 2. Is not falling (unless has horizontal velocity)
    // 3. Has sufficient ground speed
    
    bool bHasAcceleration = !Acceleration.IsNearlyZero();
    bool bHasVelocity = GroundSpeed > BlendSpaceSettings.MinMovementSpeed;
    bool bIsGrounded = MovementComponent->IsMovingOnGround();
    
    return (bHasAcceleration || bHasVelocity) && (bIsGrounded || bHasVelocity);
}

void UAnim_BlendSpaceController::SetBlendSpaceSettings(const FAnim_BlendSpaceSettings& NewSettings)
{
    BlendSpaceSettings = NewSettings;
    
    UE_LOG(LogTemp, Log, TEXT("Anim_BlendSpaceController: Updated blend space settings"));
}

void UAnim_BlendSpaceController::LogBlendSpaceData() const
{
    if (GEngine)
    {
        FString DebugString = FString::Printf(
            TEXT("BlendSpace - Speed: %.2f, Direction: %.2f, Lean: %.2f, Moving: %s, Falling: %s"),
            BlendSpaceData.Speed,
            BlendSpaceData.Direction,
            BlendSpaceData.LeanAngle,
            BlendSpaceData.bIsMoving ? TEXT("True") : TEXT("False"),
            BlendSpaceData.bIsFalling ? TEXT("True") : TEXT("False")
        );
        
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, DebugString);
    }
}