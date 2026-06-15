#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Engine/Engine.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize motion matching parameters
    VelocityWeight = 1.0f;
    AccelerationWeight = 0.5f;
    DirectionWeight = 0.8f;
    PoseWeight = 1.2f;
    BlendTime = 0.2f;
    SearchRadius = 100.0f;

    // Initialize state
    CurrentVelocity = FVector::ZeroVector;
    CurrentAcceleration = FVector::ZeroVector;
    CurrentSpeed = 0.0f;
    CurrentDirection = 0.0f;
    BestMatchIndex = -1;
    MatchScore = 0.0f;
    PreviousVelocity = FVector::ZeroVector;
    DeltaTimeAccumulator = 0.0f;

    OwnerCharacter = nullptr;
    SkeletalMeshComponent = nullptr;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();

    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        UE_LOG(LogTemp, Log, TEXT("Motion Matching System initialized for character: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching System: Owner is not a Character"));
    }
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter || MotionDatabase.Poses.Num() == 0)
    {
        return;
    }

    // Update character state
    UpdateCharacterState();

    // Accumulate delta time for motion matching frequency control
    DeltaTimeAccumulator += DeltaTime;
    
    // Run motion matching at 30 FPS to reduce computational cost
    if (DeltaTimeAccumulator >= 1.0f / 30.0f)
    {
        // Find best pose match
        int32 NewBestMatch = FindBestPoseMatch(CurrentVelocity, CurrentAcceleration, CurrentDirection);
        
        if (NewBestMatch != BestMatchIndex && NewBestMatch >= 0)
        {
            BestMatchIndex = NewBestMatch;
            
            // Log the match for debugging
            if (GEngine)
            {
                FString DebugString = FString::Printf(TEXT("Motion Match: Index %d, Speed: %.1f, Direction: %.1f"), 
                    BestMatchIndex, CurrentSpeed, CurrentDirection);
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, DebugString);
            }
        }
        
        DeltaTimeAccumulator = 0.0f;
    }

    // Update previous velocity for acceleration calculation
    PreviousVelocity = CurrentVelocity;
}

void UAnim_MotionMatchingSystem::UpdateCharacterState()
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

    // Get current velocity
    CurrentVelocity = MovementComponent->Velocity;
    CurrentSpeed = CurrentVelocity.Size();

    // Calculate acceleration
    if (DeltaTimeAccumulator > 0.0f)
    {
        CurrentAcceleration = (CurrentVelocity - PreviousVelocity) / DeltaTimeAccumulator;
    }

    // Calculate direction relative to character forward
    if (CurrentSpeed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = CurrentVelocity.GetSafeNormal();
        
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
        
        CurrentDirection = FMath::Atan2(CrossProduct, DotProduct);
    }
    else
    {
        CurrentDirection = 0.0f;
    }
}

