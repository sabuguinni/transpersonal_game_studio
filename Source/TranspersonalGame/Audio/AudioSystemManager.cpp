#include "AudioSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

UAudioSystemManager::UAudioSystemManager()
{
    // Initialize default values
    CurrentSoundscapeConfig = FAudio_SoundscapeConfig();
    CurrentMusicState = FAudio_MusicState();
    CurrentEnvironment = EAudio_EnvironmentType::Forest;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initializing audio subsystem"));
    
    // Initialize audio system
    InitializeAudioSystem();
    
    // Start consciousness update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ConsciousnessUpdateTimer,
            this,
            &UAudioSystemManager::ProcessConsciousnessInfluence,
            0.1f, // Update every 100ms
            true
        );
    }
}

void UAudioSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Deinitializing audio subsystem"));
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MusicTransitionTimer);
        World->GetTimerManager().ClearTimer(ConsciousnessUpdateTimer);
    }
    
    // Clean up audio components
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Stop();
        AmbientAudioComponent = nullptr;
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->Stop();
        MusicAudioComponent = nullptr;
    }
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->Stop();
        SFXAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeAudioSystem()
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initializing core audio components"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No valid world found"));
        return;
    }
    
    // Create audio components
    if (!AmbientAudioComponent)
    {
        AmbientAudioComponent = NewObject<UAudioComponent>(this);
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->SetVolumeMultiplier(CurrentSoundscapeConfig.AmbientVolume);
            AmbientAudioComponent->bAutoActivate = false;
        }
    }
    
    if (!MusicAudioComponent)
    {
        MusicAudioComponent = NewObject<UAudioComponent>(this);
        if (MusicAudioComponent)
        {
            MusicAudioComponent->SetVolumeMultiplier(CurrentSoundscapeConfig.MusicVolume);
            MusicAudioComponent->bAutoActivate = false;
        }
    }
    
    if (!SFXAudioComponent)
    {
        SFXAudioComponent = NewObject<UAudioComponent>(this);
        if (SFXAudioComponent)
        {
            SFXAudioComponent->SetVolumeMultiplier(CurrentSoundscapeConfig.SFXVolume);
            SFXAudioComponent->bAutoActivate = false;
        }
    }
    
    // Set initial environment
    SetEnvironmentType(EAudio_EnvironmentType::Forest);
}

void UAudioSystemManager::SetSoundscapeConfig(const FAudio_SoundscapeConfig& Config)
{
    CurrentSoundscapeConfig = Config;
    
    // Update audio component volumes
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(Config.AmbientVolume);
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(Config.MusicVolume);
    }
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->SetVolumeMultiplier(Config.SFXVolume);
    }
    
    // Update environment
    SetEnvironmentType(Config.EnvironmentType);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Soundscape config updated"));
}

void UAudioSystemManager::UpdateMusicState(const FAudio_MusicState& NewState)
{
    CurrentMusicState = NewState;
    UpdateMusicAudio();
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Music state updated - Layer: %d, Intensity: %f"), 
           (int32)NewState.CurrentLayer, NewState.Intensity);
}

void UAudioSystemManager::TransitionToMusicLayer(EAudio_MusicLayer NewLayer, float TransitionTime)
{
    if (CurrentMusicState.CurrentLayer == NewLayer)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Transitioning to music layer %d over %f seconds"), 
           (int32)NewLayer, TransitionTime);
    
    CurrentMusicState.CurrentLayer = NewLayer;
    
    // Start transition timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MusicTransitionTimer,
            [this]()
            {
                UpdateMusicAudio();
            },
            TransitionTime,
            false
        );
    }
}

void UAudioSystemManager::SetMusicIntensity(float Intensity)
{
    CurrentMusicState.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    UpdateMusicAudio();
}

void UAudioSystemManager::SetEmotionalState(float EmotionalValue)
{
    CurrentMusicState.EmotionalState = FMath::Clamp(EmotionalValue, -1.0f, 1.0f);
    UpdateMusicAudio();
}

void UAudioSystemManager::SetSpiritualResonance(float ResonanceValue)
{
    CurrentMusicState.SpiritualResonance = FMath::Clamp(ResonanceValue, 0.0f, 1.0f);
    UpdateMusicAudio();
}

void UAudioSystemManager::SetEnvironmentType(EAudio_EnvironmentType NewEnvironment)
{
    if (CurrentEnvironment == NewEnvironment)
    {
        return;
    }
    
    CurrentEnvironment = NewEnvironment;
    UpdateAmbientAudio();
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Environment changed to %d"), (int32)NewEnvironment);
}

