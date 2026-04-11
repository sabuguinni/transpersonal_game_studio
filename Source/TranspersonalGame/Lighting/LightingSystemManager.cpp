#include "LightingSystemManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

ALightingSystemManager::ALightingSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create sun light (directional light)
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetIntensity(3.0f);
    SunLight->SetLightColor(FLinearColor::White);
    SunLight->SetCastShadows(true);
    SunLight->SetCastVolumetricShadow(true);

    // Create sky light
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetIntensity(1.0f);
    SkyLight->SetLightColor(FLinearColor(0.2f, 0.4f, 1.0f));
    SkyLight->SetSourceType(ESkyLightSourceType::SLS_CapturedScene);

    // Create height fog
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);
    HeightFog->SetFogDensity(0.02f);
    HeightFog->SetFogHeightFalloff(0.2f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.8f, 0.9f, 1.0f));
    HeightFog->SetVolumetricFog(true);

    // Create sky dome mesh component
    SkyDome = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkyDome"));
    SkyDome->SetupAttachment(RootComponent);
    SkyDome->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkyDome->SetCastShadow(false);

    // Initialize default values
    TimeOfDayHours = 12.0f;
    DayDurationMinutes = 20.0f;
    CurrentWeather = EWeatherType::Clear;
    WeatherTransitionSpeed = 1.0f;
    bAutoAdvanceTime = true;
    bAutoWeatherChanges = true;
}

void ALightingSystemManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeDefaultConfigurations();
    UpdateLightingConfiguration();
    EnableLumenGlobalIllumination();
    SetVolumetricFogEnabled(true);
}

void ALightingSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Auto advance time
    if (bAutoAdvanceTime)
    {
        float HoursPerSecond = 24.0f / (DayDurationMinutes * 60.0f);
        TimeOfDayHours += HoursPerSecond * DeltaTime;
        
        if (TimeOfDayHours >= 24.0f)
        {
            TimeOfDayHours -= 24.0f;
        }
    }

    // Update sun position
    UpdateSunPosition();

    // Handle lighting transitions
    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime / TransitionDuration;
        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            bIsTransitioning = false;
            CurrentConfig = TargetConfig;
        }
        
        FLightingConfiguration StartConfig = CurrentConfig;
        InterpolateLightingValues(StartConfig, TargetConfig, TransitionProgress);
    }
    else
    {
        UpdateLightingConfiguration();
    }

    // Handle weather changes
    if (bAutoWeatherChanges)
    {
        HandleWeatherTransition(DeltaTime);
    }
}

void ALightingSystemManager::SetTimeOfDay(float Hours)
{
    TimeOfDayHours = FMath::Clamp(Hours, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateLightingConfiguration();
}

void ALightingSystemManager::SetWeather(EWeatherType NewWeather)
{
    if (CurrentWeather != NewWeather)
    {
        CurrentWeather = NewWeather;
        UpdateLightingConfiguration();
    }
}

ETimeOfDay ALightingSystemManager::GetCurrentTimeOfDay() const
{
    if (TimeOfDayHours >= 5.0f && TimeOfDayHours < 8.0f)
        return ETimeOfDay::Dawn;
    else if (TimeOfDayHours >= 8.0f && TimeOfDayHours < 11.0f)
        return ETimeOfDay::Morning;
    else if (TimeOfDayHours >= 11.0f && TimeOfDayHours < 15.0f)
        return ETimeOfDay::Midday;
    else if (TimeOfDayHours >= 15.0f && TimeOfDayHours < 18.0f)
        return ETimeOfDay::Afternoon;
    else if (TimeOfDayHours >= 18.0f && TimeOfDayHours < 21.0f)
        return ETimeOfDay::Dusk;
    else
        return ETimeOfDay::Night;
}

void ALightingSystemManager::TransitionToConfiguration(const FLightingConfiguration& TargetConfig, float TransitionTime)
{
    this->TargetConfig = TargetConfig;
    TransitionDuration = TransitionTime;
    TransitionProgress = 0.0f;
    bIsTransitioning = true;
}

void ALightingSystemManager::EnableLumenGlobalIllumination()
{
    // Enable Lumen features
    if (SunLight)
    {
        SunLight->SetCastShadows(true);
        SunLight->SetCastVolumetricShadow(true);
    }

    if (SkyLight)
    {
        SkyLight->SetRealTimeCapture(true);
        SkyLight->RecaptureSky();
    }

    UE_LOG(LogTemp, Log, TEXT("Lumen Global Illumination enabled"));
}

void ALightingSystemManager::SetVolumetricFogEnabled(bool bEnabled)
{
    if (HeightFog)
    {
        HeightFog->SetVolumetricFog(bEnabled);
        HeightFog->SetVolumetricFogScatteringDistribution(0.2f);
        HeightFog->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f));
        HeightFog->SetVolumetricFogExtinctionScale(1.0f);
    }
}

