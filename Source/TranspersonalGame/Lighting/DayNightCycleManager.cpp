#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // Update 20x per second

    InitializeDefaultPresets();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    FindAndCacheSceneLights();
    SetTimeOfDay(CurrentTimeOfDayHours);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void ADayNightCycleManager::InitializeDefaultPresets()
{
    // Dawn: 6:00 — warm pink-orange, low sun
    DawnSettings.SunPitchDegrees = -12.0f;
    DawnSettings.SunYawDegrees = 90.0f;
    DawnSettings.SunIntensity = 3.5f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(0.9f, 0.5f, 0.3f, 1.0f);
    DawnSettings.SkyLightIntensity = 1.2f;
    DawnSettings.SkyLightColor = FLinearColor(0.8f, 0.6f, 0.7f, 1.0f);
    DawnSettings.BloomIntensity = 1.2f;

    // Midday: 12:00 — bright white-yellow, high sun
    MiddaySettings.SunPitchDegrees = -72.0f;
    MiddaySettings.SunYawDegrees = 180.0f;
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    MiddaySettings.FogDensity = 0.015f;
    MiddaySettings.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    MiddaySettings.SkyLightIntensity = 2.5f;
    MiddaySettings.SkyLightColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    MiddaySettings.BloomIntensity = 0.3f;

    // Dusk: 18:00 — deep orange-purple, low western sun
    DuskSettings.SunPitchDegrees = -8.0f;
    DuskSettings.SunYawDegrees = 270.0f;
    DuskSettings.SunIntensity = 2.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(0.35f, 0.2f, 0.55f, 1.0f);
    DuskSettings.SkyLightIntensity = 1.8f;
    DuskSettings.SkyLightColor = FLinearColor(0.7f, 0.5f, 0.9f, 1.0f);
    DuskSettings.BloomIntensity = 0.8f;

    // Night: 22:00 — deep blue moonlight
    NightSettings.SunPitchDegrees = -30.0f;
    NightSettings.SunYawDegrees = 0.0f;
    NightSettings.SunIntensity = 0.3f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.FogDensity = 0.06f;
    NightSettings.FogColor = FLinearColor(0.05f, 0.05f, 0.15f, 1.0f);
    NightSettings.SkyLightIntensity = 0.4f;
    NightSettings.SkyLightColor = FLinearColor(0.2f, 0.25f, 0.5f, 1.0f);
    NightSettings.BloomIntensity = 1.5f;
}

void ADayNightCycleManager::FindAndCacheSceneLights()
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

void ADayNightCycleManager::UpdateDayNightCycle(float DeltaTime)
{
    if (DayLengthMinutes <= 0.0f) return;

    // Advance time: DayLengthMinutes real minutes = 24 game hours
    float HoursPerSecond = 24.0f / (DayLengthMinutes * 60.0f);
    CurrentTimeOfDayHours += HoursPerSecond * DeltaTime * TimeSpeedMultiplier;

    // Wrap around 24 hours
    if (CurrentTimeOfDayHours >= 24.0f)
    {
        CurrentTimeOfDayHours -= 24.0f;
    }

    NormalizedTimeOfDay = CurrentTimeOfDayHours / 24.0f;
    CurrentTimeOfDayEnum = HoursToTimeOfDayEnum(CurrentTimeOfDayHours);

    // Apply interpolated lighting based on current hour
    SetTimeOfDay(CurrentTimeOfDayHours);
}

