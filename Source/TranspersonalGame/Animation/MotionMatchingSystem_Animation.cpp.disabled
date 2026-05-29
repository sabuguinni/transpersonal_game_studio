#include "MotionMatchingSystem_Animation.h"
// DISABLED: // DISABLED: #include "MotionMatchingSystem.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

UMotionMatchingComponent::UMotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    SearchRadius = 100.0f;
    BlendTime = 0.2f;
    TrajectoryPoints = 6;
    TrajectoryTimeHorizon = 1.0f;
    CurrentFrameIndex = 0;
    CurrentPlayTime = 0.0f;
    
    CurrentTrajectory.SetNum(TrajectoryPoints);
    PredictedTrajectory.SetNum(TrajectoryPoints);
}

void UMotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (MotionDatabase)
    {
        UE_LOG(LogTemp, Log, TEXT("Motion Matching Component initialized with %d clips"), 
               MotionDatabase->MotionClips.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching Component has no Motion Database assigned!"));
    }
}

void UMotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!MotionDatabase || MotionDatabase->MotionClips.Num() == 0)
    {
        return;
    }
    
    // Update current play time
    CurrentPlayTime += DeltaTime;
    
    // Extract current motion features
    ExtractCurrentFeatures();
    
    // Check if we need to search for a new motion
    static float SearchTimer = 0.0f;
    SearchTimer += DeltaTime;
    
    if (SearchTimer >= 0.1f) // Search every 100ms
    {
        SearchTimer = 0.0f;
        
        // Find best matching motion
        FMotionClip BestMatch = FindBestMotionMatch();
        
        // Blend to new motion if it's significantly different
        if (BestMatch.AnimSequence != CurrentClip.AnimSequence)
        {
            BlendToNewMotion(BestMatch);
        }
    }
}

void UMotionMatchingComponent::UpdateTrajectory(const FVector& DesiredVelocity, const FRotator& DesiredRotation)
{
    PredictTrajectory(DesiredVelocity, DesiredRotation);
}

FMotionClip UMotionMatchingComponent::FindBestMotionMatch()
{
    if (!MotionDatabase || MotionDatabase->MotionClips.Num() == 0)
    {
        return FMotionClip();
    }
    
    float BestScore = FLT_MAX;
    int32 BestClipIndex = 0;
    int32 BestFrameIndex = 0;
    
    // Search through all clips and frames
    for (int32 ClipIndex = 0; ClipIndex < MotionDatabase->MotionClips.Num(); ClipIndex++)
    {
        const FMotionClip& Clip = MotionDatabase->MotionClips[ClipIndex];
        
        for (int32 FrameIndex = 0; FrameIndex < Clip.Frames.Num(); FrameIndex++)
        {
            const FMotionFrame& Frame = Clip.Frames[FrameIndex];
            
            // Calculate distance between current trajectory and this frame
            float TotalDistance = 0.0f;
            int32 FeatureCount = FMath::Min(CurrentTrajectory.Num(), Frame.Features.Num());
            
            for (int32 i = 0; i < FeatureCount; i++)
            {
                TotalDistance += CalculateFeatureDistance(CurrentTrajectory[i], Frame.Features[i]);
            }
            
            if (TotalDistance < BestScore)
            {
                BestScore = TotalDistance;
                BestClipIndex = ClipIndex;
                BestFrameIndex = FrameIndex;
            }
        }
    }
    
    // Return the best matching clip
    FMotionClip BestClip = MotionDatabase->MotionClips[BestClipIndex];
    CurrentFrameIndex = BestFrameIndex;
    
    return BestClip;
}

void UMotionMatchingComponent::BlendToNewMotion(const FMotionClip& NewClip)
{
    if (NewClip.AnimSequence == CurrentClip.AnimSequence)
    {
        return;
    }
    
    // Store previous clip for blending
    FMotionClip PreviousClip = CurrentClip;
    CurrentClip = NewClip;
    CurrentPlayTime = 0.0f;
    
    // Trigger Blueprint event for animation system
    OnMotionChanged(NewClip);
    
    UE_LOG(LogTemp, Log, TEXT("Motion Matching: Blending to new clip '%s' at frame %d"), 
           *NewClip.ClipName, CurrentFrameIndex);
}

float UMotionMatchingComponent::CalculateFeatureDistance(const FMotionFeature& A, const FMotionFeature& B) const
{
    float PositionDistance = FVector::Dist(A.Position, B.Position);
    float VelocityDistance = FVector::Dist(A.Velocity, B.Velocity);
    float RotationDistance = FMath::Abs(FRotator::NormalizeAxis(A.Rotation.Yaw - B.Rotation.Yaw));
    
    // Weighted combination of distances
    return (PositionDistance * 0.4f) + (VelocityDistance * 0.4f) + (RotationDistance * 0.2f);
}

