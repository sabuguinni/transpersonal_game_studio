#include "Light_AtmosphericSystem.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMaterialLibrary.h"

ALight_AtmosphericSystem::ALight_AtmosphericSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create directional light (sun)
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetIntensity(3.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.9f, 0.8f, 1.0f));
    SunLight->SetCastShadows(true);
    SunLight->SetCastVolumetricShadow(true);

    // Create sky light
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetIntensity(1.0f);
    SkyLight->SetLightColor(FLinearColor(0.5f, 0.7f, 1.0f, 1.0f));

    // Create sky atmosphere
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(RootComponent);

    // Create height fog
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);
    HeightFog->SetFogDensity(0.02f);
    HeightFog->SetFogHeightFalloff(0.2f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f));

    // Create volumetric clouds
    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    VolumetricClouds->SetupAttachment(RootComponent);

    // Initialize default values
    DayDurationMinutes = 20.0f;
    CurrentTimeOfDay = 12.0f;
    CurrentWeather = ELight_WeatherType::Clear;
    WeatherTransitionSpeed = 1.0f;
    bInteriorMode = false;
    InteriorAmbientMultiplier = 0.1f;
}

void ALight_AtmosphericSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultPresets();
    UpdateAtmosphere();
}

void ALight_AtmosphericSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateTimeOfDay(DeltaTime);
    
    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime / TransitionDuration;
        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            CurrentWeather = TargetWeather;
            bIsTransitioning = false;
        }
    }
    
    UpdateAtmosphere();
    UpdateMaterialParameters();
}

void ALight_AtmosphericSystem::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
}

void ALight_AtmosphericSystem::SetDayDuration(float Minutes)
{
    DayDurationMinutes = FMath::Max(Minutes, 1.0f);
}

ELight_TimeOfDay ALight_AtmosphericSystem::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
        return ELight_TimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

void ALight_AtmosphericSystem::SetWeather(ELight_WeatherType NewWeather)
{
    CurrentWeather = NewWeather;
    bIsTransitioning = false;
}

void ALight_AtmosphericSystem::TransitionToWeather(ELight_WeatherType NewWeather, float TransitionTime)
{
    if (NewWeather != CurrentWeather)
    {
        TargetWeather = NewWeather;
        TransitionDuration = FMath::Max(TransitionTime, 0.1f);
        TransitionProgress = 0.0f;
        bIsTransitioning = true;
    }
}

void ALight_AtmosphericSystem::ApplyAtmosphereSettings(const FLight_AtmosphereSettings& Settings)
{
    if (SunLight)
    {
        SunLight->SetIntensity(Settings.SunIntensity * (bInteriorMode ? InteriorAmbientMultiplier : 1.0f));
        SunLight->SetLightColor(Settings.SunColor);
    }

    if (SkyLight)
    {
        SkyLight->SetIntensity(Settings.SkyLightIntensity * (bInteriorMode ? InteriorAmbientMultiplier : 1.0f));
        SkyLight->SetLightColor(Settings.SkyColor);
    }

    if (HeightFog)
    {
        HeightFog->SetFogDensity(Settings.FogDensity);
        HeightFog->SetFogHeightFalloff(Settings.FogHeightFalloff);
        HeightFog->SetFogInscatteringColor(Settings.FogColor);
    }

    CurrentSettings = Settings;
}

void ALight_AtmosphericSystem::InterpolateAtmosphereSettings(const FLight_AtmosphereSettings& From, const FLight_AtmosphereSettings& To, float Alpha)
{
    FLight_AtmosphereSettings InterpolatedSettings;
    
    InterpolatedSettings.SunIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    InterpolatedSettings.SunColor = FMath::Lerp(From.SunColor, To.SunColor, Alpha);
    InterpolatedSettings.SkyLightIntensity = FMath::Lerp(From.SkyLightIntensity, To.SkyLightIntensity, Alpha);
    InterpolatedSettings.SkyColor = FMath::Lerp(From.SkyColor, To.SkyColor, Alpha);
    InterpolatedSettings.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    InterpolatedSettings.FogColor = FMath::Lerp(From.FogColor, To.FogColor, Alpha);
    InterpolatedSettings.FogHeightFalloff = FMath::Lerp(From.FogHeightFalloff, To.FogHeightFalloff, Alpha);
    InterpolatedSettings.CloudCoverage = FMath::Lerp(From.CloudCoverage, To.CloudCoverage, Alpha);
    InterpolatedSettings.CloudOpacity = FMath::Lerp(From.CloudOpacity, To.CloudOpacity, Alpha);

    ApplyAtmosphereSettings(InterpolatedSettings);
}