void ALightingSystemManager::UpdateAtmosphericScattering()
{
    // Update atmospheric scattering based on time of day and weather
    if (HeightFog)
    {
        ETimeOfDay CurrentTime = GetCurrentTimeOfDay();
        
        FLinearColor ScatteringColor;
        switch (CurrentTime)
        {
            case ETimeOfDay::Dawn:
                ScatteringColor = FLinearColor(1.0f, 0.7f, 0.4f); // Warm orange
                break;
            case ETimeOfDay::Morning:
                ScatteringColor = FLinearColor(1.0f, 0.9f, 0.7f); // Soft yellow
                break;
            case ETimeOfDay::Midday:
                ScatteringColor = FLinearColor(0.8f, 0.9f, 1.0f); // Cool blue
                break;
            case ETimeOfDay::Afternoon:
                ScatteringColor = FLinearColor(1.0f, 0.8f, 0.6f); // Warm white
                break;
            case ETimeOfDay::Dusk:
                ScatteringColor = FLinearColor(1.0f, 0.5f, 0.2f); // Deep orange
                break;
            case ETimeOfDay::Night:
                ScatteringColor = FLinearColor(0.3f, 0.4f, 0.8f); // Deep blue
                break;
        }

        HeightFog->SetFogInscatteringColor(ScatteringColor);
    }
}

void ALightingSystemManager::UpdateSunPosition()
{
    if (SunLight)
    {
        // Calculate sun angle based on time of day
        float SunAngle = (TimeOfDayHours - 6.0f) * 15.0f; // 15 degrees per hour, sunrise at 6 AM
        float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
        float SunAzimuth = (TimeOfDayHours - 6.0f) * 15.0f;

        // Clamp elevation to reasonable values
        SunElevation = FMath::Clamp(SunElevation, -90.0f, 90.0f);

        // Set sun rotation
        FRotator SunRotation = FRotator(SunElevation, SunAzimuth, 0.0f);
        SunLight->SetWorldRotation(SunRotation);

        // Adjust sun intensity based on elevation
        float IntensityMultiplier = FMath::Max(0.0f, FMath::Sin(FMath::DegreesToRadians(SunElevation)));
        SunLight->SetIntensity(CurrentConfig.SunIntensity * IntensityMultiplier);
    }
}

void ALightingSystemManager::UpdateLightingConfiguration()
{
    ETimeOfDay CurrentTime = GetCurrentTimeOfDay();
    
    // Get base configuration for time of day
    FLightingConfiguration BaseConfig;
    if (TimeOfDayConfigs.Contains(CurrentTime))
    {
        BaseConfig = TimeOfDayConfigs[CurrentTime];
    }

    // Apply weather modifications
    if (WeatherConfigs.Contains(CurrentWeather))
    {
        FLightingConfiguration WeatherMod = WeatherConfigs[CurrentWeather];
        
        // Blend weather effects with time of day
        BaseConfig.SunIntensity *= WeatherMod.SunIntensity;
        BaseConfig.SkyIntensity *= WeatherMod.SkyIntensity;
        BaseConfig.FogDensity += WeatherMod.FogDensity;
        BaseConfig.AtmosphereThickness *= WeatherMod.AtmosphereThickness;
    }

    // Apply configuration to lighting components
    if (SunLight)
    {
        SunLight->SetLightColor(BaseConfig.SunColor);
    }

    if (SkyLight)
    {
        SkyLight->SetLightColor(BaseConfig.SkyColor);
        SkyLight->SetIntensity(BaseConfig.SkyIntensity);
    }

    if (HeightFog)
    {
        HeightFog->SetFogDensity(BaseConfig.FogDensity);
        HeightFog->SetFogHeightFalloff(BaseConfig.FogHeightFalloff);
        HeightFog->SetFogInscatteringColor(BaseConfig.FogColor);
    }

    CurrentConfig = BaseConfig;
    UpdateAtmosphericScattering();
}

