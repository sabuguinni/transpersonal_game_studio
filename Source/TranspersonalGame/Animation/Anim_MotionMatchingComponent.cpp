#include "Anim_MotionMatchingComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Default settings
    SearchRadius = 1000.0f;
    MinimumMatchThreshold = 0.7f;
    bEnableDebugDrawing = false;
    UpdateFrequency = 30.0f;
    
    // Runtime initialization
    CurrentBestFrameIndex = -1;
    CurrentMatchScore = 0.0f;
    LastUpdateTime = 0.0f;
    bIsInitialized = false;
    bMotionMatchingEnabled = true;
    AccumulatedTime = 0.0f;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingComponent: BeginPlay for %s"), 
           *GetOwner()->GetName());
    
    // Initialize component
    bIsInitialized = true;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Build default database if we have source animations
    if (MotionDatabase.SourceAnimations.Num() > 0)
    {
        BuildMotionDatabase(MotionDatabase.SourceAnimations);
    }
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized || !bMotionMatchingEnabled)
    {
        return;
    }
    
    AccumulatedTime += DeltaTime;
    
    // Update at specified frequency
    float UpdateInterval = 1.0f / UpdateFrequency;
    if (AccumulatedTime >= UpdateInterval)
    {
        UpdateInternalState(AccumulatedTime);
        AccumulatedTime = 0.0f;
    }
    
    // Draw debug information
    if (bEnableDebugDrawing)
    {
        DrawDebugMotionData();
    }
}

int32 UAnim_MotionMatchingComponent::FindBestMotionMatch(const FAnim_MotionQuery& Query)
{
    if (MotionDatabase.MotionFrames.Num() == 0)
    {
        return -1;
    }
    
    int32 BestFrameIndex = -1;
    float BestScore = -1.0f;
    
    // Search through all frames in the database
    for (int32 i = 0; i < MotionDatabase.MotionFrames.Num(); ++i)
    {
        const FAnim_MotionFrame& Frame = MotionDatabase.MotionFrames[i];
        float Score = CalculateMotionScore(Frame, Query);
        
        if (Score > BestScore && Score >= MinimumMatchThreshold)
        {
            BestScore = Score;
            BestFrameIndex = i;
        }
    }
    
    // Update current best match
    CurrentBestFrameIndex = BestFrameIndex;
    CurrentMatchScore = BestScore;
    
    return BestFrameIndex;
}

float UAnim_MotionMatchingComponent::CalculateMotionScore(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query)
{
    float VelocityScore = CalculateVelocityScore(Frame, Query);
    float DirectionScore = CalculateDirectionScore(Frame, Query);
    float PositionScore = CalculatePositionScore(Frame, Query);
    
    // Weighted combination of scores
    float TotalScore = (VelocityScore * Query.VelocityWeight) +
                      (DirectionScore * Query.DirectionWeight) +
                      (PositionScore * Query.PositionWeight);
    
    // Normalize by total weight
    float TotalWeight = Query.VelocityWeight + Query.DirectionWeight + Query.PositionWeight;
    if (TotalWeight > 0.0f)
    {
        TotalScore /= TotalWeight;
    }
    
    return FMath::Clamp(TotalScore, 0.0f, 1.0f);
}

void UAnim_MotionMatchingComponent::UpdateMotionQuery(FVector DesiredVelocity, FVector CurrentPosition, FRotator CurrentRotation)
{
    CurrentQuery.DesiredVelocity = DesiredVelocity;
    CurrentQuery.DesiredSpeed = DesiredVelocity.Size();
    CurrentQuery.DesiredDirection = DesiredVelocity.GetSafeNormal();
    CurrentQuery.CurrentPosition = CurrentPosition;
    CurrentQuery.CurrentRotation = CurrentRotation;
}

FAnim_MotionFrame UAnim_MotionMatchingComponent::GetMotionFrame(int32 FrameIndex)
{
    if (IsValidFrameIndex(FrameIndex))
    {
        return MotionDatabase.MotionFrames[FrameIndex];
    }
    
    return FAnim_MotionFrame();
}

bool UAnim_MotionMatchingComponent::IsValidFrameIndex(int32 FrameIndex)
{
    return FrameIndex >= 0 && FrameIndex < MotionDatabase.MotionFrames.Num();
}

void UAnim_MotionMatchingComponent::BuildMotionDatabase(const TArray<UAnimSequence*>& Animations)
{
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingComponent: Building database with %d animations"), 
           Animations.Num());
    
    ClearMotionDatabase();
    
    for (UAnimSequence* Animation : Animations)
    {
        if (Animation)
        {
            AddAnimationToDatabase(Animation);
        }
    }
    
    MotionDatabase.SourceAnimations = Animations;
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingComponent: Database built with %d frames"), 
           MotionDatabase.MotionFrames.Num());
}

void UAnim_MotionMatchingComponent::AddAnimationToDatabase(UAnimSequence* Animation)
{
    if (!Animation)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Null animation provided"));
        return;
    }
    
    ExtractMotionFramesFromAnimation(Animation);
}