void ALight_AtmosphericSystem::InitializeDefaultPresets()
{
    // Time of day presets
    FLight_AtmosphereSettings DawnSettings;
    DawnSettings.SunIntensity = 1.5f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.3f;
    DawnSettings.SkyColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.FogDensity = 0.03f;
    DawnSettings.FogColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    TimeOfDayPresets.Add(ELight_TimeOfDay::Dawn, DawnSettings);

    FLight_AtmosphereSettings MiddaySettings;
    MiddaySettings.SunIntensity = 4.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    MiddaySettings.SkyLightIntensity = 1.5f;
    MiddaySettings.SkyColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
    MiddaySettings.FogDensity = 0.015f;
    MiddaySettings.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    TimeOfDayPresets.Add(ELight_TimeOfDay::Midday, MiddaySettings);

    FLight_AtmosphereSettings DuskSettings;
    DuskSettings.SunIntensity = 2.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.4f;
    DuskSettings.SkyColor = FLinearColor(0.8f, 0.4f, 0.6f, 1.0f);
    DuskSettings.FogDensity = 0.025f;
    DuskSettings.FogColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    TimeOfDayPresets.Add(ELight_TimeOfDay::Dusk, DuskSettings);

    FLight_AtmosphereSettings NightSettings;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SkyLightIntensity = 0.2f;
    NightSettings.SkyColor = FLinearColor(0.1f, 0.2f, 0.5f, 1.0f);
    NightSettings.FogDensity = 0.04f;
    NightSettings.FogColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    TimeOfDayPresets.Add(ELight_TimeOfDay::Night, NightSettings);

    // Weather presets
    FLight_AtmosphereSettings StormSettings;
    StormSettings.SunIntensity = 0.5f;
    StormSettings.SunColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
    StormSettings.SkyLightIntensity = 0.3f;
    StormSettings.SkyColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
    StormSettings.FogDensity = 0.08f;
    StormSettings.FogColor = FLinearColor(0.4f, 0.4f, 0.5f, 1.0f);
    StormSettings.CloudCoverage = 0.9f;
    StormSettings.CloudOpacity = 1.0f;
    WeatherPresets.Add(ELight_WeatherType::Storm, StormSettings);

    FLight_AtmosphereSettings VolcanicSettings;
    VolcanicSettings.SunIntensity = 1.0f;
    VolcanicSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    VolcanicSettings.SkyLightIntensity = 0.4f;
    VolcanicSettings.SkyColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    VolcanicSettings.FogDensity = 0.06f;
    VolcanicSettings.FogColor = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
    VolcanicSettings.CloudCoverage = 0.7f;
    VolcanicSettings.CloudOpacity = 0.9f;
    WeatherPresets.Add(ELight_WeatherType::Volcanic, VolcanicSettings);
}

void ALight_AtmosphericSystem::SaveCurrentAsPreset(ELight_TimeOfDay TimeSlot)
{
    TimeOfDayPresets.Add(TimeSlot, CurrentSettings);
}

void ALight_AtmosphericSystem::EnableInteriorMode(bool bEnable)
{
    bInteriorMode = bEnable;
}

void ALight_AtmosphericSystem::SetInteriorAmbientLevel(float AmbientMultiplier)
{
    InteriorAmbientMultiplier = FMath::Clamp(AmbientMultiplier, 0.01f, 1.0f);
}

