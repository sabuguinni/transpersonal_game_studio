#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
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

    // Auto-find sun and sky light if not set
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
        TArray<AActor*> FoundSkies;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundSkies);
        if (FoundSkies.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundSkies[0]);
        }
    }

    // Apply initial time
    SetTimeOfDay(CurrentTimeHours);
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

    // Update all lighting systems
    UpdateSunPosition(CurrentTimeHours);
    UpdateLightColors(CurrentTimeHours);
    UpdateFog(CurrentTimeHours);

    CurrentTimeOfDay = TimeHoursToEnum(CurrentTimeHours);
}

void ADayNightCycleManager::SetTimeOfDay(float NewTimeHours)
{
    CurrentTimeHours = FMath::Clamp(NewTimeHours, 0.0f, 24.0f);
    UpdateSunPosition(CurrentTimeHours);
    UpdateLightColors(CurrentTimeHours);
    UpdateFog(CurrentTimeHours);
    CurrentTimeOfDay = TimeHoursToEnum(CurrentTimeHours);
}

void ADayNightCycleManager::UpdateSunPosition(float TimeHours)
{
    if (!SunLight) return;

    // Sun travels 360 degrees in 24 hours
    // At noon (12h) sun is at highest point (pitch = -90 = directly overhead)
    // At midnight sun is below horizon
    // Pitch: -45 = good afternoon angle (confirmed by screenshot memory)
    float SunAngle = ((TimeHours - 6.0f) / 12.0f) * 180.0f; // 0 at dawn, 180 at dusk
    float SunPitch = -SunAngle; // Negative pitch = illuminates ground (MANDATORY per memory)
    float SunYaw = 60.0f; // Fixed east-to-west direction

    SunLight->SetActorRotation(FRotator(SunPitch, SunYaw, 0.0f));
}

void ADayNightCycleManager::UpdateLightColors(float TimeHours)
{
    if (!SunLight) return;

    UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (!SunComp) return;

    // Color and intensity vary by time of day
    FLinearColor SunColor;
    float SunIntensity;

    if (TimeHours < 6.0f || TimeHours > 20.0f)
    {
        // Night — very dim, blue-grey
        SunIntensity = 0.1f;
        SunColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
    }
    else if (TimeHours < 8.0f)
    {
        // Dawn — warm orange-pink
        float T = (TimeHours - 6.0f) / 2.0f;
        SunIntensity = FMath::Lerp(0.5f, 8.0f, T);
        SunColor = FLinearColor(
            FMath::Lerp(1.0f, 1.0f, T),
            FMath::Lerp(0.4f, 0.85f, T),
            FMath::Lerp(0.2f, 0.65f, T),
            1.0f
        );
    }
    else if (TimeHours < 16.0f)
    {
        // Midday — bright white-yellow
        SunIntensity = 10.0f;
        SunColor = FLinearColor(1.0f, 0.92f, 0.78f, 1.0f);
    }
    else if (TimeHours < 20.0f)
    {
        // Dusk — deep orange-red
        float T = (TimeHours - 16.0f) / 4.0f;
        SunIntensity = FMath::Lerp(10.0f, 0.5f, T);
        SunColor = FLinearColor(
            1.0f,
            FMath::Lerp(0.85f, 0.3f, T),
            FMath::Lerp(0.6f, 0.1f, T),
            1.0f
        );
    }
    else
    {
        SunIntensity = 0.1f;
        SunColor = FLinearColor(0.05f, 0.05f, 0.15f, 1.0f);
    }

    SunComp->SetIntensity(SunIntensity);
    SunComp->SetLightColor(SunColor);

    // Update sky light intensity to match
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            float SkyIntensity = FMath::Clamp(SunIntensity * 0.15f, 0.05f, 1.5f);
            SkyComp->SetIntensity(SkyIntensity);
        }
    }
}

