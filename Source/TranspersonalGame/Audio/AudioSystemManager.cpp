#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioDevice.h"

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing audio subsystem"));
    
    InitializeAudioTypeVolumes();
    LoadAudioDatabase();
    
    // Set up cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UAudioSystemManager::CleanupInactiveComponents,
            5.0f,
            true
        );
    }
}

void UAudioSystemManager::Deinitialize()
{
    StopAllSounds();
    SoundDatabase.Empty();
    ActiveAudioComponents.Empty();
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeAudioTypeVolumes()
{
    AudioTypeVolumes.Add(EAudio_AudioType::SFX, 1.0f);
    AudioTypeVolumes.Add(EAudio_AudioType::Music, 0.7f);
    AudioTypeVolumes.Add(EAudio_AudioType::Ambience, 0.8f);
    AudioTypeVolumes.Add(EAudio_AudioType::Voice, 1.0f);
    AudioTypeVolumes.Add(EAudio_AudioType::UI, 0.9f);
}

void UAudioSystemManager::PlaySound(const FString& SoundID, FVector Location, float VolumeMultiplier)
{
    if (!SoundDatabase.Contains(SoundID))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Sound ID '%s' not found in database"), *SoundID);
        return;
    }

    const FAudio_SoundEntry& SoundEntry = SoundDatabase[SoundID];
    
    if (!SoundEntry.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Sound cue for '%s' is not valid"), *SoundID);
        return;
    }

    UAudioComponent* AudioComp = CreateAudioComponent(SoundEntry, Location);
    if (AudioComp)
    {
        float FinalVolume = SoundEntry.Volume * VolumeMultiplier * MasterVolume;
        if (AudioTypeVolumes.Contains(SoundEntry.AudioType))
        {
            FinalVolume *= AudioTypeVolumes[SoundEntry.AudioType];
        }
        
        AudioComp->SetVolumeMultiplier(FinalVolume);
        AudioComp->SetPitchMultiplier(SoundEntry.Pitch);
        AudioComp->Play();
        
        if (!SoundEntry.bLooping)
        {
            ActiveAudioComponents.Add(SoundID + FString::Printf(TEXT("_%d"), FMath::Rand()), AudioComp);
        }
        else
        {
            // Store looping sounds with their ID for later control
            if (ActiveAudioComponents.Contains(SoundID))
            {
                ActiveAudioComponents[SoundID]->Stop();
            }
            ActiveAudioComponents.Add(SoundID, AudioComp);
        }
    }
}

void UAudioSystemManager::PlaySoundAtLocation(const FString& SoundID, FVector Location, float VolumeMultiplier)
{
    PlaySound(SoundID, Location, VolumeMultiplier);
}

void UAudioSystemManager::StopSound(const FString& SoundID)
{
    if (ActiveAudioComponents.Contains(SoundID))
    {
        if (UAudioComponent* AudioComp = ActiveAudioComponents[SoundID])
        {
            AudioComp->Stop();
        }
        ActiveAudioComponents.Remove(SoundID);
    }
}

void UAudioSystemManager::StopAllSounds()
{
    for (auto& Pair : ActiveAudioComponents)
    {
        if (UAudioComponent* AudioComp = Pair.Value)
        {
            AudioComp->Stop();
        }
    }
    ActiveAudioComponents.Empty();
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudioSystemManager::SetAudioTypeVolume(EAudio_AudioType AudioType, float Volume)
{
    AudioTypeVolumes.Add(AudioType, FMath::Clamp(Volume, 0.0f, 1.0f));
}

void UAudioSystemManager::RegisterSound(const FString& SoundID, USoundCue* SoundCue, EAudio_AudioType AudioType)
{
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Trying to register null sound cue for ID '%s'"), *SoundID);
        return;
    }

    FAudio_SoundEntry NewEntry;
    NewEntry.SoundID = SoundID;
    NewEntry.SoundCue = SoundCue;
    NewEntry.AudioType = AudioType;
    
    SoundDatabase.Add(SoundID, NewEntry);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered sound '%s'"), *SoundID);
}

void UAudioSystemManager::LoadAudioDatabase()
{
    // Load prehistoric-specific sounds
    // These would typically be loaded from data assets or configuration files
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Loading prehistoric audio database"));
    
    // Register default prehistoric sounds (placeholders for now)
    // In a real implementation, these would load actual sound cues from content
}

void UAudioSystemManager::PlayDinosaurRoar(const FString& DinosaurType, FVector Location)
{
    FString SoundID = FString::Printf(TEXT("Roar_%s"), *DinosaurType);
    PlaySoundAtLocation(SoundID, Location, 1.0f);
}

void UAudioSystemManager::PlayFootstepSound(const FString& SurfaceType, FVector Location, float Weight)
{
    FString SoundID = FString::Printf(TEXT("Footstep_%s"), *SurfaceType);
    float VolumeMultiplier = FMath::Clamp(Weight, 0.1f, 2.0f);
    PlaySoundAtLocation(SoundID, Location, VolumeMultiplier);
}

void UAudioSystemManager::PlayAmbienceForBiome(const FString& BiomeType)
{
    // Stop current ambience
    if (CurrentAmbienceComponent && CurrentAmbienceComponent->IsPlaying())
    {
        CurrentAmbienceComponent->Stop();
    }
    
    FString SoundID = FString::Printf(TEXT("Ambience_%s"), *BiomeType);
    if (SoundDatabase.Contains(SoundID))
    {
        const FAudio_SoundEntry& SoundEntry = SoundDatabase[SoundID];
        CurrentAmbienceComponent = CreateAudioComponent(SoundEntry, FVector::ZeroVector);
        
        if (CurrentAmbienceComponent)
        {
            CurrentAmbienceComponent->SetVolumeMultiplier(0.6f * MasterVolume);
            CurrentAmbienceComponent->Play();
        }
    }
}

void UAudioSystemManager::PlayWeatherAudio(const FString& WeatherType, float Intensity)
{
    FString SoundID = FString::Printf(TEXT("Weather_%s"), *WeatherType);
    float VolumeMultiplier = FMath::Clamp(Intensity, 0.1f, 1.5f);
    PlaySound(SoundID, FVector::ZeroVector, VolumeMultiplier);
}

UAudioComponent* UAudioSystemManager::CreateAudioComponent(const FAudio_SoundEntry& SoundEntry, FVector Location)
{
    if (!SoundEntry.SoundCue.IsValid())
    {
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        World,
        SoundEntry.SoundCue.Get(),
        Location,
        FRotator::ZeroRotator,
        1.0f,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        true
    );
    
    return AudioComp;
}

void UAudioSystemManager::CleanupInactiveComponents()
{
    TArray<FString> KeysToRemove;
    
    for (auto& Pair : ActiveAudioComponents)
    {
        if (!Pair.Value || !Pair.Value->IsPlaying())
        {
            KeysToRemove.Add(Pair.Key);
        }
    }
    
    for (const FString& Key : KeysToRemove)
    {
        ActiveAudioComponents.Remove(Key);
    }
}