#include "DayNightCycleManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth cycle
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find sun if not set
    if (!SunLight)
    {
        TArray<AActor*> FoundLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundLights);
        if (FoundLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundLights[0]);
        }
    }

    // Auto-find sky light if not set
    if (!SkyLightActor)
    {
        TArray<AActor*> FoundSkyLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundSkyLights);
        if (FoundSkyLights.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundSkyLights[0]);
        }
    }

    // Apply initial time settings
    ApplyCurrentTimeSettings();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive)
    {
        return;
    }

    // Advance time
    ElapsedSeconds += DeltaTime;
    float HoursPerSecond = TimeSpeedMultiplier / 3600.0f;
    CurrentTimeHours += HoursPerSecond * DeltaTime;

    // Wrap around 24 hours
    if (CurrentTimeHours >= 24.0f)
    {
        CurrentTimeHours -= 24.0f;
    }

    // Update time of day enum
    ELight_TimeOfDay NewTimeOfDay = ClassifyTimeOfDay(CurrentTimeHours);
    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
    }

    // Apply lighting every tick (smooth interpolation handled by settings)
    UpdateSunPosition(CurrentTimeHours);
}

void ADayNightCycleManager::SetTimeOfDay(float NewTimeHours)
{
    CurrentTimeHours = FMath::Clamp(NewTimeHours, 0.0f, 24.0f);
    CurrentTimeOfDay = ClassifyTimeOfDay(CurrentTimeHours);
    ApplyCurrentTimeSettings();
}

void ADayNightCycleManager::ApplyCurrentTimeSettings()
{
    FLight_TimeOfDaySettings Settings = GetSettingsForTime(CurrentTimeHours);
    UpdateSunPosition(CurrentTimeHours);
    UpdateFogSettings(Settings);
    UpdateSkyLight(Settings);
}

FLight_TimeOfDaySettings ADayNightCycleManager::GetSettingsForTime(float TimeHours) const
{
    FLight_TimeOfDaySettings Settings;

    // Dawn: 5:00 - 7:00
    if (TimeHours >= 5.0f && TimeHours < 7.0f)
    {
        float T = (TimeHours - 5.0f) / 2.0f;
        Settings.SunPitchDegrees = FMath::Lerp(-5.0f, -25.0f, T);
        Settings.SunIntensity = FMath::Lerp(1.5f, 6.0f, T);
        Settings.SunColor = FLinearColor(FMath::Lerp(1.0f, 1.0f, T), FMath::Lerp(0.4f, 0.75f, T), FMath::Lerp(0.1f, 0.4f, T), 1.0f);
        Settings.FogDensity = FMath::Lerp(0.08f, 0.04f, T);
        Settings.FogColor = FLinearColor(0.85f, 0.55f, 0.35f, 1.0f);
        Settings.SkyLightIntensity = FMath::Lerp(0.5f, 1.2f, T);
        Settings.AmbientTemperature = 18.0f;
    }
    // Morning: 7:00 - 10:00
    else if (TimeHours >= 7.0f && TimeHours < 10.0f)
    {
        float T = (TimeHours - 7.0f) / 3.0f;
        Settings.SunPitchDegrees = FMath::Lerp(-25.0f, -55.0f, T);
        Settings.SunIntensity = FMath::Lerp(6.0f, 10.0f, T);
        Settings.SunColor = FLinearColor(1.0f, FMath::Lerp(0.75f, 0.95f, T), FMath::Lerp(0.4f, 0.8f, T), 1.0f);
        Settings.FogDensity = FMath::Lerp(0.04f, 0.02f, T);
        Settings.FogColor = FLinearColor(0.75f, 0.85f, 0.95f, 1.0f);
        Settings.SkyLightIntensity = FMath::Lerp(1.2f, 2.0f, T);
        Settings.AmbientTemperature = 24.0f;
    }
    // Midday: 10:00 - 14:00
    else if (TimeHours >= 10.0f && TimeHours < 14.0f)
    {
        Settings.SunPitchDegrees = -72.0f;
        Settings.SunIntensity = 12.0f;
        Settings.SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
        Settings.FogDensity = 0.02f;
        Settings.FogColor = FLinearColor(0.72f, 0.82f, 0.95f, 1.0f);
        Settings.SkyLightIntensity = 2.2f;
        Settings.AmbientTemperature = 32.0f;
    }
    // Afternoon: 14:00 - 17:00
    else if (TimeHours >= 14.0f && TimeHours < 17.0f)
    {
        float T = (TimeHours - 14.0f) / 3.0f;
        Settings.SunPitchDegrees = FMath::Lerp(-72.0f, -30.0f, T);
        Settings.SunIntensity = FMath::Lerp(12.0f, 8.0f, T);
        Settings.SunColor = FLinearColor(1.0f, FMath::Lerp(0.97f, 0.85f, T), FMath::Lerp(0.88f, 0.55f, T), 1.0f);
        Settings.FogDensity = FMath::Lerp(0.02f, 0.04f, T);
        Settings.FogColor = FLinearColor(FMath::Lerp(0.72f, 0.9f, T), FMath::Lerp(0.82f, 0.7f, T), FMath::Lerp(0.95f, 0.5f, T), 1.0f);
        Settings.SkyLightIntensity = FMath::Lerp(2.2f, 1.8f, T);
        Settings.AmbientTemperature = 30.0f;
    }
    // Dusk: 17:00 - 20:00
    else if (TimeHours >= 17.0f && TimeHours < 20.0f)
    {
        float T = (TimeHours - 17.0f) / 3.0f;
        Settings.SunPitchDegrees = FMath::Lerp(-30.0f, -5.0f, T);
        Settings.SunIntensity = FMath::Lerp(8.0f, 2.0f, T);
        Settings.SunColor = FLinearColor(1.0f, FMath::Lerp(0.85f, 0.45f, T), FMath::Lerp(0.55f, 0.1f, T), 1.0f);
        Settings.FogDensity = FMath::Lerp(0.04f, 0.06f, T);
        Settings.FogColor = FLinearColor(FMath::Lerp(0.9f, 0.6f, T), FMath::Lerp(0.7f, 0.35f, T), FMath::Lerp(0.5f, 0.25f, T), 1.0f);
        Settings.SkyLightIntensity = FMath::Lerp(1.8f, 0.8f, T);
        Settings.AmbientTemperature = 22.0f;
    }
    // Night: 20:00 - 5:00
    else
    {
        Settings.SunPitchDegrees = 15.0f; // Below horizon
        Settings.SunIntensity = 0.1f;
        Settings.SunColor = FLinearColor(0.3f, 0.35f, 0.6f, 1.0f);
        Settings.FogDensity = 0.05f;
        Settings.FogColor = FLinearColor(0.05f, 0.07f, 0.15f, 1.0f);
        Settings.SkyLightIntensity = 0.3f;
        Settings.AmbientTemperature = 16.0f;
    }

    return Settings;
}

