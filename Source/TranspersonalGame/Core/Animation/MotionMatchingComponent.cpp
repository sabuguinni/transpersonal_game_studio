#include "MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UMotionMatchingComponent::UMotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Motion Matching defaults
    bMotionMatchingEnabled = true;
    BlendTime = 0.2f;
    SearchRadius = 100.0f;
    MaxSearchCandidates = 20;
    MinimumScore = 0.1f;

    // Feature weights
    PositionWeight = 1.0f;
    VelocityWeight = 2.0f;
    RotationWeight = 0.5f;

    // Trajectory settings
    TrajectoryHistorySize = 60; // 1 second at 60fps
    TrajectoryPredictionTime = 1.0f;
    TrajectoryPredictionSamples = 10;
    bDrawTrajectoryDebug = false;

    // Performance settings
    SearchQuality = 3; // High quality by default
    bAdaptiveQualityEnabled = true;
    QualityAdaptationThreshold = 16.67f; // 60fps

    // Internal state
    LastUpdateTime = 0.0f;
    AverageSearchTime = 0.0f;
    FramesSinceLastUpdate = 0;
}

void UMotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("MotionMatchingComponent: Owner is not a Character"));
        return;
    }

    // Initialize trajectory history
    TrajectoryHistory.Reserve(TrajectoryHistorySize);
    
    UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Initialized for character %s"), 
           *OwnerCharacter->GetName());
}

void UMotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bMotionMatchingEnabled || !OwnerCharacter)
        return;

    UpdateCurrentQuery(DeltaTime);
    
    // Update trajectory
    FVector CurrentVelocity = OwnerCharacter->GetVelocity();
    UpdateTrajectoryHistory(OwnerCharacter->GetActorLocation(), CurrentVelocity);

    // Adaptive quality management
    if (bAdaptiveQualityEnabled)
    {
        OptimizeSearchQuality();
    }

    // Debug drawing
    if (bDrawTrajectoryDebug)
    {
        DrawTrajectoryDebug(true);
    }

    FramesSinceLastUpdate++;
}

void UMotionMatchingComponent::InitializeMotionDatabase(const TArray<UAnimSequence*>& Animations)
{
    SourceAnimations = Animations;
    BuildMotionDatabase();
    
    UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Database initialized with %d animations, %d candidates"), 
           Animations.Num(), MotionDatabase.Num());
}

FMotionCandidate UMotionMatchingComponent::FindBestMatch(const FMotionFeature& QueryFeature)
{
    if (MotionDatabase.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Motion database is empty"));
        return FMotionCandidate();
    }

    float StartTime = FPlatformTime::Seconds();
    
    FMotionCandidate BestCandidate;
    float BestScore = FLT_MAX;
    int32 CandidatesChecked = 0;
    int32 MaxCandidatesToCheck = FMath::Min(MaxSearchCandidates, MotionDatabase.Num());

    for (int32 i = 0; i < MaxCandidatesToCheck; i++)
    {
        const FMotionCandidate& Candidate = MotionDatabase[i];
        float Score = CalculateFeatureDistance(QueryFeature, Candidate.Feature);
        
        if (Score < BestScore && Score >= MinimumScore)
        {
            BestScore = Score;
            BestCandidate = Candidate;
            BestCandidate.Score = Score;
        }
        
        CandidatesChecked++;
    }

    // Update performance metrics
    float SearchTime = FPlatformTime::Seconds() - StartTime;
    AverageSearchTime = (AverageSearchTime * 0.9f) + (SearchTime * 0.1f);

    LastSelectedCandidate = BestCandidate;
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingComponent: Found best match with score %f (checked %d candidates)"), 
           BestScore, CandidatesChecked);

    return BestCandidate;
}

void UMotionMatchingComponent::UpdateCurrentQuery(float DeltaTime)
{
    if (!OwnerCharacter)
        return;

    CurrentQuery = ExtractCurrentFeature();
    
    // Find best matching animation
    FMotionCandidate BestMatch = FindBestMatch(CurrentQuery);
    
    if (BestMatch.Animation && BestMatch.Score < FLT_MAX)
    {
        // Apply the animation (this would typically be done through the animation blueprint)
        UE_LOG(LogTemp, VeryVerbose, TEXT("MotionMatchingComponent: Best match: %s at time %f"), 
               *BestMatch.Animation->GetName(), BestMatch.StartTime);
    }
}

