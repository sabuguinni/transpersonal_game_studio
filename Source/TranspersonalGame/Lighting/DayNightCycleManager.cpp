#include "DayNightCycleManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth transitions

    // Dawn preset — warm pink-orange at horizon
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunPitchAngle = -5.0f;
    DawnSettings.FogColor = FLinearColor(0.8f, 0.5f, 0.4f, 1.0f);
    DawnSettings.FogDensity = 0.08f;
    DawnSettings.SkyLightIntensity = 0.8f;
    DawnSettings.AmbientTemperature = 18.0f;

    // Midday preset — bright white-yellow
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunPitchAngle = -80.0f;
    MiddaySettings.FogColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    MiddaySettings.FogDensity = 0.02f;
    MiddaySettings.SkyLightIntensity = 2.5f;
    MiddaySettings.AmbientTemperature = 32.0f;

    // Dusk preset — deep orange-red
    DuskSettings.SunColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunPitchAngle = -8.0f;
    DuskSettings.FogColor = FLinearColor(0.9f, 0.4f, 0.2f, 1.0f);
    DuskSettings.FogDensity = 0.1f;
    DuskSettings.SkyLightIntensity = 0.6f;
    DuskSettings.AmbientTemperature = 25.0f;

    // Night preset — deep blue moonlight
    NightSettings.SunColor = FLinearColor(0.1f, 0.15f, 0.4f, 1.0f);
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunPitchAngle = -45.0f; // Moon angle
    NightSettings.FogColor = FLinearColor(0.05f, 0.08f, 0.2f, 1.0f);
    NightSettings.FogDensity = 0.06f;
    NightSettings.SkyLightIntensity = 0.3f;
    NightSettings.AmbientTemperature = 14.0f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightingActors();
    ApplyTimePreset(GetCurrentTimeOfDay());
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive) return;

    // Advance time
    float HoursPerSecond = TimeSpeedMultiplier / 3600.0f;
    CurrentTimeHours += DeltaTime * HoursPerSecond;
    if (CurrentTimeHours >= 24.0f)
    {
        CurrentTimeHours -= 24.0f;
    }

    UpdateLighting(DeltaTime);
}

void ADayNightCycleManager::AutoFindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight (sun)
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

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDay() const
{
    if (CurrentTimeHours >= 5.0f && CurrentTimeHours < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (CurrentTimeHours >= 7.0f && CurrentTimeHours < 10.0f)  return ELight_TimeOfDay::Morning;
    if (CurrentTimeHours >= 10.0f && CurrentTimeHours < 14.0f) return ELight_TimeOfDay::Midday;
    if (CurrentTimeHours >= 14.0f && CurrentTimeHours < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentTimeHours >= 17.0f && CurrentTimeHours < 20.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentTimeHours >= 20.0f || CurrentTimeHours < 2.0f)  return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::MidNight;
}

float ADayNightCycleManager::GetNormalizedTimeOfDay() const
{
    return CurrentTimeHours / 24.0f;
}

void ADayNightCycleManager::SetTimeOfDay(float NewHours)
{
    CurrentTimeHours = FMath::Clamp(NewHours, 0.0f, 24.0f);
    ApplyTimePreset(GetCurrentTimeOfDay());
}

void ADayNightCycleManager::ApplyTimePreset(ELight_TimeOfDay TimeOfDay)
{
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
        case ELight_TimeOfDay::Morning:
            ApplySettings(DawnSettings);
            break;
        case ELight_TimeOfDay::Midday:
        case ELight_TimeOfDay::Afternoon:
            ApplySettings(MiddaySettings);
            break;
        case ELight_TimeOfDay::Dusk:
            ApplySettings(DuskSettings);
            break;
        case ELight_TimeOfDay::Night:
        case ELight_TimeOfDay::MidNight:
            ApplySettings(NightSettings);
            break;
        default:
            ApplySettings(MiddaySettings);
            break;
    }
}

float ADayNightCycleManager::GetAmbientTemperature() const
{
    ELight_TimeOfDay TOD = GetCurrentTimeOfDay();
    switch (TOD)
    {
        case ELight_TimeOfDay::Dawn:    return DawnSettings.AmbientTemperature;
        case ELight_TimeOfDay::Midday:  return MiddaySettings.AmbientTemperature;
        case ELight_TimeOfDay::Dusk:    return DuskSettings.AmbientTemperature;
        case ELight_TimeOfDay::Night:   return NightSettings.AmbientTemperature;
        default:                        return MiddaySettings.AmbientTemperature;
    }
}

void ADayNightCycleManager::UpdateLighting(float DeltaTime)
{
    // Determine blend between presets based on time
    FLight_TimeOfDaySettings TargetSettings;

    if (CurrentTimeHours >= 5.0f && CurrentTimeHours < 10.0f)
    {
        float Alpha = (CurrentTimeHours - 5.0f) / 5.0f;
        TargetSettings = InterpolateSettings(DawnSettings, MiddaySettings, Alpha);
    }
    else if (CurrentTimeHours >= 10.0f && CurrentTimeHours < 17.0f)
    {
        TargetSettings = MiddaySettings;
    }
    else if (CurrentTimeHours >= 17.0f && CurrentTimeHours < 20.0f)
    {
        float Alpha = (CurrentTimeHours - 17.0f) / 3.0f;
        TargetSettings = InterpolateSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else if (CurrentTimeHours >= 20.0f && CurrentTimeHours < 22.0f)
    {
        float Alpha = (CurrentTimeHours - 20.0f) / 2.0f;
        TargetSettings = InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }
    else
    {
        TargetSettings = NightSettings;
    }

    // Compute sun yaw from time (full 360 over 24h)
    float SunYaw = (CurrentTimeHours / 24.0f) * 360.0f;
    TargetSettings.SunPitchAngle = FMath::Clamp(TargetSettings.SunPitchAngle, -85.0f, -5.0f);

    ApplySettings(TargetSettings);

    // Update sun rotation
    if (SunLight)
    {
        FRotator NewRot(TargetSettings.SunPitchAngle, SunYaw, 0.0f);
        SunLight->SetActorRotation(NewRot);
    }
}

FLight_TimeOfDaySettings ADayNightCycleManager::InterpolateSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunPitchAngle = FMath::Lerp(A.SunPitchAngle, B.SunPitchAngle, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.AmbientTemperature = FMath::Lerp(A.AmbientTemperature, B.AmbientTemperature, Alpha);
    return Result;
}

void ADayNightCycleManager::ApplySettings(const FLight_TimeOfDaySettings& Settings)
{
    // Apply to DirectionalLight
    if (SunLight)
    {
        UDirectionalLightComponent* DLC = SunLight->GetComponent();
        if (DLC)
        {
            DLC->SetIntensity(Settings.SunIntensity);
            DLC->SetLightColor(Settings.SunColor);
        }
    }

    // Apply to ExponentialHeightFog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
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
}
