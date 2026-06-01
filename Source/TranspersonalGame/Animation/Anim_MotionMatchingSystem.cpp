#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize motion matching weights
    VelocityWeight = 1.0f;
    AccelerationWeight = 0.5f;
    DirectionWeight = 0.8f;
    GroundDistanceWeight = 0.3f;
    SearchRadius = 2.0f;
    BlendTime = 0.2f;

    // Initialize state
    CurrentBestMatchIndex = -1;
    CurrentMatchScore = 0.0f;
    bMotionMatchingEnabled = true;
    LastUpdateTime = 0.0f;
    OwnerCharacter = nullptr;
    AnimInstance = nullptr;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();

    UpdateCharacterReferences();
    InitializeMotionDatabase();
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bMotionMatchingEnabled || !OwnerCharacter)
    {
        return;
    }

    // Update current pose data
    UpdateCurrentPose();

    // Find best motion match every few frames for performance
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= 0.1f) // Update at 10Hz
    {
        int32 BestMatch = FindBestMotionMatch();
        if (BestMatch != CurrentBestMatchIndex && BestMatch >= 0)
        {
            ApplyMotionMatch(BestMatch);
        }
        LastUpdateTime = 0.0f;
    }
}

void UAnim_MotionMatchingSystem::UpdateCurrentPose()
{
    if (!OwnerCharacter)
    {
        return;
    }

    UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
    if (!MovementComponent)
    {
        return;
    }

    // Update velocity and acceleration
    FVector CurrentVelocity = MovementComponent->Velocity;
    FVector CurrentAcceleration = (CurrentVelocity - LastVelocity) / GetWorld()->GetDeltaSeconds();
    
    CurrentPose.Velocity = CurrentVelocity;
    CurrentPose.Acceleration = CurrentAcceleration;
    CurrentPose.Speed = CurrentVelocity.Size();
    
    // Calculate direction relative to character forward
    if (CurrentPose.Speed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = CurrentVelocity.GetSafeNormal();
        CurrentPose.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityDirection)));
    }
    else
    {
        CurrentPose.Direction = 0.0f;
    }

    // Update movement state
    CurrentPose.bIsInAir = MovementComponent->IsFalling();
    CurrentPose.bIsCrouching = MovementComponent->IsCrouching();

    // Calculate ground distance
    FHitResult HitResult;
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic))
    {
        CurrentPose.GroundDistance = HitResult.Distance;
    }
    else
    {
        CurrentPose.GroundDistance = 200.0f;
    }

    // Store for next frame
    LastVelocity = CurrentVelocity;
    LastAcceleration = CurrentAcceleration;
}

int32 UAnim_MotionMatchingSystem::FindBestMotionMatch()
{
    if (MotionDatabase.Num() == 0)
    {
        return -1;
    }

    int32 BestIndex = -1;
    float BestScore = FLT_MAX;

    for (int32 i = 0; i < MotionDatabase.Num(); i++)
    {
        float Score = CalculateMotionScore(MotionDatabase[i]);
        if (Score < BestScore)
        {
            BestScore = Score;
            BestIndex = i;
        }
    }

    CurrentMatchScore = BestScore;
    return BestIndex;
}

float UAnim_MotionMatchingSystem::CalculateMotionScore(const FAnim_MotionDatabaseEntry& Entry)
{
    float Score = 0.0f;

    // Velocity matching
    float VelocityScore = CalculateVelocityScore(Entry.PoseData.Velocity, CurrentPose.Velocity);
    Score += VelocityScore * VelocityWeight;

    // Acceleration matching
    float AccelScore = CalculateVelocityScore(Entry.PoseData.Acceleration, CurrentPose.Acceleration);
    Score += AccelScore * AccelerationWeight;

    // Direction matching
    float DirectionScore = CalculateDirectionScore(Entry.PoseData.Direction, CurrentPose.Direction);
    Score += DirectionScore * DirectionWeight;

    // Ground distance matching
    float GroundScore = FMath::Abs(Entry.PoseData.GroundDistance - CurrentPose.GroundDistance);
    Score += GroundScore * GroundDistanceWeight;

    // Boolean state penalties
    if (Entry.PoseData.bIsInAir != CurrentPose.bIsInAir)
    {
        Score += 10.0f; // Heavy penalty for air state mismatch
    }

    if (Entry.PoseData.bIsCrouching != CurrentPose.bIsCrouching)
    {
        Score += 5.0f; // Moderate penalty for crouch state mismatch
    }

    // Apply quality modifier
    Score /= FMath::Max(Entry.Quality, 0.1f);

    return Score;
}

void UAnim_MotionMatchingSystem::ApplyMotionMatch(int32 MatchIndex)
{
    if (!IsValidMotionMatch(MatchIndex) || !AnimInstance)
    {
        return;
    }

    CurrentBestMatchIndex = MatchIndex;
    const FAnim_MotionDatabaseEntry& Entry = MotionDatabase[MatchIndex];

    if (Entry.AnimationSequence)
    {
        // Apply the animation with blending
        // Note: In a full implementation, this would use more sophisticated blending
        UE_LOG(LogTemp, Log, TEXT("Motion Matching: Applied animation %s at time %f"), 
               *Entry.AnimationSequence->GetName(), Entry.StartTime);
    }
}

