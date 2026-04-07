#include "TranspersonalAudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

void UTranspersonalAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeAudioProfiles();
    
    UE_LOG(LogTemp, Log, TEXT("Transpersonal Audio Manager Initialized"));
}

void UTranspersonalAudioManager::Deinitialize()
{
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->Stop();
    }
    
    if (BinauralAudioComponent && IsValid(BinauralAudioComponent))
    {
        BinauralAudioComponent->Stop();
    }
    
    Super::Deinitialize();
}

void UTranspersonalAudioManager::InitializeAudioProfiles()
{
    // Ordinary Consciousness
    FAudioConsciousnessProfile& OrdinaryProfile = ConsciousnessAudioProfiles.Add(EConsciousnessState::Ordinary);
    OrdinaryProfile.ConsciousnessState = EConsciousnessState::Ordinary;
    OrdinaryProfile.BaseFrequency = 40.0f; // Beta waves
    OrdinaryProfile.ReverbWetness = 0.1f;
    OrdinaryProfile.LowPassFilterFreq = 20000.0f;
    OrdinaryProfile.SpatializationIntensity = 0.3f;

    // Expanded Awareness
    FAudioConsciousnessProfile& ExpandedProfile = ConsciousnessAudioProfiles.Add(EConsciousnessState::Expanded);
    ExpandedProfile.ConsciousnessState = EConsciousnessState::Expanded;
    ExpandedProfile.BaseFrequency = 10.0f; // Alpha waves
    ExpandedProfile.ReverbWetness = 0.4f;
    ExpandedProfile.LowPassFilterFreq = 15000.0f;
    ExpandedProfile.SpatializationIntensity = 0.6f;

    // Transcendent State
    FAudioConsciousnessProfile& TranscendentProfile = ConsciousnessAudioProfiles.Add(EConsciousnessState::Transcendent);
    TranscendentProfile.ConsciousnessState = EConsciousnessState::Transcendent;
    TranscendentProfile.BaseFrequency = 6.0f; // Theta waves
    TranscendentProfile.ReverbWetness = 0.7f;
    TranscendentProfile.LowPassFilterFreq = 10000.0f;
    TranscendentProfile.SpatializationIntensity = 0.8f;

    // Unity Consciousness
    FAudioConsciousnessProfile& UnityProfile = ConsciousnessAudioProfiles.Add(EConsciousnessState::Unity);
    UnityProfile.ConsciousnessState = EConsciousnessState::Unity;
    UnityProfile.BaseFrequency = 2.0f; // Delta waves
    UnityProfile.ReverbWetness = 0.9f;
    UnityProfile.LowPassFilterFreq = 5000.0f;
    UnityProfile.SpatializationIntensity = 1.0f;

    // Void State
    FAudioConsciousnessProfile& VoidProfile = ConsciousnessAudioProfiles.Add(EConsciousnessState::Void);
    VoidProfile.ConsciousnessState = EConsciousnessState::Void;
    VoidProfile.BaseFrequency = 0.5f; // Ultra-low frequency
    VoidProfile.ReverbWetness = 1.0f;
    VoidProfile.LowPassFilterFreq = 1000.0f;
    VoidProfile.SpatializationIntensity = 0.1f;
}

void UTranspersonalAudioManager::UpdateAudioForConsciousnessState(EConsciousnessState State, float Intensity)
{
    CurrentAudioState = State;
    CurrentIntensity = Intensity;

    if (FAudioConsciousnessProfile* Profile = ConsciousnessAudioProfiles.Find(State))
    {
        ApplyAudioProfile(*Profile, Intensity / 100.0f);
        UpdateBinauralFrequencies(State, Intensity);
    }
}

void UTranspersonalAudioManager::PlayBinauralBeats(float LeftFreq, float RightFreq, float Duration)
{
    // This would typically interface with MetaSound to generate binaural beats
    // For now, we'll log the frequencies that would be used
    
    UE_LOG(LogTemp, Log, TEXT("Playing Binaural Beats: Left=%.2fHz, Right=%.2fHz, Duration=%.1fs"), 
           LeftFreq, RightFreq, Duration);

    // In a full implementation, this would:
    // 1. Create or modify a MetaSound source with sine wave generators
    // 2. Set left channel to LeftFreq, right channel to RightFreq
    // 3. Apply appropriate volume and spatialization
}

void UTranspersonalAudioManager::SetAudioSpatializationMode(bool bEnable3DAudio, float IntensityMultiplier)
{
    // Enable/disable 3D audio spatialization based on consciousness state
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->bAllowSpatialization = bEnable3DAudio;
        
        if (bEnable3DAudio)
        {
            AmbientAudioComponent->AttenuationSettings = nullptr; // Use default 3D attenuation
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio Spatialization: %s, Intensity: %.2f"), 
           bEnable3DAudio ? TEXT("Enabled") : TEXT("Disabled"), IntensityMultiplier);
}

void UTranspersonalAudioManager::TriggerAudioTransition(EConsciousnessState FromState, EConsciousnessState ToState, float TransitionTime)
{
    // Smooth audio transition between consciousness states
    UE_LOG(LogTemp, Log, TEXT("Audio Transition: %s -> %s over %.1f seconds"), 
           *UEnum::GetValueAsString(FromState), 
           *UEnum::GetValueAsString(ToState), 
           TransitionTime);

    // In a full implementation, this would:
    // 1. Create interpolation timeline for audio parameters
    // 2. Gradually shift reverb, filtering, and binaural frequencies
    // 3. Cross-fade between different ambient soundscapes
}

void UTranspersonalAudioManager::ApplyAudioProfile(const FAudioConsciousnessProfile& Profile, float IntensityModifier)
{
    // Apply reverb settings
    float AdjustedReverb = Profile.ReverbWetness * IntensityModifier;
    
    // Apply filtering
    float AdjustedLowPass = FMath::Lerp(20000.0f, Profile.LowPassFilterFreq, IntensityModifier);
    
    // Apply spatialization
    float AdjustedSpatialization = Profile.SpatializationIntensity * IntensityModifier;
    
    SetAudioSpatializationMode(AdjustedSpatialization > 0.5f, AdjustedSpatialization);
    
    UE_LOG(LogTemp, Log, TEXT("Applied Audio Profile - State: %s, Reverb: %.2f, LowPass: %.0fHz, Spatial: %.2f"), 
           *UEnum::GetValueAsString(Profile.ConsciousnessState), 
           AdjustedReverb, 
           AdjustedLowPass, 
           AdjustedSpatialization);
}

void UTranspersonalAudioManager::UpdateBinauralFrequencies(EConsciousnessState State, float Intensity)
{
    float BaseFreq = GetBinauralFrequencyForState(State, Intensity);
    float BeatFrequency = 4.0f; // 4Hz binaural beat difference
    
    float LeftFreq = BaseFreq;
    float RightFreq = BaseFreq + BeatFrequency;
    
    PlayBinauralBeats(LeftFreq, RightFreq);
}

float UTranspersonalAudioManager::GetBinauralFrequencyForState(EConsciousnessState State, float Intensity)
{
    if (FAudioConsciousnessProfile* Profile = ConsciousnessAudioProfiles.Find(State))
    {
        // Modulate base frequency based on intensity
        float IntensityFactor = FMath::Clamp(Intensity / 100.0f, 0.1f, 2.0f);
        return Profile->BaseFrequency * IntensityFactor;
    }
    
    return 40.0f; // Default beta frequency
}