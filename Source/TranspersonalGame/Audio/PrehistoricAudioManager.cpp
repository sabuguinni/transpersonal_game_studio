#include "PrehistoricAudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

UPrehistoricAudioManager::UPrehistoricAudioManager()
{
    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    CurrentTimeOfDay = 0.5f; // Noon
    AmbienceAudioComponent = nullptr;
    MusicAudioComponent = nullptr;
    HeartbeatAudioComponent = nullptr;
}

void UPrehistoricAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("PrehistoricAudioManager: Initializing audio subsystem"));
    
    // Create persistent audio components
    UWorld* World = GetWorld();
    if (World)
    {
        // Create ambience audio component
        AmbienceAudioComponent = NewObject<UAudioComponent>(this);
        if (AmbienceAudioComponent)
        {
            AmbienceAudioComponent->bAutoActivate = false;
            AmbienceAudioComponent->SetVolumeMultiplier(AmbienceSettings.BaseVolume);
            AmbienceAudioComponent->RegisterComponent();
        }

        // Create music audio component
        MusicAudioComponent = NewObject<UAudioComponent>(this);
        if (MusicAudioComponent)
        {
            MusicAudioComponent->bAutoActivate = false;
            MusicAudioComponent->SetVolumeMultiplier(0.5f);
            MusicAudioComponent->RegisterComponent();
        }

        // Create heartbeat audio component
        HeartbeatAudioComponent = NewObject<UAudioComponent>(this);
        if (HeartbeatAudioComponent)
        {
            HeartbeatAudioComponent->bAutoActivate = false;
            HeartbeatAudioComponent->SetVolumeMultiplier(0.3f);
            HeartbeatAudioComponent->RegisterComponent();
        }
    }

    InitializeDefaultSoundProfiles();
}

void UPrehistoricAudioManager::Deinitialize()
{
    // Stop all active sounds
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->Stop();
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->Stop();
    }
    
    if (HeartbeatAudioComponent)
    {
        HeartbeatAudioComponent->Stop();
    }

    // Clean up active sound components
    for (UAudioComponent* AudioComp : ActiveSoundComponents)
    {
        if (AudioComp && IsValid(AudioComp))
        {
            AudioComp->Stop();
            AudioComp->DestroyComponent();
        }
    }
    ActiveSoundComponents.Empty();

    Super::Deinitialize();
}

void UPrehistoricAudioManager::SetBiomeAmbience(EAudio_BiomeType BiomeType, float FadeTime)
{
    if (CurrentBiome == BiomeType)
    {
        return; // Already in this biome
    }

    CurrentBiome = BiomeType;
    
    if (!AmbienceAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricAudioManager: AmbienceAudioComponent is null"));
        return;
    }

    USoundCue* NewAmbience = nullptr;
    
    switch (BiomeType)
    {
        case EAudio_BiomeType::Forest:
            NewAmbience = AmbienceSettings.ForestAmbience;
            break;
        case EAudio_BiomeType::Plains:
            NewAmbience = AmbienceSettings.PlainsAmbience;
            break;
        case EAudio_BiomeType::Swamp:
            NewAmbience = AmbienceSettings.SwampAmbience;
            break;
        default:
            NewAmbience = AmbienceSettings.ForestAmbience; // Default fallback
            break;
    }

    if (NewAmbience)
    {
        AmbienceAudioComponent->SetSound(NewAmbience);
        AmbienceAudioComponent->FadeIn(FadeTime, AmbienceSettings.BaseVolume);
        
        UE_LOG(LogTemp, Log, TEXT("PrehistoricAudioManager: Switched to %s biome ambience"), 
               *UEnum::GetValueAsString(BiomeType));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricAudioManager: No ambience sound for biome type"));
    }
}

