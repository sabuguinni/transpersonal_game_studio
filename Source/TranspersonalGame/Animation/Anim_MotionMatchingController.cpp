#include "Anim_MotionMatchingController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UAnim_MotionMatchingController::UAnim_MotionMatchingController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize settings
    FeatureMatchThreshold = 0.5f;
    VelocityWeight = 1.0f;
    AccelerationWeight = 0.5f;
    DirectionWeight = 0.8f;
    MinClipDuration = 0.5f;
    bEnableMotionMatching = true;

    // Initialize internal state
    BlendWeight = 0.0f;
    LastUpdateTime = 0.0f;
    PreviousVelocity = FVector::ZeroVector;
    bIsInitialized = false;

    MovementComponent = nullptr;
    MeshComponent = nullptr;
}

void UAnim_MotionMatchingController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    BuildMotionDatabase();
    
    bIsInitialized = true;
}

void UAnim_MotionMatchingController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsInitialized || !bEnableMotionMatching)
    {
        return;
    }

    UpdateMotionFeature(DeltaTime);

    if (MotionDatabase.Num() > 0)
    {
        FAnim_MotionClip BestMatch = FindBestMotionMatch(CurrentMotionFeature);
        
        if (BestMatch.AnimSequence != CurrentMotionClip.AnimSequence || 
            FMath::Abs(BestMatch.StartTime - CurrentMotionClip.StartTime) > 0.1f)
        {
            TransitionToMotionClip(BestMatch);
        }
    }

    CurrentPlayTime += DeltaTime;
    LastUpdateTime += DeltaTime;
}

void UAnim_MotionMatchingController::InitializeComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Get character movement component
    if (ACharacter* Character = Cast<ACharacter>(Owner))
    {
        MovementComponent = Character->GetCharacterMovement();
        MeshComponent = Character->GetMesh();
    }
    else
    {
        MovementComponent = Owner->FindComponentByClass<UCharacterMovementComponent>();
        MeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    }

    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingController: No CharacterMovementComponent found"));
    }

    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingController: No SkeletalMeshComponent found"));
    }
}

void UAnim_MotionMatchingController::UpdateMotionFeature(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }

    // Update velocity and acceleration
    FVector CurrentVelocity = MovementComponent->Velocity;
    CurrentMotionFeature.Velocity = CurrentVelocity;
    CurrentMotionFeature.Speed = CurrentVelocity.Size();

    if (DeltaTime > 0.0f)
    {
        CurrentMotionFeature.Acceleration = (CurrentVelocity - PreviousVelocity) / DeltaTime;
    }

    // Update direction relative to forward vector
    if (AActor* Owner = GetOwner())
    {
        FVector ForwardVector = Owner->GetActorForwardVector();
        if (CurrentMotionFeature.Speed > 1.0f)
        {
            FVector NormalizedVelocity = CurrentVelocity.GetSafeNormal();
            CurrentMotionFeature.Direction = FMath::Acos(FVector::DotProduct(ForwardVector, NormalizedVelocity));
        }
        else
        {
            CurrentMotionFeature.Direction = 0.0f;
        }
    }

    // Update movement states
    CurrentMotionFeature.bIsMoving = CurrentMotionFeature.Speed > 5.0f;
    CurrentMotionFeature.bIsInAir = MovementComponent->IsFalling();
    CurrentMotionFeature.bIsCrouching = MovementComponent->IsCrouching();

    // Smooth the motion feature to reduce jitter
    SmoothMotionFeature(CurrentMotionFeature, 0.1f);

    PreviousVelocity = CurrentVelocity;
}

FAnim_MotionClip UAnim_MotionMatchingController::FindBestMotionMatch(const FAnim_MotionFeature& TargetFeature)
{
    if (MotionDatabase.Num() == 0)
    {
        return FAnim_MotionClip();
    }

    float BestDistance = FLT_MAX;
    int32 BestIndex = 0;

    for (int32 i = 0; i < MotionDatabase.Num(); i++)
    {
        const FAnim_MotionClip& Clip = MotionDatabase[i];
        
        if (!IsValidMotionClip(Clip))
        {
            continue;
        }

        float Distance = CalculateMotionDistance(TargetFeature, Clip.MotionFeature);
        
        // Add quality bonus
        Distance *= (2.0f - Clip.Quality);

        if (Distance < BestDistance)
        {
            BestDistance = Distance;
            BestIndex = i;
        }
    }

    return MotionDatabase[BestIndex];
}

