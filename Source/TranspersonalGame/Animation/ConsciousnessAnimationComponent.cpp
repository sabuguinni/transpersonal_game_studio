#include "ConsciousnessAnimationComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

UConsciousnessAnimationComponent::UConsciousnessAnimationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    BaseAnimationSpeed = 1.0f;
    TransformationIntensity = 1.0f;
    bEnableAutoStateTransitions = false;
    
    TargetState = EConsciousnessState::Dormant;
    StateTransitionDuration = 2.0f;
    StateTransitionTimer = 0.0f;
    bIsTransitioning = false;
    
    CurrentTransformation = ETransformationType::Materialization;
    TransformationDuration = 3.0f;
    TransformationTimer = 0.0f;
    bIsTransforming = false;
    
    AuraPulseIntensity = 1.0f;
    AuraPulseDuration = 1.0f;
    AuraPulseTimer = 0.0f;
    bIsPulsing = false;
    
    EnergyFlowDirection = FVector::ZeroVector;
    EnergyFlowSpeed = 1.0f;
    bEnergyFlowing = false;
}

void UConsciousnessAnimationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize animation data
    AnimationData.CurrentState = EConsciousnessState::Dormant;
    AnimationData.StateTransitionProgress = 0.0f;
    AnimationData.EnergyLevel = 0.5f;
    AnimationData.AuraScale = FVector(1.0f, 1.0f, 1.0f);
    AnimationData.LuminosityIntensity = 0.0f;
}

void UConsciousnessAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateStateTransition(DeltaTime);
    UpdateTransformation(DeltaTime);
    UpdateAuraPulse(DeltaTime);
    UpdateEnergyFlow(DeltaTime);
}

void UConsciousnessAnimationComponent::SetConsciousnessState(EConsciousnessState NewState, float TransitionDuration)
{
    if (NewState != AnimationData.CurrentState)
    {
        TargetState = NewState;
        StateTransitionDuration = TransitionDuration;
        StateTransitionTimer = 0.0f;
        bIsTransitioning = true;
        
        UE_LOG(LogTemp, Log, TEXT("Consciousness state transition started: %d -> %d"), 
               (int32)AnimationData.CurrentState, (int32)NewState);
    }
}

void UConsciousnessAnimationComponent::TriggerTransformation(ETransformationType TransformType, float Duration)
{
    CurrentTransformation = TransformType;
    TransformationDuration = Duration;
    TransformationTimer = 0.0f;
    bIsTransforming = true;
    
    UE_LOG(LogTemp, Log, TEXT("Transformation triggered: %d"), (int32)TransformType);
}

void UConsciousnessAnimationComponent::UpdateEnergyLevel(float NewEnergyLevel)
{
    AnimationData.EnergyLevel = FMath::Clamp(NewEnergyLevel, 0.0f, 1.0f);
    
    // Update luminosity based on energy level
    AnimationData.LuminosityIntensity = AnimationData.EnergyLevel * TransformationIntensity;
}

void UConsciousnessAnimationComponent::PulseAura(float Intensity, float Duration)
{
    AuraPulseIntensity = Intensity;
    AuraPulseDuration = Duration;
    AuraPulseTimer = 0.0f;
    bIsPulsing = true;
}

void UConsciousnessAnimationComponent::StartEnergyFlow(FVector Direction, float Speed)
{
    EnergyFlowDirection = Direction.GetSafeNormal();
    EnergyFlowSpeed = Speed;
    bEnergyFlowing = true;
}

void UConsciousnessAnimationComponent::StopEnergyFlow()
{
    bEnergyFlowing = false;
    EnergyFlowDirection = FVector::ZeroVector;
}

