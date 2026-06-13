#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    // Initialize category volumes
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::SFX, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Ambience, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::Dialogue, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.9f);
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    LoadSoundLibrary();
    InitializeDefaultSounds();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager initialized"));
}

void UAudio_SoundManager::Deinitialize()
{
    // Clean up active sounds
    for (auto& SoundPair : ActiveSounds)
    {
        if (SoundPair.Value && IsValid(SoundPair.Value))
        {
            SoundPair.Value->Stop();
            SoundPair.Value->DestroyComponent();
        }
    }
    ActiveSounds.Empty();
    
    Super::Deinitialize();
}

void UAudio_SoundManager::PlaySound(const FString& SoundID, AActor* SourceActor, FVector Location)
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

    // Stop existing sound if already playing and not looping
    if (ActiveSounds.Contains(SoundID))
    {
        UAudioComponent* ExistingComponent = ActiveSounds[SoundID];
        if (ExistingComponent && IsValid(ExistingComponent))
        {
            if (!SoundEntry.bLoop)
            {
                ExistingComponent->Stop();
                ExistingComponent->DestroyComponent();
                ActiveSounds.Remove(SoundID);
            }
            else
            {
                return; // Already playing looped sound
            }
        }
    }

    UAudioComponent* AudioComponent = nullptr;
    
    if (SoundEntry.b3D && SourceActor)
    {
        AudioComponent = UGameplayStatics::SpawnSoundAttached(
            SoundEntry.SoundAsset.LoadSynchronous(),
            SourceActor->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }
    else if (SoundEntry.b3D)
    {
        AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(),
            SoundEntry.SoundAsset.LoadSynchronous(),
            Location
        );
    }
    else
    {
        AudioComponent = UGameplayStatics::SpawnSound2D(
            GetWorld(),
            SoundEntry.SoundAsset.LoadSynchronous()
        );
    }

    if (AudioComponent)
    {
        float EffectiveVolume = GetEffectiveVolume(SoundEntry);
        AudioComponent->SetVolumeMultiplier(EffectiveVolume);
        AudioComponent->SetPitchMultiplier(SoundEntry.Pitch);
        
        if (SoundEntry.bLoop)
        {
            ActiveSounds.Add(SoundID, AudioComponent);
        }
    }
}

void UAudio_SoundManager::StopSound(const FString& SoundID)
{
    if (ActiveSounds.Contains(SoundID))
    {
        UAudioComponent* AudioComponent = ActiveSounds[SoundID];
        if (AudioComponent && IsValid(AudioComponent))
        {
            AudioComponent->Stop();
            AudioComponent->DestroyComponent();
        }
        ActiveSounds.Remove(SoundID);
    }
}

void UAudio_SoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active sounds
    for (auto& SoundPair : ActiveSounds)
    {
        if (SoundPair.Value && IsValid(SoundPair.Value))
        {
            const FString& SoundID = SoundPair.Key;
            if (SoundLibrary.Contains(SoundID))
            {
                float EffectiveVolume = GetEffectiveVolume(SoundLibrary[SoundID]);
                SoundPair.Value->SetVolumeMultiplier(EffectiveVolume);
            }
        }
    }
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
    
    // Update active sounds of this category
    for (auto& SoundPair : ActiveSounds)
    {
        if (SoundPair.Value && IsValid(SoundPair.Value))
        {
            const FString& SoundID = SoundPair.Key;
            if (SoundLibrary.Contains(SoundID))
            {
                const FAudio_SoundEntry& Entry = SoundLibrary[SoundID];
                if (Entry.Category == Category)
                {
                    float EffectiveVolume = GetEffectiveVolume(Entry);
                    SoundPair.Value->SetVolumeMultiplier(EffectiveVolume);
                }
            }
        }
    }
}

void UAudio_SoundManager::RegisterSound(const FAudio_SoundEntry& SoundEntry)
{
    SoundLibrary.Add(SoundEntry.SoundID, SoundEntry);
}

void UAudio_SoundManager::LoadSoundLibrary()
{
    // This would typically load from a data table or config file
    // For now, we'll register some default sounds programmatically
}

bool UAudio_SoundManager::IsSoundPlaying(const FString& SoundID) const
{
    if (ActiveSounds.Contains(SoundID))
    {
        UAudioComponent* AudioComponent = ActiveSounds[SoundID];
        return AudioComponent && IsValid(AudioComponent) && AudioComponent->IsPlaying();
    }
    return false;
}

void UAudio_SoundManager::PauseAllSounds()
{
    for (auto& SoundPair : ActiveSounds)
    {
        if (SoundPair.Value && IsValid(SoundPair.Value))
        {
            SoundPair.Value->SetPaused(true);
        }
    }
}

void UAudio_SoundManager::ResumeAllSounds()
{
    for (auto& SoundPair : ActiveSounds)
    {
        if (SoundPair.Value && IsValid(SoundPair.Value))
        {
            SoundPair.Value->SetPaused(false);
        }
    }
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // Register default prehistoric survival sounds
    FAudio_SoundEntry TRexRoar;
    TRexRoar.SoundID = TEXT("TRex_Roar");
    TRexRoar.Category = EAudio_SoundCategory::SFX;
    TRexRoar.Volume = 1.0f;
    TRexRoar.b3D = true;
    RegisterSound(TRexRoar);

    FAudio_SoundEntry ForestAmbience;
    ForestAmbience.SoundID = TEXT("Forest_Ambience");
    ForestAmbience.Category = EAudio_SoundCategory::Ambience;
    ForestAmbience.Volume = 0.5f;
    ForestAmbience.bLoop = true;
    ForestAmbience.b3D = false;
    RegisterSound(ForestAmbience);

    FAudio_SoundEntry Footsteps;
    Footsteps.SoundID = TEXT("Player_Footsteps");
    Footsteps.Category = EAudio_SoundCategory::SFX;
    Footsteps.Volume = 0.7f;
    Footsteps.b3D = true;
    RegisterSound(Footsteps);

    FAudio_SoundEntry HeartbeatTension;
    HeartbeatTension.SoundID = TEXT("Heartbeat_Tension");
    HeartbeatTension.Category = EAudio_SoundCategory::SFX;
    HeartbeatTension.Volume = 0.8f;
    HeartbeatTension.bLoop = true;
    HeartbeatTension.b3D = false;
    RegisterSound(HeartbeatTension);
}

float UAudio_SoundManager::GetEffectiveVolume(const FAudio_SoundEntry& SoundEntry) const
{
    float CategoryVolume = CategoryVolumes.Contains(SoundEntry.Category) ? 
        CategoryVolumes[SoundEntry.Category] : 1.0f;
    
    return MasterVolume * CategoryVolume * SoundEntry.Volume;
}