#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    bMotionMatchingEnabled = true;
    OwnerMeshComponent = nullptr;
    OwnerAnimInstance = nullptr;
    
    AttackMontage = nullptr;
    InteractMontage = nullptr;
    HurtMontage = nullptr;
    JumpMontage = nullptr;
    
    // Initialize default settings
    MotionMatchingSettings = FAnim_MotionMatchingSettings();
    CurrentPoseData = FAnim_PoseSearchData();
    
    // Reserve space for trajectory history
    TrajectoryHistory.Reserve(MotionMatchingSettings.TrajectorySamples);
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponent();
}

void UAnim_MotionMatchingComponent::InitializeComponent()
{
    // Get owner's skeletal mesh component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        OwnerMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (OwnerMeshComponent)
        {
            OwnerAnimInstance = OwnerMeshComponent->GetAnimInstance();
            
            // Bind montage events
            if (OwnerAnimInstance)
            {
                OwnerAnimInstance->OnMontageEnded.AddDynamic(this, &UAnim_MotionMatchingComponent::OnMontageEnded);
                OwnerAnimInstance->OnMontageBlendingOut.AddDynamic(this, &UAnim_MotionMatchingComponent::OnMontageBlendingOut);
            }
        }
    }
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bMotionMatchingEnabled)
    {
        UpdatePoseSearchData();
        UpdateTrajectoryHistory();
        CleanupTrajectoryHistory();
    }
}

void UAnim_MotionMatchingComponent::UpdatePoseSearchData()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Update position and rotation
    CurrentPoseData.Position = Owner->GetActorLocation();
    CurrentPoseData.Rotation = Owner->GetActorRotation();
    
    // Update velocity and speed
    if (ACharacter* Character = Cast<ACharacter>(Owner))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            CurrentPoseData.Velocity = MovementComp->Velocity;
            CurrentPoseData.Speed = CurrentPoseData.Velocity.Size();
            CurrentPoseData.bIsGrounded = MovementComp->IsMovingOnGround();
            
            // Calculate movement direction relative to actor forward
            FVector ForwardVector = Owner->GetActorForwardVector();
            FVector VelocityNormalized = CurrentPoseData.Velocity.GetSafeNormal();
            CurrentPoseData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        }
    }
}

void UAnim_MotionMatchingComponent::UpdateTrajectoryHistory()
{
    // Add current pose data to trajectory history
    TrajectoryHistory.Add(CurrentPoseData);
    
    // Maintain maximum history size
    if (TrajectoryHistory.Num() > MotionMatchingSettings.TrajectorySamples)
    {
        TrajectoryHistory.RemoveAt(0);
    }
}

void UAnim_MotionMatchingComponent::CleanupTrajectoryHistory()
{
    // Remove old trajectory data beyond the configured samples
    while (TrajectoryHistory.Num() > MotionMatchingSettings.TrajectorySamples)
    {
        TrajectoryHistory.RemoveAt(0);
    }
}

float UAnim_MotionMatchingComponent::CalculatePoseDistance(const FAnim_PoseSearchData& PoseA, const FAnim_PoseSearchData& PoseB)
{
    float PositionDistance = FVector::Dist(PoseA.Position, PoseB.Position);
    float VelocityDistance = FVector::Dist(PoseA.Velocity, PoseB.Velocity);
    float SpeedDifference = FMath::Abs(PoseA.Speed - PoseB.Speed);
    float DirectionDifference = FMath::Abs(PoseA.Direction - PoseB.Direction);
    
    // Weighted distance calculation
    float WeightedDistance = (PositionDistance * 0.3f) + 
                           (VelocityDistance * 0.3f) + 
                           (SpeedDifference * 0.2f) + 
                           (DirectionDifference * 0.2f);
    
    return WeightedDistance;
}

void UAnim_MotionMatchingComponent::PlayAnimationMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage || !OwnerAnimInstance)
    {
        return;
    }
    
    OwnerAnimInstance->Montage_Play(Montage, PlayRate);
}

void UAnim_MotionMatchingComponent::StopAnimationMontage(UAnimMontage* Montage, float BlendOutTime)
{
    if (!Montage || !OwnerAnimInstance)
    {
        return;
    }
    
    OwnerAnimInstance->Montage_Stop(BlendOutTime, Montage);
}

bool UAnim_MotionMatchingComponent::IsPlayingMontage(UAnimMontage* Montage)
{
    if (!Montage || !OwnerAnimInstance)
    {
        return false;
    }
    
    return OwnerAnimInstance->Montage_IsPlaying(Montage);
}

float UAnim_MotionMatchingComponent::GetMontagePosition(UAnimMontage* Montage)
{
    if (!Montage || !OwnerAnimInstance)
    {
        return 0.0f;
    }
    
    return OwnerAnimInstance->Montage_GetPosition(Montage);
}

void UAnim_MotionMatchingComponent::SetMotionMatchingEnabled(bool bEnabled)
{
    bMotionMatchingEnabled = bEnabled;
}

TArray<FVector> UAnim_MotionMatchingComponent::PredictTrajectory(float PredictionTime, int32 Samples)
{
    TArray<FVector> PredictedTrajectory;
    
    if (Samples <= 0 || PredictionTime <= 0.0f)
    {
        return PredictedTrajectory;
    }
    
    float TimeStep = PredictionTime / Samples;
    FVector CurrentPosition = CurrentPoseData.Position;
    FVector CurrentVelocity = CurrentPoseData.Velocity;
    
    for (int32 i = 0; i < Samples; i++)
    {
        float Time = TimeStep * (i + 1);
        FVector PredictedPosition = CurrentPosition + (CurrentVelocity * Time);
        PredictedTrajectory.Add(PredictedPosition);
    }
    
    return PredictedTrajectory;
}

void UAnim_MotionMatchingComponent::BlendToAnimation(UAnimSequence* Animation, float BlendTime)
{
    if (!Animation || !OwnerAnimInstance)
    {
        return;
    }
    
    // This would typically be implemented with a custom animation node
    // For now, we'll use montage playback as a placeholder
    UE_LOG(LogTemp, Log, TEXT("BlendToAnimation called for: %s"), *Animation->GetName());
}

void UAnim_MotionMatchingComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!Montage)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Montage ended: %s, Interrupted: %s"), 
           *Montage->GetName(), 
           bInterrupted ? TEXT("Yes") : TEXT("No"));
    
    // Handle specific montage endings
    if (Montage == AttackMontage)
    {
        // Attack montage finished
    }
    else if (Montage == InteractMontage)
    {
        // Interaction montage finished
    }
    else if (Montage == HurtMontage)
    {
        // Hurt montage finished
    }
    else if (Montage == JumpMontage)
    {
        // Jump montage finished
    }
}

void UAnim_MotionMatchingComponent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    if (!Montage)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Montage blending out: %s, Interrupted: %s"), 
           *Montage->GetName(), 
           bInterrupted ? TEXT("Yes") : TEXT("No"));
}