void UMotionMatchingComponent::SetMotionMatchingEnabled(bool bEnabled)
{
    bMotionMatchingEnabled = bEnabled;
    UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Motion matching %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UMotionMatchingComponent::UpdateTrajectory(const FVector& InputVector, float DeltaTime)
{
    if (!OwnerCharacter)
        return;

    FVector CurrentPosition = OwnerCharacter->GetActorLocation();
    FVector CurrentVelocity = OwnerCharacter->GetVelocity();
    
    // Apply input to velocity prediction
    if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
    {
        FVector DesiredVelocity = InputVector * MovementComp->GetMaxSpeed();
        CurrentVelocity = FMath::VInterpTo(CurrentVelocity, DesiredVelocity, DeltaTime, 5.0f);
    }

    UpdateTrajectoryHistory(CurrentPosition, CurrentVelocity);
}

TArray<FTrajectoryPoint> UMotionMatchingComponent::GetPredictedTrajectory(float PredictionTime, int32 NumSamples)
{
    TArray<FTrajectoryPoint> PredictedTrajectory;
    
    if (!OwnerCharacter || NumSamples <= 0)
        return PredictedTrajectory;

    float TimeStep = PredictionTime / NumSamples;
    FVector CurrentPosition = OwnerCharacter->GetActorLocation();
    FVector CurrentVelocity = OwnerCharacter->GetVelocity();

    for (int32 i = 0; i < NumSamples; i++)
    {
        float Time = (i + 1) * TimeStep;
        FVector PredictedPosition = PredictFuturePosition(Time);
        
        FTrajectoryPoint Point;
        Point.Position = PredictedPosition;
        Point.Velocity = CurrentVelocity; // Simplified - would need more complex prediction
        Point.Time = Time;
        
        PredictedTrajectory.Add(Point);
    }

    return PredictedTrajectory;
}

void UMotionMatchingComponent::DrawTrajectoryDebug(bool bDraw)
{
    if (!bDraw || !OwnerCharacter)
        return;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World)
        return;

    // Draw trajectory history
    for (int32 i = 1; i < TrajectoryHistory.Num(); i++)
    {
        FVector Start = TrajectoryHistory[i-1].Position;
        FVector End = TrajectoryHistory[i].Position;
        DrawDebugLine(World, Start, End, FColor::Blue, false, 0.0f, 0, 2.0f);
    }

    // Draw predicted trajectory
    TArray<FTrajectoryPoint> PredictedTrajectory = GetPredictedTrajectory(TrajectoryPredictionTime, TrajectoryPredictionSamples);
    for (int32 i = 1; i < PredictedTrajectory.Num(); i++)
    {
        FVector Start = PredictedTrajectory[i-1].Position;
        FVector End = PredictedTrajectory[i].Position;
        DrawDebugLine(World, Start, End, FColor::Green, false, 0.0f, 0, 3.0f);
    }

    // Draw current feature
    DrawFeatureDebug(CurrentQuery, FColor::Red);
}

FMotionFeature UMotionMatchingComponent::ExtractCurrentFeature()
{
    FMotionFeature Feature;
    
    if (!OwnerCharacter)
        return Feature;

    Feature.Position = OwnerCharacter->GetActorLocation();
    Feature.Velocity = OwnerCharacter->GetVelocity();
    Feature.Rotation = OwnerCharacter->GetActorRotation();
    Feature.Timestamp = GetWorld()->GetTimeSeconds();

    return Feature;
}

FMotionFeature UMotionMatchingComponent::ExtractFeatureFromAnimation(UAnimSequence* Animation, float Time)
{
    FMotionFeature Feature;
    
    if (!Animation)
        return Feature;

    // This would extract pose data from the animation at the specified time
    // For now, we'll use placeholder values
    Feature.Position = FVector::ZeroVector;
    Feature.Velocity = FVector::ZeroVector;
    Feature.Rotation = FRotator::ZeroRotator;
    Feature.Timestamp = Time;

    return Feature;
}

void UMotionMatchingComponent::SetFeatureWeights(float NewPositionWeight, float NewVelocityWeight, float NewRotationWeight)
{
    PositionWeight = NewPositionWeight;
    VelocityWeight = NewVelocityWeight;
    RotationWeight = NewRotationWeight;
    
    UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Feature weights updated - Pos: %f, Vel: %f, Rot: %f"), 
           PositionWeight, VelocityWeight, RotationWeight);
}

void UMotionMatchingComponent::AddAnimationToDatabase(UAnimSequence* Animation)
{
    if (!Animation)
        return;

    SourceAnimations.AddUnique(Animation);
    
    // Add candidates for this animation
    float AnimLength = Animation->GetPlayLength();
    float SampleRate = 30.0f; // 30 samples per second
    int32 NumSamples = FMath::FloorToInt(AnimLength * SampleRate);

    for (int32 i = 0; i < NumSamples; i++)
    {
        float Time = (float)i / SampleRate;
        
        FMotionCandidate Candidate;
        Candidate.Animation = Animation;
        Candidate.StartTime = Time;
        Candidate.Feature = ExtractFeatureFromAnimation(Animation, Time);
        
        MotionDatabase.Add(Candidate);
    }

    UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Added animation %s with %d candidates"), 
           *Animation->GetName(), NumSamples);
}

void UMotionMatchingComponent::RemoveAnimationFromDatabase(UAnimSequence* Animation)
{
    if (!Animation)
        return;

    SourceAnimations.Remove(Animation);
    
    // Remove all candidates for this animation
    MotionDatabase.RemoveAll([Animation](const FMotionCandidate& Candidate)
    {
        return Candidate.Animation == Animation;
    });

    UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Removed animation %s from database"), 
           *Animation->GetName());
}

