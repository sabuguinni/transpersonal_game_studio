#include "EnvArt_WeatherSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

AEnvArt_WeatherSystem::AEnvArt_WeatherSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create directional light for sun
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetIntensity(8.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.8f, 1.0f));
    SunLight->SetCastShadows(true);
    SunLight->SetCastStaticShadows(true);
    SunLight->SetCastDynamicShadows(true);

    // Create sky light for ambient lighting
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetIntensity(1.5f);
    SkyLight->SetSourceType(ESkyLightSourceType::SLS_CapturedScene);
    SkyLight->SetCastShadows(false);

    // Create height fog
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);
    HeightFog->SetFogDensity(0.02f);
    HeightFog->SetFogHeightFalloff(0.2f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.7f, 0.8f, 0.9f, 1.0f));

    // Create rain particle system
    RainParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RainParticles"));
    RainParticles->SetupAttachment(RootComponent);
    RainParticles->SetAutoActivate(false);

    // Create mist particle system
    MistParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MistParticles"));
    MistParticles->SetupAttachment(RootComponent);
    MistParticles->SetAutoActivate(false);

    // Create ambient audio component
    AmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudio->SetupAttachment(RootComponent);
    AmbientAudio->SetAutoActivate(true);
    AmbientAudio->SetVolumeMultiplier(0.5f);

    // Initialize default values
    CurrentWeatherType = EEnvArt_WeatherType::Clear;
    CurrentTimeOfDay = 12.0f;
    SunAngleOffset = 0.0f;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    TransitionDuration = 5.0f;
}

void AEnvArt_WeatherSystem::BeginPlay()
{
    Super::BeginPlay();

    InitializeWeatherConfigs();
    UpdateSunPosition();
    ApplyPresetWeather();
}

void AEnvArt_WeatherSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsTransitioning)
    {
        UpdateTransition(DeltaTime);
    }

    // Update sun position based on time of day
    UpdateSunPosition();
}

