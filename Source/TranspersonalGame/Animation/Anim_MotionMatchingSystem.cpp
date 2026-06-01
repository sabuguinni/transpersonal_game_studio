#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS sampling

    // Initialize configuration
    MotionSampleRate = 60.0f;
    PoseMatchThreshold = 0.8f;
    BlendSpeed = 5.0f;
    MaxDatabaseEntries = 1000;

    // Initialize state
    bMotionMatchingEnabled = true;
    LastSampleTime = 0.0f;

    // Initialize current motion data
    CurrentMotion = FAnim_MotionData();
    CurrentBestMatch = FAnim_PoseMatchData();
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize motion database with basic entries
    MotionDatabase.Reserve(MaxDatabaseEntries);
    PoseDatabase.Reserve(MaxDatabaseEntries);

    // Sample initial motion state
    SampleCurrentMotion();
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bMotionMatchingEnabled)
    {
        return;
    }

    // Sample motion at configured rate
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastSampleTime >= (1.0f / MotionSampleRate))
    {
        SampleCurrentMotion();
        LastSampleTime = CurrentTime;

        // Find best matching pose
        CurrentBestMatch = FindBestPoseMatch(CurrentMotion);

        // Clean up old entries periodically
        if (MotionDatabase.Num() > MaxDatabaseEntries)
        {
            CleanupOldEntries();
        }
    }
}

FAnim_MotionData UAnim_MotionMatchingSystem::AnalyzeCurrentMotion()
{
    FAnim_MotionData MotionData;

    // Get owner velocity and movement data
    MotionData.Velocity = GetOwnerVelocity();
    MotionData.Speed = MotionData.Velocity.Size();
    MotionData.bIsMoving = MotionData.Speed > 1.0f;
    MotionData.bIsInAir = IsOwnerInAir();
    MotionData.bIsCrouching = IsOwnerCrouching();

    // Calculate movement direction relative to forward vector
    if (AActor* Owner = GetOwner())
    {
        FVector ForwardVector = Owner->GetActorForwardVector();
        FVector VelocityNormalized = MotionData.Velocity.GetSafeNormal();
        MotionData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
    }

    // Determine movement state
    if (MotionData.bIsInAir)
    {
        MotionData.MovementState = EAnim_MovementState::Jumping;
    }
    else if (MotionData.bIsCrouching)
    {
        MotionData.MovementState = EAnim_MovementState::Crouching;
    }
    else if (MotionData.Speed > 300.0f)
    {
        MotionData.MovementState = EAnim_MovementState::Running;
    }
    else if (MotionData.Speed > 50.0f)
    {
        MotionData.MovementState = EAnim_MovementState::Walking;
    }
    else
    {
        MotionData.MovementState = EAnim_MovementState::Idle;
    }

    return MotionData;
}

FAnim_PoseMatchData UAnim_MotionMatchingSystem::FindBestPoseMatch(const FAnim_MotionData& TargetMotion)
{
    FAnim_PoseMatchData BestMatch;
    float BestScore = 0.0f;

    for (const FAnim_PoseMatchData& PoseData : PoseDatabase)
    {
        // Find corresponding motion data for this pose
        for (const FAnim_MotionData& MotionData : MotionDatabase)
        {
            if (FMath::Abs(MotionData.Speed - TargetMotion.Speed) < 50.0f &&
                MotionData.MovementState == TargetMotion.MovementState)
            {
                float Similarity = CalculateMotionSimilarity(MotionData, TargetMotion);
                if (Similarity > BestScore && Similarity >= PoseMatchThreshold)
                {
                    BestScore = Similarity;
                    BestMatch = PoseData;
                    BestMatch.MatchScore = Similarity;
                }
            }
        }
    }

    return BestMatch;
}

void UAnim_MotionMatchingSystem::UpdateMotionDatabase(const FAnim_MotionData& NewMotionData)
{
    // Add new motion data to database
    MotionDatabase.Add(NewMotionData);

    // Create corresponding pose data (simplified for now)
    FAnim_PoseMatchData NewPoseData;
    NewPoseData.PoseName = FName(*FString::Printf(TEXT("Pose_%d"), MotionDatabase.Num()));
    NewPoseData.TimeStamp = GetWorld()->GetTimeSeconds();
    NewPoseData.MatchScore = 1.0f;

    PoseDatabase.Add(NewPoseData);
}

