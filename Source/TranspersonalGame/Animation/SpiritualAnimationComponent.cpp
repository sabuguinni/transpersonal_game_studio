// SpiritualAnimationComponent.cpp
// Implementation of spiritual animation component

#include "SpiritualAnimationComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Curves/CurveFloat.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

USpiritualAnimationComponent::USpiritualAnimationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentAnimationData.State = EConsciousnessState::Unconscious;
    CurrentAnimationData.IntensityLevel = 0.0f;
    CurrentAnimationData.TransitionSpeed = 1.0f;
    CurrentAnimationData.EnergyFlowDirection = FVector::UpVector;
    
    bIsInTransition = false;
    bIsMeditating = false;
    bIsFloating = false;
    CurrentFloatingHeight = 0.0f;
    AnimationTimer = 0.0f;
    TargetState = EConsciousnessState::Unconscious;
    PreviousState = EConsciousnessState::Unconscious;
}

void USpiritualAnimationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize animation state
    SetConsciousnessState(EConsciousnessState::Unconscious, 0.0f);
}

void USpiritualAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    AnimationTimer += DeltaTime;
    
    // Update various animation systems
    if (bIsInTransition)
    {
        UpdateConsciousnessTransition(DeltaTime);
    }
    
    if (bIsFloating)
    {
        UpdateFloatingMovement(DeltaTime);
    }
    
    if (bIsMeditating)
    {
        UpdateMeditationBreathing(DeltaTime);
    }
    
    UpdateEnergyFlow(DeltaTime);
    UpdateAuraPulse(DeltaTime);
}

void USpiritualAnimationComponent::SetConsciousnessState(EConsciousnessState NewState, float TransitionTime)
{
    if (NewState == CurrentAnimationData.State && !bIsInTransition)
    {
        return; // Already in target state
    }
    
    PreviousState = CurrentAnimationData.State;
    TargetState = NewState;
    bIsInTransition = true;
    AnimationTimer = 0.0f;
    
    // Set transition speed based on time
    if (TransitionTime > 0.0f)
    {
        CurrentAnimationData.TransitionSpeed = 1.0f / TransitionTime;
    }
    else
    {
        CurrentAnimationData.TransitionSpeed = 10.0f; // Instant transition
    }
    
    UE_LOG(LogTemp, Log, TEXT("Transitioning consciousness state from %d to %d"), 
           (int32)PreviousState, (int32)TargetState);
}

void USpiritualAnimationComponent::PlaySpiritualMovement(ESpiritualMovementType MovementType, float Duration)
{
    UAnimMontage* MontageToPlay = nullptr;
    
    switch (MovementType)
    {
        case ESpiritualMovementType::Meditation:
            MontageToPlay = MeditationMontage;
            StartMeditationPose();
            break;
            
        case ESpiritualMovementType::Floating:
            StartFloatingAnimation(100.0f, 0.5f);
            break;
            
        case ESpiritualMovementType::EnergyFlow:
            MontageToPlay = EnergyFlowMontage;
            StartEnergyFlowAnimation(FVector::UpVector, 1.0f);
            break;
            
        case ESpiritualMovementType::Manifestation:
            TriggerTranscendentMoment(Duration);
            break;
            
        case ESpiritualMovementType::Dissolution:
            // Special dissolution animation
            ExpandConsciousnessField(1000.0f, Duration);
            break;
    }
    
    // Play animation montage if available
    if (MontageToPlay && GetOwner())
    {
        if (USkeletalMeshComponent* MeshComp = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
        {
            if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
            {
                AnimInstance->Montage_Play(MontageToPlay, 1.0f);
            }
        }
    }
    
    // Set timer to stop movement after duration
    if (Duration > 0.0f)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, MovementType]()
        {
            switch (MovementType)
            {
                case ESpiritualMovementType::Meditation:
                    StopMeditationPose();
                    break;
                case ESpiritualMovementType::Floating:
                    StopFloatingAnimation();
                    break;
            }
        }, Duration, false);
    }
}

void USpiritualAnimationComponent::StartEnergyFlowAnimation(FVector FlowDirection, float Intensity)
{
    CurrentAnimationData.EnergyFlowDirection = FlowDirection.GetSafeNormal();
    CurrentAnimationData.IntensityLevel = FMath::Clamp(Intensity, 0.0f, 2.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Starting energy flow animation with intensity: %f"), Intensity);
}

void USpiritualAnimationComponent::TriggerTranscendentMoment(float Duration)
{
    // Play transcendence montage
    if (TranscendenceMontage && GetOwner())
    {
        if (USkeletalMeshComponent* MeshComp = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
        {
            if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
            {
                AnimInstance->Montage_Play(TranscendenceMontage, 1.0f);
            }
        }
    }
    
    // Temporarily elevate consciousness state
    EConsciousnessState OriginalState = CurrentAnimationData.State;
    SetConsciousnessState(EConsciousnessState::Unity, 0.5f);
    
    // Revert after duration
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, OriginalState]()
    {
        SetConsciousnessState(OriginalState, 1.0f);
    }, Duration, false);
}

void USpiritualAnimationComponent::StartMeditationPose(float BreathingRate)
{
    bIsMeditating = true;
    CurrentAnimationData.TransitionSpeed = BreathingRate;
    
    UE_LOG(LogTemp, Log, TEXT("Starting meditation pose with breathing rate: %f"), BreathingRate);
}

