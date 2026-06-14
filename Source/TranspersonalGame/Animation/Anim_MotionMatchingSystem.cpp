#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Default weights
    VelocityWeight = 1.0f;
    AccelerationWeight = 0.5f;
    DirectionWeight = 0.3f;
    SearchRadius = 100.0f;
    BlendTime = 0.2f;
    
    // Initialize state
    CurrentState = EAnim_MotionState::Idle;
    CurrentClip = nullptr;
    CurrentPlayTime = 0.0f;
    PreviousVelocity = FVector::ZeroVector;
    TransitionTimer = 0.0f;
    bIsTransitioning = false;
    
    MovementComponent = nullptr;
    OwnerCharacter = nullptr;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner character and movement component
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
    
    // Preprocess motion database
    PreprocessMotionDatabase();
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!MovementComponent || !OwnerCharacter)
    {
        return;
    }
    
    // Update motion data
    UpdateMotionData();
    
    // Handle transitions
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        if (TransitionTimer >= BlendTime)
        {
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
        }
    }
    else
    {
        // Find best matching animation
        FAnim_MotionClip* BestMatch = FindBestMatch();
        if (BestMatch && BestMatch != CurrentClip)
        {
            TransitionToClip(BestMatch, 0.0f);
        }
    }
    
    // Update play time
    if (CurrentClip)
    {
        CurrentPlayTime += DeltaTime;
        if (CurrentPlayTime >= CurrentClip->EndTime)
        {
            CurrentPlayTime = CurrentClip->StartTime;
        }
    }
}

void UAnim_MotionMatchingSystem::UpdateMotionData()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Get current velocity and calculate acceleration
    FVector CurrentVelocity = MovementComponent->Velocity;
    FVector CurrentAcceleration = (CurrentVelocity - PreviousVelocity) / GetWorld()->GetDeltaSeconds();
    
    // Update motion frame
    CurrentMotion.Velocity = CurrentVelocity;
    CurrentMotion.Acceleration = CurrentAcceleration;
    CurrentMotion.Speed = CurrentVelocity.Size();
    CurrentMotion.bIsMoving = CurrentMotion.Speed > 10.0f;
    CurrentMotion.bIsInAir = MovementComponent->IsFalling();
    
    // Calculate movement direction relative to character facing
    if (CurrentMotion.bIsMoving)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = CurrentVelocity.GetSafeNormal();
        CurrentMotion.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityDirection)));
    }
    else
    {
        CurrentMotion.Direction = 0.0f;
    }
    
    // Update motion state
    CurrentState = DetermineMotionState();
    
    // Store previous velocity for next frame
    PreviousVelocity = CurrentVelocity;
}

FAnim_MotionClip* UAnim_MotionMatchingSystem::FindBestMatch()
{
    if (MotionDatabase.Num() == 0)
    {
        return nullptr;
    }
    
    FAnim_MotionClip* BestClip = nullptr;
    float BestCost = FLT_MAX;
    
    for (FAnim_MotionClip& Clip : MotionDatabase)
    {
        if (!Clip.Animation || Clip.MotionData.Num() == 0)
        {
            continue;
        }
        
        // Find best frame within this clip
        for (int32 FrameIndex = 0; FrameIndex < Clip.MotionData.Num(); FrameIndex++)
        {
            const FAnim_MotionFrame& ClipFrame = Clip.MotionData[FrameIndex];
            float Cost = CalculateMotionCost(CurrentMotion, ClipFrame);
            
            if (Cost < BestCost)
            {
                BestCost = Cost;
                BestClip = &Clip;
            }
        }
    }
    
    return BestClip;
}

