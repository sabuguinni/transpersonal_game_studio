#include "AudioSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

UAudioSystemManager::UAudioSystemManager()
{
    MasterVolume = 1.0f;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultVolumes();
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager initialized"));
}

void UAudioSystemManager::Deinitialize()
{
    StopAllSounds();
    
    Super::Deinitialize();
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager deinitialized"));
}

void UAudioSystemManager::InitializeDefaultVolumes()
{
    CategoryVolumes.Empty();
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::SFX, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Voice, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.8f);
}

void UAudioSystemManager::PlaySound2D(const FAudio_SoundConfig& SoundConfig)
{
    if (!SoundConfig.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Invalid sound cue for 2D playback"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No valid world for 2D sound playback"));
        return;
    }

    USoundCue* LoadedSoundCue = SoundConfig.SoundCue.LoadSynchronous();
    if (!LoadedSoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Failed to load sound cue"));
        return;
    }

    float CategoryVolume = GetCategoryVolume(SoundConfig.Category);
    float FinalVolume = MasterVolume * CategoryVolume * SoundConfig.VolumeMultiplier;

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(
        World,
        LoadedSoundCue,
        FinalVolume,
        SoundConfig.PitchMultiplier,
        SoundConfig.FadeInTime
    );

    if (AudioComponent)
    {
        AudioComponent->bAutoDestroy = !SoundConfig.bLooping;
        ActiveAudioComponents.Add(AudioComponent);
        
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing 2D sound with volume %.2f"), FinalVolume);
    }

    CleanupFinishedComponents();
}

void UAudioSystemManager::PlaySound3D(const FAudio_SoundConfig& SoundConfig, const FVector& Location)
{
    if (!SoundConfig.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Invalid sound cue for 3D playback"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No valid world for 3D sound playback"));
        return;
    }

    USoundCue* LoadedSoundCue = SoundConfig.SoundCue.LoadSynchronous();
    if (!LoadedSoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Failed to load sound cue"));
        return;
    }

    float CategoryVolume = GetCategoryVolume(SoundConfig.Category);
    float FinalVolume = MasterVolume * CategoryVolume * SoundConfig.VolumeMultiplier;

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        World,
        LoadedSoundCue,
        Location,
        FRotator::ZeroRotator,
        FinalVolume,
        SoundConfig.PitchMultiplier,
        SoundConfig.FadeInTime
    );

    if (AudioComponent)
    {
        AudioComponent->bAutoDestroy = !SoundConfig.bLooping;
        ActiveAudioComponents.Add(AudioComponent);
        
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing 3D sound at location (%.1f, %.1f, %.1f) with volume %.2f"), 
               Location.X, Location.Y, Location.Z, FinalVolume);
    }

    CleanupFinishedComponents();
}

void UAudioSystemManager::StopAllSounds()
{
    for (UAudioComponent* AudioComponent : ActiveAudioComponents)
    {
        if (IsValid(AudioComponent))
        {
            AudioComponent->Stop();
        }
    }
    
    ActiveAudioComponents.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Stopped all active sounds"));
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Master volume set to %.2f"), MasterVolume);
}

void UAudioSystemManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    float ClampedVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    CategoryVolumes.Add(Category, ClampedVolume);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Category volume set to %.2f"), ClampedVolume);
}

float UAudioSystemManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    const float* VolumePtr = CategoryVolumes.Find(Category);
    return VolumePtr ? *VolumePtr : 1.0f;
}

void UAudioSystemManager::CleanupFinishedComponents()
{
    ActiveAudioComponents.RemoveAll([](UAudioComponent* Component)
    {
        return !IsValid(Component) || !Component->IsPlaying();
    });
}