void UConsciousnessAnimationComponent::UpdateStateTransition(float DeltaTime)
{
    if (!bIsTransitioning) return;
    
    StateTransitionTimer += DeltaTime;
    float Progress = StateTransitionTimer / StateTransitionDuration;
    
    if (Progress >= 1.0f)
    {
        // Transition complete
        AnimationData.CurrentState = TargetState;
        AnimationData.StateTransitionProgress = 0.0f;
        bIsTransitioning = false;
        
        UE_LOG(LogTemp, Log, TEXT("Consciousness state transition completed: %d"), (int32)AnimationData.CurrentState);
    }
    else
    {
        // Update transition progress
        AnimationData.StateTransitionProgress = UKismetMathLibrary::Ease(0.0f, 1.0f, Progress, EEasingFunc::EaseInOut);
        
        // Calculate blend values for smooth transitions
        float BlendValue = CalculateStateBlendValue(AnimationData.CurrentState, TargetState, Progress);
        
        // Update luminosity based on consciousness state
        switch (TargetState)
        {
            case EConsciousnessState::Dormant:
                AnimationData.LuminosityIntensity = FMath::Lerp(AnimationData.LuminosityIntensity, 0.1f, BlendValue);
                break;
            case EConsciousnessState::Awakening:
                AnimationData.LuminosityIntensity = FMath::Lerp(AnimationData.LuminosityIntensity, 0.3f, BlendValue);
                break;
            case EConsciousnessState::Expanding:
                AnimationData.LuminosityIntensity = FMath::Lerp(AnimationData.LuminosityIntensity, 0.6f, BlendValue);
                break;
            case EConsciousnessState::Transcendent:
                AnimationData.LuminosityIntensity = FMath::Lerp(AnimationData.LuminosityIntensity, 0.8f, BlendValue);
                break;
            case EConsciousnessState::Unity:
                AnimationData.LuminosityIntensity = FMath::Lerp(AnimationData.LuminosityIntensity, 1.0f, BlendValue);
                break;
        }
    }
}

void UConsciousnessAnimationComponent::UpdateTransformation(float DeltaTime)
{
    if (!bIsTransforming) return;
    
    TransformationTimer += DeltaTime;
    float Progress = TransformationTimer / TransformationDuration;
    
    if (Progress >= 1.0f)
    {
        bIsTransforming = false;
        UE_LOG(LogTemp, Log, TEXT("Transformation completed: %d"), (int32)CurrentTransformation);
    }
    else
    {
        float IntensityMultiplier = UKismetMathLibrary::Ease(0.0f, 1.0f, Progress, EEasingFunc::EaseInOut);
        
        switch (CurrentTransformation)
        {
            case ETransformationType::Materialization:
                AnimationData.AuraScale = FVector::Lerp(FVector(0.1f), FVector(1.0f), IntensityMultiplier);
                break;
            case ETransformationType::Dematerialization:
                AnimationData.AuraScale = FVector::Lerp(FVector(1.0f), FVector(0.1f), IntensityMultiplier);
                break;
            case ETransformationType::EnergyShift:
                AnimationData.LuminosityIntensity = 0.5f + 0.5f * FMath::Sin(Progress * PI * 4.0f) * IntensityMultiplier;
                break;
            case ETransformationType::ConsciousnessExpansion:
                AnimationData.AuraScale = FVector::Lerp(FVector(1.0f), FVector(2.0f), IntensityMultiplier);
                AnimationData.LuminosityIntensity = FMath::Lerp(AnimationData.LuminosityIntensity, 1.0f, IntensityMultiplier);
                break;
        }
    }
}

void UConsciousnessAnimationComponent::UpdateAuraPulse(float DeltaTime)
{
    if (!bIsPulsing) return;
    
    AuraPulseTimer += DeltaTime;
    float Progress = AuraPulseTimer / AuraPulseDuration;
    
    if (Progress >= 1.0f)
    {
        bIsPulsing = false;
        AnimationData.AuraScale = FVector(1.0f, 1.0f, 1.0f);
    }
    else
    {
        float PulseValue = FMath::Sin(Progress * PI);
        float ScaleMultiplier = 1.0f + (AuraPulseIntensity * PulseValue);
        AnimationData.AuraScale = FVector(ScaleMultiplier, ScaleMultiplier, ScaleMultiplier);
    }
}

void UConsciousnessAnimationComponent::UpdateEnergyFlow(float DeltaTime)
{
    if (!bEnergyFlowing) return;
    
    // This would typically update particle systems or other VFX
    // For now, we update the energy level cyclically
    float FlowIntensity = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * EnergyFlowSpeed);
    UpdateEnergyLevel(FlowIntensity);
}

float UConsciousnessAnimationComponent::CalculateStateBlendValue(EConsciousnessState FromState, EConsciousnessState ToState, float Progress)
{
    // Apply easing based on the type of transition
    if ((int32)ToState > (int32)FromState)
    {
        // Ascending consciousness - smooth ease in
        return UKismetMathLibrary::Ease(0.0f, 1.0f, Progress, EEasingFunc::EaseIn);
    }
    else
    {
        // Descending consciousness - sharp ease out
        return UKismetMathLibrary::Ease(0.0f, 1.0f, Progress, EEasingFunc::EaseOut);
    }
}