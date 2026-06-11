#include "SurvivalAudioLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

// Static member initialization
TMap<EEnvironmentalAudioType, FAudio_EnvironmentalSound> USurvivalAudioLibrary::EnvironmentalSoundMap;
FAudio_SurvivalFeedback USurvivalAudioLibrary::SurvivalFeedbackSounds;
bool USurvivalAudioLibrary::bIsInitialized = false;

void USurvivalAudioLibrary::PlayEnvironmentalSound(const FVector& Location, EEnvironmentalAudioType SoundType, float VolumeMultiplier)
{
    if (!bIsInitialized)
    {
        InitializePrehistoricSoundLibrary();
    }

    if (!EnvironmentalSoundMap.Contains(SoundType))
    {
        UE_LOG(LogTemp, Warning, TEXT("No environmental sound found for type"));
        return;
    }

    const FAudio_EnvironmentalSound& SoundData = EnvironmentalSoundMap[SoundType];
    USoundCue* SoundToPlay = SoundData.SoundCue.LoadSynchronous();

    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GEngine->GetCurrentPlayWorld(),
            SoundToPlay,
            Location,
            SoundData.BaseVolume * VolumeMultiplier,
            1.0f, // Pitch
            0.0f, // Start time
            nullptr, // Attenuation override
            nullptr, // Concurrency settings
            nullptr  // Owner
        );

        UE_LOG(LogTemp, Log, TEXT("Played environmental sound at location %s"), *Location.ToString());
    }
}

void USurvivalAudioLibrary::PlaySurvivalFeedbackSound(ESurvivalStat StatType, float StatValue, const FVector& PlayerLocation)
{
    if (!bIsInitialized)
    {
        InitializePrehistoricSoundLibrary();
    }

    // Only play feedback sounds when stat is critically low
    if (StatValue > SurvivalFeedbackSounds.IntensityThreshold)
    {
        return;
    }

    USoundCue* FeedbackSound = nullptr;
    float VolumeMultiplier = 1.0f - StatValue; // Lower stat = louder sound

    switch (StatType)
    {
        case ESurvivalStat::Health:
            FeedbackSound = SurvivalFeedbackSounds.HeartbeatSound.LoadSynchronous();
            break;
        case ESurvivalStat::Hunger:
            FeedbackSound = SurvivalFeedbackSounds.HungerSound.LoadSynchronous();
            break;
        case ESurvivalStat::Thirst:
            FeedbackSound = SurvivalFeedbackSounds.ThirstSound.LoadSynchronous();
            break;
        case ESurvivalStat::Fear:
            FeedbackSound = SurvivalFeedbackSounds.FearSound.LoadSynchronous();
            VolumeMultiplier = StatValue; // Higher fear = louder sound
            break;
        case ESurvivalStat::Stamina:
            FeedbackSound = SurvivalFeedbackSounds.BreathingSound.LoadSynchronous();
            break;
    }

    if (FeedbackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GEngine->GetCurrentPlayWorld(),
            FeedbackSound,
            PlayerLocation,
            VolumeMultiplier * 0.8f,
            1.0f,
            0.0f,
            nullptr,
            nullptr,
            nullptr
        );
    }
}

UAudioComponent* USurvivalAudioLibrary::CreateDynamicAudioComponent(AActor* Owner, USoundBase* Sound, bool bAutoPlay)
{
    if (!Owner || !Sound)
    {
        return nullptr;
    }

    UAudioComponent* AudioComp = Owner->CreateDefaultSubobject<UAudioComponent>(
        *FString::Printf(TEXT("DynamicAudio_%d"), FMath::RandRange(1000, 9999))
    );

    if (AudioComp)
    {
        AudioComp->SetSound(Sound);
        AudioComp->bAutoActivate = bAutoPlay;
        AudioComp->AttachToComponent(Owner->GetRootComponent(), 
            FAttachmentTransformRules::KeepRelativeTransform);

        if (bAutoPlay)
        {
            AudioComp->Play();
        }

        UE_LOG(LogTemp, Log, TEXT("Created dynamic audio component for %s"), *Owner->GetName());
    }

    return AudioComp;
}

void USurvivalAudioLibrary::UpdateAudioIntensityBasedOnThreat(float ThreatLevel, UAudioComponent* AudioComponent)
{
    if (!AudioComponent)
    {
        return;
    }

    // Increase volume and pitch based on threat level
    float VolumeMultiplier = FMath::Lerp(0.5f, 1.5f, ThreatLevel);
    float PitchMultiplier = FMath::Lerp(0.8f, 1.2f, ThreatLevel);

    AudioComponent->SetVolumeMultiplier(VolumeMultiplier);
    AudioComponent->SetPitchMultiplier(PitchMultiplier);
}

void USurvivalAudioLibrary::PlayFootstepSound(const FVector& Location, ETerrainType TerrainType, bool bIsRunning)
{
    USoundCue* FootstepSound = GetSoundForTerrainType(TerrainType, bIsRunning);
    
    if (FootstepSound)
    {
        float Volume = bIsRunning ? 0.8f : 0.5f;
        
        UGameplayStatics::PlaySoundAtLocation(
            GEngine->GetCurrentPlayWorld(),
            FootstepSound,
            Location,
            Volume,
            1.0f,
            0.0f,
            nullptr,
            nullptr,
            nullptr
        );
    }
}

