#include "CretaceousLightingManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec — sufficient for lighting

    InitializeDefaultPresets();
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();
    AutoDiscoverLightActors();
    ApplyTimeOfDayPreset(CurrentTimePhase);
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDayCycleActive)
    {
        UpdateDayCycle(DeltaTime);
    }

    if (bWeatherTransitioning)
    {
        UpdateWeatherTransition(DeltaTime);
    }
}

// ── Day/Night Cycle ──────────────────────────────────────────────────────────

void ACretaceousLightingManager::UpdateDayCycle(float DeltaTime)
{
    // Advance time: DayCycleSpeed=60 means 1 real second = 60 in-game seconds
    // A full day is 86400 seconds, so normalized advancement per real second:
    // DayCycleSpeed / 86400
    const float AdvanceRate = DayCycleSpeed / 86400.0f;
    TimeOfDayNormalized = FMath::Fmod(TimeOfDayNormalized + AdvanceRate * DeltaTime, 1.0f);

    ELight_TimeOfDay NewPhase = ComputeTimePhase(TimeOfDayNormalized);
    if (NewPhase != CurrentTimePhase)
    {
        CurrentTimePhase = NewPhase;
        ApplyTimeOfDayPreset(CurrentTimePhase);
    }

    // Continuously update sun pitch for smooth rotation
    if (SunLight)
    {
        const float SunPitch = GetSunPitchForTime(TimeOfDayNormalized);
        SunLight->SetActorRotation(FRotator(SunPitch, 45.0f, 0.0f));
    }
}

void ACretaceousLightingManager::SetTimeOfDay(float NormalizedTime)
{
    TimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    CurrentTimePhase = ComputeTimePhase(TimeOfDayNormalized);
    ApplyTimeOfDayPreset(CurrentTimePhase);
}

ELight_TimeOfDay ACretaceousLightingManager::ComputeTimePhase(float T) const
{
    // T: 0.0=midnight, 0.25=6am, 0.5=noon, 0.75=6pm, 1.0=midnight
    if (T < 0.04f || T >= 0.96f) return ELight_TimeOfDay::Night;
    if (T < 0.12f) return ELight_TimeOfDay::Dawn;
    if (T < 0.22f) return ELight_TimeOfDay::Morning;
    if (T < 0.42f) return ELight_TimeOfDay::Midday;
    if (T < 0.58f) return ELight_TimeOfDay::GoldenHour;
    if (T < 0.70f) return ELight_TimeOfDay::Dusk;
    if (T < 0.80f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Night;
}

float ACretaceousLightingManager::GetSunPitchForTime(float T) const
{
    // Sinusoidal arc: -90 at midnight, +70 at noon, back to -90 at next midnight
    // T=0.5 is noon (pitch = +70), T=0.0/1.0 is midnight (pitch = -90)
    const float AngleRad = (T - 0.25f) * 2.0f * PI; // shift so T=0.25 is sunrise
    const float SinVal = FMath::Sin(AngleRad);
    return FMath::Lerp(-90.0f, 70.0f, (SinVal + 1.0f) * 0.5f);
}

FLinearColor ACretaceousLightingManager::GetSkyColorForTime(float T) const
{
    // Dawn: orange-pink, Midday: blue-white, Dusk: deep orange, Night: deep blue
    if (T < 0.12f) return FLinearColor(0.05f, 0.02f, 0.08f, 1.0f); // pre-dawn purple
    if (T < 0.22f) return FLinearColor(0.9f, 0.45f, 0.15f, 1.0f);  // dawn orange
    if (T < 0.42f) return FLinearColor(0.7f, 0.85f, 1.0f, 1.0f);   // midday blue
    if (T < 0.58f) return FLinearColor(1.0f, 0.75f, 0.35f, 1.0f);  // golden hour
    if (T < 0.70f) return FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);    // dusk red
    return FLinearColor(0.02f, 0.03f, 0.12f, 1.0f);                 // night blue
}

// ── Preset Application ───────────────────────────────────────────────────────

