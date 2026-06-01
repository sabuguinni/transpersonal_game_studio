#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    // Initialize category volumes
    InitializeCategoryVolumes();
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Initialized"));
    
    // Initialize default sounds and category volumes
    InitializeDefaultSounds();
    InitializeCategoryVolumes();
}

void UAudio_SoundManager::Deinitialize()
{
    // Stop all ambient loops
    StopAllAmbientLoops(0.0f);
    
    // Clear all maps
    RegisteredSounds.Empty();
    ActiveAmbientLoops.Empty();
    CategoryVolumes.Empty();
    
    Super::Deinitialize();
}

void UAudio_SoundManager::PlaySoundAtLocation(const FString& SoundName, FVector Location, float VolumeMultiplier)
{
    FAudio_SoundEntry* SoundEntry = GetSoundEntry(SoundName);
    if (!SoundEntry || !SoundEntry->SoundAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Sound '%s' not found or invalid"), *SoundName);
        return;
    }

    USoundBase* SoundAsset = SoundEntry->SoundAsset.LoadSynchronous();
    if (!SoundAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Failed to load sound asset '%s'"), *SoundName);
        return;
    }

    float CategoryVolume = GetCategoryVolume(SoundEntry->Category);
    float FinalVolume = SoundEntry->Volume * VolumeMultiplier * CategoryVolume;

    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        SoundAsset,
        Location,
        FinalVolume,
        SoundEntry->Pitch
    );
}

void UAudio_SoundManager::PlaySound2D(const FString& SoundName, float VolumeMultiplier)
{
    FAudio_SoundEntry* SoundEntry = GetSoundEntry(SoundName);
    if (!SoundEntry || !SoundEntry->SoundAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Sound '%s' not found or invalid"), *SoundName);
        return;
    }

    USoundBase* SoundAsset = SoundEntry->SoundAsset.LoadSynchronous();
    if (!SoundAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Failed to load sound asset '%s'"), *SoundName);
        return;
    }

    float CategoryVolume = GetCategoryVolume(SoundEntry->Category);
    float FinalVolume = SoundEntry->Volume * VolumeMultiplier * CategoryVolume;

    UGameplayStatics::PlaySound2D(
        GetWorld(),
        SoundAsset,
        FinalVolume,
        SoundEntry->Pitch
    );
}

UAudioComponent* UAudio_SoundManager::PlaySoundAttached(const FString& SoundName, USceneComponent* AttachComponent, float VolumeMultiplier)
{
    if (!AttachComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: AttachComponent is null"));
        return nullptr;
    }

    FAudio_SoundEntry* SoundEntry = GetSoundEntry(SoundName);
    if (!SoundEntry || !SoundEntry->SoundAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Sound '%s' not found or invalid"), *SoundName);
        return nullptr;
    }

    USoundBase* SoundAsset = SoundEntry->SoundAsset.LoadSynchronous();
    if (!SoundAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Failed to load sound asset '%s'"), *SoundName);
        return nullptr;
    }

    float CategoryVolume = GetCategoryVolume(SoundEntry->Category);
    float FinalVolume = SoundEntry->Volume * VolumeMultiplier * CategoryVolume;

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAttached(
        SoundAsset,
        AttachComponent,
        NAME_None,
        FVector::ZeroVector,
        EAttachLocation::KeepRelativeOffset,
        false,
        FinalVolume,
        SoundEntry->Pitch
    );

    return AudioComponent;
}

void UAudio_SoundManager::StartAmbientLoop(const FString& SoundName, FVector Location, float FadeInTime)
{
    // Stop existing ambient loop with same name
    StopAmbientLoop(SoundName, 0.0f);

    FAudio_SoundEntry* SoundEntry = GetSoundEntry(SoundName);
    if (!SoundEntry || !SoundEntry->SoundAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Ambient sound '%s' not found or invalid"), *SoundName);
        return;
    }

    USoundBase* SoundAsset = SoundEntry->SoundAsset.LoadSynchronous();
    if (!SoundAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Failed to load ambient sound asset '%s'"), *SoundName);
        return;
    }

    float CategoryVolume = GetCategoryVolume(SoundEntry->Category);
    float FinalVolume = SoundEntry->Volume * CategoryVolume;

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        SoundAsset,
        Location,
        FRotator::ZeroRotator,
        FinalVolume,
        SoundEntry->Pitch
    );

    if (AudioComponent)
    {
        AudioComponent->bIsUISound = false;
        AudioComponent->bAutoDestroy = false;
        ActiveAmbientLoops.Add(SoundName, AudioComponent);

        // Fade in if requested
        if (FadeInTime > 0.0f)
        {
            AudioComponent->FadeIn(FadeInTime, FinalVolume);
        }
    }
}

void UAudio_SoundManager::StopAmbientLoop(const FString& SoundName, float FadeOutTime)
{
    UAudioComponent** AudioComponentPtr = ActiveAmbientLoops.Find(SoundName);
    if (AudioComponentPtr && *AudioComponentPtr && IsValid(*AudioComponentPtr))
    {
        UAudioComponent* AudioComponent = *AudioComponentPtr;
        
        if (FadeOutTime > 0.0f)
        {
            AudioComponent->FadeOut(FadeOutTime, 0.0f);
        }
        else
        {
            AudioComponent->Stop();
            AudioComponent->DestroyComponent();
        }
    }

    ActiveAmbientLoops.Remove(SoundName);
}

void UAudio_SoundManager::StopAllAmbientLoops(float FadeOutTime)
{
    for (auto& Pair : ActiveAmbientLoops)
    {
        if (Pair.Value && IsValid(Pair.Value))
        {
            if (FadeOutTime > 0.0f)
            {
                Pair.Value->FadeOut(FadeOutTime, 0.0f);
            }
            else
            {
                Pair.Value->Stop();
                Pair.Value->DestroyComponent();
            }
        }
    }

    ActiveAmbientLoops.Empty();
}

void UAudio_SoundManager::RegisterSound(const FString& SoundName, const FAudio_SoundEntry& SoundEntry)
{
    RegisteredSounds.Add(SoundName, SoundEntry);
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Registered sound '%s'"), *SoundName);
}

void UAudio_SoundManager::UnregisterSound(const FString& SoundName)
{
    RegisteredSounds.Remove(SoundName);
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Unregistered sound '%s'"), *SoundName);
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

float UAudio_SoundManager::CalculateVolumeFromDistance(float Distance, float MaxDistance) const
{
    if (MaxDistance <= 0.0f)
    {
        return 1.0f;
    }

    float NormalizedDistance = FMath::Clamp(Distance / MaxDistance, 0.0f, 1.0f);
    return 1.0f - (NormalizedDistance * NormalizedDistance); // Quadratic falloff
}

FAudio_SoundEntry* UAudio_SoundManager::GetSoundEntry(const FString& SoundName)
{
    return RegisteredSounds.Find(SoundName);
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // This would be populated with actual sound assets in a real implementation
    // For now, we just log that we're ready to register sounds
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Ready to register default sounds"));
}

void UAudio_SoundManager::InitializeCategoryVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::DinosaurVocal, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::PlayerFootsteps, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::Crafting, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Weather, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::Combat, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.8f);
}