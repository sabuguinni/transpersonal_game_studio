#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Engine/Engine.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize motion matching parameters
    PoseSearchRadius = 100.0f;
    VelocityWeight = 0.4f;
    DirectionWeight = 0.3f;
    BoneWeight = 0.3f;

    BlendAlpha = 0.0f;
    BlendTimer = 0.0f;
    BlendDuration = 0.2f;
    bIsBlending = false;

    LastUpdateTime = 0.0f;
    UpdateFrequency = 30.0f; // 30 FPS for motion matching

    OwnerCharacter = nullptr;
    AnimInstance = nullptr;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponent();
}

void UAnim_MotionMatchingComponent::InitializeComponent()
{
    // Get owner character
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Owner is not a Character"));
        return;
    }

    // Get animation instance
    if (OwnerCharacter->GetMesh())
    {
        AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
        if (!AnimInstance)
        {
            UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: No AnimInstance found"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("MotionMatchingComponent initialized for %s"), *OwnerCharacter->GetName());
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter)
    {
        return;
    }

    // Update at specified frequency for performance
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime < (1.0f / UpdateFrequency))
    {
        return;
    }
    LastUpdateTime = CurrentTime;

    UpdateMotionMatching(DeltaTime);
}

void UAnim_MotionMatchingComponent::UpdateMotionMatching(float DeltaTime)
{
    if (!OwnerCharacter || !AnimInstance)
    {
        return;
    }

    // Update current pose from character state
    UpdateCurrentPose();

    // Handle blending if active
    if (bIsBlending)
    {
        BlendTimer += DeltaTime;
        BlendAlpha = FMath::Clamp(BlendTimer / BlendDuration, 0.0f, 1.0f);

        if (BlendAlpha >= 1.0f)
        {
            bIsBlending = false;
            CurrentPose = TargetPose;
            BlendAlpha = 0.0f;
            BlendTimer = 0.0f;
        }
    }

    // Find best matching pose if we have databases
    if (MotionDatabases.Num() > 0)
    {
        FAnim_MotionPose BestPose = FindBestMatchingPose(CurrentPose);
        
        // Blend to new pose if it's significantly different
        float PoseDistance = CalculatePoseDistance(CurrentPose, BestPose);
        if (PoseDistance > PoseSearchRadius * 0.5f)
        {
            BlendToPose(BestPose, 0.15f);
        }
    }
}

void UAnim_MotionMatchingComponent::UpdateCurrentPose()
{
    if (!OwnerCharacter)
    {
        return;
    }

    // Update pose data from character
    CurrentPose.RootMotionVelocity = GetCharacterVelocity();
    CurrentPose.RootMotionDirection = GetCharacterDirection();
    CurrentPose.MovementSpeed = CurrentPose.RootMotionVelocity.Size();

    // Determine movement state
    if (CurrentPose.MovementSpeed < 10.0f)
    {
        CurrentPose.MovementState = EAnim_MovementState::Idle;
    }
    else if (CurrentPose.MovementSpeed < 300.0f)
    {
        CurrentPose.MovementState = EAnim_MovementState::Walking;
    }
    else
    {
        CurrentPose.MovementState = EAnim_MovementState::Running;
    }

    // Check if character is falling
    if (OwnerCharacter->GetCharacterMovement() && OwnerCharacter->GetCharacterMovement()->IsFalling())
    {
        CurrentPose.MovementState = EAnim_MovementState::Falling;
    }

    CurrentPose.Timestamp = GetWorld()->GetTimeSeconds();
}

FAnim_MotionPose UAnim_MotionMatchingComponent::FindBestMatchingPose(const FAnim_MotionPose& QueryPose)
{
    FAnim_MotionPose BestPose = QueryPose;
    float BestDistance = FLT_MAX;

    // Search through all motion databases
    for (const FAnim_MotionDatabase& Database : MotionDatabases)
    {
        for (const FAnim_MotionPose& Pose : Database.Poses)
        {
            // Only consider poses from the same movement state
            if (Pose.MovementState != QueryPose.MovementState)
            {
                continue;
            }

            float Distance = CalculatePoseDistance(QueryPose, Pose);
            if (Distance < BestDistance)
            {
                BestDistance = Distance;
                BestPose = Pose;
            }
        }
    }

    return BestPose;
}

