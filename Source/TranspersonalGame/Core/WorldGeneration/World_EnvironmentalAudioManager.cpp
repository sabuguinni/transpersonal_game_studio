#include "World_EnvironmentalAudioManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogEnvironmentalAudio);

UWorld_EnvironmentalAudioManager::UWorld_EnvironmentalAudioManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update twice per second for performance
    
    // Initialize default values
    CurrentBiome = EWorld_BiomeType::Forest;
    CurrentWeather = EWorld_WeatherType::Clear;
    CurrentTimeOfDay = EWorld_TimeOfDay::Day;
    
    AudioFadeSpeed = 2.0f;
    MaxAudioDistance = 5000.0f;
    BiomeTransitionRadius = 1000.0f;
    
    bIsInitialized = false;
    bAudioSystemActive = true;
}

void UWorld_EnvironmentalAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioSystem();
    
    // Start audio update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            AudioUpdateTimer,
            this,
            &UWorld_EnvironmentalAudioManager::UpdateEnvironmentalAudio,
            1.0f,
            true
        );
    }
    
    UE_LOG(LogEnvironmentalAudio, Log, TEXT("Environmental Audio Manager initialized"));
}

void UWorld_EnvironmentalAudioManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up audio components
    for (auto& AudioPair : ActiveAudioComponents)
    {
        if (AudioPair.Value && IsValid(AudioPair.Value))
        {
            AudioPair.Value->Stop();
            AudioPair.Value->DestroyComponent();
        }
    }
    ActiveAudioComponents.Empty();
    
    // Clear timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AudioUpdateTimer);
    }
    
    Super::EndPlay(EndPlayReason);
}

void UWorld_EnvironmentalAudioManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bAudioSystemActive || !bIsInitialized)
    {
        return;
    }
    
    // Update audio component volumes based on distance and biome
    UpdateAudioVolumes(DeltaTime);
}

void UWorld_EnvironmentalAudioManager::InitializeAudioSystem()
{
    if (bIsInitialized)
    {
        return;
    }
    
    // Initialize biome audio configurations
    InitializeBiomeAudioConfigs();
    
    // Initialize weather audio configurations
    InitializeWeatherAudioConfigs();
    
    // Initialize time-of-day audio configurations
    InitializeTimeOfDayAudioConfigs();
    
    bIsInitialized = true;
    
    UE_LOG(LogEnvironmentalAudio, Log, TEXT("Audio system initialization complete"));
}

void UWorld_EnvironmentalAudioManager::InitializeBiomeAudioConfigs()
{
    // Forest biome configuration
    FWorld_BiomeAudioConfig ForestConfig;
    ForestConfig.BiomeType = EWorld_BiomeType::Forest;
    ForestConfig.AmbientVolume = 0.7f;
    ForestConfig.WildlifeVolume = 0.8f;
    ForestConfig.WeatherVolume = 0.6f;
    ForestConfig.bEnableReverb = true;
    ForestConfig.ReverbSettings.Density = 0.8f;
    ForestConfig.ReverbSettings.Diffusion = 0.9f;
    ForestConfig.ReverbSettings.Gain = 0.7f;
    BiomeAudioConfigs.Add(EWorld_BiomeType::Forest, ForestConfig);
    
    // Swamp biome configuration
    FWorld_BiomeAudioConfig SwampConfig;
    SwampConfig.BiomeType = EWorld_BiomeType::Swamp;
    SwampConfig.AmbientVolume = 0.8f;
    SwampConfig.WildlifeVolume = 0.9f;
    SwampConfig.WeatherVolume = 0.7f;
    SwampConfig.bEnableReverb = true;
    SwampConfig.ReverbSettings.Density = 0.9f;
    SwampConfig.ReverbSettings.Diffusion = 0.8f;
    SwampConfig.ReverbSettings.Gain = 0.8f;
    BiomeAudioConfigs.Add(EWorld_BiomeType::Swamp, SwampConfig);
    
    // Savanna biome configuration
    FWorld_BiomeAudioConfig SavannaConfig;
    SavannaConfig.BiomeType = EWorld_BiomeType::Savanna;
    SavannaConfig.AmbientVolume = 0.6f;
    SavannaConfig.WildlifeVolume = 0.7f;
    SavannaConfig.WeatherVolume = 0.8f;
    SavannaConfig.bEnableReverb = false;
    SavannaConfig.ReverbSettings.Density = 0.3f;
    SavannaConfig.ReverbSettings.Diffusion = 0.4f;
    SavannaConfig.ReverbSettings.Gain = 0.3f;
    BiomeAudioConfigs.Add(EWorld_BiomeType::Savanna, SavannaConfig);
    
    // Desert biome configuration
    FWorld_BiomeAudioConfig DesertConfig;
    DesertConfig.BiomeType = EWorld_BiomeType::Desert;
    DesertConfig.AmbientVolume = 0.4f;
    DesertConfig.WildlifeVolume = 0.3f;
    DesertConfig.WeatherVolume = 0.9f;
    DesertConfig.bEnableReverb = false;
    DesertConfig.ReverbSettings.Density = 0.1f;
    DesertConfig.ReverbSettings.Diffusion = 0.2f;
    DesertConfig.ReverbSettings.Gain = 0.1f;
    BiomeAudioConfigs.Add(EWorld_BiomeType::Desert, DesertConfig);
    
    // Mountain biome configuration
    FWorld_BiomeAudioConfig MountainConfig;
    MountainConfig.BiomeType = EWorld_BiomeType::Mountain;
    MountainConfig.AmbientVolume = 0.5f;
    MountainConfig.WildlifeVolume = 0.4f;
    MountainConfig.WeatherVolume = 1.0f;
    MountainConfig.bEnableReverb = true;
    MountainConfig.ReverbSettings.Density = 0.6f;
    MountainConfig.ReverbSettings.Diffusion = 0.7f;
    MountainConfig.ReverbSettings.Gain = 0.9f;
    BiomeAudioConfigs.Add(EWorld_BiomeType::Mountain, MountainConfig);
}

