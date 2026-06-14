#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Default feature weights
    FeatureWeight_Velocity = 1.0f;
    FeatureWeight_Speed = 0.8f;
    FeatureWeight_Direction = 0.6f;
    BlendTime = 0.2f;

    CurrentClipIndex = -1;
    CurrentClipTime = 0.0f;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize with basic motion clips if none provided
    if (MotionDatabase.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Database is empty. Add motion clips for proper functionality."));
    }
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character || !Character->GetCharacterMovement())
    {
        return;
    }

    // Update current motion features
    FVector Velocity = Character->GetCharacterMovement()->Velocity;
    FVector Acceleration = Character->GetCharacterMovement()->GetCurrentAcceleration();
    bool bIsGrounded = Character->GetCharacterMovement()->IsMovingOnGround();

    UpdateCurrentFeatures(Velocity, Acceleration, bIsGrounded);

    // Find best matching clip
    if (MotionDatabase.Num() > 0)
    {
        int32 BestClip = FindBestMatchingClip(CurrentFeatures);
        if (BestClip != CurrentClipIndex && BestClip >= 0)
        {
            CurrentClipIndex = BestClip;
            CurrentClipTime = 0.0f;
            
            UE_LOG(LogTemp, Log, TEXT("Motion Matching: Switched to clip %d (%s)"), 
                BestClip, *MotionDatabase[BestClip].ClipName);
        }
    }

    // Update clip time
    if (CurrentClipIndex >= 0 && CurrentClipIndex < MotionDatabase.Num())
    {
        CurrentClipTime += DeltaTime;
        const FAnim_MotionClip& CurrentClip = MotionDatabase[CurrentClipIndex];
        
        if (CurrentClipTime >= CurrentClip.EndTime - CurrentClip.StartTime)
        {
            CurrentClipTime = 0.0f; // Loop the animation
        }
    }
}

void UAnim_MotionMatchingSystem::UpdateCurrentFeatures(const FVector& InVelocity, const FVector& InAcceleration, bool bInIsGrounded)
{
    CurrentFeatures.Velocity = InVelocity;
    CurrentFeatures.Acceleration = InAcceleration;
    CurrentFeatures.Speed = InVelocity.Size();
    CurrentFeatures.Direction = FMath::Atan2(InVelocity.Y, InVelocity.X);
    CurrentFeatures.bIsGrounded = bInIsGrounded;

    // Calculate ground distance using line trace
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        FVector Start = Character->GetActorLocation();
        FVector End = Start - FVector(0, 0, 200.0f);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Character);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
        {
            CurrentFeatures.GroundDistance = HitResult.Distance;
        }
        else
        {
            CurrentFeatures.GroundDistance = 200.0f;
        }
    }
}

int32 UAnim_MotionMatchingSystem::FindBestMatchingClip(const FAnim_MotionFeature& TargetFeatures)
{
    if (MotionDatabase.Num() == 0)
    {
        return -1;
    }

    int32 BestClipIndex = 0;
    float BestDistance = FLT_MAX;

    for (int32 i = 0; i < MotionDatabase.Num(); i++)
    {
        const FAnim_MotionClip& Clip = MotionDatabase[i];
        
        // For simplicity, compare with the first feature in the clip
        if (Clip.Features.Num() > 0)
        {
            float Distance = CalculateFeatureDistance(TargetFeatures, Clip.Features[0]);
            
            if (Distance < BestDistance)
            {
                BestDistance = Distance;
                BestClipIndex = i;
            }
        }
    }

    return BestClipIndex;
}

float UAnim_MotionMatchingSystem::CalculateFeatureDistance(const FAnim_MotionFeature& A, const FAnim_MotionFeature& B)
{
    float Distance = 0.0f;

    // Velocity distance
    Distance += FeatureWeight_Velocity * FVector::Dist(A.Velocity, B.Velocity);

    // Speed distance
    Distance += FeatureWeight_Speed * FMath::Abs(A.Speed - B.Speed);

    // Direction distance (handle wrap-around)
    float DirDiff = FMath::Abs(A.Direction - B.Direction);
    if (DirDiff > PI)
    {
        DirDiff = 2.0f * PI - DirDiff;
    }
    Distance += FeatureWeight_Direction * DirDiff;

    // Ground state penalty
    if (A.bIsGrounded != B.bIsGrounded)
    {
        Distance += 100.0f; // Heavy penalty for ground state mismatch
    }

    return Distance;
}

