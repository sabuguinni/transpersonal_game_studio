#include "Audio_AudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UAudio_AudioManager::UAudio_AudioManager()
{
    // Initialize default values
    CurrentBiome = EAudio_BiomeType::Savanna;
    CurrentTemperature = 25.0f;
    CurrentHumidity = 60.0f;
    
    bProximityAlertActive = false;
    
    MasterVolume = 1.0f;
    AmbientVolume = 0.7f;
    MusicVolume = 0.5f;
    AlertVolume = 0.9f;
    
    WindStrength = 0.0f;
    RainIntensity = 0.0f;
    TimeOfDay = 12.0f; // Noon
    
    bTransitioningBiome = false;
    BiomeTransitionTime = 0.0f;
    TargetBiome = EAudio_BiomeType::Savanna;
}

void UAudio_AudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Initializing adaptive audio system"));
    
    InitializeBiomeProfiles();
    InitializeAudioComponents();
    InitializeAudioSystem();
}

void UAudio_AudioManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Deinitializing audio system"));
    
    // Clean up audio components
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->Stop();
        AmbientAudioComponent = nullptr;
    }
    
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->Stop();
        MusicAudioComponent = nullptr;
    }
    
    if (AlertAudioComponent && IsValid(AlertAudioComponent))
    {
        AlertAudioComponent->Stop();
        AlertAudioComponent = nullptr;
    }
    
    if (TTSAudioComponent && IsValid(TTSAudioComponent))
    {
        TTSAudioComponent->Stop();
        TTSAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudio_AudioManager::InitializeAudioSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Audio system initialized for Cretaceous survival"));
    
    // Set initial biome to Savanna (center of map)
    SetCurrentBiome(EAudio_BiomeType::Savanna);
    
    // Initialize environmental conditions
    UpdateBiomeAudio(25.0f, 60.0f); // Typical savanna conditions
}

void UAudio_AudioManager::UpdateAudioSystem(float DeltaTime)
{
    if (bTransitioningBiome)
    {
        UpdateBiomeTransition(DeltaTime);
    }
    
    if (bProximityAlertActive)
    {
        UpdateProximityAlertAudio(DeltaTime);
    }
    
    CalculateVolumeModifiers();
}

void UAudio_AudioManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Transitioning from %d to %d"), 
               (int32)CurrentBiome, (int32)NewBiome);
        
        TargetBiome = NewBiome;
        bTransitioningBiome = true;
        BiomeTransitionTime = 0.0f;
    }
}

void UAudio_AudioManager::UpdateBiomeAudio(float Temperature, float Humidity)
{
    CurrentTemperature = Temperature;
    CurrentHumidity = Humidity;
    
    // Find current biome profile
    if (BiomeProfiles.Contains(CurrentBiome))
    {
        FAudio_BiomeProfile& Profile = BiomeProfiles[CurrentBiome];
        
        // Apply temperature and humidity modifiers
        float TempModifier = FMath::Clamp(Temperature / 30.0f, 0.5f, 1.5f);
        float HumidityModifier = FMath::Clamp(Humidity / 100.0f, 0.3f, 1.2f);
        
        Profile.TemperatureModifier = TempModifier;
        Profile.HumidityModifier = HumidityModifier;
        
        UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Updated biome audio - Temp: %.1f, Humidity: %.1f"), 
               Temperature, Humidity);
    }
}

void UAudio_AudioManager::TriggerProximityAlert(const FString& CreatureName, float Distance, EAudio_AlertLevel AlertLevel)
{
    CurrentProximityAlert.CreatureName = CreatureName;
    CurrentProximityAlert.Distance = Distance;
    CurrentProximityAlert.AlertLevel = AlertLevel;
    
    // Calculate volume based on distance and alert level
    float DistanceVolume = FMath::Clamp(1.0f - (Distance / 1000.0f), 0.1f, 1.0f);
    float AlertMultiplier = 1.0f;
    
    switch (AlertLevel)
    {
        case EAudio_AlertLevel::Caution:
            AlertMultiplier = 0.7f;
            break;
        case EAudio_AlertLevel::Danger:
            AlertMultiplier = 0.9f;
            break;
        case EAudio_AlertLevel::Critical:
            AlertMultiplier = 1.2f;
            break;
        default:
            AlertMultiplier = 0.5f;
            break;
    }
    
    CurrentProximityAlert.Volume = DistanceVolume * AlertMultiplier * AlertVolume;
    bProximityAlertActive = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Proximity alert triggered - %s at %.1fm, Level: %d"), 
           *CreatureName, Distance, (int32)AlertLevel);
    
    // Play alert sound if available
    if (AlertAudioComponent && IsValid(AlertAudioComponent))
    {
        AlertAudioComponent->SetVolumeMultiplier(CurrentProximityAlert.Volume);
        // AlertAudioComponent->Play(); // Would play if we had the sound asset
    }
}