void UWorld_EnvironmentalAudioManager::InitializeWeatherAudioConfigs()
{
    // Clear weather configuration
    FWorld_WeatherAudioConfig ClearConfig;
    ClearConfig.WeatherType = EWorld_WeatherType::Clear;
    ClearConfig.WindVolume = 0.3f;
    ClearConfig.RainVolume = 0.0f;
    ClearConfig.ThunderVolume = 0.0f;
    ClearConfig.AtmosphereVolume = 0.5f;
    WeatherAudioConfigs.Add(EWorld_WeatherType::Clear, ClearConfig);
    
    // Rain weather configuration
    FWorld_WeatherAudioConfig RainConfig;
    RainConfig.WeatherType = EWorld_WeatherType::Rain;
    RainConfig.WindVolume = 0.6f;
    RainConfig.RainVolume = 0.8f;
    RainConfig.ThunderVolume = 0.4f;
    RainConfig.AtmosphereVolume = 0.3f;
    WeatherAudioConfigs.Add(EWorld_WeatherType::Rain, RainConfig);
    
    // Storm weather configuration
    FWorld_WeatherAudioConfig StormConfig;
    StormConfig.WeatherType = EWorld_WeatherType::Storm;
    StormConfig.WindVolume = 0.9f;
    StormConfig.RainVolume = 1.0f;
    StormConfig.ThunderVolume = 0.8f;
    StormConfig.AtmosphereVolume = 0.2f;
    WeatherAudioConfigs.Add(EWorld_WeatherType::Storm, StormConfig);
    
    // Fog weather configuration
    FWorld_WeatherAudioConfig FogConfig;
    FogConfig.WeatherType = EWorld_WeatherType::Fog;
    FogConfig.WindVolume = 0.2f;
    FogConfig.RainVolume = 0.1f;
    FogConfig.ThunderVolume = 0.0f;
    FogConfig.AtmosphereVolume = 0.7f;
    WeatherAudioConfigs.Add(EWorld_WeatherType::Fog, FogConfig);
}

void UWorld_EnvironmentalAudioManager::InitializeTimeOfDayAudioConfigs()
{
    // Day time configuration
    FWorld_TimeAudioConfig DayConfig;
    DayConfig.TimeOfDay = EWorld_TimeOfDay::Day;
    DayConfig.BirdVolume = 0.8f;
    DayConfig.InsectVolume = 0.6f;
    DayConfig.NightCreatureVolume = 0.0f;
    DayConfig.AmbientVolume = 0.7f;
    TimeOfDayAudioConfigs.Add(EWorld_TimeOfDay::Day, DayConfig);
    
    // Night time configuration
    FWorld_TimeAudioConfig NightConfig;
    NightConfig.TimeOfDay = EWorld_TimeOfDay::Night;
    NightConfig.BirdVolume = 0.1f;
    NightConfig.InsectVolume = 0.9f;
    NightConfig.NightCreatureVolume = 0.8f;
    NightConfig.AmbientVolume = 0.5f;
    TimeOfDayAudioConfigs.Add(EWorld_TimeOfDay::Night, NightConfig);
    
    // Dawn time configuration
    FWorld_TimeAudioConfig DawnConfig;
    DawnConfig.TimeOfDay = EWorld_TimeOfDay::Dawn;
    DawnConfig.BirdVolume = 1.0f;
    DawnConfig.InsectVolume = 0.4f;
    DawnConfig.NightCreatureVolume = 0.2f;
    DawnConfig.AmbientVolume = 0.6f;
    TimeOfDayAudioConfigs.Add(EWorld_TimeOfDay::Dawn, DawnConfig);
    
    // Dusk time configuration
    FWorld_TimeAudioConfig DuskConfig;
    DuskConfig.TimeOfDay = EWorld_TimeOfDay::Dusk;
    DuskConfig.BirdVolume = 0.6f;
    DuskConfig.InsectVolume = 0.7f;
    DuskConfig.NightCreatureVolume = 0.5f;
    DuskConfig.AmbientVolume = 0.6f;
    TimeOfDayAudioConfigs.Add(EWorld_TimeOfDay::Dusk, DuskConfig);
}

