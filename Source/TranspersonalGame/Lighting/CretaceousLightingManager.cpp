#include "CretaceousLightingManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for performance

    // Default Golden Hour preset (Cretaceous afternoon)
    GoldenHourPreset.SunPitchDegrees = -38.0f;
    GoldenHourPreset.SunYawDegrees = 45.0f;
    GoldenHourPreset.SunIntensity = 12.0f;
    GoldenHourPreset.SunColor = FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);
    GoldenHourPreset.SkyLightIntensity = 2.0f;
    GoldenHourPreset.FogDensity = 0.02f;
    GoldenHourPreset.FogColor = FLinearColor(0.45f, 0.62f, 0.75f, 1.0f);

    // Midday preset — harsh tropical sun
    MiddayPreset.SunPitchDegrees = -75.0f;
    MiddayPreset.SunYawDegrees = 0.0f;
    MiddayPreset.SunIntensity = 18.0f;
    MiddayPreset.SunColor = FLinearColor(1.0f, 0.97f, 0.90f, 1.0f);
    MiddayPreset.SkyLightIntensity = 3.0f;
    MiddayPreset.FogDensity = 0.01f;
    MiddayPreset.FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);

    // Dawn preset — cool blue-pink horizon
    DawnPreset.SunPitchDegrees = -5.0f;
    DawnPreset.SunYawDegrees = -90.0f;
    DawnPreset.SunIntensity = 4.0f;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DawnPreset.SkyLightIntensity = 0.8f;
    DawnPreset.FogDensity = 0.04f;
    DawnPreset.FogColor = FLinearColor(0.55f, 0.65f, 0.8f, 1.0f);

    // Night preset — moonlit jungle
    NightPreset.SunPitchDegrees = 30.0f;
    NightPreset.SunYawDegrees = 180.0f;
    NightPreset.SunIntensity = 0.5f;
    NightPreset.SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    NightPreset.SkyLightIntensity = 0.3f;
    NightPreset.FogDensity = 0.035f;
    NightPreset.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find sun and sky light if not set
    if (!SunLight)
    {
        TArray<AActor*> FoundLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundLights);
        if (FoundLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundLights[0]);
        }
    }

    if (!SkyLightActor)
    {
        TArray<AActor*> FoundSkylights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundSkylights);
        if (FoundSkylights.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundSkylights[0]);
        }
    }

    // Apply default golden hour on start
    ApplyGoldenHourPreset();
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDayCycleActive)
    {
        return;
    }

    ElapsedSeconds += DeltaTime;

    // One full day = 86400 real seconds / DayCycleSpeed
    const float DayDurationSeconds = 86400.0f / FMath::Max(DayCycleSpeed, 0.1f);
    TimeOfDayNormalized = FMath::Fmod(ElapsedSeconds / DayDurationSeconds, 1.0f);

    UpdateSunPosition(TimeOfDayNormalized);
    CurrentTimeOfDay = ClassifyTimeOfDay(TimeOfDayNormalized);
}

void ACretaceousLightingManager::SetTimeOfDay(float NormalizedTime)
{
    TimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    UpdateSunPosition(TimeOfDayNormalized);
    CurrentTimeOfDay = ClassifyTimeOfDay(TimeOfDayNormalized);
}

void ACretaceousLightingManager::SetWeather(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
    UpdateFogForWeather(NewWeather);
}

void ACretaceousLightingManager::ApplyGoldenHourPreset()
{
    ApplyPreset(GoldenHourPreset);
}

void ACretaceousLightingManager::ApplyPreset(const FLight_TimeOfDayPreset& Preset)
{
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Preset.SunPitchDegrees, Preset.SunYawDegrees, 0.0f));
        UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(Preset.SunIntensity);
            SunComp->SetLightColor(Preset.SunColor);
        }
    }

    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Preset.SkyLightIntensity);
        }
    }
}

ELight_TimeOfDay ACretaceousLightingManager::GetCurrentTimeOfDay() const
{
    return CurrentTimeOfDay;
}

float ACretaceousLightingManager::GetSunPitchForTime(float NormalizedTime) const
{
    // 0.0 = midnight (sun below horizon), 0.25 = dawn, 0.5 = noon, 0.75 = dusk
    // Convert to angle: 0.5 (noon) = -90 degrees (straight up), edges = +30 (below horizon)
    const float AngleRadians = (NormalizedTime - 0.5f) * 2.0f * PI;
    return FMath::Sin(AngleRadians) * 90.0f; // -90 at noon, +90 at midnight
}

