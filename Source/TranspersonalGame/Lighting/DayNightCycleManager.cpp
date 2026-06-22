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
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth transitions
    InitializeDefaultPresets();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find sun and sky if not set
    if (!SunLight)
    {
        TArray<AActor*> Lights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), Lights);
        if (Lights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(Lights[0]);
        }
    }

    if (!SkyLightActor)
    {
        TArray<AActor*> SkyLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), SkyLights);
        if (SkyLights.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(SkyLights[0]);
        }
    }

    SetTimeOfDay(CurrentHour);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive) return;

    // Advance time
    float HoursPerSecond = TimeScale / 3600.0f;
    CurrentHour += DeltaTime * HoursPerSecond;
    if (CurrentHour >= 24.0f) CurrentHour -= 24.0f;

    UpdateSunPosition(CurrentHour);
}

void ADayNightCycleManager::SetTimeOfDay(float Hour)
{
    CurrentHour = FMath::Clamp(Hour, 0.0f, 24.0f);
    UpdateSunPosition(CurrentHour);
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentHour >= 5.0f && CurrentHour < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (CurrentHour >= 7.0f && CurrentHour < 10.0f)  return ELight_TimeOfDay::Morning;
    if (CurrentHour >= 10.0f && CurrentHour < 14.0f) return ELight_TimeOfDay::Midday;
    if (CurrentHour >= 14.0f && CurrentHour < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentHour >= 17.0f && CurrentHour < 20.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentHour >= 20.0f && CurrentHour < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

void ADayNightCycleManager::UpdateSunPosition(float Hour)
{
    if (!SunLight) return;

    UDirectionalLightComponent* LightComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (!LightComp) return;

    // Sun arc: rises at hour 6, sets at hour 18
    // Pitch: -90 at noon (straight down), +90 at midnight (below horizon)
    float SunProgress = (Hour - 6.0f) / 12.0f; // 0 at sunrise, 1 at sunset
    float SunPitch = -90.0f + (SunProgress * 180.0f); // -90 (noon) to +90 (midnight)
    // Clamp so sun doesn't go too far below horizon for visual
    SunPitch = FMath::Clamp(SunPitch, -85.0f, 85.0f);

    float SunYaw = 45.0f + (Hour / 24.0f) * 30.0f; // Slight yaw drift

    SunLight->SetActorRotation(FRotator(SunPitch, SunYaw, 0.0f));

    // Intensity based on sun height
    float SunHeight = FMath::Sin(FMath::DegreesToRadians(-SunPitch));
    float Intensity = FMath::Max(0.0f, SunHeight) * 10.0f;

    // Dawn/dusk warm glow
    FLinearColor SunColor = FLinearColor::White;
    if (Hour >= 5.0f && Hour < 8.0f)
    {
        // Dawn — warm orange
        float T = (Hour - 5.0f) / 3.0f;
        SunColor = FMath::Lerp(FLinearColor(1.0f, 0.4f, 0.1f, 1.0f), FLinearColor(1.0f, 0.95f, 0.85f, 1.0f), T);
        Intensity = FMath::Max(Intensity, 2.0f);
    }
    else if (Hour >= 17.0f && Hour < 20.0f)
    {
        // Dusk — deep orange/red
        float T = (Hour - 17.0f) / 3.0f;
        SunColor = FMath::Lerp(FLinearColor(1.0f, 0.95f, 0.85f, 1.0f), FLinearColor(1.0f, 0.3f, 0.05f, 1.0f), T);
        Intensity = FMath::Max(Intensity, 1.5f);
    }
    else if (Hour >= 20.0f || Hour < 5.0f)
    {
        // Night — dim moonlight blue
        SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
        Intensity = 0.3f;
    }

    LightComp->SetIntensity(Intensity);
    LightComp->SetLightColor(SunColor);

    // Update sky light intensity
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            float SkyIntensity = FMath::Lerp(0.1f, 1.0f, FMath::Max(0.0f, SunHeight));
            SkyComp->SetIntensity(SkyIntensity);
        }
    }
}

void ADayNightCycleManager::ApplyLightingPreset(const FLight_TimeOfDaySettings& Settings)
{
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Settings.SunPitch, Settings.SunYaw, 0.0f));
        UDirectionalLightComponent* Comp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (Comp)
        {
            Comp->SetIntensity(Settings.SunIntensity);
            Comp->SetLightColor(Settings.SunColor);
        }
    }
}

void ADayNightCycleManager::SetToDawn()    { SetTimeOfDay(6.0f); }
void ADayNightCycleManager::SetToMidday()  { SetTimeOfDay(12.0f); }
void ADayNightCycleManager::SetToDusk()    { SetTimeOfDay(18.0f); }
void ADayNightCycleManager::SetToNight()   { SetTimeOfDay(22.0f); }

void ADayNightCycleManager::InitializeDefaultPresets()
{
    FLight_TimeOfDaySettings Dawn;
    Dawn.SunPitch = -5.0f;
    Dawn.SunYaw = 90.0f;
    Dawn.SunIntensity = 2.0f;
    Dawn.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    Dawn.FogDensity = 0.04f;
    Dawn.FogColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
    Dawn.SkyLightIntensity = 0.3f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Dawn, Dawn);

    FLight_TimeOfDaySettings Midday;
    Midday.SunPitch = -75.0f;
    Midday.SunYaw = 45.0f;
    Midday.SunIntensity = 10.0f;
    Midday.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    Midday.FogDensity = 0.015f;
    Midday.FogColor = FLinearColor(0.5f, 0.65f, 0.85f, 1.0f);
    Midday.SkyLightIntensity = 1.0f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Midday, Midday);

    FLight_TimeOfDaySettings Dusk;
    Dusk.SunPitch = -8.0f;
    Dusk.SunYaw = 270.0f;
    Dusk.SunIntensity = 3.0f;
    Dusk.SunColor = FLinearColor(1.0f, 0.35f, 0.05f, 1.0f);
    Dusk.FogDensity = 0.035f;
    Dusk.FogColor = FLinearColor(0.7f, 0.4f, 0.2f, 1.0f);
    Dusk.SkyLightIntensity = 0.4f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Dusk, Dusk);

    FLight_TimeOfDaySettings Night;
    Night.SunPitch = 60.0f;
    Night.SunYaw = 180.0f;
    Night.SunIntensity = 0.3f;
    Night.SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    Night.FogDensity = 0.03f;
    Night.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    Night.SkyLightIntensity = 0.05f;
    TimeOfDayPresets.Add(ELight_TimeOfDay::Night, Night);
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
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.ExposureBias = FMath::Lerp(A.ExposureBias, B.ExposureBias, Alpha);
    return Result;
}
