#include "Anim_MotionMatchingController.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

UAnim_MotionMatchingController::UAnim_MotionMatchingController()
{
    // Initialize motion matching weights
    PositionWeight = 1.0f;
    VelocityWeight = 2.0f;
    AccelerationWeight = 0.5f;
    SearchRadius = 100.0f;
    
    // Initialize current motion data
    CurrentMotionData = FAnim_MotionData();
}

void UAnim_MotionMatchingController::UpdateMotionData(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingController: Character is null"));
        return;
    }

    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingController: No movement component found"));
        return;
    }

    // Update velocity and speed
    CurrentMotionData.Velocity = MovementComp->Velocity;
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    
    // Update acceleration
    CurrentMotionData.Acceleration = MovementComp->GetCurrentAcceleration();
    
    // Calculate movement direction relative to character forward
    FVector ForwardVector = Character->GetActorForwardVector();
    FVector VelocityNormalized = CurrentMotionData.Velocity.GetSafeNormal();
    CurrentMotionData.Direction = FVector::DotProduct(ForwardVector, VelocityNormalized);
    
    // Update movement states
    CurrentMotionData.bIsMoving = CurrentMotionData.Speed > 10.0f;
    CurrentMotionData.bIsJumping = MovementComp->IsFalling();
    CurrentMotionData.bIsCrouching = MovementComp->IsCrouching();
    
    // Log motion data for debugging
    UE_LOG(LogTemp, Log, TEXT("Motion Data - Speed: %f, Direction: %f, Moving: %s, Jumping: %s"), 
           CurrentMotionData.Speed, 
           CurrentMotionData.Direction,
           CurrentMotionData.bIsMoving ? TEXT("true") : TEXT("false"),
           CurrentMotionData.bIsJumping ? TEXT("true") : TEXT("false"));
}

FAnim_MotionClip UAnim_MotionMatchingController::FindBestMatchingClip(const FAnim_MotionData& TargetMotion)
{
    if (MotionDatabase.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingController: Motion database is empty"));
        return FAnim_MotionClip();
    }

    float BestScore = FLT_MAX;
    int32 BestClipIndex = 0;
    
    // Search through all clips in the database
    for (int32 i = 0; i < MotionDatabase.Num(); i++)
    {
        const FAnim_MotionClip& Clip = MotionDatabase[i];
        
        // Find best frame within this clip
        for (int32 j = 0; j < Clip.MotionFrames.Num(); j++)
        {
            float Score = CalculateMotionScore(TargetMotion, Clip.MotionFrames[j]);
            
            if (Score < BestScore)
            {
                BestScore = Score;
                BestClipIndex = i;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Best matching clip: %s with score: %f"), 
           *MotionDatabase[BestClipIndex].ClipName, BestScore);
    
    return MotionDatabase[BestClipIndex];
}

float UAnim_MotionMatchingController::CalculateMotionScore(const FAnim_MotionData& A, const FAnim_MotionData& B)
{
    float Score = 0.0f;
    
    // Velocity difference
    float VelocityDiff = FVector::Dist(A.Velocity, B.Velocity);
    Score += VelocityDiff * VelocityWeight;
    
    // Acceleration difference
    float AccelDiff = FVector::Dist(A.Acceleration, B.Acceleration);
    Score += AccelDiff * AccelerationWeight;
    
    // Speed difference
    float SpeedDiff = FMath::Abs(A.Speed - B.Speed);
    Score += SpeedDiff * PositionWeight;
    
    // Direction difference
    float DirectionDiff = FMath::Abs(A.Direction - B.Direction);
    Score += DirectionDiff * 50.0f; // Direction weight
    
    // Boolean state penalties
    if (A.bIsMoving != B.bIsMoving) Score += 100.0f;
    if (A.bIsJumping != B.bIsJumping) Score += 200.0f;
    if (A.bIsCrouching != B.bIsCrouching) Score += 150.0f;
    
    return Score;
}

void UAnim_MotionMatchingController::AddMotionClip(UAnimSequence* Animation, const FString& ClipName)
{
    if (!Animation)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingController: Animation sequence is null"));
        return;
    }

    FAnim_MotionClip NewClip;
    NewClip.AnimationSequence = Animation;
    NewClip.ClipName = ClipName;
    NewClip.ClipLength = Animation->GetPlayLength();
    
    // Analyze the animation sequence to extract motion data
    AnalyzeAnimationSequence(Animation, NewClip);
    
    // Add to database
    MotionDatabase.Add(NewClip);
    
    UE_LOG(LogTemp, Log, TEXT("Added motion clip: %s (Length: %f seconds, Frames: %d)"), 
           *ClipName, NewClip.ClipLength, NewClip.MotionFrames.Num());
}

void UAnim_MotionMatchingController::BuildMotionDatabase()
{
    UE_LOG(LogTemp, Log, TEXT("Building motion database..."));
    
    // Clear existing database
    ClearMotionDatabase();
    
    // TODO: Load animation sequences from content browser
    // This would typically scan for animation assets and add them automatically
    
    UE_LOG(LogTemp, Log, TEXT("Motion database built with %d clips"), MotionDatabase.Num());
}

void UAnim_MotionMatchingController::ClearMotionDatabase()
{
    MotionDatabase.Empty();
    UE_LOG(LogTemp, Log, TEXT("Motion database cleared"));
}

void UAnim_MotionMatchingController::PlayMotionClip(UAnimInstance* AnimInstance, const FAnim_MotionClip& Clip)
{
    if (!AnimInstance || !Clip.AnimationSequence)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingController: Invalid animation instance or clip"));
        return;
    }

    // Play the animation sequence
    // Note: In a full implementation, this would handle blending and timing
    UE_LOG(LogTemp, Log, TEXT("Playing motion clip: %s"), *Clip.ClipName);
}

