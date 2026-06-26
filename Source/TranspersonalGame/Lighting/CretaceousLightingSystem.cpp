#include "CretaceousLightingSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ACretaceousLightingSystem::ACretaceousLightingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f;  // 20fps tick — sufficient for smooth lighting
    InitializeDefaultPresets();
}

void ACretaceousLightingSystem::BeginPlay()
{
    Super::BeginPlay();
    DiscoverLightingActors();
    // Apply current hour preset on start
    FLight_TimeOfDayPreset Preset = GetPresetForHour(CurrentHour);
    ApplyPreset(Preset);
}

void ACretaceousLightingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        TickDayCycle(DeltaTime);
    }
}

void ACretaceousLightingSystem::DiscoverLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        SunActor = *It;
        break;
    }

    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        SkyLightActor = *It;
        break;
    }
}

void ACretaceousLightingSystem::ApplyPreset(const FLight_TimeOfDayPreset& Preset)
{
    // Configure sun
    if (SunActor)
    {
        SunActor->SetActorRotation(FRotator(Preset.SunPitchDegrees, Preset.SunYawDegrees, 0.0f));
        CurrentSunPitch = Preset.SunPitchDegrees;

        UDirectionalLightComponent* SunComp = SunActor->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(Preset.SunIntensity);
            SunComp->SetLightColor(Preset.SunColor.ToFColor(true));
        }
    }

    // Configure skylight
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->Intensity = Preset.SkyLightIntensity;
            SkyComp->RecaptureSky();
        }
    }
}

void ACretaceousLightingSystem::TickDayCycle(float DeltaTime)
{
    // Advance time: DayCycleSpeed=100 means 100 real seconds = 1 game hour
    const float HoursPerSecond = DayCycleSpeed / 3600.0f;
    CurrentHour += DeltaTime * HoursPerSecond;
    if (CurrentHour >= 24.0f) CurrentHour -= 24.0f;

    FLight_TimeOfDayPreset Preset = GetPresetForHour(CurrentHour);
    ApplyPreset(Preset);
}

FLight_TimeOfDayPreset ACretaceousLightingSystem::GetPresetForHour(float Hour) const
{
    if (TimeOfDayPresets.Num() == 0)
    {
        FLight_TimeOfDayPreset Default;
        return Default;
    }

    // Find the two surrounding presets and interpolate
    // Map hour 0-24 to preset index
    float NormalizedHour = Hour / 24.0f;
    float FloatIndex = NormalizedHour * TimeOfDayPresets.Num();
    int32 IndexA = FMath::FloorToInt(FloatIndex) % TimeOfDayPresets.Num();
    int32 IndexB = (IndexA + 1) % TimeOfDayPresets.Num();
    float Alpha = FMath::Frac(FloatIndex);

    return InterpolatePresets(TimeOfDayPresets[IndexA], TimeOfDayPresets[IndexB], Alpha);
}

FLight_TimeOfDayPreset ACretaceousLightingSystem::InterpolatePresets(
    const FLight_TimeOfDayPreset& A, const FLight_TimeOfDayPreset& B, float Alpha) const
{
    FLight_TimeOfDayPreset Result;
    Result.SunPitchDegrees = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
    Result.SunYawDegrees = FMath::Lerp(A.SunYawDegrees, B.SunYawDegrees, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.RayleighScatteringScale = FMath::Lerp(A.RayleighScatteringScale, B.RayleighScatteringScale, Alpha);
    Result.MieScatteringScale = FMath::Lerp(A.MieScatteringScale, B.MieScatteringScale, Alpha);
    return Result;
}

void ACretaceousLightingSystem::InitializeDefaultPresets()
{
    TimeOfDayPresets.Empty();

    // Dawn (5:00 AM)
    FLight_TimeOfDayPreset Dawn;
    Dawn.TimeOfDay = ELight_TimeOfDay::Dawn;
    Dawn.SunPitchDegrees = -5.0f;
    Dawn.SunYawDegrees = -90.0f;
    Dawn.SunIntensity = 3.0f;
    Dawn.SunColor = FLinearColor(1.0f, 0.55f, 0.25f, 1.0f);  // Deep orange
    Dawn.SkyLightIntensity = 0.8f;
    Dawn.FogDensity = 0.08f;
    Dawn.FogColor = FLinearColor(0.6f, 0.45f, 0.35f, 1.0f);
    Dawn.RayleighScatteringScale = 0.04f;
    Dawn.MieScatteringScale = 0.008f;
    TimeOfDayPresets.Add(Dawn);

    // Morning (9:00 AM)
    FLight_TimeOfDayPreset Morning;
    Morning.TimeOfDay = ELight_TimeOfDay::Morning;
    Morning.SunPitchDegrees = -30.0f;
    Morning.SunYawDegrees = -60.0f;
    Morning.SunIntensity = 8.0f;
    Morning.SunColor = FLinearColor(1.0f, 0.92f, 0.75f, 1.0f);  // Warm white
    Morning.SkyLightIntensity = 1.5f;
    Morning.FogDensity = 0.04f;
    Morning.FogColor = FLinearColor(0.8f, 0.78f, 0.7f, 1.0f);
    Morning.RayleighScatteringScale = 0.0331f;
    Morning.MieScatteringScale = 0.004f;
    TimeOfDayPresets.Add(Morning);

    // Midday (12:00 PM)
    FLight_TimeOfDayPreset Midday;
    Midday.TimeOfDay = ELight_TimeOfDay::Midday;
    Midday.SunPitchDegrees = -75.0f;
    Midday.SunYawDegrees = 0.0f;
    Midday.SunIntensity = 15.0f;
    Midday.SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);  // Near white
    Midday.SkyLightIntensity = 2.5f;
    Midday.FogDensity = 0.02f;
    Midday.FogColor = FLinearColor(0.75f, 0.82f, 0.9f, 1.0f);  // Blue-ish haze
    Midday.RayleighScatteringScale = 0.033f;
    Midday.MieScatteringScale = 0.002f;
    TimeOfDayPresets.Add(Midday);

    // Golden Hour (17:00 PM) — cinematic hero shot
    FLight_TimeOfDayPreset GoldenHour;
    GoldenHour.TimeOfDay = ELight_TimeOfDay::GoldenHour;
    GoldenHour.SunPitchDegrees = -38.0f;
    GoldenHour.SunYawDegrees = 45.0f;
    GoldenHour.SunIntensity = 12.0f;
    GoldenHour.SunColor = FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);  // Warm gold #FFE09E
    GoldenHour.SkyLightIntensity = 2.0f;
    GoldenHour.FogDensity = 0.035f;
    GoldenHour.FogColor = FLinearColor(0.85f, 0.72f, 0.55f, 1.0f);
    GoldenHour.RayleighScatteringScale = 0.0331f;
    GoldenHour.MieScatteringScale = 0.004f;
    TimeOfDayPresets.Add(GoldenHour);

    // Dusk (19:30 PM)
    FLight_TimeOfDayPreset Dusk;
    Dusk.TimeOfDay = ELight_TimeOfDay::Dusk;
    Dusk.SunPitchDegrees = -8.0f;
    Dusk.SunYawDegrees = 90.0f;
    Dusk.SunIntensity = 4.0f;
    Dusk.SunColor = FLinearColor(1.0f, 0.35f, 0.1f, 1.0f);  // Deep red-orange
    Dusk.SkyLightIntensity = 0.6f;
    Dusk.FogDensity = 0.065f;
    Dusk.FogColor = FLinearColor(0.7f, 0.3f, 0.15f, 1.0f);
    Dusk.RayleighScatteringScale = 0.045f;
    Dusk.MieScatteringScale = 0.01f;
    TimeOfDayPresets.Add(Dusk);

    // Night (22:00 PM)
    FLight_TimeOfDayPreset Night;
    Night.TimeOfDay = ELight_TimeOfDay::Night;
    Night.SunPitchDegrees = 20.0f;  // Below horizon
    Night.SunYawDegrees = 180.0f;
    Night.SunIntensity = 0.1f;
    Night.SunColor = FLinearColor(0.3f, 0.35f, 0.6f, 1.0f);  // Cool blue moonlight
    Night.SkyLightIntensity = 0.3f;
    Night.FogDensity = 0.05f;
    Night.FogColor = FLinearColor(0.1f, 0.12f, 0.2f, 1.0f);
    Night.RayleighScatteringScale = 0.025f;
    Night.MieScatteringScale = 0.001f;
    TimeOfDayPresets.Add(Night);
}

