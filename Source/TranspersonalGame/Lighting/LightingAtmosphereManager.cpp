#include "LightingAtmosphereManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ALightingAtmosphereManager::ALightingAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 ticks/sec is enough for lighting

    // Default preset: Midday — bright, warm, clear
    MiddaySettings.SunIntensity = 10.0f;
    MiddaySettings.SunRotation = FRotator(-60.0f, 45.0f, 0.0f);
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    MiddaySettings.SkyLightIntensity = 1.5f;
    MiddaySettings.FogDensity = 0.01f;
    MiddaySettings.FogInscatteringColor = FLinearColor(0.8f, 0.88f, 1.0f, 1.0f);
    MiddaySettings.ExposureBias = 1.0f;

    // Dawn — low sun, warm orange, heavier fog
    DawnSettings.SunIntensity = 4.0f;
    DawnSettings.SunRotation = FRotator(-5.0f, 90.0f, 0.0f);
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.5f;
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogInscatteringColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    DawnSettings.ExposureBias = 0.5f;

    // Dusk — low sun, red-orange, atmospheric haze
    DuskSettings.SunIntensity = 3.0f;
    DuskSettings.SunRotation = FRotator(-8.0f, -90.0f, 0.0f);
    DuskSettings.SunColor = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.4f;
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogInscatteringColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    DuskSettings.ExposureBias = 0.4f;

    // Night — moonlight blue, dense fog, dark
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunRotation = FRotator(-15.0f, 180.0f, 0.0f);
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SkyLightIntensity = 0.1f;
    NightSettings.FogDensity = 0.06f;
    NightSettings.FogInscatteringColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    NightSettings.ExposureBias = -2.0f;
}

void ALightingAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find lighting actors in the world if not manually assigned
    UWorld* World = GetWorld();
    if (!World) return;

    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }

    if (!SkyLightActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
        }
    }

    if (!FogActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
    }

    if (!PostProcessVolume)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
        }
    }

    // Apply initial time of day
    SetTimeOfDay(CurrentHour);
}

void ALightingAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        AdvanceTime(DeltaTime);
    }

    // Weather transition
    if (WeatherTransitionTimer > 0.0f)
    {
        WeatherTransitionTimer -= DeltaTime;
        if (WeatherTransitionTimer <= 0.0f)
        {
            WeatherTransitionTimer = 0.0f;
            CurrentWeather.WeatherState = TargetWeatherState;
        }
    }
}

void ALightingAtmosphereManager::AdvanceTime(float DeltaTime)
{
    if (DayDurationSeconds <= 0.0f) return;

    // Advance hour proportionally to real-time delta
    float HoursPerSecond = 24.0f / DayDurationSeconds;
    CurrentHour += HoursPerSecond * DeltaTime;
    if (CurrentHour >= 24.0f) CurrentHour -= 24.0f;

    CurrentTimeOfDay = HourToTimeOfDay(CurrentHour);
    UpdateSunPosition();
    UpdateFog();
    UpdateSkyLight();
    UpdatePostProcess();
}

void ALightingAtmosphereManager::SetTimeOfDay(float Hour)
{
    CurrentHour = FMath::Clamp(Hour, 0.0f, 24.0f);
    CurrentTimeOfDay = HourToTimeOfDay(CurrentHour);
    UpdateSunPosition();
    UpdateFog();
    UpdateSkyLight();
    UpdatePostProcess();
}

void ALightingAtmosphereManager::SetWeatherState(ELight_WeatherState NewWeather, float TransitionTime)
{
    PreviousWeather = CurrentWeather;
    TargetWeatherState = NewWeather;
    WeatherTransitionDuration = FMath::Max(TransitionTime, 0.1f);
    WeatherTransitionTimer = WeatherTransitionDuration;
}

void ALightingAtmosphereManager::ApplyLightingPreset(const FLight_TimeOfDaySettings& Settings)
{
    if (SunLight)
    {
        UDirectionalLightComponent* Comp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (Comp)
        {
            Comp->SetIntensity(Settings.SunIntensity);
            Comp->SetLightColor(Settings.SunColor);
        }
        // Ensure pitch is always negative (sun points down)
        FRotator SafeRot = Settings.SunRotation;
        if (SafeRot.Pitch >= 0.0f) SafeRot.Pitch = -FMath::Abs(SafeRot.Pitch) - 1.0f;
        SunLight->SetActorRotation(SafeRot);
    }

    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogInscatteringColor);
        }
    }

    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Settings.SkyLightIntensity);
        }
    }
}

