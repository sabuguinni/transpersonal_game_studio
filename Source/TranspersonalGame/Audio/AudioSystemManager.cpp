#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "AudioDevice.h"

UAudioSystemManager::UAudioSystemManager()
{
    CurrentBiome = EEng_BiomeType::Forest;
    MasterVolume = 1.0f;
    SFXVolume = 1.0f;
    MusicVolume = 0.7f;
    AmbientVolume = 0.8f;
    
    CurrentMusicState.CurrentDangerLevel = EAudio_DangerLevel::Safe;
    CurrentMusicState.IntensityLevel = 0.0f;
    CurrentMusicState.bInCombat = false;
    CurrentMusicState.HeartRate = 60.0f;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initializing prehistoric audio system"));
    
    InitializeAudioComponents();
    
    // Start with safe forest ambient
    StartAmbientSoundscape(EEng_BiomeType::Forest);
    SetDangerLevel(EAudio_DangerLevel::Safe);
}

void UAudioSystemManager::Deinitialize()
{
    StopAllSounds();
    
    // Clean up audio components
    for (auto& AudioPair : AudioComponents)
    {
        if (AudioPair.Value)
        {
            AudioPair.Value->Stop();
            AudioPair.Value = nullptr;
        }
    }
    AudioComponents.Empty();
    
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
    
    if (HeartbeatAudioComponent)
    {
        HeartbeatAudioComponent->Stop();
        HeartbeatAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeAudioComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: No world available for audio component creation"));
        return;
    }
    
    // Create ambient audio component
    AmbientAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoDestroy = false;
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume);
    }
    
    // Create music audio component
    MusicAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
    if (MusicAudioComponent)
    {
        MusicAudioComponent->bAutoDestroy = false;
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume);
    }
    
    // Create heartbeat audio component
    HeartbeatAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
    if (HeartbeatAudioComponent)
    {
        HeartbeatAudioComponent->bAutoDestroy = false;
        HeartbeatAudioComponent->SetVolumeMultiplier(SFXVolume * 0.5f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Audio components initialized"));
}

void UAudioSystemManager::PlaySound(USoundCue* SoundCue, FVector Location, const FAudio_SoundSettings& Settings)
{
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Attempted to play null sound cue"));
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    if (Settings.b3D)
    {
        UGameplayStatics::PlaySoundAtLocation(
            World,
            SoundCue,
            Location,
            FRotator::ZeroRotator,
            Settings.Volume * SFXVolume * MasterVolume,
            Settings.Pitch
        );
    }
    else
    {
        UGameplayStatics::PlaySound2D(
            World,
            SoundCue,
            Settings.Volume * SFXVolume * MasterVolume,
            Settings.Pitch
        );
    }
}

void UAudioSystemManager::PlaySound2D(USoundCue* SoundCue, const FAudio_SoundSettings& Settings)
{
    if (!SoundCue)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    UGameplayStatics::PlaySound2D(
        World,
        SoundCue,
        Settings.Volume * SFXVolume * MasterVolume,
        Settings.Pitch
    );
}

void UAudioSystemManager::StopSound(USoundCue* SoundCue)
{
    // Note: UE5 doesn't have a direct way to stop specific sound cues
    // This would typically be handled by storing audio component references
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Stop sound requested"));
}

void UAudioSystemManager::StopAllSounds()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Stop all audio components
    for (auto& AudioPair : AudioComponents)
    {
        if (AudioPair.Value)
        {
            AudioPair.Value->Stop();
        }
    }
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Stop();
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->Stop();
    }
    
    if (HeartbeatAudioComponent)
    {
        HeartbeatAudioComponent->Stop();
    }
}

void UAudioSystemManager::StartAmbientSoundscape(EEng_BiomeType BiomeType)
{
    CurrentBiome = BiomeType;
    
    // Find the appropriate ambient sound for this biome
    USoundCue** FoundSound = BiomeAmbientSounds.Find(BiomeType);
    if (FoundSound && *FoundSound && AmbientAudioComponent)
    {
        AmbientAudioComponent->SetSound(*FoundSound);
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
        AmbientAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Started ambient soundscape for biome %d"), (int32)BiomeType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No ambient sound found for biome %d"), (int32)BiomeType);
    }
}

