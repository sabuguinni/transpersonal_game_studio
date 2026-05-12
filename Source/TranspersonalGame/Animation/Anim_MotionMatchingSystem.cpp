#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS

    // Initialize motion matching weights
    VelocityWeight = 1.0f;
    AccelerationWeight = 0.5f;
    DirectionWeight = 0.8f;
    SpeedWeight = 1.2f;
    MatchingThreshold = 0.3f;
    BlendTime = 0.2f;

    // Performance settings
    MaxDatabaseSize = 100;
    UpdateFrequency = 60.0f;

    // Internal state
    LastUpdateTime = 0.0f;
    CurrentClipIndex = -1;
    bIsInitialized = false;

    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MeshComponent = nullptr;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();

    CacheCharacterReferences();
    InitializeMotionDatabase();

    UE_LOG(LogTemp, Log, TEXT("Motion Matching System initialized for %s"), 
           OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsInitialized || !OwnerCharacter || !MovementComponent)
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= (1.0f / UpdateFrequency))
    {
        UpdateCurrentMotionFrame();
        
        // Find best matching clip
        FAnim_MotionClip BestClip = FindBestMatchingClip(CurrentMotionFrame);
        
        // Play matched animation if different from current
        if (BestClip.AnimationSequence && BestClip.ClipName != CurrentPlayingClip.ClipName)
        {
            PlayMatchedAnimation(BestClip);
            CurrentPlayingClip = BestClip;
        }

        LastUpdateTime = CurrentTime;
    }
}

void UAnim_MotionMatchingSystem::InitializeMotionDatabase()
{
    MotionDatabase.Empty();
    
    // Create default motion clips for basic states
    // Note: In production, these would be loaded from actual animation assets
    
    // Idle clip
    FAnim_MotionClip IdleClip;
    IdleClip.ClipName = TEXT("Idle");
    IdleClip.MovementState = ECharacterMovementState::Idle;
    IdleClip.ClipDuration = 2.0f;
    
    // Create sample motion frames for idle
    for (int32 i = 0; i < 10; ++i)
    {
        FAnim_MotionFrame Frame;
        Frame.Velocity = FVector::ZeroVector;
        Frame.Speed = 0.0f;
        Frame.bIsMoving = false;
        Frame.TimeStamp = i * 0.2f;
        IdleClip.MotionFrames.Add(Frame);
    }
    MotionDatabase.Add(IdleClip);

    // Walking clip
    FAnim_MotionClip WalkClip;
    WalkClip.ClipName = TEXT("Walk");
    WalkClip.MovementState = ECharacterMovementState::Walking;
    WalkClip.ClipDuration = 1.5f;
    
    for (int32 i = 0; i < 8; ++i)
    {
        FAnim_MotionFrame Frame;
        Frame.Velocity = FVector(200.0f, 0.0f, 0.0f);
        Frame.Speed = 200.0f;
        Frame.bIsMoving = true;
        Frame.Direction = 0.0f;
        Frame.TimeStamp = i * 0.1875f;
        WalkClip.MotionFrames.Add(Frame);
    }
    MotionDatabase.Add(WalkClip);

    // Running clip
    FAnim_MotionClip RunClip;
    RunClip.ClipName = TEXT("Run");
    RunClip.MovementState = ECharacterMovementState::Running;
    RunClip.ClipDuration = 1.0f;
    
    for (int32 i = 0; i < 6; ++i)
    {
        FAnim_MotionFrame Frame;
        Frame.Velocity = FVector(600.0f, 0.0f, 0.0f);
        Frame.Speed = 600.0f;
        Frame.bIsMoving = true;
        Frame.Direction = 0.0f;
        Frame.TimeStamp = i * 0.167f;
        RunClip.MotionFrames.Add(Frame);
    }
    MotionDatabase.Add(RunClip);

    // Jumping clip
    FAnim_MotionClip JumpClip;
    JumpClip.ClipName = TEXT("Jump");
    JumpClip.MovementState = ECharacterMovementState::Jumping;
    JumpClip.ClipDuration = 0.8f;
    
    for (int32 i = 0; i < 4; ++i)
    {
        FAnim_MotionFrame Frame;
        Frame.Velocity = FVector(300.0f, 0.0f, 500.0f);
        Frame.Speed = 300.0f;
        Frame.bIsJumping = true;
        Frame.bIsFalling = i > 1;
        Frame.TimeStamp = i * 0.2f;
        JumpClip.MotionFrames.Add(Frame);
    }
    MotionDatabase.Add(JumpClip);

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Motion database initialized with %d clips"), MotionDatabase.Num());
}

