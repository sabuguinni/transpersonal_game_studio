// DayNightCycleManager.cpp
// Agent #08 — Lighting & Atmosphere | PROD_CYCLE_AUTO_20260629_001 | CAP v50
// Full day/night cycle with weather, sky palettes, and Lumen integration

#include "DayNightCycleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default time: 10:00 AM (morning)
    TimeOfDayHours = 10.0f;
    DayDurationSeconds = 1200.0f; // 20 real minutes = 1 game day
    bPauseCycle = false;
    CurrentTimeOfDay = ELight_TimeOfDay::Morning;
    CurrentWeather = ELight_WeatherState::Clear;
    WeatherTransitionSpeed = 0.5f;
    CurrentWeatherBlend = 0.0f;

    SunLight = nullptr;
    HeightFog = nullptr;
    SkyLightActor = nullptr;

    // --- Dawn Palette (5:00-7:00) ---
    DawnPalette.SunColor = FLinearColor(1.0f, 0.55f, 0.25f, 1.0f);
    DawnPalette.SunIntensity = 3.0f;
    DawnPalette.SunPitch = -8.0f;
    DawnPalette.FogColor = FLinearColor(0.85f, 0.65f, 0.55f, 1.0f);
    DawnPalette.FogDensity = 0.06f;
    DawnPalette.SkyLightIntensity = 0.8f;

    // --- Noon Palette (11:00-14:00) ---
    NoonPalette.SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
    NoonPalette.SunIntensity = 12.0f;
    NoonPalette.SunPitch = -80.0f;
    NoonPalette.FogColor = FLinearColor(0.55f, 0.7f, 0.95f, 1.0f);
    NoonPalette.FogDensity = 0.015f;
    NoonPalette.SkyLightIntensity = 2.5f;

    // --- Dusk Palette (17:00-19:00) ---
    DuskPalette.SunColor = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);
    DuskPalette.SunIntensity = 4.0f;
    DuskPalette.SunPitch = -6.0f;
    DuskPalette.FogColor = FLinearColor(0.9f, 0.5f, 0.3f, 1.0f);
    DuskPalette.FogDensity = 0.07f;
    DuskPalette.SkyLightIntensity = 0.6f;

    // --- Night Palette (21:00-4:00) ---
    NightPalette.SunColor = FLinearColor(0.1f, 0.15f, 0.4f, 1.0f);
    NightPalette.SunIntensity = 0.5f;
    NightPalette.SunPitch = -90.0f;
    NightPalette.FogColor = FLinearColor(0.05f, 0.08f, 0.18f, 1.0f);
    NightPalette.FogDensity = 0.04f;
    NightPalette.SkyLightIntensity = 0.2f;

    ActivePalette = NoonPalette;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindSceneActors();
    UpdateSunPosition();
    UpdateSkyColors();
    UpdateFogAtmosphere();
    UpdateSkyLight();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bPauseCycle)
    {
        AdvanceTime(DeltaTime);
    }

    UpdateSunPosition();
    UpdateSkyColors();
    UpdateFogAtmosphere();
    UpdateSkyLight();
    ApplyWeatherEffects();
}

void ADayNightCycleManager::AdvanceTime(float DeltaTime)
{
    // Convert real seconds to game hours
    float HoursPerSecond = 24.0f / DayDurationSeconds;
    TimeOfDayHours += HoursPerSecond * DeltaTime;

    if (TimeOfDayHours >= 24.0f)
    {
        TimeOfDayHours -= 24.0f;
    }

    CurrentTimeOfDay = ClassifyTimeOfDay(TimeOfDayHours);
}

