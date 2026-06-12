#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize search parameters
    SearchRadius = 1000.0f;
    VelocityWeight = 1.0f;
    PositionWeight = 0.5f;
    RotationWeight = 0.3f;
    
    OwningCharacter = nullptr;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the owning character
    OwningCharacter = Cast<ACharacter>(GetOwner());
    
    // Build motion database if animations are provided
    if (MotionDatabase.Animations.Num() > 0)
    {
        BuildDatabaseFromAnimations();
    }
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwningCharacter)
    {
        UpdateCurrentFrame();
        
        // Find best matching frame for smooth transitions
        if (MotionDatabase.Frames.Num() > 0)
        {
            FAnim_MotionFrame BestMatch = FindBestMatch(CurrentFrame);
            // Here you would apply the best match to the animation system
        }
    }
}

void UAnim_MotionMatchingComponent::UpdateCurrentFrame()
{
    if (!OwningCharacter)
    {
        return;
    }
    
    // Update current frame with character's current state
    CurrentFrame.Position = OwningCharacter->GetActorLocation();
    CurrentFrame.Rotation = OwningCharacter->GetActorRotation();
    
    if (UCharacterMovementComponent* MovementComp = OwningCharacter->GetCharacterMovement())
    {
        CurrentFrame.Velocity = MovementComp->Velocity;
    }
    
    CurrentFrame.Time = GetWorld()->GetTimeSeconds();
}

FAnim_MotionFrame UAnim_MotionMatchingComponent::FindBestMatch(const FAnim_MotionFrame& TargetFrame)
{
    if (MotionDatabase.Frames.Num() == 0)
    {
        return FAnim_MotionFrame();
    }
    
    float BestDistance = FLT_MAX;
    int32 BestIndex = 0;
    
    for (int32 i = 0; i < MotionDatabase.Frames.Num(); i++)
    {
        float Distance = CalculateFrameDistance(TargetFrame, MotionDatabase.Frames[i]);
        
        if (Distance < BestDistance)
        {
            BestDistance = Distance;
            BestIndex = i;
        }
    }
    
    return MotionDatabase.Frames[BestIndex];
}

float UAnim_MotionMatchingComponent::CalculateFrameDistance(const FAnim_MotionFrame& Frame1, const FAnim_MotionFrame& Frame2)
{
    // Calculate weighted distance between two motion frames
    float VelocityDistance = FVector::Dist(Frame1.Velocity, Frame2.Velocity) * VelocityWeight;
    float PositionDistance = FVector::Dist(Frame1.Position, Frame2.Position) * PositionWeight;
    
    // Calculate rotation distance
    float RotationDistance = FMath::Abs(FRotator::NormalizeAxis(Frame1.Rotation.Yaw - Frame2.Rotation.Yaw)) * RotationWeight;
    
    return VelocityDistance + PositionDistance + RotationDistance;
}

void UAnim_MotionMatchingComponent::AddFrameToDatabase(const FAnim_MotionFrame& Frame)
{
    MotionDatabase.Frames.Add(Frame);
}

void UAnim_MotionMatchingComponent::BuildDatabaseFromAnimations()
{
    // Clear existing frames
    MotionDatabase.Frames.Empty();
    
    // This would typically extract motion data from animation sequences
    // For now, we'll create some sample frames for basic locomotion
    
    // Sample idle frames
    for (int32 i = 0; i < 10; i++)
    {
        FAnim_MotionFrame IdleFrame;
        IdleFrame.Position = FVector::ZeroVector;
        IdleFrame.Velocity = FVector::ZeroVector;
        IdleFrame.Rotation = FRotator::ZeroRotator;
        IdleFrame.Time = i * 0.1f;
        IdleFrame.AnimationName = TEXT("Idle");
        MotionDatabase.Frames.Add(IdleFrame);
    }
    
    // Sample walking frames
    for (int32 i = 0; i < 20; i++)
    {
        FAnim_MotionFrame WalkFrame;
        WalkFrame.Position = FVector(i * 50.0f, 0.0f, 0.0f);
        WalkFrame.Velocity = FVector(150.0f, 0.0f, 0.0f);
        WalkFrame.Rotation = FRotator::ZeroRotator;
        WalkFrame.Time = i * 0.05f;
        WalkFrame.AnimationName = TEXT("Walk");
        MotionDatabase.Frames.Add(WalkFrame);
    }
    
    // Sample running frames
    for (int32 i = 0; i < 15; i++)
    {
        FAnim_MotionFrame RunFrame;
        RunFrame.Position = FVector(i * 100.0f, 0.0f, 0.0f);
        RunFrame.Velocity = FVector(500.0f, 0.0f, 0.0f);
        RunFrame.Rotation = FRotator::ZeroRotator;
        RunFrame.Time = i * 0.03f;
        RunFrame.AnimationName = TEXT("Run");
        MotionDatabase.Frames.Add(RunFrame);
    }
}