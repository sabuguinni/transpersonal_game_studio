#include "CretaceousLightingManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;  // Update 10x/sec for smooth cycle

    // Golden Hour preset (default — warm Cretaceous afternoon)
    GoldenHourPreset.SunPitchDegrees = -28.0f;
    GoldenHourPreset.SunYawDegrees = 45.0f;
    GoldenHourPreset.SunIntensity = 8.5f;
    GoldenHourPreset.SunColor = FLinearColor(1.0f, 0.82f, 0.55f, 1.0f);
    GoldenHourPreset.SkyLightIntensity = 2.2f;
    GoldenHourPreset.FogDensity = 0.025f;
    GoldenHourPreset.FogColor = FLinearColor(0.72f, 0.58f, 0.42f, 1.0f);
    GoldenHourPreset.bVolumetricFog = true;

    // Midday preset (harsh tropical sun)
    MiddayPreset.SunPitchDegrees = -75.0f;
    MiddayPreset.SunYawDegrees = 0.0f;
    MiddayPreset.SunIntensity = 12.0f;
    MiddayPreset.SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
    MiddayPreset.SkyLightIntensity = 3.5f;
    MiddayPreset.FogDensity = 0.012f;
    MiddayPreset.FogColor = FLinearColor(0.65f, 0.75f, 0.85f, 1.0f);
    MiddayPreset.bVolumetricFog = false;

    // Night preset (moonlit Cretaceous)
    NightPreset.SunPitchDegrees = 15.0f;  // Below horizon
    NightPreset.SunYawDegrees = 180.0f;
    NightPreset.SunIntensity = 0.3f;
    NightPreset.SunColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    NightPreset.SkyLightIntensity = 0.4f;
    NightPreset.FogDensity = 0.04f;
    NightPreset.FogColor = FLinearColor(0.1f, 0.12f, 0.22f, 1.0f);
    NightPreset.bVolumetricFog = true;

    // Dawn preset (misty Cretaceous sunrise)
    DawnPreset.SunPitchDegrees = -10.0f;
    DawnPreset.SunYawDegrees = -90.0f;
    DawnPreset.SunIntensity = 4.0f;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.65f, 0.35f, 1.0f);
    DawnPreset.SkyLightIntensity = 1.2f;
    DawnPreset.FogDensity = 0.045f;
    DawnPreset.FogColor = FLinearColor(0.85f, 0.65f, 0.45f, 1.0f);
    DawnPreset.bVolumetricFog = true;

    CurrentPreset = GoldenHourPreset;
    TargetPreset = GoldenHourPreset;
}

// ============================================================
// BEGIN PLAY
// ============================================================

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightActors();
    SetTimeOfDay(CurrentHourOfDay);
}

// ============================================================
// TICK
// ============================================================

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        TickDayNightCycle(DeltaTime);
    }

    // Blend towards target preset
    if (PresetBlendAlpha < 1.0f)
    {
        PresetBlendAlpha = FMath::Min(1.0f, PresetBlendAlpha + DeltaTime * WeatherTransitionSpeed);
        FLight_DayPreset Blended = LerpPresets(CurrentPreset, TargetPreset, PresetBlendAlpha);
        ApplyLightingPreset(Blended);
    }
}

// ============================================================
// DAY/NIGHT CYCLE TICK
// ============================================================

void ACretaceousLightingManager::TickDayNightCycle(float DeltaTime)
{
    // Advance time: DayDurationMinutes real minutes = 24 in-game hours
    float HoursPerSecond = 24.0f / (DayDurationMinutes * 60.0f);
    CurrentHourOfDay += HoursPerSecond * DeltaTime;

    if (CurrentHourOfDay >= 24.0f)
    {
        CurrentHourOfDay -= 24.0f;
    }

    CurrentTimeOfDay = GetTimeOfDayEnum(CurrentHourOfDay);
    UpdateSunTransform(CurrentHourOfDay);
}

// ============================================================
// SET TIME OF DAY
// ============================================================