void ALightingSystemManager::InterpolateLightingValues(const FLightingConfiguration& From, const FLightingConfiguration& To, float Alpha)
{
    FLightingConfiguration InterpolatedConfig;
    
    InterpolatedConfig.SunColor = FMath::Lerp(From.SunColor, To.SunColor, Alpha);
    InterpolatedConfig.SunIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    InterpolatedConfig.SkyColor = FMath::Lerp(From.SkyColor, To.SkyColor, Alpha);
    InterpolatedConfig.SkyIntensity = FMath::Lerp(From.SkyIntensity, To.SkyIntensity, Alpha);
    InterpolatedConfig.FogColor = FMath::Lerp(From.FogColor, To.FogColor, Alpha);
    InterpolatedConfig.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    InterpolatedConfig.FogHeightFalloff = FMath::Lerp(From.FogHeightFalloff, To.FogHeightFalloff, Alpha);
    InterpolatedConfig.AtmosphereThickness = FMath::Lerp(From.AtmosphereThickness, To.AtmosphereThickness, Alpha);
    InterpolatedConfig.VolumetricScatteringIntensity = FMath::Lerp(From.VolumetricScatteringIntensity, To.VolumetricScatteringIntensity, Alpha);

    // Apply interpolated values
    if (SunLight)
    {
        SunLight->SetLightColor(InterpolatedConfig.SunColor);
    }

    if (SkyLight)
    {
        SkyLight->SetLightColor(InterpolatedConfig.SkyColor);
        SkyLight->SetIntensity(InterpolatedConfig.SkyIntensity);
    }

    if (HeightFog)
    {
        HeightFog->SetFogDensity(InterpolatedConfig.FogDensity);
        HeightFog->SetFogHeightFalloff(InterpolatedConfig.FogHeightFalloff);
        HeightFog->SetFogInscatteringColor(InterpolatedConfig.FogColor);
    }
}

