#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// Lighting & Atmosphere Agent #08 — Day/Night Cycle Manager Implementation
// Drives the full 24-hour Cretaceous day/night cycle with dynamic sun, moon,
// fog, and sky atmosphere transitions.

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth transitions
    InitializeDefaultPhaseSettings();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find sun and sky if not assigned
    if (!SunLight)
    {
        TArray<AActor*> FoundLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundLights);
        if (FoundLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundLights[0]);
        }
    }

    if (!SkyLightActor)
    {
        TArray<AActor*> FoundSkyLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundSkyLights);
        if (FoundSkyLights.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundSkyLights[0]);
        }
    }

    // Apply initial time
    SetTimeOfDay(CurrentTimeOfDay);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive) return;

    // Advance time
    AccumulatedTime += DeltaTime;
    float HoursPerSecond = TimeScale / 3600.0f;
    CurrentTimeOfDay += HoursPerSecond * DeltaTime;

    // Wrap at 24 hours
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    // Update phase
    CurrentPhase = GetPhaseForTime(CurrentTimeOfDay);

    // Update lighting every tick
    UpdateSunPosition(CurrentTimeOfDay);
    UpdateFogSettings(CurrentTimeOfDay);
    UpdateSkyLight(CurrentTimeOfDay);
}

void ADayNightCycleManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    CurrentPhase = GetPhaseForTime(CurrentTimeOfDay);
    UpdateSunPosition(CurrentTimeOfDay);
    UpdateFogSettings(CurrentTimeOfDay);
    UpdateSkyLight(CurrentTimeOfDay);
}

FString ADayNightCycleManager::GetTimeString() const
{
    int32 Hours = FMath::FloorToInt(CurrentTimeOfDay);
    int32 Minutes = FMath::FloorToInt((CurrentTimeOfDay - Hours) * 60.0f);
    return FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);
}

void ADayNightCycleManager::ApplyCurrentTimeInEditor()
{
    SetTimeOfDay(CurrentTimeOfDay);
}

void ADayNightCycleManager::UpdateSunPosition(float TimeHours)
{
    if (!SunLight) return;

    // Map 0-24h to sun arc: rises at 6h, sets at 18h
    // Pitch: -90 at noon (straight down), +90 at midnight (below horizon)
    float SunAngle = (TimeHours - 6.0f) / 12.0f * 180.0f; // 0-180 degrees over the day
    float SunPitch = -90.0f + SunAngle; // -90 at noon, 0 at sunrise/sunset, +90 at midnight

    // Yaw rotates sun from east to west
    float SunYaw = FMath::Lerp(-90.0f, 90.0f, (TimeHours - 6.0f) / 12.0f);

    SunLight->SetActorRotation(FRotator(SunPitch, SunYaw, 0.0f));

    // Sun color and intensity based on time
    UDirectionalLightComponent* DLComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (!DLComp) return;

    float Intensity = 0.0f;
    FLinearColor SunColor = FLinearColor::Black;

    if (TimeHours >= 6.0f && TimeHours < 8.0f)
    {
        // Dawn: soft pink/orange
        float T = (TimeHours - 6.0f) / 2.0f;
        Intensity = FMath::Lerp(0.5f, 5.0f, T);
        SunColor = FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.4f, 0.2f, 1.0f), FLinearColor(1.0f, 0.85f, 0.6f, 1.0f), T);
    }
    else if (TimeHours >= 8.0f && TimeHours < 11.0f)
    {
        // Morning: warm yellow
        float T = (TimeHours - 8.0f) / 3.0f;
        Intensity = FMath::Lerp(5.0f, 10.0f, T);
        SunColor = FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.85f, 0.6f, 1.0f), FLinearColor(1.0f, 0.95f, 0.85f, 1.0f), T);
    }
    else if (TimeHours >= 11.0f && TimeHours < 14.0f)
    {
        // Midday: bright white
        Intensity = 12.0f;
        SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    }
    else if (TimeHours >= 14.0f && TimeHours < 17.0f)
    {
        // Afternoon: slightly warm
        float T = (TimeHours - 14.0f) / 3.0f;
        Intensity = FMath::Lerp(10.0f, 8.0f, T);
        SunColor = FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.98f, 0.92f, 1.0f), FLinearColor(1.0f, 0.88f, 0.65f, 1.0f), T);
    }
    else if (TimeHours >= 17.0f && TimeHours < 19.0f)
    {
        // Golden hour: deep amber/orange
        float T = (TimeHours - 17.0f) / 2.0f;
        Intensity = FMath::Lerp(8.0f, 3.0f, T);
        SunColor = FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.88f, 0.65f, 1.0f), FLinearColor(1.0f, 0.35f, 0.1f, 1.0f), T);
    }
    else if (TimeHours >= 19.0f && TimeHours < 20.0f)
    {
        // Dusk: deep red/purple
        float T = (TimeHours - 19.0f);
        Intensity = FMath::Lerp(3.0f, 0.2f, T);
        SunColor = FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.35f, 0.1f, 1.0f), FLinearColor(0.4f, 0.1f, 0.3f, 1.0f), T);
    }
    else
    {
        // Night: moon light (very dim blue-white)
        Intensity = 0.15f;
        SunColor = FLinearColor(0.5f, 0.6f, 0.9f, 1.0f);
    }

    DLComp->SetIntensity(Intensity);
    DLComp->SetLightColor(SunColor);
}

