#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default settings
    VelocityWeight = 1.0f;
    DirectionWeight = 0.8f;
    SpeedWeight = 1.2f;
    BlendTime = 0.3f;
    
    CurrentBlendTime = 0.0f;
    bIsBlending = false;
    
    OwnerCharacter = nullptr;
    AnimInstance = nullptr;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
        if (MeshComp)
        {
            AnimInstance = MeshComp->GetAnimInstance();
        }
    }
    
    // Initialize default database if empty
    if (MotionDatabase.Num() == 0)
    {
        InitializeDefaultDatabase();
    }
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !AnimInstance)
        return;
    
    // Update motion data
    UpdateMotionData();
    
    // Find best matching clip
    FAnim_MotionClip NewBestMatch = FindBestMatch();
    
    // Check if we need to blend to a new clip
    if (NewBestMatch.AnimSequence != BestMatchClip.AnimSequence)
    {
        BestMatchClip = NewBestMatch;
        BlendToClip(BestMatchClip);
    }
    
    // Update blend state
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

void UAnim_MotionMatchingComponent::UpdateMotionData()
{
    if (!OwnerCharacter)
        return;
    
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
        return;
    
    // Update velocity and speed
    CurrentMotionData.Velocity = MovementComp->Velocity;
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    
    // Update direction relative to character forward
    if (CurrentMotionData.Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNorm = CurrentMotionData.Velocity.GetSafeNormal();
        CurrentMotionData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNorm)));
        
        // Check if moving right or left
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        if (FVector::DotProduct(RightVector, VelocityNorm) < 0.0f)
        {
            CurrentMotionData.Direction = -CurrentMotionData.Direction;
        }
    }
    else
    {
        CurrentMotionData.Direction = 0.0f;
    }
    
    // Update movement states
    CurrentMotionData.bIsMoving = CurrentMotionData.Speed > 5.0f;
    CurrentMotionData.bIsFalling = MovementComp->IsFalling();
    CurrentMotionData.bIsCrouching = OwnerCharacter->bIsCrouched;
}

FAnim_MotionClip UAnim_MotionMatchingComponent::FindBestMatch()
{
    if (MotionDatabase.Num() == 0)
    {
        return FAnim_MotionClip();
    }
    
    FAnim_MotionClip BestClip;
    float BestScore = -1.0f;
    
    for (const FAnim_MotionClip& Clip : MotionDatabase)
    {
        float Score = CalculateMotionScore(Clip);
        if (Score > BestScore)
        {
            BestScore = Score;
            BestClip = Clip;
        }
    }
    
    return BestClip;
}

float UAnim_MotionMatchingComponent::CalculateMotionScore(const FAnim_MotionClip& Clip)
{
    float Score = 0.0f;
    
    // Velocity matching
    float VelocityDiff = FVector::Dist(CurrentMotionData.Velocity, Clip.MotionData.Velocity);
    float VelocityScore = FMath::Exp(-VelocityDiff * 0.01f) * VelocityWeight;
    Score += VelocityScore;
    
    // Speed matching
    float SpeedDiff = FMath::Abs(CurrentMotionData.Speed - Clip.MotionData.Speed);
    float SpeedScore = FMath::Exp(-SpeedDiff * 0.1f) * SpeedWeight;
    Score += SpeedScore;
    
    // Direction matching
    float DirectionDiff = FMath::Abs(CurrentMotionData.Direction - Clip.MotionData.Direction);
    float DirectionScore = FMath::Exp(-DirectionDiff * 0.05f) * DirectionWeight;
    Score += DirectionScore;
    
    // State matching bonuses
    if (CurrentMotionData.bIsMoving == Clip.MotionData.bIsMoving)
        Score += 0.5f;
    if (CurrentMotionData.bIsFalling == Clip.MotionData.bIsFalling)
        Score += 0.8f;
    if (CurrentMotionData.bIsCrouching == Clip.MotionData.bIsCrouching)
        Score += 0.3f;
    
    return Score;
}

