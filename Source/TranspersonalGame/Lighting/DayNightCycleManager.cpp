#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth transitions

    // Dawn defaults (5:00 - 7:00)
    DawnSettings.SunPitch = -5.0f;
    DawnSettings.SunYaw = 90.0f;
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.SkyLightIntensity = 0.5f;
    DawnSettings.ExposureBias = 0.5f;

    // Midday defaults (12:00)
    MiddaySettings.SunPitch = -75.0f;
    MiddaySettings.SunYaw = 180.0f;
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
    MiddaySettings.FogDensity = 0.01f;
    MiddaySettings.SkyLightIntensity = 2.0f;
    MiddaySettings.ExposureBias = 1.0f;

    // Dusk defaults (18:00 - 20:00)
    DuskSettings.SunPitch = -8.0f;
    DuskSettings.SunYaw = 270.0f;
    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);
    DuskSettings.FogDensity = 0.035f;
    DuskSettings.SkyLightIntensity = 0.6f;
    DuskSettings.ExposureBias = 0.6f;

    // Night defaults (22:00 - 4:00)
    NightSettings.SunPitch = -45.0f;
    NightSettings.SunYaw = 0.0f;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.35f, 0.6f, 1.0f);
    NightSettings.FogDensity = 0.05f;
    NightSettings.SkyLightIntensity = 0.2f;
    NightSettings.ExposureBias = -1.5f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindActorsInternal();
    // Apply initial time settings
    SetTimeOfDay(CurrentTimeOfDay);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bEnableDayNightCycle)
        return;

    UpdateSunPosition(DeltaTime);
}

void ADayNightCycleManager::UpdateSunPosition(float DeltaTime)
{
    // Advance time
    float HoursPerSecond = TimeScale / 3600.0f;
    CurrentTimeOfDay += DeltaTime * HoursPerSecond;
    if (CurrentTimeOfDay >= 24.0f)
        CurrentTimeOfDay -= 24.0f;

    // Determine current phase and interpolate settings
    FLight_TimeSettings CurrentSettings;
    float T = CurrentTimeOfDay;

    if (T >= 4.0f && T < 7.0f)
    {
        // Dawn: 4-7
        float Alpha = (T - 4.0f) / 3.0f;
        CurrentSettings = InterpolateSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (T >= 7.0f && T < 12.0f)
    {
        // Morning to Midday: 7-12
        float Alpha = (T - 7.0f) / 5.0f;
        CurrentSettings = InterpolateSettings(DawnSettings, MiddaySettings, Alpha);
    }
    else if (T >= 12.0f && T < 18.0f)
    {
        // Midday to Dusk: 12-18
        float Alpha = (T - 12.0f) / 6.0f;
        CurrentSettings = InterpolateSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else if (T >= 18.0f && T < 22.0f)
    {
        // Dusk to Night: 18-22
        float Alpha = (T - 18.0f) / 4.0f;
        CurrentSettings = InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }
    else
    {
        // Night: 22-4
        CurrentSettings = NightSettings;
    }

    ApplyTimeSettings(CurrentSettings);
}

void ADayNightCycleManager::ApplyTimeSettings(const FLight_TimeSettings& Settings)
{
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

    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
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

FLight_TimeSettings ADayNightCycleManager::InterpolateSettings(const FLight_TimeSettings& A, const FLight_TimeSettings& B, float Alpha) const
{
    FLight_TimeSettings Result;
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
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.ExposureBias = FMath::Lerp(A.ExposureBias, B.ExposureBias, Alpha);
    return Result;
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 4.0f && CurrentTimeOfDay < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 10.0f)  return ELight_TimeOfDay::Morning;
    if (CurrentTimeOfDay >= 10.0f && CurrentTimeOfDay < 14.0f) return ELight_TimeOfDay::Midday;
    if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 18.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 21.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentTimeOfDay >= 21.0f || CurrentTimeOfDay < 2.0f)  return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

void ADayNightCycleManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    // Force immediate update
    FLight_TimeSettings Settings;
    float T = CurrentTimeOfDay;
    if (T >= 4.0f && T < 7.0f)
        Settings = InterpolateSettings(NightSettings, DawnSettings, (T - 4.0f) / 3.0f);
    else if (T >= 7.0f && T < 12.0f)
        Settings = InterpolateSettings(DawnSettings, MiddaySettings, (T - 7.0f) / 5.0f);
    else if (T >= 12.0f && T < 18.0f)
        Settings = InterpolateSettings(MiddaySettings, DuskSettings, (T - 12.0f) / 6.0f);
    else if (T >= 18.0f && T < 22.0f)
        Settings = InterpolateSettings(DuskSettings, NightSettings, (T - 18.0f) / 4.0f);
    else
        Settings = NightSettings;
    ApplyTimeSettings(Settings);
}

float ADayNightCycleManager::GetNormalizedDayProgress() const
{
    return CurrentTimeOfDay / 24.0f;
}

FString ADayNightCycleManager::GetTimeString() const
{
    int32 Hours = FMath::FloorToInt(CurrentTimeOfDay);
    int32 Minutes = FMath::FloorToInt((CurrentTimeOfDay - Hours) * 60.0f);
    return FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);
}

void ADayNightCycleManager::AutoFindLightActors()
{
    AutoFindActorsInternal();
}

void ADayNightCycleManager::AutoFindActorsInternal()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }

    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }
}
