#include "DayNightCycleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth transitions

    InitializeDefaultPresets();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightingActors();
    ForceUpdateLighting();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive)
        return;

    // Advance time: TimeSpeedMultiplier=60 means 1 real second = 1 game minute
    // Full day = 86400 real seconds / TimeSpeedMultiplier
    float DeltaNormalized = (DeltaTime * TimeSpeedMultiplier) / 86400.0f;
    CurrentTimeNormalized = FMath::Fmod(CurrentTimeNormalized + DeltaNormalized, 1.0f);

    UpdateSunPosition(DeltaTime);
    UpdateFogSettings();
    UpdateSkyLight();
}

void ADayNightCycleManager::InitializeDefaultPresets()
{
    TimeOfDayPresets.Empty();

    // Dawn (5:00 AM) — 0.208
    FLight_TimeOfDayConfig Dawn;
    Dawn.TimeOfDay = ELight_TimeOfDay::Dawn;
    Dawn.SunPitchDegrees = -5.0f;
    Dawn.SunYawDegrees = 90.0f;
    Dawn.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    Dawn.SunIntensity = 2.0f;
    Dawn.FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    Dawn.FogDensity = 0.04f;
    Dawn.FogHeightFalloff = 0.15f;
    Dawn.AmbientBrightness = 0.3f;
    TimeOfDayPresets.Add(Dawn);

    // Morning (9:00 AM) — 0.375
    FLight_TimeOfDayConfig Morning;
    Morning.TimeOfDay = ELight_TimeOfDay::Morning;
    Morning.SunPitchDegrees = -40.0f;
    Morning.SunYawDegrees = 120.0f;
    Morning.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    Morning.SunIntensity = 7.0f;
    Morning.FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    Morning.FogDensity = 0.025f;
    Morning.FogHeightFalloff = 0.2f;
    Morning.AmbientBrightness = 0.7f;
    TimeOfDayPresets.Add(Morning);

    // Midday (12:00 PM) — 0.5
    FLight_TimeOfDayConfig Midday;
    Midday.TimeOfDay = ELight_TimeOfDay::Midday;
    Midday.SunPitchDegrees = -80.0f;
    Midday.SunYawDegrees = 180.0f;
    Midday.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
    Midday.SunIntensity = 10.0f;
    Midday.FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
    Midday.FogDensity = 0.015f;
    Midday.FogHeightFalloff = 0.25f;
    Midday.AmbientBrightness = 1.0f;
    TimeOfDayPresets.Add(Midday);

    // Golden Hour (5:30 PM) — 0.729
    FLight_TimeOfDayConfig GoldenHour;
    GoldenHour.TimeOfDay = ELight_TimeOfDay::GoldenHour;
    GoldenHour.SunPitchDegrees = -15.0f;
    GoldenHour.SunYawDegrees = 260.0f;
    GoldenHour.SunColor = FLinearColor(1.0f, 0.6f, 0.15f, 1.0f);
    GoldenHour.SunIntensity = 5.0f;
    GoldenHour.FogColor = FLinearColor(0.9f, 0.5f, 0.2f, 1.0f);
    GoldenHour.FogDensity = 0.035f;
    GoldenHour.FogHeightFalloff = 0.12f;
    GoldenHour.AmbientBrightness = 0.6f;
    TimeOfDayPresets.Add(GoldenHour);

    // Dusk (7:30 PM) — 0.812
    FLight_TimeOfDayConfig Dusk;
    Dusk.TimeOfDay = ELight_TimeOfDay::Dusk;
    Dusk.SunPitchDegrees = -2.0f;
    Dusk.SunYawDegrees = 280.0f;
    Dusk.SunColor = FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);
    Dusk.SunIntensity = 1.5f;
    Dusk.FogColor = FLinearColor(0.6f, 0.3f, 0.4f, 1.0f);
    Dusk.FogDensity = 0.05f;
    Dusk.FogHeightFalloff = 0.1f;
    Dusk.AmbientBrightness = 0.2f;
    TimeOfDayPresets.Add(Dusk);

    // Night (10:00 PM) — 0.916
    FLight_TimeOfDayConfig Night;
    Night.TimeOfDay = ELight_TimeOfDay::Night;
    Night.SunPitchDegrees = 30.0f; // Below horizon
    Night.SunYawDegrees = 0.0f;
    Night.SunColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    Night.SunIntensity = 0.1f;
    Night.FogColor = FLinearColor(0.05f, 0.08f, 0.15f, 1.0f);
    Night.FogDensity = 0.06f;
    Night.FogHeightFalloff = 0.08f;
    Night.AmbientBrightness = 0.05f;
    TimeOfDayPresets.Add(Night);
}