FLinearColor ACretaceousLightingManager::GetSunColorForTime(float NormalizedTime) const
{
    // Dawn/dusk: warm orange-red, midday: white-yellow, night: deep blue
    if (NormalizedTime < 0.2f || NormalizedTime > 0.8f)
    {
        // Night — deep blue moonlight
        return FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    }
    else if (NormalizedTime < 0.3f || NormalizedTime > 0.7f)
    {
        // Dawn/dusk — warm orange
        return FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    }
    else if (NormalizedTime < 0.4f || NormalizedTime > 0.6f)
    {
        // Morning/afternoon — warm gold
        return FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);
    }
    else
    {
        // Midday — bright white-yellow
        return FLinearColor(1.0f, 0.97f, 0.90f, 1.0f);
    }
}

void ACretaceousLightingManager::UpdateSunPosition(float NormalizedTime)
{
    if (!SunLight)
    {
        return;
    }

    const float SunPitch = GetSunPitchForTime(NormalizedTime);
    const float SunYaw = 45.0f; // Fixed east-to-west arc
    SunLight->SetActorRotation(FRotator(SunPitch, SunYaw, 0.0f));

    UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (SunComp)
    {
        const FLinearColor SunColor = GetSunColorForTime(NormalizedTime);
        SunComp->SetLightColor(SunColor);

        // Intensity varies: 0 at night, 18 at noon, 12 at golden hour
        const float DayFraction = FMath::Clamp((NormalizedTime - 0.2f) / 0.6f, 0.0f, 1.0f);
        const float IntensityCurve = FMath::Sin(DayFraction * PI);
        SunComp->SetIntensity(FMath::Lerp(0.5f, 18.0f, IntensityCurve));
    }

    // Update sky light intensity based on time
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            const float DayFraction = FMath::Clamp((NormalizedTime - 0.2f) / 0.6f, 0.0f, 1.0f);
            const float SkyIntensity = FMath::Lerp(0.3f, 3.0f, FMath::Sin(DayFraction * PI));
            SkyComp->SetIntensity(SkyIntensity);
        }
    }
}

void ACretaceousLightingManager::UpdateFogForWeather(ELight_WeatherState Weather)
{
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FogActors);

    if (FogActors.Num() == 0)
    {
        return;
    }

    AExponentialHeightFog* FogActor = Cast<AExponentialHeightFog>(FogActors[0]);
    if (!FogActor)
    {
        return;
    }

    UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp)
    {
        return;
    }

    switch (Weather)
    {
    case ELight_WeatherState::Clear:
        FogComp->SetFogDensity(0.02f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.45f, 0.62f, 0.75f, 1.0f));
        break;

    case ELight_WeatherState::Overcast:
        FogComp->SetFogDensity(0.04f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.5f, 0.55f, 0.6f, 1.0f));
        break;

    case ELight_WeatherState::Rainy:
        FogComp->SetFogDensity(0.06f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.35f, 0.4f, 0.5f, 1.0f));
        break;

    case ELight_WeatherState::Stormy:
        FogComp->SetFogDensity(0.08f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.2f, 0.25f, 0.35f, 1.0f));
        break;

    case ELight_WeatherState::Foggy:
        FogComp->SetFogDensity(0.12f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.7f, 0.75f, 0.8f, 1.0f));
        break;

    default:
        break;
    }
}

ELight_TimeOfDay ACretaceousLightingManager::ClassifyTimeOfDay(float NormalizedTime) const
{
    if (NormalizedTime < 0.1f || NormalizedTime >= 0.9f)
        return ELight_TimeOfDay::Night;
    if (NormalizedTime < 0.2f)
        return ELight_TimeOfDay::Dawn;
    if (NormalizedTime < 0.35f)
        return ELight_TimeOfDay::Morning;
    if (NormalizedTime < 0.55f)
        return ELight_TimeOfDay::Midday;
    if (NormalizedTime < 0.65f)
        return ELight_TimeOfDay::Afternoon;
    if (NormalizedTime < 0.75f)
        return ELight_TimeOfDay::GoldenHour;
    if (NormalizedTime < 0.85f)
        return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}