float UAnim_MotionMatchingController::CalculateMotionDistance(const FAnim_MotionFeature& FeatureA, const FAnim_MotionFeature& FeatureB)
{
    float Distance = 0.0f;

    // Velocity distance
    float VelocityDistance = FVector::Dist(FeatureA.Velocity, FeatureB.Velocity);
    Distance += VelocityDistance * VelocityWeight;

    // Acceleration distance
    float AccelerationDistance = FVector::Dist(FeatureA.Acceleration, FeatureB.Acceleration);
    Distance += AccelerationDistance * AccelerationWeight;

    // Direction distance
    float DirectionDistance = FMath::Abs(FeatureA.Direction - FeatureB.Direction);
    Distance += DirectionDistance * DirectionWeight;

    // Boolean state penalties
    if (FeatureA.bIsMoving != FeatureB.bIsMoving)
    {
        Distance += 100.0f;
    }

    if (FeatureA.bIsInAir != FeatureB.bIsInAir)
    {
        Distance += 200.0f;
    }

    if (FeatureA.bIsCrouching != FeatureB.bIsCrouching)
    {
        Distance += 150.0f;
    }

    return Distance;
}

void UAnim_MotionMatchingController::TransitionToMotionClip(const FAnim_MotionClip& NewClip)
{
    if (!IsValidMotionClip(NewClip))
    {
        return;
    }

    CurrentMotionClip = NewClip;
    CurrentPlayTime = NewClip.StartTime;

    // Apply animation to mesh component
    if (MeshComponent && MeshComponent->GetAnimInstance())
    {
        UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
        
        // This would typically be handled by the AnimBlueprint
        // For now, we just log the transition
        UE_LOG(LogTemp, Log, TEXT("Motion Matching: Transitioning to %s at time %f"), 
               NewClip.AnimSequence ? *NewClip.AnimSequence->GetName() : TEXT("None"), 
               NewClip.StartTime);
    }
}

void UAnim_MotionMatchingController::AddMotionClipToDatabase(UAnimSequence* AnimSequence, float StartTime, float EndTime, const FAnim_MotionFeature& Feature, EAnim_MovementState MovementState)
{
    if (!AnimSequence)
    {
        return;
    }

    FAnim_MotionClip NewClip;
    NewClip.AnimSequence = AnimSequence;
    NewClip.StartTime = StartTime;
    NewClip.EndTime = EndTime;
    NewClip.MotionFeature = Feature;
    NewClip.MovementState = MovementState;
    NewClip.Quality = 1.0f;

    MotionDatabase.Add(NewClip);
}

void UAnim_MotionMatchingController::BuildMotionDatabase()
{
    ClearMotionDatabase();

    // This would typically load animation sequences from content
    // For now, we create placeholder entries for different movement states
    
    // Idle motion features
    FAnim_MotionFeature IdleFeature;
    IdleFeature.Speed = 0.0f;
    IdleFeature.bIsMoving = false;
    IdleFeature.bIsInAir = false;
    IdleFeature.bIsCrouching = false;

    // Walking motion features
    FAnim_MotionFeature WalkFeature;
    WalkFeature.Speed = 150.0f;
    WalkFeature.Velocity = FVector(150.0f, 0.0f, 0.0f);
    WalkFeature.bIsMoving = true;
    WalkFeature.bIsInAir = false;
    WalkFeature.bIsCrouching = false;

    // Running motion features
    FAnim_MotionFeature RunFeature;
    RunFeature.Speed = 400.0f;
    RunFeature.Velocity = FVector(400.0f, 0.0f, 0.0f);
    RunFeature.bIsMoving = true;
    RunFeature.bIsInAir = false;
    RunFeature.bIsCrouching = false;

    // Jumping motion features
    FAnim_MotionFeature JumpFeature;
    JumpFeature.Speed = 200.0f;
    JumpFeature.Velocity = FVector(200.0f, 0.0f, 300.0f);
    JumpFeature.bIsMoving = true;
    JumpFeature.bIsInAir = true;
    JumpFeature.bIsCrouching = false;

    UE_LOG(LogTemp, Log, TEXT("Motion Matching: Built database with %d clips"), MotionDatabase.Num());
}

