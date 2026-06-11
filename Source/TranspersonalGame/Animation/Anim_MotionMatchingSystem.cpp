#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize motion matching weights
    VelocityWeight = 1.0f;
    DirectionWeight = 0.8f;
    SpeedWeight = 0.6f;
    SearchRadius = 100.0f;
    MaxSearchFrames = 60;
    
    // Performance settings
    UpdateFrequency = 0.1f; // Update 10 times per second
    LastUpdateTime = 0.0f;
    
    // Initialize results
    BestMatchAnimation = nullptr;
    BestMatchTime = 0.0f;
    MatchScore = 0.0f;
    OwnerCharacter = nullptr;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner character reference
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingSystem: Owner is not a Character!"));
        return;
    }
    
    // Build motion database if not already built
    if (MotionDatabase.Num() == 0)
    {
        BuildMotionDatabase();
    }
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingSystem initialized with %d motion clips"), MotionDatabase.Num());
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter)
        return;
    
    // Update at specified frequency for performance
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime < UpdateFrequency)
        return;
    
    LastUpdateTime = CurrentTime;
    
    // Update motion data and find best match
    UpdateMotionData();
    FindBestMotionMatch();
    
    // Apply the best match
    if (BestMatchAnimation && MatchScore > 0.5f)
    {
        PlayBestMatch();
    }
}

void UAnim_MotionMatchingSystem::UpdateMotionData()
{
    if (!OwnerCharacter)
        return;
    
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
        return;
    
    // Update current motion data
    CurrentMotionData.Velocity = MovementComp->Velocity;
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    
    if (CurrentMotionData.Speed > 0.1f)
    {
        CurrentMotionData.Direction = CurrentMotionData.Velocity.GetSafeNormal();
    }
    else
    {
        CurrentMotionData.Direction = OwnerCharacter->GetActorForwardVector();
    }
    
    CurrentMotionData.bIsOnGround = MovementComp->IsMovingOnGround();
    CurrentMotionData.bIsCrouching = MovementComp->IsCrouching();
    
    // Calculate turn rate based on angular velocity
    FVector AngularVelocity = OwnerCharacter->GetActorRotation().Vector() - OwnerCharacter->GetActorForwardVector();
    CurrentMotionData.TurnRate = AngularVelocity.Size();
}

void UAnim_MotionMatchingSystem::FindBestMotionMatch()
{
    if (MotionDatabase.Num() == 0)
        return;
    
    float BestScore = -1.0f;
    UAnimSequence* BestAnim = nullptr;
    float BestTime = 0.0f;
    
    // Search through all motion clips
    for (const FAnim_MotionClip& Clip : MotionDatabase)
    {
        if (!Clip.AnimSequence || Clip.MotionFrames.Num() == 0)
            continue;
        
        // Search through frames in this clip
        int32 FramesToSearch = FMath::Min(Clip.MotionFrames.Num(), MaxSearchFrames);
        
        for (int32 FrameIndex = 0; FrameIndex < FramesToSearch; FrameIndex++)
        {
            const FAnim_MotionData& ClipMotion = Clip.MotionFrames[FrameIndex];
            float Score = CalculateMotionScore(CurrentMotionData, ClipMotion);
            
            if (Score > BestScore)
            {
                BestScore = Score;
                BestAnim = Clip.AnimSequence;
                BestTime = (float)FrameIndex / (float)FramesToSearch * Clip.ClipDuration;
            }
        }
    }
    
    // Update best match results
    BestMatchAnimation = BestAnim;
    BestMatchTime = BestTime;
    MatchScore = BestScore;
}

float UAnim_MotionMatchingSystem::CalculateMotionScore(const FAnim_MotionData& TargetMotion, const FAnim_MotionData& ClipMotion)
{
    float Score = 0.0f;
    float TotalWeight = 0.0f;
    
    // Velocity matching
    if (VelocityWeight > 0.0f)
    {
        float VelocityDiff = FVector::Dist(TargetMotion.Velocity, ClipMotion.Velocity);
        float VelocityScore = FMath::Exp(-VelocityDiff / SearchRadius);
        Score += VelocityScore * VelocityWeight;
        TotalWeight += VelocityWeight;
    }
    
    // Direction matching
    if (DirectionWeight > 0.0f)
    {
        float DirectionDot = FVector::DotProduct(TargetMotion.Direction, ClipMotion.Direction);
        float DirectionScore = (DirectionDot + 1.0f) * 0.5f; // Convert from [-1,1] to [0,1]
        Score += DirectionScore * DirectionWeight;
        TotalWeight += DirectionWeight;
    }
    
    // Speed matching
    if (SpeedWeight > 0.0f)
    {
        float SpeedDiff = FMath::Abs(TargetMotion.Speed - ClipMotion.Speed);
        float SpeedScore = FMath::Exp(-SpeedDiff / SearchRadius);
        Score += SpeedScore * SpeedWeight;
        TotalWeight += SpeedWeight;
    }
    
    // Boolean state matching (ground, crouching)
    if (TargetMotion.bIsOnGround == ClipMotion.bIsOnGround)
        Score += 0.2f;
    if (TargetMotion.bIsCrouching == ClipMotion.bIsCrouching)
        Score += 0.2f;
    
    TotalWeight += 0.4f; // For the boolean states
    
    // Normalize score
    return TotalWeight > 0.0f ? Score / TotalWeight : 0.0f;
}