void ACretaceousLightingManager::SetTimeOfDay(float HourOfDay)
{
    CurrentHourOfDay = FMath::Clamp(HourOfDay, 0.0f, 24.0f);
    CurrentTimeOfDay = GetTimeOfDayEnum(CurrentHourOfDay);

    FLight_DayPreset NewPreset = GetPresetForHour(CurrentHourOfDay);
    CurrentPreset = NewPreset;
    TargetPreset = NewPreset;
    PresetBlendAlpha = 1.0f;

    ApplyLightingPreset(NewPreset);
    UpdateSunTransform(CurrentHourOfDay);
}

// ============================================================
// SET WEATHER STATE
// ============================================================

void ACretaceousLightingManager::SetWeatherState(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
    UpdateFogForWeather(NewWeather);
}

// ============================================================
// APPLY LIGHTING PRESET
// ============================================================

void ACretaceousLightingManager::ApplyLightingPreset(const FLight_DayPreset& Preset)
{
    // Apply to DirectionalLight (Sun)
    if (SunActor)
    {
        SunActor->SetActorRotation(FRotator(Preset.SunPitchDegrees, Preset.SunYawDegrees, 0.0f));

        UDirectionalLightComponent* DLComp = SunActor->GetComponent();
        if (DLComp)
        {
            DLComp->SetIntensity(Preset.SunIntensity);
            DLComp->SetLightColor(Preset.SunColor);
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SLComp = SkyLightActor->GetLightComponent();
        if (SLComp)
        {
            SLComp->SetIntensity(Preset.SkyLightIntensity);
        }
    }

    // Apply to Fog
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Preset.FogDensity);
            FogComp->SetFogInscatteringColor(Preset.FogColor);
            FogComp->SetVolumetricFog(Preset.bVolumetricFog);
        }
    }
}

// ============================================================
// GET TIME OF DAY ENUM
// ============================================================

