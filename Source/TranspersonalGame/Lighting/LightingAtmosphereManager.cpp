#include "LightingAtmosphereManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"

ALightingAtmosphereManager::ALightingAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth cycle

    // Default presets — Cretaceous world lighting
    // Dawn: cool blue-pink, low sun, heavy mist
    DawnSettings.SunPitchAngle = -5.0f;
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.5f, 1.0f);
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.4f;

    // Midday: bright warm white, high sun, light haze
    MiddaySettings.SunPitchAngle = -75.0f;
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    MiddaySettings.FogDensity = 0.01f;
    MiddaySettings.FogColor = FLinearColor(0.7f, 0.8f, 0.95f, 1.0f);
    MiddaySettings.SkyLightIntensity = 2.0f;

    // Dusk: deep amber-orange, low sun, warm fog
    DuskSettings.SunPitchAngle = -8.0f;
    DuskSettings.SunIntensity = 3.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.6f;

    // Night: deep blue moonlight, heavy fog, low intensity
    NightSettings.SunPitchAngle = 30.0f; // Below horizon
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    NightSettings.FogDensity = 0.06f;
    NightSettings.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    NightSettings.SkyLightIntensity = 0.15f;
}

void ALightingAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    FindAndCacheLightingActors();
    // Apply initial lighting based on starting time
    SetTimeOfDay(TimeOfDayNormalized);
}

void ALightingAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDayCycleActive)
    {
        UpdateDayCycle(DeltaTime);
    }
}

void ALightingAtmosphereManager::FindAndCacheLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight (Sun)
    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        SunLight = *It;
        break;
    }

    // Find ExponentialHeightFog
    for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
    {
        HeightFog = *It;
        break;
    }

    // Find SkyLight
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        SkyLightActor = *It;
        break;
    }
}

void ALightingAtmosphereManager::SetTimeOfDay(float NormalizedTime)
{
    TimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    CurrentTimeOfDay = GetCurrentTimeOfDay();
    UpdateSunPosition();
}

void ALightingAtmosphereManager::SetWeatherState(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
    UpdateFogForWeather();
}

ELight_TimeOfDay ALightingAtmosphereManager::GetCurrentTimeOfDay() const
{
    // 0.0 = midnight, 0.25 = dawn, 0.5 = noon, 0.75 = dusk, 1.0 = midnight
    if (TimeOfDayNormalized < 0.22f || TimeOfDayNormalized > 0.82f)
        return ELight_TimeOfDay::Night;
    if (TimeOfDayNormalized < 0.30f)
        return ELight_TimeOfDay::Dawn;
    if (TimeOfDayNormalized < 0.42f)
        return ELight_TimeOfDay::Morning;
    if (TimeOfDayNormalized < 0.58f)
        return ELight_TimeOfDay::Midday;
    if (TimeOfDayNormalized < 0.70f)
        return ELight_TimeOfDay::Afternoon;
    return ELight_TimeOfDay::Dusk;
}

void ALightingAtmosphereManager::ApplyLightingPreset(const FLight_DayPhaseSettings& Settings)
{
    if (SunLight)
    {
        FRotator CurrentRot = SunLight->GetActorRotation();
        SunLight->SetActorRotation(FRotator(Settings.SunPitchAngle, CurrentRot.Yaw, CurrentRot.Roll));

        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(Settings.SunIntensity);
            DLC->SetLightColor(Settings.SunColor);
        }
    }

    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }

    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(Settings.SkyLightIntensity);
        }
    }
}

void ALightingAtmosphereManager::ApplyCretaceousLightingNow()
{
    FindAndCacheLightingActors();
    // Apply warm afternoon Cretaceous lighting
    FLight_DayPhaseSettings CretaceousAfternoon;
    CretaceousAfternoon.SunPitchAngle = -45.0f;
    CretaceousAfternoon.SunIntensity = 8.5f;
    CretaceousAfternoon.SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);
    CretaceousAfternoon.FogDensity = 0.02f;
    CretaceousAfternoon.FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);
    CretaceousAfternoon.SkyLightIntensity = 1.2f;
    ApplyLightingPreset(CretaceousAfternoon);
}

void ALightingAtmosphereManager::UpdateDayCycle(float DeltaTime)
{
    // Advance time
    const float DayDurationSeconds = 86400.0f / DayCycleSpeed;
    TimeOfDayNormalized += DeltaTime / DayDurationSeconds;
    if (TimeOfDayNormalized > 1.0f)
    {
        TimeOfDayNormalized -= 1.0f;
    }

    CurrentTimeOfDay = GetCurrentTimeOfDay();
    UpdateSunPosition();
}

void ALightingAtmosphereManager::UpdateSunPosition()
{
    // Interpolate between presets based on time
    FLight_DayPhaseSettings TargetSettings;
    const float T = TimeOfDayNormalized;

    if (T < 0.25f)
    {
        // Night → Dawn
        float Alpha = T / 0.25f;
        TargetSettings = InterpolateSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (T < 0.5f)
    {
        // Dawn → Midday
        float Alpha = (T - 0.25f) / 0.25f;
        TargetSettings = InterpolateSettings(DawnSettings, MiddaySettings, Alpha);
    }
    else if (T < 0.75f)
    {
        // Midday → Dusk
        float Alpha = (T - 0.5f) / 0.25f;
        TargetSettings = InterpolateSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else
    {
        // Dusk → Night
        float Alpha = (T - 0.75f) / 0.25f;
        TargetSettings = InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }

    ApplyLightingPreset(TargetSettings);
}

void ALightingAtmosphereManager::UpdateFogForWeather()
{
    if (!HeightFog) return;

    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp) return;

    switch (CurrentWeather)
    {
    case ELight_WeatherState::Clear:
        FogComp->SetFogDensity(0.01f);
        break;
    case ELight_WeatherState::Overcast:
        FogComp->SetFogDensity(0.03f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.5f, 0.55f, 0.6f, 1.0f));
        break;
    case ELight_WeatherState::Stormy:
        FogComp->SetFogDensity(0.05f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.3f, 0.35f, 0.4f, 1.0f));
        break;
    case ELight_WeatherState::Foggy:
        FogComp->SetFogDensity(0.12f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.7f, 0.75f, 0.8f, 1.0f));
        break;
    case ELight_WeatherState::HeavyRain:
        FogComp->SetFogDensity(0.07f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.4f, 0.45f, 0.55f, 1.0f));
        break;
    }
}

FLight_DayPhaseSettings ALightingAtmosphereManager::InterpolateSettings(
    const FLight_DayPhaseSettings& A,
    const FLight_DayPhaseSettings& B,
    float Alpha) const
{
    FLight_DayPhaseSettings Result;
    Result.SunPitchAngle = FMath::Lerp(A.SunPitchAngle, B.SunPitchAngle, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    return Result;
}
