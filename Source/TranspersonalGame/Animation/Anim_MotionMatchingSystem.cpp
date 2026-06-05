#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize motion data
    CurrentMotionData = FAnim_MotionData();
    PreviousState = EAnim_MotionState::Idle;
    StateTransitionTime = 0.0f;
    TimeSinceLastTransition = 0.0f;

    // Initialize motion settings
    MotionSettings = FAnim_MotionMatchingSettings();

    // Reserve motion history array
    MotionHistory.Reserve(MaxMotionHistoryFrames);
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMotionMatching();
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateMotionData(DeltaTime);
    UpdateMotionHistory();

    // Determine new state
    EAnim_MotionState NewState = DetermineMotionState();
    
    // Handle state transitions
    if (NewState != CurrentMotionData.CurrentState)
    {
        TransitionToState(NewState);
    }

    TimeSinceLastTransition += DeltaTime;
}

void UAnim_MotionMatchingSystem::UpdateMotionData(float DeltaTime)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Get character if available
    ACharacter* Character = Cast<ACharacter>(Owner);
    if (Character && Character->GetCharacterMovement())
    {
        UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
        
        // Update velocity and speed
        CurrentMotionData.Velocity = MovementComp->Velocity;
        CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
        
        // Update direction relative to actor forward
        CurrentMotionData.Direction = CalculateDirectionAngle();
        
        // Update air state
        CurrentMotionData.bIsInAir = MovementComp->IsFalling();
        
        // Update crouching state
        CurrentMotionData.bIsCrouching = MovementComp->IsCrouching();
    }
    else
    {
        // Fallback for non-character actors
        FVector CurrentLocation = Owner->GetActorLocation();
        static FVector LastLocation = CurrentLocation;
        
        CurrentMotionData.Velocity = (CurrentLocation - LastLocation) / DeltaTime;
        CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
        CurrentMotionData.Direction = CalculateDirectionAngle();
        
        LastLocation = CurrentLocation;
    }
}

EAnim_MotionState UAnim_MotionMatchingSystem::DetermineMotionState()
{
    // Air states have priority
    if (CurrentMotionData.bIsInAir)
    {
        if (CurrentMotionData.Velocity.Z > 50.0f)
        {
            return EAnim_MotionState::Jumping;
        }
        else
        {
            return EAnim_MotionState::Falling;
        }
    }

    // Ground states
    if (CurrentMotionData.bIsCrouching)
    {
        return EAnim_MotionState::Crouching;
    }

    // Movement states based on speed
    if (CurrentMotionData.Speed < 10.0f)
    {
        return EAnim_MotionState::Idle;
    }
    else if (CurrentMotionData.Speed < 300.0f)
    {
        return EAnim_MotionState::Walking;
    }
    else
    {
        return EAnim_MotionState::Running;
    }
}

void UAnim_MotionMatchingSystem::TransitionToState(EAnim_MotionState NewState)
{
    PreviousState = CurrentMotionData.CurrentState;
    CurrentMotionData.CurrentState = NewState;
    TimeSinceLastTransition = 0.0f;

    // Handle specific state transitions
    switch (NewState)
    {
        case EAnim_MotionState::Jumping:
            if (JumpMontage)
            {
                PlayMotionMontage(JumpMontage);
            }
            break;
            
        case EAnim_MotionState::Landing:
            if (LandingMontage)
            {
                PlayMotionMontage(LandingMontage);
            }
            break;
            
        default:
            // Use blend space for other states
            break;
    }

    // Log state change for debugging
    UE_LOG(LogTemp, Log, TEXT("Motion state changed from %d to %d"), (int32)PreviousState, (int32)NewState);
}

float UAnim_MotionMatchingSystem::CalculateDirectionAngle()
{
    if (!GetOwner() || CurrentMotionData.Speed < 1.0f)
    {
        return 0.0f;
    }

    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector VelocityDirection = CurrentMotionData.Velocity.GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
    
    return FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
}

void UAnim_MotionMatchingSystem::PlayMotionMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage || !SkeletalMeshComponent)
    {
        return;
    }

    UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
    }
}