void UMotionMatchingComponent::PredictTrajectory(const FVector& DesiredVelocity, const FRotator& DesiredRotation)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    FVector CurrentPosition = Owner->GetActorLocation();
    FRotator CurrentRotation = Owner->GetActorRotation();
    FVector CurrentVelocity = DesiredVelocity;
    
    float TimeStep = TrajectoryTimeHorizon / TrajectoryPoints;
    
    for (int32 i = 0; i < TrajectoryPoints; i++)
    {
        float PredictionTime = (i + 1) * TimeStep;
        
        // Simple trajectory prediction - can be enhanced with more sophisticated physics
        FVector PredictedPosition = CurrentPosition + (CurrentVelocity * PredictionTime);
        FRotator PredictedRotation = FMath::RInterpTo(CurrentRotation, DesiredRotation, 1.0f, PredictionTime);
        
        PredictedTrajectory[i].Position = PredictedPosition;
        PredictedTrajectory[i].Velocity = CurrentVelocity;
        PredictedTrajectory[i].Rotation = PredictedRotation;
        PredictedTrajectory[i].Weight = 1.0f - (PredictionTime / TrajectoryTimeHorizon); // Decay weight over time
    }
}

void UMotionMatchingComponent::ExtractCurrentFeatures()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Get current state
    FVector CurrentPosition = Owner->GetActorLocation();
    FRotator CurrentRotation = Owner->GetActorRotation();
    FVector CurrentVelocity = FVector::ZeroVector;
    
    // Try to get velocity from character movement
    if (ACharacter* Character = Cast<ACharacter>(Owner))
    {
        if (Character->GetCharacterMovement())
        {
            CurrentVelocity = Character->GetCharacterMovement()->Velocity;
        }
    }
    
    // Update current trajectory with actual values
    for (int32 i = 0; i < CurrentTrajectory.Num(); i++)
    {
        CurrentTrajectory[i].Position = CurrentPosition;
        CurrentTrajectory[i].Velocity = CurrentVelocity;
        CurrentTrajectory[i].Rotation = CurrentRotation;
        CurrentTrajectory[i].Weight = 1.0f;
    }
}

// Motion Database Implementation
int32 UMotionDatabase::FindBestMatch(const TArray<FMotionFeature>& QueryFeatures, float& OutScore) const
{
    OutScore = FLT_MAX;
    int32 BestMatch = -1;
    
    for (int32 ClipIndex = 0; ClipIndex < MotionClips.Num(); ClipIndex++)
    {
        const FMotionClip& Clip = MotionClips[ClipIndex];
        
        for (int32 FrameIndex = 0; FrameIndex < Clip.Frames.Num(); FrameIndex++)
        {
            const FMotionFrame& Frame = Clip.Frames[FrameIndex];
            
            float TotalDistance = 0.0f;
            int32 FeatureCount = FMath::Min(QueryFeatures.Num(), Frame.Features.Num());
            
            for (int32 i = 0; i < FeatureCount; i++)
            {
                float Distance = FVector::Dist(QueryFeatures[i].Position, Frame.Features[i].Position);
                Distance += FVector::Dist(QueryFeatures[i].Velocity, Frame.Features[i].Velocity);
                TotalDistance += Distance * QueryFeatures[i].Weight;
            }
            
            if (TotalDistance < OutScore)
            {
                OutScore = TotalDistance;
                BestMatch = ClipIndex * 1000 + FrameIndex; // Encode clip and frame index
            }
        }
    }
    
    return BestMatch;
}

void UMotionDatabase::BuildDatabase()
{
    UE_LOG(LogTemp, Log, TEXT("Building Motion Database with %d clips"), MotionClips.Num());
    
    for (FMotionClip& Clip : MotionClips)
    {
        if (Clip.AnimSequence)
        {
            // Extract motion features from animation sequence
            float ClipDuration = Clip.AnimSequence->GetPlayLength();
            int32 FrameCount = FMath::CeilToInt(ClipDuration * FrameRate);
            
            Clip.Frames.SetNum(FrameCount);
            Clip.ClipLength = ClipDuration;
            
            for (int32 FrameIndex = 0; FrameIndex < FrameCount; FrameIndex++)
            {
                float TimeStamp = (float)FrameIndex / FrameRate;
                Clip.Frames[FrameIndex].TimeStamp = TimeStamp;
                
                // TODO: Extract actual bone positions and velocities from animation
                // This would require access to the skeletal mesh and bone data
                // For now, we'll use placeholder data
                Clip.Frames[FrameIndex].Features.SetNum(FeatureCount);
            }
            
            UE_LOG(LogTemp, Log, TEXT("Processed clip '%s' with %d frames"), 
                   *Clip.ClipName, FrameCount);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Motion Database build complete"));
}