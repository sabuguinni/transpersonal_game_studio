#include "Audio_SystemManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAudio_SystemManager::UAudio_SystemManager()
{
    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentThreatLevel = EAudio_ThreatLevel::Peaceful;
    CurrentTimeOfDay = 0.5f; // Noon
}

void UAudio_SystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize audio components
    UWorld* World = GetWorld();
    if (World)
    {
        // Create ambient audio component
        AmbientAudioComponent = NewObject<UAudioComponent>(this);
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->bAutoActivate = false;
            AmbientAudioComponent->SetVolumeMultiplier(0.5f);
        }

        // Create music audio component
        MusicAudioComponent = NewObject<UAudioComponent>(this);
        if (MusicAudioComponent)
        {
            MusicAudioComponent->bAutoActivate = false;
            MusicAudioComponent->SetVolumeMultiplier(0.3f);
        }

        // Create threat audio component
        ThreatAudioComponent = NewObject<UAudioComponent>(this);
        if (ThreatAudioComponent)
        {
            ThreatAudioComponent->bAutoActivate = false;
            ThreatAudioComponent->SetVolumeMultiplier(0.4f);
        }

        // Initialize default biome settings
        FAudio_BiomeSettings ForestSettings;
        ForestSettings.Volume = 0.5f;
        ForestSettings.FadeInTime = 2.0f;
        ForestSettings.FadeOutTime = 1.5f;
        BiomeSettings.Add(EAudio_BiomeType::Forest, ForestSettings);

        FAudio_BiomeSettings PlainsSettings;
        PlainsSettings.Volume = 0.4f;
        PlainsSettings.FadeInTime = 3.0f;
        PlainsSettings.FadeOutTime = 2.0f;
        BiomeSettings.Add(EAudio_BiomeType::Plains, PlainsSettings);

        FAudio_BiomeSettings RiverSettings;
        RiverSettings.Volume = 0.6f;
        RiverSettings.FadeInTime = 1.5f;
        RiverSettings.FadeOutTime = 1.0f;
        BiomeSettings.Add(EAudio_BiomeType::River, RiverSettings);

        // Initialize threat settings
        FAudio_ThreatSettings PeacefulSettings;
        PeacefulSettings.IntensityMultiplier = 0.5f;
        PeacefulSettings.HeartbeatVolume = 0.0f;
        ThreatSettings.Add(EAudio_ThreatLevel::Peaceful, PeacefulSettings);

        FAudio_ThreatSettings DangerSettings;
        DangerSettings.IntensityMultiplier = 1.5f;
        DangerSettings.HeartbeatVolume = 0.3f;
        ThreatSettings.Add(EAudio_ThreatLevel::Danger, DangerSettings);

        FAudio_ThreatSettings CombatSettings;
        CombatSettings.IntensityMultiplier = 2.0f;
        CombatSettings.HeartbeatVolume = 0.5f;
        ThreatSettings.Add(EAudio_ThreatLevel::Combat, CombatSettings);

        UE_LOG(LogTemp, Log, TEXT("Audio System Manager initialized"));
    }
}

void UAudio_SystemManager::Deinitialize()
{
    // Clean up audio components
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Stop();
        AmbientAudioComponent = nullptr;
    }

    if (MusicAudioComponent)
    {
        MusicAudioComponent->Stop();
        MusicAudioComponent = nullptr;
    }

    if (ThreatAudioComponent)
    {
        ThreatAudioComponent->Stop();
        ThreatAudioComponent = nullptr;
    }

    RegisteredZones.Empty();

    Super::Deinitialize();
}

void UAudio_SystemManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        CurrentBiome = NewBiome;
        UpdateAmbientAudio();
        UE_LOG(LogTemp, Log, TEXT("Audio biome changed to: %d"), (int32)NewBiome);
    }
}

void UAudio_SystemManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel != NewThreatLevel)
    {
        CurrentThreatLevel = NewThreatLevel;
        UpdateThreatAudio();
        UE_LOG(LogTemp, Log, TEXT("Threat level changed to: %d"), (int32)NewThreatLevel);
    }
}

