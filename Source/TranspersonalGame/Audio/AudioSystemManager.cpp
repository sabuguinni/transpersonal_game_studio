#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultVolumes();
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager initialized"));
}

void UAudioSystemManager::Deinitialize()
{
    StopAllSounds();
    ActiveAudioComponents.Empty();
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeDefaultVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Dinosaur, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Player, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Weather, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::Combat, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.5f);
}

void UAudioSystemManager::PlaySound2D(const FAudio_SoundConfig& SoundConfig)
{
    if (!SoundConfig.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid sound cue in PlaySound2D"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    float CategoryVolume = CategoryVolumes.FindRef(SoundConfig.Category);
    float FinalVolume = SoundConfig.VolumeMultiplier * CategoryVolume * MasterVolume;

    UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(
        World,
        SoundConfig.SoundCue.Get(),
        FinalVolume,
        SoundConfig.PitchMultiplier
    );

    if (AudioComp)
    {
        ActiveAudioComponents.Add(AudioComp);
    }

    CleanupFinishedComponents();
}

void UAudioSystemManager::PlaySound3D(const FAudio_SoundConfig& SoundConfig, FVector Location)
{
    if (!SoundConfig.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid sound cue in PlaySound3D"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    float CategoryVolume = CategoryVolumes.FindRef(SoundConfig.Category);
    float FinalVolume = SoundConfig.VolumeMultiplier * CategoryVolume * MasterVolume;

    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        World,
        SoundConfig.SoundCue.Get(),
        Location,
        FRotator::ZeroRotator,
        FinalVolume,
        SoundConfig.PitchMultiplier
    );

    if (AudioComp)
    {
        ActiveAudioComponents.Add(AudioComp);
    }

    CleanupFinishedComponents();
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudioSystemManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    float ClampedVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    CategoryVolumes.Add(Category, ClampedVolume);
}

void UAudioSystemManager::StopAllSounds()
{
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
    }
    
    ActiveAudioComponents.Empty();
}

void UAudioSystemManager::StopSoundsOfCategory(EAudio_SoundCategory Category)
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (IsValid(AudioComp))
        {
            // Note: We'd need to store category info with each component for this to work fully
            // For now, this is a placeholder implementation
            AudioComp->Stop();
            ActiveAudioComponents.RemoveAt(i);
        }
    }
}

void UAudioSystemManager::CleanupFinishedComponents()
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (!IsValid(AudioComp) || !AudioComp->IsPlaying())
        {
            ActiveAudioComponents.RemoveAt(i);
        }
    }
}