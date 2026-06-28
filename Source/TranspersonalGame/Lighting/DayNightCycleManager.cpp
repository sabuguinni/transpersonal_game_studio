#include "DayNightCycleManager.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth transitions

    InitializeDefaultPresets();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightActors();
    ApplyTimePreset(GetCurrentTimeOfDayEnum());
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCycleActive)
    {
        UpdateLighting(DeltaTime);
    }
}

void ADayNightCycleManager::UpdateLighting(float DeltaTime)
{
    // Advance time (TimeScale = how many in-game hours pass per real second)
    float HoursPerSecond = TimeScale / 3600.0f;
    CurrentTimeOfDay += DeltaTime * HoursPerSecond;

    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    // Get interpolated settings for current time
    FLight_TimeOfDaySettings CurrentSettings = GetSettingsForTime(CurrentTimeOfDay);
    ApplySettings(CurrentSettings);
}

FLight_TimeOfDaySettings ADayNightCycleManager::GetSettingsForTime(float TimeHours) const
{
    // Define key times
    // Dawn: 5-7, Morning: 7-11, Midday: 11-14, Afternoon: 14-17, GoldenHour: 17-19, Dusk: 19-21, Night: 21-5

    FLight_TimeOfDaySettings Result;

    if (TimeHours >= 5.0f && TimeHours < 7.0f)
    {
        // Dawn — interpolate from night to dawn
        float Alpha = (TimeHours - 5.0f) / 2.0f;
        InterpolateLightingSettings(NightSettings, DawnSettings, Alpha);
        return DawnSettings; // simplified — return dawn
    }
    else if (TimeHours >= 7.0f && TimeHours < 17.0f)
    {
        // Day — interpolate between dawn and golden hour
        float Alpha = (TimeHours - 7.0f) / 10.0f;
        FLight_TimeOfDaySettings Blended;
        Blended.SunPitch = FMath::Lerp(DawnSettings.SunPitch, MiddaySettings.SunPitch, FMath::Sin(Alpha * PI));
        Blended.SunYaw = FMath::Lerp(DawnSettings.SunYaw, MiddaySettings.SunYaw, Alpha);
        Blended.SunIntensity = FMath::Lerp(DawnSettings.SunIntensity, MiddaySettings.SunIntensity, Alpha);
        Blended.SunColor = FLinearColor(
            FMath::Lerp(DawnSettings.SunColor.R, MiddaySettings.SunColor.R, Alpha),
            FMath::Lerp(DawnSettings.SunColor.G, MiddaySettings.SunColor.G, Alpha),
            FMath::Lerp(DawnSettings.SunColor.B, MiddaySettings.SunColor.B, Alpha),
            1.0f
        );
        Blended.FogDensity = FMath::Lerp(DawnSettings.FogDensity, MiddaySettings.FogDensity, Alpha);
        Blended.FogColor = FLinearColor(
            FMath::Lerp(DawnSettings.FogColor.R, MiddaySettings.FogColor.R, Alpha),
            FMath::Lerp(DawnSettings.FogColor.G, MiddaySettings.FogColor.G, Alpha),
            FMath::Lerp(DawnSettings.FogColor.B, MiddaySettings.FogColor.B, Alpha),
            1.0f
        );
        Blended.SkyLightIntensity = FMath::Lerp(DawnSettings.SkyLightIntensity, MiddaySettings.SkyLightIntensity, Alpha);
        return Blended;
    }
    else if (TimeHours >= 17.0f && TimeHours < 21.0f)
    {
        // Golden hour to dusk
        float Alpha = (TimeHours - 17.0f) / 4.0f;
        FLight_TimeOfDaySettings Blended;
        Blended.SunPitch = FMath::Lerp(GoldenHourSettings.SunPitch, NightSettings.SunPitch, Alpha);
        Blended.SunYaw = FMath::Lerp(GoldenHourSettings.SunYaw, NightSettings.SunYaw, Alpha);
        Blended.SunIntensity = FMath::Lerp(GoldenHourSettings.SunIntensity, NightSettings.SunIntensity, Alpha);
        Blended.SunColor = FLinearColor(
            FMath::Lerp(GoldenHourSettings.SunColor.R, NightSettings.SunColor.R, Alpha),
            FMath::Lerp(GoldenHourSettings.SunColor.G, NightSettings.SunColor.G, Alpha),
            FMath::Lerp(GoldenHourSettings.SunColor.B, NightSettings.SunColor.B, Alpha),
            1.0f
        );
        Blended.FogDensity = FMath::Lerp(GoldenHourSettings.FogDensity, NightSettings.FogDensity, Alpha);
        Blended.FogColor = FLinearColor(
            FMath::Lerp(GoldenHourSettings.FogColor.R, NightSettings.FogColor.R, Alpha),
            FMath::Lerp(GoldenHourSettings.FogColor.G, NightSettings.FogColor.G, Alpha),
            FMath::Lerp(GoldenHourSettings.FogColor.B, NightSettings.FogColor.B, Alpha),
            1.0f
        );
        Blended.SkyLightIntensity = FMath::Lerp(GoldenHourSettings.SkyLightIntensity, NightSettings.SkyLightIntensity, Alpha);
        return Blended;
    }
    else
    {
        // Night
        return NightSettings;
    }
}

