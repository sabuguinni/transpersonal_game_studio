#include "DayNightCycleManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second — sufficient for smooth transitions

    InitDefaultPresets();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindSceneActors();
    SetTimeOfDay(CurrentHour);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive) return;

    // Advance time — TimeSpeed=60 means 1 real second = 1 game minute
    const float HoursPerSecond = TimeSpeed / 3600.0f;
    CurrentHour = FMath::Fmod(CurrentHour + DeltaTime * HoursPerSecond, 24.0f);

    UpdateSunTransform(CurrentHour);

    // Detect time-of-day transitions
    ELight_TimeOfDay NewTOD = GetTimeOfDayEnum();
    if (NewTOD != PreviousTimeOfDay)
    {
        OnTimeOfDayChanged(NewTOD);
        PreviousTimeOfDay = NewTOD;
    }

    // Detect sunrise/sunset events
    bool bIsNight = IsNightTime();
    if (bWasNight && !bIsNight)
    {
        OnSunrise();
    }
    else if (!bWasNight && bIsNight)
    {
        OnSunset();
    }
    bWasNight = bIsNight;

    // Update fog and sky with current settings
    FLight_TimeOfDaySettings CurrentSettings = GetCurrentSettings();
    UpdateFogSettings(CurrentSettings);
    UpdateSkyLight(CurrentSettings);
}

void ADayNightCycleManager::SetTimeOfDay(float NewHour)
{
    CurrentHour = FMath::Clamp(NewHour, 0.0f, 24.0f);
    UpdateSunTransform(CurrentHour);

    FLight_TimeOfDaySettings Settings = GetCurrentSettings();
    UpdateFogSettings(Settings);
    UpdateSkyLight(Settings);

    CurrentTimeOfDay = GetTimeOfDayEnum();
    PreviousTimeOfDay = CurrentTimeOfDay;
    bWasNight = IsNightTime();
}

ELight_TimeOfDay ADayNightCycleManager::GetTimeOfDayEnum() const
{
    if (CurrentHour >= 5.0f && CurrentHour < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (CurrentHour >= 7.0f && CurrentHour < 10.0f)  return ELight_TimeOfDay::Morning;
    if (CurrentHour >= 10.0f && CurrentHour < 14.0f) return ELight_TimeOfDay::Noon;
    if (CurrentHour >= 14.0f && CurrentHour < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentHour >= 17.0f && CurrentHour < 20.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentHour >= 20.0f || CurrentHour < 2.0f)  return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::MidNight;
}

float ADayNightCycleManager::GetNormalizedDayProgress() const
{
    return CurrentHour / 24.0f;
}

bool ADayNightCycleManager::IsNightTime() const
{
    return CurrentHour >= 20.0f || CurrentHour < 5.0f;
}

FLight_TimeOfDaySettings ADayNightCycleManager::GetCurrentSettings() const
{
    // Blend between key presets based on hour
    // Dawn: 5-7, Noon: 12, Dusk: 18, Night: 22
    if (CurrentHour >= 5.0f && CurrentHour < 12.0f)
    {
        float Alpha = (CurrentHour - 5.0f) / 7.0f;
        return LerpSettings(DawnSettings, NoonSettings, Alpha);
    }
    else if (CurrentHour >= 12.0f && CurrentHour < 18.0f)
    {
        float Alpha = (CurrentHour - 12.0f) / 6.0f;
        return LerpSettings(NoonSettings, DuskSettings, Alpha);
    }
    else if (CurrentHour >= 18.0f && CurrentHour < 22.0f)
    {
        float Alpha = (CurrentHour - 18.0f) / 4.0f;
        return LerpSettings(DuskSettings, NightSettings, Alpha);
    }
    else
    {
        // Night to dawn (22:00 - 5:00)
        float NightDuration = 7.0f; // 22 to 5 (wrapping)
        float HoursIntoNight = (CurrentHour >= 22.0f) ? (CurrentHour - 22.0f) : (CurrentHour + 2.0f);
        float Alpha = FMath::Clamp(HoursIntoNight / NightDuration, 0.0f, 1.0f);
        return LerpSettings(NightSettings, DawnSettings, Alpha);
    }
}

void ADayNightCycleManager::UpdateSunTransform(float Hour)
{
    if (!SunLight) return;

    // Sun arc: rises at 5:00 (east), peaks at 12:00 (south), sets at 19:00 (west)
    // Map hour to sun pitch: -90 (horizon) at 5:00, -90 at 19:00, -10 (near zenith) at 12:00
    // Night: sun below horizon (positive pitch = underground)

    float SunPitch = 0.0f;
    float SunYaw = 0.0f;
    float SunIntensity = 0.0f;

    if (Hour >= 5.0f && Hour <= 19.0f)
    {
        // Daytime arc — pitch goes from -5 (horizon) at 5h, to -80 (near zenith) at 12h, back to -5 at 19h
        float DayProgress = (Hour - 5.0f) / 14.0f; // 0.0 to 1.0
        float ArcAngle = FMath::Sin(DayProgress * PI); // 0 to 1 to 0
        SunPitch = FMath::Lerp(-5.0f, -80.0f, ArcAngle);
        SunYaw = FMath::Lerp(-90.0f, 90.0f, DayProgress); // East to West

        // Intensity: low at dawn/dusk, high at noon
        SunIntensity = FMath::Lerp(2.0f, 10.0f, ArcAngle);
    }
    else
    {
        // Night — sun below horizon, minimal ambient
        SunPitch = 10.0f; // Above horizon = pointing at sky = no ground illumination
        SunYaw = 180.0f;
        SunIntensity = 0.1f;
    }

    CurrentSunPitch = SunPitch;
    SunLight->SetActorRotation(FRotator(SunPitch, SunYaw, 0.0f));

    UDirectionalLightComponent* DLComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (DLComp)
    {
        DLComp->SetIntensity(SunIntensity);

        // Color temperature: warm at dawn/dusk, neutral at noon
        FLight_TimeOfDaySettings Settings = GetCurrentSettings();
        DLComp->SetLightColor(Settings.SunColor);
    }
}

void ADayNightCycleManager::UpdateFogSettings(const FLight_TimeOfDaySettings& Settings)
{
    if (!HeightFog) return;

    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp) return;

    FogComp->SetFogDensity(Settings.FogDensity);
    FogComp->SetFogHeightFalloff(Settings.FogHeightFalloff);
    FogComp->SetFogInscatteringColor(Settings.FogInscatteringColor);
}

