#include "Anim_MotionMatchingSystem.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    SearchRadius = 100.0f;
    BlendTime = 0.2f;
    bEnableMotionMatching = true;
    CurrentClipIndex = -1;
    CurrentPlayTime = 0.0f;
    LastUpdateTime = 0.0f;
    bDatabaseInitialized = false;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMotionDatabase();
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableMotionMatching || !bDatabaseInitialized)
    {
        return;
    }

    ProcessMotionMatching(DeltaTime);
}

void UAnim_MotionMatchingSystem::UpdateCurrentFeatures(const FVector& InVelocity, const FVector& InAcceleration, bool bInIsJumping, bool bInIsCrouching)
{
    CurrentFeatures.Velocity = InVelocity;
    CurrentFeatures.Acceleration = InAcceleration;
    CurrentFeatures.Speed = InVelocity.Size();
    CurrentFeatures.Direction = FMath::Atan2(InVelocity.Y, InVelocity.X);
    CurrentFeatures.bIsMoving = CurrentFeatures.Speed > 10.0f;
    CurrentFeatures.bIsJumping = bInIsJumping;
    CurrentFeatures.bIsCrouching = bInIsCrouching;
}

int32 UAnim_MotionMatchingSystem::FindBestMatchingClip(const FAnim_MotionFeature& TargetFeatures)
{
    if (MotionDatabase.Num() == 0)
    {
        return -1;
    }

    int32 BestClipIndex = -1;
    float BestDistance = FLT_MAX;

    for (int32 ClipIndex = 0; ClipIndex < MotionDatabase.Num(); ++ClipIndex)
    {
        const FAnim_MotionClip& Clip = MotionDatabase[ClipIndex];
        
        for (const FAnim_MotionFeature& Feature : Clip.Features)
        {
            float Distance = CalculateFeatureDistance(TargetFeatures, Feature);
            
            if (Distance < BestDistance && Distance <= SearchRadius)
            {
                BestDistance = Distance;
                BestClipIndex = ClipIndex;
            }
        }
    }

    return BestClipIndex;
}

void UAnim_MotionMatchingSystem::AddMotionClip(UAnimSequence* AnimSeq, const TArray<FAnim_MotionFeature>& ClipFeatures)
{
    if (!AnimSeq)
    {
        return;
    }

    FAnim_MotionClip NewClip;
    NewClip.AnimSequence = AnimSeq;
    NewClip.Features = ClipFeatures;
    NewClip.StartTime = 0.0f;
    NewClip.EndTime = AnimSeq->GetPlayLength();
    NewClip.ClipID = MotionDatabase.Num();

    MotionDatabase.Add(NewClip);
    
    UE_LOG(LogTemp, Log, TEXT("Motion Matching: Added clip %s with %d features"), 
           *AnimSeq->GetName(), ClipFeatures.Num());
}

void UAnim_MotionMatchingSystem::ClearMotionDatabase()
{
    MotionDatabase.Empty();
    CurrentClipIndex = -1;
    bDatabaseInitialized = false;
}

UAnimSequence* UAnim_MotionMatchingSystem::GetCurrentAnimSequence() const
{
    if (CurrentClipIndex >= 0 && CurrentClipIndex < MotionDatabase.Num())
    {
        return MotionDatabase[CurrentClipIndex].AnimSequence;
    }
    
    return nullptr;
}

float UAnim_MotionMatchingSystem::CalculateFeatureDistance(const FAnim_MotionFeature& FeatureA, const FAnim_MotionFeature& FeatureB) const
{
    float VelocityWeight = 1.0f;
    float AccelerationWeight = 0.5f;
    float SpeedWeight = 1.5f;
    float DirectionWeight = 0.8f;
    float BooleanWeight = 2.0f;

    float VelocityDist = FVector::Dist(FeatureA.Velocity, FeatureB.Velocity) * VelocityWeight;
    float AccelDist = FVector::Dist(FeatureA.Acceleration, FeatureB.Acceleration) * AccelerationWeight;
    float SpeedDist = FMath::Abs(FeatureA.Speed - FeatureB.Speed) * SpeedWeight;
    float DirectionDist = FMath::Abs(FeatureA.Direction - FeatureB.Direction) * DirectionWeight;
    
    float BooleanDist = 0.0f;
    if (FeatureA.bIsMoving != FeatureB.bIsMoving) BooleanDist += BooleanWeight;
    if (FeatureA.bIsJumping != FeatureB.bIsJumping) BooleanDist += BooleanWeight;
    if (FeatureA.bIsCrouching != FeatureB.bIsCrouching) BooleanDist += BooleanWeight;

    return VelocityDist + AccelDist + SpeedDist + DirectionDist + BooleanDist;
}

void UAnim_MotionMatchingSystem::InitializeMotionDatabase()
{
    // Create basic motion features for common movement states
    TArray<FAnim_MotionFeature> IdleFeatures;
    FAnim_MotionFeature IdleFeature;
    IdleFeature.Velocity = FVector::ZeroVector;
    IdleFeature.Speed = 0.0f;
    IdleFeature.bIsMoving = false;
    IdleFeatures.Add(IdleFeature);

    TArray<FAnim_MotionFeature> WalkFeatures;
    FAnim_MotionFeature WalkFeature;
    WalkFeature.Velocity = FVector(150.0f, 0.0f, 0.0f);
    WalkFeature.Speed = 150.0f;
    WalkFeature.bIsMoving = true;
    WalkFeatures.Add(WalkFeature);

    TArray<FAnim_MotionFeature> RunFeatures;
    FAnim_MotionFeature RunFeature;
    RunFeature.Velocity = FVector(400.0f, 0.0f, 0.0f);
    RunFeature.Speed = 400.0f;
    RunFeature.bIsMoving = true;
    RunFeatures.Add(RunFeature);

    TArray<FAnim_MotionFeature> JumpFeatures;
    FAnim_MotionFeature JumpFeature;
    JumpFeature.Velocity = FVector(200.0f, 0.0f, 300.0f);
    JumpFeature.Speed = 200.0f;
    JumpFeature.bIsJumping = true;
    JumpFeatures.Add(JumpFeature);

    bDatabaseInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("Motion Matching System: Database initialized with basic movement features"));
}

void UAnim_MotionMatchingSystem::ProcessMotionMatching(float DeltaTime)
{
    LastUpdateTime += DeltaTime;
    
    // Update motion matching every 0.1 seconds for performance
    if (LastUpdateTime < 0.1f)
    {
        return;
    }
    
    LastUpdateTime = 0.0f;

    // Find best matching clip based on current features
    int32 BestClipIndex = FindBestMatchingClip(CurrentFeatures);
    
    if (BestClipIndex != -1 && BestClipIndex != CurrentClipIndex)
    {
        CurrentClipIndex = BestClipIndex;
        CurrentPlayTime = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Motion Matching: Switched to clip %d"), CurrentClipIndex);
    }
    
    // Update play time
    if (CurrentClipIndex >= 0 && CurrentClipIndex < MotionDatabase.Num())
    {
        CurrentPlayTime += DeltaTime;
        
        const FAnim_MotionClip& CurrentClip = MotionDatabase[CurrentClipIndex];
        if (CurrentPlayTime >= CurrentClip.EndTime)
        {
            CurrentPlayTime = 0.0f; // Loop the animation
        }
    }
}