bool UAnim_MotionMatchingSystem::IsMoving() const
{
    return CurrentMotionData.Speed > 10.0f;
}

bool UAnim_MotionMatchingSystem::IsInAir() const
{
    return CurrentMotionData.bIsInAir;
}

float UAnim_MotionMatchingSystem::GetCurrentSpeed() const
{
    return CurrentMotionData.Speed;
}

FVector UAnim_MotionMatchingSystem::GetCurrentVelocity() const
{
    return CurrentMotionData.Velocity;
}

void UAnim_MotionMatchingSystem::InitializeMotionMatching()
{
    // Find skeletal mesh component if not set
    if (!SkeletalMeshComponent)
    {
        SkeletalMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    }

    if (SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("Motion Matching System initialized for %s"), *GetOwner()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No SkeletalMeshComponent found for Motion Matching System on %s"), *GetOwner()->GetName());
    }
}

void UAnim_MotionMatchingSystem::SetSkeletalMeshComponent(USkeletalMeshComponent* MeshComp)
{
    SkeletalMeshComponent = MeshComp;
}

void UAnim_MotionMatchingSystem::UpdateMotionHistory()
{
    // Add current motion data to history
    MotionHistory.Add(CurrentMotionData);

    // Maintain maximum history size
    if (MotionHistory.Num() > MaxMotionHistoryFrames)
    {
        MotionHistory.RemoveAt(0);
    }
}

void UAnim_MotionMatchingSystem::PredictFutureMotion(float PredictionTime, FAnim_MotionData& OutPredictedMotion)
{
    OutPredictedMotion = CurrentMotionData;

    if (MotionHistory.Num() < 2)
    {
        return;
    }

    // Simple linear prediction based on recent motion
    FAnim_MotionData RecentMotion = MotionHistory.Last();
    FAnim_MotionData PreviousMotion = MotionHistory[MotionHistory.Num() - 2];

    FVector VelocityChange = RecentMotion.Velocity - PreviousMotion.Velocity;
    OutPredictedMotion.Velocity = RecentMotion.Velocity + (VelocityChange * PredictionTime);
    OutPredictedMotion.Speed = OutPredictedMotion.Velocity.Size();
}

float UAnim_MotionMatchingSystem::CalculateMotionSimilarity(const FAnim_MotionData& A, const FAnim_MotionData& B)
{
    float VelocitySimilarity = 1.0f - FVector::Dist(A.Velocity, B.Velocity) / 1000.0f;
    float SpeedSimilarity = 1.0f - FMath::Abs(A.Speed - B.Speed) / 500.0f;
    float DirectionSimilarity = 1.0f - FMath::Abs(A.Direction - B.Direction) / 180.0f;

    VelocitySimilarity = FMath::Clamp(VelocitySimilarity, 0.0f, 1.0f);
    SpeedSimilarity = FMath::Clamp(SpeedSimilarity, 0.0f, 1.0f);
    DirectionSimilarity = FMath::Clamp(DirectionSimilarity, 0.0f, 1.0f);

    return (VelocitySimilarity * MotionSettings.VelocityWeight + 
            SpeedSimilarity * 0.5f + 
            DirectionSimilarity * 0.3f) / (MotionSettings.VelocityWeight + 0.8f);
}

void UAnim_MotionMatchingSystem::SmoothTransition(EAnim_MotionState FromState, EAnim_MotionState ToState, float DeltaTime)
{
    // Implement smooth blending between states
    StateTransitionTime += DeltaTime;
    
    // Transition duration varies by state change
    float TransitionDuration = 0.2f;
    if (FromState == EAnim_MotionState::Idle && ToState == EAnim_MotionState::Walking)
    {
        TransitionDuration = 0.1f;
    }
    else if (FromState == EAnim_MotionState::Walking && ToState == EAnim_MotionState::Running)
    {
        TransitionDuration = 0.15f;
    }

    float BlendAlpha = FMath::Clamp(StateTransitionTime / TransitionDuration, 0.0f, 1.0f);
    
    // Apply smooth transition logic here
    // This would typically involve blending animation weights
}