void ALightingAtmosphereManager::UpdateSunPosition()
{
    // Interpolate between presets based on hour
    FLight_TimeOfDaySettings TargetSettings;

    if (CurrentHour >= 5.0f && CurrentHour < 8.0f)
    {
        // Dawn
        float Alpha = (CurrentHour - 5.0f) / 3.0f;
        TargetSettings = InterpolateSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (CurrentHour >= 8.0f && CurrentHour < 12.0f)
    {
        // Morning to Midday
        float Alpha = (CurrentHour - 8.0f) / 4.0f;
        TargetSettings = InterpolateSettings(DawnSettings, MiddaySettings, Alpha);
    }
    else if (CurrentHour >= 12.0f && CurrentHour < 17.0f)
    {
        // Midday to Afternoon
        TargetSettings = MiddaySettings;
    }
    else if (CurrentHour >= 17.0f && CurrentHour < 20.0f)
    {
        // Afternoon to Dusk
        float Alpha = (CurrentHour - 17.0f) / 3.0f;
        TargetSettings = InterpolateSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else if (CurrentHour >= 20.0f && CurrentHour < 22.0f)
    {
        // Dusk to Night
        float Alpha = (CurrentHour - 20.0f) / 2.0f;
        TargetSettings = InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }
    else
    {
        // Full Night
        TargetSettings = NightSettings;
    }

    ApplyLightingPreset(TargetSettings);
}

void ALightingAtmosphereManager::UpdateFog()
{
    // Weather overrides fog density
    if (FogActor && CurrentWeather.FogDensityOverride >= 0.0f)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(CurrentWeather.FogDensityOverride);
        }
    }
}

void ALightingAtmosphereManager::UpdateSkyLight()
{
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp && SkyComp->IsRealTimeCaptureEnabled())
        {
            SkyComp->RecaptureSky();
        }
    }
}

void ALightingAtmosphereManager::UpdatePostProcess()
{
    // PostProcess is managed via console variables for Lumen
    // Individual PPV settings are set in BeginPlay via sanity guard
}

ELight_TimeOfDay ALightingAtmosphereManager::HourToTimeOfDay(float Hour) const
{
    if (Hour >= 5.0f && Hour < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (Hour >= 7.0f && Hour < 11.0f)  return ELight_TimeOfDay::Morning;
    if (Hour >= 11.0f && Hour < 14.0f) return ELight_TimeOfDay::Midday;
    if (Hour >= 14.0f && Hour < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (Hour >= 17.0f && Hour < 20.0f) return ELight_TimeOfDay::Dusk;
    if (Hour >= 20.0f && Hour < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

FLight_TimeOfDaySettings ALightingAtmosphereManager::InterpolateSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunRotation = FRotator(
        FMath::Lerp(A.SunRotation.Pitch, B.SunRotation.Pitch, Alpha),
        FMath::Lerp(A.SunRotation.Yaw, B.SunRotation.Yaw, Alpha),
        0.0f
    );
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogInscatteringColor = FLinearColor(
        FMath::Lerp(A.FogInscatteringColor.R, B.FogInscatteringColor.R, Alpha),
        FMath::Lerp(A.FogInscatteringColor.G, B.FogInscatteringColor.G, Alpha),
        FMath::Lerp(A.FogInscatteringColor.B, B.FogInscatteringColor.B, Alpha),
        1.0f
    );
    Result.ExposureBias = FMath::Lerp(A.ExposureBias, B.ExposureBias, Alpha);
    return Result;
}

ELight_TimeOfDay ALightingAtmosphereManager::GetCurrentTimeOfDay() const
{
    return CurrentTimeOfDay;
}

float ALightingAtmosphereManager::GetCurrentHour() const
{
    return CurrentHour;
}

bool ALightingAtmosphereManager::IsDaytime() const
{
    return CurrentHour >= 6.0f && CurrentHour < 20.0f;
}

bool ALightingAtmosphereManager::IsNighttime() const
{
    return !IsDaytime();
}

void ALightingAtmosphereManager::ForceMiddayLighting()
{
    SetTimeOfDay(12.0f);
}

void ALightingAtmosphereManager::ForceDawnLighting()
{
    SetTimeOfDay(6.0f);
}

void ALightingAtmosphereManager::ForceDuskLighting()
{
    SetTimeOfDay(18.5f);
}