FAnim_MotionClip UAnim_MotionMatchingSystem::FindBestMatchingClip(const FAnim_MotionFrame& CurrentFrame)
{
    if (MotionDatabase.Num() == 0)
    {
        return FAnim_MotionClip();
    }

    float BestScore = FLT_MAX;
    int32 BestClipIndex = 0;

    for (int32 ClipIndex = 0; ClipIndex < MotionDatabase.Num(); ++ClipIndex)
    {
        const FAnim_MotionClip& Clip = MotionDatabase[ClipIndex];
        
        if (Clip.MotionFrames.Num() == 0)
        {
            continue;
        }

        // Find the best matching frame within this clip
        float ClipBestScore = FLT_MAX;
        for (const FAnim_MotionFrame& Frame : Clip.MotionFrames)
        {
            float Distance = CalculateMotionDistance(CurrentFrame, Frame);
            if (Distance < ClipBestScore)
            {
                ClipBestScore = Distance;
            }
        }

        // Check if this clip is better than our current best
        if (ClipBestScore < BestScore)
        {
            BestScore = ClipBestScore;
            BestClipIndex = ClipIndex;
        }
    }

    // Return best matching clip if score is below threshold
    if (BestScore < MatchingThreshold)
    {
        return MotionDatabase[BestClipIndex];
    }

    // Return current playing clip if no good match found
    return CurrentPlayingClip;
}

void UAnim_MotionMatchingSystem::UpdateCurrentMotionFrame()
{
    if (!MovementComponent || !OwnerCharacter)
    {
        return;
    }

    // Update motion frame data
    CurrentMotionFrame.Velocity = MovementComponent->Velocity;
    CurrentMotionFrame.Acceleration = MovementComponent->GetCurrentAcceleration();
    CurrentMotionFrame.Speed = CurrentMotionFrame.Velocity.Size();
    CurrentMotionFrame.bIsMoving = CurrentMotionFrame.Speed > 10.0f;
    CurrentMotionFrame.bIsFalling = MovementComponent->IsFalling();
    CurrentMotionFrame.bIsJumping = MovementComponent->IsMovingOnGround() == false && CurrentMotionFrame.Velocity.Z > 0.0f;
    CurrentMotionFrame.TimeStamp = GetWorld()->GetTimeSeconds();

    // Calculate movement direction relative to character forward
    if (CurrentMotionFrame.bIsMoving)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = CurrentMotionFrame.Velocity.GetSafeNormal();
        CurrentMotionFrame.Direction = FMath::Atan2(
            FVector::DotProduct(OwnerCharacter->GetActorRightVector(), VelocityNormalized),
            FVector::DotProduct(ForwardVector, VelocityNormalized)
        );
    }
    else
    {
        CurrentMotionFrame.Direction = 0.0f;
    }
}

float UAnim_MotionMatchingSystem::CalculateMotionDistance(const FAnim_MotionFrame& FrameA, const FAnim_MotionFrame& FrameB)
{
    float VelocityDistance = FVector::Dist(FrameA.Velocity, FrameB.Velocity) * VelocityWeight;
    float AccelerationDistance = FVector::Dist(FrameA.Acceleration, FrameB.Acceleration) * AccelerationWeight;
    float SpeedDistance = FMath::Abs(FrameA.Speed - FrameB.Speed) * SpeedWeight;
    float DirectionDistance = FMath::Abs(FrameA.Direction - FrameB.Direction) * DirectionWeight;

    // Boolean state penalties
    float StatePenalty = 0.0f;
    if (FrameA.bIsMoving != FrameB.bIsMoving) StatePenalty += 100.0f;
    if (FrameA.bIsFalling != FrameB.bIsFalling) StatePenalty += 200.0f;
    if (FrameA.bIsJumping != FrameB.bIsJumping) StatePenalty += 150.0f;

    return VelocityDistance + AccelerationDistance + SpeedDistance + DirectionDistance + StatePenalty;
}