void UAnim_MotionMatchingComponent::BlendToClip(const FAnim_MotionClip& TargetClip)
{
    if (!AnimInstance || !TargetClip.AnimSequence)
        return;
    
    // Store previous clip for blending
    PreviousClip = BestMatchClip;
    
    // Start blend
    bIsBlending = true;
    CurrentBlendTime = 0.0f;
    
    // This would typically interface with the animation blueprint
    // For now, we'll use a simple approach
    if (TargetClip.AnimSequence)
    {
        // Play the animation with the calculated play rate
        float PlayRate = FMath::Clamp(TargetClip.PlayRate, 0.1f, 3.0f);
        
        // In a real implementation, this would blend through the AnimBP
        UE_LOG(LogTemp, Log, TEXT("Motion Matching: Blending to %s at rate %.2f"), 
               *TargetClip.AnimSequence->GetName(), PlayRate);
    }
}

void UAnim_MotionMatchingComponent::AddMotionClip(UAnimSequence* Animation, const FAnim_MotionData& MotionData)
{
    if (!Animation)
        return;
    
    FAnim_MotionClip NewClip;
    NewClip.AnimSequence = Animation;
    NewClip.MotionData = MotionData;
    NewClip.BlendWeight = 1.0f;
    NewClip.PlayRate = 1.0f;
    
    MotionDatabase.Add(NewClip);
}

void UAnim_MotionMatchingComponent::ClearDatabase()
{
    MotionDatabase.Empty();
}

void UAnim_MotionMatchingComponent::InitializeDefaultDatabase()
{
    // Create default motion clips for basic locomotion
    // These would normally be loaded from actual animation assets
    
    // Idle clip
    FAnim_MotionData IdleData;
    IdleData.Speed = 0.0f;
    IdleData.bIsMoving = false;
    IdleData.bIsFalling = false;
    IdleData.bIsCrouching = false;
    
    FAnim_MotionClip IdleClip;
    IdleClip.MotionData = IdleData;
    IdleClip.BlendWeight = 1.0f;
    IdleClip.PlayRate = 1.0f;
    MotionDatabase.Add(IdleClip);
    
    // Walk forward clip
    FAnim_MotionData WalkData;
    WalkData.Speed = 150.0f;
    WalkData.Direction = 0.0f;
    WalkData.bIsMoving = true;
    WalkData.bIsFalling = false;
    WalkData.bIsCrouching = false;
    
    FAnim_MotionClip WalkClip;
    WalkClip.MotionData = WalkData;
    WalkClip.BlendWeight = 1.0f;
    WalkClip.PlayRate = 1.0f;
    MotionDatabase.Add(WalkClip);
    
    // Run forward clip
    FAnim_MotionData RunData;
    RunData.Speed = 400.0f;
    RunData.Direction = 0.0f;
    RunData.bIsMoving = true;
    RunData.bIsFalling = false;
    RunData.bIsCrouching = false;
    
    FAnim_MotionClip RunClip;
    RunClip.MotionData = RunData;
    RunClip.BlendWeight = 1.0f;
    RunClip.PlayRate = 1.0f;
    MotionDatabase.Add(RunClip);
    
    // Jump/Fall clip
    FAnim_MotionData JumpData;
    JumpData.Speed = 200.0f;
    JumpData.bIsMoving = true;
    JumpData.bIsFalling = true;
    JumpData.bIsCrouching = false;
    
    FAnim_MotionClip JumpClip;
    JumpClip.MotionData = JumpData;
    JumpClip.BlendWeight = 1.0f;
    JumpClip.PlayRate = 1.0f;
    MotionDatabase.Add(JumpClip);
    
    UE_LOG(LogTemp, Log, TEXT("Motion Matching: Initialized default database with %d clips"), MotionDatabase.Num());
}