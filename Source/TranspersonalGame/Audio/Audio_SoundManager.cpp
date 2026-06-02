#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    MasterVolume = 1.0f;
    
    // Initialize default category volumes
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::SFX, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Voice, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::Dinosaur, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Environment, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Combat, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Crafting, 0.9f);
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager initialized"));
    
    // Clear any existing components
    ActiveAudioComponents.Empty();
}

void UAudio_SoundManager::Deinitialize()
{
    StopAllSounds();
    ActiveAudioComponents.Empty();
    SoundConfigs.Empty();
    
    Super::Deinitialize();
}

UAudioComponent* UAudio_SoundManager::PlaySound2D(USoundBase* Sound, float Volume, float Pitch)
{
    if (!Sound || !GetWorld())
    {
        return nullptr;
    }

    UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(
        GetWorld(),
        Sound,
        Volume * MasterVolume,
        Pitch
    );

    if (AudioComp)
    {
        ActiveAudioComponents.Add(AudioComp);
        CleanupFinishedComponents();
    }

    return AudioComp;
}

UAudioComponent* UAudio_SoundManager::PlaySoundAtLocation(USoundBase* Sound, FVector Location, float Volume, float Pitch)
{
    if (!Sound || !GetWorld())
    {
        return nullptr;
    }

    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        Sound,
        Location,
        FRotator::ZeroRotator,
        Volume * MasterVolume,
        Pitch
    );

    if (AudioComp)
    {
        ActiveAudioComponents.Add(AudioComp);
        CleanupFinishedComponents();
    }

    return AudioComp;
}

UAudioComponent* UAudio_SoundManager::PlaySoundAttached(USoundBase* Sound, USceneComponent* AttachComponent, float Volume, float Pitch)
{
    if (!Sound || !AttachComponent || !GetWorld())
    {
        return nullptr;
    }

    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAttached(
        Sound,
        AttachComponent,
        NAME_None,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        false,
        Volume * MasterVolume,
        Pitch
    );

    if (AudioComp)
    {
        ActiveAudioComponents.Add(AudioComp);
        CleanupFinishedComponents();
    }

    return AudioComp;
}

void UAudio_SoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active components
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->SetVolumeMultiplier(AudioComp->VolumeMultiplier * MasterVolume);
        }
    }
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
}

float UAudio_SoundManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    if (const float* FoundVolume = CategoryVolumes.Find(Category))
    {
        return *FoundVolume;
    }
    return 1.0f;
}

void UAudio_SoundManager::StopAllSounds()
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

void UAudio_SoundManager::StopSoundsByCategory(EAudio_SoundCategory Category)
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (IsValid(AudioComp))
        {
            // Note: In a full implementation, we'd need to track category per component
            // For now, this is a placeholder for the system architecture
            AudioComp->Stop();
            ActiveAudioComponents.RemoveAt(i);
        }
    }
}

void UAudio_SoundManager::RegisterSoundConfig(const FString& SoundName, const FAudio_SoundConfig& Config)
{
    SoundConfigs.Add(SoundName, Config);
}

UAudioComponent* UAudio_SoundManager::PlayConfiguredSound(const FString& SoundName, FVector Location)
{
    const FAudio_SoundConfig* Config = SoundConfigs.Find(SoundName);
    if (!Config || !Config->SoundAsset.IsValid())
    {
        return nullptr;
    }

    USoundBase* Sound = Config->SoundAsset.LoadSynchronous();
    if (!Sound)
    {
        return nullptr;
    }

    float FinalVolume = CalculateFinalVolume(Config->Category, Config->Volume);
    
    if (Config->bIs3D && Location != FVector::ZeroVector)
    {
        return PlaySoundAtLocation(Sound, Location, FinalVolume, Config->Pitch);
    }
    else
    {
        return PlaySound2D(Sound, FinalVolume, Config->Pitch);
    }
}

void UAudio_SoundManager::CleanupFinishedComponents()
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

float UAudio_SoundManager::CalculateFinalVolume(EAudio_SoundCategory Category, float BaseVolume) const
{
    float CategoryVolume = GetCategoryVolume(Category);
    return BaseVolume * CategoryVolume * MasterVolume;
}