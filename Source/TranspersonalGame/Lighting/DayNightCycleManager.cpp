#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// ADayNightCycleManager Implementation
// Agent #08 — Lighting & Atmosphere
// Cretaceous prehistoric survival game
// ============================================================

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20fps tick for lighting updates

    // --- Cretaceous Dawn Preset ---
    // Pre-sunrise: deep orange-red horizon, cool blue upper sky
    DawnPreset.SunColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
    DawnPreset.SkyColor = FLinearColor(0.15f, 0.2f, 0.5f, 1.0f);
    DawnPreset.FogColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnPreset.SunIntensity = 2.0f;
    DawnPreset.SkyLightIntensity = 0.3f;
    DawnPreset.FogDensity = 0.05f;

    // --- Morning Preset ---
    // Golden hour: warm amber, long shadows, mist rising from jungle floor
    MorningPreset.SunColor = FLinearColor(1.0f, 0.78f, 0.45f, 1.0f);
    MorningPreset.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    MorningPreset.FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
    MorningPreset.SunIntensity = 6.0f;
    MorningPreset.SkyLightIntensity = 0.8f;
    MorningPreset.FogDensity = 0.03f;

    // --- Midday Preset ---
    // High sun: harsh white-yellow light, deep shadows, high contrast
    MiddayPreset.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    MiddayPreset.SkyColor = FLinearColor(0.3f, 0.55f, 1.0f, 1.0f);
    MiddayPreset.FogColor = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);
    MiddayPreset.SunIntensity = 10.0f;
    MiddayPreset.SkyLightIntensity = 1.2f;
    MiddayPreset.FogDensity = 0.01f;

    // --- Dusk Preset ---
    // Sunset: deep amber-red, dramatic silhouettes, danger time
    DuskPreset.SunColor = FLinearColor(1.0f, 0.35f, 0.05f, 1.0f);
    DuskPreset.SkyColor = FLinearColor(0.2f, 0.15f, 0.4f, 1.0f);
    DuskPreset.FogColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    DuskPreset.SunIntensity = 3.0f;
    DuskPreset.SkyLightIntensity = 0.4f;
    DuskPreset.FogDensity = 0.04f;

    // --- Night Preset ---
    // Full night: cool blue moonlight, deep shadows, predator territory
    NightPreset.SunColor = FLinearColor(0.1f, 0.15f, 0.4f, 1.0f);
    NightPreset.SkyColor = FLinearColor(0.02f, 0.03f, 0.1f, 1.0f);
    NightPreset.FogColor = FLinearColor(0.05f, 0.08f, 0.2f, 1.0f);
    NightPreset.SunIntensity = 0.1f;
    NightPreset.SkyLightIntensity = 0.05f;
    NightPreset.FogDensity = 0.06f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightActors();
    UpdateSunPosition();
    UpdateSkyColors();
    UpdateFog();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bPauseDayNightCycle)
    {
        AdvanceTime(DeltaTime);
    }

    UpdateSunPosition();
    UpdateSkyColors();
    UpdateFog();
    UpdateWeatherTransition(DeltaTime);
}

void ADayNightCycleManager::AutoFindLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight (sun)
    if (!SunLight)
    {
        TArray<AActor*> FoundLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
        if (FoundLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundLights[0]);
        }
    }

    // Find SkyLight
    if (!SkyLightActor)
    {
        TArray<AActor*> FoundSkyLights;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundSkyLights);
        if (FoundSkyLights.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundSkyLights[0]);
        }
    }

    // Find ExponentialHeightFog
    if (!HeightFogActor)
    {
        TArray<AActor*> FoundFog;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFog);
        if (FoundFog.Num() > 0)
        {
            HeightFogActor = Cast<AExponentialHeightFog>(FoundFog[0]);
        }
    }
}

void ADayNightCycleManager::AdvanceTime(float DeltaTime)
{
    // DayDurationSeconds = real seconds for one full 24h cycle
    float HoursPerSecond = 24.0f / DayDurationSeconds;
    CurrentTimeOfDay += HoursPerSecond * DeltaTime * TimeScale;

    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    CurrentTimeEnum = HourToTimeEnum(CurrentTimeOfDay);
}