void UAudioSystemManager::UpdateAmbientIntensity(float Intensity)
{
    if (AmbientAudioComponent)
    {
        float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
        float NewVolume = AmbientVolume * MasterVolume * ClampedIntensity;
        AmbientAudioComponent->SetVolumeMultiplier(NewVolume);
    }
}

void UAudioSystemManager::TransitionToNewBiome(EEng_BiomeType NewBiome, float TransitionTime)
{
    if (NewBiome == CurrentBiome)
    {
        return;
    }
    
    // Fade out current ambient
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->FadeOut(TransitionTime, 0.0f);
    }
    
    // Start new ambient after transition
    FTimerHandle TransitionTimer;
    GetWorld()->GetTimerManager().SetTimer(TransitionTimer, [this, NewBiome]()
    {
        StartAmbientSoundscape(NewBiome);
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->FadeIn(2.0f, AmbientVolume * MasterVolume);
        }
    }, TransitionTime, false);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Transitioning from biome %d to %d"), (int32)CurrentBiome, (int32)NewBiome);
}

void UAudioSystemManager::PlayDinosaurFootstep(FVector Location, float DinosaurSize)
{
    if (!DinosaurFootstepSound)
    {
        return;
    }
    
    FAudio_SoundSettings FootstepSettings;
    FootstepSettings.Volume = FMath::Clamp(DinosaurSize * 0.1f, 0.2f, 2.0f);
    FootstepSettings.Pitch = FMath::Clamp(2.0f - DinosaurSize * 0.1f, 0.5f, 1.5f);
    FootstepSettings.b3D = true;
    FootstepSettings.AttenuationRadius = DinosaurSize * 100.0f;
    
    PlaySound(DinosaurFootstepSound, Location, FootstepSettings);
}

void UAudioSystemManager::PlayDinosaurRoar(FVector Location, EAudio_DangerLevel ThreatLevel)
{
    if (!DinosaurRoarSound)
    {
        return;
    }
    
    FAudio_SoundSettings RoarSettings;
    RoarSettings.Volume = 0.5f + (float)ThreatLevel * 0.2f;
    RoarSettings.Pitch = 0.8f + (float)ThreatLevel * 0.1f;
    RoarSettings.b3D = true;
    RoarSettings.AttenuationRadius = 2000.0f + (float)ThreatLevel * 500.0f;
    
    PlaySound(DinosaurRoarSound, Location, RoarSettings);
    
    // Update danger level based on roar
    SetDangerLevel(ThreatLevel);
}

void UAudioSystemManager::PlayDinosaurBreathing(FVector Location, bool bAggressive)
{
    // This would use a breathing sound asset if available
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing dinosaur breathing at location %s, aggressive: %s"), 
           *Location.ToString(), bAggressive ? TEXT("true") : TEXT("false"));
}

void UAudioSystemManager::UpdateHeartbeat(float HeartRate)
{
    CurrentMusicState.HeartRate = HeartRate;
    
    if (HeartbeatAudioComponent && PlayerHeartbeatSound)
    {
        // Adjust heartbeat based on heart rate
        float NormalizedRate = FMath::Clamp((HeartRate - 60.0f) / 120.0f, 0.0f, 1.0f);
        float Volume = NormalizedRate * SFXVolume * MasterVolume * 0.3f;
        float Pitch = 0.8f + NormalizedRate * 0.4f;
        
        HeartbeatAudioComponent->SetSound(PlayerHeartbeatSound);
        HeartbeatAudioComponent->SetVolumeMultiplier(Volume);
        HeartbeatAudioComponent->SetPitchMultiplier(Pitch);
        
        if (HeartRate > 80.0f && !HeartbeatAudioComponent->IsPlaying())
        {
            HeartbeatAudioComponent->Play();
        }
        else if (HeartRate <= 80.0f && HeartbeatAudioComponent->IsPlaying())
        {
            HeartbeatAudioComponent->FadeOut(2.0f, 0.0f);
        }
    }
}

