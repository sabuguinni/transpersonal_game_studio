#include "Audio_SoundManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    bIsDynamicMusicActive = false;
    CurrentTimeOfDay = 0.5f; // Start at noon
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Initializing audio subsystem"));
    
    InitializeAudioComponents();
    LoadDefaultSounds();
    
    // Start dynamic music system
    StartDynamicMusic();
}

void UAudio_SoundManager::Deinitialize()
{
    // Clean up timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MusicUpdateTimer);
        World->GetTimerManager().ClearTimer(BiomeTransitionTimer);
    }
    
    // Stop all audio components
    if (BiomeAudioComponent && IsValid(BiomeAudioComponent))
    {
        BiomeAudioComponent->Stop();
    }
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->Stop();
    }
    if (ThreatAudioComponent && IsValid(ThreatAudioComponent))
    {
        ThreatAudioComponent->Stop();
    }
    if (NarrationAudioComponent && IsValid(NarrationAudioComponent))
    {
        NarrationAudioComponent->Stop();
    }
    if (WeatherAudioComponent && IsValid(WeatherAudioComponent))
    {
        WeatherAudioComponent->Stop();
    }
    
    Super::Deinitialize();
}

void UAudio_SoundManager::InitializeAudioComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: No valid world for audio components"));
        return;
    }
    
    // Create audio components
    BiomeAudioComponent = NewObject<UAudioComponent>(this, TEXT("BiomeAudioComponent"));
    if (BiomeAudioComponent)
    {
        BiomeAudioComponent->bAutoActivate = false;
        BiomeAudioComponent->SetVolumeMultiplier(0.7f);
    }
    
    MusicAudioComponent = NewObject<UAudioComponent>(this, TEXT("MusicAudioComponent"));
    if (MusicAudioComponent)
    {
        MusicAudioComponent->bAutoActivate = false;
        MusicAudioComponent->SetVolumeMultiplier(0.5f);
    }
    
    ThreatAudioComponent = NewObject<UAudioComponent>(this, TEXT("ThreatAudioComponent"));
    if (ThreatAudioComponent)
    {
        ThreatAudioComponent->bAutoActivate = false;
        ThreatAudioComponent->SetVolumeMultiplier(0.8f);
    }
    
    NarrationAudioComponent = NewObject<UAudioComponent>(this, TEXT("NarrationAudioComponent"));
    if (NarrationAudioComponent)
    {
        NarrationAudioComponent->bAutoActivate = false;
        NarrationAudioComponent->SetVolumeMultiplier(1.0f);
    }
    
    WeatherAudioComponent = NewObject<UAudioComponent>(this, TEXT("WeatherAudioComponent"));
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->bAutoActivate = false;
        WeatherAudioComponent->SetVolumeMultiplier(0.6f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Audio components initialized"));
}

void UAudio_SoundManager::LoadDefaultSounds()
{
    // Initialize default biome settings
    FAudio_BiomeSettings ForestSettings;
    ForestSettings.BaseVolume = 0.7f;
    ForestSettings.FadeInTime = 2.0f;
    ForestSettings.FadeOutTime = 3.0f;
    BiomeSettings.Add(EAudio_BiomeType::Forest, ForestSettings);
    
    FAudio_BiomeSettings PlainsSettings;
    PlainsSettings.BaseVolume = 0.6f;
    PlainsSettings.FadeInTime = 3.0f;
    PlainsSettings.FadeOutTime = 2.0f;
    BiomeSettings.Add(EAudio_BiomeType::Plains, PlainsSettings);
    
    FAudio_BiomeSettings RiverSettings;
    RiverSettings.BaseVolume = 0.8f;
    RiverSettings.FadeInTime = 1.5f;
    RiverSettings.FadeOutTime = 4.0f;
    BiomeSettings.Add(EAudio_BiomeType::River, RiverSettings);
    
    // Initialize threat settings
    FAudio_ThreatSettings SafeSettings;
    SafeSettings.IntensityMultiplier = 1.0f;
    SafeSettings.HeartbeatVolume = 0.0f;
    ThreatSettings.Add(EAudio_ThreatLevel::Safe, SafeSettings);
    
    FAudio_ThreatSettings DangerSettings;
    DangerSettings.IntensityMultiplier = 1.8f;
    DangerSettings.HeartbeatVolume = 0.6f;
    ThreatSettings.Add(EAudio_ThreatLevel::Danger, DangerSettings);
    
    FAudio_ThreatSettings CombatSettings;
    CombatSettings.IntensityMultiplier = 2.5f;
    CombatSettings.HeartbeatVolume = 1.0f;
    ThreatSettings.Add(EAudio_ThreatLevel::Combat, CombatSettings);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Default sound settings loaded"));
}