void AEnvArt_WeatherSystem::InitializeWeatherConfigs()
{
    // Clear sunny weather
    FEnvArt_WeatherConfig ClearWeather;
    ClearWeather.SunIntensity = 8.0f;
    ClearWeather.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    ClearWeather.SkyLightIntensity = 1.5f;
    ClearWeather.FogDensity = 0.01f;
    ClearWeather.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    ClearWeather.FogHeightFalloff = 0.2f;
    ClearWeather.bEnableRainParticles = false;
    ClearWeather.RainIntensity = 0.0f;
    ClearWeather.AmbientVolume = 0.3f;
    ClearWeather.WindStrength = 0.2f;
    WeatherConfigs.Add(EEnvArt_WeatherType::Clear, ClearWeather);

    // Partly cloudy
    FEnvArt_WeatherConfig PartlyCloudyWeather = ClearWeather;
    PartlyCloudyWeather.SunIntensity = 6.0f;
    PartlyCloudyWeather.SkyLightIntensity = 1.8f;
    PartlyCloudyWeather.FogDensity = 0.015f;
    PartlyCloudyWeather.AmbientVolume = 0.4f;
    PartlyCloudyWeather.WindStrength = 0.3f;
    WeatherConfigs.Add(EEnvArt_WeatherType::PartlyCloudy, PartlyCloudyWeather);

    // Overcast
    FEnvArt_WeatherConfig OvercastWeather = ClearWeather;
    OvercastWeather.SunIntensity = 4.0f;
    OvercastWeather.SunColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
    OvercastWeather.SkyLightIntensity = 2.5f;
    OvercastWeather.FogDensity = 0.025f;
    OvercastWeather.FogColor = FLinearColor(0.6f, 0.65f, 0.7f, 1.0f);
    OvercastWeather.AmbientVolume = 0.5f;
    OvercastWeather.WindStrength = 0.4f;
    WeatherConfigs.Add(EEnvArt_WeatherType::Overcast, OvercastWeather);

    // Light rain
    FEnvArt_WeatherConfig LightRainWeather = OvercastWeather;
    LightRainWeather.SunIntensity = 3.0f;
    LightRainWeather.FogDensity = 0.035f;
    LightRainWeather.bEnableRainParticles = true;
    LightRainWeather.RainIntensity = 0.3f;
    LightRainWeather.AmbientVolume = 0.6f;
    LightRainWeather.WindStrength = 0.5f;
    WeatherConfigs.Add(EEnvArt_WeatherType::LightRain, LightRainWeather);

    // Heavy rain
    FEnvArt_WeatherConfig HeavyRainWeather = LightRainWeather;
    HeavyRainWeather.SunIntensity = 2.0f;
    HeavyRainWeather.SunColor = FLinearColor(0.7f, 0.75f, 0.8f, 1.0f);
    HeavyRainWeather.SkyLightIntensity = 3.0f;
    HeavyRainWeather.FogDensity = 0.05f;
    HeavyRainWeather.FogColor = FLinearColor(0.5f, 0.55f, 0.6f, 1.0f);
    HeavyRainWeather.RainIntensity = 1.0f;
    HeavyRainWeather.AmbientVolume = 0.8f;
    HeavyRainWeather.WindStrength = 0.8f;
    WeatherConfigs.Add(EEnvArt_WeatherType::HeavyRain, HeavyRainWeather);

    // Storm
    FEnvArt_WeatherConfig StormWeather = HeavyRainWeather;
    StormWeather.SunIntensity = 1.5f;
    StormWeather.SunColor = FLinearColor(0.6f, 0.65f, 0.7f, 1.0f);
    StormWeather.SkyLightIntensity = 3.5f;
    StormWeather.FogDensity = 0.08f;
    StormWeather.FogColor = FLinearColor(0.4f, 0.45f, 0.5f, 1.0f);
    StormWeather.RainIntensity = 1.5f;
    StormWeather.AmbientVolume = 1.0f;
    StormWeather.WindStrength = 1.2f;
    WeatherConfigs.Add(EEnvArt_WeatherType::Storm, StormWeather);

    // Dense fog
    FEnvArt_WeatherConfig FogWeather = ClearWeather;
    FogWeather.SunIntensity = 2.5f;
    FogWeather.SunColor = FLinearColor(0.9f, 0.9f, 0.85f, 1.0f);
    FogWeather.SkyLightIntensity = 2.0f;
    FogWeather.FogDensity = 0.15f;
    FogWeather.FogColor = FLinearColor(0.8f, 0.82f, 0.85f, 1.0f);
    FogWeather.FogHeightFalloff = 0.1f;
    FogWeather.AmbientVolume = 0.2f;
    FogWeather.WindStrength = 0.1f;
    WeatherConfigs.Add(EEnvArt_WeatherType::Fog, FogWeather);

    // Light mist
    FEnvArt_WeatherConfig MistWeather = ClearWeather;
    MistWeather.SunIntensity = 5.0f;
    MistWeather.SkyLightIntensity = 1.8f;
    MistWeather.FogDensity = 0.04f;
    MistWeather.FogColor = FLinearColor(0.75f, 0.8f, 0.85f, 1.0f);
    MistWeather.AmbientVolume = 0.35f;
    MistWeather.WindStrength = 0.15f;
    WeatherConfigs.Add(EEnvArt_WeatherType::Mist, MistWeather);
}

void AEnvArt_WeatherSystem::SetWeatherType(EEnvArt_WeatherType NewWeatherType)
{
    if (NewWeatherType == CurrentWeatherType)
        return;

    CurrentWeatherType = NewWeatherType;
    bIsTransitioning = false;

    if (WeatherConfigs.Contains(NewWeatherType))
    {
        ApplyWeatherConfig(WeatherConfigs[NewWeatherType]);
    }

    UE_LOG(LogTemp, Log, TEXT("Weather changed to: %d"), (int32)NewWeatherType);
}

