#include "ConsciousnessAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Curves/CurveFloat.h"

UConsciousnessAnimationController::UConsciousnessAnimationController()
{
    // Initialize default values
    Speed = 0.0f;
    bIsMoving = false;
    Direction = 0.0f;
    ConsciousnessLevel = 0.5f;
    MeditationDepth = 0.0f;
    ShadowIntegration = 0.0f;
    
    BreathingCycle = 0.0f;
    BreathingAmplitude = 1.0f;
    bIsBreathing = true;
    
    EnergyFlowIntensity = 0.3f;
    EnergyDirection = FVector::UpVector;
    
    AuraIntensity = 0.5f;
    AuraColor = FLinearColor::White;
    ParticleEmissionRate = 10.0f;
    
    // Internal state
    PreviousState = EConsciousnessState::Neutral;
    StateTransitionTimer = 0.0f;
    StateTransitionDuration = 1.0f;
    
    BreathingTimer = 0.0f;
    InhaleDuration = 4.0f;
    ExhaleDuration = 6.0f;
    bInhaling = true;
}

void UConsciousnessAnimationController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Initialize animation data
    AnimationData.CurrentState = EConsciousnessState::Neutral;
    AnimationData.Intensity = EEmotionalIntensity::Low;
    AnimationData.TransitionSpeed = 1.0f;
    AnimationData.AuraOpacity = 0.5f;
    AnimationData.EnergyFlowDirection = FVector::UpVector;
    
    // Start breathing cycle
    StartBreathingCycle();
}

void UConsciousnessAnimationController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    // Get movement data from character
    if (APawn* OwningPawn = TryGetPawnOwner())
    {
        FVector Velocity = OwningPawn->GetVelocity();
        Speed = Velocity.Size();
        bIsMoving = Speed > 3.0f;
        
        if (bIsMoving)
        {
            Direction = UKismetMathLibrary::CalculateDirection(Velocity, OwningPawn->GetActorRotation()).Yaw;
        }
    }
    
    // Update animation systems
    UpdateBreathingAnimation(DeltaTimeX);
    UpdateEnergyFlow(DeltaTimeX);
    UpdateStateTransition(DeltaTimeX);
    UpdateAuraEffects();
}

void UConsciousnessAnimationController::SetConsciousnessState(EConsciousnessState NewState, float TransitionDuration)
{
    if (AnimationData.CurrentState != NewState)
    {
        PreviousState = AnimationData.CurrentState;
        AnimationData.CurrentState = NewState;
        StateTransitionTimer = 0.0f;
        StateTransitionDuration = TransitionDuration;
        
        // Update consciousness level based on state
        switch (NewState)
        {
            case EConsciousnessState::Neutral:
                ConsciousnessLevel = 0.3f;
                break;
            case EConsciousnessState::Meditative:
                ConsciousnessLevel = 0.6f;
                MeditationDepth = 0.7f;
                break;
            case EConsciousnessState::Transcendent:
                ConsciousnessLevel = 0.9f;
                MeditationDepth = 1.0f;
                break;
            case EConsciousnessState::Shadow:
                ConsciousnessLevel = 0.2f;
                ShadowIntegration = 0.8f;
                break;
            case EConsciousnessState::Integrated:
                ConsciousnessLevel = 0.8f;
                ShadowIntegration = 0.5f;
                break;
            case EConsciousnessState::Transforming:
                ConsciousnessLevel = 0.7f;
                break;
        }
    }
}

void UConsciousnessAnimationController::SetEmotionalIntensity(EEmotionalIntensity NewIntensity)
{
    AnimationData.Intensity = NewIntensity;
    
    // Adjust breathing and energy flow based on intensity
    switch (NewIntensity)
    {
        case EEmotionalIntensity::Low:
            BreathingAmplitude = 0.5f;
            EnergyFlowIntensity = 0.2f;
            break;
        case EEmotionalIntensity::Medium:
            BreathingAmplitude = 0.8f;
            EnergyFlowIntensity = 0.5f;
            break;
        case EEmotionalIntensity::High:
            BreathingAmplitude = 1.2f;
            EnergyFlowIntensity = 0.8f;
            break;
        case EEmotionalIntensity::Peak:
            BreathingAmplitude = 1.5f;
            EnergyFlowIntensity = 1.0f;
            break;
    }
}

void UConsciousnessAnimationController::TriggerTransformation(EConsciousnessState TargetState)
{
    // Set transforming state first
    SetConsciousnessState(EConsciousnessState::Transforming, 0.5f);
    
    // Schedule transition to target state
    FTimerHandle TransformationTimer;
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            TransformationTimer,
            [this, TargetState]()
            {
                SetConsciousnessState(TargetState, 1.5f);
            },
            0.5f,
            false
        );
    }
}

void UConsciousnessAnimationController::StartBreathingCycle(float InhaleTime, float ExhaleTime)
{
    InhaleDuration = InhaleTime;
    ExhaleDuration = ExhaleTime;
    BreathingTimer = 0.0f;
    bInhaling = true;
    bIsBreathing = true;
}

void UConsciousnessAnimationController::SetEnergyFlowDirection(FVector Direction)
{
    EnergyDirection = Direction.GetSafeNormal();
    AnimationData.EnergyFlowDirection = EnergyDirection;
}