void ACretaceousLightingManager::ApplyTimeOfDayPreset(ELight_TimeOfDay Phase)
{
    switch (Phase)
    {
        case ELight_TimeOfDay::GoldenHour:  ApplyPhaseSettings(GoldenHourSettings); break;
        case ELight_TimeOfDay::Midday:      ApplyPhaseSettings(MiddaySettings);     break;
        case ELight_TimeOfDay::Dawn:        ApplyPhaseSettings(DawnSettings);       break;
        case ELight_TimeOfDay::Night:       ApplyPhaseSettings(NightSettings);      break;
        case ELight_TimeOfDay::Morning:
        {
            FLight_DayPhaseSettings Morning;
            Morning.SunPitchDegrees = -15.0f;
            Morning.SunIntensity = 5.0f;
            Morning.SunColor = FLinearColor(1.0f, 0.92f, 0.75f, 1.0f);
            Morning.SkyLightIntensity = 1.8f;
            Morning.FogDensity = 0.025f;
            Morning.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
            ApplyPhaseSettings(Morning);
            break;
        }
        case ELight_TimeOfDay::Dusk:
        {
            FLight_DayPhaseSettings Dusk;
            Dusk.SunPitchDegrees = -10.0f;
            Dusk.SunIntensity = 4.0f;
            Dusk.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
            Dusk.SkyLightIntensity = 1.2f;
            Dusk.FogDensity = 0.03f;
            Dusk.FogColor = FLinearColor(0.7f, 0.4f, 0.25f, 1.0f);
            ApplyPhaseSettings(Dusk);
            break;
        }
        default: break;
    }
}

void ACretaceousLightingManager::ApplyPhaseSettings(const FLight_DayPhaseSettings& Settings)
{
    // Apply to DirectionalLight (Sun)
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Settings.SunPitchDegrees, Settings.SunYawDegrees, 0.0f));
        UDirectionalLightComponent* DLC = SunLight->GetComponent();
        if (DLC)
        {
            DLC->SetIntensity(Settings.SunIntensity);
            DLC->SetLightColor(Settings.SunColor);
            DLC->SetIndirectLightingIntensity(Settings.IndirectLightingIntensity);
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetLightComponent();
        if (SLC)
        {
            SLC->SetIntensity(Settings.SkyLightIntensity);
        }
    }

    // Apply to Fog
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
            FogComp->SetFogMaxOpacity(Settings.FogMaxOpacity);
        }
    }
}

// ── Weather ──────────────────────────────────────────────────────────────────

void ACretaceousLightingManager::TransitionToWeather(ELight_WeatherState NewWeather, float TransitionSeconds)
{
    TargetWeather.WeatherState = NewWeather;
    WeatherTransitionDuration = FMath::Max(TransitionSeconds, 1.0f);
    WeatherTransitionElapsed = 0.0f;
    bWeatherTransitioning = true;
}

void ACretaceousLightingManager::UpdateWeatherTransition(float DeltaTime)
{
    WeatherTransitionElapsed += DeltaTime;
    const float Alpha = FMath::Clamp(WeatherTransitionElapsed / WeatherTransitionDuration, 0.0f, 1.0f);

    // Interpolate fog density based on weather
    float TargetFogDensity = 0.018f;
    switch (TargetWeather.WeatherState)
    {
        case ELight_WeatherState::Rain:      TargetFogDensity = 0.04f;  break;
        case ELight_WeatherState::HeavyRain: TargetFogDensity = 0.08f;  break;
        case ELight_WeatherState::Fog:       TargetFogDensity = 0.12f;  break;
        case ELight_WeatherState::Storm:     TargetFogDensity = 0.06f;  break;
        case ELight_WeatherState::Overcast:  TargetFogDensity = 0.03f;  break;
        default: TargetFogDensity = 0.018f; break;
    }

    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        if (FogComp)
        {
            const float CurrentDensity = FogComp->FogDensity;
            FogComp->SetFogDensity(FMath::Lerp(CurrentDensity, TargetFogDensity, Alpha));
        }
    }

    if (Alpha >= 1.0f)
    {
        CurrentWeather = TargetWeather;
        bWeatherTransitioning = false;
    }
}