void UPrehistoricAudioManager::SetThreatLevel(EAudio_ThreatLevel ThreatLevel)
{
    if (CurrentThreatLevel == ThreatLevel)
    {
        return;
    }

    EAudio_ThreatLevel PreviousThreatLevel = CurrentThreatLevel;
    CurrentThreatLevel = ThreatLevel;

    // Adjust ambience volume based on threat level
    float VolumeMultiplier = 1.0f;
    switch (ThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:
            VolumeMultiplier = 1.0f;
            break;
        case EAudio_ThreatLevel::Cautious:
            VolumeMultiplier = 0.7f;
            break;
        case EAudio_ThreatLevel::Danger:
            VolumeMultiplier = 0.4f;
            break;
        case EAudio_ThreatLevel::Panic:
            VolumeMultiplier = 0.1f;
            break;
    }

    if (AmbienceAudioComponent)
    {
        float TargetVolume = AmbienceSettings.BaseVolume * VolumeMultiplier;
        AmbienceAudioComponent->SetVolumeMultiplier(TargetVolume);
    }

    UE_LOG(LogTemp, Log, TEXT("PrehistoricAudioManager: Threat level changed to %s"), 
           *UEnum::GetValueAsString(ThreatLevel));
}

void UPrehistoricAudioManager::UpdateTimeOfDay(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 1.0f);
    
    // Adjust audio based on time of day
    // Night time (0.0-0.2 and 0.8-1.0) should be quieter and more ominous
    bool bIsNight = (TimeOfDay < 0.2f || TimeOfDay > 0.8f);
    
    if (AmbienceAudioComponent)
    {
        float TimeVolumeMultiplier = bIsNight ? 0.6f : 1.0f;
        float CurrentVolume = AmbienceAudioComponent->VolumeMultiplier;
        float TargetVolume = AmbienceSettings.BaseVolume * TimeVolumeMultiplier;
        
        // Smooth transition
        AmbienceAudioComponent->SetVolumeMultiplier(FMath::Lerp(CurrentVolume, TargetVolume, 0.1f));
    }
}

void UPrehistoricAudioManager::PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location)
{
    FAudio_DinosaurSoundProfile* SoundProfile = DinosaurSoundProfiles.Find(DinosaurType);
    if (!SoundProfile)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricAudioManager: No sound profile found for dinosaur type: %s"), *DinosaurType);
        return;
    }

    USoundCue* SoundToPlay = nullptr;
    
    if (SoundType == "Idle")
    {
        SoundToPlay = SoundProfile->IdleSound;
    }
    else if (SoundType == "Alert")
    {
        SoundToPlay = SoundProfile->AlertSound;
    }
    else if (SoundType == "Attack")
    {
        SoundToPlay = SoundProfile->AttackSound;
    }
    else if (SoundType == "Footstep")
    {
        SoundToPlay = SoundProfile->FootstepSound;
    }

    if (SoundToPlay)
    {
        UAudioComponent* AudioComp = CreateSpatialAudioComponent(Location);
        if (AudioComp)
        {
            AudioComp->SetSound(SoundToPlay);
            
            // Randomize pitch for variation
            float RandomPitch = FMath::RandRange(SoundProfile->MinPitch, SoundProfile->MaxPitch);
            AudioComp->SetPitchMultiplier(RandomPitch);
            
            // Set attenuation distance
            AudioComp->AttenuationSettings.FalloffDistance = SoundProfile->MaxAudibleDistance;
            
            AudioComp->Play();
            ActiveSoundComponents.Add(AudioComp);
            
            UE_LOG(LogTemp, Log, TEXT("PrehistoricAudioManager: Playing %s sound for %s at location %s"), 
                   *SoundType, *DinosaurType, *Location.ToString());
        }
    }
}

void UPrehistoricAudioManager::RegisterDinosaurSoundProfile(const FString& DinosaurType, const FAudio_DinosaurSoundProfile& SoundProfile)
{
    DinosaurSoundProfiles.Add(DinosaurType, SoundProfile);
    UE_LOG(LogTemp, Log, TEXT("PrehistoricAudioManager: Registered sound profile for %s"), *DinosaurType);
}

void UPrehistoricAudioManager::PlaySurvivalSound(const FString& SoundName, FVector Location, float Volume)
{
    // This would play survival-related sounds like crafting, fire, etc.
    // For now, we'll log the request
    UE_LOG(LogTemp, Log, TEXT("PrehistoricAudioManager: Playing survival sound %s at %s with volume %f"), 
           *SoundName, *Location.ToString(), Volume);
}

