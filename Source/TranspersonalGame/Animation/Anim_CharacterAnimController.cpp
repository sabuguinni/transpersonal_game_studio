#include "Anim_CharacterAnimController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_CharacterAnimController::UAnim_CharacterAnimController()
{
    // Initialize movement variables
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    bIsCrouching = false;
    
    // Initialize combat variables
    bIsHoldingWeapon = false;
    CurrentWeaponType = EAnim_WeaponType::None;
    bIsAttacking = false;
    
    // Initialize survival variables
    HealthPercentage = 100.0f;
    StaminaPercentage = 100.0f;
    bIsInjured = false;
    EmotionalState = EAnim_EmotionalState::Calm;
    
    // Initialize IK variables
    LeftFootIKOffset = 0.0f;
    RightFootIKOffset = 0.0f;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    
    // Initialize motion matching variables
    MotionMatchingWeight = 1.0f;
    DesiredVelocity = FVector::ZeroVector;
    TurnInPlaceAngle = 0.0f;
    
    // IK Settings
    IKTraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    
    // Motion Matching Settings
    MotionMatchingThreshold = 0.8f;
    VelocityMatchWeight = 0.7f;
    DirectionMatchWeight = 0.3f;
    
    // Animation Smoothing
    SpeedSmoothingRate = 10.0f;
    DirectionSmoothingRate = 8.0f;
}

void UAnim_CharacterAnimController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        CharacterMovement = OwningCharacter->GetCharacterMovement();
    }
}

void UAnim_CharacterAnimController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Update all animation states
    UpdateMovementState();
    UpdateCombatState();
    UpdateSurvivalState();
    CalculateFootIK();
    UpdateMotionMatching();
}

void UAnim_CharacterAnimController::UpdateMovementState()
{
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Calculate speed
    FVector Velocity = OwningCharacter->GetVelocity();
    float NewSpeed = Velocity.Size2D();
    Speed = FMath::FInterpTo(Speed, NewSpeed, GetWorld()->GetDeltaSeconds(), SpeedSmoothingRate);
    
    // Calculate direction
    if (Speed > 0.1f)
    {
        FVector Forward = OwningCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        float DotProduct = FVector::DotProduct(Forward, VelocityNormalized);
        float CrossProduct = FVector::CrossProduct(Forward, VelocityNormalized).Z;
        
        float NewDirection = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
        Direction = FMath::FInterpTo(Direction, NewDirection, GetWorld()->GetDeltaSeconds(), DirectionSmoothingRate);
    }
    
    // Update movement flags
    bIsInAir = CharacterMovement->IsFalling();
    bIsAccelerating = CharacterMovement->GetCurrentAcceleration().SizeSquared() > 0.0f;
    bIsCrouching = CharacterMovement->IsCrouching();
    
    // Update desired velocity for motion matching
    DesiredVelocity = CharacterMovement->GetCurrentAcceleration().GetSafeNormal() * CharacterMovement->GetMaxSpeed();
}

void UAnim_CharacterAnimController::UpdateCombatState()
{
    // This would typically check weapon component or combat system
    // For now, using placeholder logic
    
    // Check if character is holding a weapon (placeholder)
    // bIsHoldingWeapon = OwningCharacter->HasWeapon();
    
    // Update weapon type based on equipped weapon
    // CurrentWeaponType = OwningCharacter->GetWeaponType();
    
    // Check if currently attacking
    // bIsAttacking = OwningCharacter->IsAttacking();
}

void UAnim_CharacterAnimController::UpdateSurvivalState()
{
    // This would typically interface with survival stats component
    // For now, using placeholder logic
    
    // Get health percentage from character
    // HealthPercentage = OwningCharacter->GetHealthPercentage();
    
    // Get stamina percentage
    // StaminaPercentage = OwningCharacter->GetStaminaPercentage();
    
    // Determine if injured
    bIsInjured = HealthPercentage < 50.0f;
    
    // Update emotional state based on survival conditions
    if (HealthPercentage < 25.0f)
    {
        EmotionalState = EAnim_EmotionalState::Panicked;
    }
    else if (HealthPercentage < 50.0f)
    {
        EmotionalState = EAnim_EmotionalState::Fearful;
    }
    else if (StaminaPercentage < 30.0f)
    {
        EmotionalState = EAnim_EmotionalState::Tired;
    }
    else
    {
        EmotionalState = EAnim_EmotionalState::Calm;
    }
}