void UAudio_SoundManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome == NewBiome)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Changing biome from %d to %d"), 
           static_cast<int32>(CurrentBiome), static_cast<int32>(NewBiome));
    
    EAudio_BiomeType OldBiome = CurrentBiome;
    CurrentBiome = NewBiome;
    
    // Start crossfade transition
    CrossfadeBiomeAudio(OldBiome, NewBiome, 3.0f);
}

void UAudio_SoundManager::TransitionToBiome(EAudio_BiomeType TargetBiome, float TransitionTime)
{
    if (CurrentBiome == TargetBiome)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Transitioning to biome %d over %.2f seconds"), 
           static_cast<int32>(TargetBiome), TransitionTime);
    
    EAudio_BiomeType OldBiome = CurrentBiome;
    CurrentBiome = TargetBiome;
    
    CrossfadeBiomeAudio(OldBiome, TargetBiome, TransitionTime);
}

void UAudio_SoundManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel == NewThreatLevel)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Threat level changed from %d to %d"), 
           static_cast<int32>(CurrentThreatLevel), static_cast<int32>(NewThreatLevel));
    
    CurrentThreatLevel = NewThreatLevel;
    
    // Trigger appropriate threat audio
    if (NewThreatLevel != EAudio_ThreatLevel::Safe)
    {
        TriggerThreatStinger(NewThreatLevel);
    }
    
    // Update music intensity
    UpdateDynamicMusic();
}

void UAudio_SoundManager::TriggerThreatStinger(EAudio_ThreatLevel ThreatType)
{
    if (!ThreatAudioComponent || !IsValid(ThreatAudioComponent))
    {
        return;
    }
    
    const FAudio_ThreatSettings* Settings = ThreatSettings.Find(ThreatType);
    if (!Settings)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Triggering threat stinger for level %d"), 
           static_cast<int32>(ThreatType));
    
    // Stop current threat audio
    ThreatAudioComponent->Stop();
    
    // Play threat stinger if available
    if (Settings->ThreatStinger.IsValid())
    {
        USoundCue* StingerSound = Settings->ThreatStinger.LoadSynchronous();
        if (StingerSound)
        {
            ThreatAudioComponent->SetSound(StingerSound);
            ThreatAudioComponent->SetVolumeMultiplier(Settings->HeartbeatVolume);
            ThreatAudioComponent->Play();
        }
    }
}

void UAudio_SoundManager::StartDynamicMusic()
{
    if (bIsDynamicMusicActive)
    {
        return;
    }
    
    bIsDynamicMusicActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Starting dynamic music system"));
    
    // Start music update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MusicUpdateTimer, this, 
                                        &UAudio_SoundManager::UpdateDynamicMusic, 
                                        1.0f, true);
    }
    
    UpdateDynamicMusic();
}

void UAudio_SoundManager::StopDynamicMusic(float FadeOutTime)
{
    if (!bIsDynamicMusicActive)
    {
        return;
    }
    
    bIsDynamicMusicActive = false;
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Stopping dynamic music system"));
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MusicUpdateTimer);
    }
    
    // Fade out music
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->FadeOut(FadeOutTime, 0.0f);
    }
}

void UAudio_SoundManager::PlayEnvironmentalSound(const FString& SoundName, FVector Location, float Volume)
{
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing environmental sound '%s' at location %s"), 
           *SoundName, *Location.ToString());
    
    // This would typically load and play a sound at the specified location
    // For now, we log the request
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), nullptr, Location, Volume);
}

void UAudio_SoundManager::SetWeatherAudio(bool bIsRaining, bool bIsStormy)
{
    if (!WeatherAudioComponent || !IsValid(WeatherAudioComponent))
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Weather audio - Raining: %s, Stormy: %s"), 
           bIsRaining ? TEXT("Yes") : TEXT("No"), bIsStormy ? TEXT("Yes") : TEXT("No"));
    
    if (bIsRaining)
    {
        WeatherAudioComponent->SetVolumeMultiplier(bIsStormy ? 0.9f : 0.6f);
        if (!WeatherAudioComponent->IsPlaying())
        {
            WeatherAudioComponent->Play();
        }
    }
    else
    {
        WeatherAudioComponent->Stop();
    }
}

void UAudio_SoundManager::SetTimeOfDayAudio(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Time of day set to %.2f"), CurrentTimeOfDay);
    
    // Update dynamic music based on time of day
    UpdateDynamicMusic();
}

