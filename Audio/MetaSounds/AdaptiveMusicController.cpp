// AdaptiveMusicController.cpp
// Transpersonal Game Studio - Jurassic Survival Game
// Audio Agent Implementation - Adaptive Music System

#include "AdaptiveMusicController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioModulation.h"
#include "MetasoundSource.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY(LogAdaptiveMusic);

UAdaptiveMusicController::UAdaptiveMusicController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize emotional state
    CurrentEmotionalState = EEmotionalState::Calm_Exploration;
    TargetEmotionalState = EEmotionalState::Calm_Exploration;
    
    // Initialize transition parameters
    TransitionDuration = 2.0f;
    TransitionProgress = 0.0f;
    bIsTransitioning = false;
    
    // Initialize layer volumes
    BaseRhythmVolume = 1.0f;
    MelodyVolume = 0.8f;
    HarmonyVolume = 0.6f;
    TensionVolume = 0.0f;
    
    // Initialize threat detection
    ThreatLevel = 0.0f;
    PlayerFearLevel = 0.0f;
    LastDangerTime = 0.0f;
    
    // Performance settings
    MaxConcurrentLayers = 8;
    AudioUpdateRate = 10.0f;
}

void UAdaptiveMusicController::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize MetaSound parameters
    InitializeMetaSoundParameters();
    
    // Start with calm exploration music
    SetEmotionalState(EEmotionalState::Calm_Exploration, false);
    
    UE_LOG(LogAdaptiveMusic, Log, TEXT("Adaptive Music Controller initialized"));
}

void UAdaptiveMusicController::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update threat assessment
    UpdateThreatAssessment(DeltaTime);
    
    // Process emotional state transitions
    ProcessEmotionalTransition(DeltaTime);
    
    // Update MetaSound parameters
    UpdateMetaSoundParameters(DeltaTime);
    
    // Handle layer crossfading
    UpdateLayerVolumes(DeltaTime);
}

void UAdaptiveMusicController::SetEmotionalState(EEmotionalState NewState, bool bForceImmediate)
{
    if (NewState == CurrentEmotionalState && !bIsTransitioning)
    {
        return; // Already in this state
    }
    
    // Log state change for debugging
    UE_LOG(LogAdaptiveMusic, Log, TEXT("Emotional state changing from %s to %s"), 
        *GetEmotionalStateString(CurrentEmotionalState),
        *GetEmotionalStateString(NewState));
    
    TargetEmotionalState = NewState;
    
    if (bForceImmediate)
    {
        CurrentEmotionalState = NewState;
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
        ApplyEmotionalStateImmediate();
    }
    else
    {
        // Start smooth transition
        TransitionProgress = 0.0f;
        bIsTransitioning = true;
        TransitionDuration = GetTransitionDuration(CurrentEmotionalState, NewState);
    }
}

void UAdaptiveMusicController::UpdateThreatAssessment(float DeltaTime)
{
    // Get player reference
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    // Reset threat level
    float NewThreatLevel = 0.0f;
    
    // Check for nearby dinosaurs
    TArray<AActor*> NearbyDinosaurs;
    // This would integrate with the NPC Behavior Agent's dinosaur detection system
    // For now, we'll simulate threat detection
    
    // Simulate threat level based on game state
    // In real implementation, this would query the AI system
    float TimeSinceLastDanger = GetWorld()->GetTimeSeconds() - LastDangerTime;
    
    if (TimeSinceLastDanger < 5.0f)
    {
        NewThreatLevel = FMath::Max(0.8f, ThreatLevel);
    }
    else if (TimeSinceLastDanger < 15.0f)
    {
        NewThreatLevel = FMath::Lerp(ThreatLevel, 0.3f, DeltaTime * 0.5f);
    }
    else
    {
        NewThreatLevel = FMath::Lerp(ThreatLevel, 0.0f, DeltaTime * 0.2f);
    }
    
    ThreatLevel = FMath::Clamp(NewThreatLevel, 0.0f, 1.0f);
    
    // Determine appropriate emotional state based on threat level
    EEmotionalState NewTargetState = DetermineEmotionalStateFromThreat(ThreatLevel);
    
    if (NewTargetState != TargetEmotionalState)
    {
        SetEmotionalState(NewTargetState, false);
    }
}