void UAnim_MotionMatchingSystem::AddMotionToDatabase(UAnimSequence* Animation, float StartTime, float EndTime, const TArray<FString>& Tags)
{
    if (!Animation)
    {
        return;
    }

    FAnim_MotionDatabaseEntry NewEntry;
    NewEntry.AnimationSequence = Animation;
    NewEntry.StartTime = StartTime;
    NewEntry.EndTime = EndTime;
    NewEntry.Tags = Tags;
    NewEntry.Quality = 1.0f;

    // Initialize pose data - in a full implementation, this would be extracted from the animation
    NewEntry.PoseData.Velocity = FVector::ZeroVector;
    NewEntry.PoseData.Speed = 0.0f;
    NewEntry.PoseData.Direction = 0.0f;
    NewEntry.PoseData.bIsInAir = false;
    NewEntry.PoseData.bIsCrouching = false;
    NewEntry.PoseData.GroundDistance = 0.0f;

    MotionDatabase.Add(NewEntry);
}

void UAnim_MotionMatchingSystem::BuildMotionDatabase()
{
    // Clear existing database
    ClearMotionDatabase();

    // In a full implementation, this would scan for animation assets and build the database
    UE_LOG(LogTemp, Log, TEXT("Motion Matching: Building motion database"));

    // Add default entries for common locomotion states
    // This is a simplified version - real implementation would analyze actual animations
    
    // Idle state
    FAnim_MotionDatabaseEntry IdleEntry;
    IdleEntry.PoseData.Speed = 0.0f;
    IdleEntry.PoseData.Velocity = FVector::ZeroVector;
    IdleEntry.Quality = 1.0f;
    IdleEntry.Tags.Add(TEXT("Idle"));
    MotionDatabase.Add(IdleEntry);

    // Walk state
    FAnim_MotionDatabaseEntry WalkEntry;
    WalkEntry.PoseData.Speed = 150.0f;
    WalkEntry.PoseData.Velocity = FVector(150.0f, 0.0f, 0.0f);
    WalkEntry.Quality = 1.0f;
    WalkEntry.Tags.Add(TEXT("Walk"));
    MotionDatabase.Add(WalkEntry);

    // Run state
    FAnim_MotionDatabaseEntry RunEntry;
    RunEntry.PoseData.Speed = 400.0f;
    RunEntry.PoseData.Velocity = FVector(400.0f, 0.0f, 0.0f);
    RunEntry.Quality = 1.0f;
    RunEntry.Tags.Add(TEXT("Run"));
    MotionDatabase.Add(RunEntry);

    UE_LOG(LogTemp, Log, TEXT("Motion Matching: Database built with %d entries"), MotionDatabase.Num());
}

void UAnim_MotionMatchingSystem::ClearMotionDatabase()
{
    MotionDatabase.Empty();
    CurrentBestMatchIndex = -1;
    CurrentMatchScore = 0.0f;
}

bool UAnim_MotionMatchingSystem::IsValidMotionMatch(int32 Index) const
{
    return Index >= 0 && Index < MotionDatabase.Num();
}

FString UAnim_MotionMatchingSystem::GetCurrentMotionInfo() const
{
    if (!IsValidMotionMatch(CurrentBestMatchIndex))
    {
        return TEXT("No valid motion match");
    }

    const FAnim_MotionDatabaseEntry& Entry = MotionDatabase[CurrentBestMatchIndex];
    return FString::Printf(TEXT("Motion: %s, Score: %.2f, Speed: %.1f"), 
                          Entry.AnimationSequence ? *Entry.AnimationSequence->GetName() : TEXT("None"),
                          CurrentMatchScore,
                          CurrentPose.Speed);
}

void UAnim_MotionMatchingSystem::SetMotionMatchingEnabled(bool bEnabled)
{
    bMotionMatchingEnabled = bEnabled;
    if (!bEnabled)
    {
        CurrentBestMatchIndex = -1;
        CurrentMatchScore = 0.0f;
    }
}

void UAnim_MotionMatchingSystem::InitializeMotionDatabase()
{
    BuildMotionDatabase();
}

void UAnim_MotionMatchingSystem::UpdateCharacterReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        USkeletalMeshComponent* SkeletalMesh = OwnerCharacter->GetMesh();
        if (SkeletalMesh)
        {
            AnimInstance = SkeletalMesh->GetAnimInstance();
        }
    }
}

float UAnim_MotionMatchingSystem::CalculateVelocityScore(const FVector& TargetVelocity, const FVector& CurrentVelocity) const
{
    return FVector::Dist(TargetVelocity, CurrentVelocity);
}

float UAnim_MotionMatchingSystem::CalculateDirectionScore(float TargetDirection, float CurrentDirection) const
{
    float DifferenceDegrees = FMath::Abs(TargetDirection - CurrentDirection);
    // Handle wrap-around for angles
    if (DifferenceDegrees > 180.0f)
    {
        DifferenceDegrees = 360.0f - DifferenceDegrees;
    }
    return DifferenceDegrees;
}