#include "Audio_EnvironmentalSoundManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UAudio_EnvironmentalSoundManager::UAudio_EnvironmentalSoundManager()
{
    CurrentZone = EAudio_EnvironmentalZone::Forest;
    CurrentAmbientComponent = nullptr;
    MasterEnvironmentalVolume = 1.0f;
    ZoneTransitionRadius = 1000.0f;
}

void UAudio_EnvironmentalSoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeZoneConfigurations();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_EnvironmentalSoundManager initialized"));
}

void UAudio_EnvironmentalSoundManager::Deinitialize()
{
    if (CurrentAmbientComponent && IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->Stop();
        CurrentAmbientComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudio_EnvironmentalSoundManager::InitializeZoneConfigurations()
{
    ZoneConfigurations.Empty();
    
    // Forest zone configuration
    FAudio_EnvironmentalZoneData ForestZone;
    ForestZone.ZoneType = EAudio_EnvironmentalZone::Forest;
    ForestZone.Volume = 0.6f;
    ForestZone.FadeInTime = 3.0f;
    ForestZone.FadeOutTime = 2.0f;
    ZoneConfigurations.Add(ForestZone);
    
    // River zone configuration
    FAudio_EnvironmentalZoneData RiverZone;
    RiverZone.ZoneType = EAudio_EnvironmentalZone::River;
    RiverZone.Volume = 0.7f;
    RiverZone.FadeInTime = 2.0f;
    RiverZone.FadeOutTime = 2.0f;
    ZoneConfigurations.Add(RiverZone);
    
    // Plains zone configuration
    FAudio_EnvironmentalZoneData PlainsZone;
    PlainsZone.ZoneType = EAudio_EnvironmentalZone::Plains;
    PlainsZone.Volume = 0.4f;
    PlainsZone.FadeInTime = 4.0f;
    PlainsZone.FadeOutTime = 3.0f;
    ZoneConfigurations.Add(PlainsZone);
    
    // Cave zone configuration
    FAudio_EnvironmentalZoneData CaveZone;
    CaveZone.ZoneType = EAudio_EnvironmentalZone::Cave;
    CaveZone.Volume = 0.3f;
    CaveZone.FadeInTime = 1.5f;
    CaveZone.FadeOutTime = 1.5f;
    ZoneConfigurations.Add(CaveZone);
    
    // Swamp zone configuration
    FAudio_EnvironmentalZoneData SwampZone;
    SwampZone.ZoneType = EAudio_EnvironmentalZone::Swamp;
    SwampZone.Volume = 0.5f;
    SwampZone.FadeInTime = 2.5f;
    SwampZone.FadeOutTime = 2.5f;
    ZoneConfigurations.Add(SwampZone);
}

void UAudio_EnvironmentalSoundManager::SetEnvironmentalZone(EAudio_EnvironmentalZone NewZone, FVector PlayerLocation)
{
    if (NewZone == CurrentZone)
    {
        return;
    }
    
    FAudio_EnvironmentalZoneData* ZoneData = GetZoneConfiguration(NewZone);
    if (ZoneData)
    {
        TransitionToZone(*ZoneData, PlayerLocation);
        CurrentZone = NewZone;
        
        UE_LOG(LogTemp, Log, TEXT("Environmental zone changed to: %d"), (int32)NewZone);
    }
}

void UAudio_EnvironmentalSoundManager::UpdatePlayerLocation(FVector NewLocation)
{
    if (CurrentAmbientComponent && IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->SetWorldLocation(NewLocation);
    }
}

void UAudio_EnvironmentalSoundManager::SetMasterEnvironmentalVolume(float Volume)
{
    MasterEnvironmentalVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (CurrentAmbientComponent && IsValid(CurrentAmbientComponent))
    {
        FAudio_EnvironmentalZoneData* ZoneData = GetZoneConfiguration(CurrentZone);
        if (ZoneData)
        {
            float FinalVolume = ZoneData->Volume * MasterEnvironmentalVolume;
            CurrentAmbientComponent->SetVolumeMultiplier(FinalVolume);
        }
    }
}

void UAudio_EnvironmentalSoundManager::TransitionToZone(const FAudio_EnvironmentalZoneData& ZoneData, FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Fade out current ambient sound
    if (CurrentAmbientComponent && IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->FadeOut(ZoneData.FadeOutTime, 0.0f);
    }
    
    // Create new ambient component if we have a sound cue
    if (ZoneData.AmbientLoop.IsValid())
    {
        USoundCue* SoundCue = ZoneData.AmbientLoop.LoadSynchronous();
        if (SoundCue)
        {
            CurrentAmbientComponent = UGameplayStatics::SpawnSoundAtLocation(
                World,
                SoundCue,
                Location,
                FRotator::ZeroRotator,
                0.0f, // Start at 0 volume for fade in
                1.0f, // Pitch
                0.0f, // Start time
                nullptr, // Attenuation override
                nullptr, // Concurrency override
                true // Auto destroy
            );
            
            if (CurrentAmbientComponent)
            {
                float FinalVolume = ZoneData.Volume * MasterEnvironmentalVolume;
                CurrentAmbientComponent->FadeIn(ZoneData.FadeInTime, FinalVolume);
                CurrentAmbientComponent->SetUISound(false);
            }
        }
    }
}

FAudio_EnvironmentalZoneData* UAudio_EnvironmentalSoundManager::GetZoneConfiguration(EAudio_EnvironmentalZone Zone)
{
    for (FAudio_EnvironmentalZoneData& ZoneData : ZoneConfigurations)
    {
        if (ZoneData.ZoneType == Zone)
        {
            return &ZoneData;
        }
    }
    return nullptr;
}