void UMotionMatchingComponent::RebuildDatabase()
{
    MotionDatabase.Empty();
    BuildMotionDatabase();
    
    UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Database rebuilt with %d candidates"), 
           MotionDatabase.Num());
}

void UMotionMatchingComponent::SetSearchQuality(int32 Quality)
{
    SearchQuality = FMath::Clamp(Quality, 1, 4);
    
    // Adjust max candidates based on quality
    switch (SearchQuality)
    {
        case 1: MaxSearchCandidates = 5; break;
        case 2: MaxSearchCandidates = 10; break;
        case 3: MaxSearchCandidates = 20; break;
        case 4: MaxSearchCandidates = 50; break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Search quality set to %d (max candidates: %d)"), 
           SearchQuality, MaxSearchCandidates);
}

void UMotionMatchingComponent::SetMaxCandidates(int32 MaxCandidates)
{
    MaxSearchCandidates = FMath::Max(1, MaxCandidates);
    UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Max candidates set to %d"), MaxSearchCandidates);
}

void UMotionMatchingComponent::EnableAdaptiveQuality(bool bEnable)
{
    bAdaptiveQualityEnabled = bEnable;
    UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Adaptive quality %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UMotionMatchingComponent::BuildMotionDatabase()
{
    MotionDatabase.Empty();
    
    for (UAnimSequence* Animation : SourceAnimations)
    {
        if (!Animation)
            continue;

        AddAnimationToDatabase(Animation);
    }

    ValidateDatabase();
}

float UMotionMatchingComponent::CalculateFeatureDistance(const FMotionFeature& A, const FMotionFeature& B)
{
    float PositionDistance = FVector::Dist(A.Position, B.Position) * PositionWeight;
    float VelocityDistance = FVector::Dist(A.Velocity, B.Velocity) * VelocityWeight;
    float RotationDistance = FMath::Abs(FRotator::NormalizeAxis(A.Rotation.Yaw - B.Rotation.Yaw)) * RotationWeight;
    
    return PositionDistance + VelocityDistance + RotationDistance;
}

void UMotionMatchingComponent::UpdateTrajectoryHistory(const FVector& CurrentPosition, const FVector& CurrentVelocity)
{
    FTrajectoryPoint Point;
    Point.Position = CurrentPosition;
    Point.Velocity = CurrentVelocity;
    Point.Time = GetWorld()->GetTimeSeconds();
    
    TrajectoryHistory.Add(Point);
    
    // Keep history size limited
    if (TrajectoryHistory.Num() > TrajectoryHistorySize)
    {
        TrajectoryHistory.RemoveAt(0);
    }
}

FVector UMotionMatchingComponent::PredictFuturePosition(float Time)
{
    if (!OwnerCharacter)
        return FVector::ZeroVector;

    FVector CurrentPosition = OwnerCharacter->GetActorLocation();
    FVector CurrentVelocity = OwnerCharacter->GetVelocity();
    
    // Simple linear prediction (could be enhanced with acceleration)
    return CurrentPosition + (CurrentVelocity * Time);
}

void UMotionMatchingComponent::OptimizeSearchQuality()
{
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    float FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Adjust quality based on frame time
    if (FrameTime > QualityAdaptationThreshold && SearchQuality > 1)
    {
        SetSearchQuality(SearchQuality - 1);
    }
    else if (FrameTime < QualityAdaptationThreshold * 0.5f && SearchQuality < 4)
    {
        SetSearchQuality(SearchQuality + 1);
    }
}

void UMotionMatchingComponent::ValidateDatabase()
{
    int32 ValidCandidates = 0;
    
    for (const FMotionCandidate& Candidate : MotionDatabase)
    {
        if (Candidate.Animation)
        {
            ValidCandidates++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Database validation - %d/%d valid candidates"), 
           ValidCandidates, MotionDatabase.Num());
}

void UMotionMatchingComponent::DrawFeatureDebug(const FMotionFeature& Feature, const FColor& Color)
{
    if (!OwnerCharacter)
        return;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World)
        return;

    // Draw position
    DrawDebugSphere(World, Feature.Position, 10.0f, 8, Color, false, 0.0f, 0, 2.0f);
    
    // Draw velocity
    FVector VelEnd = Feature.Position + (Feature.Velocity * 0.01f); // Scale for visibility
    DrawDebugDirectionalArrow(World, Feature.Position, VelEnd, 20.0f, Color, false, 0.0f, 0, 3.0f);
}

void UMotionMatchingComponent::LogMotionMatchingStats()
{
    UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent Stats:"));
    UE_LOG(LogTemp, Warning, TEXT("- Database size: %d candidates"), MotionDatabase.Num());
    UE_LOG(LogTemp, Warning, TEXT("- Search quality: %d"), SearchQuality);
    UE_LOG(LogTemp, Warning, TEXT("- Average search time: %f ms"), AverageSearchTime * 1000.0f);
    UE_LOG(LogTemp, Warning, TEXT("- Trajectory history: %d points"), TrajectoryHistory.Num());
}