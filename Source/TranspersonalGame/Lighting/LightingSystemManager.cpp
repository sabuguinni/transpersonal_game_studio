#include "LightingSystemManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ALightingSystemManager::ALightingSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create Sun Light (Primary Directional Light)
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);
    SunLight->SetCastShadows(true);
    SunLight->SetCastVolumetricShadow(true);
    SunLight->SetCastCloudShadows(true);
    
    // Create Moon Light (Secondary Directional Light)
    MoonLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLight"));
    MoonLight->SetupAttachment(RootComponent);
    MoonLight->SetMobility(EComponentMobility::Movable);
    MoonLight->SetAtmosphereSunLight(true);
    MoonLight->SetAtmosphereSunLightIndex(1);
    MoonLight->SetCastShadows(true);
    MoonLight->SetIntensity(0.1f); // Much dimmer than sun
    
    // Create Sky Light
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetSourceType(ESkyLightSourceType::SLS_CapturedScene);
    SkyLight->SetRealTimeCapture(true);
    
    // Create Sky Atmosphere
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(RootComponent);
    
    // Create Volumetric Clouds
    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    VolumetricClouds->SetupAttachment(RootComponent);
    
    // Create Height Fog
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);
    HeightFog->SetVolumetricFog(true);
    
    // Initialize default configurations
    InitializeDefaultConfigurations();
}

void ALightingSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial lighting state
    ForceUpdateLighting();
}

void ALightingSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Performance optimization - don't update every frame
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime < UpdateFrequency)
    {
        return;
    }
    
    float ActualDeltaTime = LastUpdateTime;
    LastUpdateTime = 0.0f;
    
    // Update time progression
    if (bAutoAdvanceTime)
    {
        UpdateTimeOfDay(ActualDeltaTime);
    }
    
    // Update weather transitions
    if (bTransitioningWeather)
    {
        UpdateWeatherTransition(ActualDeltaTime);
    }
    
    // Update dynamic weather events
    for (int32 i = ActiveWeatherEvents.Num() - 1; i >= 0; i--)
    {
        ActiveWeatherEvents[i].RemainingTime -= ActualDeltaTime;
        if (ActiveWeatherEvents[i].RemainingTime <= 0.0f)
        {
            ActiveWeatherEvents.RemoveAt(i);
        }
    }
    
    // Update lighting configuration
    UpdateLightingConfiguration();
}

void ALightingSystemManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Fmod(NewTime, 24.0f);
    if (CurrentTimeOfDay < 0.0f)
    {
        CurrentTimeOfDay += 24.0f;
    }
    ForceUpdateLighting();
}

void ALightingSystemManager::SetWeather(EWeatherState NewWeather, float TransitionTime)
{
    if (NewWeather != CurrentWeather)
    {
        TargetWeather = NewWeather;
        WeatherTransitionSpeed = TransitionTime > 0.0f ? 1.0f / TransitionTime : 10.0f;
        bTransitioningWeather = true;
        WeatherTransitionProgress = 0.0f;
    }
}

void ALightingSystemManager::SetEmotionalTone(EEmotionalTone NewTone, float Intensity)
{
    CurrentTone = NewTone;
    EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    ForceUpdateLighting();
}

ETimeOfDay ALightingSystemManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ETimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ETimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
        return ETimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
        return ETimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
        return ETimeOfDay::Dusk;
    else if (CurrentTimeOfDay >= 20.0f && CurrentTimeOfDay < 23.0f)
        return ETimeOfDay::Night;
    else
        return ETimeOfDay::DeepNight;
}

void ALightingSystemManager::ForceUpdateLighting()
{
    UpdateLightingConfiguration();
}

void ALightingSystemManager::CreateDynamicWeatherEvent(EWeatherState Weather, float Duration)
{
    FWeatherEvent NewEvent;
    NewEvent.Weather = Weather;
    NewEvent.Duration = Duration;
    NewEvent.RemainingTime = Duration;
    ActiveWeatherEvents.Add(NewEvent);
}

void ALightingSystemManager::UpdateTimeOfDay(float DeltaTime)
{
    float TimeIncrement = (DeltaTime * TimeSpeed) / 3600.0f; // Convert seconds to hours
    CurrentTimeOfDay += TimeIncrement;
    
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }
}

void ALightingSystemManager::UpdateWeatherTransition(float DeltaTime)
{
    WeatherTransitionProgress += WeatherTransitionSpeed * DeltaTime;
    
    if (WeatherTransitionProgress >= 1.0f)
    {
        CurrentWeather = TargetWeather;
        bTransitioningWeather = false;
        WeatherTransitionProgress = 0.0f;
    }
}

void ALightingSystemManager::UpdateLightingConfiguration()
{
    FLightingConfiguration CurrentConfig = GetCurrentLightingConfiguration();
    ApplyLightingConfiguration(CurrentConfig);
}

