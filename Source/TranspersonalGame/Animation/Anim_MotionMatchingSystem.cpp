#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimSequence.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    VelocityWeight = 1.0f;
    DirectionWeight = 0.8f;
    AccelerationWeight = 0.6f;
    MinBlendTime = 0.1f;
    MaxBlendTime = 0.5f;
    
    // Internal state
    PreviousVelocity = FVector::ZeroVector;
    LastUpdateTime = 0.0f;
    bIsInitialized = false;
    
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache owner references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        bIsInitialized = true;
        
        UE_LOG(LogTemp, Log, TEXT("Motion Matching System initialized for: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching System: Owner is not a Character!"));
    }
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized || !OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update motion data from character movement
    FVector CurrentVelocity = MovementComponent->Velocity;
    bool bIsMoving = CurrentVelocity.Size() > 10.0f;
    bool bIsFalling = MovementComponent->IsFalling();
    bool bIsJumping = MovementComponent->IsMovingOnGround() == false && CurrentVelocity.Z > 0.0f;
    
    UpdateMotionData(CurrentVelocity, bIsMoving, bIsFalling, bIsJumping);
    
    // Update best matching animation
    UpdateBestMatch();
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_MotionMatchingSystem::UpdateMotionData(const FVector& InVelocity, bool bInIsMoving, bool bInIsFalling, bool bInIsJumping)
{
    CurrentMotionData.Velocity = InVelocity;
    CurrentMotionData.Speed = InVelocity.Size();
    CurrentMotionData.bIsMoving = bInIsMoving;
    CurrentMotionData.bIsFalling = bInIsFalling;
    CurrentMotionData.bIsJumping = bInIsJumping;
    
    // Calculate direction relative to character forward
    if (OwnerCharacter && bInIsMoving)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector NormalizedVelocity = InVelocity.GetSafeNormal();
        
        float DotProduct = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float CrossProduct = FVector::CrossProduct(ForwardVector, NormalizedVelocity).Z;
        
        CurrentMotionData.Direction = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
        CurrentMotionData.Direction = NormalizeAngle(CurrentMotionData.Direction);
    }
    else
    {
        CurrentMotionData.Direction = 0.0f;
    }
    
    // Calculate acceleration
    if (LastUpdateTime > 0.0f)
    {
        float DeltaTime = GetWorld()->GetTimeSeconds() - LastUpdateTime;
        if (DeltaTime > 0.0f)
        {
            FVector AccelVector = (InVelocity - PreviousVelocity) / DeltaTime;
            CurrentMotionData.Acceleration = AccelVector.Size();
        }
    }
    
    PreviousVelocity = InVelocity;
}

FAnim_MotionClip UAnim_MotionMatchingSystem::FindBestMatchingClip(const FAnim_MotionData& TargetMotion)
{
    if (MotionDatabase.Num() == 0)
    {
        return FAnim_MotionClip();
    }
    
    float BestScore = FLT_MAX;
    int32 BestIndex = 0;
    
    for (int32 i = 0; i < MotionDatabase.Num(); i++)
    {
        float Score = CalculateMotionScore(TargetMotion, MotionDatabase[i].MotionData);
        if (Score < BestScore)
        {
            BestScore = Score;
            BestIndex = i;
        }
    }
    
    return MotionDatabase[BestIndex];
}

