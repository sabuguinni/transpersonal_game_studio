#include "Light_DynamicLightingManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"

ALight_DynamicLightingManager::ALight_DynamicLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default time settings
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnSettings.SunRotation = FRotator(-10.0f, 90.0f, 0.0f);
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    NoonSettings.SunIntensity = 8.0f;
    NoonSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    NoonSettings.SunRotation = FRotator(-80.0f, 180.0f, 0.0f);
    NoonSettings.FogDensity = 0.01f;
    NoonSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    DuskSettings.SunIntensity = 1.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DuskSettings.SunRotation = FRotator(-5.0f, 270.0f, 0.0f);
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);

    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SunRotation = FRotator(10.0f, 0.0f, 0.0f);
    NightSettings.FogDensity = 0.03f;
    NightSettings.FogColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);

    // Initialize weather settings
    CurrentWeather.WeatherType = EWeatherType::Clear;
    CurrentWeather.CloudCoverage = 0.3f;
    CurrentWeather.RainIntensity = 0.0f;
    CurrentWeather.WindStrength = 1.0f;
    CurrentWeather.Visibility = 1.0f;

    TargetWeather = CurrentWeather;
}

void ALight_DynamicLightingManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingActors();
    UpdateLighting();
}

void ALight_DynamicLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        // Update time of day
        CurrentTimeOfDay += DeltaTime / DayDuration;
        if (CurrentTimeOfDay >= 1.0f)
        {
            CurrentTimeOfDay -= 1.0f;
        }

        UpdateLighting();
    }

    // Handle weather transitions
    if (bIsTransitioningWeather)
    {
        WeatherTransitionTimer += DeltaTime;
        float Alpha = FMath::Clamp(WeatherTransitionTimer / WeatherTransitionSpeed, 0.0f, 1.0f);

        // Interpolate weather settings
        CurrentWeather.CloudCoverage = FMath::Lerp(CurrentWeather.CloudCoverage, TargetWeather.CloudCoverage, Alpha);
        CurrentWeather.RainIntensity = FMath::Lerp(CurrentWeather.RainIntensity, TargetWeather.RainIntensity, Alpha);
        CurrentWeather.WindStrength = FMath::Lerp(CurrentWeather.WindStrength, TargetWeather.WindStrength, Alpha);
        CurrentWeather.Visibility = FMath::Lerp(CurrentWeather.Visibility, TargetWeather.Visibility, Alpha);

        if (Alpha >= 1.0f)
        {
            CurrentWeather = TargetWeather;
            bIsTransitioningWeather = false;
            WeatherTransitionTimer = 0.0f;
        }

        ApplyWeatherSettings(CurrentWeather);
    }
}

void ALight_DynamicLightingManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight (Sun)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find SkyAtmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
    }

    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    // Find VolumetricCloud
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        VolumetricClouds = Cast<AVolumetricCloud>(FoundActors[0]);
    }
}

void ALight_DynamicLightingManager::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 1.0f);
    UpdateLighting();
}

void ALight_DynamicLightingManager::SetWeather(EWeatherType NewWeatherType, float TransitionTime)
{
    TargetWeather.WeatherType = NewWeatherType;
    
    switch (NewWeatherType)
    {
        case EWeatherType::Clear:
            TargetWeather.CloudCoverage = 0.2f;
            TargetWeather.RainIntensity = 0.0f;
            TargetWeather.WindStrength = 0.5f;
            TargetWeather.Visibility = 1.0f;
            break;
            
        case EWeatherType::Cloudy:
            TargetWeather.CloudCoverage = 0.7f;
            TargetWeather.RainIntensity = 0.0f;
            TargetWeather.WindStrength = 1.0f;
            TargetWeather.Visibility = 0.8f;
            break;
            
        case EWeatherType::Rainy:
            TargetWeather.CloudCoverage = 0.9f;
            TargetWeather.RainIntensity = 0.6f;
            TargetWeather.WindStrength = 1.5f;
            TargetWeather.Visibility = 0.5f;
            break;
            
        case EWeatherType::Stormy:
            TargetWeather.CloudCoverage = 1.0f;
            TargetWeather.RainIntensity = 1.0f;
            TargetWeather.WindStrength = 2.5f;
            TargetWeather.Visibility = 0.3f;
            break;
            
        case EWeatherType::Foggy:
            TargetWeather.CloudCoverage = 0.8f;
            TargetWeather.RainIntensity = 0.0f;
            TargetWeather.WindStrength = 0.2f;
            TargetWeather.Visibility = 0.2f;
            break;
    }

    WeatherTransitionSpeed = TransitionTime;
    WeatherTransitionTimer = 0.0f;
    bIsTransitioningWeather = true;
}

void ALight_DynamicLightingManager::UpdateLighting()
{
    FLight_TimeOfDaySettings CurrentSettings = InterpolateTimeSettings(CurrentTimeOfDay);
    ApplyTimeSettings(CurrentSettings);
    ApplyWeatherSettings(CurrentWeather);
}

FLight_TimeOfDaySettings ALight_DynamicLightingManager::GetCurrentTimeSettings() const
{
    return InterpolateTimeSettings(CurrentTimeOfDay);
}