void UConsciousnessAnimationController::UpdateBreathingAnimation(float DeltaTime)
{
    if (!bIsBreathing) return;
    
    BreathingTimer += DeltaTime;
    
    float CycleDuration = bInhaling ? InhaleDuration : ExhaleDuration;
    float CycleProgress = BreathingTimer / CycleDuration;
    
    if (CycleProgress >= 1.0f)
    {
        bInhaling = !bInhaling;
        BreathingTimer = 0.0f;
        CycleProgress = 0.0f;
    }
    
    // Create smooth breathing curve
    float BreathValue = bInhaling ? 
        UKismetMathLibrary::Ease(0.0f, 1.0f, CycleProgress, EEasingFunc::EaseInOut) :
        UKismetMathLibrary::Ease(1.0f, 0.0f, CycleProgress, EEasingFunc::EaseInOut);
    
    BreathingCycle = BreathValue * BreathingAmplitude;
}

void UConsciousnessAnimationController::UpdateEnergyFlow(float DeltaTime)
{
    // Create flowing energy pattern
    float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    float FlowPattern = FMath::Sin(TimeSeconds * 2.0f) * 0.5f + 0.5f;
    
    EnergyFlowIntensity = FMath::Lerp(0.2f, 1.0f, FlowPattern);
    
    // Modulate based on consciousness state
    float StateMultiplier = GetStateIntensity(AnimationData.CurrentState, AnimationData.Intensity);
    EnergyFlowIntensity *= StateMultiplier;
}

void UConsciousnessAnimationController::UpdateStateTransition(float DeltaTime)
{
    if (StateTransitionTimer < StateTransitionDuration)
    {
        StateTransitionTimer += DeltaTime;
        float TransitionProgress = StateTransitionTimer / StateTransitionDuration;
        
        // Smooth transition curve
        float EasedProgress = UKismetMathLibrary::Ease(0.0f, 1.0f, TransitionProgress, EEasingFunc::EaseInOut);
        
        // Interpolate consciousness level during transition
        float PreviousLevel = GetStateIntensity(PreviousState, AnimationData.Intensity);
        float CurrentLevel = GetStateIntensity(AnimationData.CurrentState, AnimationData.Intensity);
        ConsciousnessLevel = FMath::Lerp(PreviousLevel, CurrentLevel, EasedProgress);
    }
}

void UConsciousnessAnimationController::UpdateAuraEffects()
{
    // Update aura color based on state
    AuraColor = GetStateColor(AnimationData.CurrentState);
    
    // Update aura intensity based on consciousness level and breathing
    float BaseIntensity = ConsciousnessLevel;
    float BreathingModulation = BreathingCycle * 0.3f;
    AuraIntensity = FMath::Clamp(BaseIntensity + BreathingModulation, 0.1f, 1.0f);
    
    // Update particle emission rate
    ParticleEmissionRate = AuraIntensity * 50.0f;
    
    // Store in animation data
    AnimationData.AuraOpacity = AuraIntensity;
}

FLinearColor UConsciousnessAnimationController::GetStateColor(EConsciousnessState State)
{
    switch (State)
    {
        case EConsciousnessState::Neutral:
            return FLinearColor::White;
        case EConsciousnessState::Meditative:
            return FLinearColor(0.2f, 0.6f, 1.0f, 1.0f); // Blue
        case EConsciousnessState::Transcendent:
            return FLinearColor(1.0f, 0.8f, 0.2f, 1.0f); // Golden
        case EConsciousnessState::Shadow:
            return FLinearColor(0.6f, 0.2f, 0.8f, 1.0f); // Purple
        case EConsciousnessState::Integrated:
            return FLinearColor(0.2f, 0.8f, 0.4f, 1.0f); // Green
        case EConsciousnessState::Transforming:
            return FLinearColor(1.0f, 0.4f, 0.6f, 1.0f); // Pink
        default:
            return FLinearColor::White;
    }
}

float UConsciousnessAnimationController::GetStateIntensity(EConsciousnessState State, EEmotionalIntensity Intensity)
{
    float BaseIntensity = 0.5f;
    
    switch (State)
    {
        case EConsciousnessState::Neutral:
            BaseIntensity = 0.3f;
            break;
        case EConsciousnessState::Meditative:
            BaseIntensity = 0.6f;
            break;
        case EConsciousnessState::Transcendent:
            BaseIntensity = 0.9f;
            break;
        case EConsciousnessState::Shadow:
            BaseIntensity = 0.4f;
            break;
        case EConsciousnessState::Integrated:
            BaseIntensity = 0.8f;
            break;
        case EConsciousnessState::Transforming:
            BaseIntensity = 0.7f;
            break;
    }
    
    // Apply intensity multiplier
    float IntensityMultiplier = 1.0f;
    switch (Intensity)
    {
        case EEmotionalIntensity::Low:
            IntensityMultiplier = 0.7f;
            break;
        case EEmotionalIntensity::Medium:
            IntensityMultiplier = 1.0f;
            break;
        case EEmotionalIntensity::High:
            IntensityMultiplier = 1.3f;
            break;
        case EEmotionalIntensity::Peak:
            IntensityMultiplier = 1.6f;
            break;
    }
    
    return FMath::Clamp(BaseIntensity * IntensityMultiplier, 0.1f, 1.0f);
}