void ADayNightCycleManager::InterpolateLightingSettings(const FLight_TimeOfDaySettings& From, const FLight_TimeOfDaySettings& To, float Alpha)
{
    // This is a utility — actual interpolation done inline in GetSettingsForTime
    (void)From; (void)To; (void)Alpha;
}

void ADayNightCycleManager::ApplySettings(const FLight_TimeOfDaySettings& Settings)
{
    // Apply to directional light (sun)
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Settings.SunPitch, Settings.SunYaw, 0.0f));
        UDirectionalLightComponent* DirComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DirComp)
        {
            DirComp->SetIntensity(Settings.SunIntensity);
            DirComp->SetLightColor(Settings.SunColor);
        }
    }

    // Apply to sky light
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Settings.SkyLightIntensity);
            SkyComp->SetLightColor(Settings.SkyLightColor);
        }
    }

    // Apply to fog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 4.5f && CurrentTimeOfDay < 6.5f)   return ELight_TimeOfDay::Dawn;
    if (CurrentTimeOfDay >= 6.5f && CurrentTimeOfDay < 11.0f)  return ELight_TimeOfDay::Morning;
    if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 14.0f) return ELight_TimeOfDay::Midday;
    if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay < 19.0f) return ELight_TimeOfDay::GoldenHour;
    if (CurrentTimeOfDay >= 19.0f && CurrentTimeOfDay < 21.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentTimeOfDay >= 21.0f || CurrentTimeOfDay < 2.0f)  return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

FString ADayNightCycleManager::GetTimeAsString() const
{
    int32 Hours = FMath::FloorToInt(CurrentTimeOfDay);
    int32 Minutes = FMath::FloorToInt((CurrentTimeOfDay - Hours) * 60.0f);
    return FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);
}

void ADayNightCycleManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    FLight_TimeOfDaySettings Settings = GetSettingsForTime(CurrentTimeOfDay);
    ApplySettings(Settings);
}

void ADayNightCycleManager::ApplyTimePreset(ELight_TimeOfDay TimePreset)
{
    switch (TimePreset)
    {
        case ELight_TimeOfDay::Dawn:        ApplySettings(DawnSettings); break;
        case ELight_TimeOfDay::Midday:      ApplySettings(MiddaySettings); break;
        case ELight_TimeOfDay::GoldenHour:  ApplySettings(GoldenHourSettings); break;
        case ELight_TimeOfDay::Night:       ApplySettings(NightSettings); break;
        default:                            ApplySettings(GoldenHourSettings); break;
    }
}

void ADayNightCycleManager::SetCycleActive(bool bActive)
{
    bCycleActive = bActive;
}

void ADayNightCycleManager::InitializeDefaultPresets()
{
    // Dawn — cool blue-pink light, low sun
    DawnSettings.SunPitch = -5.0f;
    DawnSettings.SunYaw = -90.0f;
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.6f, 1.0f);
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogColor = FLinearColor(0.7f, 0.6f, 0.8f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.8f;
    DawnSettings.SkyLightColor = FLinearColor(0.6f, 0.5f, 0.7f, 1.0f);

    // Midday — bright white light, high sun
    MiddaySettings.SunPitch = -75.0f;
    MiddaySettings.SunYaw = -60.0f;
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    MiddaySettings.FogDensity = 0.015f;
    MiddaySettings.FogColor = FLinearColor(0.7f, 0.8f, 0.95f, 1.0f);
    MiddaySettings.SkyLightIntensity = 2.0f;
    MiddaySettings.SkyLightColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);

    // Golden Hour — warm amber, low sun
    GoldenHourSettings.SunPitch = -22.0f;
    GoldenHourSettings.SunYaw = -60.0f;
    GoldenHourSettings.SunIntensity = 8.0f;
    GoldenHourSettings.SunColor = FLinearColor(1.0f, 0.65f, 0.25f, 1.0f);
    GoldenHourSettings.FogDensity = 0.035f;
    GoldenHourSettings.FogColor = FLinearColor(0.8f, 0.55f, 0.3f, 1.0f);
    GoldenHourSettings.SkyLightIntensity = 1.5f;
    GoldenHourSettings.SkyLightColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);

    // Night — dark blue, moon simulation
    NightSettings.SunPitch = 20.0f;  // Sun below horizon
    NightSettings.SunYaw = 120.0f;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.FogDensity = 0.05f;
    NightSettings.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    NightSettings.SkyLightIntensity = 0.3f;
    NightSettings.SkyLightColor = FLinearColor(0.2f, 0.25f, 0.5f, 1.0f);
}

void ADayNightCycleManager::AutoFindLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Auto-find directional light if not set
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    // Auto-find sky light if not set
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }

    // Auto-find height fog if not set
    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }
}
