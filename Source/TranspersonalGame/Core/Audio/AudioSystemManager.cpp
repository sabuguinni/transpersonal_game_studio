// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "AudioSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioMixerBlueprintLibrary.h"
#include "TimerManager.h"

UAudioSystemManager::UAudioSystemManager()
{
    CurrentEnvironment = EAudioEnvironment::Forest_Dense;
    CurrentMusicState = EMusicEmotionalState::Calm_Exploration;
    MasterVolume = 1.0f;
    AmbientAudioComponent = nullptr;
    WindAudioComponent = nullptr;
    MusicAudioComponent = nullptr;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing prehistoric audio system"));
    
    InitializeAudioComponents();
    
    // Set up periodic updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            EnvironmentUpdateTimer,
            this,
            &UAudioSystemManager::UpdateEnvironmentalAudio,
            1.0f, // Update every second
            true
        );
        
        World->GetTimerManager().SetTimer(
            ComponentCleanupTimer,
            this,
            &UAudioSystemManager::CleanupInactiveComponents,
            10.0f, // Cleanup every 10 seconds
            true
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initialization complete"));
}

void UAudioSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Shutting down audio system"));
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(EnvironmentUpdateTimer);
        World->GetTimerManager().ClearTimer(ComponentCleanupTimer);
    }
    
    // Stop all active audio components
    for (auto& ComponentPair : ActiveAudioComponents)
    {
        if (ComponentPair.Value && IsValid(ComponentPair.Value))
        {
            ComponentPair.Value->Stop();
        }
    }
    ActiveAudioComponents.Empty();
    
    // Stop core components
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->Stop();
    }
    if (WindAudioComponent && IsValid(WindAudioComponent))
    {
        WindAudioComponent->Stop();
    }
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->Stop();
    }
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeAudioComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: No valid world for audio component creation"));
        return;
    }
    
    // Create ambient audio component
    AmbientAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(0.7f);
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Created ambient audio component"));
    }
    
    // Create wind audio component
    WindAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
    if (WindAudioComponent)
    {
        WindAudioComponent->SetVolumeMultiplier(0.5f);
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Created wind audio component"));
    }
    
    // Create music audio component
    MusicAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(0.8f);
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Created music audio component"));
    }
}

void UAudioSystemManager::SetAudioEnvironment(EAudioEnvironment NewEnvironment, float TransitionTime)
{
    if (CurrentEnvironment == NewEnvironment)
    {
        return;
    }
    
    EAudioEnvironment PreviousEnvironment = CurrentEnvironment;
    CurrentEnvironment = NewEnvironment;
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Transitioning environment from %d to %d over %.2f seconds"),
        (int32)PreviousEnvironment, (int32)NewEnvironment, TransitionTime);
    
    // Trigger environmental audio transition
    UpdateEnvironmentalAudio();
    
    // Broadcast state change
    OnAudioStateChanged.Broadcast(
        FString::Printf(TEXT("Environment_%d"), (int32)NewEnvironment),
        1.0f
    );
}

void UAudioSystemManager::SetMusicEmotionalState(EMusicEmotionalState NewState, float TransitionTime)
{
    if (CurrentMusicState == NewState)
    {
        return;
    }
    
    EMusicEmotionalState PreviousState = CurrentMusicState;
    CurrentMusicState = NewState;
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Transitioning music state from %d to %d over %.2f seconds"),
        (int32)PreviousState, (int32)NewState, TransitionTime);
    
    TransitionMusicLayers(NewState, TransitionTime);
    
    // Broadcast music transition
    OnMusicTransition.Broadcast(
        FString::Printf(TEXT("MusicState_%d"), (int32)NewState)
    );
}

void UAudioSystemManager::PlayDinosaurSound(const FString& DinosaurSpecies, const FString& SoundType, FVector Location)
{
    if (!DinosaurProfiles.Contains(DinosaurSpecies))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No audio profile found for dinosaur species: %s"), *DinosaurSpecies);
        return;
    }
    
    const FDinosaurAudioProfile& Profile = DinosaurProfiles[DinosaurSpecies];
    TArray<USoundCue*> SoundsToPlay;
    
    // Select appropriate sound array based on type
    if (SoundType == TEXT("Idle"))
    {
        SoundsToPlay = Profile.IdleSounds;
    }
    else if (SoundType == TEXT("Movement"))
    {
        SoundsToPlay = Profile.MovementSounds;
    }
    else if (SoundType == TEXT("Alert"))
    {
        SoundsToPlay = Profile.AlertSounds;
    }
    else if (SoundType == TEXT("Aggressive"))
    {
        SoundsToPlay = Profile.AggressiveSounds;
    }
    
    if (SoundsToPlay.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No sounds available for %s %s"), *DinosaurSpecies, *SoundType);
        return;
    }
    
    // Randomly select a sound
    int32 RandomIndex = FMath::RandRange(0, SoundsToPlay.Num() - 1);
    USoundCue* SelectedSound = SoundsToPlay[RandomIndex];
    
    if (!SelectedSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Selected sound is null for %s %s"), *DinosaurSpecies, *SoundType);
        return;
    }
    
    // Calculate volume and pitch variations
    float VolumeMultiplier = 1.0f + FMath::RandRange(-Profile.VolumeVariance, Profile.VolumeVariance);
    float PitchMultiplier = 1.0f + FMath::RandRange(-Profile.PitchVariance, Profile.PitchVariance);
    
    // Play the sound at the specified location
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        SelectedSound,
        Location,
        FRotator::ZeroRotator,
        VolumeMultiplier,
        PitchMultiplier,
        0.0f, // Start time
        Profile.AttenuationSettings
    );
    
    if (AudioComp)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing %s sound for %s at location %s"),
            *SoundType, *DinosaurSpecies, *Location.ToString());
    }
}