void ADayNightCycleManager::SetTimeOfDay(float HoursIn24Format)
{
    CurrentTimeOfDayHours = FMath::Clamp(HoursIn24Format, 0.0f, 24.0f);
    CurrentTimeOfDayEnum = HoursToTimeOfDayEnum(CurrentTimeOfDayHours);

    FLight_TimeOfDaySettings TargetSettings;

    // Interpolate between presets based on hour
    float Hour = CurrentTimeOfDayHours;

    if (Hour >= 5.0f && Hour < 8.0f)
    {
        // Dawn to Morning
        float Alpha = (Hour - 5.0f) / 3.0f;
        TargetSettings = InterpolateSettings(DawnSettings, MiddaySettings, Alpha * 0.3f);
    }
    else if (Hour >= 8.0f && Hour < 16.0f)
    {
        // Morning to Afternoon
        float Alpha = (Hour - 8.0f) / 8.0f;
        TargetSettings = InterpolateSettings(MiddaySettings, MiddaySettings, Alpha);
    }
    else if (Hour >= 16.0f && Hour < 19.0f)
    {
        // Afternoon to Dusk
        float Alpha = (Hour - 16.0f) / 3.0f;
        TargetSettings = InterpolateSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else if (Hour >= 19.0f && Hour < 21.0f)
    {
        // Dusk to Night
        float Alpha = (Hour - 19.0f) / 2.0f;
        TargetSettings = InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }
    else if (Hour >= 21.0f || Hour < 5.0f)
    {
        // Night
        TargetSettings = NightSettings;
    }
    else
    {
        TargetSettings = MiddaySettings;
    }

    ApplyLightingSettings(TargetSettings);
}

void ADayNightCycleManager::SetTimeOfDayPreset(ELight_TimeOfDay TimePreset)
{
    switch (TimePreset)
    {
    case ELight_TimeOfDay::Dawn:
        SetTimeOfDay(6.0f);
        break;
    case ELight_TimeOfDay::Morning:
        SetTimeOfDay(9.0f);
        break;
    case ELight_TimeOfDay::Midday:
        SetTimeOfDay(12.0f);
        break;
    case ELight_TimeOfDay::Afternoon:
        SetTimeOfDay(15.0f);
        break;
    case ELight_TimeOfDay::GoldenHour:
        SetTimeOfDay(17.0f);
        break;
    case ELight_TimeOfDay::Dusk:
        SetTimeOfDay(18.5f);
        break;
    case ELight_TimeOfDay::Twilight:
        SetTimeOfDay(20.0f);
        break;
    case ELight_TimeOfDay::Night:
        SetTimeOfDay(22.0f);
        break;
    case ELight_TimeOfDay::Midnight:
        SetTimeOfDay(0.0f);
        break;
    }
}

void ADayNightCycleManager::ApplyLightingSettings(const FLight_TimeOfDaySettings& Settings)
{
    // Apply sun rotation and color
    if (SunLight)
    {
        FRotator SunRot(Settings.SunPitchDegrees, Settings.SunYawDegrees, 0.0f);
        SunLight->SetActorRotation(SunRot);

        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(Settings.SunIntensity);
            DLC->SetLightColor(Settings.SunColor);
        }
    }

    // Apply fog settings
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }

    // Apply sky light
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Settings.SkyLightIntensity);
            SkyComp->SetLightColor(Settings.SkyLightColor);
        }
    }
}

float ADayNightCycleManager::GetCurrentHour() const
{
    return CurrentTimeOfDayHours;
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDayEnum() const
{
    return CurrentTimeOfDayEnum;
}

ELight_TimeOfDay ADayNightCycleManager::HoursToTimeOfDayEnum(float Hours) const
{
    if (Hours >= 5.0f && Hours < 7.0f)  return ELight_TimeOfDay::Dawn;
    if (Hours >= 7.0f && Hours < 11.0f) return ELight_TimeOfDay::Morning;
    if (Hours >= 11.0f && Hours < 14.0f) return ELight_TimeOfDay::Midday;
    if (Hours >= 14.0f && Hours < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (Hours >= 17.0f && Hours < 18.5f) return ELight_TimeOfDay::GoldenHour;
    if (Hours >= 18.5f && Hours < 20.0f) return ELight_TimeOfDay::Dusk;
    if (Hours >= 20.0f && Hours < 21.5f) return ELight_TimeOfDay::Twilight;
    if (Hours >= 21.5f && Hours < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

FLight_TimeOfDaySettings ADayNightCycleManager::InterpolateSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

    Result.SunPitchDegrees = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
    Result.SunYawDegrees = FMath::Lerp(A.SunYawDegrees, B.SunYawDegrees, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.SkyLightColor = FMath::Lerp(A.SkyLightColor, B.SkyLightColor, Alpha);
    Result.bVolumetricFog = Alpha < 0.5f ? A.bVolumetricFog : B.bVolumetricFog;
    Result.BloomIntensity = FMath::Lerp(A.BloomIntensity, B.BloomIntensity, Alpha);

    return Result;
}

void ADayNightCycleManager::ApplyDuskPreset()
{
    SetTimeOfDayPreset(ELight_TimeOfDay::Dusk);
}

void ADayNightCycleManager::ApplyDawnPreset()
{
    SetTimeOfDayPreset(ELight_TimeOfDay::Dawn);
}

void ADayNightCycleManager::ApplyNightPreset()
{
    SetTimeOfDayPreset(ELight_TimeOfDay::Night);
}
