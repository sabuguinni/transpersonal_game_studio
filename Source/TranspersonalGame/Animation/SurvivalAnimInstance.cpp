#include "SurvivalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

USurvivalAnimInstance::USurvivalAnimInstance()
{
    // Initialize default values
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    bIsCrouching = false;
    bIsAiming = false;
    bIsCarryingObject = false;
    bIsFearful = false;
    bIsInjured = false;
    bIsExhausted = false;
    
    // Survival state defaults
    HealthPercentage = 1.0f;
    StaminaPercentage = 1.0f;
    FearLevel = 0.0f;
    
    // Movement thresholds
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 350.0f;
    
    // Animation blend times
    MovementBlendTime = 0.25f;
    StateTransitionTime = 0.15f;
    
    // IK settings
    bEnableFootIK = true;
    FootIKInterpSpeed = 15.0f;
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
}

void USurvivalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Cache character reference
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        CharacterMovement = OwningCharacter->GetCharacterMovement();
    }
}

void USurvivalAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Update basic movement values
    UpdateMovementValues();
    
    // Update survival states
    UpdateSurvivalStates();
    
    // Update foot IK if enabled
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTimeX);
    }
}

void USurvivalAnimInstance::UpdateMovementValues()
{
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = OwningCharacter->GetVelocity();
    Speed = Velocity.Size2D();
    
    // Calculate direction relative to character rotation
    if (Speed > 0.0f)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector RightVector = OwningCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
        
        float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        
        Direction = UKismetMathLibrary::DegAtan2(RightDot, ForwardDot);
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Check if character is in air
    bIsInAir = CharacterMovement->IsFalling();
    
    // Check if character is accelerating
    FVector Acceleration = CharacterMovement->GetCurrentAcceleration();
    bIsAccelerating = Acceleration.SizeSquared() > 0.0f;
    
    // Check if character is crouching
    bIsCrouching = CharacterMovement->IsCrouching();
}

void USurvivalAnimInstance::UpdateSurvivalStates()
{
    // This would typically interface with a survival component
    // For now, we'll use placeholder logic
    
    // Update fear level based on nearby threats (placeholder)
    // In a real implementation, this would check for nearby dinosaurs
    FearLevel = FMath::Clamp(FearLevel, 0.0f, 1.0f);
    bIsFearful = FearLevel > 0.3f;
    
    // Update health and stamina percentages (placeholder)
    // These would come from actual survival stats
    HealthPercentage = FMath::Clamp(HealthPercentage, 0.0f, 1.0f);
    StaminaPercentage = FMath::Clamp(StaminaPercentage, 0.0f, 1.0f);
    
    // Determine injury and exhaustion states
    bIsInjured = HealthPercentage < 0.5f;
    bIsExhausted = StaminaPercentage < 0.2f;
}

void USurvivalAnimInstance::UpdateFootIK(float DeltaTime)
{
    if (!OwningCharacter)
    {
        return;
    }
    
    // Perform line traces for foot IK
    FVector LeftFootTarget = CalculateFootIKOffset("foot_l", DeltaTime);
    FVector RightFootTarget = CalculateFootIKOffset("foot_r", DeltaTime);
    
    // Interpolate to smooth foot placement
    LeftFootIKOffset = FMath::VInterpTo(LeftFootIKOffset, LeftFootTarget, DeltaTime, FootIKInterpSpeed);
    RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, RightFootTarget, DeltaTime, FootIKInterpSpeed);
}

FVector USurvivalAnimInstance::CalculateFootIKOffset(const FString& FootBoneName, float DeltaTime)
{
    if (!OwningCharacter)
    {
        return FVector::ZeroVector;
    }
    
    // Get foot bone location in world space
    USkeletalMeshComponent* MeshComp = OwningCharacter->GetMesh();
    if (!MeshComp)
    {
        return FVector::ZeroVector;
    }
    
    FVector FootLocation = MeshComp->GetBoneLocation(*FootBoneName, EBoneSpaces::WorldSpace);
    
    // Perform line trace downward from foot
    FVector TraceStart = FootLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = FootLocation - FVector(0, 0, 100.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        // Calculate offset needed to place foot on ground
        float GroundZ = HitResult.Location.Z;
        float FootZ = FootLocation.Z;
        float Offset = GroundZ - FootZ;
        
        // Limit the offset to reasonable values
        Offset = FMath::Clamp(Offset, -50.0f, 50.0f);
        
        return FVector(0, 0, Offset);
    }
    
    return FVector::ZeroVector;
}

void USurvivalAnimInstance::SetCarryingObject(bool bCarrying)
{
    bIsCarryingObject = bCarrying;
}

void USurvivalAnimInstance::SetAiming(bool bAiming)
{
    bIsAiming = bAiming;
}

void USurvivalAnimInstance::SetFearLevel(float NewFearLevel)
{
    FearLevel = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);
    bIsFearful = FearLevel > 0.3f;
}

void USurvivalAnimInstance::SetHealthPercentage(float NewHealthPercentage)
{
    HealthPercentage = FMath::Clamp(NewHealthPercentage, 0.0f, 1.0f);
    bIsInjured = HealthPercentage < 0.5f;
}

void USurvivalAnimInstance::SetStaminaPercentage(float NewStaminaPercentage)
{
    StaminaPercentage = FMath::Clamp(NewStaminaPercentage, 0.0f, 1.0f);
    bIsExhausted = StaminaPercentage < 0.2f;
}

bool USurvivalAnimInstance::IsMoving() const
{
    return Speed > 5.0f;
}

bool USurvivalAnimInstance::IsWalking() const
{
    return Speed > 5.0f && Speed <= WalkSpeedThreshold;
}

bool USurvivalAnimInstance::IsRunning() const
{
    return Speed > WalkSpeedThreshold;
}

bool USurvivalAnimInstance::IsSprinting() const
{
    return Speed > RunSpeedThreshold;
}