void UWorld_EnvironmentalAudioManager::UpdateEnvironmentalAudio()
{
    if (!bAudioSystemActive || !bIsInitialized)
    {
        return;
    }
    
    // Update biome-based audio
    UpdateBiomeAudio();
    
    // Update weather-based audio
    UpdateWeatherAudio();
    
    // Update time-of-day audio
    UpdateTimeOfDayAudio();
    
    // Process audio transitions
    ProcessAudioTransitions();
}

void UWorld_EnvironmentalAudioManager::UpdateBiomeAudio()
{
    // Get current biome configuration
    if (FWorld_BiomeAudioConfig* Config = BiomeAudioConfigs.Find(CurrentBiome))
    {
        // Update biome-specific audio volumes
        SetAudioCategoryVolume(TEXT("BiomeAmbient"), Config->AmbientVolume);
        SetAudioCategoryVolume(TEXT("BiomeWildlife"), Config->WildlifeVolume);
        SetAudioCategoryVolume(TEXT("BiomeWeather"), Config->WeatherVolume);
        
        // Apply reverb settings if enabled
        if (Config->bEnableReverb)
        {
            ApplyReverbSettings(Config->ReverbSettings);
        }
        else
        {
            DisableReverb();
        }
    }
}

void UWorld_EnvironmentalAudioManager::UpdateWeatherAudio()
{
    // Get current weather configuration
    if (FWorld_WeatherAudioConfig* Config = WeatherAudioConfigs.Find(CurrentWeather))
    {
        SetAudioCategoryVolume(TEXT("WeatherWind"), Config->WindVolume);
        SetAudioCategoryVolume(TEXT("WeatherRain"), Config->RainVolume);
        SetAudioCategoryVolume(TEXT("WeatherThunder"), Config->ThunderVolume);
        SetAudioCategoryVolume(TEXT("WeatherAtmosphere"), Config->AtmosphereVolume);
    }
}

void UWorld_EnvironmentalAudioManager::UpdateTimeOfDayAudio()
{
    // Get current time-of-day configuration
    if (FWorld_TimeAudioConfig* Config = TimeOfDayAudioConfigs.Find(CurrentTimeOfDay))
    {
        SetAudioCategoryVolume(TEXT("TimeOfDayBirds"), Config->BirdVolume);
        SetAudioCategoryVolume(TEXT("TimeOfDayInsects"), Config->InsectVolume);
        SetAudioCategoryVolume(TEXT("TimeOfDayNightCreatures"), Config->NightCreatureVolume);
        SetAudioCategoryVolume(TEXT("TimeOfDayAmbient"), Config->AmbientVolume);
    }
}

void UWorld_EnvironmentalAudioManager::UpdateAudioVolumes(float DeltaTime)
{
    // Update volumes for all active audio components based on distance and settings
    for (auto& AudioPair : ActiveAudioComponents)
    {
        if (UAudioComponent* AudioComp = AudioPair.Value)
        {
            if (IsValid(AudioComp))
            {
                // Calculate distance-based volume
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), AudioComp->GetComponentLocation());
                float VolumeMultiplier = FMath::Clamp(1.0f - (Distance / MaxAudioDistance), 0.0f, 1.0f);
                
                // Apply smooth volume transitions
                float CurrentVolume = AudioComp->GetVolumeMultiplier();
                float TargetVolume = VolumeMultiplier;
                float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, AudioFadeSpeed);
                
                AudioComp->SetVolumeMultiplier(NewVolume);
            }
        }
    }
}

void UWorld_EnvironmentalAudioManager::ProcessAudioTransitions()
{
    // Handle smooth transitions between different audio states
    // This would include biome transitions, weather changes, and time-of-day shifts
    
    // For now, implement basic transition logic
    // In a full implementation, this would use more sophisticated blending
}