void UAnim_CharacterAnimController::CalculateFootIK()
{
    if (!OwningCharacter)
    {
        return;
    }
    
    // Calculate IK for left foot
    float NewLeftFootOffset = GetFootIKOffset(TEXT("foot_l"), IKTraceDistance);
    LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, NewLeftFootOffset, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
    LeftFootIKRotation = GetFootIKRotation(TEXT("foot_l"));
    
    // Calculate IK for right foot
    float NewRightFootOffset = GetFootIKOffset(TEXT("foot_r"), IKTraceDistance);
    RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, NewRightFootOffset, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
    RightFootIKRotation = GetFootIKRotation(TEXT("foot_r"));
}

float UAnim_CharacterAnimController::GetFootIKOffset(FName SocketName, float TraceDistance)
{
    if (!OwningCharacter)
    {
        return 0.0f;
    }
    
    USkeletalMeshComponent* MeshComp = OwningCharacter->GetMesh();
    if (!MeshComp)
    {
        return 0.0f;
    }
    
    // Get foot socket location
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    FVector StartTrace = SocketLocation;
    FVector EndTrace = StartTrace - FVector(0, 0, TraceDistance);
    
    // Perform line trace
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartTrace,
        EndTrace,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        // Calculate offset needed to place foot on ground
        float DistanceToGround = (StartTrace - HitResult.Location).Z;
        return DistanceToGround - (TraceDistance * 0.5f);
    }
    
    return 0.0f;
}

FRotator UAnim_CharacterAnimController::GetFootIKRotation(FName SocketName)
{
    if (!OwningCharacter)
    {
        return FRotator::ZeroRotator;
    }
    
    USkeletalMeshComponent* MeshComp = OwningCharacter->GetMesh();
    if (!MeshComp)
    {
        return FRotator::ZeroRotator;
    }
    
    // Get foot socket location
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    FVector StartTrace = SocketLocation;
    FVector EndTrace = StartTrace - FVector(0, 0, IKTraceDistance);
    
    // Perform line trace
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartTrace,
        EndTrace,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        // Calculate rotation to align foot with surface normal
        FVector SurfaceNormal = HitResult.Normal;
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
        
        return UKismetMathLibrary::MakeRotFromXZ(ForwardVector, SurfaceNormal);
    }
    
    return FRotator::ZeroRotator;
}

void UAnim_CharacterAnimController::UpdateMotionMatching()
{
    if (!OwningCharacter)
    {
        return;
    }
    
    // Calculate current velocity
    FVector CurrentVelocity = OwningCharacter->GetVelocity();
    
    // Calculate motion matching score
    float MatchScore = CalculateMotionMatchingScore(CurrentVelocity, DesiredVelocity);
    
    // Update motion matching weight based on score
    MotionMatchingWeight = FMath::Clamp(MatchScore, 0.0f, 1.0f);
    
    // Calculate turn in place angle
    if (Speed < 10.0f && bIsAccelerating)
    {
        FVector InputDirection = CharacterMovement->GetCurrentAcceleration().GetSafeNormal();
        FVector ForwardDirection = OwningCharacter->GetActorForwardVector();
        
        float DotProduct = FVector::DotProduct(ForwardDirection, InputDirection);
        float CrossProduct = FVector::CrossProduct(ForwardDirection, InputDirection).Z;
        
        TurnInPlaceAngle = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
    }
    else
    {
        TurnInPlaceAngle = 0.0f;
    }
}

float UAnim_CharacterAnimController::CalculateMotionMatchingScore(const FVector& CurrentVelocity, const FVector& TargetVelocity)
{
    // Calculate velocity similarity
    float VelocityDifference = FVector::Dist(CurrentVelocity, TargetVelocity);
    float MaxVelocity = FMath::Max(CurrentVelocity.Size(), TargetVelocity.Size());
    float VelocityScore = MaxVelocity > 0.0f ? (1.0f - (VelocityDifference / MaxVelocity)) : 1.0f;
    
    // Calculate direction similarity
    float DirectionScore = 1.0f;
    if (CurrentVelocity.SizeSquared() > 0.0f && TargetVelocity.SizeSquared() > 0.0f)
    {
        DirectionScore = FVector::DotProduct(CurrentVelocity.GetSafeNormal(), TargetVelocity.GetSafeNormal());
        DirectionScore = (DirectionScore + 1.0f) * 0.5f; // Normalize to 0-1 range
    }
    
    // Combine scores with weights
    float FinalScore = (VelocityScore * VelocityMatchWeight) + (DirectionScore * DirectionMatchWeight);
    
    return FMath::Clamp(FinalScore, 0.0f, 1.0f);
}