void ALightingSystemManager::ApplyLightingConfiguration(const FLightingConfiguration& Config)
{
    // Update Sun Light
    float SunAngle = CalculateSunAngle(CurrentTimeOfDay);
    FRotator SunRotation = FRotator(-SunAngle, 0.0f, 0.0f);
    SunLight->SetWorldRotation(SunRotation);
    SunLight->SetIntensity(Config.SunIntensity);
    SunLight->SetLightColor(Config.SunColor);
    
    // Update Moon Light (opposite to sun)
    FRotator MoonRotation = FRotator(-SunAngle + 180.0f, 0.0f, 0.0f);
    MoonLight->SetWorldRotation(MoonRotation);
    MoonLight->SetIntensity(Config.MoonIntensity);
    MoonLight->SetLightColor(Config.MoonColor);
    
    // Update Sky Light
    SkyLight->SetIntensity(Config.SkyLightIntensity);
    SkyLight->SetLightColor(Config.SkyTint);
    
    // Update Sky Atmosphere
    SkyAtmosphere->SetAtmosphereHeight(Config.AtmosphereHeight);
    SkyAtmosphere->SetRayleighScatteringScale(Config.RayleighScatteringScale);
    SkyAtmosphere->SetMieScatteringScale(Config.MieScatteringScale);
    
    // Update Volumetric Clouds
    // Note: These would typically be set via material parameters
    // For now, we'll store the values for use by the cloud material
    
    // Update Height Fog
    HeightFog->SetFogDensity(Config.FogDensity);
    HeightFog->SetFogHeightFalloff(Config.FogHeightFalloff);
    HeightFog->SetFogInscatteringColor(Config.FogInscatteringColor);
}

FLightingConfiguration ALightingSystemManager::GetCurrentLightingConfiguration() const
{
    // Get base configuration from time of day
    ETimeOfDay CurrentTimeEnum = GetCurrentTimeOfDayEnum();
    FLightingConfiguration BaseConfig;
    
    if (TimeBasedConfigs.Contains(CurrentTimeEnum))
    {
        BaseConfig = TimeBasedConfigs[CurrentTimeEnum];
    }
    
    // Blend with weather configuration
    FLightingConfiguration WeatherConfig;
    if (WeatherBasedConfigs.Contains(CurrentWeather))
    {
        WeatherConfig = WeatherBasedConfigs[CurrentWeather];
    }
    
    FLightingConfiguration BlendedConfig;
    BlendLightingConfigurations(BaseConfig, WeatherConfig, 0.3f, BlendedConfig);
    
    // Apply emotional tone modifications
    if (EmotionBasedConfigs.Contains(CurrentTone))
    {
        FLightingConfiguration EmotionalConfig = EmotionBasedConfigs[CurrentTone];
        FLightingConfiguration FinalConfig;
        BlendLightingConfigurations(BlendedConfig, EmotionalConfig, EmotionalIntensity, FinalConfig);
        return FinalConfig;
    }
    
    return BlendedConfig;
}