void UAudio_AudioManager::ClearProximityAlert()
{
    bProximityAlertActive = false;
    CurrentProximityAlert = FAudio_ProximityAlert();
    
    if (AlertAudioComponent && IsValid(AlertAudioComponent))
    {
        AlertAudioComponent->Stop();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Proximity alert cleared"));
}

void UAudio_AudioManager::PlayTTSNarration(const FString& NarrationText, float Volume)
{
    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Playing TTS narration - %s"), *NarrationText);
    
    if (TTSAudioComponent && IsValid(TTSAudioComponent))
    {
        TTSAudioComponent->SetVolumeMultiplier(Volume * MasterVolume);
        // TTSAudioComponent->Play(); // Would play TTS audio if loaded
    }
}

void UAudio_AudioManager::PlaySystemAlert(const FString& AlertText, EAudio_AlertLevel AlertLevel)
{
    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: System alert - %s (Level: %d)"), 
           *AlertText, (int32)AlertLevel);
    
    float AlertVolumeMultiplier = 1.0f;
    switch (AlertLevel)
    {
        case EAudio_AlertLevel::Critical:
            AlertVolumeMultiplier = 1.5f;
            break;
        case EAudio_AlertLevel::Danger:
            AlertVolumeMultiplier = 1.2f;
            break;
        case EAudio_AlertLevel::Caution:
            AlertVolumeMultiplier = 0.8f;
            break;
        default:
            AlertVolumeMultiplier = 0.6f;
            break;
    }
    
    if (AlertAudioComponent && IsValid(AlertAudioComponent))
    {
        AlertAudioComponent->SetVolumeMultiplier(AlertVolumeMultiplier * AlertVolume * MasterVolume);
    }
}

void UAudio_AudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Master volume set to %.2f"), MasterVolume);
}

void UAudio_AudioManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
}

void UAudio_AudioManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
}

void UAudio_AudioManager::SetAlertVolume(float Volume)
{
    AlertVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
    if (AlertAudioComponent && IsValid(AlertAudioComponent))
    {
        AlertAudioComponent->SetVolumeMultiplier(AlertVolume * MasterVolume);
    }
}

void UAudio_AudioManager::UpdateWeatherAudio(float WindStrength, float RainIntensity)
{
    this->WindStrength = FMath::Clamp(WindStrength, 0.0f, 1.0f);
    this->RainIntensity = FMath::Clamp(RainIntensity, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Weather updated - Wind: %.2f, Rain: %.2f"), 
           WindStrength, RainIntensity);
    
    // Adjust ambient volume based on weather
    float WeatherModifier = 1.0f + (WindStrength * 0.3f) + (RainIntensity * 0.5f);
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * WeatherModifier * MasterVolume);
    }
}

void UAudio_AudioManager::UpdateTimeOfDayAudio(float TimeOfDay)
{
    this->TimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 24.0f);
    
    // Calculate day/night audio modifiers
    bool bIsNight = (TimeOfDay < 6.0f || TimeOfDay > 20.0f);
    float TimeModifier = bIsNight ? 0.7f : 1.0f; // Quieter at night
    
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * TimeModifier * MasterVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Time of day updated - %.1f (Night: %s)"), 
           TimeOfDay, bIsNight ? TEXT("Yes") : TEXT("No"));
}

