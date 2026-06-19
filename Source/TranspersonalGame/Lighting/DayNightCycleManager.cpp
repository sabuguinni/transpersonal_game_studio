#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for performance
    CurrentPeriod = ELight_TimeOfDay::Morning;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find sun and fog if not set
    if (!SunLight)
    {
        TArray<AActor*> FoundLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundLights);
        if (FoundLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundLights[0]);
        }
    }

    if (!HeightFog)
    {
        TArray<AActor*> FoundFog;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundFog);
        if (FoundFog.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(FoundFog[0]);
        }
    }

    ApplyCurrentLighting();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleEnabled) return;

    ElapsedTime += DeltaTime * TimeScale;
    if (DayDurationSeconds > 0.0f)
    {
        CurrentTimeOfDay = FMath::Fmod(ElapsedTime / DayDurationSeconds, 1.0f);
    }

    CurrentPeriod = ClassifyTimeOfDay(CurrentTimeOfDay);
    ApplyCurrentLighting();
}

void ADayNightCycleManager::SetTimeOfDay(float NormalizedTime)
{
    CurrentTimeOfDay = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    ElapsedTime = CurrentTimeOfDay * DayDurationSeconds;
    CurrentPeriod = ClassifyTimeOfDay(CurrentTimeOfDay);
    ApplyCurrentLighting();
}

FLight_TimeOfDaySettings ADayNightCycleManager::GetSettingsForTime(float T) const
{
    FLight_TimeOfDaySettings Settings;

    // T: 0.0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk, 1.0=midnight
    if (T < 0.2f) // Night → Dawn
    {
        float Alpha = T / 0.2f;
        Settings.SunIntensity = FMath::Lerp(0.1f, 2.0f, Alpha);
        Settings.SunColor = FMath::Lerp(FLinearColor(0.1f, 0.1f, 0.3f, 1.0f), FLinearColor(1.0f, 0.5f, 0.2f, 1.0f), Alpha);
        Settings.SunPitch = FMath::Lerp(-5.0f, -20.0f, Alpha);
        Settings.FogDensity = FMath::Lerp(0.08f, 0.05f, Alpha);
        Settings.FogColor = FMath::Lerp(FLinearColor(0.05f, 0.05f, 0.15f, 1.0f), FLinearColor(0.8f, 0.4f, 0.2f, 1.0f), Alpha);
        Settings.AmbientIntensity = FMath::Lerp(0.1f, 0.4f, Alpha);
    }
    else if (T < 0.35f) // Dawn → Morning
    {
        float Alpha = (T - 0.2f) / 0.15f;
        Settings.SunIntensity = FMath::Lerp(2.0f, 8.0f, Alpha);
        Settings.SunColor = FMath::Lerp(FLinearColor(1.0f, 0.5f, 0.2f, 1.0f), FLinearColor(1.0f, 0.95f, 0.8f, 1.0f), Alpha);
        Settings.SunPitch = FMath::Lerp(-20.0f, -40.0f, Alpha);
        Settings.FogDensity = FMath::Lerp(0.05f, 0.02f, Alpha);
        Settings.FogColor = FMath::Lerp(FLinearColor(0.8f, 0.4f, 0.2f, 1.0f), FLinearColor(0.6f, 0.7f, 0.9f, 1.0f), Alpha);
        Settings.AmbientIntensity = FMath::Lerp(0.4f, 0.7f, Alpha);
    }
    else if (T < 0.65f) // Morning → Afternoon (midday)
    {
        float Alpha = (T - 0.35f) / 0.30f;
        Settings.SunIntensity = FMath::Lerp(8.0f, 12.0f, FMath::Sin(Alpha * PI));
        Settings.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
        Settings.SunPitch = FMath::Lerp(-40.0f, -85.0f, FMath::Sin(Alpha * PI * 0.5f));
        Settings.FogDensity = 0.015f;
        Settings.FogColor = FLinearColor(0.5f, 0.65f, 0.9f, 1.0f);
        Settings.AmbientIntensity = 0.8f;
    }
    else if (T < 0.8f) // Afternoon → Dusk
    {
        float Alpha = (T - 0.65f) / 0.15f;
        Settings.SunIntensity = FMath::Lerp(10.0f, 3.0f, Alpha);
        Settings.SunColor = FMath::Lerp(FLinearColor(1.0f, 0.9f, 0.7f, 1.0f), FLinearColor(1.0f, 0.4f, 0.1f, 1.0f), Alpha);
        Settings.SunPitch = FMath::Lerp(-45.0f, -10.0f, Alpha);
        Settings.FogDensity = FMath::Lerp(0.02f, 0.06f, Alpha);
        Settings.FogColor = FMath::Lerp(FLinearColor(0.5f, 0.65f, 0.9f, 1.0f), FLinearColor(0.9f, 0.3f, 0.1f, 1.0f), Alpha);
        Settings.AmbientIntensity = FMath::Lerp(0.7f, 0.3f, Alpha);
    }
    else // Dusk → Night
    {
        float Alpha = (T - 0.8f) / 0.2f;
        Settings.SunIntensity = FMath::Lerp(3.0f, 0.1f, Alpha);
        Settings.SunColor = FMath::Lerp(FLinearColor(1.0f, 0.4f, 0.1f, 1.0f), FLinearColor(0.1f, 0.1f, 0.3f, 1.0f), Alpha);
        Settings.SunPitch = FMath::Lerp(-10.0f, -5.0f, Alpha);
        Settings.FogDensity = FMath::Lerp(0.06f, 0.08f, Alpha);
        Settings.FogColor = FMath::Lerp(FLinearColor(0.9f, 0.3f, 0.1f, 1.0f), FLinearColor(0.05f, 0.05f, 0.15f, 1.0f), Alpha);
        Settings.AmbientIntensity = FMath::Lerp(0.3f, 0.1f, Alpha);
    }

    return Settings;
}

