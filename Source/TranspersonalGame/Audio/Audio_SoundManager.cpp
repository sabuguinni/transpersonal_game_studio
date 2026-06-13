#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeCategoryVolumes();
    InitializeDefaultSounds();
    LoadSoundLibrary();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager initialized"));
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
    
    if (!SoundEntry.SoundAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound asset is null for ID: %s"), *SoundID);
        return;
    }

    float FinalVolume = SoundEntry.Volume * VolumeMultiplier;
    if (CategoryVolumes.Contains(SoundEntry.Category))
    {
        FinalVolume *= CategoryVolumes[SoundEntry.Category];
    }

    UGameplayStatics::PlaySound2D(GetWorld(), SoundEntry.SoundAsset.Get(), FinalVolume, SoundEntry.Pitch);
}

void UAudio_SoundManager::PlaySoundAtLocation(const FString& SoundID, const FVector& Location, float VolumeMultiplier)
{
    if (!SoundLibrary.Contains(SoundID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound ID not found: %s"), *SoundID);
        return;
    }

    const FAudio_SoundEntry& SoundEntry = SoundLibrary[SoundID];
    
    if (!SoundEntry.SoundAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound asset is null for ID: %s"), *SoundID);
        return;
    }

    float FinalVolume = SoundEntry.Volume * VolumeMultiplier;
    if (CategoryVolumes.Contains(SoundEntry.Category))
    {
        FinalVolume *= CategoryVolumes[SoundEntry.Category];
    }

    UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundEntry.SoundAsset.Get(), Location, FinalVolume, SoundEntry.Pitch);
}

void UAudio_SoundManager::PlaySoundAttached(const FString& SoundID, USceneComponent* AttachComponent, float VolumeMultiplier)
{
    if (!AttachComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("AttachComponent is null for sound: %s"), *SoundID);
        return;
    }

    if (!SoundLibrary.Contains(SoundID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound ID not found: %s"), *SoundID);
        return;
    }

    const FAudio_SoundEntry& SoundEntry = SoundLibrary[SoundID];
    
    if (!SoundEntry.SoundAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound asset is null for ID: %s"), *SoundID);
        return;
    }

    // Stop existing sound if already playing
    if (ActiveSounds.Contains(SoundID))
    {
        StopSound(SoundID);
    }

    UAudioComponent* AudioComp = CreateAudioComponent(SoundEntry);
    if (AudioComp)
    {
        AudioComp->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepWorldTransform);
        
        float FinalVolume = SoundEntry.Volume * VolumeMultiplier;
        if (CategoryVolumes.Contains(SoundEntry.Category))
        {
            FinalVolume *= CategoryVolumes[SoundEntry.Category];
        }
        
        AudioComp->SetVolumeMultiplier(FinalVolume);
        AudioComp->SetPitchMultiplier(SoundEntry.Pitch);
        AudioComp->Play();
        
        ActiveSounds.Add(SoundID, AudioComp);
    }
}

void UAudio_SoundManager::StopSound(const FString& SoundID)
{
    if (ActiveSounds.Contains(SoundID))
    {
        UAudioComponent* AudioComp = ActiveSounds[SoundID];
        if (AudioComp && IsValid(AudioComp))
        {
            AudioComp->Stop();
            AudioComp->DestroyComponent();
        }
        ActiveSounds.Remove(SoundID);
    }
}

void UAudio_SoundManager::StopAllSounds()
{
    for (auto& SoundPair : ActiveSounds)
    {
        UAudioComponent* AudioComp = SoundPair.Value;
        if (AudioComp && IsValid(AudioComp))
        {
            AudioComp->Stop();
            AudioComp->DestroyComponent();
        }
    }
    ActiveSounds.Empty();
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
    
    // Update active sounds of this category
    for (auto& SoundPair : ActiveSounds)
    {
        const FString& SoundID = SoundPair.Key;
        UAudioComponent* AudioComp = SoundPair.Value;
        
        if (SoundLibrary.Contains(SoundID) && AudioComp && IsValid(AudioComp))
        {
            const FAudio_SoundEntry& SoundEntry = SoundLibrary[SoundID];
            if (SoundEntry.Category == Category)
            {
                float NewVolume = SoundEntry.Volume * CategoryVolumes[Category];
                AudioComp->SetVolumeMultiplier(NewVolume);
            }
        }
    }
}

void UAudio_SoundManager::RegisterSound(const FAudio_SoundEntry& SoundEntry)
{
    if (SoundEntry.SoundID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register sound with empty ID"));
        return;
    }
    
    SoundLibrary.Add(SoundEntry.SoundID, SoundEntry);
    UE_LOG(LogTemp, Log, TEXT("Registered sound: %s"), *SoundEntry.SoundID);
}

void UAudio_SoundManager::LoadSoundLibrary()
{
    // This would typically load from a data table or config file
    // For now, we'll register some default prehistoric sounds
    
    FAudio_SoundEntry ForestAmbient;
    ForestAmbient.SoundID = "ForestAmbient";
    ForestAmbient.Category = EAudio_SoundCategory::Ambient;
    ForestAmbient.bLooping = true;
    ForestAmbient.Volume = 0.6f;
    RegisterSound(ForestAmbient);
    
    FAudio_SoundEntry DinoFootsteps;
    DinoFootsteps.SoundID = "DinoFootsteps";
    DinoFootsteps.Category = EAudio_SoundCategory::SFX;
    DinoFootsteps.Volume = 0.8f;
    RegisterSound(DinoFootsteps);
    
    FAudio_SoundEntry TribalDrums;
    TribalDrums.SoundID = "TribalDrums";
    TribalDrums.Category = EAudio_SoundCategory::Music;
    TribalDrums.bLooping = true;
    TribalDrums.Volume = 0.5f;
    RegisterSound(TribalDrums);
    
    FAudio_SoundEntry WindTrees;
    WindTrees.SoundID = "WindTrees";
    WindTrees.Category = EAudio_SoundCategory::Ambient;
    WindTrees.bLooping = true;
    WindTrees.Volume = 0.4f;
    RegisterSound(WindTrees);
}

bool UAudio_SoundManager::IsSoundPlaying(const FString& SoundID) const
{
    if (ActiveSounds.Contains(SoundID))
    {
        UAudioComponent* AudioComp = ActiveSounds[SoundID];
        return AudioComp && IsValid(AudioComp) && AudioComp->IsPlaying();
    }
    return false;
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // Initialize any default sound configurations here
    UE_LOG(LogTemp, Log, TEXT("Default sounds initialized"));
}

void UAudio_SoundManager::InitializeCategoryVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::SFX, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Voice, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.8f);
}

UAudioComponent* UAudio_SoundManager::CreateAudioComponent(const FAudio_SoundEntry& SoundEntry)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UAudioComponent* AudioComp = NewObject<UAudioComponent>(World);
    if (AudioComp)
    {
        AudioComp->SetSound(SoundEntry.SoundAsset.Get());
        AudioComp->bAutoActivate = false;
        AudioComp->bIsUISound = !SoundEntry.bIs3D;
        
        if (SoundEntry.bLooping)
        {
            AudioComp->bAutoDestroy = false;
        }
        
        AudioComp->RegisterComponent();
    }
    
    return AudioComp;
}