void AEnvArt_WeatherSystem::TransitionToWeather(EEnvArt_WeatherType NewWeatherType, float TransitionDuration)
{
    if (NewWeatherType == CurrentWeatherType)
        return;

    if (!WeatherConfigs.Contains(CurrentWeatherType) || !WeatherConfigs.Contains(NewWeatherType))
        return;

    TargetWeatherType = NewWeatherType;
    this->TransitionDuration = FMath::Max(TransitionDuration, 0.1f);
    StartWeatherConfig = WeatherConfigs[CurrentWeatherType];
    TargetWeatherConfig = WeatherConfigs[NewWeatherType];
    
    bIsTransitioning = true;
    TransitionProgress = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Starting weather transition from %d to %d over %.1f seconds"), 
           (int32)CurrentWeatherType, (int32)NewWeatherType, TransitionDuration);
}

void AEnvArt_WeatherSystem::UpdateTransition(float DeltaTime)
{
    if (!bIsTransitioning)
        return;

    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
        CurrentWeatherType = TargetWeatherType;
        ApplyWeatherConfig(TargetWeatherConfig);
        UE_LOG(LogTemp, Log, TEXT("Weather transition completed"));
    }
    else
    {
        FEnvArt_WeatherConfig LerpedConfig = LerpWeatherConfigs(StartWeatherConfig, TargetWeatherConfig, TransitionProgress);
        ApplyWeatherConfig(LerpedConfig);
    }
}

FEnvArt_WeatherConfig AEnvArt_WeatherSystem::LerpWeatherConfigs(const FEnvArt_WeatherConfig& A, const FEnvArt_WeatherConfig& B, float Alpha)
{
    FEnvArt_WeatherConfig Result;
    
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.RainIntensity = FMath::Lerp(A.RainIntensity, B.RainIntensity, Alpha);
    Result.AmbientVolume = FMath::Lerp(A.AmbientVolume, B.AmbientVolume, Alpha);
    Result.WindStrength = FMath::Lerp(A.WindStrength, B.WindStrength, Alpha);
    Result.WindDirection = FMath::Lerp(A.WindDirection, B.WindDirection, Alpha);
    
    Result.bEnableRainParticles = (Alpha > 0.5f) ? B.bEnableRainParticles : A.bEnableRainParticles;
    
    return Result;
}

void AEnvArt_WeatherSystem::ApplyWeatherConfig(const FEnvArt_WeatherConfig& Config)
{
    if (SunLight)
    {
        SunLight->SetIntensity(Config.SunIntensity);
        SunLight->SetLightColor(Config.SunColor);
    }

    if (SkyLight)
    {
        SkyLight->SetIntensity(Config.SkyLightIntensity);
    }

    if (HeightFog)
    {
        HeightFog->SetFogDensity(Config.FogDensity);
        HeightFog->SetFogInscatteringColor(Config.FogColor);
        HeightFog->SetFogHeightFalloff(Config.FogHeightFalloff);
    }

    UpdateParticleEffects();
    UpdateAmbientAudio();
}

void AEnvArt_WeatherSystem::UpdateParticleEffects()
{
    if (!WeatherConfigs.Contains(CurrentWeatherType))
        return;

    const FEnvArt_WeatherConfig& Config = WeatherConfigs[CurrentWeatherType];

    if (RainParticles)
    {
        if (Config.bEnableRainParticles && Config.RainIntensity > 0.0f)
        {
            if (!RainParticles->IsActive())
            {
                RainParticles->Activate();
            }
            // Set rain intensity parameter if the Niagara system has it
            RainParticles->SetFloatParameter(TEXT("RainIntensity"), Config.RainIntensity);
        }
        else
        {
            if (RainParticles->IsActive())
            {
                RainParticles->Deactivate();
            }
        }
    }

    if (MistParticles)
    {
        bool bShouldShowMist = (CurrentWeatherType == EEnvArt_WeatherType::Mist || 
                               CurrentWeatherType == EEnvArt_WeatherType::Fog);
        
        if (bShouldShowMist)
        {
            if (!MistParticles->IsActive())
            {
                MistParticles->Activate();
            }
            float MistIntensity = (CurrentWeatherType == EEnvArt_WeatherType::Fog) ? 1.0f : 0.5f;
            MistParticles->SetFloatParameter(TEXT("MistIntensity"), MistIntensity);
        }
        else
        {
            if (MistParticles->IsActive())
            {
                MistParticles->Deactivate();
            }
        }
    }
}