void UAnim_MotionMatchingSystem::BuildMotionDatabase(const TArray<UAnimSequence*>& Animations)
{
    MotionDatabase.Poses.Empty();
    MotionDatabase.SourceAnimations = Animations;

    for (UAnimSequence* Animation : Animations)
    {
        if (!Animation)
        {
            continue;
        }

        float AnimationLength = Animation->GetPlayLength();
        float SampleInterval = 1.0f / MotionDatabase.SamplingRate;
        int32 NumSamples = FMath::FloorToInt(AnimationLength / SampleInterval);

        for (int32 SampleIndex = 0; SampleIndex < NumSamples; ++SampleIndex)
        {
            float SampleTime = SampleIndex * SampleInterval;
            
            FAnim_MotionPose NewPose;
            NewPose.TimeStamp = SampleTime;
            NewPose.SourceAnimation = Animation;
            
            SampleAnimationPose(Animation, SampleTime, NewPose);
            MotionDatabase.Poses.Add(NewPose);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Motion Database built with %d poses from %d animations"), 
        MotionDatabase.Poses.Num(), Animations.Num());
}

void UAnim_MotionMatchingSystem::SampleAnimationPose(UAnimSequence* Animation, float Time, FAnim_MotionPose& OutPose)
{
    if (!Animation || !SkeletalMeshComponent)
    {
        return;
    }

    // Sample bone transforms at the given time
    // This is a simplified version - in a full implementation, you'd extract actual bone poses
    
    // For now, we'll extract velocity and direction from root motion
    FTransform RootMotionTransform = Animation->ExtractRootMotion(Time, Time + 0.033f, false);
    
    OutPose.Velocity = RootMotionTransform.GetLocation() / 0.033f; // Convert to velocity
    OutPose.Speed = OutPose.Velocity.Size();
    
    if (OutPose.Speed > 1.0f)
    {
        FVector VelocityDirection = OutPose.Velocity.GetSafeNormal();
        OutPose.Direction = FMath::Atan2(VelocityDirection.Y, VelocityDirection.X);
    }
    else
    {
        OutPose.Direction = 0.0f;
    }

    // Calculate acceleration by sampling adjacent frames
    if (Time > 0.033f)
    {
        FTransform PrevTransform = Animation->ExtractRootMotion(Time - 0.033f, Time, false);
        FVector PrevVelocity = PrevTransform.GetLocation() / 0.033f;
        OutPose.Acceleration = (OutPose.Velocity - PrevVelocity) / 0.033f;
    }
    else
    {
        OutPose.Acceleration = FVector::ZeroVector;
    }
}

int32 UAnim_MotionMatchingSystem::FindBestPoseMatch(const FVector& TargetVelocity, const FVector& TargetAcceleration, float TargetDirection)
{
    if (MotionDatabase.Poses.Num() == 0)
    {
        return -1;
    }

    int32 BestIndex = -1;
    float BestDistance = FLT_MAX;

    for (int32 i = 0; i < MotionDatabase.Poses.Num(); ++i)
    {
        const FAnim_MotionPose& Pose = MotionDatabase.Poses[i];
        
        float Distance = CalculatePoseDistance(
            FAnim_MotionPose{TargetVelocity, TargetAcceleration, TargetVelocity.Size(), TargetDirection, {}, 0.0f, nullptr},
            Pose
        );

        if (Distance < BestDistance)
        {
            BestDistance = Distance;
            BestIndex = i;
        }
    }

    MatchScore = BestDistance;
    return BestIndex;
}

float UAnim_MotionMatchingSystem::CalculatePoseDistance(const FAnim_MotionPose& PoseA, const FAnim_MotionPose& PoseB)
{
    float VelocityDistance = CalculateFeatureDistance(PoseA.Velocity, PoseB.Velocity, VelocityWeight);
    float AccelerationDistance = CalculateFeatureDistance(PoseA.Acceleration, PoseB.Acceleration, AccelerationWeight);
    
    float DirectionDistance = FMath::Abs(PoseA.Direction - PoseB.Direction) * DirectionWeight;
    if (DirectionDistance > PI)
    {
        DirectionDistance = 2.0f * PI - DirectionDistance;
    }

    // In a full implementation, you'd also compare bone poses
    float PoseDistance = 0.0f; // Simplified for now

    return VelocityDistance + AccelerationDistance + DirectionDistance + (PoseDistance * PoseWeight);
}

float UAnim_MotionMatchingSystem::CalculateFeatureDistance(const FVector& A, const FVector& B, float Weight)
{
    return FVector::Dist(A, B) * Weight;
}

UAnimSequence* UAnim_MotionMatchingSystem::GetCurrentBestAnimation() const
{
    if (BestMatchIndex >= 0 && BestMatchIndex < MotionDatabase.Poses.Num())
    {
        return MotionDatabase.Poses[BestMatchIndex].SourceAnimation;
    }
    return nullptr;
}

float UAnim_MotionMatchingSystem::GetCurrentAnimationTime() const
{
    if (BestMatchIndex >= 0 && BestMatchIndex < MotionDatabase.Poses.Num())
    {
        return MotionDatabase.Poses[BestMatchIndex].TimeStamp;
    }
    return 0.0f;
}