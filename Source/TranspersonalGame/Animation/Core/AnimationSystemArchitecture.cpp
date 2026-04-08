#include "AnimationSystemArchitecture.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Initialize default values
    Velocity = FVector::ZeroVector;
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsMoving = false;
    
    LeftFootIKOffset = 0.0f;
    RightFootIKOffset = 0.0f;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Cache references
    OwningPawn = TryGetPawnOwner();
    if (OwningPawn)
    {
        MovementComponent = OwningPawn->FindComponentByClass<UCharacterMovementComponent>();
    }
    
    // Initialize personality-based multipliers
    UpdatePersonalityInfluence();
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningPawn || !MovementComponent)
    {
        return;
    }
    
    // Update core animation data
    UpdateMovementData();
    UpdateTerrainAdaptation();
    UpdatePersonalityInfluence();
}

void UTranspersonalAnimInstance::UpdateMovementData()
{
    if (!OwningPawn || !MovementComponent)
    {
        return;
    }
    
    // Get movement data
    Velocity = OwningPawn->GetVelocity();
    Speed = Velocity.Size();
    bIsMoving = Speed > 3.0f; // Threshold for considering movement
    bIsInAir = MovementComponent->IsFalling();
    
    // Calculate movement direction relative to actor rotation
    if (bIsMoving)
    {
        FVector ForwardVector = OwningPawn->GetActorForwardVector();
        FVector VelocityDirection = Velocity.GetSafeNormal();
        
        // Get angle between forward vector and velocity
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
        
        Direction = FMath::Atan2(CrossProduct, DotProduct);
        Direction = FMath::RadiansToDegrees(Direction);
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Apply personality influence to movement data
    Speed *= PersonalitySpeedMultiplier;
}

void UTranspersonalAnimInstance::UpdateTerrainAdaptation()
{
    if (!OwningPawn)
    {
        return;
    }
    
    // Calculate IK for both feet
    CalculateFootIK(TEXT("foot_l"), LeftFootIKOffset, LeftFootIKRotation);
    CalculateFootIK(TEXT("foot_r"), RightFootIKOffset, RightFootIKRotation);
}

void UTranspersonalAnimInstance::UpdatePersonalityInfluence()
{
    // Calculate speed multiplier based on personality
    float ConfidenceInfluence = FMath::Lerp(0.85f, 1.15f, Personality.Confidence);
    float FatigueInfluence = FMath::Lerp(1.0f, 0.7f, Personality.Fatigue);
    float FearInfluence = FMath::Lerp(1.0f, 1.3f, Personality.Fear); // Fear can make movement faster but less controlled
    
    PersonalitySpeedMultiplier = ConfidenceInfluence * FatigueInfluence * FearInfluence;
    PersonalitySpeedMultiplier = FMath::Clamp(PersonalitySpeedMultiplier, 0.5f, 1.5f);
    
    // Calculate animation scale based on personality
    float AlertnessInfluence = FMath::Lerp(0.9f, 1.1f, Personality.Alertness);
    float AggressionInfluence = FMath::Lerp(0.95f, 1.05f, Personality.Aggression);
    
    PersonalityAnimationScale = AlertnessInfluence * AggressionInfluence;
    PersonalityAnimationScale = FMath::Clamp(PersonalityAnimationScale, 0.8f, 1.2f);
}

void UTranspersonalAnimInstance::CalculateFootIK(const FName& FootBoneName, float& OutOffset, FRotator& OutRotation)
{
    if (!OwningPawn)
    {
        OutOffset = 0.0f;
        OutRotation = FRotator::ZeroRotator;
        return;
    }
    
    // Get foot location in world space
    FVector FootLocation = GetFootLocation(FootBoneName);
    
    // Trace downward from foot location
    FVector TraceStart = FootLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = FootLocation - FVector(0, 0, 100.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningPawn);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        // Calculate offset
        float TargetZ = HitResult.Location.Z;
        float CurrentZ = FootLocation.Z;
        OutOffset = TargetZ - CurrentZ;
        
        // Calculate rotation to match surface normal
        FVector SurfaceNormal = HitResult.Normal;
        FVector ForwardVector = OwningPawn->GetActorForwardVector();
        FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
        
        OutRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, SurfaceNormal);
        OutRotation = UKismetMathLibrary::NormalizedDeltaRotator(OutRotation, OwningPawn->GetActorRotation());
    }
    else
    {
        OutOffset = 0.0f;
        OutRotation = FRotator::ZeroRotator;
    }
    
    // Clamp offset to reasonable values
    OutOffset = FMath::Clamp(OutOffset, -50.0f, 50.0f);
}

FVector UTranspersonalAnimInstance::GetFootLocation(const FName& FootBoneName) const
{
    if (!GetSkelMeshComponent())
    {
        return FVector::ZeroVector;
    }
    
    return GetSkelMeshComponent()->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
}

void UTranspersonalAnimInstance::SetPersonality(const FAnimationPersonality& NewPersonality)
{
    Personality = NewPersonality;
    UpdatePersonalityInfluence();
}

float UTranspersonalAnimInstance::GetContextualBlendTime(const FString& Context) const
{
    if (const float* FoundBlendTime = MotionMatchingConfig.ContextBlendTimes.Find(Context))
    {
        return *FoundBlendTime;
    }
    
    return MotionMatchingConfig.DefaultBlendTime;
}

void UTranspersonalAnimInstance::SetMotionMatchingDatabase(UPoseSearchDatabase* NewDatabase)
{
    MotionMatchingConfig.PoseDatabase = NewDatabase;
}