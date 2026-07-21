#include "LightingSystemManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ALightingSystemManager::ALightingSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Initialize components
    SunLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLightComponent"));
    RootComponent = SunLightComponent;

    AtmosphereComponent = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("AtmosphereComponent"));
    CloudComponent = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("CloudComponent"));

    // Set default values
    CurrentWeather = ELight_WeatherType::Clear;
    WeatherTransitionSpeed = 1.0f;

    // Initialize Cretaceous settings
    CretaceousSettings.SunIntensity = 3.5f;
    CretaceousSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    CretaceousSettings.Temperature = 5500.0f;
    CretaceousSettings.RayleighScattering = 0.8f;
    CretaceousSettings.MieScattering = 0.6f;
    CretaceousSettings.AtmosphericDensity = 12.0f;

    // Initialize day/night cycle
    DayNightSettings.DayDurationMinutes = 24.0f;
    DayNightSettings.CurrentTimeOfDay = 12.0f;
    DayNightSettings.bAutoAdvanceTime = true;
    DayNightSettings.TimeMultiplier = 1.0f;
}

void ALightingSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingComponents();
    ApplyCretaceousAtmosphere();
    ConfigureLumenSettings();
}

void ALightingSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (DayNightSettings.bAutoAdvanceTime)
    {
        UpdateDayNightCycle(DeltaTime);
    }
    
    ApplyWeatherEffects();
}

void ALightingSystemManager::InitializeLightingComponents()
{
    if (SunLightComponent)
    {
        SunLightComponent->SetIntensity(CretaceousSettings.SunIntensity);
        SunLightComponent->SetLightColor(CretaceousSettings.SunColor);
        SunLightComponent->SetTemperature(CretaceousSettings.Temperature);
        SunLightComponent->SetCastShadows(true);
        SunLightComponent->SetCastVolumetricShadow(true);
    }

    if (AtmosphereComponent)
    {
        AtmosphereComponent->SetRayleighScatteringScale(CretaceousSettings.RayleighScattering);
        AtmosphereComponent->SetRayleighExponentialDistribution(CretaceousSettings.AtmosphericDensity);
        AtmosphereComponent->SetMieScatteringScale(CretaceousSettings.MieScattering);
        AtmosphereComponent->SetMieAbsorptionScale(0.4f);
        AtmosphereComponent->SetOtherAbsorptionScale(0.2f);
    }

    if (CloudComponent)
    {
        CloudComponent->SetLayerBottomAltitude(0.5f);
        CloudComponent->SetLayerHeight(2.0f);
        CloudComponent->SetViewSampleCountScale(0.8f);
        CloudComponent->SetReflectionSampleCountScale(0.6f);
    }
}

void ALightingSystemManager::UpdateDayNightCycle(float DeltaTime)
{
    // Advance time
    float TimeIncrement = (DeltaTime / 60.0f) * DayNightSettings.TimeMultiplier;
    DayNightSettings.CurrentTimeOfDay += TimeIncrement / DayNightSettings.DayDurationMinutes * 24.0f;
    
    // Wrap around 24 hours
    if (DayNightSettings.CurrentTimeOfDay >= 24.0f)
    {
        DayNightSettings.CurrentTimeOfDay -= 24.0f;
    }
    
    // Update sun position and lighting
    UpdateSunPosition(DayNightSettings.CurrentTimeOfDay);
    InterpolateLightingSettings(DayNightSettings.CurrentTimeOfDay);
}

void ALightingSystemManager::UpdateSunPosition(float TimeInHours)
{
    if (!SunLightComponent)
        return;

    // Calculate sun angle based on time (0-24 hours)
    float SunAngle = (TimeInHours - 6.0f) * 15.0f; // 6 AM = 0 degrees, 6 PM = 180 degrees
    float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    float SunAzimuth = 135.0f; // South-east facing for Cretaceous period

    // Clamp elevation to prevent negative values (underground sun)
    SunElevation = FMath::Max(SunElevation, -90.0f);

    FRotator SunRotation = FRotator(-SunElevation, SunAzimuth, 0.0f);
    SetActorRotation(SunRotation);
}

void ALightingSystemManager::InterpolateLightingSettings(float TimeInHours)
{
    if (!SunLightComponent)
        return;

    float IntensityMultiplier = 1.0f;
    FLinearColor ColorMultiplier = FLinearColor::White;

    // Dawn (5-7 AM)
    if (TimeInHours >= 5.0f && TimeInHours < 7.0f)
    {
        float Alpha = (TimeInHours - 5.0f) / 2.0f;
        IntensityMultiplier = FMath::Lerp(0.1f, 0.8f, Alpha);
        ColorMultiplier = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f); // Orange dawn
    }
    // Morning (7-10 AM)
    else if (TimeInHours >= 7.0f && TimeInHours < 10.0f)
    {
        float Alpha = (TimeInHours - 7.0f) / 3.0f;
        IntensityMultiplier = FMath::Lerp(0.8f, 1.0f, Alpha);
        ColorMultiplier = FMath::Lerp(FLinearColor(1.0f, 0.8f, 0.6f, 1.0f), FLinearColor::White, Alpha);
    }
    // Midday (10 AM - 4 PM)
    else if (TimeInHours >= 10.0f && TimeInHours < 16.0f)
    {
        IntensityMultiplier = 1.0f;
        ColorMultiplier = CretaceousSettings.SunColor;
    }
    // Evening (4-7 PM)
    else if (TimeInHours >= 16.0f && TimeInHours < 19.0f)
    {
        float Alpha = (TimeInHours - 16.0f) / 3.0f;
        IntensityMultiplier = FMath::Lerp(1.0f, 0.6f, Alpha);
        ColorMultiplier = FMath::Lerp(CretaceousSettings.SunColor, FLinearColor(1.0f, 0.6f, 0.3f, 1.0f), Alpha);
    }
    // Dusk (7-9 PM)
    else if (TimeInHours >= 19.0f && TimeInHours < 21.0f)
    {
        float Alpha = (TimeInHours - 19.0f) / 2.0f;
        IntensityMultiplier = FMath::Lerp(0.6f, 0.1f, Alpha);
        ColorMultiplier = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f); // Deep orange dusk
    }
    // Night (9 PM - 5 AM)
    else
    {
        IntensityMultiplier = 0.05f; // Moonlight
        ColorMultiplier = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f); // Blue moonlight
    }

    // Apply lighting changes
    float FinalIntensity = CretaceousSettings.SunIntensity * IntensityMultiplier;
    FLinearColor FinalColor = CretaceousSettings.SunColor * ColorMultiplier;

    SunLightComponent->SetIntensity(FinalIntensity);
    SunLightComponent->SetLightColor(FinalColor);
}