void UAudioSystemManager::PlayEnvironmentSound(const FString& SoundName, FVector Location, float Volume)
{
    if (!SFXAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No SFX audio component available"));
        return;
    }
    
    // Play 3D sound at location
    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        nullptr, // Would load sound by name in real implementation
        Location,
        Volume * CurrentSoundscapeConfig.SFXVolume
    );
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing environment sound '%s' at location %s"), 
           *SoundName, *Location.ToString());
}

void UAudioSystemManager::TriggerConsciousnessShift(float ShiftIntensity)
{
    float ClampedIntensity = FMath::Clamp(ShiftIntensity, 0.0f, 1.0f);
    
    // Influence spiritual resonance
    CurrentMusicState.SpiritualResonance += ClampedIntensity * 0.3f;
    CurrentMusicState.SpiritualResonance = FMath::Clamp(CurrentMusicState.SpiritualResonance, 0.0f, 1.0f);
    
    // Trigger special consciousness sound effect
    PlaySacredSound(TEXT("ConsciousnessShift"), ClampedIntensity);
    
    UpdateMusicAudio();
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Consciousness shift triggered with intensity %f"), ClampedIntensity);
}

void UAudioSystemManager::PlaySacredSound(const FString& SoundName, float SpiritualPower)
{
    if (!SFXAudioComponent)
    {
        return;
    }
    
    float Volume = SpiritualPower * CurrentSoundscapeConfig.SFXVolume;
    
    // In real implementation, would load and play sacred sound from SacredSounds map
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing sacred sound '%s' with spiritual power %f"), 
           *SoundName, SpiritualPower);
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    float ClampedVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Apply to all audio components
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(CurrentSoundscapeConfig.AmbientVolume * ClampedVolume);
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(CurrentSoundscapeConfig.MusicVolume * ClampedVolume);
    }
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->SetVolumeMultiplier(CurrentSoundscapeConfig.SFXVolume * ClampedVolume);
    }
}

void UAudioSystemManager::SetAmbientVolume(float Volume)
{
    CurrentSoundscapeConfig.AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(CurrentSoundscapeConfig.AmbientVolume);
    }
}

void UAudioSystemManager::SetMusicVolume(float Volume)
{
    CurrentSoundscapeConfig.MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(CurrentSoundscapeConfig.MusicVolume);
    }
}

void UAudioSystemManager::SetSFXVolume(float Volume)
{
    CurrentSoundscapeConfig.SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->SetVolumeMultiplier(CurrentSoundscapeConfig.SFXVolume);
    }
}

void UAudioSystemManager::UpdateAmbientAudio()
{
    if (!AmbientAudioComponent)
    {
        return;
    }
    
    // In real implementation, would switch ambient sounds based on environment
    switch (CurrentEnvironment)
    {
        case EAudio_EnvironmentType::Forest:
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Updating ambient audio for Forest environment"));
            break;
        case EAudio_EnvironmentType::Cave:
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Updating ambient audio for Cave environment"));
            break;
        case EAudio_EnvironmentType::Plains:
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Updating ambient audio for Plains environment"));
            break;
        case EAudio_EnvironmentType::Sacred:
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Updating ambient audio for Sacred environment"));
            break;
    }
}

void UAudioSystemManager::UpdateMusicAudio()
{
    if (!MusicAudioComponent || !CurrentSoundscapeConfig.bEnableAdaptiveMusic)
    {
        return;
    }
    
    // Calculate final music parameters based on state
    float FinalVolume = CurrentSoundscapeConfig.MusicVolume * CurrentMusicState.Intensity;
    
    // Apply spiritual resonance influence
    if (CurrentMusicState.SpiritualResonance > 0.5f)
    {
        FinalVolume *= (1.0f + CurrentMusicState.SpiritualResonance * 0.3f);
    }
    
    MusicAudioComponent->SetVolumeMultiplier(FinalVolume);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Music updated - Layer: %d, Volume: %f, Spiritual: %f"), 
           (int32)CurrentMusicState.CurrentLayer, FinalVolume, CurrentMusicState.SpiritualResonance);
}

void UAudioSystemManager::ProcessConsciousnessInfluence()
{
    if (!CurrentSoundscapeConfig.bEnableAdaptiveMusic)
    {
        return;
    }
    
    // Gradually decay spiritual resonance
    if (CurrentMusicState.SpiritualResonance > 0.0f)
    {
        CurrentMusicState.SpiritualResonance -= 0.01f; // Decay rate
        CurrentMusicState.SpiritualResonance = FMath::Max(0.0f, CurrentMusicState.SpiritualResonance);
    }
    
    // Apply consciousness influence to music
    float ConsciousnessEffect = CurrentSoundscapeConfig.ConsciousnessInfluence * CurrentMusicState.SpiritualResonance;
    
    if (ConsciousnessEffect > 0.1f)
    {
        // Consciousness is influencing the audio
        UpdateMusicAudio();
    }
}