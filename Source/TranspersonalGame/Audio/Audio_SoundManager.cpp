#include "Audio_SoundManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    // Initialize category volumes
    CategoryVolumes.Add(EAudio_SoundCategory::Ambience, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::SFX, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Voice, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.9f);
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    LoadSoundLibrary();
    InitializeDefaultSounds();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager initialized successfully"));
}

void UAudio_SoundManager::Deinitialize()
{
    StopAllSounds();
    SoundLibrary.Empty();
    ActiveSounds.Empty();
    
    Super::Deinitialize();
}

void UAudio_SoundManager::PlaySound2D(const FString& SoundID, float VolumeMultiplier)
{
    if (!SoundLibrary.Contains(SoundID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound ID not found: %s"), *SoundID);
        return;
    }

    const FAudio_SoundEntry& SoundEntry = SoundLibrary[SoundID];
    if (!SoundEntry.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound cue is invalid for ID: %s"), *SoundID);
        return;
    }

    float FinalVolume = SoundEntry.Volume * VolumeMultiplier * MasterVolume * GetCategoryVolume(SoundEntry.Category);

    if (UWorld* World = GetWorld())
    {
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(
            World,
            SoundEntry.SoundCue.Get(),
            FinalVolume,
            SoundEntry.Pitch
        );

        if (AudioComp && !SoundEntry.bLooping)
        {
            ActiveSounds.Add(SoundID, AudioComp);
        }
    }
}

void UAudio_SoundManager::PlaySound3D(const FString& SoundID, FVector Location, float VolumeMultiplier)
{
    if (!SoundLibrary.Contains(SoundID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound ID not found: %s"), *SoundID);
        return;
    }

    const FAudio_SoundEntry& SoundEntry = SoundLibrary[SoundID];
    if (!SoundEntry.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound cue is invalid for ID: %s"), *SoundID);
        return;
    }

    float FinalVolume = SoundEntry.Volume * VolumeMultiplier * MasterVolume * GetCategoryVolume(SoundEntry.Category);

    if (UWorld* World = GetWorld())
    {
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
            World,
            SoundEntry.SoundCue.Get(),
            Location,
            FRotator::ZeroRotator,
            FinalVolume,
            SoundEntry.Pitch
        );

        if (AudioComp && !SoundEntry.bLooping)
        {
            ActiveSounds.Add(SoundID, AudioComp);
        }
    }
}

void UAudio_SoundManager::StopSound(const FString& SoundID)
{
    if (UAudioComponent** AudioCompPtr = ActiveSounds.Find(SoundID))
    {
        if (UAudioComponent* AudioComp = *AudioCompPtr)
        {
            AudioComp->Stop();
        }
        ActiveSounds.Remove(SoundID);
    }
}

void UAudio_SoundManager::StopAllSounds()
{
    for (auto& SoundPair : ActiveSounds)
    {
        if (UAudioComponent* AudioComp = SoundPair.Value)
        {
            AudioComp->Stop();
        }
    }
    ActiveSounds.Empty();
}

void UAudio_SoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
}

void UAudio_SoundManager::RegisterSound(const FAudio_SoundEntry& SoundEntry)
{
    if (!SoundEntry.SoundID.IsEmpty())
    {
        SoundLibrary.Add(SoundEntry.SoundID, SoundEntry);
        UE_LOG(LogTemp, Log, TEXT("Registered sound: %s"), *SoundEntry.SoundID);
    }
}

void UAudio_SoundManager::LoadSoundLibrary()
{
    // This would typically load from a data table or config file
    // For now, we'll register some default sounds programmatically
    UE_LOG(LogTemp, Log, TEXT("Loading sound library..."));
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // Register default prehistoric sounds
    FAudio_SoundEntry ForestAmbience;
    ForestAmbience.SoundID = TEXT("ForestAmbience");
    ForestAmbience.Category = EAudio_SoundCategory::Ambience;
    ForestAmbience.Volume = 0.6f;
    ForestAmbience.bLooping = true;
    ForestAmbience.b3D = false;
    RegisterSound(ForestAmbience);

    FAudio_SoundEntry TRexRoar;
    TRexRoar.SoundID = TEXT("TRexRoar");
    TRexRoar.Category = EAudio_SoundCategory::SFX;
    TRexRoar.Volume = 1.0f;
    TRexRoar.bLooping = false;
    TRexRoar.b3D = true;
    RegisterSound(TRexRoar);

    FAudio_SoundEntry Footsteps;
    Footsteps.SoundID = TEXT("PlayerFootsteps");
    Footsteps.Category = EAudio_SoundCategory::SFX;
    Footsteps.Volume = 0.8f;
    Footsteps.bLooping = false;
    Footsteps.b3D = true;
    RegisterSound(Footsteps);

    FAudio_SoundEntry WindAmbience;
    WindAmbience.SoundID = TEXT("WindAmbience");
    WindAmbience.Category = EAudio_SoundCategory::Ambience;
    WindAmbience.Volume = 0.4f;
    WindAmbience.bLooping = true;
    WindAmbience.b3D = false;
    RegisterSound(WindAmbience);
}

float UAudio_SoundManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    if (const float* Volume = CategoryVolumes.Find(Category))
    {
        return *Volume;
    }
    return 1.0f;
}