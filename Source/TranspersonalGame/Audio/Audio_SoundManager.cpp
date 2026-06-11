#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    InitializeCategoryVolumes();
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeCategoryVolumes();
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager initialized"));
}

void UAudio_SoundManager::Deinitialize()
{
    // Stop all active audio components
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
    }
    ActiveAudioComponents.Empty();
    
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager deinitialized"));
}

void UAudio_SoundManager::InitializeCategoryVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Footsteps, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Combat, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Crafting, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Voice, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.6f);
}

void UAudio_SoundManager::PlaySound2D(USoundBase* Sound, float Volume, float Pitch)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaySound2D: Sound is null"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaySound2D: World is null"));
        return;
    }

    UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(World, Sound, Volume, Pitch);
    if (AudioComp)
    {
        ActiveAudioComponents.Add(AudioComp);
        CleanupFinishedComponents();
    }
}

void UAudio_SoundManager::PlaySound3D(USoundBase* Sound, FVector Location, float Volume, float Pitch)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaySound3D: Sound is null"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaySound3D: World is null"));
        return;
    }

    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(World, Sound, Location, FRotator::ZeroRotator, Volume, Pitch);
    if (AudioComp)
    {
        ActiveAudioComponents.Add(AudioComp);
        CleanupFinishedComponents();
    }
}

void UAudio_SoundManager::PlaySoundAtActor(USoundBase* Sound, AActor* Actor, float Volume, float Pitch)
{
    if (!Sound || !Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaySoundAtActor: Sound or Actor is null"));
        return;
    }

    PlaySound3D(Sound, Actor->GetActorLocation(), Volume, Pitch);
}

void UAudio_SoundManager::StopAllSoundsOfCategory(EAudio_SoundCategory Category)
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (IsValid(AudioComp))
        {
            // Note: In a full implementation, we'd store category info with each component
            // For now, this is a basic implementation
            AudioComp->Stop();
            ActiveAudioComponents.RemoveAt(i);
        }
    }
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
}

float UAudio_SoundManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    const float* VolumePtr = CategoryVolumes.Find(Category);
    return VolumePtr ? *VolumePtr : 1.0f;
}

void UAudio_SoundManager::RegisterSound(const FString& SoundName, const FAudio_SoundEntry& SoundEntry)
{
    RegisteredSounds.Add(SoundName, SoundEntry);
    UE_LOG(LogTemp, Log, TEXT("Registered sound: %s"), *SoundName);
}

void UAudio_SoundManager::PlayRegisteredSound(const FString& SoundName, FVector Location)
{
    const FAudio_SoundEntry* SoundEntry = RegisteredSounds.Find(SoundName);
    if (!SoundEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayRegisteredSound: Sound '%s' not found"), *SoundName);
        return;
    }

    USoundBase* Sound = SoundEntry->Sound.LoadSynchronous();
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayRegisteredSound: Failed to load sound '%s'"), *SoundName);
        return;
    }

    float CategoryVolume = GetCategoryVolume(SoundEntry->Category);
    float FinalVolume = SoundEntry->Volume * CategoryVolume;

    if (SoundEntry->bIs3D && Location != FVector::ZeroVector)
    {
        PlaySound3D(Sound, Location, FinalVolume, SoundEntry->Pitch);
    }
    else
    {
        PlaySound2D(Sound, FinalVolume, SoundEntry->Pitch);
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