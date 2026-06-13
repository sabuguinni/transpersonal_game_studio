#include "Audio_SoundManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    SetupCategoryVolumes();
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    SetupCategoryVolumes();
    InitializeDefaultSounds();
    LoadSoundLibrary();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager initialized with %d sounds"), SoundLibrary.Num());
}

void UAudio_SoundManager::Deinitialize()
{
    // Stop all active sounds
    for (auto& SoundPair : ActiveSounds)
    {
        if (SoundPair.Value && IsValid(SoundPair.Value))
        {
            SoundPair.Value->Stop();
        }
    }
    ActiveSounds.Empty();
    
    Super::Deinitialize();
}

void UAudio_SoundManager::SetupCategoryVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Footsteps, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::DinosaurVocals, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Combat, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::Narration, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Weather, 0.5f);
    CategoryVolumes.Add(EAudio_SoundCategory::Fire, 0.7f);
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // Register default prehistoric sounds
    FAudio_SoundEntry ForestAmbient;
    ForestAmbient.SoundID = TEXT("forest_ambient");
    ForestAmbient.Category = EAudio_SoundCategory::Ambient;
    ForestAmbient.bLooping = true;
    ForestAmbient.Volume = 0.6f;
    SoundLibrary.Add(ForestAmbient);

    FAudio_SoundEntry TRexFootsteps;
    TRexFootsteps.SoundID = TEXT("trex_footsteps");
    TRexFootsteps.Category = EAudio_SoundCategory::Footsteps;
    TRexFootsteps.Volume = 1.0f;
    SoundLibrary.Add(TRexFootsteps);

    FAudio_SoundEntry TRexRoar;
    TRexRoar.SoundID = TEXT("trex_roar");
    TRexRoar.Category = EAudio_SoundCategory::DinosaurVocals;
    TRexRoar.Volume = 1.0f;
    SoundLibrary.Add(TRexRoar);

    FAudio_SoundEntry RaptorScreech;
    RaptorScreech.SoundID = TEXT("raptor_screech");
    RaptorScreech.Category = EAudio_SoundCategory::DinosaurVocals;
    RaptorScreech.Volume = 0.9f;
    SoundLibrary.Add(RaptorScreech);

    FAudio_SoundEntry CampfireCrackle;
    CampfireCrackle.SoundID = TEXT("campfire_crackle");
    CampfireCrackle.Category = EAudio_SoundCategory::Fire;
    CampfireCrackle.bLooping = true;
    CampfireCrackle.Volume = 0.7f;
    SoundLibrary.Add(CampfireCrackle);

    FAudio_SoundEntry PlayerFootsteps;
    PlayerFootsteps.SoundID = TEXT("player_footsteps");
    PlayerFootsteps.Category = EAudio_SoundCategory::Footsteps;
    PlayerFootsteps.Volume = 0.5f;
    SoundLibrary.Add(PlayerFootsteps);

    FAudio_SoundEntry WindAmbient;
    WindAmbient.SoundID = TEXT("wind_ambient");
    WindAmbient.Category = EAudio_SoundCategory::Weather;
    WindAmbient.bLooping = true;
    WindAmbient.Volume = 0.4f;
    SoundLibrary.Add(WindAmbient);

    FAudio_SoundEntry SurvivalNarration;
    SurvivalNarration.SoundID = TEXT("survival_narration");
    SurvivalNarration.Category = EAudio_SoundCategory::Narration;
    SurvivalNarration.Volume = 1.0f;
    SoundLibrary.Add(SurvivalNarration);
}

void UAudio_SoundManager::LoadSoundLibrary()
{
    // Load sound assets from Content Browser
    // This would typically load from a data table or asset registry
    UE_LOG(LogTemp, Log, TEXT("Loading sound library with %d entries"), SoundLibrary.Num());
}