void UWorld_EnvironmentalAudioManager::SetAudioCategoryVolume(const FString& Category, float Volume)
{
    // Set volume for a specific audio category
    // This would interface with UE5's audio system to control category volumes
    
    UE_LOG(LogEnvironmentalAudio, VeryVerbose, TEXT("Setting %s volume to %f"), *Category, Volume);
}

void UWorld_EnvironmentalAudioManager::ApplyReverbSettings(const FWorld_ReverbSettings& Settings)
{
    // Apply reverb settings to the current audio environment
    // This would interface with UE5's reverb system
    
    UE_LOG(LogEnvironmentalAudio, VeryVerbose, TEXT("Applying reverb: Density=%f, Diffusion=%f, Gain=%f"), 
           Settings.Density, Settings.Diffusion, Settings.Gain);
}

void UWorld_EnvironmentalAudioManager::DisableReverb()
{
    // Disable reverb effects
    UE_LOG(LogEnvironmentalAudio, VeryVerbose, TEXT("Disabling reverb"));
}

void UWorld_EnvironmentalAudioManager::SetCurrentBiome(EWorld_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        EWorld_BiomeType PreviousBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        
        UE_LOG(LogEnvironmentalAudio, Log, TEXT("Biome changed from %d to %d"), 
               (int32)PreviousBiome, (int32)NewBiome);
        
        // Trigger biome transition
        OnBiomeChanged(PreviousBiome, NewBiome);
    }
}

void UWorld_EnvironmentalAudioManager::SetCurrentWeather(EWorld_WeatherType NewWeather)
{
    if (CurrentWeather != NewWeather)
    {
        EWorld_WeatherType PreviousWeather = CurrentWeather;
        CurrentWeather = NewWeather;
        
        UE_LOG(LogEnvironmentalAudio, Log, TEXT("Weather changed from %d to %d"), 
               (int32)PreviousWeather, (int32)NewWeather);
        
        // Trigger weather transition
        OnWeatherChanged(PreviousWeather, NewWeather);
    }
}

void UWorld_EnvironmentalAudioManager::SetCurrentTimeOfDay(EWorld_TimeOfDay NewTimeOfDay)
{
    if (CurrentTimeOfDay != NewTimeOfDay)
    {
        EWorld_TimeOfDay PreviousTimeOfDay = CurrentTimeOfDay;
        CurrentTimeOfDay = NewTimeOfDay;
        
        UE_LOG(LogEnvironmentalAudio, Log, TEXT("Time of day changed from %d to %d"), 
               (int32)PreviousTimeOfDay, (int32)NewTimeOfDay);
        
        // Trigger time-of-day transition
        OnTimeOfDayChanged(PreviousTimeOfDay, NewTimeOfDay);
    }
}

void UWorld_EnvironmentalAudioManager::OnBiomeChanged(EWorld_BiomeType PreviousBiome, EWorld_BiomeType NewBiome)
{
    // Handle biome transition logic
    // This could include cross-fading between biome audio sets
}

void UWorld_EnvironmentalAudioManager::OnWeatherChanged(EWorld_WeatherType PreviousWeather, EWorld_WeatherType NewWeather)
{
    // Handle weather transition logic
    // This could include gradual weather audio changes
}

void UWorld_EnvironmentalAudioManager::OnTimeOfDayChanged(EWorld_TimeOfDay PreviousTimeOfDay, EWorld_TimeOfDay NewTimeOfDay)
{
    // Handle time-of-day transition logic
    // This could include dawn/dusk audio transitions
}

void UWorld_EnvironmentalAudioManager::SetAudioSystemActive(bool bActive)
{
    bAudioSystemActive = bActive;
    
    if (!bActive)
    {
        // Stop all audio when system is deactivated
        for (auto& AudioPair : ActiveAudioComponents)
        {
            if (AudioPair.Value && IsValid(AudioPair.Value))
            {
                AudioPair.Value->Stop();
            }
        }
    }
    
    UE_LOG(LogEnvironmentalAudio, Log, TEXT("Audio system %s"), bActive ? TEXT("activated") : TEXT("deactivated"));
}

float UWorld_EnvironmentalAudioManager::GetCurrentBiomeVolume() const
{
    if (const FWorld_BiomeAudioConfig* Config = BiomeAudioConfigs.Find(CurrentBiome))
    {
        return Config->AmbientVolume;
    }
    return 0.5f; // Default volume
}

float UWorld_EnvironmentalAudioManager::GetCurrentWeatherVolume() const
{
    if (const FWorld_WeatherAudioConfig* Config = WeatherAudioConfigs.Find(CurrentWeather))
    {
        return Config->AtmosphereVolume;
    }
    return 0.5f; // Default volume
}