void ADayNightCycleManager::UpdateSunPosition()
{
    if (!SunLight) return;

    float SunPitch = HourToSunPitch(CurrentTimeOfDay);
    CurrentSunPitch = SunPitch;

    // Yaw rotates sun east-to-west across the sky
    float SunYaw = FMath::Lerp(-90.0f, 90.0f, CurrentTimeOfDay / 24.0f);

    FRotator NewRotation(SunPitch, SunYaw, 0.0f);
    SunLight->SetActorRotation(NewRotation);
}

float ADayNightCycleManager::HourToSunPitch(float Hour) const
{
    // Sun rises at 6am, sets at 18pm
    // Pitch: -90 = directly overhead, 0 = horizon, positive = below horizon
    // We use negative pitch for above-horizon sun
    if (Hour < 6.0f || Hour > 18.0f)
    {
        // Night: sun below horizon
        return 10.0f;
    }

    // Map 6-18 hours to pitch -10 (horizon) to -90 (zenith) back to -10
    float DayProgress = (Hour - 6.0f) / 12.0f; // 0 to 1
    float SinValue = FMath::Sin(DayProgress * PI);
    float Pitch = FMath::Lerp(-10.0f, -80.0f, SinValue);
    return Pitch;
}

void ADayNightCycleManager::UpdateSkyColors()
{
    if (!SunLight) return;

    FLight_SkyColorPreset CurrentPreset = GetPresetForHour(CurrentTimeOfDay);

    // Apply weather multipliers
    float WeatherIntensityMult = FMath::Lerp(1.0f, 0.4f, CurrentWeather.CloudCoverage);

    // Set sun color and intensity
    UDirectionalLightComponent* DirLightComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (DirLightComp)
    {
        DirLightComp->SetIntensity(CurrentPreset.SunIntensity * WeatherIntensityMult);
        DirLightComp->SetLightColor(FLinearColor(
            CurrentPreset.SunColor.R,
            CurrentPreset.SunColor.G,
            CurrentPreset.SunColor.B
        ));
    }

    // Set sky light intensity
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(CurrentPreset.SkyLightIntensity * WeatherIntensityMult);
        }
    }
}

void ADayNightCycleManager::UpdateFog()
{
    if (!HeightFogActor) return;

    UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp) return;

    FLight_SkyColorPreset CurrentPreset = GetPresetForHour(CurrentTimeOfDay);
    float WeatherFogMult = CurrentWeather.FogDensityMultiplier;

    FogComp->SetFogDensity(CurrentPreset.FogDensity * WeatherFogMult);
    FogComp->SetFogInscatteringColor(FLinearColor(
        CurrentPreset.FogColor.R,
        CurrentPreset.FogColor.G,
        CurrentPreset.FogColor.B
    ));
}

void ADayNightCycleManager::UpdateWeatherTransition(float DeltaTime)
{
    if (WeatherTransitionAlpha >= 1.0f) return;

    float TransitionSpeed = 1.0f / FMath::Max(TargetWeather.TransitionDuration, 1.0f);
    WeatherTransitionAlpha = FMath::Clamp(WeatherTransitionAlpha + TransitionSpeed * DeltaTime, 0.0f, 1.0f);

    // Lerp weather config
    CurrentWeather.CloudCoverage = FMath::Lerp(CurrentWeather.CloudCoverage, TargetWeather.CloudCoverage, WeatherTransitionAlpha);
    CurrentWeather.RainIntensity = FMath::Lerp(CurrentWeather.RainIntensity, TargetWeather.RainIntensity, WeatherTransitionAlpha);
    CurrentWeather.FogDensityMultiplier = FMath::Lerp(CurrentWeather.FogDensityMultiplier, TargetWeather.FogDensityMultiplier, WeatherTransitionAlpha);

    if (WeatherTransitionAlpha >= 1.0f)
    {
        CurrentWeather = TargetWeather;
        CurrentWeather.WeatherState = TargetWeather.WeatherState;
    }
}