void UAudio_AudioManager::InitializeBiomeProfiles()
{
    // Initialize biome profiles with default settings
    FAudio_BiomeProfile SwampProfile;
    SwampProfile.BiomeType = EAudio_BiomeType::Swamp;
    SwampProfile.BaseVolume = 0.8f;
    BiomeProfiles.Add(EAudio_BiomeType::Swamp, SwampProfile);
    
    FAudio_BiomeProfile ForestProfile;
    ForestProfile.BiomeType = EAudio_BiomeType::Forest;
    ForestProfile.BaseVolume = 0.9f;
    BiomeProfiles.Add(EAudio_BiomeType::Forest, ForestProfile);
    
    FAudio_BiomeProfile SavannaProfile;
    SavannaProfile.BiomeType = EAudio_BiomeType::Savanna;
    SavannaProfile.BaseVolume = 0.7f;
    BiomeProfiles.Add(EAudio_BiomeType::Savanna, SavannaProfile);
    
    FAudio_BiomeProfile DesertProfile;
    DesertProfile.BiomeType = EAudio_BiomeType::Desert;
    DesertProfile.BaseVolume = 0.5f;
    BiomeProfiles.Add(EAudio_BiomeType::Desert, DesertProfile);
    
    FAudio_BiomeProfile MountainProfile;
    MountainProfile.BiomeType = EAudio_BiomeType::Mountain;
    MountainProfile.BaseVolume = 0.6f;
    BiomeProfiles.Add(EAudio_BiomeType::Mountain, MountainProfile);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Initialized %d biome profiles"), BiomeProfiles.Num());
}

void UAudio_AudioManager::InitializeAudioComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_AudioManager: Cannot initialize audio components - no world"));
        return;
    }
    
    // Create audio components
    AmbientAudioComponent = NewObject<UAudioComponent>(this);
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
        AmbientAudioComponent->bAutoActivate = false;
    }
    
    MusicAudioComponent = NewObject<UAudioComponent>(this);
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
        MusicAudioComponent->bAutoActivate = false;
    }
    
    AlertAudioComponent = NewObject<UAudioComponent>(this);
    if (AlertAudioComponent)
    {
        AlertAudioComponent->SetVolumeMultiplier(AlertVolume * MasterVolume);
        AlertAudioComponent->bAutoActivate = false;
    }
    
    TTSAudioComponent = NewObject<UAudioComponent>(this);
    if (TTSAudioComponent)
    {
        TTSAudioComponent->SetVolumeMultiplier(MasterVolume);
        TTSAudioComponent->bAutoActivate = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Audio components initialized"));
}

void UAudio_AudioManager::UpdateBiomeTransition(float DeltaTime)
{
    BiomeTransitionTime += DeltaTime;
    
    // Transition duration: 3 seconds
    const float TransitionDuration = 3.0f;
    
    if (BiomeTransitionTime >= TransitionDuration)
    {
        CurrentBiome = TargetBiome;
        bTransitioningBiome = false;
        BiomeTransitionTime = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Biome transition completed to %d"), (int32)CurrentBiome);
    }
    else
    {
        // Calculate transition progress
        float TransitionProgress = BiomeTransitionTime / TransitionDuration;
        
        // Apply crossfade between biome audio profiles
        if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
        {
            float CrossfadeVolume = FMath::Lerp(1.0f, 0.5f, TransitionProgress);
            AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * CrossfadeVolume * MasterVolume);
        }
    }
}

void UAudio_AudioManager::UpdateProximityAlertAudio(float DeltaTime)
{
    // Update proximity alert audio based on current state
    if (AlertAudioComponent && IsValid(AlertAudioComponent))
    {
        // Pulse effect for critical alerts
        if (CurrentProximityAlert.AlertLevel == EAudio_AlertLevel::Critical)
        {
            float PulseValue = FMath::Sin(GetWorld()->GetTimeSeconds() * 4.0f) * 0.2f + 0.8f;
            AlertAudioComponent->SetVolumeMultiplier(CurrentProximityAlert.Volume * PulseValue);
        }
    }
}

void UAudio_AudioManager::CalculateVolumeModifiers()
{
    // Apply environmental modifiers to all audio components
    float EnvironmentalModifier = 1.0f;
    
    // Weather effects
    EnvironmentalModifier += (WindStrength * 0.2f);
    EnvironmentalModifier += (RainIntensity * 0.3f);
    
    // Time of day effects
    bool bIsNight = (TimeOfDay < 6.0f || TimeOfDay > 20.0f);
    if (bIsNight)
    {
        EnvironmentalModifier *= 0.8f; // Quieter at night
    }
    
    // Temperature effects
    if (CurrentTemperature > 35.0f) // Very hot
    {
        EnvironmentalModifier *= 0.9f; // Slightly quieter in extreme heat
    }
    else if (CurrentTemperature < 5.0f) // Very cold
    {
        EnvironmentalModifier *= 1.1f; // Sound carries better in cold
    }
    
    // Apply modifiers to ambient audio
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        float FinalVolume = AmbientVolume * EnvironmentalModifier * MasterVolume;
        AmbientAudioComponent->SetVolumeMultiplier(FMath::Clamp(FinalVolume, 0.0f, 2.0f));
    }
}