#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    MotionMatchingThreshold = 0.8f;
    BlendTime = 0.2f;
    bEnableMotionMatching = true;
    
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponent();
}

void UAnim_MotionMatchingSystem::InitializeComponent()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Motion Matching System initialized for character: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching System: Owner is not a Character"));
    }
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableMotionMatching || !OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    UpdateCurrentMotionData();
    
    if (PoseDatabase.Num() > 0)
    {
        BestMatchPose = FindBestMatchingPose(CurrentMotionData);
    }
}

void UAnim_MotionMatchingSystem::UpdateCurrentMotionData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    CurrentMotionData.Velocity = GetCharacterVelocity();
    CurrentMotionData.Acceleration = GetCharacterAcceleration();
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    CurrentMotionData.Direction = FMath::Atan2(CurrentMotionData.Velocity.Y, CurrentMotionData.Velocity.X);
    CurrentMotionData.bIsMoving = IsCharacterMoving();
    CurrentMotionData.bIsJumping = IsCharacterJumping();
    CurrentMotionData.bIsCrouching = IsCharacterCrouching();
}

void UAnim_MotionMatchingSystem::UpdateMotionData(const FVector& InVelocity, const FVector& InAcceleration, bool bInIsMoving, bool bInIsJumping, bool bInIsCrouching)
{
    CurrentMotionData.Velocity = InVelocity;
    CurrentMotionData.Acceleration = InAcceleration;
    CurrentMotionData.Speed = InVelocity.Size();
    CurrentMotionData.Direction = FMath::Atan2(InVelocity.Y, InVelocity.X);
    CurrentMotionData.bIsMoving = bInIsMoving;
    CurrentMotionData.bIsJumping = bInIsJumping;
    CurrentMotionData.bIsCrouching = bInIsCrouching;
}

FAnim_MotionMatchingPose UAnim_MotionMatchingSystem::FindBestMatchingPose(const FAnim_MotionData& TargetMotion)
{
    FAnim_MotionMatchingPose BestPose;
    float BestScore = -1.0f;
    
    for (FAnim_MotionMatchingPose& Pose : PoseDatabase)
    {
        float Score = CalculateMotionScore(Pose.MotionData, TargetMotion);
        Pose.MatchScore = Score;
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestPose = Pose;
        }
    }
    
    BestPose.MatchScore = BestScore;
    return BestPose;
}

float UAnim_MotionMatchingSystem::CalculateMotionScore(const FAnim_MotionData& PoseMotion, const FAnim_MotionData& TargetMotion)
{
    float Score = 0.0f;
    
    // Velocity matching (40% weight)
    float VelocityDiff = FVector::Dist(PoseMotion.Velocity, TargetMotion.Velocity);
    float VelocityScore = FMath::Clamp(1.0f - (VelocityDiff / 1000.0f), 0.0f, 1.0f);
    Score += VelocityScore * 0.4f;
    
    // Speed matching (20% weight)
    float SpeedDiff = FMath::Abs(PoseMotion.Speed - TargetMotion.Speed);
    float SpeedScore = FMath::Clamp(1.0f - (SpeedDiff / 1000.0f), 0.0f, 1.0f);
    Score += SpeedScore * 0.2f;
    
    // Direction matching (20% weight)
    float DirectionDiff = FMath::Abs(PoseMotion.Direction - TargetMotion.Direction);
    if (DirectionDiff > PI) DirectionDiff = 2 * PI - DirectionDiff; // Handle wrap-around
    float DirectionScore = FMath::Clamp(1.0f - (DirectionDiff / PI), 0.0f, 1.0f);
    Score += DirectionScore * 0.2f;
    
    // State matching (20% weight)
    float StateScore = 0.0f;
    if (PoseMotion.bIsMoving == TargetMotion.bIsMoving) StateScore += 0.33f;
    if (PoseMotion.bIsJumping == TargetMotion.bIsJumping) StateScore += 0.33f;
    if (PoseMotion.bIsCrouching == TargetMotion.bIsCrouching) StateScore += 0.34f;
    Score += StateScore * 0.2f;
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}

void UAnim_MotionMatchingSystem::AddPoseToDatabase(UAnimSequence* AnimSeq, float Time, const FAnim_MotionData& Motion)
{
    if (!AnimSeq)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot add null animation sequence to pose database"));
        return;
    }
    
    FAnim_MotionMatchingPose NewPose;
    NewPose.AnimSequence = AnimSeq;
    NewPose.TimeInSequence = Time;
    NewPose.MotionData = Motion;
    NewPose.MatchScore = 0.0f;
    
    PoseDatabase.Add(NewPose);
    
    UE_LOG(LogTemp, Log, TEXT("Added pose to database: %s at time %f"), *AnimSeq->GetName(), Time);
}

void UAnim_MotionMatchingSystem::ClearPoseDatabase()
{
    PoseDatabase.Empty();
    UE_LOG(LogTemp, Log, TEXT("Cleared motion matching pose database"));
}

void UAnim_MotionMatchingSystem::BuildPoseDatabaseFromAnimations(const TArray<UAnimSequence*>& AnimSequences)
{
    ClearPoseDatabase();
    
    for (UAnimSequence* AnimSeq : AnimSequences)
    {
        if (!AnimSeq)
        {
            continue;
        }
        
        float AnimLength = AnimSeq->GetPlayLength();
        float SampleRate = 30.0f; // 30 samples per second
        float SampleInterval = 1.0f / SampleRate;
        
        for (float Time = 0.0f; Time < AnimLength; Time += SampleInterval)
        {
            // For now, create sample motion data
            // In a real implementation, this would extract actual motion data from the animation
            FAnim_MotionData SampleMotion;
            SampleMotion.Speed = FMath::RandRange(0.0f, 600.0f);
            SampleMotion.Velocity = FVector(FMath::RandRange(-600.0f, 600.0f), FMath::RandRange(-600.0f, 600.0f), 0.0f);
            SampleMotion.bIsMoving = SampleMotion.Speed > 10.0f;
            
            AddPoseToDatabase(AnimSeq, Time, SampleMotion);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Built pose database with %d poses from %d animations"), PoseDatabase.Num(), AnimSequences.Num());
}

FVector UAnim_MotionMatchingSystem::GetCharacterVelocity() const
{
    if (MovementComponent)
    {
        return MovementComponent->Velocity;
    }
    return FVector::ZeroVector;
}

FVector UAnim_MotionMatchingSystem::GetCharacterAcceleration() const
{
    if (MovementComponent)
    {
        return MovementComponent->GetCurrentAcceleration();
    }
    return FVector::ZeroVector;
}

bool UAnim_MotionMatchingSystem::IsCharacterMoving() const
{
    if (MovementComponent)
    {
        return MovementComponent->Velocity.SizeSquared() > FMath::Square(10.0f);
    }
    return false;
}

bool UAnim_MotionMatchingSystem::IsCharacterJumping() const
{
    if (MovementComponent)
    {
        return MovementComponent->IsFalling();
    }
    return false;
}

bool UAnim_MotionMatchingSystem::IsCharacterCrouching() const
{
    if (MovementComponent)
    {
        return MovementComponent->IsCrouching();
    }
    return false;
}