float UAnim_MotionMatchingSystem::CalculateMotionSimilarity(const FAnim_MotionData& MotionA, const FAnim_MotionData& MotionB)
{
    float Similarity = 1.0f;

    // Speed similarity (weighted 40%)
    float SpeedDiff = FMath::Abs(MotionA.Speed - MotionB.Speed);
    float SpeedSimilarity = FMath::Clamp(1.0f - (SpeedDiff / 500.0f), 0.0f, 1.0f);
    Similarity *= (SpeedSimilarity * 0.4f + 0.6f);

    // Direction similarity (weighted 30%)
    float DirectionDiff = FMath::Abs(MotionA.Direction - MotionB.Direction);
    float DirectionSimilarity = FMath::Clamp(1.0f - (DirectionDiff / 180.0f), 0.0f, 1.0f);
    Similarity *= (DirectionSimilarity * 0.3f + 0.7f);

    // State matching (weighted 30%)
    if (MotionA.MovementState == MotionB.MovementState)
    {
        Similarity *= 1.0f;
    }
    else
    {
        Similarity *= 0.5f; // Penalty for different states
    }

    // Boolean state matching
    if (MotionA.bIsInAir == MotionB.bIsInAir)
    {
        Similarity *= 1.0f;
    }
    else
    {
        Similarity *= 0.3f; // Heavy penalty for air/ground mismatch
    }

    return FMath::Clamp(Similarity, 0.0f, 1.0f);
}

void UAnim_MotionMatchingSystem::BlendToTargetPose(const FAnim_PoseMatchData& TargetPose, float BlendTime)
{
    // This would integrate with the animation system to blend to the target pose
    // For now, we'll just log the blend request
    if (AActor* Owner = GetOwner())
    {
        UE_LOG(LogTemp, Log, TEXT("Motion Matching: Blending to pose %s with score %f over %f seconds"),
            *TargetPose.PoseName.ToString(), TargetPose.MatchScore, BlendTime);
    }
}

void UAnim_MotionMatchingSystem::SetMotionMatchingEnabled(bool bEnabled)
{
    bMotionMatchingEnabled = bEnabled;
    
    if (bEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Motion Matching System: ENABLED"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Motion Matching System: DISABLED"));
    }
}

void UAnim_MotionMatchingSystem::SampleCurrentMotion()
{
    CurrentMotion = AnalyzeCurrentMotion();
    
    // Add to database if motion is significantly different from recent entries
    bool bShouldAddToDatabase = true;
    if (MotionDatabase.Num() > 0)
    {
        const FAnim_MotionData& LastMotion = MotionDatabase.Last();
        float Similarity = CalculateMotionSimilarity(CurrentMotion, LastMotion);
        if (Similarity > 0.95f) // Very similar to last sample
        {
            bShouldAddToDatabase = false;
        }
    }

    if (bShouldAddToDatabase)
    {
        UpdateMotionDatabase(CurrentMotion);
    }
}

void UAnim_MotionMatchingSystem::CleanupOldEntries()
{
    // Remove oldest 10% of entries when database is full
    int32 EntriesToRemove = FMath::Max(1, MaxDatabaseEntries / 10);
    
    if (MotionDatabase.Num() > EntriesToRemove)
    {
        MotionDatabase.RemoveAt(0, EntriesToRemove);
    }
    
    if (PoseDatabase.Num() > EntriesToRemove)
    {
        PoseDatabase.RemoveAt(0, EntriesToRemove);
    }

    UE_LOG(LogTemp, Log, TEXT("Motion Matching: Cleaned up %d old entries. Database size: %d"), 
        EntriesToRemove, MotionDatabase.Num());
}

FVector UAnim_MotionMatchingSystem::GetOwnerVelocity() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            return MovementComp->Velocity;
        }
    }
    return FVector::ZeroVector;
}

bool UAnim_MotionMatchingSystem::IsOwnerInAir() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            return MovementComp->IsFalling();
        }
    }
    return false;
}

bool UAnim_MotionMatchingSystem::IsOwnerCrouching() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        return Character->bIsCrouched;
    }
    return false;
}