#include "Anim_MotionMatchingSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize motion matching parameters
    MotionMatchingThreshold = 0.8f;
    BlendTime = 0.3f;
    
    // Initialize IK parameters
    IKTraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    IKTraceChannel = ECC_WorldStatic;
    
    // Initialize state
    TimeSinceLastMotionUpdate = 0.0f;
    bIKSystemEnabled = true;
    PreviousVelocity = FVector::ZeroVector;
    
    // Initialize component references
    OwnerMeshComponent = nullptr;
    OwnerMovementComponent = nullptr;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get component references
    if (AActor* Owner = GetOwner())
    {
        OwnerMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        OwnerMovementComponent = Owner->FindComponentByClass<UCharacterMovementComponent>();
        
        if (!OwnerMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("MotionMatchingSystem: No SkeletalMeshComponent found on owner"));
        }
        
        if (!OwnerMovementComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("MotionMatchingSystem: No CharacterMovementComponent found on owner"));
        }
    }
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerMeshComponent || !OwnerMovementComponent)
    {
        return;
    }
    
    // Update motion data
    FVector CurrentVelocity = OwnerMovementComponent->Velocity;
    bool bInAir = OwnerMovementComponent->IsFalling();
    bool bCrouching = OwnerMovementComponent->IsCrouching();
    
    UpdateMotionData(CurrentVelocity, bInAir, bCrouching);
    
    // Update IK system
    if (bIKSystemEnabled)
    {
        UpdateFootIK(DeltaTime);
    }
    
    TimeSinceLastMotionUpdate += DeltaTime;
}

void UAnim_MotionMatchingSystem::UpdateMotionData(const FVector& CurrentVelocity, bool bInAir, bool bCrouching)
{
    CurrentMotionData.Velocity = CurrentVelocity;
    CurrentMotionData.Speed = CurrentVelocity.Size();
    CurrentMotionData.bIsInAir = bInAir;
    CurrentMotionData.bIsCrouching = bCrouching;
    
    // Calculate direction relative to forward vector
    if (AActor* Owner = GetOwner())
    {
        FVector ForwardVector = Owner->GetActorForwardVector();
        FVector RightVector = Owner->GetActorRightVector();
        
        float ForwardDot = FVector::DotProduct(CurrentVelocity.GetSafeNormal(), ForwardVector);
        float RightDot = FVector::DotProduct(CurrentVelocity.GetSafeNormal(), RightVector);
        
        CurrentMotionData.Direction = FMath::Atan2(RightDot, ForwardDot) * 180.0f / PI;
    }
    
    // Determine movement state
    if (bInAir)
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Jumping;
    }
    else if (bCrouching)
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Crouching;
    }
    else if (CurrentMotionData.Speed > 400.0f)
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Running;
    }
    else if (CurrentMotionData.Speed > 50.0f)
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Walking;
    }
    else
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Idle;
    }
}

UAnimSequence* UAnim_MotionMatchingSystem::FindBestMatchingAnimation(const FAnim_MotionData& TargetMotion)
{
    if (MotionDatabase.Num() == 0)
    {
        return nullptr;
    }
    
    UAnimSequence* BestMatch = nullptr;
    float BestScore = -1.0f;
    
    for (UAnimSequence* AnimSeq : MotionDatabase)
    {
        if (!AnimSeq)
        {
            continue;
        }
        
        // For now, use a simple scoring system based on movement state
        // In a full implementation, this would analyze pose data from the animation
        float Score = CalculateMotionScore(CurrentMotionData, TargetMotion);
        
        if (Score > BestScore && Score >= MotionMatchingThreshold)
        {
            BestScore = Score;
            BestMatch = AnimSeq;
        }
    }
    
    return BestMatch;
}

float UAnim_MotionMatchingSystem::CalculateMotionScore(const FAnim_MotionData& CurrentMotion, const FAnim_MotionData& TargetMotion)
{
    float Score = 0.0f;
    
    // Speed similarity (0-1)
    float SpeedDiff = FMath::Abs(CurrentMotion.Speed - TargetMotion.Speed);
    float SpeedScore = FMath::Clamp(1.0f - (SpeedDiff / 600.0f), 0.0f, 1.0f);
    Score += SpeedScore * 0.4f;
    
    // Direction similarity (0-1)
    float DirectionDiff = FMath::Abs(CurrentMotion.Direction - TargetMotion.Direction);
    if (DirectionDiff > 180.0f)
    {
        DirectionDiff = 360.0f - DirectionDiff;
    }
    float DirectionScore = FMath::Clamp(1.0f - (DirectionDiff / 180.0f), 0.0f, 1.0f);
    Score += DirectionScore * 0.3f;
    
    // State match (0-1)
    float StateScore = (CurrentMotion.MovementState == TargetMotion.MovementState) ? 1.0f : 0.0f;
    Score += StateScore * 0.3f;
    
    return Score;
}