float UAnim_MotionMatchingComponent::CalculatePoseDistance(const FAnim_MotionPose& PoseA, const FAnim_MotionPose& PoseB)
{
    float Distance = 0.0f;

    // Velocity distance
    float VelocityDistance = FVector::Dist(PoseA.RootMotionVelocity, PoseB.RootMotionVelocity);
    Distance += VelocityDistance * VelocityWeight;

    // Direction distance
    float DirectionDistance = FVector::Dist(PoseA.RootMotionDirection, PoseB.RootMotionDirection);
    Distance += DirectionDistance * DirectionWeight;

    // Speed distance
    float SpeedDistance = FMath::Abs(PoseA.MovementSpeed - PoseB.MovementSpeed);
    Distance += SpeedDistance * 0.01f; // Scale down speed difference

    // Bone transform distance (simplified)
    if (PoseA.BoneTransforms.Num() > 0 && PoseB.BoneTransforms.Num() > 0)
    {
        int32 MinBones = FMath::Min(PoseA.BoneTransforms.Num(), PoseB.BoneTransforms.Num());
        float BoneDistance = 0.0f;
        
        for (int32 i = 0; i < MinBones; ++i)
        {
            BoneDistance += FVector::Dist(PoseA.BoneTransforms[i].GetLocation(), PoseB.BoneTransforms[i].GetLocation());
        }
        
        Distance += (BoneDistance / MinBones) * BoneWeight;
    }

    return Distance;
}

void UAnim_MotionMatchingComponent::BuildMotionDatabase(UAnimSequence* Animation, const FString& DatabaseName)
{
    if (!Animation)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: Cannot build database from null animation"));
        return;
    }

    FAnim_MotionDatabase NewDatabase;
    NewDatabase.SourceAnimation = Animation;
    NewDatabase.DatabaseName = DatabaseName;

    // Sample poses from animation at regular intervals
    float AnimLength = Animation->GetPlayLength();
    float SampleRate = 30.0f; // 30 samples per second
    int32 NumSamples = FMath::FloorToInt(AnimLength * SampleRate);

    for (int32 i = 0; i < NumSamples; ++i)
    {
        float SampleTime = (float)i / SampleRate;
        
        FAnim_MotionPose SamplePose;
        SamplePose.Timestamp = SampleTime;
        
        // Extract pose data at this time
        // Note: In a full implementation, you would extract bone transforms and root motion here
        // For now, we'll create a basic pose structure
        
        NewDatabase.Poses.Add(SamplePose);
    }

    MotionDatabases.Add(NewDatabase);
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingComponent: Built database '%s' with %d poses"), *DatabaseName, NewDatabase.Poses.Num());
}

void UAnim_MotionMatchingComponent::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentPose.MovementState != NewState)
    {
        CurrentPose.MovementState = NewState;
        UE_LOG(LogTemp, Log, TEXT("MotionMatchingComponent: Movement state changed to %d"), (int32)NewState);
    }
}

EAnim_MovementState UAnim_MotionMatchingComponent::GetCurrentMovementState() const
{
    return CurrentPose.MovementState;
}

void UAnim_MotionMatchingComponent::BlendToPose(const FAnim_MotionPose& NewPose, float BlendTime)
{
    TargetPose = NewPose;
    BlendDuration = BlendTime;
    BlendTimer = 0.0f;
    BlendAlpha = 0.0f;
    bIsBlending = true;
}

FVector UAnim_MotionMatchingComponent::GetCharacterVelocity() const
{
    if (OwnerCharacter && OwnerCharacter->GetCharacterMovement())
    {
        return OwnerCharacter->GetCharacterMovement()->Velocity;
    }
    return FVector::ZeroVector;
}

FVector UAnim_MotionMatchingComponent::GetCharacterDirection() const
{
    if (OwnerCharacter)
    {
        FVector Velocity = GetCharacterVelocity();
        if (Velocity.SizeSquared() > 1.0f)
        {
            return Velocity.GetSafeNormal();
        }
        return OwnerCharacter->GetActorForwardVector();
    }
    return FVector::ForwardVector;
}