#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    MatchThreshold = 0.8f;
    BlendTime = 0.3f;
    CurrentBlendTime = 0.0f;
    bIsBlending = false;
    TargetAnimInstance = nullptr;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the character's anim instance
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (USkeletalMeshComponent* Mesh = Character->GetMesh())
        {
            TargetAnimInstance = Mesh->GetAnimInstance();
        }
    }
    
    InitializeMotionDatabase();
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!TargetAnimInstance)
        return;
    
    // Update current motion data from character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            FVector Velocity = Movement->Velocity;
            bool bIsMoving = Velocity.Size() > 10.0f;
            bool bIsCrouching = Movement->IsCrouching();
            bool bIsJumping = Movement->IsFalling();
            
            UpdateMotionData(Velocity, bIsMoving, bIsCrouching, bIsJumping);
        }
    }
    
    // Find best matching animation
    FAnim_MotionClip BestMatch = FindBestMatch(CurrentMotionData);
    
    // Blend to new animation if different enough
    if (BestMatch.AnimSequence && BestMatch.AnimSequence != BestMatchClip.AnimSequence)
    {
        BlendToClip(BestMatch);
        BestMatchClip = BestMatch;
    }
    
    // Handle blending
    if (bIsBlending)
    {
        CurrentBlendTime += DeltaTime;
        if (CurrentBlendTime >= BlendTime)
        {
            bIsBlending = false;
            CurrentBlendTime = 0.0f;
        }
    }
}

void UAnim_MotionMatchingSystem::UpdateMotionData(const FVector& InVelocity, bool bInIsMoving, bool bInIsCrouching, bool bInIsJumping)
{
    CurrentMotionData.Velocity = InVelocity;
    CurrentMotionData.Speed = InVelocity.Size();
    CurrentMotionData.bIsMoving = bInIsMoving;
    CurrentMotionData.bIsCrouching = bInIsCrouching;
    CurrentMotionData.bIsJumping = bInIsJumping;
    
    // Calculate direction relative to character forward
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        FVector Forward = Character->GetActorForwardVector();
        FVector Right = Character->GetActorRightVector();
        
        FVector NormalizedVelocity = InVelocity.GetSafeNormal();
        float ForwardDot = FVector::DotProduct(NormalizedVelocity, Forward);
        float RightDot = FVector::DotProduct(NormalizedVelocity, Right);
        
        CurrentMotionData.Direction = UKismetMathLibrary::Atan2(RightDot, ForwardDot) * 180.0f / PI;
    }
}

FAnim_MotionClip UAnim_MotionMatchingSystem::FindBestMatch(const FAnim_MotionData& TargetMotion)
{
    FAnim_MotionClip BestMatch;
    float BestScore = -1.0f;
    
    for (const FAnim_MotionClip& Clip : MotionDatabase)
    {
        float Score = CalculateMotionScore(TargetMotion, Clip.MotionData);
        if (Score > BestScore && Score >= MatchThreshold)
        {
            BestScore = Score;
            BestMatch = Clip;
        }
    }
    
    return BestMatch;
}

float UAnim_MotionMatchingSystem::CalculateMotionScore(const FAnim_MotionData& A, const FAnim_MotionData& B)
{
    float Score = 1.0f;
    
    // Speed matching (weighted heavily)
    float SpeedDiff = FMath::Abs(A.Speed - B.Speed) / FMath::Max(A.Speed, B.Speed);
    Score *= (1.0f - FMath::Clamp(SpeedDiff, 0.0f, 1.0f)) * 0.4f;
    
    // Direction matching
    float DirectionDiff = FMath::Abs(A.Direction - B.Direction) / 180.0f;
    Score *= (1.0f - FMath::Clamp(DirectionDiff, 0.0f, 1.0f)) * 0.3f;
    
    // State matching (boolean states must match exactly)
    if (A.bIsMoving != B.bIsMoving) Score *= 0.1f;
    if (A.bIsCrouching != B.bIsCrouching) Score *= 0.1f;
    if (A.bIsJumping != B.bIsJumping) Score *= 0.1f;
    
    return Score;
}

void UAnim_MotionMatchingSystem::BlendToClip(const FAnim_MotionClip& TargetClip)
{
    if (!TargetAnimInstance || !TargetClip.AnimSequence)
        return;
    
    PreviousClip = BestMatchClip;
    bIsBlending = true;
    CurrentBlendTime = 0.0f;
    
    // This would typically interface with the AnimBP
    // For now, we log the transition
    UE_LOG(LogTemp, Log, TEXT("Motion Matching: Blending to new animation clip"));
}

void UAnim_MotionMatchingSystem::InitializeMotionDatabase()
{
    // Initialize with basic motion clips
    // In a real implementation, these would be loaded from assets
    
    MotionDatabase.Empty();
    
    // Idle clip
    FAnim_MotionClip IdleClip;
    IdleClip.MotionData.Speed = 0.0f;
    IdleClip.MotionData.bIsMoving = false;
    IdleClip.MotionData.bIsCrouching = false;
    IdleClip.MotionData.bIsJumping = false;
    MotionDatabase.Add(IdleClip);
    
    // Walk forward clip
    FAnim_MotionClip WalkClip;
    WalkClip.MotionData.Speed = 150.0f;
    WalkClip.MotionData.Direction = 0.0f;
    WalkClip.MotionData.bIsMoving = true;
    WalkClip.MotionData.bIsCrouching = false;
    WalkClip.MotionData.bIsJumping = false;
    MotionDatabase.Add(WalkClip);
    
    // Run forward clip
    FAnim_MotionClip RunClip;
    RunClip.MotionData.Speed = 400.0f;
    RunClip.MotionData.Direction = 0.0f;
    RunClip.MotionData.bIsMoving = true;
    RunClip.MotionData.bIsCrouching = false;
    RunClip.MotionData.bIsJumping = false;
    MotionDatabase.Add(RunClip);
    
    // Crouch idle clip
    FAnim_MotionClip CrouchIdleClip;
    CrouchIdleClip.MotionData.Speed = 0.0f;
    CrouchIdleClip.MotionData.bIsMoving = false;
    CrouchIdleClip.MotionData.bIsCrouching = true;
    CrouchIdleClip.MotionData.bIsJumping = false;
    MotionDatabase.Add(CrouchIdleClip);
    
    // Jump clip
    FAnim_MotionClip JumpClip;
    JumpClip.MotionData.bIsJumping = true;
    JumpClip.MotionData.bIsCrouching = false;
    MotionDatabase.Add(JumpClip);
    
    UE_LOG(LogTemp, Log, TEXT("Motion Matching Database initialized with %d clips"), MotionDatabase.Num());
}

void UAnim_MotionMatchingSystem::AddMotionClip(UAnimSequence* AnimSeq, const FAnim_MotionData& MotionData)
{
    if (!AnimSeq)
        return;
    
    FAnim_MotionClip NewClip;
    NewClip.AnimSequence = AnimSeq;
    NewClip.MotionData = MotionData;
    NewClip.BlendWeight = 1.0f;
    NewClip.PlayRate = 1.0f;
    
    MotionDatabase.Add(NewClip);
    
    UE_LOG(LogTemp, Log, TEXT("Added motion clip: %s"), *AnimSeq->GetName());
}