void UAnim_MotionMatchingSystem::AddMotionClip(UAnimSequence* Animation, const FString& ClipName)
{
    if (!Animation)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingSystem: Cannot add null animation clip"));
        return;
    }
    
    FAnim_MotionClip NewClip;
    NewClip.AnimSequence = Animation;
    NewClip.ClipName = ClipName;
    NewClip.ClipDuration = Animation->GetPlayLength();
    
    // Extract motion data from animation
    ExtractMotionDataFromAnimation(Animation, NewClip.MotionFrames);
    
    MotionDatabase.Add(NewClip);
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingSystem: Added clip '%s' with %d frames"), 
           *ClipName, NewClip.MotionFrames.Num());
}

void UAnim_MotionMatchingSystem::BuildMotionDatabase()
{
    // Clear existing database
    MotionDatabase.Empty();
    
    // This would typically load animations from a predefined set
    // For now, we'll create placeholder clips that can be configured in Blueprint
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingSystem: Motion database built with %d clips"), MotionDatabase.Num());
}

void UAnim_MotionMatchingSystem::PlayBestMatch()
{
    if (!OwnerCharacter || !BestMatchAnimation)
        return;
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
        return;
    
    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!AnimInstance)
        return;
    
    // Play the best match animation
    // This would typically involve more sophisticated blending
    // For now, we'll log the result
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingSystem: Playing best match - Score: %.2f, Time: %.2f"), 
           MatchScore, BestMatchTime);
}

void UAnim_MotionMatchingSystem::ExtractMotionDataFromAnimation(UAnimSequence* Animation, TArray<FAnim_MotionData>& OutMotionFrames)
{
    if (!Animation)
        return;
    
    OutMotionFrames.Empty();
    
    // This is a simplified version - real implementation would extract actual motion data
    // from animation curves and bone transforms
    
    float Duration = Animation->GetPlayLength();
    float FrameRate = 30.0f; // Assume 30 FPS
    int32 NumFrames = FMath::CeilToInt(Duration * FrameRate);
    
    for (int32 i = 0; i < NumFrames; i++)
    {
        FAnim_MotionData FrameData;
        
        // Placeholder motion data - would be extracted from actual animation
        float Time = (float)i / FrameRate;
        float NormalizedTime = Time / Duration;
        
        // Generate some sample motion data based on animation name
        FString AnimName = Animation->GetName();
        if (AnimName.Contains(TEXT("Walk")))
        {
            FrameData.Speed = 150.0f;
            FrameData.Velocity = FVector(FrameData.Speed, 0, 0);
        }
        else if (AnimName.Contains(TEXT("Run")))
        {
            FrameData.Speed = 400.0f;
            FrameData.Velocity = FVector(FrameData.Speed, 0, 0);
        }
        else if (AnimName.Contains(TEXT("Idle")))
        {
            FrameData.Speed = 0.0f;
            FrameData.Velocity = FVector::ZeroVector;
        }
        
        FrameData.Direction = FrameData.Velocity.GetSafeNormal();
        FrameData.bIsOnGround = true;
        FrameData.bIsCrouching = AnimName.Contains(TEXT("Crouch"));
        FrameData.TurnRate = 0.0f;
        
        OutMotionFrames.Add(FrameData);
    }
}

void UAnim_MotionMatchingSystem::DebugDrawMotionData()
{
    if (!OwnerCharacter)
        return;
    
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    FVector ActorLocation = OwnerCharacter->GetActorLocation();
    
    // Draw velocity vector
    DrawDebugLine(World, ActorLocation, ActorLocation + CurrentMotionData.Velocity, 
                  FColor::Red, false, 0.1f, 0, 2.0f);
    
    // Draw direction vector
    DrawDebugLine(World, ActorLocation + FVector(0, 0, 50), 
                  ActorLocation + FVector(0, 0, 50) + CurrentMotionData.Direction * 100.0f, 
                  FColor::Blue, false, 0.1f, 0, 2.0f);
    
    // Draw speed as text
    FString SpeedText = FString::Printf(TEXT("Speed: %.1f"), CurrentMotionData.Speed);
    DrawDebugString(World, ActorLocation + FVector(0, 0, 100), SpeedText, nullptr, FColor::White, 0.1f);
}

void UAnim_MotionMatchingSystem::LogMotionMatchingInfo()
{
    UE_LOG(LogTemp, Log, TEXT("=== Motion Matching Info ==="));
    UE_LOG(LogTemp, Log, TEXT("Current Speed: %.2f"), CurrentMotionData.Speed);
    UE_LOG(LogTemp, Log, TEXT("Current Velocity: %s"), *CurrentMotionData.Velocity.ToString());
    UE_LOG(LogTemp, Log, TEXT("Current Direction: %s"), *CurrentMotionData.Direction.ToString());
    UE_LOG(LogTemp, Log, TEXT("Is On Ground: %s"), CurrentMotionData.bIsOnGround ? TEXT("True") : TEXT("False"));
    UE_LOG(LogTemp, Log, TEXT("Is Crouching: %s"), CurrentMotionData.bIsCrouching ? TEXT("True") : TEXT("False"));
    UE_LOG(LogTemp, Log, TEXT("Best Match Score: %.2f"), MatchScore);
    UE_LOG(LogTemp, Log, TEXT("Motion Database Size: %d"), MotionDatabase.Num());
    UE_LOG(LogTemp, Log, TEXT("============================="));
}