void ADayNightCycleManager::AutoFindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (!SunLight)
    {
        TArray<AActor*> FoundLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
        if (FoundLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundLights[0]);
        }
    }

    if (!HeightFog)
    {
        TArray<AActor*> FoundFogs;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFogs);
        if (FoundFogs.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(FoundFogs[0]);
        }
    }

    if (!SkyLightActor)
    {
        TArray<AActor*> FoundSkyLights;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundSkyLights);
        if (FoundSkyLights.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundSkyLights[0]);
        }
    }
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDay() const
{
    float Hour = GetCurrentHour();

    if (Hour < 6.0f)  return ELight_TimeOfDay::Midnight;
    if (Hour < 8.0f)  return ELight_TimeOfDay::Dawn;
    if (Hour < 11.0f) return ELight_TimeOfDay::Morning;
    if (Hour < 14.0f) return ELight_TimeOfDay::Midday;
    if (Hour < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (Hour < 19.0f) return ELight_TimeOfDay::GoldenHour;
    if (Hour < 21.0f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}

float ADayNightCycleManager::GetCurrentHour() const
{
    return CurrentTimeNormalized * 24.0f;
}

void ADayNightCycleManager::SetTimeNormalized(float NewTime)
{
    CurrentTimeNormalized = FMath::Clamp(NewTime, 0.0f, 1.0f);
    ForceUpdateLighting();
}

void ADayNightCycleManager::SetTimeByHour(float Hour)
{
    SetTimeNormalized(FMath::Clamp(Hour / 24.0f, 0.0f, 1.0f));
}

void ADayNightCycleManager::SetCycleActive(bool bActive)
{
    bCycleActive = bActive;
}

FLight_TimeOfDayConfig ADayNightCycleManager::InterpolateConfigs(
    const FLight_TimeOfDayConfig& A,
    const FLight_TimeOfDayConfig& B,
    float Alpha) const
{
    FLight_TimeOfDayConfig Result;
    Result.SunPitchDegrees = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
    Result.SunYawDegrees = FMath::Lerp(A.SunYawDegrees, B.SunYawDegrees, Alpha);
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.AmbientBrightness = FMath::Lerp(A.AmbientBrightness, B.AmbientBrightness, Alpha);
    return Result;
}

void ADayNightCycleManager::UpdateSunPosition(float DeltaTime)
{
    if (!SunLight || TimeOfDayPresets.Num() < 2) return;

    float Hour = GetCurrentHour();

    // Find surrounding presets based on normalized time
    // Simple linear search through presets
    FLight_TimeOfDayConfig CurrentConfig;
    bool bFound = false;

    // Map presets to approximate hours
    TArray<float> PresetHours = {5.0f, 9.0f, 12.0f, 17.5f, 19.5f, 22.0f};

    for (int32 i = 0; i < PresetHours.Num() - 1 && i < TimeOfDayPresets.Num() - 1; ++i)
    {
        if (Hour >= PresetHours[i] && Hour < PresetHours[i + 1])
        {
            float Alpha = (Hour - PresetHours[i]) / (PresetHours[i + 1] - PresetHours[i]);
            CurrentConfig = InterpolateConfigs(TimeOfDayPresets[i], TimeOfDayPresets[i + 1], Alpha);
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        // Use last preset for night
        CurrentConfig = TimeOfDayPresets.Last();
    }

    // Apply to directional light
    UDirectionalLightComponent* DirComp = SunLight->GetComponent();
    if (DirComp)
    {
        SunLight->SetActorRotation(FRotator(CurrentConfig.SunPitchDegrees, CurrentConfig.SunYawDegrees, 0.0f));
        DirComp->SetLightColor(CurrentConfig.SunColor);
        DirComp->SetIntensity(CurrentConfig.SunIntensity);
    }
}

void ADayNightCycleManager::UpdateFogSettings()
{
    if (!HeightFog || TimeOfDayPresets.Num() < 2) return;

    float Hour = GetCurrentHour();
    TArray<float> PresetHours = {5.0f, 9.0f, 12.0f, 17.5f, 19.5f, 22.0f};

    FLight_TimeOfDayConfig CurrentConfig;
    bool bFound = false;

    for (int32 i = 0; i < PresetHours.Num() - 1 && i < TimeOfDayPresets.Num() - 1; ++i)
    {
        if (Hour >= PresetHours[i] && Hour < PresetHours[i + 1])
        {
            float Alpha = (Hour - PresetHours[i]) / (PresetHours[i + 1] - PresetHours[i]);
            CurrentConfig = InterpolateConfigs(TimeOfDayPresets[i], TimeOfDayPresets[i + 1], Alpha);
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        CurrentConfig = TimeOfDayPresets.Last();
    }

    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    if (FogComp)
    {
        FogComp->SetFogDensity(CurrentConfig.FogDensity);
        FogComp->SetFogHeightFalloff(CurrentConfig.FogHeightFalloff);
        FogComp->SetFogInscatteringColor(CurrentConfig.FogColor);
    }
}

void ADayNightCycleManager::UpdateSkyLight()
{
    if (!SkyLightActor) return;

    USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
    if (SkyComp)
    {
        // Recapture sky every few ticks for accurate ambient
        SkyComp->RecaptureSky();
    }
}

void ADayNightCycleManager::ForceUpdateLighting()
{
    AutoFindLightingActors();
    UpdateSunPosition(0.0f);
    UpdateFogSettings();
    UpdateSkyLight();
}