FLight_TimeOfDaySettings ALight_DynamicLightingManager::InterpolateTimeSettings(float TimeOfDay) const
{
    FLight_TimeOfDaySettings Result;
    
    if (TimeOfDay < 0.25f) // Night to Dawn
    {
        float Alpha = TimeOfDay / 0.25f;
        Result.SunIntensity = FMath::Lerp(NightSettings.SunIntensity, DawnSettings.SunIntensity, Alpha);
        Result.SunColor = FLinearColor::LerpUsingHSV(NightSettings.SunColor, DawnSettings.SunColor, Alpha);
        Result.SunRotation = FMath::Lerp(NightSettings.SunRotation, DawnSettings.SunRotation, Alpha);
        Result.FogDensity = FMath::Lerp(NightSettings.FogDensity, DawnSettings.FogDensity, Alpha);
        Result.FogColor = FLinearColor::LerpUsingHSV(NightSettings.FogColor, DawnSettings.FogColor, Alpha);
    }
    else if (TimeOfDay < 0.5f) // Dawn to Noon
    {
        float Alpha = (TimeOfDay - 0.25f) / 0.25f;
        Result.SunIntensity = FMath::Lerp(DawnSettings.SunIntensity, NoonSettings.SunIntensity, Alpha);
        Result.SunColor = FLinearColor::LerpUsingHSV(DawnSettings.SunColor, NoonSettings.SunColor, Alpha);
        Result.SunRotation = FMath::Lerp(DawnSettings.SunRotation, NoonSettings.SunRotation, Alpha);
        Result.FogDensity = FMath::Lerp(DawnSettings.FogDensity, NoonSettings.FogDensity, Alpha);
        Result.FogColor = FLinearColor::LerpUsingHSV(DawnSettings.FogColor, NoonSettings.FogColor, Alpha);
    }
    else if (TimeOfDay < 0.75f) // Noon to Dusk
    {
        float Alpha = (TimeOfDay - 0.5f) / 0.25f;
        Result.SunIntensity = FMath::Lerp(NoonSettings.SunIntensity, DuskSettings.SunIntensity, Alpha);
        Result.SunColor = FLinearColor::LerpUsingHSV(NoonSettings.SunColor, DuskSettings.SunColor, Alpha);
        Result.SunRotation = FMath::Lerp(NoonSettings.SunRotation, DuskSettings.SunRotation, Alpha);
        Result.FogDensity = FMath::Lerp(NoonSettings.FogDensity, DuskSettings.FogDensity, Alpha);
        Result.FogColor = FLinearColor::LerpUsingHSV(NoonSettings.FogColor, DuskSettings.FogColor, Alpha);
    }
    else // Dusk to Night
    {
        float Alpha = (TimeOfDay - 0.75f) / 0.25f;
        Result.SunIntensity = FMath::Lerp(DuskSettings.SunIntensity, NightSettings.SunIntensity, Alpha);
        Result.SunColor = FLinearColor::LerpUsingHSV(DuskSettings.SunColor, NightSettings.SunColor, Alpha);
        Result.SunRotation = FMath::Lerp(DuskSettings.SunRotation, NightSettings.SunRotation, Alpha);
        Result.FogDensity = FMath::Lerp(DuskSettings.FogDensity, NightSettings.FogDensity, Alpha);
        Result.FogColor = FLinearColor::LerpUsingHSV(DuskSettings.FogColor, NightSettings.FogColor, Alpha);
    }
    
    return Result;
}

void ALight_DynamicLightingManager::ApplyTimeSettings(const FLight_TimeOfDaySettings& Settings)
{
    if (SunLight)
    {
        UDirectionalLightComponent* LightComp = SunLight->GetComponent();
        if (LightComp)
        {
            LightComp->SetIntensity(Settings.SunIntensity);
            LightComp->SetLightColor(Settings.SunColor);
        }
        SunLight->SetActorRotation(Settings.SunRotation);
    }

    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }
}

void ALight_DynamicLightingManager::ApplyWeatherSettings(const FLight_WeatherSettings& Weather)
{
    if (VolumetricClouds)
    {
        UVolumetricCloudComponent* CloudComp = VolumetricClouds->GetComponent();
        if (CloudComp)
        {
            // Apply cloud coverage and other weather effects
            // Note: Specific properties depend on UE5 version
        }
    }

    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        if (FogComp)
        {
            float WeatherFogMultiplier = 1.0f / FMath::Max(Weather.Visibility, 0.1f);
            FogComp->SetFogDensity(FogComp->GetFogDensity() * WeatherFogMultiplier);
        }
    }
}

void ALight_DynamicLightingManager::StartRain(float Intensity)
{
    SetWeather(EWeatherType::Rainy, 3.0f);
    TargetWeather.RainIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void ALight_DynamicLightingManager::StopRain()
{
    SetWeather(EWeatherType::Clear, 5.0f);
}

void ALight_DynamicLightingManager::StartStorm()
{
    SetWeather(EWeatherType::Stormy, 2.0f);
}

void ALight_DynamicLightingManager::ClearWeather()
{
    SetWeather(EWeatherType::Clear, 10.0f);
}