void ALight_AtmosphericSystem::UpdateTimeOfDay(float DeltaTime)
{
    if (DayDurationMinutes > 0.0f)
    {
        float TimeIncrement = (DeltaTime / 60.0f) * (24.0f / DayDurationMinutes);
        CurrentTimeOfDay += TimeIncrement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
    }

    // Update sun rotation based on time
    if (SunLight)
    {
        float SunAngle = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at dawn
        FRotator SunRotation = FRotator(-SunAngle * 0.5f, 135.0f, 0.0f);
        SetActorRotation(SunRotation);
    }
}

void ALight_AtmosphericSystem::UpdateAtmosphere()
{
    FLight_AtmosphereSettings BaseSettings = GetInterpolatedTimeSettings();
    FLight_AtmosphereSettings FinalSettings = GetWeatherModifiedSettings(BaseSettings);
    
    if (bIsTransitioning && WeatherPresets.Contains(CurrentWeather) && WeatherPresets.Contains(TargetWeather))
    {
        FLight_AtmosphereSettings CurrentWeatherSettings = GetWeatherModifiedSettings(BaseSettings);
        FLight_AtmosphereSettings TargetWeatherSettings = WeatherPresets[TargetWeather];
        
        InterpolateAtmosphereSettings(CurrentWeatherSettings, TargetWeatherSettings, TransitionProgress);
    }
    else
    {
        ApplyAtmosphereSettings(FinalSettings);
    }
}

void ALight_AtmosphericSystem::UpdateMaterialParameters()
{
    if (GlobalLightingMPC)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            UKismetMaterialLibrary::SetScalarParameterValue(World, GlobalLightingMPC, FName("TimeOfDay"), CurrentTimeOfDay);
            UKismetMaterialLibrary::SetScalarParameterValue(World, GlobalLightingMPC, FName("SunIntensity"), CurrentSettings.SunIntensity);
            UKismetMaterialLibrary::SetVectorParameterValue(World, GlobalLightingMPC, FName("SunColor"), CurrentSettings.SunColor);
            UKismetMaterialLibrary::SetScalarParameterValue(World, GlobalLightingMPC, FName("FogDensity"), CurrentSettings.FogDensity);
            UKismetMaterialLibrary::SetVectorParameterValue(World, GlobalLightingMPC, FName("FogColor"), CurrentSettings.FogColor);
        }
    }
}

FLight_AtmosphereSettings ALight_AtmosphericSystem::GetInterpolatedTimeSettings() const
{
    ELight_TimeOfDay CurrentTimeEnum = GetCurrentTimeOfDayEnum();
    
    if (TimeOfDayPresets.Contains(CurrentTimeEnum))
    {
        return TimeOfDayPresets[CurrentTimeEnum];
    }
    
    // Return default settings if preset not found
    return FLight_AtmosphereSettings();
}

FLight_AtmosphereSettings ALight_AtmosphericSystem::GetWeatherModifiedSettings(const FLight_AtmosphereSettings& BaseSettings) const
{
    FLight_AtmosphereSettings ModifiedSettings = BaseSettings;
    
    if (WeatherPresets.Contains(CurrentWeather))
    {
        const FLight_AtmosphereSettings& WeatherMod = WeatherPresets[CurrentWeather];
        
        // Apply weather modifications
        ModifiedSettings.SunIntensity *= WeatherMod.SunIntensity / 3.0f; // Normalize against default
        ModifiedSettings.SkyLightIntensity *= WeatherMod.SkyLightIntensity;
        ModifiedSettings.FogDensity = FMath::Max(ModifiedSettings.FogDensity, WeatherMod.FogDensity);
        ModifiedSettings.FogColor = FMath::Lerp(ModifiedSettings.FogColor, WeatherMod.FogColor, 0.5f);
        ModifiedSettings.CloudCoverage = WeatherMod.CloudCoverage;
        ModifiedSettings.CloudOpacity = WeatherMod.CloudOpacity;
    }
    
    return ModifiedSettings;
}