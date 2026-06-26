#include "CretaceousLightingSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ACretaceousLightingSystem::ACretaceousLightingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 Hz — sufficient for day/night

    // Default afternoon config (Cretaceous golden hour)
    AfternoonConfig.SunPitch = -38.0f;
    AfternoonConfig.SunIntensity = 12.0f;
    AfternoonConfig.SunColor = FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);
    AfternoonConfig.SkyLightIntensity = 2.0f;
    AfternoonConfig.FogDensity = 0.035f;
    AfternoonConfig.ColorTemperature = 5500.0f;

    // Dawn config
    DawnConfig.SunPitch = -5.0f;
    DawnConfig.SunIntensity = 4.0f;
    DawnConfig.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnConfig.SkyLightIntensity = 0.8f;
    DawnConfig.FogDensity = 0.08f;
    DawnConfig.ColorTemperature = 3200.0f;

    // Night config
    NightConfig.SunPitch = 30.0f;
    NightConfig.SunIntensity = 0.1f;
    NightConfig.SunColor = FLinearColor(0.2f, 0.25f, 0.5f, 1.0f);
    NightConfig.SkyLightIntensity = 0.3f;
    NightConfig.FogDensity = 0.06f;
    NightConfig.ColorTemperature = 8000.0f;

    // Atmosphere defaults
    AtmosphereSettings.VolumetricFogDensity = 0.035f;
    AtmosphereSettings.VolumetricFogScattering = 0.35f;
    AtmosphereSettings.CloudLayerBottom = 2.0f;
    AtmosphereSettings.CloudLayerHeight = 12.0f;
    AtmosphereSettings.FogAlbedo = FLinearColor(0.863f, 0.922f, 0.784f, 1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingSystem::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find sun and skylight if not assigned
    if (!SunActor)
    {
        for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
        {
            SunActor = *It;
            break;
        }
    }

    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }

    // Apply initial afternoon preset
    ApplyAfternoonPreset();
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        TickDayNightCycle(DeltaTime);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Day/Night Cycle
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingSystem::TickDayNightCycle(float DeltaTime)
{
    if (DayDurationSeconds <= 0.0f) return;

    ElapsedTime += DeltaTime;
    CurrentTimeNormalized = FMath::Fmod(ElapsedTime / DayDurationSeconds, 1.0f);

    UpdateSunPosition(CurrentTimeNormalized);
    UpdateSkyLightIntensity(CurrentTimeNormalized);
    UpdateFogDensity(CurrentTimeNormalized);

    // Update time of day enum
    if (CurrentTimeNormalized < 0.15f)
        CurrentTimeOfDay = ELight_TimeOfDay::Night;
    else if (CurrentTimeNormalized < 0.25f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
    else if (CurrentTimeNormalized < 0.4f)
        CurrentTimeOfDay = ELight_TimeOfDay::Morning;
    else if (CurrentTimeNormalized < 0.6f)
        CurrentTimeOfDay = ELight_TimeOfDay::Midday;
    else if (CurrentTimeNormalized < 0.75f)
        CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeNormalized < 0.85f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
    else
        CurrentTimeOfDay = ELight_TimeOfDay::Night;
}

void ACretaceousLightingSystem::UpdateSunPosition(float TimeNormalized)
{
    if (!SunActor) return;

    // Sun arc: rises at 0.2, peaks at 0.5, sets at 0.8
    float SunAngle = (TimeNormalized - 0.5f) * 180.0f;
    float SunPitch = -FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 80.0f;

    FRotator CurrentRot = SunActor->GetActorRotation();
    SunActor->SetActorRotation(FRotator(SunPitch, CurrentRot.Yaw, 0.0f));

    // Intensity curve: 0 at night, 12 at midday
    float IntensityAlpha = FMath::Clamp((TimeNormalized - 0.2f) / 0.6f, 0.0f, 1.0f);
    float SinAlpha = FMath::Sin(IntensityAlpha * PI);
    float Intensity = SinAlpha * AfternoonConfig.SunIntensity;

    UDirectionalLightComponent* DC = SunActor->GetComponentByClass<UDirectionalLightComponent>();
    if (DC)
    {
        DC->SetIntensity(FMath::Max(Intensity, 0.05f));

        // Color shift: warm at dawn/dusk, white at midday
        float WarmAlpha = 1.0f - FMath::Abs(TimeNormalized - 0.5f) * 4.0f;
        WarmAlpha = FMath::Clamp(WarmAlpha, 0.0f, 1.0f);
        FLinearColor WarmColor = FLinearColor::LerpUsingHSV(DawnConfig.SunColor, AfternoonConfig.SunColor, WarmAlpha);
        DC->SetLightColor(WarmColor);
    }
}

void ACretaceousLightingSystem::UpdateSkyLightIntensity(float TimeNormalized)
{
    if (!SkyLightActor) return;

    USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
    if (!SLC) return;

    float DayAlpha = FMath::Clamp((TimeNormalized - 0.2f) / 0.6f, 0.0f, 1.0f);
    float SkyIntensity = FMath::Sin(DayAlpha * PI) * AfternoonConfig.SkyLightIntensity;
    SLC->SetIntensity(FMath::Max(SkyIntensity, NightConfig.SkyLightIntensity));
}

void ACretaceousLightingSystem::UpdateFogDensity(float TimeNormalized)
{
    // Fog is denser at dawn and dusk
    float FogAlpha = 1.0f - FMath::Abs(TimeNormalized - 0.5f) * 2.0f;
    FogAlpha = FMath::Clamp(FogAlpha, 0.0f, 1.0f);
    float FogDensity = FMath::Lerp(0.08f, AfternoonConfig.FogDensity, FogAlpha);

    // Note: ExponentialHeightFog component update would require a reference
    // This is handled by the UE5 Python script for initial setup
    // Runtime updates can be added by assigning a fog actor reference
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingSystem::SetTimeOfDay(ELight_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;

    switch (NewTime)
    {
    case ELight_TimeOfDay::Dawn:
    case ELight_TimeOfDay::Dusk:
        ApplyLightingConfig(DawnConfig);
        break;
    case ELight_TimeOfDay::Night:
        ApplyLightingConfig(NightConfig);
        break;
    default:
        ApplyLightingConfig(AfternoonConfig);
        break;
    }
}

void ACretaceousLightingSystem::SetWeatherState(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;

    // Adjust fog and sky light based on weather
    float WeatherFogMultiplier = 1.0f;
    float WeatherSkyMultiplier = 1.0f;

    switch (NewWeather)
    {
    case ELight_WeatherState::Overcast:
        WeatherFogMultiplier = 1.5f;
        WeatherSkyMultiplier = 0.6f;
        break;
    case ELight_WeatherState::Rainy:
        WeatherFogMultiplier = 2.0f;
        WeatherSkyMultiplier = 0.4f;
        break;
    case ELight_WeatherState::Stormy:
        WeatherFogMultiplier = 3.0f;
        WeatherSkyMultiplier = 0.2f;
        break;
    case ELight_WeatherState::Foggy:
        WeatherFogMultiplier = 4.0f;
        WeatherSkyMultiplier = 0.5f;
        break;
    default:
        break;
    }

    // Apply multipliers to sky light
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(AfternoonConfig.SkyLightIntensity * WeatherSkyMultiplier);
        }
    }
}