void ALightingSystemManager::BlendLightingConfigurations(const FLightingConfiguration& A, const FLightingConfiguration& B, float Alpha, FLightingConfiguration& Result)
{
    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
    
    // Blend all numeric values
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunAngle = FMath::Lerp(A.SunAngle, B.SunAngle, Alpha);
    Result.MoonIntensity = FMath::Lerp(A.MoonIntensity, B.MoonIntensity, Alpha);
    Result.MoonColor = FMath::Lerp(A.MoonColor, B.MoonColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.SkyTint = FMath::Lerp(A.SkyTint, B.SkyTint, Alpha);
    Result.AtmosphereHeight = FMath::Lerp(A.AtmosphereHeight, B.AtmosphereHeight, Alpha);
    Result.RayleighScatteringScale = FMath::Lerp(A.RayleighScatteringScale, B.RayleighScatteringScale, Alpha);
    Result.MieScatteringScale = FMath::Lerp(A.MieScatteringScale, B.MieScatteringScale, Alpha);
    Result.CloudCoverage = FMath::Lerp(A.CloudCoverage, B.CloudCoverage, Alpha);
    Result.CloudDensity = FMath::Lerp(A.CloudDensity, B.CloudDensity, Alpha);
    Result.CloudSpeed = FMath::Lerp(A.CloudSpeed, B.CloudSpeed, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.FogInscatteringColor = FMath::Lerp(A.FogInscatteringColor, B.FogInscatteringColor, Alpha);
    Result.ExposureBias = FMath::Lerp(A.ExposureBias, B.ExposureBias, Alpha);
    Result.Contrast = FMath::Lerp(A.Contrast, B.Contrast, Alpha);
    Result.Saturation = FMath::Lerp(A.Saturation, B.Saturation, Alpha);
    Result.ColorGrading = FMath::Lerp(A.ColorGrading, B.ColorGrading, Alpha);
}

float ALightingSystemManager::CalculateSunAngle(float TimeHours) const
{
    // Convert time to angle (0-360 degrees over 24 hours)
    // Noon (12:00) = 0 degrees (sun directly overhead)
    // Sunrise/Sunset = 90 degrees
    float NormalizedTime = (TimeHours - 12.0f) / 12.0f; // -1 to 1
    return NormalizedTime * 90.0f; // -90 to 90 degrees
}

FLinearColor ALightingSystemManager::CalculateSunColor(float SunAngle) const
{
    // Warmer colors at low angles (sunrise/sunset), cooler at high angles (noon)
    float AbsAngle = FMath::Abs(SunAngle);
    
    if (AbsAngle > 90.0f)
    {
        // Sun is below horizon - return very dim, cool color
        return FLinearColor(0.1f, 0.1f, 0.2f, 1.0f);
    }
    
    // Interpolate between warm (low angle) and cool (high angle)
    float WarmFactor = FMath::Clamp(AbsAngle / 90.0f, 0.0f, 1.0f);
    FLinearColor WarmColor(1.0f, 0.8f, 0.6f, 1.0f); // Warm sunset/sunrise
    FLinearColor CoolColor(1.0f, 1.0f, 0.95f, 1.0f); // Cool midday
    
    return FMath::Lerp(WarmColor, CoolColor, 1.0f - WarmFactor);
}

FLinearColor ALightingSystemManager::CalculateSkyColor(float SunAngle, EWeatherState Weather) const
{
    // Base sky color calculation similar to sun color
    FLinearColor BaseColor = CalculateSunColor(SunAngle);
    
    // Modify based on weather
    switch (Weather)
    {
        case EWeatherState::Clear:
            return BaseColor;
        case EWeatherState::PartlyCloudy:
            return FMath::Lerp(BaseColor, FLinearColor(0.8f, 0.8f, 0.9f, 1.0f), 0.2f);
        case EWeatherState::Overcast:
            return FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
        case EWeatherState::LightRain:
            return FLinearColor(0.5f, 0.5f, 0.6f, 1.0f);
        case EWeatherState::HeavyRain:
        case EWeatherState::Storm:
            return FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
        case EWeatherState::Fog:
            return FLinearColor(0.7f, 0.7f, 0.8f, 1.0f);
        default:
            return BaseColor;
    }
}

void ALightingSystemManager::InitializeDefaultConfigurations()
{
    // Initialize time-based configurations
    FLightingConfiguration DawnConfig;
    DawnConfig.SunIntensity = 2.0f;
    DawnConfig.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnConfig.SkyLightIntensity = 0.3f;
    DawnConfig.FogDensity = 0.05f;
    TimeBasedConfigs.Add(ETimeOfDay::Dawn, DawnConfig);
    
    FLightingConfiguration MiddayConfig;
    MiddayConfig.SunIntensity = 10.0f;
    MiddayConfig.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    MiddayConfig.SkyLightIntensity = 1.0f;
    MiddayConfig.FogDensity = 0.01f;
    TimeBasedConfigs.Add(ETimeOfDay::Midday, MiddayConfig);
    
    FLightingConfiguration NightConfig;
    NightConfig.SunIntensity = 0.0f;
    NightConfig.MoonIntensity = 0.5f;
    NightConfig.SkyLightIntensity = 0.1f;
    NightConfig.FogDensity = 0.03f;
    TimeBasedConfigs.Add(ETimeOfDay::Night, NightConfig);
    
    // Initialize weather-based configurations
    FLightingConfiguration StormConfig;
    StormConfig.SunIntensity = 0.3f;
    StormConfig.SkyLightIntensity = 0.2f;
    StormConfig.FogDensity = 0.08f;
    StormConfig.CloudCoverage = 0.9f;
    WeatherBasedConfigs.Add(EWeatherState::Storm, StormConfig);
    
    // Initialize emotion-based configurations
    FLightingConfiguration DangerousConfig;
    DangerousConfig.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f); // Warmer, more threatening
    DangerousConfig.Contrast = 1.2f;
    DangerousConfig.Saturation = 0.8f;
    EmotionBasedConfigs.Add(EEmotionalTone::Dangerous, DangerousConfig);
    
    FLightingConfiguration PeacefulConfig;
    PeacefulConfig.SunColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // Pure, clean light
    PeacefulConfig.Contrast = 0.9f;
    PeacefulConfig.Saturation = 1.1f;
    EmotionBasedConfigs.Add(EEmotionalTone::Peaceful, PeacefulConfig);
}