void UAnim_MotionMatchingSystem::AddMotionClipToDatabase(UAnimSequence* AnimSequence, ECharacterMovementState State)
{
    if (!AnimSequence)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot add null animation sequence to motion database"));
        return;
    }

    if (MotionDatabase.Num() >= MaxDatabaseSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion database is full, cannot add more clips"));
        return;
    }

    FAnim_MotionClip NewClip;
    NewClip.AnimationSequence = AnimSequence;
    NewClip.ClipName = AnimSequence->GetName();
    NewClip.MovementState = State;
    NewClip.ClipDuration = AnimSequence->GetPlayLength();

    ExtractMotionDataFromAnimation(AnimSequence, NewClip);
    MotionDatabase.Add(NewClip);

    UE_LOG(LogTemp, Log, TEXT("Added motion clip '%s' to database"), *NewClip.ClipName);
}

void UAnim_MotionMatchingSystem::PlayMatchedAnimation(const FAnim_MotionClip& MotionClip)
{
    if (!MeshComponent || !MotionClip.AnimationSequence)
    {
        return;
    }

    UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
    if (AnimInstance)
    {
        // In a real implementation, this would trigger the animation blueprint
        // to play the matched animation with proper blending
        UE_LOG(LogTemp, Log, TEXT("Playing matched animation: %s"), *MotionClip.ClipName);
        
        // For now, just log the transition
        // AnimInstance->Montage_Play(MotionClip.AnimationSequence, 1.0f);
    }
}

void UAnim_MotionMatchingSystem::SetCharacterReference(ACharacter* InCharacter)
{
    OwnerCharacter = InCharacter;
    CacheCharacterReferences();
}

void UAnim_MotionMatchingSystem::DebugDrawMotionData()
{
    if (!OwnerCharacter || !GetWorld())
    {
        return;
    }

    FVector ActorLocation = OwnerCharacter->GetActorLocation();
    
    // Draw velocity vector
    DrawDebugLine(GetWorld(), ActorLocation, 
                  ActorLocation + CurrentMotionFrame.Velocity * 0.01f, 
                  FColor::Green, false, 0.1f, 0, 2.0f);

    // Draw acceleration vector
    DrawDebugLine(GetWorld(), ActorLocation + FVector(0, 0, 50), 
                  ActorLocation + FVector(0, 0, 50) + CurrentMotionFrame.Acceleration * 0.01f, 
                  FColor::Red, false, 0.1f, 0, 2.0f);

    // Draw speed indicator
    FString SpeedText = FString::Printf(TEXT("Speed: %.1f"), CurrentMotionFrame.Speed);
    DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 100), SpeedText, nullptr, FColor::White, 0.1f);
}

FString UAnim_MotionMatchingSystem::GetCurrentMotionInfo()
{
    return FString::Printf(TEXT("Clip: %s | Speed: %.1f | Moving: %s | Falling: %s"), 
                          *CurrentPlayingClip.ClipName,
                          CurrentMotionFrame.Speed,
                          CurrentMotionFrame.bIsMoving ? TEXT("Yes") : TEXT("No"),
                          CurrentMotionFrame.bIsFalling ? TEXT("Yes") : TEXT("No"));
}

void UAnim_MotionMatchingSystem::ExtractMotionDataFromAnimation(UAnimSequence* AnimSequence, FAnim_MotionClip& OutClip)
{
    if (!AnimSequence)
    {
        return;
    }

    // In a real implementation, this would extract root motion data from the animation
    // For now, create sample data based on the animation length
    float Duration = AnimSequence->GetPlayLength();
    int32 FrameCount = FMath::Max(1, FMath::FloorToInt(Duration * 30.0f)); // 30 FPS sampling

    OutClip.MotionFrames.Empty();
    for (int32 i = 0; i < FrameCount; ++i)
    {
        FAnim_MotionFrame Frame;
        Frame.TimeStamp = (float)i / 30.0f;
        
        // Sample motion data (this would be extracted from actual animation data)
        Frame.Velocity = FVector(100.0f, 0.0f, 0.0f); // Placeholder
        Frame.Speed = 100.0f;
        Frame.bIsMoving = true;
        
        OutClip.MotionFrames.Add(Frame);
    }
}

void UAnim_MotionMatchingSystem::ValidateMotionDatabase()
{
    for (int32 i = MotionDatabase.Num() - 1; i >= 0; --i)
    {
        if (!MotionDatabase[i].AnimationSequence || MotionDatabase[i].MotionFrames.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Removing invalid motion clip at index %d"), i);
            MotionDatabase.RemoveAt(i);
        }
    }
}

void UAnim_MotionMatchingSystem::CacheCharacterReferences()
{
    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(GetOwner());
    }

    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
    }
}