void ADayNightCycleManager::UpdateSkyLight(const FLight_TimeOfDaySettings& Settings)
{
    if (!SkyLightActor) return;

    USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
    if (!SkyComp) return;

    SkyComp->SetIntensity(Settings.SkyLightIntensity);
    SkyComp->RecaptureSky();
}

FLight_TimeOfDaySettings ADayNightCycleManager::LerpSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunYaw = FMath::Lerp(A.SunYaw, B.SunYaw, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.FogInscatteringColor = FLinearColor(
        FMath::Lerp(A.FogInscatteringColor.R, B.FogInscatteringColor.R, Alpha),
        FMath::Lerp(A.FogInscatteringColor.G, B.FogInscatteringColor.G, Alpha),
        FMath::Lerp(A.FogInscatteringColor.B, B.FogInscatteringColor.B, Alpha),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    return Result;
}

void ADayNightCycleManager::AutoFindSceneActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight (Sun)
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    // Find ExponentialHeightFog
    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
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
}

void ADayNightCycleManager::InitDefaultPresets()
{
    // Dawn — 5:00-7:00 — warm orange, low angle, thick haze
    DawnSettings.SunPitch = -5.0f;
    DawnSettings.SunYaw = -80.0f;
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogHeightFalloff = 0.15f;
    DawnSettings.FogInscatteringColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.5f;

    // Noon — 12:00 — bright white, high sun, thin haze
    NoonSettings.SunPitch = -75.0f;
    NoonSettings.SunYaw = 0.0f;
    NoonSettings.SunIntensity = 10.0f;
    NoonSettings.SunColor = FLinearColor(1.0f, 0.97f, 0.9f, 1.0f);
    NoonSettings.FogDensity = 0.01f;
    NoonSettings.FogHeightFalloff = 0.3f;
    NoonSettings.FogInscatteringColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
    NoonSettings.SkyLightIntensity = 1.2f;

    // Dusk — 18:00-20:00 — deep orange/red, low angle, thick haze
    DuskSettings.SunPitch = -8.0f;
    DuskSettings.SunYaw = 80.0f;
    DuskSettings.SunIntensity = 3.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
    DuskSettings.FogDensity = 0.06f;
    DuskSettings.FogHeightFalloff = 0.12f;
    DuskSettings.FogInscatteringColor = FLinearColor(0.9f, 0.4f, 0.2f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.4f;

    // Night — 22:00-5:00 — deep blue, stars, minimal light
    NightSettings.SunPitch = 20.0f; // Below horizon
    NightSettings.SunYaw = 180.0f;
    NightSettings.SunIntensity = 0.05f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.FogDensity = 0.03f;
    NightSettings.FogHeightFalloff = 0.1f;
    NightSettings.FogInscatteringColor = FLinearColor(0.05f, 0.08f, 0.2f, 1.0f);
    NightSettings.SkyLightIntensity = 0.1f;
}