void UAnim_MotionMatchingSystem::AddMotionClip(UAnimSequence* AnimSeq, const FString& ClipName)
{
    if (!AnimSeq)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot add null animation sequence to motion database"));
        return;
    }

    FAnim_MotionClip NewClip;
    NewClip.AnimSequence = AnimSeq;
    NewClip.ClipName = ClipName;
    NewClip.StartTime = 0.0f;
    NewClip.EndTime = AnimSeq->GetPlayLength();

    // Extract features from the animation
    ExtractFeaturesFromAnimation(AnimSeq, NewClip);

    MotionDatabase.Add(NewClip);
    
    UE_LOG(LogTemp, Log, TEXT("Added motion clip: %s (Duration: %.2f)"), 
        *ClipName, NewClip.EndTime);
}

UAnimSequence* UAnim_MotionMatchingSystem::GetCurrentAnimation()
{
    if (CurrentClipIndex >= 0 && CurrentClipIndex < MotionDatabase.Num())
    {
        return MotionDatabase[CurrentClipIndex].AnimSequence;
    }
    return nullptr;
}

void UAnim_MotionMatchingSystem::SetFeatureWeights(float VelWeight, float SpeedWeight, float DirWeight)
{
    FeatureWeight_Velocity = FMath::Clamp(VelWeight, 0.0f, 10.0f);
    FeatureWeight_Speed = FMath::Clamp(SpeedWeight, 0.0f, 10.0f);
    FeatureWeight_Direction = FMath::Clamp(DirWeight, 0.0f, 10.0f);
}

void UAnim_MotionMatchingSystem::ExtractFeaturesFromAnimation(UAnimSequence* AnimSeq, FAnim_MotionClip& OutClip)
{
    if (!AnimSeq)
    {
        return;
    }

    // Sample features at regular intervals
    float SampleRate = 30.0f; // 30 samples per second
    float Duration = AnimSeq->GetPlayLength();
    int32 NumSamples = FMath::CeilToInt(Duration * SampleRate);

    OutClip.Features.Empty();
    OutClip.Features.Reserve(NumSamples);

    for (int32 i = 0; i < NumSamples; i++)
    {
        float SampleTime = (float)i / SampleRate;
        if (SampleTime > Duration)
        {
            SampleTime = Duration;
        }

        FAnim_MotionFeature Feature;
        SampleAnimationFeatures(AnimSeq, SampleTime, Feature);
        OutClip.Features.Add(Feature);
    }

    UE_LOG(LogTemp, Log, TEXT("Extracted %d features from animation %s"), 
        OutClip.Features.Num(), *AnimSeq->GetName());
}

void UAnim_MotionMatchingSystem::SampleAnimationFeatures(UAnimSequence* AnimSeq, float SampleTime, FAnim_MotionFeature& OutFeature)
{
    // This is a simplified feature extraction
    // In a real implementation, you would sample bone positions and calculate velocities
    
    // For now, create placeholder features based on animation name and time
    FString AnimName = AnimSeq->GetName().ToLower();
    
    if (AnimName.Contains(TEXT("idle")))
    {
        OutFeature.Velocity = FVector::ZeroVector;
        OutFeature.Speed = 0.0f;
        OutFeature.Direction = 0.0f;
        OutFeature.bIsGrounded = true;
    }
    else if (AnimName.Contains(TEXT("walk")))
    {
        OutFeature.Speed = 150.0f;
        OutFeature.Velocity = FVector(OutFeature.Speed, 0, 0);
        OutFeature.Direction = 0.0f;
        OutFeature.bIsGrounded = true;
    }
    else if (AnimName.Contains(TEXT("run")))
    {
        OutFeature.Speed = 400.0f;
        OutFeature.Velocity = FVector(OutFeature.Speed, 0, 0);
        OutFeature.Direction = 0.0f;
        OutFeature.bIsGrounded = true;
    }
    else if (AnimName.Contains(TEXT("jump")))
    {
        OutFeature.Speed = 200.0f;
        OutFeature.Velocity = FVector(200.0f, 0, 300.0f);
        OutFeature.Direction = 0.0f;
        OutFeature.bIsGrounded = false;
    }
    else
    {
        // Default features
        OutFeature.Speed = 100.0f;
        OutFeature.Velocity = FVector(100.0f, 0, 0);
        OutFeature.Direction = 0.0f;
        OutFeature.bIsGrounded = true;
    }

    OutFeature.Acceleration = FVector::ZeroVector;
    OutFeature.GroundDistance = OutFeature.bIsGrounded ? 0.0f : 50.0f;
}