float UAnim_MotionMatchingSystem::CalculateMotionScore(const FAnim_MotionData& MotionA, const FAnim_MotionData& MotionB)
{
    float Score = 0.0f;
    
    // Velocity difference
    float VelocityDiff = FVector::Dist(MotionA.Velocity, MotionB.Velocity);
    Score += VelocityDiff * VelocityWeight;
    
    // Speed difference
    float SpeedDiff = FMath::Abs(MotionA.Speed - MotionB.Speed);
    Score += SpeedDiff * VelocityWeight;
    
    // Direction difference
    float DirectionDiff = FMath::Abs(MotionA.Direction - MotionB.Direction);
    if (DirectionDiff > 180.0f)
    {
        DirectionDiff = 360.0f - DirectionDiff;
    }
    Score += DirectionDiff * DirectionWeight;
    
    // Acceleration difference
    float AccelDiff = FMath::Abs(MotionA.Acceleration - MotionB.Acceleration);
    Score += AccelDiff * AccelerationWeight;
    
    // Boolean state penalties
    if (MotionA.bIsMoving != MotionB.bIsMoving) Score += 100.0f;
    if (MotionA.bIsFalling != MotionB.bIsFalling) Score += 200.0f;
    if (MotionA.bIsJumping != MotionB.bIsJumping) Score += 150.0f;
    
    return Score;
}

void UAnim_MotionMatchingSystem::AddMotionClip(UAnimSequence* AnimSeq, const FAnim_MotionData& MotionData)
{
    if (!AnimSeq)
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching: Cannot add null animation sequence"));
        return;
    }
    
    FAnim_MotionClip NewClip;
    NewClip.AnimSequence = AnimSeq;
    NewClip.MotionData = MotionData;
    NewClip.Weight = 1.0f;
    NewClip.BlendTime = FMath::Clamp(0.2f, MinBlendTime, MaxBlendTime);
    
    MotionDatabase.Add(NewClip);
    
    UE_LOG(LogTemp, Log, TEXT("Motion Matching: Added clip %s to database (Total: %d)"), 
           *AnimSeq->GetName(), MotionDatabase.Num());
}

void UAnim_MotionMatchingSystem::ClearMotionDatabase()
{
    MotionDatabase.Empty();
    BestMatchClip = FAnim_MotionClip();
    UE_LOG(LogTemp, Log, TEXT("Motion Matching: Database cleared"));
}

UAnimSequence* UAnim_MotionMatchingSystem::GetCurrentBestAnimation() const
{
    return BestMatchClip.AnimSequence;
}

void UAnim_MotionMatchingSystem::AnalyzeMotionData()
{
    // This function could be expanded to analyze motion patterns
    // and automatically categorize animations
    if (MotionDatabase.Num() == 0)
    {
        return;
    }
    
    // Log current motion state for debugging
    UE_LOG(LogTemp, VeryVerbose, TEXT("Motion Analysis - Speed: %.2f, Direction: %.2f, Moving: %s, Falling: %s"), 
           CurrentMotionData.Speed, 
           CurrentMotionData.Direction,
           CurrentMotionData.bIsMoving ? TEXT("Yes") : TEXT("No"),
           CurrentMotionData.bIsFalling ? TEXT("Yes") : TEXT("No"));
}

void UAnim_MotionMatchingSystem::UpdateBestMatch()
{
    if (MotionDatabase.Num() == 0)
    {
        return;
    }
    
    // Find the best matching clip for current motion
    FAnim_MotionClip NewBestMatch = FindBestMatchingClip(CurrentMotionData);
    
    // Update if we found a better match
    if (NewBestMatch.AnimSequence != BestMatchClip.AnimSequence)
    {
        BestMatchClip = NewBestMatch;
        
        // Calculate blend time based on motion difference
        if (BestMatchClip.AnimSequence)
        {
            float MotionDifference = CalculateMotionScore(CurrentMotionData, BestMatchClip.MotionData);
            float NormalizedDiff = FMath::Clamp(MotionDifference / 100.0f, 0.0f, 1.0f);
            BestMatchClip.BlendTime = FMath::Lerp(MinBlendTime, MaxBlendTime, NormalizedDiff);
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("Motion Matching: New best match - %s (Blend: %.2f)"), 
                   *BestMatchClip.AnimSequence->GetName(), BestMatchClip.BlendTime);
        }
    }
}

float UAnim_MotionMatchingSystem::NormalizeAngle(float Angle)
{
    while (Angle > 180.0f)
    {
        Angle -= 360.0f;
    }
    while (Angle < -180.0f)
    {
        Angle += 360.0f;
    }
    return Angle;
}