float UAnim_MotionMatchingSystem::CalculateMotionCost(const FAnim_MotionFrame& TargetMotion, const FAnim_MotionFrame& ClipMotion)
{
    float Cost = 0.0f;
    
    // Velocity cost
    float VelocityCost = FVector::Dist(TargetMotion.Velocity, ClipMotion.Velocity);
    Cost += VelocityCost * VelocityWeight;
    
    // Acceleration cost
    float AccelerationCost = FVector::Dist(TargetMotion.Acceleration, ClipMotion.Acceleration);
    Cost += AccelerationCost * AccelerationWeight;
    
    // Direction cost
    float DirectionCost = FMath::Abs(TargetMotion.Direction - ClipMotion.Direction);
    Cost += DirectionCost * DirectionWeight;
    
    // State matching bonus
    if (TargetMotion.bIsMoving == ClipMotion.bIsMoving)
    {
        Cost *= 0.8f; // 20% bonus for matching movement state
    }
    
    if (TargetMotion.bIsInAir == ClipMotion.bIsInAir)
    {
        Cost *= 0.9f; // 10% bonus for matching air state
    }
    
    return Cost;
}

void UAnim_MotionMatchingSystem::TransitionToClip(FAnim_MotionClip* NewClip, float StartTime)
{
    if (!NewClip || !NewClip->Animation)
    {
        return;
    }
    
    CurrentClip = NewClip;
    CurrentPlayTime = StartTime;
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
    
    // Play animation on skeletal mesh
    if (OwnerCharacter && OwnerCharacter->GetMesh())
    {
        OwnerCharacter->GetMesh()->PlayAnimation(NewClip->Animation, true);
    }
}

EAnim_MotionState UAnim_MotionMatchingSystem::DetermineMotionState()
{
    if (!MovementComponent)
    {
        return EAnim_MotionState::Idle;
    }
    
    if (MovementComponent->IsFalling())
    {
        return MovementComponent->Velocity.Z < 0 ? EAnim_MotionState::Falling : EAnim_MotionState::Jumping;
    }
    
    if (MovementComponent->IsCrouching())
    {
        return CurrentMotion.bIsMoving ? EAnim_MotionState::Crawling : EAnim_MotionState::Crouching;
    }
    
    if (CurrentMotion.Speed > 300.0f)
    {
        return EAnim_MotionState::Running;
    }
    else if (CurrentMotion.Speed > 50.0f)
    {
        return EAnim_MotionState::Walking;
    }
    
    return EAnim_MotionState::Idle;
}

void UAnim_MotionMatchingSystem::AddMotionClip(UAnimSequence* Animation, const FString& ClipName)
{
    if (!Animation)
    {
        return;
    }
    
    FAnim_MotionClip NewClip;
    NewClip.Animation = Animation;
    NewClip.ClipName = ClipName;
    NewClip.StartTime = 0.0f;
    NewClip.EndTime = Animation->GetPlayLength();
    
    // TODO: Extract motion data from animation
    // This would require analyzing bone positions and velocities frame by frame
    // For now, create placeholder motion data
    int32 NumFrames = FMath::Max(1, FMath::FloorToInt(NewClip.EndTime * 30.0f)); // 30 FPS sampling
    NewClip.MotionData.Reserve(NumFrames);
    
    for (int32 i = 0; i < NumFrames; i++)
    {
        FAnim_MotionFrame Frame;
        // Placeholder - in real implementation, extract from animation
        Frame.Speed = FMath::RandRange(0.0f, 400.0f);
        Frame.bIsMoving = Frame.Speed > 10.0f;
        NewClip.MotionData.Add(Frame);
    }
    
    MotionDatabase.Add(NewClip);
}

void UAnim_MotionMatchingSystem::PreprocessMotionDatabase()
{
    // Validate and process all clips in the database
    for (int32 i = MotionDatabase.Num() - 1; i >= 0; i--)
    {
        FAnim_MotionClip& Clip = MotionDatabase[i];
        
        if (!Clip.Animation)
        {
            UE_LOG(LogTemp, Warning, TEXT("Motion clip %s has no animation - removing"), *Clip.ClipName);
            MotionDatabase.RemoveAt(i);
            continue;
        }
        
        // Ensure motion data exists
        if (Clip.MotionData.Num() == 0)
        {
            UE_LOG(LogTemp, Log, TEXT("Generating motion data for clip %s"), *Clip.ClipName);
            AddMotionClip(Clip.Animation, Clip.ClipName);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Motion database preprocessed: %d clips"), MotionDatabase.Num());
}