void ADayNightCycleManager::ApplyCurrentLighting()
{
    FLight_TimeOfDaySettings Settings = GetSettingsForTime(CurrentTimeOfDay);
    UpdateSunPosition(CurrentTimeOfDay);
    UpdateFog(Settings);
}

void ADayNightCycleManager::UpdateSunPosition(float T)
{
    if (!SunLight) return;

    FLight_TimeOfDaySettings Settings = GetSettingsForTime(T);

    // CRITICAL: pitch MUST be negative (memory hugo_sun_pitch_negative_proven)
    float Pitch = FMath::Clamp(Settings.SunPitch, -89.0f, -1.0f);
    float Yaw = T * 360.0f; // Sun orbits over the day

    SunLight->SetActorRotation(FRotator(Pitch, Yaw, 0.0f));

    UDirectionalLightComponent* LightComp = SunLight->GetComponent();
    if (LightComp)
    {
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
    }
}

void ADayNightCycleManager::UpdateFog(const FLight_TimeOfDaySettings& Settings)
{
    if (!HeightFog) return;

    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    if (FogComp)
    {
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
    }
}

void ADayNightCycleManager::UpdateSkyLight()
{
    // SkyLight auto-captures from Lumen — no manual update needed
}

ELight_TimeOfDay ADayNightCycleManager::ClassifyTimeOfDay(float T) const
{
    if (T < 0.1f)  return ELight_TimeOfDay::Midnight;
    if (T < 0.25f) return ELight_TimeOfDay::Night;
    if (T < 0.35f) return ELight_TimeOfDay::Dawn;
    if (T < 0.45f) return ELight_TimeOfDay::Morning;
    if (T < 0.6f)  return ELight_TimeOfDay::Midday;
    if (T < 0.75f) return ELight_TimeOfDay::Afternoon;
    if (T < 0.85f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}
