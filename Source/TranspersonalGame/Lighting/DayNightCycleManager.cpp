#include "DayNightCycleManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // Update 20x per second for smooth transitions
    InitializeDefaultSettings();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find lighting actors in the world if not manually assigned
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(GetWorld()); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }

    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }

    // Apply initial time of day
    UpdateTimeOfDayEnum();
    ApplyTimeOfDayPreset(CurrentTimeOfDay);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCycleActive)
    {
        UpdateLighting(DeltaTime);
    }
}

float ADayNightCycleManager::GetCurrentHour() const
{
    return TimeOfDayNormalized * 24.0f;
}

void ADayNightCycleManager::SetTimeOfDay(float NewTime)
{
    TimeOfDayNormalized = FMath::Clamp(NewTime, 0.0f, 1.0f);
    UpdateTimeOfDayEnum();
}

void ADayNightCycleManager::ApplyTimeOfDayPreset(ELight_TimeOfDay Preset)
{
    switch (Preset)
    {
        case ELight_TimeOfDay::Dawn:
            TimeOfDayNormalized = 0.25f; // 6:00 AM
            ApplySettingsToScene(DawnSettings);
            break;
        case ELight_TimeOfDay::Morning:
            TimeOfDayNormalized = 0.33f; // 8:00 AM
            break;
        case ELight_TimeOfDay::Midday:
            TimeOfDayNormalized = 0.5f;  // 12:00 PM
            ApplySettingsToScene(MiddaySettings);
            break;
        case ELight_TimeOfDay::Afternoon:
            TimeOfDayNormalized = 0.625f; // 3:00 PM
            break;
        case ELight_TimeOfDay::Dusk:
            TimeOfDayNormalized = 0.75f; // 6:00 PM
            ApplySettingsToScene(DuskSettings);
            break;
        case ELight_TimeOfDay::Night:
            TimeOfDayNormalized = 0.875f; // 9:00 PM
            break;
        case ELight_TimeOfDay::Midnight:
            TimeOfDayNormalized = 0.0f;  // 12:00 AM
            ApplySettingsToScene(NightSettings);
            break;
    }
    CurrentTimeOfDay = Preset;
}

float ADayNightCycleManager::GetAmbientTemperature() const
{
    float Hour = GetCurrentHour();
    // Temperature curve: coldest at dawn (4°C), warmest at 2pm (35°C)
    // Prehistoric Cretaceous was warmer than today
    float BaseTemp = 18.0f;
    float TempRange = 17.0f;
    float TempCurve = FMath::Sin((Hour - 4.0f) / 24.0f * PI * 2.0f - PI * 0.5f);
    return BaseTemp + TempRange * (TempCurve * 0.5f + 0.5f);
}

void ADayNightCycleManager::UpdateLighting(float DeltaTime)
{
    // Advance time
    float TimeAdvance = (DeltaTime / 86400.0f) * DayCycleSpeed;
    TimeOfDayNormalized = FMath::Fmod(TimeOfDayNormalized + TimeAdvance, 1.0f);

    UpdateTimeOfDayEnum();

    float Hour = GetCurrentHour();

    // Interpolate between key time settings
    FLight_TimeOfDaySettings CurrentSettings;

    if (Hour < 6.0f) // Midnight to Dawn
    {
        float Alpha = Hour / 6.0f;
        CurrentSettings = InterpolateSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (Hour < 12.0f) // Dawn to Midday
    {
        float Alpha = (Hour - 6.0f) / 6.0f;
        CurrentSettings = InterpolateSettings(DawnSettings, MiddaySettings, Alpha);
    }
    else if (Hour < 18.0f) // Midday to Dusk
    {
        float Alpha = (Hour - 12.0f) / 6.0f;
        CurrentSettings = InterpolateSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else // Dusk to Night
    {
        float Alpha = (Hour - 18.0f) / 6.0f;
        CurrentSettings = InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }

    ApplySettingsToScene(CurrentSettings);
}

void ADayNightCycleManager::UpdateTimeOfDayEnum()
{
    float Hour = GetCurrentHour();

    if (Hour < 1.0f || Hour >= 23.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Midnight;
    else if (Hour < 7.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
    else if (Hour < 10.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Morning;
    else if (Hour < 14.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Midday;
    else if (Hour < 17.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
    else if (Hour < 20.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
    else
        CurrentTimeOfDay = ELight_TimeOfDay::Night;
}

FLight_TimeOfDaySettings ADayNightCycleManager::InterpolateSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
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
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.AmbientTemperature = FMath::Lerp(A.AmbientTemperature, B.AmbientTemperature, Alpha);
    return Result;
}

void ADayNightCycleManager::ApplySettingsToScene(const FLight_TimeOfDaySettings& Settings)
{
    // Apply to DirectionalLight (Sun)
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Settings.SunPitch, Settings.SunYaw, 0.0f));
        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(Settings.SunIntensity);
            DLC->SetLightColor(Settings.SunColor);
        }
    }

    // Apply to ExponentialHeightFog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Settings.SkyLightIntensity);
            SkyComp->RecaptureSky();
        }
    }
}

void ADayNightCycleManager::InitializeDefaultSettings()
{
    // Dawn — cool blue-pink, low sun
    DawnSettings.SunPitch = -8.0f;
    DawnSettings.SunYaw = 80.0f;
    DawnSettings.SunIntensity = 2.5f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnSettings.FogDensity = 0.06f;
    DawnSettings.FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
    DawnSettings.SkyLightIntensity = 1.2f;
    DawnSettings.AmbientTemperature = 16.0f;

    // Midday — bright white-yellow, high sun
    MiddaySettings.SunPitch = -72.0f;
    MiddaySettings.SunYaw = 180.0f;
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
    MiddaySettings.FogDensity = 0.015f;
    MiddaySettings.FogColor = FLinearColor(0.8f, 0.88f, 1.0f, 1.0f);
    MiddaySettings.SkyLightIntensity = 2.5f;
    MiddaySettings.AmbientTemperature = 35.0f;

    // Dusk — warm amber-orange, low sun
    DuskSettings.SunPitch = -12.0f;
    DuskSettings.SunYaw = 270.0f;
    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);
    DuskSettings.FogDensity = 0.06f;
    DuskSettings.FogColor = FLinearColor(0.85f, 0.45f, 0.2f, 1.0f);
    DuskSettings.SkyLightIntensity = 1.0f;
    DuskSettings.AmbientTemperature = 28.0f;

    // Night — deep blue, moon light
    NightSettings.SunPitch = 15.0f; // Below horizon
    NightSettings.SunYaw = 0.0f;
    NightSettings.SunIntensity = 0.3f;
    NightSettings.SunColor = FLinearColor(0.4f, 0.5f, 0.9f, 1.0f);
    NightSettings.FogDensity = 0.04f;
    NightSettings.FogColor = FLinearColor(0.05f, 0.08f, 0.2f, 1.0f);
    NightSettings.SkyLightIntensity = 0.3f;
    NightSettings.AmbientTemperature = 18.0f;
}