void USpiritualAnimationComponent::StopMeditationPose()
{
    bIsMeditating = false;
    
    UE_LOG(LogTemp, Log, TEXT("Stopping meditation pose"));
}

void USpiritualAnimationComponent::StartFloatingAnimation(float Height, float Speed)
{
    bIsFloating = true;
    CurrentFloatingHeight = Height;
    CurrentAnimationData.TransitionSpeed = Speed;
    
    UE_LOG(LogTemp, Log, TEXT("Starting floating animation at height: %f"), Height);
}

void USpiritualAnimationComponent::StopFloatingAnimation()
{
    bIsFloating = false;
    CurrentFloatingHeight = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Stopping floating animation"));
}

void USpiritualAnimationComponent::PulseAuraAnimation(float PulseRate, float MaxIntensity)
{
    CurrentAnimationData.TransitionSpeed = PulseRate;
    CurrentAnimationData.IntensityLevel = MaxIntensity;
    
    UE_LOG(LogTemp, Log, TEXT("Pulsing aura with rate: %f, intensity: %f"), PulseRate, MaxIntensity);
}

void USpiritualAnimationComponent::ExpandConsciousnessField(float ExpansionRadius, float Duration)
{
    // This would typically trigger particle effects and visual expansion
    // For now, we'll log the expansion
    UE_LOG(LogTemp, Log, TEXT("Expanding consciousness field to radius: %f over %f seconds"), 
           ExpansionRadius, Duration);
    
    // Gradually increase intensity over duration
    float StartIntensity = CurrentAnimationData.IntensityLevel;
    float TargetIntensity = 2.0f;
    
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, StartIntensity]()
    {
        CurrentAnimationData.IntensityLevel = StartIntensity;
    }, Duration, false);
}

void USpiritualAnimationComponent::UpdateConsciousnessTransition(float DeltaTime)
{
    if (!bIsInTransition)
        return;
    
    float TransitionProgress = AnimationTimer * CurrentAnimationData.TransitionSpeed;
    
    // Use curve if available, otherwise linear interpolation
    if (ConsciousnessTransitionCurve)
    {
        TransitionProgress = ConsciousnessTransitionCurve->GetFloatValue(TransitionProgress);
    }
    
    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        CurrentAnimationData.State = TargetState;
        bIsInTransition = false;
        AnimationTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Consciousness transition completed to state: %d"), (int32)TargetState);
    }
    else
    {
        // Update intensity based on transition progress
        float StateIntensity = GetStateTransitionProgress();
        CurrentAnimationData.IntensityLevel = FMath::Lerp(0.0f, 1.0f, StateIntensity);
    }
}

void USpiritualAnimationComponent::UpdateFloatingMovement(float DeltaTime)
{
    if (!bIsFloating || !GetOwner())
        return;
    
    FVector FloatingOffset = CalculateFloatingOffset(DeltaTime);
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector NewLocation = CurrentLocation + FloatingOffset;
    
    GetOwner()->SetActorLocation(NewLocation);
}

void USpiritualAnimationComponent::UpdateMeditationBreathing(float DeltaTime)
{
    if (!bIsMeditating)
        return;
    
    float BreathingIntensity = CalculateBreathingIntensity(DeltaTime);
    CurrentAnimationData.IntensityLevel = BreathingIntensity;
}

void USpiritualAnimationComponent::UpdateEnergyFlow(float DeltaTime)
{
    // Update energy flow visualization
    // This would typically update particle systems or material parameters
    if (CurrentAnimationData.EnergyFlowDirection.SizeSquared() > 0.0f)
    {
        // Rotate energy flow direction slightly for dynamic effect
        FRotator FlowRotation = FRotator(0.0f, DeltaTime * 30.0f, 0.0f);
        CurrentAnimationData.EnergyFlowDirection = FlowRotation.RotateVector(CurrentAnimationData.EnergyFlowDirection);
    }
}

void USpiritualAnimationComponent::UpdateAuraPulse(float DeltaTime)
{
    // Create pulsing effect for aura
    if (AuraPulseCurve)
    {
        float PulseValue = AuraPulseCurve->GetFloatValue(AnimationTimer * CurrentAnimationData.TransitionSpeed);
        // This would typically update material parameters or particle systems
    }
}

float USpiritualAnimationComponent::GetStateTransitionProgress() const
{
    if (!bIsInTransition)
        return 1.0f;
    
    float Progress = AnimationTimer * CurrentAnimationData.TransitionSpeed;
    return FMath::Clamp(Progress, 0.0f, 1.0f);
}

FVector USpiritualAnimationComponent::CalculateFloatingOffset(float DeltaTime) const
{
    if (!FloatingCurve)
    {
        // Simple sine wave floating
        float FloatValue = FMath::Sin(AnimationTimer * CurrentAnimationData.TransitionSpeed) * CurrentFloatingHeight;
        return FVector(0.0f, 0.0f, FloatValue * DeltaTime);
    }
    
    float CurveValue = FloatingCurve->GetFloatValue(AnimationTimer);
    return FVector(0.0f, 0.0f, CurveValue * CurrentFloatingHeight * DeltaTime);
}

float USpiritualAnimationComponent::CalculateBreathingIntensity(float DeltaTime) const
{
    if (BreathingCurve)
    {
        return BreathingCurve->GetFloatValue(AnimationTimer * CurrentAnimationData.TransitionSpeed);
    }
    
    // Simple breathing pattern
    return (FMath::Sin(AnimationTimer * CurrentAnimationData.TransitionSpeed * 2.0f) + 1.0f) * 0.5f;
}