void UAudio_SoundManager::PlayFootstepSound(ESurfaceType SurfaceType, bool bIsRunning)
{
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing footstep sound - Surface: %d, Running: %s"), 
           static_cast<int32>(SurfaceType), bIsRunning ? TEXT("Yes") : TEXT("No"));
    
    // This would play appropriate footstep sounds based on surface type and movement speed
    float Volume = bIsRunning ? 0.8f : 0.5f;
    UGameplayStatics::PlaySound2D(GetWorld(), nullptr, Volume);
}

void UAudio_SoundManager::PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location)
{
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing dinosaur sound - Type: '%s', Sound: '%s', Location: %s"), 
           *DinosaurType, *SoundType, *Location.ToString());
    
    // This would play appropriate dinosaur sounds at the specified location
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), nullptr, Location, 1.0f);
}

void UAudio_SoundManager::PlayNarration(const FString& NarrationKey, bool bInterruptCurrent)
{
    if (!NarrationAudioComponent || !IsValid(NarrationAudioComponent))
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing narration '%s', Interrupt: %s"), 
           *NarrationKey, bInterruptCurrent ? TEXT("Yes") : TEXT("No"));
    
    if (bInterruptCurrent || !NarrationAudioComponent->IsPlaying())
    {
        // Stop current narration if interrupting
        if (bInterruptCurrent)
        {
            NarrationAudioComponent->Stop();
        }
        
        // This would load and play the appropriate narration audio
        // For now, we just log the request
    }
}

void UAudio_SoundManager::StopCurrentNarration()
{
    if (NarrationAudioComponent && IsValid(NarrationAudioComponent))
    {
        UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Stopping current narration"));
        NarrationAudioComponent->Stop();
    }
}

void UAudio_SoundManager::UpdateDynamicMusic()
{
    if (!bIsDynamicMusicActive || !MusicAudioComponent || !IsValid(MusicAudioComponent))
    {
        return;
    }
    
    // Calculate music intensity based on threat level and time of day
    float BaseIntensity = 0.5f;
    
    // Threat level modifier
    const FAudio_ThreatSettings* ThreatSettings_Ptr = ThreatSettings.Find(CurrentThreatLevel);
    if (ThreatSettings_Ptr)
    {
        BaseIntensity *= ThreatSettings_Ptr->IntensityMultiplier;
    }
    
    // Time of day modifier (night is more intense)
    float TimeModifier = 1.0f;
    if (CurrentTimeOfDay < 0.25f || CurrentTimeOfDay > 0.75f) // Night time
    {
        TimeModifier = 1.3f;
    }
    
    float FinalVolume = FMath::Clamp(BaseIntensity * TimeModifier, 0.1f, 1.0f);
    MusicAudioComponent->SetVolumeMultiplier(FinalVolume);
    
    // Start music if not playing
    if (!MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->Play();
    }
}

void UAudio_SoundManager::CrossfadeBiomeAudio(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome, float Duration)
{
    if (!BiomeAudioComponent || !IsValid(BiomeAudioComponent))
    {
        return;
    }
    
    const FAudio_BiomeSettings* FromSettings = BiomeSettings.Find(FromBiome);
    const FAudio_BiomeSettings* ToSettings = BiomeSettings.Find(ToBiome);
    
    if (!ToSettings)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Crossfading from biome %d to %d over %.2f seconds"), 
           static_cast<int32>(FromBiome), static_cast<int32>(ToBiome), Duration);
    
    // Fade out current biome audio
    if (FromSettings && BiomeAudioComponent->IsPlaying())
    {
        BiomeAudioComponent->FadeOut(Duration * 0.5f, 0.0f);
    }
    
    // Set up timer to fade in new biome audio
    if (UWorld* World = GetWorld())
    {
        FTimerDelegate FadeInDelegate;
        FadeInDelegate.BindLambda([this, ToSettings, Duration]()
        {
            if (BiomeAudioComponent && IsValid(BiomeAudioComponent) && ToSettings->AmbientSound.IsValid())
            {
                USoundCue* NewBiomeSound = ToSettings->AmbientSound.LoadSynchronous();
                if (NewBiomeSound)
                {
                    BiomeAudioComponent->SetSound(NewBiomeSound);
                    BiomeAudioComponent->SetVolumeMultiplier(0.0f);
                    BiomeAudioComponent->Play();
                    BiomeAudioComponent->FadeIn(Duration * 0.5f, ToSettings->BaseVolume);
                }
            }
        });
        
        World->GetTimerManager().SetTimer(BiomeTransitionTimer, FadeInDelegate, Duration * 0.5f, false);
    }
}