void ALightingSystemManager::InitializeDefaultConfigurations()
{
    // Dawn configuration
    FLightingConfiguration DawnConfig;
    DawnConfig.SunColor = FLinearColor(1.0f, 0.7f, 0.4f);
    DawnConfig.SunIntensity = 2.0f;
    DawnConfig.SkyColor = FLinearColor(0.8f, 0.6f, 0.9f);
    DawnConfig.SkyIntensity = 0.8f;
    DawnConfig.FogColor = FLinearColor(1.0f, 0.8f, 0.6f);
    DawnConfig.FogDensity = 0.03f;
    TimeOfDayConfigs.Add(ETimeOfDay::Dawn, DawnConfig);

    // Morning configuration
    FLightingConfiguration MorningConfig;
    MorningConfig.SunColor = FLinearColor(1.0f, 0.95f, 0.8f);
    MorningConfig.SunIntensity = 3.5f;
    MorningConfig.SkyColor = FLinearColor(0.6f, 0.8f, 1.0f);
    MorningConfig.SkyIntensity = 1.0f;
    MorningConfig.FogColor = FLinearColor(0.9f, 0.9f, 1.0f);
    MorningConfig.FogDensity = 0.02f;
    TimeOfDayConfigs.Add(ETimeOfDay::Morning, MorningConfig);

    // Midday configuration
    FLightingConfiguration MiddayConfig;
    MiddayConfig.SunColor = FLinearColor::White;
    MiddayConfig.SunIntensity = 5.0f;
    MiddayConfig.SkyColor = FLinearColor(0.3f, 0.6f, 1.0f);
    MiddayConfig.SkyIntensity = 1.2f;
    MiddayConfig.FogColor = FLinearColor(0.8f, 0.9f, 1.0f);
    MiddayConfig.FogDensity = 0.015f;
    TimeOfDayConfigs.Add(ETimeOfDay::Midday, MiddayConfig);

    // Afternoon configuration
    FLightingConfiguration AfternoonConfig;
    AfternoonConfig.SunColor = FLinearColor(1.0f, 0.9f, 0.7f);
    AfternoonConfig.SunIntensity = 4.0f;
    AfternoonConfig.SkyColor = FLinearColor(0.4f, 0.7f, 1.0f);
    AfternoonConfig.SkyIntensity = 1.1f;
    AfternoonConfig.FogColor = FLinearColor(0.9f, 0.8f, 0.7f);
    AfternoonConfig.FogDensity = 0.018f;
    TimeOfDayConfigs.Add(ETimeOfDay::Afternoon, AfternoonConfig);

    // Dusk configuration
    FLightingConfiguration DuskConfig;
    DuskConfig.SunColor = FLinearColor(1.0f, 0.5f, 0.2f);
    DuskConfig.SunIntensity = 2.5f;
    DuskConfig.SkyColor = FLinearColor(0.8f, 0.4f, 0.6f);
    DuskConfig.SkyIntensity = 0.9f;
    DuskConfig.FogColor = FLinearColor(1.0f, 0.6f, 0.4f);
    DuskConfig.FogDensity = 0.025f;
    TimeOfDayConfigs.Add(ETimeOfDay::Dusk, DuskConfig);

    // Night configuration
    FLightingConfiguration NightConfig;
    NightConfig.SunColor = FLinearColor(0.3f, 0.4f, 0.8f);
    NightConfig.SunIntensity = 0.1f;
    NightConfig.SkyColor = FLinearColor(0.1f, 0.2f, 0.5f);
    NightConfig.SkyIntensity = 0.3f;
    NightConfig.FogColor = FLinearColor(0.4f, 0.5f, 0.8f);
    NightConfig.FogDensity = 0.01f;
    TimeOfDayConfigs.Add(ETimeOfDay::Night, NightConfig);

    // Weather configurations
    FLightingConfiguration ClearWeather;
    ClearWeather.SunIntensity = 1.0f;
    ClearWeather.SkyIntensity = 1.0f;
    ClearWeather.FogDensity = 0.0f;
    WeatherConfigs.Add(EWeatherType::Clear, ClearWeather);

    FLightingConfiguration CloudyWeather;
    CloudyWeather.SunIntensity = 0.7f;
    CloudyWeather.SkyIntensity = 1.2f;
    CloudyWeather.FogDensity = 0.005f;
    WeatherConfigs.Add(EWeatherType::Cloudy, CloudyWeather);

    FLightingConfiguration OvercastWeather;
    OvercastWeather.SunIntensity = 0.4f;
    OvercastWeather.SkyIntensity = 1.5f;
    OvercastWeather.FogDensity = 0.01f;
    WeatherConfigs.Add(EWeatherType::Overcast, OvercastWeather);

    FLightingConfiguration FoggyWeather;
    FoggyWeather.SunIntensity = 0.3f;
    FoggyWeather.SkyIntensity = 0.8f;
    FoggyWeather.FogDensity = 0.08f;
    WeatherConfigs.Add(EWeatherType::Foggy, FoggyWeather);

    FLightingConfiguration StormyWeather;
    StormyWeather.SunIntensity = 0.2f;
    StormyWeather.SkyIntensity = 0.6f;
    StormyWeather.FogDensity = 0.04f;
    WeatherConfigs.Add(EWeatherType::Stormy, StormyWeather);

    FLightingConfiguration DustyWeather;
    DustyWeather.SunIntensity = 0.6f;
    DustyWeather.SkyIntensity = 0.9f;
    DustyWeather.FogDensity = 0.06f;
    WeatherConfigs.Add(EWeatherType::Dusty, DustyWeather);
}

void ALightingSystemManager::HandleWeatherTransition(float DeltaTime)
{
    WeatherTransitionTimer += DeltaTime;
    
    if (WeatherTransitionTimer >= NextWeatherChangeTime)
    {
        // Randomly select new weather
        TArray<EWeatherType> WeatherTypes = {
            EWeatherType::Clear,
            EWeatherType::Cloudy,
            EWeatherType::Overcast,
            EWeatherType::Foggy,
            EWeatherType::Dusty
        };
        
        EWeatherType NewWeather = WeatherTypes[FMath::RandRange(0, WeatherTypes.Num() - 1)];
        
        if (NewWeather != CurrentWeather)
        {
            SetWeather(NewWeather);
        }
        
        // Set next weather change time (5-15 minutes)
        NextWeatherChangeTime = FMath::RandRange(300.0f, 900.0f);
        WeatherTransitionTimer = 0.0f;
    }
}