void ACretaceousLightingSystem::ApplyLightingConfig(const FLight_TimeOfDayConfig& Config)
{
    if (SunActor)
    {
        SunActor->SetActorRotation(FRotator(Config.SunPitch, 45.0f, 0.0f));
        UDirectionalLightComponent* DC = SunActor->GetComponentByClass<UDirectionalLightComponent>();
        if (DC)
        {
            DC->SetIntensity(Config.SunIntensity);
            DC->SetLightColor(Config.SunColor);
        }
    }

    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(Config.SkyLightIntensity);
        }
    }
}

float ACretaceousLightingSystem::GetCurrentSunPitch() const
{
    if (SunActor)
    {
        return SunActor->GetActorRotation().Pitch;
    }
    return AfternoonConfig.SunPitch;
}

FLinearColor ACretaceousLightingSystem::GetCurrentSkyColor() const
{
    float DayAlpha = FMath::Clamp((CurrentTimeNormalized - 0.2f) / 0.6f, 0.0f, 1.0f);
    return FLinearColor::LerpUsingHSV(NightConfig.SunColor, AfternoonConfig.SunColor, DayAlpha);
}

// ─────────────────────────────────────────────────────────────────────────────
// Editor Presets (CallInEditor)
// ─────────────────────────────────────────────────────────────────────────────

void ACretaceousLightingSystem::ApplyAfternoonPreset()
{
    ApplyLightingConfig(AfternoonConfig);
    CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
    CurrentTimeNormalized = 0.65f;
}

void ACretaceousLightingSystem::ApplyDawnPreset()
{
    ApplyLightingConfig(DawnConfig);
    CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
    CurrentTimeNormalized = 0.22f;
}

void ACretaceousLightingSystem::ApplyNightPreset()
{
    ApplyLightingConfig(NightConfig);
    CurrentTimeOfDay = ELight_TimeOfDay::Night;
    CurrentTimeNormalized = 0.05f;
}

// ─────────────────────────────────────────────────────────────────────────────
// Private Helpers
// ─────────────────────────────────────────────────────────────────────────────

FLight_TimeOfDayConfig ACretaceousLightingSystem::LerpConfigs(
    const FLight_TimeOfDayConfig& A,
    const FLight_TimeOfDayConfig& B,
    float Alpha) const
{
    FLight_TimeOfDayConfig Result;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.ColorTemperature = FMath::Lerp(A.ColorTemperature, B.ColorTemperature, Alpha);
    return Result;
}