void UAudio_SystemManager::PlayFootstepSound(const FVector& Location, bool bIsHeavy)
{
    // Play footstep sound at location
    UWorld* World = GetWorld();
    if (World)
    {
        // TODO: Load appropriate footstep sound based on surface type and weight
        // For now, just log the footstep
        UE_LOG(LogTemp, Log, TEXT("Footstep at location: %s, Heavy: %s"), 
               *Location.ToString(), bIsHeavy ? TEXT("Yes") : TEXT("No"));
    }
}

void UAudio_SystemManager::PlayDinosaurSound(const FString& DinosaurType, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (World)
    {
        // TODO: Load and play dinosaur-specific sounds
        UE_LOG(LogTemp, Log, TEXT("Dinosaur sound: %s at location: %s"), 
               *DinosaurType, *Location.ToString());
    }
}

void UAudio_SystemManager::SetTimeOfDay(float TimeRatio)
{
    CurrentTimeOfDay = FMath::Clamp(TimeRatio, 0.0f, 1.0f);
    UpdateTimeBasedAudio();
}

void UAudio_SystemManager::RegisterAudioZone(class AAudio_Zone* Zone)
{
    if (Zone && !RegisteredZones.Contains(Zone))
    {
        RegisteredZones.Add(Zone);
        UE_LOG(LogTemp, Log, TEXT("Audio zone registered"));
    }
}

void UAudio_SystemManager::UnregisterAudioZone(class AAudio_Zone* Zone)
{
    if (Zone)
    {
        RegisteredZones.Remove(Zone);
        UE_LOG(LogTemp, Log, TEXT("Audio zone unregistered"));
    }
}

void UAudio_SystemManager::UpdateAmbientAudio()
{
    if (!AmbientAudioComponent)
        return;

    const FAudio_BiomeSettings* Settings = BiomeSettings.Find(CurrentBiome);
    if (Settings)
    {
        // Fade out current audio
        FadeAudioComponent(AmbientAudioComponent, 0.0f, Settings->FadeOutTime);

        // TODO: Load and play new biome ambient sound
        // For now, just adjust volume
        FadeAudioComponent(AmbientAudioComponent, Settings->Volume, Settings->FadeInTime);
    }
}

void UAudio_SystemManager::UpdateThreatAudio()
{
    if (!ThreatAudioComponent)
        return;

    const FAudio_ThreatSettings* Settings = ThreatSettings.Find(CurrentThreatLevel);
    if (Settings)
    {
        float TargetVolume = Settings->IntensityMultiplier * 0.3f;
        FadeAudioComponent(ThreatAudioComponent, TargetVolume, 1.0f);

        // TODO: Trigger heartbeat audio if needed
        if (Settings->HeartbeatVolume > 0.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("Heartbeat audio triggered with volume: %f"), Settings->HeartbeatVolume);
        }
    }
}

void UAudio_SystemManager::UpdateTimeBasedAudio()
{
    // Adjust audio based on time of day
    float NightMultiplier = 1.0f;
    
    // Night time (0.0-0.2 and 0.8-1.0) should be quieter
    if (CurrentTimeOfDay < 0.2f || CurrentTimeOfDay > 0.8f)
    {
        NightMultiplier = 0.6f; // Quieter at night
    }

    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(0.5f * NightMultiplier);
    }

    UE_LOG(LogTemp, Log, TEXT("Time-based audio updated. Time: %f, Multiplier: %f"), 
           CurrentTimeOfDay, NightMultiplier);
}

void UAudio_SystemManager::FadeAudioComponent(UAudioComponent* Component, float TargetVolume, float FadeTime)
{
    if (!Component)
        return;

    // Simple immediate volume change for now
    // TODO: Implement smooth fade transition
    Component->SetVolumeMultiplier(TargetVolume);
    
    UE_LOG(LogTemp, Log, TEXT("Audio component volume set to: %f"), TargetVolume);
}