void UAudioSystemManager::RegisterDinosaurAudioProfile(const FString& SpeciesName, const FDinosaurAudioProfile& Profile)
{
    DinosaurProfiles.Add(SpeciesName, Profile);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered audio profile for species: %s"), *SpeciesName);
}

void UAudioSystemManager::UpdateWindIntensity(float Intensity)
{
    if (!WindAudioComponent || !IsValid(WindAudioComponent))
    {
        return;
    }
    
    float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    WindAudioComponent->SetVolumeMultiplier(ClampedIntensity * 0.6f);
    WindAudioComponent->SetPitchMultiplier(0.8f + (ClampedIntensity * 0.4f));
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("AudioSystemManager: Updated wind intensity to %.2f"), ClampedIntensity);
}

void UAudioSystemManager::UpdateTimeOfDay(float TimeNormalized)
{
    float ClampedTime = FMath::Clamp(TimeNormalized, 0.0f, 1.0f);
    
    // Adjust ambient volume based on time of day
    // Night time (0.0-0.25 and 0.75-1.0) should be quieter
    float AmbientVolume = 1.0f;
    if (ClampedTime < 0.25f || ClampedTime > 0.75f)
    {
        // Night time - reduce ambient volume
        AmbientVolume = 0.4f;
    }
    else if (ClampedTime > 0.4f && ClampedTime < 0.6f)
    {
        // Midday - full ambient volume
        AmbientVolume = 1.0f;
    }
    else
    {
        // Dawn/Dusk - moderate ambient volume
        AmbientVolume = 0.7f;
    }
    
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * 0.7f);
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("AudioSystemManager: Updated time of day to %.2f, ambient volume %.2f"),
        ClampedTime, AmbientVolume);
}

void UAudioSystemManager::TriggerWeatherTransition(const FString& WeatherType, float Intensity)
{
    float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Weather transition to %s with intensity %.2f"),
        *WeatherType, ClampedIntensity);
    
    // Adjust wind based on weather
    if (WeatherType == TEXT("Storm") || WeatherType == TEXT("Rain"))
    {
        UpdateWindIntensity(ClampedIntensity * 0.8f);
    }
    else if (WeatherType == TEXT("Calm"))
    {
        UpdateWindIntensity(0.2f);
    }
    
    // Broadcast weather change
    OnAudioStateChanged.Broadcast(
        FString::Printf(TEXT("Weather_%s"), *WeatherType),
        ClampedIntensity
    );
}

void UAudioSystemManager::AddMusicLayer(const FString& LayerName, const FAudioLayerConfig& LayerConfig)
{
    MusicLayers.Add(LayerName, LayerConfig);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Added music layer: %s"), *LayerName);
}

void UAudioSystemManager::SetMusicLayerVolume(const FString& LayerName, float Volume, float FadeTime)
{
    if (!MusicLayers.Contains(LayerName))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Music layer not found: %s"), *LayerName);
        return;
    }
    
    FAudioLayerConfig& LayerConfig = MusicLayers[LayerName];
    LayerConfig.Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Apply volume to active component if it exists
    if (ActiveAudioComponents.Contains(LayerName))
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[LayerName];
        if (AudioComp && IsValid(AudioComp))
        {
            AudioComp->SetVolumeMultiplier(LayerConfig.Volume);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Set music layer %s volume to %.2f"), *LayerName, Volume);
}