void UAudio_SoundManager::PlaySound(const FString& SoundID, FVector Location, float VolumeMultiplier)
{
    FAudio_SoundEntry* SoundEntry = SoundLibrary.FindByPredicate([&SoundID](const FAudio_SoundEntry& Entry)
    {
        return Entry.SoundID == SoundID;
    });

    if (!SoundEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound not found: %s"), *SoundID);
        return;
    }

    if (!SoundEntry->SoundAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound asset not valid for: %s"), *SoundID);
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Stop existing sound if playing
    if (ActiveSounds.Contains(SoundID))
    {
        StopSound(SoundID);
    }

    // Calculate final volume
    float CategoryVolume = CategoryVolumes.Contains(SoundEntry->Category) ? CategoryVolumes[SoundEntry->Category] : 1.0f;
    float FinalVolume = SoundEntry->Volume * VolumeMultiplier * CategoryVolume;

    // Create and play audio component
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        World,
        SoundEntry->SoundAsset.Get(),
        Location,
        FRotator::ZeroRotator,
        FinalVolume,
        SoundEntry->Pitch
    );

    if (AudioComp)
    {
        ActiveSounds.Add(SoundID, AudioComp);
        UE_LOG(LogTemp, Log, TEXT("Playing sound: %s at location %s"), *SoundID, *Location.ToString());
    }
}

void UAudio_SoundManager::PlaySoundAtLocation(const FString& SoundID, FVector Location, float VolumeMultiplier)
{
    PlaySound(SoundID, Location, VolumeMultiplier);
}

void UAudio_SoundManager::StopSound(const FString& SoundID)
{
    if (UAudioComponent** AudioCompPtr = ActiveSounds.Find(SoundID))
    {
        UAudioComponent* AudioComp = *AudioCompPtr;
        if (AudioComp && IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
        ActiveSounds.Remove(SoundID);
        UE_LOG(LogTemp, Log, TEXT("Stopped sound: %s"), *SoundID);
    }
}

void UAudio_SoundManager::StopAllSoundsInCategory(EAudio_SoundCategory Category)
{
    TArray<FString> SoundsToStop;
    
    for (const auto& SoundPair : ActiveSounds)
    {
        FAudio_SoundEntry* SoundEntry = SoundLibrary.FindByPredicate([&SoundPair](const FAudio_SoundEntry& Entry)
        {
            return Entry.SoundID == SoundPair.Key;
        });
        
        if (SoundEntry && SoundEntry->Category == Category)
        {
            SoundsToStop.Add(SoundPair.Key);
        }
    }
    
    for (const FString& SoundID : SoundsToStop)
    {
        StopSound(SoundID);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Stopped %d sounds in category"), SoundsToStop.Num());
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
    UE_LOG(LogTemp, Log, TEXT("Set category volume to %f"), Volume);
}

void UAudio_SoundManager::RegisterSound(const FAudio_SoundEntry& SoundEntry)
{
    // Remove existing entry with same ID
    SoundLibrary.RemoveAll([&SoundEntry](const FAudio_SoundEntry& Entry)
    {
        return Entry.SoundID == SoundEntry.SoundID;
    });
    
    // Add new entry
    SoundLibrary.Add(SoundEntry);
    UE_LOG(LogTemp, Log, TEXT("Registered sound: %s"), *SoundEntry.SoundID);
}

bool UAudio_SoundManager::IsSoundPlaying(const FString& SoundID)
{
    if (UAudioComponent** AudioCompPtr = ActiveSounds.Find(SoundID))
    {
        UAudioComponent* AudioComp = *AudioCompPtr;
        return AudioComp && IsValid(AudioComp) && AudioComp->IsPlaying();
    }
    return false;
}

void UAudio_SoundManager::FadeInSound(const FString& SoundID, float FadeTime)
{
    if (UAudioComponent** AudioCompPtr = ActiveSounds.Find(SoundID))
    {
        UAudioComponent* AudioComp = *AudioCompPtr;
        if (AudioComp && IsValid(AudioComp))
        {
            AudioComp->FadeIn(FadeTime);
            UE_LOG(LogTemp, Log, TEXT("Fading in sound: %s over %f seconds"), *SoundID, FadeTime);
        }
    }
}

void UAudio_SoundManager::FadeOutSound(const FString& SoundID, float FadeTime)
{
    if (UAudioComponent** AudioCompPtr = ActiveSounds.Find(SoundID))
    {
        UAudioComponent* AudioComp = *AudioCompPtr;
        if (AudioComp && IsValid(AudioComp))
        {
            AudioComp->FadeOut(FadeTime, 0.0f);
            UE_LOG(LogTemp, Log, TEXT("Fading out sound: %s over %f seconds"), *SoundID, FadeTime);
        }
    }
}