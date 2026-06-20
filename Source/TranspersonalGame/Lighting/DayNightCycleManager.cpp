#include "DayNightCycleManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Math/UnrealMathUtility.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCycleEnabled && SecondsPerGameHour > 0.0f)
    {
        float HoursPerSecond = 1.0f / SecondsPerGameHour;
        CurrentTimeOfDay += DeltaTime * HoursPerSecond;
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
        UpdateLighting(DeltaTime);
    }
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 10.0f)  return ELight_TimeOfDay::Morning;
    if (CurrentTimeOfDay >= 10.0f && CurrentTimeOfDay < 14.0f) return ELight_TimeOfDay::Midday;
    if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay < 20.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentTimeOfDay >= 20.0f && CurrentTimeOfDay < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

float ADayNightCycleManager::GetNormalizedTimeOfDay() const
{
    return CurrentTimeOfDay / 24.0f;
}

void ADayNightCycleManager::SetTimeOfDay(float NewHour)
{
    CurrentTimeOfDay = FMath::Clamp(NewHour, 0.0f, 23.99f);
    UpdateLighting(0.0f);
}

FLight_TimeOfDaySettings ADayNightCycleManager::GetSettingsForTime(float Hour) const
{
    FLight_TimeOfDaySettings Settings;

    // Dawn (5-7)
    if (Hour >= 5.0f && Hour < 7.0f)
    {
        Settings.SunIntensity = 3.0f;
        Settings.SunRotation = FRotator(-10.0f, 90.0f, 0.0f);
        Settings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
        Settings.SkyLightIntensity = 0.5f;
        Settings.FogDensity = 0.05f;
        Settings.FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
        Settings.ExposureBias = 0.5f;
    }
    // Morning (7-10)
    else if (Hour >= 7.0f && Hour < 10.0f)
    {
        Settings.SunIntensity = 8.0f;
        Settings.SunRotation = FRotator(-30.0f, 60.0f, 0.0f);
        Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        Settings.SkyLightIntensity = 1.2f;
        Settings.FogDensity = 0.03f;
        Settings.FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
        Settings.ExposureBias = 0.8f;
    }
    // Midday (10-14)
    else if (Hour >= 10.0f && Hour < 14.0f)
    {
        Settings.SunIntensity = 10.0f;
        Settings.SunRotation = FRotator(-75.0f, 45.0f, 0.0f);
        Settings.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
        Settings.SkyLightIntensity = 1.5f;
        Settings.FogDensity = 0.01f;
        Settings.FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
        Settings.ExposureBias = 1.0f;
    }
    // Afternoon (14-17)
    else if (Hour >= 14.0f && Hour < 17.0f)
    {
        Settings.SunIntensity = 9.0f;
        Settings.SunRotation = FRotator(-45.0f, -30.0f, 0.0f);
        Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.75f, 1.0f);
        Settings.SkyLightIntensity = 1.3f;
        Settings.FogDensity = 0.015f;
        Settings.FogColor = FLinearColor(0.55f, 0.65f, 0.75f, 1.0f);
        Settings.ExposureBias = 0.9f;
    }
    // Dusk (17-20)
    else if (Hour >= 17.0f && Hour < 20.0f)
    {
        Settings.SunIntensity = 4.0f;
        Settings.SunRotation = FRotator(-8.0f, -60.0f, 0.0f);
        Settings.SunColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
        Settings.SkyLightIntensity = 0.6f;
        Settings.FogDensity = 0.04f;
        Settings.FogColor = FLinearColor(0.9f, 0.4f, 0.2f, 1.0f);
        Settings.ExposureBias = 0.4f;
    }
    // Night (20-5)
    else
    {
        Settings.SunIntensity = 0.1f;
        Settings.SunRotation = FRotator(-5.0f, 180.0f, 0.0f);
        Settings.SunColor = FLinearColor(0.3f, 0.35f, 0.5f, 1.0f);
        Settings.SkyLightIntensity = 0.15f;
        Settings.FogDensity = 0.06f;
        Settings.FogColor = FLinearColor(0.1f, 0.12f, 0.2f, 1.0f);
        Settings.ExposureBias = -1.0f;
    }

    return Settings;
}

void ADayNightCycleManager::UpdateLighting(float DeltaTime)
{
    FLight_TimeOfDaySettings CurrentSettings = GetSettingsForTime(CurrentTimeOfDay);
    ApplyLightingSettings(CurrentSettings);
}

void ADayNightCycleManager::ApplyLightingSettings(const FLight_TimeOfDaySettings& Settings)
{
    // Apply to Sun (DirectionalLight)
    if (SunActor)
    {
        UDirectionalLightComponent* DirLight = SunActor->FindComponentByClass<UDirectionalLightComponent>();
        if (DirLight)
        {
            DirLight->SetIntensity(Settings.SunIntensity);
            DirLight->SetLightColor(Settings.SunColor);
            SunActor->SetActorRotation(Settings.SunRotation);
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SkyLight = SkyLightActor->FindComponentByClass<USkyLightComponent>();
        if (SkyLight)
        {
            SkyLight->SetIntensity(Settings.SkyLightIntensity);
        }
    }

    // Apply to Fog
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->FindComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }
}

FLight_TimeOfDaySettings ADayNightCycleManager::LerpSettings(
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
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.ExposureBias = FMath::Lerp(A.ExposureBias, B.ExposureBias, Alpha);
    return Result;
}