void ADayNightCycleManager::UpdateFogSettings(float TimeHours)
{
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() == 0) return;

    AExponentialHeightFog* FogActor = Cast<AExponentialHeightFog>(FogActors[0]);
    if (!FogActor) return;

    UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp) return;

    // Fog density varies by time: thicker at dawn/dusk, thinner at midday
    float FogDensity = 0.02f;
    FLinearColor FogColor = FLinearColor(0.72f, 0.82f, 0.95f, 1.0f);

    if (TimeHours >= 5.0f && TimeHours < 9.0f)
    {
        // Dawn mist: thick, cool blue
        float T = (TimeHours - 5.0f) / 4.0f;
        FogDensity = FMath::Lerp(0.08f, 0.03f, T);
        FogColor = FLinearColor::LerpUsingHSV(FLinearColor(0.6f, 0.7f, 0.9f, 1.0f), FLinearColor(0.85f, 0.88f, 0.92f, 1.0f), T);
    }
    else if (TimeHours >= 9.0f && TimeHours < 16.0f)
    {
        // Day: light haze
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.85f, 0.88f, 0.95f, 1.0f);
    }
    else if (TimeHours >= 16.0f && TimeHours < 20.0f)
    {
        // Golden hour/dusk: warm amber fog
        float T = (TimeHours - 16.0f) / 4.0f;
        FogDensity = FMath::Lerp(0.025f, 0.06f, T);
        FogColor = FLinearColor::LerpUsingHSV(FLinearColor(0.85f, 0.78f, 0.65f, 1.0f), FLinearColor(0.5f, 0.3f, 0.4f, 1.0f), T);
    }
    else
    {
        // Night: dark blue mist
        FogDensity = 0.05f;
        FogColor = FLinearColor(0.1f, 0.12f, 0.25f, 1.0f);
    }

    FogComp->SetFogDensity(FogDensity);
    FogComp->SetFogInscatteringColor(FogColor);
}

void ADayNightCycleManager::UpdateSkyLight(float TimeHours)
{
    if (!SkyLightActor) return;

    USkyLightComponent* SLComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
    if (!SLComp) return;

    float SkyIntensity = 1.8f;
    if (TimeHours >= 20.0f || TimeHours < 6.0f)
    {
        SkyIntensity = 0.3f; // Night
    }
    else if (TimeHours >= 6.0f && TimeHours < 8.0f)
    {
        SkyIntensity = FMath::Lerp(0.3f, 1.5f, (TimeHours - 6.0f) / 2.0f); // Dawn ramp up
    }
    else if (TimeHours >= 18.0f && TimeHours < 20.0f)
    {
        SkyIntensity = FMath::Lerp(1.5f, 0.3f, (TimeHours - 18.0f) / 2.0f); // Dusk ramp down
    }

    SLComp->SetIntensity(SkyIntensity);
}