void AEnvArt_WeatherSystem::UpdateAmbientAudio()
{
    if (!WeatherConfigs.Contains(CurrentWeatherType) || !AmbientAudio)
        return;

    const FEnvArt_WeatherConfig& Config = WeatherConfigs[CurrentWeatherType];
    AmbientAudio->SetVolumeMultiplier(Config.AmbientVolume);

    // Switch ambient sound based on weather type
    USoundBase* NewSound = nullptr;
    switch (CurrentWeatherType)
    {
        case EEnvArt_WeatherType::LightRain:
        case EEnvArt_WeatherType::HeavyRain:
        case EEnvArt_WeatherType::Storm:
            NewSound = RainAmbientSound;
            break;
        case EEnvArt_WeatherType::Clear:
        case EEnvArt_WeatherType::PartlyCloudy:
            NewSound = ForestAmbientSound;
            break;
        default:
            NewSound = WindAmbientSound;
            break;
    }

    if (NewSound && AmbientAudio->GetSound() != NewSound)
    {
        AmbientAudio->SetSound(NewSound);
        if (!AmbientAudio->IsPlaying())
        {
            AmbientAudio->Play();
        }
    }
}

void AEnvArt_WeatherSystem::SetTimeOfDay(float TimeInHours)
{
    CurrentTimeOfDay = FMath::Clamp(TimeInHours, 0.0f, 24.0f);
    UpdateSunPosition();
}

void AEnvArt_WeatherSystem::UpdateSunPosition()
{
    if (!SunLight)
        return;

    // Convert time of day to sun angle (12:00 = overhead, 6:00 = horizon east, 18:00 = horizon west)
    float SunAngle = ((CurrentTimeOfDay - 12.0f) / 12.0f) * 180.0f + SunAngleOffset;
    
    // Create rotation for sun direction
    FRotator SunRotation = FRotator(-SunAngle, 0.0f, 0.0f);
    SunLight->SetWorldRotation(SunRotation);

    // Adjust sun intensity based on time of day (dimmer at dawn/dusk)
    float TimeBasedIntensity = 1.0f;
    if (CurrentTimeOfDay < 6.0f || CurrentTimeOfDay > 18.0f)
    {
        // Night time - very dim
        TimeBasedIntensity = 0.1f;
    }
    else if (CurrentTimeOfDay < 8.0f || CurrentTimeOfDay > 16.0f)
    {
        // Dawn/dusk - reduced intensity
        float DawnDuskFactor = FMath::Min(
            FMath::Abs(CurrentTimeOfDay - 6.0f) / 2.0f,  // Dawn fade in
            FMath::Abs(18.0f - CurrentTimeOfDay) / 2.0f   // Dusk fade out
        );
        TimeBasedIntensity = FMath::Lerp(0.1f, 1.0f, DawnDuskFactor);
    }

    if (WeatherConfigs.Contains(CurrentWeatherType))
    {
        float BaseIntensity = WeatherConfigs[CurrentWeatherType].SunIntensity;
        SunLight->SetIntensity(BaseIntensity * TimeBasedIntensity);
    }
}

void AEnvArt_WeatherSystem::SetSunIntensity(float Intensity)
{
    if (SunLight)
    {
        SunLight->SetIntensity(Intensity);
    }
}

void AEnvArt_WeatherSystem::SetFogDensity(float Density)
{
    if (HeightFog)
    {
        HeightFog->SetFogDensity(Density);
    }
}

void AEnvArt_WeatherSystem::ToggleRainEffects()
{
    if (RainParticles)
    {
        if (RainParticles->IsActive())
        {
            RainParticles->Deactivate();
        }
        else
        {
            RainParticles->Activate();
        }
    }
}

void AEnvArt_WeatherSystem::ApplyPresetWeather()
{
    SetWeatherType(CurrentWeatherType);
}