void UAudioSystemManager::PlaySurvivalWarning(EAudio_DangerLevel DangerLevel)
{
    if (!SurvivalWarningSound)
    {
        return;
    }
    
    FAudio_SoundSettings WarningSettings;
    WarningSettings.Volume = 0.3f + (float)DangerLevel * 0.15f;
    WarningSettings.Pitch = 0.9f + (float)DangerLevel * 0.05f;
    WarningSettings.b3D = false;
    
    PlaySound2D(SurvivalWarningSound, WarningSettings);
}

void UAudioSystemManager::PlayPlayerFootstep(FVector Location, bool bSneaking)
{
    if (!PlayerFootstepSound)
    {
        return;
    }
    
    FAudio_SoundSettings FootstepSettings;
    FootstepSettings.Volume = bSneaking ? 0.1f : 0.3f;
    FootstepSettings.Pitch = bSneaking ? 0.8f : 1.0f;
    FootstepSettings.b3D = true;
    FootstepSettings.AttenuationRadius = bSneaking ? 200.0f : 500.0f;
    
    PlaySound(PlayerFootstepSound, Location, FootstepSettings);
}

void UAudioSystemManager::UpdateMusicState(const FAudio_DynamicMusicState& NewState)
{
    FAudio_DynamicMusicState OldState = CurrentMusicState;
    CurrentMusicState = NewState;
    
    // Check if we need to transition music
    if (OldState.CurrentDangerLevel != NewState.CurrentDangerLevel || 
        OldState.bInCombat != NewState.bInCombat)
    {
        ProcessMusicTransition();
    }
    
    // Update heartbeat
    UpdateHeartbeat(NewState.HeartRate);
}

void UAudioSystemManager::SetDangerLevel(EAudio_DangerLevel DangerLevel)
{
    if (CurrentMusicState.CurrentDangerLevel != DangerLevel)
    {
        CurrentMusicState.CurrentDangerLevel = DangerLevel;
        ProcessMusicTransition();
    }
}

void UAudioSystemManager::StartCombatMusic()
{
    CurrentMusicState.bInCombat = true;
    ProcessMusicTransition();
}

void UAudioSystemManager::EndCombatMusic()
{
    CurrentMusicState.bInCombat = false;
    ProcessMusicTransition();
}

void UAudioSystemManager::ProcessMusicTransition()
{
    if (!MusicAudioComponent)
    {
        return;
    }
    
    // Find appropriate music track
    EAudio_DangerLevel TargetLevel = CurrentMusicState.bInCombat ? 
        EAudio_DangerLevel::Critical : CurrentMusicState.CurrentDangerLevel;
    
    USoundCue** FoundTrack = MusicTracks.Find(TargetLevel);
    if (FoundTrack && *FoundTrack)
    {
        // Fade out current music
        if (MusicAudioComponent->IsPlaying())
        {
            MusicAudioComponent->FadeOut(CurrentMusicState.TransitionTime, 0.0f);
        }
        
        // Start new music after fade
        FTimerHandle MusicTimer;
        GetWorld()->GetTimerManager().SetTimer(MusicTimer, [this, FoundTrack]()
        {
            if (MusicAudioComponent)
            {
                MusicAudioComponent->SetSound(*FoundTrack);
                MusicAudioComponent->FadeIn(2.0f, MusicVolume * MasterVolume);
            }
        }, CurrentMusicState.TransitionTime, false);
        
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Transitioning to danger level %d music"), (int32)TargetLevel);
    }
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateAudioMixing();
}

void UAudioSystemManager::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateAudioMixing();
}

void UAudioSystemManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
}

void UAudioSystemManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
}

void UAudioSystemManager::UpdateAudioMixing()
{
    // Update all audio component volumes
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
    
    if (HeartbeatAudioComponent)
    {
        HeartbeatAudioComponent->SetVolumeMultiplier(SFXVolume * MasterVolume * 0.3f);
    }
}

void UAudioSystemManager::CalculateAudioAttenuation(FVector ListenerLocation, FVector SoundLocation, float& OutVolume, float& OutPitch)
{
    float Distance = FVector::Dist(ListenerLocation, SoundLocation);
    
    // Simple distance-based attenuation
    OutVolume = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f);
    
    // Slight pitch variation based on distance (Doppler effect simulation)
    OutPitch = FMath::Clamp(1.0f - (Distance / 10000.0f), 0.8f, 1.2f);
}