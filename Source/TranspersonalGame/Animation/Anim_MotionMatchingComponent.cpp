#include "Anim_MotionMatchingComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Motion Matching Configuration
    TrajectoryTimeHorizon = 1.0f;
    HistoryTimeHorizon = 0.5f;
    MaxPoseSearchResults = 10;
    PoseMatchingThreshold = 0.8f;
    
    // IK Configuration
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    
    LastUpdateTime = 0.0f;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get component references
    AActor* Owner = GetOwner();
    if (Owner)
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        MovementComponent = Owner->FindComponentByClass<UCharacterMovementComponent>();
    }
    
    // Initialize trajectory arrays
    TrajectoryHistory.Reserve(50);
    TrajectoryPrediction.Reserve(20);
    PoseSearchHistory.Reserve(100);
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!SkeletalMeshComponent || !MovementComponent)
    {
        return;
    }
    
    // Update motion data
    FVector Velocity = MovementComponent->Velocity;
    float Speed = Velocity.Size();
    FVector Direction = Speed > 0.1f ? Velocity.GetSafeNormal() : FVector::ZeroVector;
    float Acceleration = MovementComponent->GetCurrentAcceleration().Size();
    
    UpdateMotionData(Speed, Direction, Acceleration);
    
    // Update trajectory prediction
    UpdateTrajectoryPrediction(DeltaTime);
    
    // Analyze current pose for motion matching
    AnalyzeCurrentPose();
    
    // Update foot IK
    UpdateFootIK(DeltaTime);
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_MotionMatchingComponent::UpdateMotionData(float Velocity, FVector Direction, float Acceleration)
{
    CurrentMotionData.Velocity = Velocity;
    CurrentMotionData.Direction = Direction;
    CurrentMotionData.Acceleration = Acceleration;
    
    if (MovementComponent)
    {
        CurrentMotionData.bIsGrounded = MovementComponent->IsMovingOnGround();
        
        // Calculate turn rate
        FVector CurrentForward = GetOwner()->GetActorForwardVector();
        float DotProduct = FVector::DotProduct(CurrentForward, Direction);
        CurrentMotionData.TurnRate = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
        
        // Calculate ground slope
        FHitResult HitResult;
        FVector Start = GetOwner()->GetActorLocation();
        FVector End = Start - FVector(0, 0, 200.0f);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
        {
            FVector GroundNormal = HitResult.Normal;
            CurrentMotionData.GroundSlope = FMath::Acos(FVector::DotProduct(GroundNormal, FVector::UpVector));
        }
    }
}

FAnim_PoseSearchData UAnim_MotionMatchingComponent::FindBestPoseMatch(const FAnim_MotionMatchingData& TargetMotion)
{
    FAnim_PoseSearchData BestMatch;
    float BestScore = -1.0f;
    
    // Search through pose history for best match
    for (const FAnim_PoseSearchData& PoseData : PoseSearchHistory)
    {
        float Score = CalculatePoseScore(PoseData, BestMatch);
        if (Score > BestScore && Score >= PoseMatchingThreshold)
        {
            BestScore = Score;
            BestMatch = PoseData;
        }
    }
    
    return BestMatch;
}

void UAnim_MotionMatchingComponent::PlayActionMontage(ESurvivalActionState ActionState)
{
    if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetAnimInstance())
    {
        return;
    }
    
    UAnimMontage** FoundMontage = ActionMontages.Find(ActionState);
    if (FoundMontage && *FoundMontage)
    {
        UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        AnimInstance->Montage_Play(*FoundMontage);
        
        CurrentMotionData.ActionState = ActionState;
    }
}

void UAnim_MotionMatchingComponent::StopActionMontage()
{
    if (SkeletalMeshComponent && SkeletalMeshComponent->GetAnimInstance())
    {
        UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        AnimInstance->Montage_Stop(0.2f);
        
        CurrentMotionData.ActionState = ESurvivalActionState::Idle;
    }
}

bool UAnim_MotionMatchingComponent::IsActionMontageActive() const
{
    if (SkeletalMeshComponent && SkeletalMeshComponent->GetAnimInstance())
    {
        UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        return AnimInstance->IsAnyMontagePlaying();
    }
    
    return false;
}