void ALightingSystemManager::ApplyWeatherEffects()
{
    if (!AtmosphereComponent || !CloudComponent)
        return;

    switch (CurrentWeather)
    {
        case ELight_WeatherType::Clear:
            AtmosphereComponent->SetRayleighScatteringScale(CretaceousSettings.RayleighScattering);
            CloudComponent->SetLayerBottomAltitude(0.5f);
            break;
            
        case ELight_WeatherType::Cloudy:
            AtmosphereComponent->SetRayleighScatteringScale(CretaceousSettings.RayleighScattering * 1.2f);
            CloudComponent->SetLayerBottomAltitude(0.3f);
            break;
            
        case ELight_WeatherType::Overcast:
            AtmosphereComponent->SetRayleighScatteringScale(CretaceousSettings.RayleighScattering * 1.5f);
            CloudComponent->SetLayerBottomAltitude(0.1f);
            if (SunLightComponent)
            {
                SunLightComponent->SetIntensity(CretaceousSettings.SunIntensity * 0.6f);
            }
            break;
            
        case ELight_WeatherType::Foggy:
            AtmosphereComponent->SetMieScatteringScale(CretaceousSettings.MieScattering * 2.0f);
            CloudComponent->SetLayerHeight(4.0f);
            break;
            
        case ELight_WeatherType::Stormy:
            AtmosphereComponent->SetRayleighScatteringScale(CretaceousSettings.RayleighScattering * 2.0f);
            if (SunLightComponent)
            {
                SunLightComponent->SetIntensity(CretaceousSettings.SunIntensity * 0.3f);
                SunLightComponent->SetLightColor(FLinearColor(0.8f, 0.8f, 0.9f, 1.0f));
            }
            break;
    }
}

void ALightingSystemManager::SetTimeOfDay(float TimeInHours)
{
    DayNightSettings.CurrentTimeOfDay = FMath::Clamp(TimeInHours, 0.0f, 24.0f);
    UpdateSunPosition(DayNightSettings.CurrentTimeOfDay);
    InterpolateLightingSettings(DayNightSettings.CurrentTimeOfDay);
}

void ALightingSystemManager::SetWeatherType(ELight_WeatherType NewWeather)
{
    CurrentWeather = NewWeather;
    ApplyWeatherEffects();
}

void ALightingSystemManager::ApplyCretaceousAtmosphere()
{
    InitializeLightingComponents();
    
    UE_LOG(LogTemp, Warning, TEXT("Cretaceous atmospheric lighting applied"));
}

void ALightingSystemManager::ConfigureLumenSettings()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    // Configure Lumen via console commands for Cretaceous period
    if (GEngine)
    {
        GEngine->Exec(World, TEXT("r.Lumen.GlobalIllumination 1"));
        GEngine->Exec(World, TEXT("r.Lumen.Reflections 1"));
        GEngine->Exec(World, TEXT("r.Lumen.ScreenProbeGather 1"));
        GEngine->Exec(World, TEXT("r.Lumen.TranslucencyGI 1"));
        GEngine->Exec(World, TEXT("r.Lumen.GlobalIllumination.MaxLuminance 50.0"));
        GEngine->Exec(World, TEXT("r.Lumen.DiffuseColorBoost 1.2"));
        GEngine->Exec(World, TEXT("r.Lumen.SkylightLeaking 0.8"));
    }
}

ELight_TimeOfDay ALightingSystemManager::GetCurrentTimeOfDay() const
{
    float Time = DayNightSettings.CurrentTimeOfDay;
    
    if (Time >= 5.0f && Time < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (Time >= 7.0f && Time < 10.0f)
        return ELight_TimeOfDay::Morning;
    else if (Time >= 10.0f && Time < 16.0f)
        return ELight_TimeOfDay::Midday;
    else if (Time >= 16.0f && Time < 19.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (Time >= 19.0f && Time < 21.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

float ALightingSystemManager::GetSunIntensity() const
{
    return SunLightComponent ? SunLightComponent->Intensity : 0.0f;
}

FLinearColor ALightingSystemManager::GetSunColor() const
{
    return SunLightComponent ? SunLightComponent->GetLightColor() : FLinearColor::White;
}

bool ALightingSystemManager::IsNightTime() const
{
    ELight_TimeOfDay CurrentTime = GetCurrentTimeOfDay();
    return CurrentTime == ELight_TimeOfDay::Night;
}