EEmotionalState UAdaptiveMusicController::DetermineEmotionalStateFromThreat(float InThreatLevel)
{
    if (InThreatLevel >= 0.9f)
    {
        return EEmotionalState::Combat_Survival;
    }
    else if (InThreatLevel >= 0.7f)
    {
        return EEmotionalState::Immediate_Threat;
    }
    else if (InThreatLevel >= 0.4f)
    {
        return EEmotionalState::Active_Danger;
    }
    else if (InThreatLevel >= 0.2f)
    {
        return EEmotionalState::Tense_Awareness;
    }
    else if (InThreatLevel >= 0.1f)
    {
        return EEmotionalState::Calm_Exploration;
    }
    else
    {
        return EEmotionalState::Peaceful_Base;
    }
}

void UAdaptiveMusicController::ProcessEmotionalTransition(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }
    
    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        TransitionProgress = 1.0f;
        CurrentEmotionalState = TargetEmotionalState;
        bIsTransitioning = false;
        
        UE_LOG(LogAdaptiveMusic, Log, TEXT("Emotional state transition complete: %s"), 
            *GetEmotionalStateString(CurrentEmotionalState));
    }
    
    // Apply interpolated state during transition
    ApplyEmotionalStateTransition(TransitionProgress);
}

void UAdaptiveMusicController::UpdateMetaSoundParameters(float DeltaTime)
{
    if (!MetaSoundSource)
    {
        return;
    }
    
    // Update core parameters
    MetaSoundSource->SetFloatParameter(FName("ThreatLevel"), ThreatLevel);
    MetaSoundSource->SetFloatParameter(FName("PlayerFear"), PlayerFearLevel);
    MetaSoundSource->SetFloatParameter(FName("TransitionProgress"), TransitionProgress);
    
    // Update layer volumes
    MetaSoundSource->SetFloatParameter(FName("BaseRhythmVolume"), BaseRhythmVolume);
    MetaSoundSource->SetFloatParameter(FName("MelodyVolume"), MelodyVolume);
    MetaSoundSource->SetFloatParameter(FName("HarmonyVolume"), HarmonyVolume);
    MetaSoundSource->SetFloatParameter(FName("TensionVolume"), TensionVolume);
    
    // Update emotional state as integer
    MetaSoundSource->SetIntParameter(FName("EmotionalState"), (int32)CurrentEmotionalState);
}

void UAdaptiveMusicController::UpdateLayerVolumes(float DeltaTime)
{
    // Define target volumes based on current emotional state
    float TargetBaseRhythm = 1.0f;
    float TargetMelody = 0.0f;
    float TargetHarmony = 0.0f;
    float TargetTension = 0.0f;
    
    switch (CurrentEmotionalState)
    {
        case EEmotionalState::Calm_Exploration:
            TargetBaseRhythm = 0.6f;
            TargetMelody = 0.8f;
            TargetHarmony = 0.7f;
            TargetTension = 0.0f;
            break;
            
        case EEmotionalState::Tense_Awareness:
            TargetBaseRhythm = 0.8f;
            TargetMelody = 0.5f;
            TargetHarmony = 0.3f;
            TargetTension = 0.4f;
            break;
            
        case EEmotionalState::Active_Danger:
            TargetBaseRhythm = 1.0f;
            TargetMelody = 0.2f;
            TargetHarmony = 0.1f;
            TargetTension = 0.8f;
            break;
            
        case EEmotionalState::Immediate_Threat:
            TargetBaseRhythm = 1.2f;
            TargetMelody = 0.0f;
            TargetHarmony = 0.0f;
            TargetTension = 1.0f;
            break;
            
        case EEmotionalState::Combat_Survival:
            TargetBaseRhythm = 1.5f;
            TargetMelody = 0.0f;
            TargetHarmony = 0.0f;
            TargetTension = 1.2f;
            break;
            
        case EEmotionalState::Post_Trauma:
            TargetBaseRhythm = 0.3f;
            TargetMelody = 0.1f;
            TargetHarmony = 0.0f;
            TargetTension = 0.2f;
            break;
            
        case EEmotionalState::Peaceful_Base:
            TargetBaseRhythm = 0.5f;
            TargetMelody = 0.9f;
            TargetHarmony = 0.8f;
            TargetTension = 0.0f;
            break;
            
        case EEmotionalState::Discovery_Wonder:
            TargetBaseRhythm = 0.7f;
            TargetMelody = 1.0f;
            TargetHarmony = 0.9f;
            TargetTension = 0.0f;
            break;
    }
    
    // Smooth interpolation to target volumes
    float LerpSpeed = 2.0f; // Adjust for faster/slower transitions
    
    BaseRhythmVolume = FMath::FInterpTo(BaseRhythmVolume, TargetBaseRhythm, DeltaTime, LerpSpeed);
    MelodyVolume = FMath::FInterpTo(MelodyVolume, TargetMelody, DeltaTime, LerpSpeed);
    HarmonyVolume = FMath::FInterpTo(HarmonyVolume, TargetHarmony, DeltaTime, LerpSpeed);
    TensionVolume = FMath::FInterpTo(TensionVolume, TargetTension, DeltaTime, LerpSpeed);
}