void UAnim_MotionMatchingSystem::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMeshComponent)
    {
        return;
    }
    
    // Update left foot IK
    FVector LeftFootLocation = OwnerMeshComponent->GetSocketLocation(TEXT("foot_l"));
    LeftFootIK = CalculateFootIK(LeftFootLocation, TEXT("foot_l"));
    
    // Update right foot IK
    FVector RightFootLocation = OwnerMeshComponent->GetSocketLocation(TEXT("foot_r"));
    RightFootIK = CalculateFootIK(RightFootLocation, TEXT("foot_r"));
    
    // Smooth IK transitions
    if (LeftFootIK.bIsValidIKTarget)
    {
        LeftFootIK.IKAlpha = FMath::FInterpTo(LeftFootIK.IKAlpha, 1.0f, DeltaTime, IKInterpSpeed);
    }
    else
    {
        LeftFootIK.IKAlpha = FMath::FInterpTo(LeftFootIK.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
    }
    
    if (RightFootIK.bIsValidIKTarget)
    {
        RightFootIK.IKAlpha = FMath::FInterpTo(RightFootIK.IKAlpha, 1.0f, DeltaTime, IKInterpSpeed);
    }
    else
    {
        RightFootIK.IKAlpha = FMath::FInterpTo(RightFootIK.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
    }
}

FAnim_IKFootData UAnim_MotionMatchingSystem::CalculateFootIK(const FVector& FootSocketLocation, const FName& FootBoneName)
{
    FAnim_IKFootData FootData;
    
    FVector HitLocation;
    FRotator SurfaceRotation;
    
    if (PerformFootTrace(FootSocketLocation, HitLocation, SurfaceRotation))
    {
        FootData.FootLocation = HitLocation;
        FootData.FootRotation = SurfaceRotation;
        FootData.bIsValidIKTarget = true;
        
        // Calculate distance for IK alpha
        float Distance = FVector::Dist(FootSocketLocation, HitLocation);
        FootData.IKAlpha = FMath::Clamp(Distance / IKTraceDistance, 0.0f, 1.0f);
    }
    else
    {
        FootData.bIsValidIKTarget = false;
        FootData.IKAlpha = 0.0f;
    }
    
    return FootData;
}

bool UAnim_MotionMatchingSystem::PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FRotator& OutSurfaceRotation)
{
    if (!GetWorld())
    {
        return false;
    }
    
    FVector TraceStart = StartLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = StartLocation - FVector(0, 0, IKTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        IKTraceChannel,
        QueryParams
    );
    
    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        
        // Calculate surface rotation from normal
        FVector Normal = HitResult.Normal;
        FVector Forward = FVector::CrossProduct(Normal, FVector::RightVector).GetSafeNormal();
        FVector Right = FVector::CrossProduct(Forward, Normal).GetSafeNormal();
        
        OutSurfaceRotation = UKismetMathLibrary::MakeRotationFromAxes(Forward, Right, Normal);
        
        // Debug visualization
        if (CVarShowDebugIK.GetValueOnGameThread())
        {
            DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, FColor::Green, false, 0.1f);
            DrawDebugSphere(GetWorld(), HitResult.Location, 2.0f, 8, FColor::Red, false, 0.1f);
        }
        
        return true;
    }
    
    return false;
}

void UAnim_MotionMatchingSystem::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMotionData.MovementState != NewState)
    {
        CurrentMotionData.MovementState = NewState;
        UE_LOG(LogTemp, Log, TEXT("Movement state changed to: %d"), (int32)NewState);
    }
}

void UAnim_MotionMatchingSystem::PlayAnimationMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage || !OwnerMeshComponent)
    {
        return;
    }
    
    if (UAnimInstance* AnimInstance = OwnerMeshComponent->GetAnimInstance())
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
    }
}

void UAnim_MotionMatchingSystem::StopAnimationMontage(UAnimMontage* Montage)
{
    if (!Montage || !OwnerMeshComponent)
    {
        return;
    }
    
    if (UAnimInstance* AnimInstance = OwnerMeshComponent->GetAnimInstance())
    {
        AnimInstance->Montage_Stop(0.3f, Montage);
    }
}