void UAnim_MotionMatchingController::ClearMotionDatabase()
{
    MotionDatabase.Empty();
}

void UAnim_MotionMatchingController::SetBlendWeight(float Weight)
{
    BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
}

float UAnim_MotionMatchingController::GetCurrentBlendWeight() const
{
    return BlendWeight;
}

void UAnim_MotionMatchingController::DebugDrawMotionFeature(const FAnim_MotionFeature& Feature, FVector WorldLocation)
{
    if (!GetWorld())
    {
        return;
    }

    // Draw velocity vector
    DrawDebugDirectionalArrow(GetWorld(), WorldLocation, WorldLocation + Feature.Velocity, 50.0f, FColor::Green, false, 0.1f, 0, 2.0f);

    // Draw acceleration vector
    DrawDebugDirectionalArrow(GetWorld(), WorldLocation + FVector(0, 0, 50), WorldLocation + FVector(0, 0, 50) + Feature.Acceleration, 30.0f, FColor::Red, false, 0.1f, 0, 1.0f);

    // Draw speed as text
    FString SpeedText = FString::Printf(TEXT("Speed: %.1f"), Feature.Speed);
    DrawDebugString(GetWorld(), WorldLocation + FVector(0, 0, 100), SpeedText, nullptr, FColor::White, 0.1f);
}

void UAnim_MotionMatchingController::PrintMotionDatabaseInfo()
{
    UE_LOG(LogTemp, Log, TEXT("=== Motion Database Info ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Clips: %d"), MotionDatabase.Num());
    
    for (int32 i = 0; i < MotionDatabase.Num(); i++)
    {
        const FAnim_MotionClip& Clip = MotionDatabase[i];
        UE_LOG(LogTemp, Log, TEXT("Clip %d: %s (%.2f-%.2f) Speed:%.1f Quality:%.2f"), 
               i, 
               Clip.AnimSequence ? *Clip.AnimSequence->GetName() : TEXT("None"),
               Clip.StartTime,
               Clip.EndTime,
               Clip.MotionFeature.Speed,
               Clip.Quality);
    }
}

bool UAnim_MotionMatchingController::IsValidMotionClip(const FAnim_MotionClip& Clip) const
{
    if (!Clip.AnimSequence)
    {
        return false;
    }

    if (Clip.EndTime <= Clip.StartTime)
    {
        return false;
    }

    if ((Clip.EndTime - Clip.StartTime) < MinClipDuration)
    {
        return false;
    }

    return true;
}

void UAnim_MotionMatchingController::SmoothMotionFeature(FAnim_MotionFeature& Feature, float SmoothingFactor)
{
    // Simple exponential smoothing
    static FAnim_MotionFeature PreviousFeature;
    
    if (LastUpdateTime > 0.0f)
    {
        Feature.Velocity = FMath::Lerp(PreviousFeature.Velocity, Feature.Velocity, SmoothingFactor);
        Feature.Acceleration = FMath::Lerp(PreviousFeature.Acceleration, Feature.Acceleration, SmoothingFactor);
        Feature.Speed = FMath::Lerp(PreviousFeature.Speed, Feature.Speed, SmoothingFactor);
        Feature.Direction = FMath::Lerp(PreviousFeature.Direction, Feature.Direction, SmoothingFactor);
    }
    
    PreviousFeature = Feature;
}

void UAnim_MotionMatchingController::ValidateMotionDatabase()
{
    for (int32 i = MotionDatabase.Num() - 1; i >= 0; i--)
    {
        if (!IsValidMotionClip(MotionDatabase[i]))
        {
            UE_LOG(LogTemp, Warning, TEXT("Removing invalid motion clip at index %d"), i);
            MotionDatabase.RemoveAt(i);
        }
    }
}