float UAdaptiveMusicController::GetTransitionDuration(EEmotionalState FromState, EEmotionalState ToState)
{
    // Define transition durations based on emotional distance
    // Calm to danger should be quick, danger to calm should be slow
    
    if (FromState == EEmotionalState::Calm_Exploration && ToState == EEmotionalState::Immediate_Threat)
    {
        return 0.5f; // Very quick for sudden danger
    }
    else if (FromState == EEmotionalState::Combat_Survival && ToState == EEmotionalState::Calm_Exploration)
    {
        return 8.0f; // Slow recovery from combat
    }
    else if (FromState == EEmotionalState::Immediate_Threat && ToState == EEmotionalState::Post_Trauma)
    {
        return 1.0f; // Quick transition to trauma state
    }
    
    // Default transition duration
    return 3.0f;
}

void UAdaptiveMusicController::InitializeMetaSoundParameters()
{
    // This would be called to set up the MetaSound source with initial parameters
    // In a real implementation, this would load and configure the MetaSound asset
    
    UE_LOG(LogAdaptiveMusic, Log, TEXT("MetaSound parameters initialized"));
}

void UAdaptiveMusicController::ApplyEmotionalStateImmediate()
{
    // Immediately apply the emotional state without transition
    UpdateLayerVolumes(0.0f);
}

void UAdaptiveMusicController::ApplyEmotionalStateTransition(float Progress)
{
    // Apply interpolated emotional state during transition
    // This could include more complex blending logic
}

FString UAdaptiveMusicController::GetEmotionalStateString(EEmotionalState State)
{
    switch (State)
    {
        case EEmotionalState::Calm_Exploration: return TEXT("Calm_Exploration");
        case EEmotionalState::Tense_Awareness: return TEXT("Tense_Awareness");
        case EEmotionalState::Active_Danger: return TEXT("Active_Danger");
        case EEmotionalState::Immediate_Threat: return TEXT("Immediate_Threat");
        case EEmotionalState::Combat_Survival: return TEXT("Combat_Survival");
        case EEmotionalState::Post_Trauma: return TEXT("Post_Trauma");
        case EEmotionalState::Peaceful_Base: return TEXT("Peaceful_Base");
        case EEmotionalState::Discovery_Wonder: return TEXT("Discovery_Wonder");
        default: return TEXT("Unknown");
    }
}

// External trigger functions for other systems to call
void UAdaptiveMusicController::OnDinosaurSpotted(float ThreatRating)
{
    ThreatLevel = FMath::Max(ThreatLevel, ThreatRating);
    LastDangerTime = GetWorld()->GetTimeSeconds();
}

void UAdaptiveMusicController::OnPlayerEnterSafeZone()
{
    SetEmotionalState(EEmotionalState::Peaceful_Base, false);
}

void UAdaptiveMusicController::OnCombatStart()
{
    SetEmotionalState(EEmotionalState::Combat_Survival, true);
}

void UAdaptiveMusicController::OnCombatEnd()
{
    SetEmotionalState(EEmotionalState::Post_Trauma, false);
}

void UAdaptiveMusicController::OnDiscoveryMoment()
{
    SetEmotionalState(EEmotionalState::Discovery_Wonder, false);
}