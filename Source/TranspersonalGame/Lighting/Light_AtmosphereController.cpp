#include "Light_AtmosphereController.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphereController::ALight_AtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth cycle
}

void ALight_AtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    ApplyCurrentLighting();
}

void ALight_AtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDayCycleActive) return;

    ElapsedSeconds += DeltaTime;

    // Advance time: DayCycleSpeed=1 means 1 real second = 1 game second (86400s day)
    // DayCycleSpeed=100 means 100x faster
    const float DayLengthSeconds = 86400.0f / DayCycleSpeed;
    TimeOfDayNormalized = FMath::Fmod(ElapsedSeconds / DayLengthSeconds, 1.0f);

    // Apply lighting every tick (throttled by TickInterval = 0.1s)
    ApplyCurrentLighting();
}

void ALight_AtmosphereController::SetTimeOfDay(float NormalizedTime)
{
    TimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    ElapsedSeconds = TimeOfDayNormalized * (86400.0f / DayCycleSpeed);
    ApplyCurrentLighting();
}

ELight_TimeOfDay ALight_AtmosphereController::GetCurrentTimeOfDayEnum() const
{
    if (TimeOfDayNormalized < 0.125f || TimeOfDayNormalized >= 0.917f) return ELight_TimeOfDay::Night;
    if (TimeOfDayNormalized < 0.208f) return ELight_TimeOfDay::Dawn;
    if (TimeOfDayNormalized < 0.375f) return ELight_TimeOfDay::Morning;
    if (TimeOfDayNormalized < 0.583f) return ELight_TimeOfDay::Midday;
    if (TimeOfDayNormalized < 0.75f)  return ELight_TimeOfDay::Afternoon;
    return ELight_TimeOfDay::Dusk;
}

void ALight_AtmosphereController::SetWeatherState(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
    ApplyCurrentLighting();
}

FLight_TimeOfDaySettings ALight_AtmosphereController::GetSettingsForTime(float NormalizedTime) const
{
    FLight_TimeOfDaySettings Settings;

    // Sun angle: 0.0 = midnight (pitch=90 up), 0.5 = noon (pitch=-90 down)
    // Map 0-1 normalized time to sun pitch: sunrise ~0.2, sunset ~0.8
    const float SunAngle = (NormalizedTime - 0.5f) * 360.0f; // -180 to +180
    Settings.SunPitch = FMath::Clamp(-SunAngle, -89.0f, 89.0f);
    Settings.SunYaw = 45.0f;

    // Ensure sun always points down during day (negative pitch)
    if (NormalizedTime > 0.2f && NormalizedTime < 0.8f)
    {
        Settings.SunPitch = FMath::Clamp(Settings.SunPitch, -89.0f, -5.0f);
    }

    // Intensity curve: peak at noon, zero at night
    const float DayFactor = FMath::Clamp(FMath::Sin(NormalizedTime * PI), 0.0f, 1.0f);
    Settings.SunIntensity = FMath::Lerp(0.0f, 10.0f, DayFactor);

    // Color temperature: warm at dawn/dusk, neutral at noon
    ELight_TimeOfDay TOD = GetCurrentTimeOfDayEnum();
    switch (TOD)
    {
        case ELight_TimeOfDay::Dawn:
            Settings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
            Settings.FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
            Settings.FogDensity = 0.04f;
            Settings.SkyLightIntensity = 0.4f;
            break;
        case ELight_TimeOfDay::Morning:
            Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
            Settings.FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
            Settings.FogDensity = 0.025f;
            Settings.SkyLightIntensity = 0.8f;
            break;
        case ELight_TimeOfDay::Midday:
            Settings.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
            Settings.FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
            Settings.FogDensity = 0.015f;
            Settings.SkyLightIntensity = 1.0f;
            break;
        case ELight_TimeOfDay::Afternoon:
            Settings.SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
            Settings.FogColor = FLinearColor(0.7f, 0.6f, 0.5f, 1.0f);
            Settings.FogDensity = 0.02f;
            Settings.SkyLightIntensity = 0.9f;
            break;
        case ELight_TimeOfDay::Dusk:
            Settings.SunColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
            Settings.FogColor = FLinearColor(0.9f, 0.4f, 0.2f, 1.0f);
            Settings.FogDensity = 0.05f;
            Settings.SkyLightIntensity = 0.3f;
            break;
        case ELight_TimeOfDay::Night:
        default:
            Settings.SunColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
            Settings.FogColor = FLinearColor(0.05f, 0.05f, 0.15f, 1.0f);
            Settings.FogDensity = 0.06f;
            Settings.SkyLightIntensity = 0.1f;
            Settings.SunIntensity = 0.0f;
            break;
    }

    // Weather modifiers
    switch (CurrentWeather)
    {
        case ELight_WeatherState::Overcast:
            Settings.SunIntensity *= 0.3f;
            Settings.SkyLightIntensity *= 1.5f;
            Settings.FogDensity *= 2.0f;
            break;
        case ELight_WeatherState::Rain:
            Settings.SunIntensity *= 0.1f;
            Settings.SkyLightIntensity *= 1.2f;
            Settings.FogDensity *= 3.0f;
            Settings.FogColor = FLinearColor(0.3f, 0.35f, 0.4f, 1.0f);
            break;
        case ELight_WeatherState::Fog:
            Settings.FogDensity *= 5.0f;
            Settings.SunIntensity *= 0.5f;
            break;
        default:
            break;
    }

    return Settings;
}

void ALight_AtmosphereController::ApplyCurrentLighting()
{
    FLight_TimeOfDaySettings Settings = GetSettingsForTime(TimeOfDayNormalized);

    UpdateSunPosition(TimeOfDayNormalized);
    UpdateFogSettings(Settings);
    UpdateSkyLight(Settings);
}

void ALight_AtmosphereController::UpdateSunPosition(float NormalizedTime)
{
    if (!SunLight) return;

    FLight_TimeOfDaySettings Settings = GetSettingsForTime(NormalizedTime);

    SunLight->SetActorRotation(FRotator(Settings.SunPitch, Settings.SunYaw, 0.0f));

    UDirectionalLightComponent* DLC = SunLight->FindComponentByClass<UDirectionalLightComponent>();
    if (DLC)
    {
        DLC->SetIntensity(Settings.SunIntensity);
        DLC->SetLightColor(Settings.SunColor);
    }
}

void ALight_AtmosphereController::UpdateFogSettings(const FLight_TimeOfDaySettings& Settings)
{
    if (!FogActor) return;

    UExponentialHeightFogComponent* FogComp = FogActor->FindComponentByClass<UExponentialHeightFogComponent>();
    if (FogComp)
    {
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
    }
}

void ALight_AtmosphereController::UpdateSkyLight(const FLight_TimeOfDaySettings& Settings)
{
    if (!SkyLightActor) return;

    USkyLightComponent* SLC = SkyLightActor->FindComponentByClass<USkyLightComponent>();
    if (SLC)
    {
        SLC->SetIntensity(Settings.SkyLightIntensity);
    }
}