void UAnim_MotionMatchingComponent::UpdateFootIK(float DeltaTime)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Get foot bone locations
    FVector LeftFootLocation = SkeletalMeshComponent->GetBoneLocation(FName("foot_l"));
    FVector RightFootLocation = SkeletalMeshComponent->GetBoneLocation(FName("foot_r"));
    
    // Trace down from each foot to find ground
    FHitResult LeftHit, RightHit;
    FVector TraceStart, TraceEnd;
    
    // Left foot trace
    TraceStart = LeftFootLocation + FVector(0, 0, 50.0f);
    TraceEnd = LeftFootLocation - FVector(0, 0, 100.0f);
    
    if (GetWorld()->LineTraceSingleByChannel(LeftHit, TraceStart, TraceEnd, ECC_Visibility))
    {
        float TargetIK = LeftFootLocation.Z - LeftHit.Location.Z;
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, FMath::Clamp(TargetIK / 20.0f, 0.0f, 1.0f), DeltaTime, 10.0f);
        LeftFootIKLocation = LeftHit.Location;
    }
    else
    {
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaTime, 10.0f);
    }
    
    // Right foot trace
    TraceStart = RightFootLocation + FVector(0, 0, 50.0f);
    TraceEnd = RightFootLocation - FVector(0, 0, 100.0f);
    
    if (GetWorld()->LineTraceSingleByChannel(RightHit, TraceStart, TraceEnd, ECC_Visibility))
    {
        float TargetIK = RightFootLocation.Z - RightHit.Location.Z;
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, FMath::Clamp(TargetIK / 20.0f, 0.0f, 1.0f), DeltaTime, 10.0f);
        RightFootIKLocation = RightHit.Location;
    }
    else
    {
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaTime, 10.0f);
    }
}

FVector UAnim_MotionMatchingComponent::GetFootPlacementLocation(bool bIsLeftFoot) const
{
    return bIsLeftFoot ? LeftFootIKLocation : RightFootIKLocation;
}

float UAnim_MotionMatchingComponent::GetFootIKAlpha(bool bIsLeftFoot) const
{
    return bIsLeftFoot ? LeftFootIKAlpha : RightFootIKAlpha;
}

void UAnim_MotionMatchingComponent::AnalyzeCurrentPose()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    FAnim_PoseSearchData NewPoseData;
    NewPoseData.TimeStamp = GetWorld()->GetTimeSeconds();
    
    // Sample key bone positions and rotations
    TArray<FName> KeyBones = {
        FName("pelvis"),
        FName("spine_01"),
        FName("spine_02"),
        FName("spine_03"),
        FName("head"),
        FName("upperarm_l"),
        FName("upperarm_r"),
        FName("lowerarm_l"),
        FName("lowerarm_r"),
        FName("thigh_l"),
        FName("thigh_r"),
        FName("calf_l"),
        FName("calf_r"),
        FName("foot_l"),
        FName("foot_r")
    };
    
    for (const FName& BoneName : KeyBones)
    {
        FVector BoneLocation = SkeletalMeshComponent->GetBoneLocation(BoneName);
        FQuat BoneRotation = SkeletalMeshComponent->GetBoneQuaternion(BoneName);
        
        NewPoseData.BonePositions.Add(BoneLocation);
        NewPoseData.BoneRotations.Add(BoneRotation);
        
        // Calculate bone velocity (simple approximation)
        if (PoseSearchHistory.Num() > 0)
        {
            const FAnim_PoseSearchData& LastPose = PoseSearchHistory.Last();
            if (LastPose.BonePositions.IsValidIndex(NewPoseData.BonePositions.Num() - 1))
            {
                FVector LastBonePos = LastPose.BonePositions[NewPoseData.BonePositions.Num() - 1];
                float TimeDelta = NewPoseData.TimeStamp - LastPose.TimeStamp;
                FVector BoneVelocity = TimeDelta > 0.0f ? (BoneLocation - LastBonePos) / TimeDelta : FVector::ZeroVector;
                NewPoseData.BoneVelocities.Add(BoneVelocity);
            }
        }
    }
    
    // Add to history
    PoseSearchHistory.Add(NewPoseData);
    
    // Limit history size
    if (PoseSearchHistory.Num() > 100)
    {
        PoseSearchHistory.RemoveAt(0);
    }
}

float UAnim_MotionMatchingComponent::CalculatePoseScore(const FAnim_PoseSearchData& PoseA, const FAnim_PoseSearchData& PoseB) const
{
    if (PoseA.BonePositions.Num() != PoseB.BonePositions.Num())
    {
        return 0.0f;
    }
    
    float TotalScore = 0.0f;
    float MaxDistance = 100.0f; // cm
    
    for (int32 i = 0; i < PoseA.BonePositions.Num(); i++)
    {
        float Distance = FVector::Dist(PoseA.BonePositions[i], PoseB.BonePositions[i]);
        float NormalizedDistance = FMath::Clamp(Distance / MaxDistance, 0.0f, 1.0f);
        TotalScore += (1.0f - NormalizedDistance);
    }
    
    return TotalScore / PoseA.BonePositions.Num();
}

void UAnim_MotionMatchingComponent::UpdateTrajectoryPrediction(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Add current position to history
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    TrajectoryHistory.Add(CurrentLocation);
    
    // Limit history size
    if (TrajectoryHistory.Num() > 50)
    {
        TrajectoryHistory.RemoveAt(0);
    }
    
    // Simple trajectory prediction based on current velocity
    TrajectoryPrediction.Empty();
    FVector CurrentVelocity = MovementComponent->Velocity;
    FVector PredictedLocation = CurrentLocation;
    
    float PredictionStep = TrajectoryTimeHorizon / 20.0f;
    for (int32 i = 0; i < 20; i++)
    {
        PredictedLocation += CurrentVelocity * PredictionStep;
        TrajectoryPrediction.Add(PredictedLocation);
    }
}