// ── Auto-Discovery ───────────────────────────────────────────────────────────

void ACretaceousLightingManager::AutoDiscoverLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    // Find SkyLight
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }

    // Find ExponentialHeightFog
    if (!FogActor)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            FogActor = *It;
            break;
        }
    }

    // Find PostProcessVolume
    if (!PostProcessVolume)
    {
        for (TActorIterator<APostProcessVolume> It(World); It; ++It)
        {
            PostProcessVolume = *It;
            break;
        }
    }
}

// ── Default Presets ──────────────────────────────────────────────────────────

void ACretaceousLightingManager::InitializeDefaultPresets()
{
    // Golden Hour — warm amber, low sun, dramatic shadows
    GoldenHourSettings.SunPitchDegrees = -25.0f;
    GoldenHourSettings.SunYawDegrees = 45.0f;
    GoldenHourSettings.SunIntensity = 8.0f;
    GoldenHourSettings.SunColor = FLinearColor(1.0f, 0.88f, 0.65f, 1.0f);
    GoldenHourSettings.SkyLightIntensity = 2.5f;
    GoldenHourSettings.FogDensity = 0.018f;
    GoldenHourSettings.FogColor = FLinearColor(0.55f, 0.72f, 0.85f, 1.0f);
    GoldenHourSettings.FogMaxOpacity = 0.88f;
    GoldenHourSettings.bVolumetricFog = true;
    GoldenHourSettings.IndirectLightingIntensity = 1.8f;

    // Midday — bright white-blue, high sun, strong shadows
    MiddaySettings.SunPitchDegrees = -70.0f;
    MiddaySettings.SunYawDegrees = 0.0f;
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.97f, 0.92f, 1.0f);
    MiddaySettings.SkyLightIntensity = 3.5f;
    MiddaySettings.FogDensity = 0.008f;
    MiddaySettings.FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);
    MiddaySettings.FogMaxOpacity = 0.6f;
    MiddaySettings.bVolumetricFog = true;
    MiddaySettings.IndirectLightingIntensity = 2.2f;

    // Dawn — cool blue-pink, horizon sun, misty
    DawnSettings.SunPitchDegrees = -5.0f;
    DawnSettings.SunYawDegrees = 90.0f;
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnSettings.SkyLightIntensity = 1.0f;
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogColor = FLinearColor(0.6f, 0.65f, 0.8f, 1.0f);
    DawnSettings.FogMaxOpacity = 0.95f;
    DawnSettings.bVolumetricFog = true;
    DawnSettings.IndirectLightingIntensity = 0.8f;

    // Night — dim blue moonlight, dense fog
    NightSettings.SunPitchDegrees = 45.0f; // Sun below horizon
    NightSettings.SunYawDegrees = 180.0f;
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SkyLightIntensity = 0.3f;
    NightSettings.FogDensity = 0.035f;
    NightSettings.FogColor = FLinearColor(0.05f, 0.07f, 0.18f, 1.0f);
    NightSettings.FogMaxOpacity = 0.98f;
    NightSettings.bVolumetricFog = true;
    NightSettings.IndirectLightingIntensity = 0.2f;
}

// ── CallInEditor Debug Functions ─────────────────────────────────────────────

void ACretaceousLightingManager::ApplyGoldenHourNow()
{
    SetTimeOfDay(0.5f); // ~3pm golden hour
    ApplyTimeOfDayPreset(ELight_TimeOfDay::GoldenHour);
}

void ACretaceousLightingManager::ApplyMiddayNow()
{
    SetTimeOfDay(0.375f); // ~9am-noon
    ApplyTimeOfDayPreset(ELight_TimeOfDay::Midday);
}

void ACretaceousLightingManager::ApplyNightNow()
{
    SetTimeOfDay(0.0f);
    ApplyTimeOfDayPreset(ELight_TimeOfDay::Night);
}