void UAudioSystemManager::EnableMusicLayer(const FString& LayerName, bool bEnable, float FadeTime)
{
    if (!MusicLayers.Contains(LayerName))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Music layer not found: %s"), *LayerName);
        return;
    }
    
    const FAudioLayerConfig& LayerConfig = MusicLayers[LayerName];
    
    if (bEnable)
    {
        // Start playing the layer
        if (!ActiveAudioComponents.Contains(LayerName) && LayerConfig.MetaSoundAsset)
        {
            UAudioComponent* AudioComp = UGameplayStatics::CreateSound2D(
                GetWorld(),
                LayerConfig.MetaSoundAsset
            );
            
            if (AudioComp)
            {
                AudioComp->SetVolumeMultiplier(LayerConfig.Volume);
                AudioComp->Play();
                ActiveAudioComponents.Add(LayerName, AudioComp);
                
                UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Enabled music layer: %s"), *LayerName);
            }
        }
    }
    else
    {
        // Stop playing the layer
        if (ActiveAudioComponents.Contains(LayerName))
        {
            UAudioComponent* AudioComp = ActiveAudioComponents[LayerName];
            if (AudioComp && IsValid(AudioComp))
            {
                AudioComp->Stop();
            }
            ActiveAudioComponents.Remove(LayerName);
            
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Disabled music layer: %s"), *LayerName);
        }
    }
}

void UAudioSystemManager::Play3DSound(USoundCue* SoundCue, FVector Location, float VolumeMultiplier)
{
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Attempted to play null sound cue"));
        return;
    }
    
    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        SoundCue,
        Location,
        VolumeMultiplier * MasterVolume
    );
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("AudioSystemManager: Played 3D sound at location %s"), *Location.ToString());
}

UAudioComponent* UAudioSystemManager::PlayLooping3DSound(USoundCue* SoundCue, FVector Location, float VolumeMultiplier)
{
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Attempted to play null looping sound cue"));
        return nullptr;
    }
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        SoundCue,
        Location,
        FRotator::ZeroRotator,
        VolumeMultiplier * MasterVolume
    );
    
    if (AudioComp)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Started looping 3D sound at location %s"), *Location.ToString());
    }
    
    return AudioComp;
}

void UAudioSystemManager::UpdateEnvironmentalAudio()
{
    // This method is called periodically to update environmental audio
    // Based on current environment, time of day, weather, etc.
    
    // Example implementation - in a real game this would be much more sophisticated
    switch (CurrentEnvironment)
    {
        case EAudioEnvironment::Forest_Dense:
            // Dense forest should have rich ambient sounds
            break;
        case EAudioEnvironment::Plains_Open:
            // Open plains should emphasize wind and distant sounds
            UpdateWindIntensity(0.6f);
            break;
        case EAudioEnvironment::Cave_Deep:
            // Deep caves should be very quiet with subtle reverb
            UpdateWindIntensity(0.1f);
            break;
        default:
            break;
    }
}

void UAudioSystemManager::TransitionMusicLayers(EMusicEmotionalState NewState, float TransitionTime)
{
    // This method handles transitioning between different music layers
    // based on the emotional state
    
    switch (NewState)
    {
        case EMusicEmotionalState::Calm_Exploration:
            EnableMusicLayer(TEXT("Ambient_Calm"), true, TransitionTime);
            EnableMusicLayer(TEXT("Tension_Low"), false, TransitionTime);
            EnableMusicLayer(TEXT("Danger_High"), false, TransitionTime);
            break;
            
        case EMusicEmotionalState::Tense_Awareness:
            EnableMusicLayer(TEXT("Ambient_Calm"), false, TransitionTime);
            EnableMusicLayer(TEXT("Tension_Low"), true, TransitionTime);
            EnableMusicLayer(TEXT("Danger_High"), false, TransitionTime);
            break;
            
        case EMusicEmotionalState::Fear_Immediate:
            EnableMusicLayer(TEXT("Ambient_Calm"), false, TransitionTime);
            EnableMusicLayer(TEXT("Tension_Low"), false, TransitionTime);
            EnableMusicLayer(TEXT("Danger_High"), true, TransitionTime);
            break;
            
        case EMusicEmotionalState::Silence_Suspense:
            // Fade out all music layers for suspenseful silence
            EnableMusicLayer(TEXT("Ambient_Calm"), false, TransitionTime);
            EnableMusicLayer(TEXT("Tension_Low"), false, TransitionTime);
            EnableMusicLayer(TEXT("Danger_High"), false, TransitionTime);
            break;
            
        default:
            break;
    }
}

void UAudioSystemManager::CleanupInactiveComponents()
{
    // Remove audio components that are no longer playing
    TArray<FString> ComponentsToRemove;
    
    for (auto& ComponentPair : ActiveAudioComponents)
    {
        UAudioComponent* AudioComp = ComponentPair.Value;
        if (!AudioComp || !IsValid(AudioComp) || !AudioComp->IsPlaying())
        {
            ComponentsToRemove.Add(ComponentPair.Key);
        }
    }
    
    for (const FString& ComponentKey : ComponentsToRemove)
    {
        ActiveAudioComponents.Remove(ComponentKey);
        UE_LOG(LogTemp, VeryVerbose, TEXT("AudioSystemManager: Cleaned up inactive audio component: %s"), *ComponentKey);
    }
}