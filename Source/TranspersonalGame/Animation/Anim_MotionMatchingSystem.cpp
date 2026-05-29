#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    SearchRadius = 100.0f;
    BlendTime = 0.2f;
    bEnableMotionMatching = true;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultMotions();
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableMotionMatching)
    {
        return;
    }

    // Get owner character
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        return;
    }

    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }

    // Update current motion data
    FVector CurrentVelocity = MovementComp->Velocity;
    FVector CurrentAcceleration = MovementComp->GetCurrentAcceleration();
    
    UpdateMotionData(CurrentVelocity, CurrentAcceleration);

    // Find best matching animation
    if (MotionDatabase.Num() > 0)
    {
        CurrentBestMatch = FindBestMatch(CurrentMotionData);
    }
}

void UAnim_MotionMatchingSystem::UpdateMotionData(const FVector& InVelocity, const FVector& InAcceleration)
{
    CurrentMotionData.Velocity = InVelocity;
    CurrentMotionData.Acceleration = InAcceleration;
    CurrentMotionData.Speed = InVelocity.Size2D();
    CurrentMotionData.bIsMoving = CurrentMotionData.Speed > 10.0f;

    // Calculate direction relative to forward vector
    if (CurrentMotionData.bIsMoving)
    {
        ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
        if (OwnerCharacter)
        {
            FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
            FVector VelocityNormalized = InVelocity.GetSafeNormal2D();
            CurrentMotionData.Direction = FMath::RadiansToDegrees(
                FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized))
            );
        }
    }
    else
    {
        CurrentMotionData.Direction = 0.0f;
    }
}

FAnim_MotionClip UAnim_MotionMatchingSystem::FindBestMatch(const FAnim_MotionData& TargetMotion)
{
    if (MotionDatabase.Num() == 0)
    {
        return FAnim_MotionClip();
    }

    FAnim_MotionClip BestMatch = MotionDatabase[0];
    float BestScore = CalculateMotionScore(TargetMotion, BestMatch.MotionData);

    for (int32 i = 1; i < MotionDatabase.Num(); i++)
    {
        float Score = CalculateMotionScore(TargetMotion, MotionDatabase[i].MotionData);
        if (Score < BestScore) // Lower score is better
        {
            BestScore = Score;
            BestMatch = MotionDatabase[i];
        }
    }

    BestMatch.QualityScore = BestScore;
    return BestMatch;
}

void UAnim_MotionMatchingSystem::AddMotionClip(UAnimSequence* AnimSequence, const FAnim_MotionData& MotionData, float StartTime, float EndTime)
{
    if (!AnimSequence)
    {
        return;
    }

    FAnim_MotionClip NewClip;
    NewClip.AnimSequence = AnimSequence;
    NewClip.MotionData = MotionData;
    NewClip.StartTime = FMath::Clamp(StartTime, 0.0f, 1.0f);
    NewClip.EndTime = FMath::Clamp(EndTime, StartTime, 1.0f);
    NewClip.QualityScore = 1.0f;

    MotionDatabase.Add(NewClip);
}

void UAnim_MotionMatchingSystem::ClearMotionDatabase()
{
    MotionDatabase.Empty();
}

float UAnim_MotionMatchingSystem::CalculateMotionScore(const FAnim_MotionData& A, const FAnim_MotionData& B) const
{
    // Weighted scoring system
    float VelocityScore = CalculateVelocityScore(A.Velocity, B.Velocity) * 0.6f;
    float SpeedScore = FMath::Abs(A.Speed - B.Speed) / 600.0f * 0.3f; // Normalize by max expected speed
    float DirectionScore = CalculateDirectionScore(A.Direction, B.Direction) * 0.1f;

    return VelocityScore + SpeedScore + DirectionScore;
}

float UAnim_MotionMatchingSystem::CalculateVelocityScore(const FVector& A, const FVector& B) const
{
    float Distance = FVector::Dist(A, B);
    return Distance / 600.0f; // Normalize by max expected velocity
}

float UAnim_MotionMatchingSystem::CalculateDirectionScore(float A, float B) const
{
    float AngleDiff = FMath::Abs(A - B);
    AngleDiff = FMath::Min(AngleDiff, 360.0f - AngleDiff); // Handle wrap-around
    return AngleDiff / 180.0f; // Normalize to 0-1 range
}

void UAnim_MotionMatchingSystem::InitializeDefaultMotions()
{
    // Initialize with basic motion patterns
    // These would normally be loaded from animation assets
    
    // Idle motion
    FAnim_MotionData IdleMotion;
    IdleMotion.Velocity = FVector::ZeroVector;
    IdleMotion.Speed = 0.0f;
    IdleMotion.bIsMoving = false;
    
    // Walking forward motion
    FAnim_MotionData WalkMotion;
    WalkMotion.Velocity = FVector(150.0f, 0.0f, 0.0f);
    WalkMotion.Speed = 150.0f;
    WalkMotion.bIsMoving = true;
    WalkMotion.Direction = 0.0f;
    
    // Running forward motion
    FAnim_MotionData RunMotion;
    RunMotion.Velocity = FVector(400.0f, 0.0f, 0.0f);
    RunMotion.Speed = 400.0f;
    RunMotion.bIsMoving = true;
    RunMotion.Direction = 0.0f;

    // Note: Actual animation sequences would be assigned in Blueprint or through asset loading
    // For now, we create the motion data structure without the animation references
}