FLight_SkyColorPreset ADayNightCycleManager::GetPresetForHour(float Hour) const
{
    // Blend between presets based on hour
    if (Hour < 5.0f || Hour >= 22.0f)
    {
        return NightPreset;
    }
    else if (Hour < 7.0f)
    {
        float Alpha = (Hour - 5.0f) / 2.0f;
        return InterpolatePresets(NightPreset, DawnPreset, Alpha);
    }
    else if (Hour < 9.0f)
    {
        float Alpha = (Hour - 7.0f) / 2.0f;
        return InterpolatePresets(DawnPreset, MorningPreset, Alpha);
    }
    else if (Hour < 12.0f)
    {
        float Alpha = (Hour - 9.0f) / 3.0f;
        return InterpolatePresets(MorningPreset, MiddayPreset, Alpha);
    }
    else if (Hour < 16.0f)
    {
        return MiddayPreset;
    }
    else if (Hour < 19.0f)
    {
        float Alpha = (Hour - 16.0f) / 3.0f;
        return InterpolatePresets(MiddayPreset, DuskPreset, Alpha);
    }
    else if (Hour < 22.0f)
    {
        float Alpha = (Hour - 19.0f) / 3.0f;
        return InterpolatePresets(DuskPreset, NightPreset, Alpha);
    }

    return NightPreset;
}

FLight_SkyColorPreset ADayNightCycleManager::InterpolatePresets(
    const FLight_SkyColorPreset& A,
    const FLight_SkyColorPreset& B,
    float Alpha) const
{
    FLight_SkyColorPreset Result;
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SkyColor = FMath::Lerp(A.SkyColor, B.SkyColor, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    return Result;
}

ELight_TimeOfDay ADayNightCycleManager::HourToTimeEnum(float Hour) const
{
    if (Hour < 5.0f || Hour >= 22.0f) return ELight_TimeOfDay::Midnight;
    if (Hour < 7.0f) return ELight_TimeOfDay::Dawn;
    if (Hour < 10.0f) return ELight_TimeOfDay::Morning;
    if (Hour < 14.0f) return ELight_TimeOfDay::Midday;
    if (Hour < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (Hour < 20.0f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}

void ADayNightCycleManager::SetTimeOfDay(float NewHour)
{
    CurrentTimeOfDay = FMath::Clamp(NewHour, 0.0f, 24.0f);
    CurrentTimeEnum = HourToTimeEnum(CurrentTimeOfDay);
    UpdateSunPosition();
    UpdateSkyColors();
    UpdateFog();
}

void ADayNightCycleManager::SetWeather(ELight_WeatherState NewWeather, float TransitionTime)
{
    TargetWeather.WeatherState = NewWeather;
    TargetWeather.TransitionDuration = TransitionTime;
    WeatherTransitionAlpha = 0.0f;

    switch (NewWeather)
    {
        case ELight_WeatherState::Clear:
            TargetWeather.CloudCoverage = 0.0f;
            TargetWeather.RainIntensity = 0.0f;
            TargetWeather.FogDensityMultiplier = 1.0f;
            break;
        case ELight_WeatherState::Overcast:
            TargetWeather.CloudCoverage = 0.8f;
            TargetWeather.RainIntensity = 0.0f;
            TargetWeather.FogDensityMultiplier = 1.5f;
            break;
        case ELight_WeatherState::Rain:
            TargetWeather.CloudCoverage = 0.9f;
            TargetWeather.RainIntensity = 0.6f;
            TargetWeather.FogDensityMultiplier = 2.0f;
            break;
        case ELight_WeatherState::Storm:
            TargetWeather.CloudCoverage = 1.0f;
            TargetWeather.RainIntensity = 1.0f;
            TargetWeather.LightningFrequency = 0.3f;
            TargetWeather.FogDensityMultiplier = 2.5f;
            break;
        case ELight_WeatherState::Fog:
            TargetWeather.CloudCoverage = 0.3f;
            TargetWeather.RainIntensity = 0.0f;
            TargetWeather.FogDensityMultiplier = 5.0f;
            break;
        case ELight_WeatherState::Haze:
            TargetWeather.CloudCoverage = 0.2f;
            TargetWeather.RainIntensity = 0.0f;
            TargetWeather.FogDensityMultiplier = 2.0f;
            break;
    }
}

bool ADayNightCycleManager::IsNight() const
{
    return CurrentTimeOfDay < 6.0f || CurrentTimeOfDay > 20.0f;
}

bool ADayNightCycleManager::IsDawn() const
{
    return CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.5f;
}

float ADayNightCycleManager::GetNormalizedDayProgress() const
{
    return CurrentTimeOfDay / 24.0f;
}