void UPrehistoricAudioManager::PlayPlayerHeartbeat(float Intensity)
{
    if (!HeartbeatAudioComponent)
    {
        return;
    }

    // Intensity affects both volume and pitch
    float HeartbeatVolume = FMath::Clamp(Intensity * 0.5f, 0.1f, 0.8f);
    float HeartbeatPitch = FMath::Clamp(1.0f + (Intensity * 0.3f), 0.8f, 1.5f);
    
    HeartbeatAudioComponent->SetVolumeMultiplier(HeartbeatVolume);
    HeartbeatAudioComponent->SetPitchMultiplier(HeartbeatPitch);
    
    if (!HeartbeatAudioComponent->IsPlaying())
    {
        HeartbeatAudioComponent->Play();
    }
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricAudioManager: Playing heartbeat with intensity %f"), Intensity);
}

void UPrehistoricAudioManager::StopPlayerHeartbeat()
{
    if (HeartbeatAudioComponent && HeartbeatAudioComponent->IsPlaying())
    {
        HeartbeatAudioComponent->FadeOut(1.0f, 0.0f);
    }
}

void UPrehistoricAudioManager::PlayWeatherSound(const FString& WeatherType, float Intensity)
{
    UE_LOG(LogTemp, Log, TEXT("PrehistoricAudioManager: Playing weather sound %s with intensity %f"), 
           *WeatherType, Intensity);
}

void UPrehistoricAudioManager::UpdateWindIntensity(float WindSpeed)
{
    // Adjust ambient wind sounds based on wind speed
    UE_LOG(LogTemp, Log, TEXT("PrehistoricAudioManager: Updating wind intensity to %f"), WindSpeed);
}

void UPrehistoricAudioManager::InitializeDefaultSoundProfiles()
{
    // Initialize default dinosaur sound profiles
    // These would normally load actual sound assets
    
    FAudio_DinosaurSoundProfile TRexProfile;
    TRexProfile.MinPitch = 0.7f;
    TRexProfile.MaxPitch = 1.0f;
    TRexProfile.MaxAudibleDistance = 5000.0f;
    RegisterDinosaurSoundProfile("TRex", TRexProfile);
    
    FAudio_DinosaurSoundProfile RaptorProfile;
    RaptorProfile.MinPitch = 1.0f;
    RaptorProfile.MaxPitch = 1.4f;
    RaptorProfile.MaxAudibleDistance = 2000.0f;
    RegisterDinosaurSoundProfile("Raptor", RaptorProfile);
    
    FAudio_DinosaurSoundProfile BrachiosaurusProfile;
    BrachiosaurusProfile.MinPitch = 0.5f;
    BrachiosaurusProfile.MaxPitch = 0.8f;
    BrachiosaurusProfile.MaxAudibleDistance = 8000.0f;
    RegisterDinosaurSoundProfile("Brachiosaurus", BrachiosaurusProfile);
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricAudioManager: Initialized default dinosaur sound profiles"));
}

void UPrehistoricAudioManager::CleanupInactiveSounds()
{
    // Remove finished audio components
    for (int32 i = ActiveSoundComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveSoundComponents[i];
        if (!AudioComp || !IsValid(AudioComp) || !AudioComp->IsPlaying())
        {
            if (AudioComp && IsValid(AudioComp))
            {
                AudioComp->DestroyComponent();
            }
            ActiveSoundComponents.RemoveAt(i);
        }
    }
}

UAudioComponent* UPrehistoricAudioManager::CreateSpatialAudioComponent(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UAudioComponent* AudioComp = NewObject<UAudioComponent>(this);
    if (AudioComp)
    {
        AudioComp->bAutoActivate = false;
        AudioComp->SetWorldLocation(Location);
        AudioComp->RegisterComponent();
        
        // Clean up old components periodically
        CleanupInactiveSounds();
    }
    
    return AudioComp;
}