void USurvivalAudioLibrary::PlayCraftingSound(ECraftingAction ActionType, const FVector& Location)
{
    USoundCue* CraftingSound = GetCraftingSoundForAction(ActionType);
    
    if (CraftingSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GEngine->GetCurrentPlayWorld(),
            CraftingSound,
            Location,
            0.7f,
            1.0f,
            0.0f,
            nullptr,
            nullptr,
            nullptr
        );
        
        UE_LOG(LogTemp, Log, TEXT("Played crafting sound for action at %s"), *Location.ToString());
    }
}

void USurvivalAudioLibrary::SetupAmbientAudioZone(const FVector& Center, float Radius, EEnvironmentalAudioType ZoneType)
{
    // This would create a persistent ambient audio zone
    // For now, just play the environmental sound
    PlayEnvironmentalSound(Center, ZoneType, 0.6f);
    
    UE_LOG(LogTemp, Log, TEXT("Setup ambient audio zone of type %d at %s with radius %f"), 
           (int32)ZoneType, *Center.ToString(), Radius);
}

void USurvivalAudioLibrary::StopAmbientSoundsInRadius(const FVector& Center, float Radius)
{
    // This would stop all ambient sounds within the specified radius
    // Implementation would require tracking active audio components
    UE_LOG(LogTemp, Log, TEXT("Stopping ambient sounds in radius %f around %s"), 
           Radius, *Center.ToString());
}

void USurvivalAudioLibrary::InitializePrehistoricSoundLibrary()
{
    if (bIsInitialized)
    {
        return;
    }

    LoadEnvironmentalSounds();
    LoadSurvivalFeedbackSounds();
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Prehistoric sound library initialized"));
}

void USurvivalAudioLibrary::PlayWeatherSound(EWeatherType WeatherType, float Intensity, const FVector& Location)
{
    EEnvironmentalAudioType AudioType = EEnvironmentalAudioType::Forest; // Default
    
    switch (WeatherType)
    {
        case EWeatherType::Rain:
            AudioType = EEnvironmentalAudioType::Rain;
            break;
        case EWeatherType::Storm:
            AudioType = EEnvironmentalAudioType::Storm;
            break;
        case EWeatherType::Wind:
            AudioType = EEnvironmentalAudioType::Wind;
            break;
        default:
            return; // No sound for clear weather
    }
    
    PlayEnvironmentalSound(Location, AudioType, Intensity);
}

void USurvivalAudioLibrary::LoadEnvironmentalSounds()
{
    // Forest sounds
    FAudio_EnvironmentalSound ForestSound;
    ForestSound.BaseVolume = 0.6f;
    ForestSound.MinDistance = 500.0f;
    ForestSound.MaxDistance = 2000.0f;
    ForestSound.bIsLooping = true;
    EnvironmentalSoundMap.Add(EEnvironmentalAudioType::Forest, ForestSound);
    
    // River sounds
    FAudio_EnvironmentalSound RiverSound;
    RiverSound.BaseVolume = 0.8f;
    RiverSound.MinDistance = 300.0f;
    RiverSound.MaxDistance = 1500.0f;
    RiverSound.bIsLooping = true;
    EnvironmentalSoundMap.Add(EEnvironmentalAudioType::River, RiverSound);
    
    // Cave sounds
    FAudio_EnvironmentalSound CaveSound;
    CaveSound.BaseVolume = 0.4f;
    CaveSound.MinDistance = 200.0f;
    CaveSound.MaxDistance = 800.0f;
    CaveSound.bIsLooping = true;
    EnvironmentalSoundMap.Add(EEnvironmentalAudioType::Cave, CaveSound);
    
    // Wind sounds
    FAudio_EnvironmentalSound WindSound;
    WindSound.BaseVolume = 0.5f;
    WindSound.MinDistance = 1000.0f;
    WindSound.MaxDistance = 5000.0f;
    WindSound.bIsLooping = true;
    EnvironmentalSoundMap.Add(EEnvironmentalAudioType::Wind, WindSound);
}

void USurvivalAudioLibrary::LoadSurvivalFeedbackSounds()
{
    // Initialize survival feedback sound references
    // In a full implementation, these would load actual sound assets
    SurvivalFeedbackSounds.IntensityThreshold = 0.3f;
}

USoundCue* USurvivalAudioLibrary::GetSoundForTerrainType(ETerrainType TerrainType, bool bIsRunning)
{
    // This would return different footstep sounds based on terrain
    // For now, return nullptr - sounds would be loaded from content
    return nullptr;
}

USoundCue* USurvivalAudioLibrary::GetCraftingSoundForAction(ECraftingAction ActionType)
{
    // This would return different crafting sounds based on action
    // For now, return nullptr - sounds would be loaded from content
    return nullptr;
}