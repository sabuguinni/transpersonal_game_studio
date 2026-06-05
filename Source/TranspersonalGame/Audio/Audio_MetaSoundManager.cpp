#include "Audio_MetaSoundManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

void UAudio_MetaSoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize category volumes
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Dinosaur, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Environment, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Combat, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::Narration, 1.0f);
    
    MasterVolume = 1.0f;
    
    LoadDefaultSounds();
    
    UE_LOG(LogTemp, Log, TEXT("Audio MetaSound Manager initialized"));
}

void UAudio_MetaSoundManager::Deinitialize()
{
    StopAllSounds();
    SoundRegistry.Empty();
    ActiveSounds.Empty();
    CategoryVolumes.Empty();
    
    Super::Deinitialize();
}

void UAudio_MetaSoundManager::PlaySound2D(const FString& SoundID, float VolumeMultiplier)
{
    const FAudio_SoundEntry* SoundEntry = SoundRegistry.Find(SoundID);
    if (!SoundEntry || !SoundEntry->SoundAsset.LoadSynchronous())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound not found: %s"), *SoundID);
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Stop existing sound with same ID
    StopSound(SoundID);
    
    float EffectiveVolume = GetEffectiveVolume(*SoundEntry, VolumeMultiplier);
    
    UAudioComponent* AudioComponent = UGameplayStatics::CreateSound2D(
        World,
        SoundEntry->SoundAsset.LoadSynchronous(),
        EffectiveVolume,
        SoundEntry->Pitch
    );
    
    if (AudioComponent)
    {
        ActiveSounds.Add(SoundID, AudioComponent);
        AudioComponent->Play();
    }
}

void UAudio_MetaSoundManager::PlaySound3D(const FString& SoundID, const FVector& Location, float VolumeMultiplier)
{
    const FAudio_SoundEntry* SoundEntry = SoundRegistry.Find(SoundID);
    if (!SoundEntry || !SoundEntry->SoundAsset.LoadSynchronous())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound not found: %s"), *SoundID);
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Stop existing sound with same ID
    StopSound(SoundID);
    
    float EffectiveVolume = GetEffectiveVolume(*SoundEntry, VolumeMultiplier);
    
    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        World,
        SoundEntry->SoundAsset.LoadSynchronous(),
        Location,
        FRotator::ZeroRotator,
        EffectiveVolume,
        SoundEntry->Pitch
    );
    
    if (AudioComponent)
    {
        ActiveSounds.Add(SoundID, AudioComponent);
    }
}

void UAudio_MetaSoundManager::PlaySoundAttached(const FString& SoundID, USceneComponent* AttachComponent, float VolumeMultiplier)
{
    if (!AttachComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("AttachComponent is null for sound: %s"), *SoundID);
        return;
    }
    
    const FAudio_SoundEntry* SoundEntry = SoundRegistry.Find(SoundID);
    if (!SoundEntry || !SoundEntry->SoundAsset.LoadSynchronous())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound not found: %s"), *SoundID);
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Stop existing sound with same ID
    StopSound(SoundID);
    
    float EffectiveVolume = GetEffectiveVolume(*SoundEntry, VolumeMultiplier);
    
    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAttached(
        SoundEntry->SoundAsset.LoadSynchronous(),
        AttachComponent,
        NAME_None,
        FVector::ZeroVector,
        EAttachLocation::KeepRelativeOffset,
        false,
        EffectiveVolume,
        SoundEntry->Pitch
    );
    
    if (AudioComponent)
    {
        ActiveSounds.Add(SoundID, AudioComponent);
    }
}

void UAudio_MetaSoundManager::StopSound(const FString& SoundID)
{
    UAudioComponent** AudioComponentPtr = ActiveSounds.Find(SoundID);
    if (AudioComponentPtr && *AudioComponentPtr)
    {
        (*AudioComponentPtr)->Stop();
        ActiveSounds.Remove(SoundID);
    }
}

void UAudio_MetaSoundManager::StopAllSounds()
{
    for (auto& SoundPair : ActiveSounds)
    {
        if (SoundPair.Value)
        {
            SoundPair.Value->Stop();
        }
    }
    ActiveSounds.Empty();
}

void UAudio_MetaSoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active sounds
    for (auto& SoundPair : ActiveSounds)
    {
        if (SoundPair.Value)
        {
            const FAudio_SoundEntry* SoundEntry = SoundRegistry.Find(SoundPair.Key);
            if (SoundEntry)
            {
                float EffectiveVolume = GetEffectiveVolume(*SoundEntry, 1.0f);
                SoundPair.Value->SetVolumeMultiplier(EffectiveVolume);
            }
        }
    }
}

void UAudio_MetaSoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
    
    // Update all active sounds in this category
    for (auto& SoundPair : ActiveSounds)
    {
        const FAudio_SoundEntry* SoundEntry = SoundRegistry.Find(SoundPair.Key);
        if (SoundEntry && SoundEntry->Category == Category && SoundPair.Value)
        {
            float EffectiveVolume = GetEffectiveVolume(*SoundEntry, 1.0f);
            SoundPair.Value->SetVolumeMultiplier(EffectiveVolume);
        }
    }
}

void UAudio_MetaSoundManager::RegisterSound(const FString& SoundID, USoundBase* SoundAsset, EAudio_SoundCategory Category)
{
    if (!SoundAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Trying to register null sound asset for ID: %s"), *SoundID);
        return;
    }
    
    FAudio_SoundEntry NewEntry;
    NewEntry.SoundID = SoundID;
    NewEntry.SoundAsset = SoundAsset;
    NewEntry.Category = Category;
    
    SoundRegistry.Add(SoundID, NewEntry);
    
    UE_LOG(LogTemp, Log, TEXT("Registered sound: %s"), *SoundID);
}

bool UAudio_MetaSoundManager::IsSoundPlaying(const FString& SoundID) const
{
    UAudioComponent* const* AudioComponentPtr = ActiveSounds.Find(SoundID);
    if (AudioComponentPtr && *AudioComponentPtr)
    {
        return (*AudioComponentPtr)->IsPlaying();
    }
    return false;
}

void UAudio_MetaSoundManager::LoadDefaultSounds()
{
    // Register default prehistoric sounds
    // These would be loaded from content browser in a real implementation
    
    UE_LOG(LogTemp, Log, TEXT("Loading default prehistoric audio library"));
    
    // Placeholder for sound registration - in real implementation these would load actual assets
    // RegisterSound("ambient_wind", LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/Ambient/Wind_Cue")), EAudio_SoundCategory::Ambient);
    // RegisterSound("trex_roar", LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/Dinosaurs/TRex_Roar_Cue")), EAudio_SoundCategory::Dinosaur);
}

float UAudio_MetaSoundManager::GetEffectiveVolume(const FAudio_SoundEntry& SoundEntry, float VolumeMultiplier) const
{
    float CategoryVolume = CategoryVolumes.FindRef(SoundEntry.Category);
    return SoundEntry.Volume * CategoryVolume * MasterVolume * VolumeMultiplier;
}