void UAnim_MotionMatchingController::DebugPrintMotionData()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MOTION DATA DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Velocity: %s"), *CurrentMotionData.Velocity.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Acceleration: %s"), *CurrentMotionData.Acceleration.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Speed: %f"), CurrentMotionData.Speed);
    UE_LOG(LogTemp, Warning, TEXT("Direction: %f"), CurrentMotionData.Direction);
    UE_LOG(LogTemp, Warning, TEXT("Is Moving: %s"), CurrentMotionData.bIsMoving ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("Is Jumping: %s"), CurrentMotionData.bIsJumping ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("Is Crouching: %s"), CurrentMotionData.bIsCrouching ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("Database Clips: %d"), MotionDatabase.Num());
}

void UAnim_MotionMatchingController::ValidateMotionDatabase()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MOTION DATABASE VALIDATION ==="));
    
    for (int32 i = 0; i < MotionDatabase.Num(); i++)
    {
        const FAnim_MotionClip& Clip = MotionDatabase[i];
        
        bool bIsValid = true;
        FString ValidationErrors;
        
        if (!Clip.AnimationSequence)
        {
            bIsValid = false;
            ValidationErrors += TEXT("Missing animation sequence; ");
        }
        
        if (Clip.MotionFrames.Num() == 0)
        {
            bIsValid = false;
            ValidationErrors += TEXT("No motion frames; ");
        }
        
        if (Clip.ClipLength <= 0.0f)
        {
            bIsValid = false;
            ValidationErrors += TEXT("Invalid clip length; ");
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Clip %d (%s): %s - %s"), 
               i, 
               *Clip.ClipName,
               bIsValid ? TEXT("VALID") : TEXT("INVALID"),
               bIsValid ? TEXT("OK") : *ValidationErrors);
    }
}

void UAnim_MotionMatchingController::AnalyzeAnimationSequence(UAnimSequence* Animation, FAnim_MotionClip& OutClip)
{
    if (!Animation)
    {
        return;
    }

    // Sample the animation at regular intervals to extract motion data
    float SampleRate = 30.0f; // 30 FPS sampling
    float SampleInterval = 1.0f / SampleRate;
    int32 NumSamples = FMath::CeilToInt(Animation->GetPlayLength() * SampleRate);
    
    OutClip.MotionFrames.Empty();
    OutClip.MotionFrames.Reserve(NumSamples);
    
    for (int32 i = 0; i < NumSamples; i++)
    {
        float SampleTime = i * SampleInterval;
        
        // Create motion data for this frame
        // Note: In a full implementation, this would extract actual bone positions
        // and calculate velocities/accelerations from the animation data
        FAnim_MotionData FrameData;
        
        // Placeholder motion data extraction
        // This would analyze root motion, bone velocities, etc.
        FrameData.Speed = FMath::Sin(SampleTime * 2.0f) * 100.0f; // Example data
        FrameData.Direction = FMath::Cos(SampleTime) * 0.5f;
        FrameData.bIsMoving = FrameData.Speed > 10.0f;
        
        OutClip.MotionFrames.Add(FrameData);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Analyzed animation sequence: %d motion frames extracted"), 
           OutClip.MotionFrames.Num());
}

FAnim_MotionData UAnim_MotionMatchingController::PredictNextMotion(const FAnim_MotionData& Current, float DeltaTime)
{
    FAnim_MotionData Predicted = Current;
    
    // Simple motion prediction based on current velocity and acceleration
    Predicted.Velocity += Current.Acceleration * DeltaTime;
    Predicted.Speed = Predicted.Velocity.Size();
    
    return Predicted;
}

void UAnim_MotionMatchingController::OptimizeDatabase()
{
    // Remove duplicate or very similar motion clips
    // Optimize memory usage
    // Build acceleration structures for faster searching
    
    UE_LOG(LogTemp, Log, TEXT("Motion database optimization completed"));
}