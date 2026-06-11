#include "Audio_SoundManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    MasterVolume = 1.0f;
    
    // Initialize default volume levels for each sound type
    SoundTypeVolumes.Add(EAudio_SoundType::Ambient, 0.7f);
    SoundTypeVolumes.Add(EAudio_SoundType::DinosaurFootsteps, 0.9f);
    SoundTypeVolumes.Add(EAudio_SoundType::DinosaurRoar, 1.0f);
    SoundTypeVolumes.Add(EAudio_SoundType::PlayerFootsteps, 0.5f);
    SoundTypeVolumes.Add(EAudio_SoundType::Fire, 0.6f);
    SoundTypeVolumes.Add(EAudio_SoundType::Weather, 0.8f);
    SoundTypeVolumes.Add(EAudio_SoundType::Combat, 0.9f);
    SoundTypeVolumes.Add(EAudio_SoundType::UI, 0.7f);
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeDefaultSounds();
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager initialized"));
}

void UAudio_SoundManager::Deinitialize()
{
    StopAllSounds();
    Super::Deinitialize();
}

void UAudio_SoundManager::PlaySound(EAudio_SoundType SoundType, FVector Location, float VolumeMultiplier)
{
    FAudio_SoundEntry* SoundEntry = FindSoundEntry(SoundType);
    if (!SoundEntry || !SoundEntry->SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound entry not found or invalid for type: %d"), (int32)SoundType);
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Stop existing sound of this type if not looping
    if (!SoundEntry->bLooping && ActiveSounds.Contains(SoundType))
    {
        StopSound(SoundType);
    }

    float FinalVolume = SoundEntry->Volume * VolumeMultiplier * MasterVolume;
    if (SoundTypeVolumes.Contains(SoundType))
    {
        FinalVolume *= SoundTypeVolumes[SoundType];
    }

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        World,
        SoundEntry->SoundCue.Get(),
        Location,
        FRotator::ZeroRotator,
        FinalVolume,
        SoundEntry->Pitch
    );

    if (AudioComponent && SoundEntry->bLooping)
    {
        ActiveSounds.Add(SoundType, AudioComponent);
    }
}

void UAudio_SoundManager::PlaySoundAtLocation(EAudio_SoundType SoundType, FVector Location, float VolumeMultiplier)
{
    PlaySound(SoundType, Location, VolumeMultiplier);
}

void UAudio_SoundManager::StopSound(EAudio_SoundType SoundType)
{
    if (ActiveSounds.Contains(SoundType))
    {
        UAudioComponent* AudioComponent = ActiveSounds[SoundType];
        if (AudioComponent && IsValid(AudioComponent))
        {
            AudioComponent->Stop();
        }
        ActiveSounds.Remove(SoundType);
    }
}

void UAudio_SoundManager::StopAllSounds()
{
    for (auto& SoundPair : ActiveSounds)
    {
        if (SoundPair.Value && IsValid(SoundPair.Value))
        {
            SoundPair.Value->Stop();
        }
    }
    ActiveSounds.Empty();
}

void UAudio_SoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudio_SoundManager::SetSoundTypeVolume(EAudio_SoundType SoundType, float Volume)
{
    SoundTypeVolumes.Add(SoundType, FMath::Clamp(Volume, 0.0f, 1.0f));
}

void UAudio_SoundManager::RegisterSoundEntry(const FAudio_SoundEntry& SoundEntry)
{
    // Remove existing entry of the same type
    SoundEntries.RemoveAll([SoundEntry](const FAudio_SoundEntry& Entry)
    {
        return Entry.SoundType == SoundEntry.SoundType;
    });
    
    // Add new entry
    SoundEntries.Add(SoundEntry);
}

void UAudio_SoundManager::PlayDinosaurFootstep(FVector Location, float DinosaurSize)
{
    float VolumeMultiplier = FMath::Clamp(DinosaurSize, 0.1f, 3.0f);
    PlaySoundAtLocation(EAudio_SoundType::DinosaurFootsteps, Location, VolumeMultiplier);
}

void UAudio_SoundManager::PlayDinosaurRoar(FVector Location, float IntensityLevel)
{
    float VolumeMultiplier = FMath::Clamp(IntensityLevel, 0.5f, 2.0f);
    PlaySoundAtLocation(EAudio_SoundType::DinosaurRoar, Location, VolumeMultiplier);
}

FAudio_SoundEntry* UAudio_SoundManager::FindSoundEntry(EAudio_SoundType SoundType)
{
    for (FAudio_SoundEntry& Entry : SoundEntries)
    {
        if (Entry.SoundType == SoundType)
        {
            return &Entry;
        }
    }
    return nullptr;
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // Initialize with placeholder sound entries
    // These would be replaced with actual sound assets in production
    
    FAudio_SoundEntry AmbientEntry;
    AmbientEntry.SoundType = EAudio_SoundType::Ambient;
    AmbientEntry.Volume = 0.7f;
    AmbientEntry.bLooping = true;
    SoundEntries.Add(AmbientEntry);

    FAudio_SoundEntry FootstepEntry;
    FootstepEntry.SoundType = EAudio_SoundType::DinosaurFootsteps;
    FootstepEntry.Volume = 0.9f;
    FootstepEntry.bLooping = false;
    SoundEntries.Add(FootstepEntry);

    FAudio_SoundEntry RoarEntry;
    RoarEntry.SoundType = EAudio_SoundType::DinosaurRoar;
    RoarEntry.Volume = 1.0f;
    RoarEntry.bLooping = false;
    SoundEntries.Add(RoarEntry);

    FAudio_SoundEntry FireEntry;
    FireEntry.SoundType = EAudio_SoundType::Fire;
    FireEntry.Volume = 0.6f;
    FireEntry.bLooping = true;
    SoundEntries.Add(FireEntry);
}