ELight_TimeOfDay ACretaceousLightingManager::GetTimeOfDayEnum(float Hour) const
{
    if (Hour >= 5.0f && Hour < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (Hour >= 7.0f && Hour < 10.0f)  return ELight_TimeOfDay::Morning;
    if (Hour >= 10.0f && Hour < 14.0f) return ELight_TimeOfDay::Midday;
    if (Hour >= 14.0f && Hour < 16.0f) return ELight_TimeOfDay::Afternoon;
    if (Hour >= 16.0f && Hour < 18.5f) return ELight_TimeOfDay::GoldenHour;
    if (Hour >= 18.5f && Hour < 20.0f) return ELight_TimeOfDay::Dusk;
    if (Hour >= 20.0f || Hour < 2.0f)  return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

// ============================================================
// GET SUN PITCH FOR HOUR
// ============================================================

float ACretaceousLightingManager::GetSunPitchForHour(float Hour) const
{
    // Map 0-24h to sun arc: rises at 6am (-10°), peaks at noon (-80°), sets at 6pm (-10°), below horizon at night (+20°)
    if (Hour < 6.0f || Hour > 18.0f)
    {
        return 20.0f;  // Below horizon
    }

    // Normalized 0-1 across daylight hours (6am to 6pm)
    float DayProgress = (Hour - 6.0f) / 12.0f;
    // Sine arc: 0 at sunrise, 1 at noon, 0 at sunset
    float SinArc = FMath::Sin(DayProgress * PI);
    // Map to pitch: -10° at horizon, -80° at zenith
    float Pitch = FMath::Lerp(-10.0f, -80.0f, SinArc);
    return Pitch;
}

// ============================================================
// GET SUN COLOR FOR HOUR
// ============================================================

FLinearColor ACretaceousLightingManager::GetSunColorForHour(float Hour) const
{
    // Dawn/Dusk: warm orange-red
    if ((Hour >= 5.5f && Hour < 7.5f) || (Hour >= 17.0f && Hour < 19.0f))
    {
        return FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    }
    // Golden hour: warm amber
    if ((Hour >= 7.5f && Hour < 9.0f) || (Hour >= 16.0f && Hour < 17.0f))
    {
        return FLinearColor(1.0f, 0.82f, 0.55f, 1.0f);
    }
    // Midday: bright white-yellow
    if (Hour >= 10.0f && Hour < 15.0f)
    {
        return FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
    }
    // Night: cool blue moonlight
    return FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
}

// ============================================================
// AUTO FIND LIGHT ACTORS
// ============================================================

void ACretaceousLightingManager::AutoFindLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight
    if (!SunActor)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunActor = *It;
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
    if (!PostProcessActor)
    {
        for (TActorIterator<APostProcessVolume> It(World); It; ++It)
        {
            PostProcessActor = *It;
            break;
        }
    }
}

// ============================================================
// CALL-IN-EDITOR HELPERS
// ============================================================

void ACretaceousLightingManager::ApplyGoldenHourNow()
{
    SetTimeOfDay(16.5f);
}

void ACretaceousLightingManager::ApplyMiddayNow()
{
    SetTimeOfDay(12.0f);
}

void ACretaceousLightingManager::ApplyNightNow()
{
    SetTimeOfDay(22.0f);
}

// ============================================================
// PRIVATE HELPERS
// ============================================================

void ACretaceousLightingManager::UpdateSunTransform(float Hour)
{
    if (!SunActor) return;

    float Pitch = GetSunPitchForHour(Hour);
    float Yaw = FMath::Lerp(-90.0f, 90.0f, (Hour - 6.0f) / 12.0f);
    SunActor->SetActorRotation(FRotator(Pitch, Yaw, 0.0f));

    UDirectionalLightComponent* DLComp = SunActor->GetComponent();
    if (DLComp)
    {
        DLComp->SetLightColor(GetSunColorForHour(Hour));
    }
}

void ACretaceousLightingManager::UpdateFogForWeather(ELight_WeatherState Weather)
{
    if (!FogActor) return;

    UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
    if (!FogComp) return;

    switch (Weather)
    {
        case ELight_WeatherState::Clear:
            FogComp->SetFogDensity(0.015f);
            break;
        case ELight_WeatherState::PartlyCloudy:
            FogComp->SetFogDensity(0.022f);
            break;
        case ELight_WeatherState::Overcast:
            FogComp->SetFogDensity(0.035f);
            FogComp->SetFogInscatteringColor(FLinearColor(0.5f, 0.55f, 0.6f, 1.0f));
            break;
        case ELight_WeatherState::Stormy:
            FogComp->SetFogDensity(0.06f);
            FogComp->SetFogInscatteringColor(FLinearColor(0.3f, 0.32f, 0.38f, 1.0f));
            break;
        case ELight_WeatherState::Rain:
            FogComp->SetFogDensity(0.05f);
            FogComp->SetFogInscatteringColor(FLinearColor(0.4f, 0.45f, 0.52f, 1.0f));
            break;
        case ELight_WeatherState::HeavyRain:
            FogComp->SetFogDensity(0.08f);
            FogComp->SetFogInscatteringColor(FLinearColor(0.25f, 0.28f, 0.35f, 1.0f));
            break;
        default:
            break;
    }
}

FLight_DayPreset ACretaceousLightingManager::GetPresetForHour(float Hour) const
{
    ELight_TimeOfDay TOD = GetTimeOfDayEnum(Hour);

    switch (TOD)
    {
        case ELight_TimeOfDay::Dawn:
        case ELight_TimeOfDay::Morning:
            return DawnPreset;
        case ELight_TimeOfDay::Midday:
        case ELight_TimeOfDay::Afternoon:
            return MiddayPreset;
        case ELight_TimeOfDay::GoldenHour:
        case ELight_TimeOfDay::Dusk:
            return GoldenHourPreset;
        case ELight_TimeOfDay::Night:
        case ELight_TimeOfDay::Midnight:
        default:
            return NightPreset;
    }
}

FLight_DayPreset ACretaceousLightingManager::LerpPresets(const FLight_DayPreset& A, const FLight_DayPreset& B, float Alpha) const
{
    FLight_DayPreset Result;
    Result.SunPitchDegrees = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
    Result.SunYawDegrees = FMath::Lerp(A.SunYawDegrees, B.SunYawDegrees, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.bVolumetricFog = Alpha < 0.5f ? A.bVolumetricFog : B.bVolumetricFog;
    return Result;
}