ELight_TimeOfDay ADayNightCycleManager::ClassifyTimeOfDay(float Hours) const
{
    if (Hours >= 5.0f && Hours < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (Hours >= 7.0f && Hours < 11.0f)  return ELight_TimeOfDay::Morning;
    if (Hours >= 11.0f && Hours < 14.0f) return ELight_TimeOfDay::Noon;
    if (Hours >= 14.0f && Hours < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (Hours >= 17.0f && Hours < 19.5f) return ELight_TimeOfDay::Dusk;
    if (Hours >= 19.5f && Hours < 21.0f) return ELight_TimeOfDay::Evening;
    if (Hours >= 21.0f || Hours < 2.0f)  return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

void ADayNightCycleManager::UpdateSunPosition()
{
    if (!SunLight) return;

    // Map 0-24h to sun pitch angle
    // Midnight = -90 (below horizon), Noon = -80 (high in sky)
    float NormalizedTime = TimeOfDayHours / 24.0f;
    float SunAngle = NormalizedTime * 360.0f - 90.0f;

    // Pitch: negative = above horizon, positive = below
    float Pitch = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * -80.0f;
    Pitch = FMath::Clamp(Pitch, -90.0f, -5.0f);

    FRotator CurrentRot = SunLight->GetActorRotation();
    SunLight->SetActorRotation(FRotator(Pitch, CurrentRot.Yaw, CurrentRot.Roll));
}

FLight_SkyPalette ADayNightCycleManager::InterpolatePalettes(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha) const
{
    FLight_SkyPalette Result;
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    return Result;
}

void ADayNightCycleManager::UpdateSkyColors()
{
    if (!SunLight) return;

    // Determine which palettes to blend between
    FLight_SkyPalette PaletteA, PaletteB;
    float BlendAlpha = 0.0f;

    float H = TimeOfDayHours;

    if (H >= 5.0f && H < 11.0f)
    {
        PaletteA = DawnPalette;
        PaletteB = NoonPalette;
        BlendAlpha = (H - 5.0f) / 6.0f;
    }
    else if (H >= 11.0f && H < 17.0f)
    {
        PaletteA = NoonPalette;
        PaletteB = DuskPalette;
        BlendAlpha = (H - 11.0f) / 6.0f;
    }
    else if (H >= 17.0f && H < 21.0f)
    {
        PaletteA = DuskPalette;
        PaletteB = NightPalette;
        BlendAlpha = (H - 17.0f) / 4.0f;
    }
    else
    {
        PaletteA = NightPalette;
        PaletteB = DawnPalette;
        float NightHours = (H >= 21.0f) ? (H - 21.0f) : (H + 3.0f);
        BlendAlpha = NightHours / 8.0f;
    }

    ActivePalette = InterpolatePalettes(PaletteA, PaletteB, FMath::Clamp(BlendAlpha, 0.0f, 1.0f));

    // Apply to sun
    UDirectionalLightComponent* LightComp = SunLight->FindComponentByClass<UDirectionalLightComponent>();
    if (LightComp)
    {
        LightComp->SetLightColor(ActivePalette.SunColor);
        LightComp->SetIntensity(ActivePalette.SunIntensity);
    }
}

void ADayNightCycleManager::UpdateFogAtmosphere()
{
    if (!HeightFog) return;

    UExponentialHeightFogComponent* FogComp = HeightFog->FindComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp) return;

    FogComp->SetFogDensity(ActivePalette.FogDensity);
    FogComp->SetFogInscatteringColor(ActivePalette.FogColor);

    // Weather modifiers
    if (CurrentWeather == ELight_WeatherState::Foggy)
    {
        FogComp->SetFogDensity(ActivePalette.FogDensity * 4.0f);
    }
    else if (CurrentWeather == ELight_WeatherState::Stormy || CurrentWeather == ELight_WeatherState::HeavyRain)
    {
        FogComp->SetFogDensity(ActivePalette.FogDensity * 2.5f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.3f, 0.35f, 0.45f, 1.0f));
    }
    else if (CurrentWeather == ELight_WeatherState::Overcast)
    {
        FogComp->SetFogDensity(ActivePalette.FogDensity * 1.8f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.5f, 0.55f, 0.6f, 1.0f));
    }
}

void ADayNightCycleManager::UpdateSkyLight()
{
    if (!SkyLightActor) return;

    USkyLightComponent* SkyComp = SkyLightActor->FindComponentByClass<USkyLightComponent>();
    if (!SkyComp) return;

    SkyComp->SetIntensity(ActivePalette.SkyLightIntensity);
}

void ADayNightCycleManager::ApplyWeatherEffects()
{
    // Weather-specific sun intensity modifiers
    if (!SunLight) return;

    UDirectionalLightComponent* LightComp = SunLight->FindComponentByClass<UDirectionalLightComponent>();
    if (!LightComp) return;

    float WeatherIntensityMult = 1.0f;
    switch (CurrentWeather)
    {
        case ELight_WeatherState::Overcast:  WeatherIntensityMult = 0.4f; break;
        case ELight_WeatherState::Stormy:    WeatherIntensityMult = 0.2f; break;
        case ELight_WeatherState::HeavyRain: WeatherIntensityMult = 0.25f; break;
        case ELight_WeatherState::Foggy:     WeatherIntensityMult = 0.6f; break;
        case ELight_WeatherState::Haze:      WeatherIntensityMult = 0.75f; break;
        default:                             WeatherIntensityMult = 1.0f; break;
    }

    LightComp->SetIntensity(ActivePalette.SunIntensity * WeatherIntensityMult);
}

void ADayNightCycleManager::SetTimeOfDay(float NewHours)
{
    TimeOfDayHours = FMath::Clamp(NewHours, 0.0f, 23.99f);
    CurrentTimeOfDay = ClassifyTimeOfDay(TimeOfDayHours);
    UpdateSunPosition();
    UpdateSkyColors();
    UpdateFogAtmosphere();
    UpdateSkyLight();
}

void ADayNightCycleManager::SetWeather(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
    ApplyWeatherEffects();
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDay() const
{
    return CurrentTimeOfDay;
}

float ADayNightCycleManager::GetNormalizedTimeOfDay() const
{
    return TimeOfDayHours / 24.0f;
}

bool ADayNightCycleManager::IsNightTime() const
{
    return (TimeOfDayHours >= 21.0f || TimeOfDayHours < 5.0f);
}

bool ADayNightCycleManager::IsDaytime() const
{
    return (TimeOfDayHours >= 7.0f && TimeOfDayHours < 19.0f);
}

FLinearColor ADayNightCycleManager::GetCurrentSunColor() const
{
    return ActivePalette.SunColor;
}

float ADayNightCycleManager::GetCurrentSunIntensity() const
{
    return ActivePalette.SunIntensity;
}

void ADayNightCycleManager::AutoFindSceneActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Auto-find DirectionalLight
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    // Auto-find ExponentialHeightFog
    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }

    // Auto-find SkyLight
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }
}
