#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for performance

    // --- Golden Hour (17:00-18:00) — Cretaceous warm amber ---
    GoldenHourSettings.SunPitch = -15.0f;
    GoldenHourSettings.SunYaw = 80.0f;
    GoldenHourSettings.SunIntensity = 8.0f;
    GoldenHourSettings.SunColor = FLinearColor(1.0f, 0.65f, 0.3f, 1.0f);
    GoldenHourSettings.FogDensity = 0.035f;
    GoldenHourSettings.FogColor = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
    GoldenHourSettings.SkyLightIntensity = 0.8f;
    GoldenHourSettings.ExposureBias = 0.8f;

    // --- Dawn (5:30-6:30) — Cretaceous misty morning ---
    DawnSettings.SunPitch = -5.0f;
    DawnSettings.SunYaw = -90.0f;
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnSettings.FogDensity = 0.06f;
    DawnSettings.FogColor = FLinearColor(0.6f, 0.55f, 0.7f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.4f;
    DawnSettings.ExposureBias = 0.5f;

    // --- Night (21:00-4:00) — Cretaceous dark ---
    NightSettings.SunPitch = 30.0f; // Below horizon
    NightSettings.SunYaw = -90.0f;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.35f, 0.6f, 1.0f);
    NightSettings.FogDensity = 0.04f;
    NightSettings.FogColor = FLinearColor(0.05f, 0.06f, 0.15f, 1.0f);
    NightSettings.SkyLightIntensity = 0.15f;
    NightSettings.ExposureBias = -1.5f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find lights if not assigned
    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }

    if (!SkyLightActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
        }
    }

    if (!FogActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
    }
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
    // Advance time
    float HoursPerSecond = TimeSpeed / 3600.0f;
    CurrentTimeOfDay += DeltaTime * HoursPerSecond;
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    // Get and apply settings for current time
    FLight_TimeOfDaySettings CurrentSettings = GetSettingsForTime(CurrentTimeOfDay);
    ApplyTimeOfDaySettings(CurrentSettings);
}

FLight_TimeOfDaySettings ADayNightCycleManager::GetSettingsForTime(float TimeHours) const
{
    // Piecewise interpolation across the day
    // Dawn: 5.0-7.0, Morning: 7.0-11.0, Midday: 11.0-14.0, Afternoon: 14.0-17.0
    // GoldenHour: 17.0-19.0, Dusk: 19.0-21.0, Night: 21.0-5.0

    // Midday settings (inline)
    FLight_TimeOfDaySettings MiddaySettings;
    MiddaySettings.SunPitch = -70.0f;
    MiddaySettings.SunYaw = 0.0f;
    MiddaySettings.SunIntensity = 15.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.97f, 0.9f, 1.0f);
    MiddaySettings.FogDensity = 0.015f;
    MiddaySettings.FogColor = FLinearColor(0.5f, 0.65f, 0.85f, 1.0f);
    MiddaySettings.SkyLightIntensity = 1.2f;
    MiddaySettings.ExposureBias = 1.2f;

    if (TimeHours >= 5.0f && TimeHours < 7.0f)
    {
        float Alpha = (TimeHours - 5.0f) / 2.0f;
        return InterpolateSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (TimeHours >= 7.0f && TimeHours < 12.0f)
    {
        float Alpha = (TimeHours - 7.0f) / 5.0f;
        return InterpolateSettings(DawnSettings, MiddaySettings, Alpha);
    }
    else if (TimeHours >= 12.0f && TimeHours < 17.0f)
    {
        float Alpha = (TimeHours - 12.0f) / 5.0f;
        return InterpolateSettings(MiddaySettings, GoldenHourSettings, Alpha);
    }
    else if (TimeHours >= 17.0f && TimeHours < 20.0f)
    {
        float Alpha = (TimeHours - 17.0f) / 3.0f;
        return InterpolateSettings(GoldenHourSettings, NightSettings, Alpha);
    }
    else
    {
        return NightSettings;
    }
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
    Result.ExposureBias = FMath::Lerp(A.ExposureBias, B.ExposureBias, Alpha);
    return Result;
}

void ADayNightCycleManager::ApplyTimeOfDaySettings(const FLight_TimeOfDaySettings& Settings)
{
    // Apply sun rotation and intensity
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

    // Apply fog
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
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
        }
    }
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f) return ELight_TimeOfDay::Dawn;
    if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f) return ELight_TimeOfDay::Morning;
    if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 14.0f) return ELight_TimeOfDay::Midday;
    if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay < 19.0f) return ELight_TimeOfDay::GoldenHour;
    if (CurrentTimeOfDay >= 19.0f && CurrentTimeOfDay < 21.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentTimeOfDay >= 21.0f || CurrentTimeOfDay < 2.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

void ADayNightCycleManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    FLight_TimeOfDaySettings Settings = GetSettingsForTime(CurrentTimeOfDay);
    ApplyTimeOfDaySettings(Settings);
}

float ADayNightCycleManager::GetNormalizedDayProgress() const
{
    return CurrentTimeOfDay / 24.0f;
}

void ADayNightCycleManager::ApplyGoldenHourPreset()
{
    SetTimeOfDay(17.5f);
}

void ADayNightCycleManager::ApplyDawnPreset()
{
    SetTimeOfDay(6.0f);
}

void ADayNightCycleManager::ApplyNightPreset()
{
    SetTimeOfDay(22.0f);
}