ELight_TimeOfDay ADayNightCycleManager::GetPhaseForTime(float TimeHours) const
{
    if (TimeHours >= 5.0f && TimeHours < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (TimeHours >= 7.0f && TimeHours < 10.0f)  return ELight_TimeOfDay::Morning;
    if (TimeHours >= 10.0f && TimeHours < 13.0f) return ELight_TimeOfDay::Midday;
    if (TimeHours >= 13.0f && TimeHours < 16.0f) return ELight_TimeOfDay::Afternoon;
    if (TimeHours >= 16.0f && TimeHours < 18.0f) return ELight_TimeOfDay::GoldenHour;
    if (TimeHours >= 18.0f && TimeHours < 20.0f) return ELight_TimeOfDay::Dusk;
    if (TimeHours >= 20.0f && TimeHours < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
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
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.AmbientTemperature = FMath::Lerp(A.AmbientTemperature, B.AmbientTemperature, Alpha);
    return Result;
}

void ADayNightCycleManager::InitializeDefaultPhaseSettings()
{
    // Dawn
    FLight_TimeOfDaySettings Dawn;
    Dawn.SunPitch = -10.0f; Dawn.SunYaw = -90.0f;
    Dawn.SunIntensity = 2.0f; Dawn.SunColor = FLinearColor(1.0f, 0.5f, 0.3f, 1.0f);
    Dawn.FogDensity = 0.07f; Dawn.FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
    Dawn.SkyLightIntensity = 0.8f; Dawn.AmbientTemperature = 22.0f;
    PhaseSettings.Add(ELight_TimeOfDay::Dawn, Dawn);

    // Midday
    FLight_TimeOfDaySettings Midday;
    Midday.SunPitch = -85.0f; Midday.SunYaw = 0.0f;
    Midday.SunIntensity = 12.0f; Midday.SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    Midday.FogDensity = 0.02f; Midday.FogColor = FLinearColor(0.85f, 0.88f, 0.95f, 1.0f);
    Midday.SkyLightIntensity = 2.0f; Midday.AmbientTemperature = 35.0f;
    PhaseSettings.Add(ELight_TimeOfDay::Midday, Midday);

    // Golden Hour
    FLight_TimeOfDaySettings GoldenHour;
    GoldenHour.SunPitch = -22.0f; GoldenHour.SunYaw = 60.0f;
    GoldenHour.SunIntensity = 6.0f; GoldenHour.SunColor = FLinearColor(1.0f, 0.72f, 0.3f, 1.0f);
    GoldenHour.FogDensity = 0.04f; GoldenHour.FogColor = FLinearColor(0.9f, 0.75f, 0.55f, 1.0f);
    GoldenHour.SkyLightIntensity = 1.5f; GoldenHour.AmbientTemperature = 30.0f;
    PhaseSettings.Add(ELight_TimeOfDay::GoldenHour, GoldenHour);

    // Night
    FLight_TimeOfDaySettings Night;
    Night.SunPitch = 45.0f; Night.SunYaw = 0.0f;
    Night.SunIntensity = 0.15f; Night.SunColor = FLinearColor(0.5f, 0.6f, 0.9f, 1.0f);
    Night.FogDensity = 0.05f; Night.FogColor = FLinearColor(0.1f, 0.12f, 0.25f, 1.0f);
    Night.SkyLightIntensity = 0.3f; Night.AmbientTemperature = 18.0f;
    PhaseSettings.Add(ELight_TimeOfDay::Night, Night);
}