void ADayNightCycleManager::UpdateFog(float TimeHours)
{
    // Find fog actor in world
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FogActors);

    for (AActor* Actor : FogActors)
    {
        UExponentialHeightFogComponent* FogComp = Actor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (!FogComp) continue;

        // Dawn/dusk have more fog, midday less
        float FogDensity;
        FLinearColor FogColor;

        if (TimeHours < 6.0f || TimeHours > 20.0f)
        {
            // Night fog — dense blue
            FogDensity = 0.04f;
            FogColor = FLinearColor(0.1f, 0.1f, 0.25f, 1.0f);
        }
        else if (TimeHours < 9.0f || TimeHours > 17.0f)
        {
            // Dawn/dusk fog — warm haze
            FogDensity = 0.035f;
            FogColor = FLinearColor(0.6f, 0.4f, 0.25f, 1.0f);
        }
        else
        {
            // Midday — light atmospheric haze
            FogDensity = 0.015f;
            FogColor = FLinearColor(0.4f, 0.5f, 0.7f, 1.0f);
        }

        FogComp->SetFogDensity(FogDensity);
        FogComp->SetFogInscatteringColor(FogColor);
        break; // Only update first fog actor
    }
}

void ADayNightCycleManager::ApplyLightingPreset(const FLight_TimeOfDaySettings& Settings)
{
    if (SunLight)
    {
        UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(Settings.SunIntensity);
            SunComp->SetLightColor(Settings.SunColor);
        }
        // MANDATORY: pitch must be negative to illuminate ground
        float SafePitch = FMath::Min(Settings.SunPitch, -1.0f);
        SunLight->SetActorRotation(FRotator(SafePitch, 60.0f, 0.0f));
    }

    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Settings.SkyLightIntensity);
        }
    }
}

FLight_TimeOfDaySettings ADayNightCycleManager::GetSettingsForTime(float TimeHours) const
{
    ELight_TimeOfDay TOD = TimeHoursToEnum(TimeHours);
    if (TimeOfDayPresets.Contains(TOD))
    {
        return TimeOfDayPresets[TOD];
    }
    return FLight_TimeOfDaySettings();
}

ELight_TimeOfDay ADayNightCycleManager::TimeHoursToEnum(float TimeHours) const
{
    if (TimeHours >= 5.0f && TimeHours < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (TimeHours >= 7.0f && TimeHours < 11.0f)  return ELight_TimeOfDay::Morning;
    if (TimeHours >= 11.0f && TimeHours < 14.0f) return ELight_TimeOfDay::Midday;
    if (TimeHours >= 14.0f && TimeHours < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (TimeHours >= 17.0f && TimeHours < 20.0f) return ELight_TimeOfDay::Dusk;
    if (TimeHours >= 20.0f && TimeHours < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

void ADayNightCycleManager::InitializeDefaultPresets()
{
    // Dawn preset
    FLight_TimeOfDaySettings Dawn;
    Dawn.SunIntensity = 3.0f;
    Dawn.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    Dawn.SunPitch = -15.0f;
    Dawn.SkyLightIntensity = 0.4f;
    Dawn.FogDensity = 0.04f;
    Dawn.FogColor = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
    Dawn.AmbientTemperature = 18.0f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Dawn, Dawn);

    // Midday preset
    FLight_TimeOfDaySettings Midday;
    Midday.SunIntensity = 10.0f;
    Midday.SunColor = FLinearColor(1.0f, 0.92f, 0.78f, 1.0f);
    Midday.SunPitch = -75.0f;
    Midday.SkyLightIntensity = 1.5f;
    Midday.FogDensity = 0.015f;
    Midday.FogColor = FLinearColor(0.4f, 0.5f, 0.7f, 1.0f);
    Midday.AmbientTemperature = 32.0f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Midday, Midday);

    // Dusk preset
    FLight_TimeOfDaySettings Dusk;
    Dusk.SunIntensity = 4.0f;
    Dusk.SunColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
    Dusk.SunPitch = -10.0f;
    Dusk.SkyLightIntensity = 0.5f;
    Dusk.FogDensity = 0.035f;
    Dusk.FogColor = FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);
    Dusk.AmbientTemperature = 25.0f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Dusk, Dusk);

    // Night preset
    FLight_TimeOfDaySettings Night;
    Night.SunIntensity = 0.05f;
    Night.SunColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
    Night.SunPitch = -45.0f;
    Night.SkyLightIntensity = 0.1f;
    Night.FogDensity = 0.04f;
    Night.FogColor = FLinearColor(0.05f, 0.05f, 0.2f, 1.0f);
    Night.AmbientTemperature = 15.0f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Night, Night);
}