void ADayNightCycleManager::UpdateSunPosition(float TimeHours)
{
    if (!SunLight)
    {
        return;
    }

    FLight_TimeOfDaySettings Settings = GetSettingsForTime(TimeHours);

    // Enforce CAP: pitch must never exceed -30 degrees (too flat/bright)
    float SafePitch = FMath::Min(Settings.SunPitchDegrees, -30.0f);
    // Exception: night time sun goes above horizon
    if (TimeHours < 5.0f || TimeHours >= 20.0f)
    {
        SafePitch = Settings.SunPitchDegrees; // Allow positive pitch at night
    }

    SunLight->SetActorRotation(FRotator(SafePitch, Settings.SunYawDegrees, 0.0f));

    UDirectionalLightComponent* LightComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (LightComp)
    {
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
    }
}

void ADayNightCycleManager::UpdateFogSettings(const FLight_TimeOfDaySettings& Settings)
{
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FogActors);

    if (FogActors.Num() == 0)
    {
        return;
    }

    AExponentialHeightFog* Fog = Cast<AExponentialHeightFog>(FogActors[0]);
    if (!Fog)
    {
        return;
    }

    UExponentialHeightFogComponent* FogComp = Fog->GetComponentByClass<UExponentialHeightFogComponent>();
    if (FogComp)
    {
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
    }
}

void ADayNightCycleManager::UpdateSkyLight(const FLight_TimeOfDaySettings& Settings)
{
    if (!SkyLightActor)
    {
        return;
    }

    USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
    if (SkyComp)
    {
        SkyComp->SetIntensity(Settings.SkyLightIntensity);
    }
}

ELight_TimeOfDay ADayNightCycleManager::ClassifyTimeOfDay(float TimeHours) const
{
    if (TimeHours >= 5.0f && TimeHours < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (TimeHours >= 7.0f && TimeHours < 10.0f)  return ELight_TimeOfDay::Morning;
    if (TimeHours >= 10.0f && TimeHours < 14.0f) return ELight_TimeOfDay::Midday;
    if (TimeHours >= 14.0f && TimeHours < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (TimeHours >= 17.0f && TimeHours < 20.0f) return ELight_TimeOfDay::Dusk;
    if (TimeHours >= 20.0f && TimeHours < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::MidNight;
}