void ACretaceousLightingSystem::ApplyGoldenHourPreset()
{
    CurrentHour = 17.0f;
    FLight_TimeOfDayPreset Preset;
    Preset.TimeOfDay = ELight_TimeOfDay::GoldenHour;
    Preset.SunPitchDegrees = -38.0f;
    Preset.SunYawDegrees = 45.0f;
    Preset.SunIntensity = 12.0f;
    Preset.SunColor = FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);
    Preset.SkyLightIntensity = 2.0f;
    Preset.FogDensity = 0.035f;
    Preset.FogColor = FLinearColor(0.85f, 0.72f, 0.55f, 1.0f);
    Preset.RayleighScatteringScale = 0.0331f;
    Preset.MieScatteringScale = 0.004f;
    DiscoverLightingActors();
    ApplyPreset(Preset);
}

void ACretaceousLightingSystem::ApplyDawnPreset()
{
    CurrentHour = 5.0f;
    if (TimeOfDayPresets.Num() > 0)
    {
        DiscoverLightingActors();
        ApplyPreset(TimeOfDayPresets[0]);
    }
}

void ACretaceousLightingSystem::ApplyMiddayPreset()
{
    CurrentHour = 12.0f;
    if (TimeOfDayPresets.Num() > 2)
    {
        DiscoverLightingActors();
        ApplyPreset(TimeOfDayPresets[2]);
    }
}

void ACretaceousLightingSystem::ApplyNightPreset()
{
    CurrentHour = 22.0f;
    if (TimeOfDayPresets.Num() > 5)
    {
        DiscoverLightingActors();
        ApplyPreset(TimeOfDayPresets[5]);
    }
}

void ACretaceousLightingSystem::SetWeatherState(ELight_WeatherState NewWeather, float TransitionDuration)
{
    TargetWeatherState = NewWeather;
    WeatherTransitionDuration = TransitionDuration;
    WeatherTransitionTimer = 0.0f;
    WeatherState = NewWeather;
}

ELight_TimeOfDay ACretaceousLightingSystem::GetCurrentTimeOfDayEnum() const
{
    if (CurrentHour < 6.0f) return ELight_TimeOfDay::Dawn;
    if (CurrentHour < 10.0f) return ELight_TimeOfDay::Morning;
    if (CurrentHour < 14.0f) return ELight_TimeOfDay::Midday;
    if (CurrentHour < 16.5f) return ELight_TimeOfDay::Afternoon;
    if (CurrentHour < 18.5f) return ELight_TimeOfDay::GoldenHour;
    if (CurrentHour < 20.5f) return ELight_TimeOfDay::Dusk;
    if (CurrentHour < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

#if WITH_EDITOR
void ACretaceousLightingSystem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Live-update lighting when properties change in editor
    DiscoverLightingActors();
    FLight_TimeOfDayPreset Preset = GetPresetForHour(CurrentHour);
    ApplyPreset(Preset);
}
#endif