void UAnim_MotionMatchingComponent::ClearMotionDatabase()
{
    MotionDatabase.MotionFrames.Empty();
    MotionDatabase.DatabaseDuration = 0.0f;
    CurrentBestFrameIndex = -1;
    CurrentMatchScore = 0.0f;
}

void UAnim_MotionMatchingComponent::SetMotionMatchingEnabled(bool bEnabled)
{
    bMotionMatchingEnabled = bEnabled;
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingComponent: Motion matching %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

bool UAnim_MotionMatchingComponent::IsMotionMatchingEnabled() const
{
    return bMotionMatchingEnabled;
}

void UAnim_MotionMatchingComponent::DrawDebugMotionData()
{
    if (!GetWorld() || CurrentBestFrameIndex < 0)
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Draw current query
    DrawDebugSphere(GetWorld(), OwnerLocation, 50.0f, 12, FColor::Blue, false, 0.1f);
    DrawDebugDirectionalArrow(GetWorld(), OwnerLocation, 
                             OwnerLocation + CurrentQuery.DesiredVelocity, 
                             20.0f, FColor::Green, false, 0.1f);
    
    // Draw best match information
    if (IsValidFrameIndex(CurrentBestFrameIndex))
    {
        const FAnim_MotionFrame& BestFrame = MotionDatabase.MotionFrames[CurrentBestFrameIndex];
        FVector DebugLocation = OwnerLocation + FVector(0, 0, 100);
        
        DrawDebugString(GetWorld(), DebugLocation, 
                       FString::Printf(TEXT("Best Match: %s (Score: %.2f)"), 
                                     *BestFrame.AnimationName, CurrentMatchScore),
                       nullptr, FColor::White, 0.1f);
    }
}

void UAnim_MotionMatchingComponent::ExtractMotionFramesFromAnimation(UAnimSequence* Animation)
{
    if (!Animation)
    {
        return;
    }
    
    float AnimDuration = Animation->GetPlayLength();
    float FrameTime = 1.0f / MotionDatabase.FramesPerSecond;
    int32 NumFrames = FMath::FloorToInt(AnimDuration / FrameTime);
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingComponent: Extracting %d frames from %s"), 
           NumFrames, *Animation->GetName());
    
    for (int32 i = 0; i < NumFrames; ++i)
    {
        float Time = i * FrameTime;
        
        FAnim_MotionFrame NewFrame;
        NewFrame.TimeStamp = Time;
        NewFrame.AnimationName = Animation->GetName();
        
        // For now, create placeholder data
        // In a full implementation, you would extract actual bone transforms
        // and root motion data from the animation sequence
        NewFrame.RootPosition = FVector(0, 0, 0);
        NewFrame.RootRotation = FRotator(0, 0, 0);
        NewFrame.Velocity = FVector(100, 0, 0); // Placeholder velocity
        NewFrame.Acceleration = FVector(0, 0, 0);
        
        MotionDatabase.MotionFrames.Add(NewFrame);
    }
    
    MotionDatabase.DatabaseDuration += AnimDuration;
}

float UAnim_MotionMatchingComponent::CalculateVelocityScore(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query)
{
    float VelocityDifference = FVector::Dist(Frame.Velocity, Query.DesiredVelocity);
    float MaxVelocity = FMath::Max(Frame.Velocity.Size(), Query.DesiredVelocity.Size());
    
    if (MaxVelocity <= 0.0f)
    {
        return 1.0f; // Both velocities are zero
    }
    
    float NormalizedDifference = VelocityDifference / MaxVelocity;
    return FMath::Clamp(1.0f - NormalizedDifference, 0.0f, 1.0f);
}

float UAnim_MotionMatchingComponent::CalculateDirectionScore(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query)
{
    FVector FrameDirection = Frame.Velocity.GetSafeNormal();
    FVector QueryDirection = Query.DesiredDirection;
    
    if (FrameDirection.IsZero() || QueryDirection.IsZero())
    {
        return 0.5f; // Neutral score for zero directions
    }
    
    float DotProduct = FVector::DotProduct(FrameDirection, QueryDirection);
    return FMath::Clamp((DotProduct + 1.0f) * 0.5f, 0.0f, 1.0f);
}

float UAnim_MotionMatchingComponent::CalculatePositionScore(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query)
{
    // For position matching, we might consider relative positions or trajectory matching
    // For now, return a neutral score
    return 0.5f;
}

void UAnim_MotionMatchingComponent::UpdateInternalState(float DeltaTime)
{
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Update current query based on owner's state
    if (GetOwner())
    {
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        FRotator CurrentRotation = GetOwner()->GetActorRotation();
        
        // For now, use a simple desired velocity
        // In practice, this would come from player input or AI
        FVector DesiredVel = GetOwner()->GetActorForwardVector() * 100.0f;
        
        UpdateMotionQuery(DesiredVel, CurrentLocation, CurrentRotation);
        
        // Find best match
        FindBestMotionMatch(CurrentQuery);
    }
}