#include "LightingAtmosphereManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"

ALightingAtmosphereManager::ALightingAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update every second for performance

    // Default dawn preset — cool blue-pink prehistoric dawn
    DawnPreset.SunPitchAngle = -5.0f;
    DawnPreset.SunIntensity = 2.0f;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DawnPreset.FogDensity = 0.08f;
    DawnPreset.FogColor = FLinearColor(0.7f, 0.65f, 0.8f, 1.0f);
    DawnPreset.SkyLightIntensity = 0.8f;

    // Default noon preset — harsh tropical midday
    NoonPreset.SunPitchAngle = -80.0f;
    NoonPreset.SunIntensity = 12.0f;
    NoonPreset.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    NoonPreset.FogDensity = 0.02f;
    NoonPreset.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    NoonPreset.SkyLightIntensity = 2.0f;

    // Default dusk preset — warm amber prehistoric sunset
    DuskPreset.SunPitchAngle = -8.0f;
    DuskPreset.SunIntensity = 3.0f;
    DuskPreset.SunColor = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);
    DuskPreset.FogDensity = 0.06f;
    DuskPreset.FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    DuskPreset.SkyLightIntensity = 0.6f;

    // Default night preset — moonlit prehistoric darkness
    NightPreset.SunPitchAngle = 30.0f;
    NightPreset.SunIntensity = 0.3f;
    NightPreset.SunColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    NightPreset.FogDensity = 0.05f;
    NightPreset.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    NightPreset.SkyLightIntensity = 0.2f;
}

void ALightingAtmosphereManager::BeginPlay()
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

    // Apply initial time of day
    SetTimeOfDay(HourToTimeOfDay(CurrentHour));
}

void ALightingAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bEnableDayNightCycle) return;

    TimeAccumulator += DeltaTime;

    // Advance time based on day duration
    float HoursPerSecond = 24.0f / FMath::Max(DayDurationSeconds, 1.0f);
    CurrentHour += HoursPerSecond * DeltaTime;

    if (CurrentHour >= 24.0f)
    {
        CurrentHour -= 24.0f;
    }

    UpdateSunPosition();

    ELight_TimeOfDay NewTimeOfDay = HourToTimeOfDay(CurrentHour);
    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
    }
}

void ALightingAtmosphereManager::SetTimeOfDay(ELight_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;

    FLight_TimeOfDaySettings TargetSettings;

    switch (NewTime)
    {
    case ELight_TimeOfDay::Dawn:
        TargetSettings = DawnPreset;
        break;
    case ELight_TimeOfDay::Morning:
        TargetSettings = InterpolatePresets(DawnPreset, NoonPreset, 0.3f);
        break;
    case ELight_TimeOfDay::Noon:
        TargetSettings = NoonPreset;
        break;
    case ELight_TimeOfDay::Afternoon:
        TargetSettings = InterpolatePresets(NoonPreset, DuskPreset, 0.5f);
        break;
    case ELight_TimeOfDay::Dusk:
        TargetSettings = DuskPreset;
        break;
    case ELight_TimeOfDay::Night:
        TargetSettings = InterpolatePresets(DuskPreset, NightPreset, 0.5f);
        break;
    case ELight_TimeOfDay::Midnight:
        TargetSettings = NightPreset;
        break;
    default:
        TargetSettings = NoonPreset;
        break;
    }

    ApplyLightingPreset(TargetSettings);
}

void ALightingAtmosphereManager::SetWeatherState(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
    UpdateFogForWeather();
}

void ALightingAtmosphereManager::ApplyLightingPreset(const FLight_TimeOfDaySettings& Settings)
{
    if (!SunLight) return;

    // Apply sun rotation
    FRotator CurrentRot = SunLight->GetActorRotation();
    SunLight->SetActorRotation(FRotator(Settings.SunPitchAngle, CurrentRot.Yaw, CurrentRot.Roll));

    // Apply sun light properties
    UDirectionalLightComponent* LightComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (LightComp)
    {
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
    }
}

void ALightingAtmosphereManager::AdvanceTime(float DeltaHours)
{
    CurrentHour = FMath::Fmod(CurrentHour + DeltaHours, 24.0f);
    if (CurrentHour < 0.0f) CurrentHour += 24.0f;

    UpdateSunPosition();
    CurrentTimeOfDay = HourToTimeOfDay(CurrentHour);
}

void ALightingAtmosphereManager::UpdateSunPosition()
{
    if (!SunLight) return;

    // Map hour to sun pitch: 6am = horizon (-5°), 12pm = zenith (-80°), 18pm = horizon (-5°), midnight = below horizon (30°)
    float NormalizedHour = CurrentHour / 24.0f;
    float SunAngle = 0.0f;

    if (CurrentHour >= 6.0f && CurrentHour <= 18.0f)
    {
        // Daytime arc: pitch from -5 at dawn to -80 at noon back to -5 at dusk
        float DayProgress = (CurrentHour - 6.0f) / 12.0f;
        float SinValue = FMath::Sin(DayProgress * PI);
        SunAngle = FMath::Lerp(-5.0f, -80.0f, SinValue);
    }
    else
    {
        // Nighttime: sun below horizon
        SunAngle = 30.0f;
    }

    FRotator CurrentRot = SunLight->GetActorRotation();
    SunLight->SetActorRotation(FRotator(SunAngle, CurrentRot.Yaw, CurrentRot.Roll));

    // Interpolate color based on time
    FLinearColor DayColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    FLinearColor DawnDuskColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    FLinearColor NightColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);

    FLinearColor TargetColor;
    float TargetIntensity;

    if (CurrentHour >= 6.0f && CurrentHour <= 18.0f)
    {
        float DayProgress = (CurrentHour - 6.0f) / 12.0f;
        float MidnessAlpha = FMath::Sin(DayProgress * PI);
        TargetColor = FLinearColor::LerpUsingHSV(DawnDuskColor, DayColor, MidnessAlpha);
        TargetIntensity = FMath::Lerp(2.0f, 12.0f, MidnessAlpha);
    }
    else
    {
        TargetColor = NightColor;
        TargetIntensity = 0.3f;
    }

    UDirectionalLightComponent* LightComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (LightComp)
    {
        LightComp->SetLightColor(TargetColor);
        LightComp->SetIntensity(TargetIntensity);
    }
}

void ALightingAtmosphereManager::UpdateFogForWeather()
{
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FogActors);

    for (AActor* Actor : FogActors)
    {
        UExponentialHeightFogComponent* FogComp = Actor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (!FogComp) continue;

        switch (CurrentWeather)
        {
        case ELight_WeatherState::Clear:
            FogComp->SetFogDensity(0.02f);
            FogComp->SetFogInscatteringColor(FLinearColor(0.6f, 0.75f, 0.9f, 1.0f));
            break;
        case ELight_WeatherState::Overcast:
            FogComp->SetFogDensity(0.05f);
            FogComp->SetFogInscatteringColor(FLinearColor(0.5f, 0.55f, 0.6f, 1.0f));
            break;
        case ELight_WeatherState::Foggy:
            FogComp->SetFogDensity(0.12f);
            FogComp->SetFogInscatteringColor(FLinearColor(0.7f, 0.72f, 0.75f, 1.0f));
            break;
        case ELight_WeatherState::Stormy:
            FogComp->SetFogDensity(0.08f);
            FogComp->SetFogInscatteringColor(FLinearColor(0.3f, 0.35f, 0.4f, 1.0f));
            break;
        case ELight_WeatherState::Rain:
            FogComp->SetFogDensity(0.06f);
            FogComp->SetFogInscatteringColor(FLinearColor(0.4f, 0.5f, 0.6f, 1.0f));
            break;
        default:
            break;
        }
        break; // Only update first fog actor
    }
}

FLight_TimeOfDaySettings ALightingAtmosphereManager::InterpolatePresets(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha)
{
    FLight_TimeOfDaySettings Result;
    Result.SunPitchAngle = FMath::Lerp(A.SunPitchAngle, B.SunPitchAngle, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    return Result;
}

ELight_TimeOfDay ALightingAtmosphereManager::HourToTimeOfDay(float Hour) const
{
    if (Hour >= 5.0f && Hour < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (Hour >= 7.0f && Hour < 11.0f)  return ELight_TimeOfDay::Morning;
    if (Hour >= 11.0f && Hour < 14.0f) return ELight_TimeOfDay::Noon;
    if (Hour >= 14.0f && Hour < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (Hour >= 17.0f && Hour < 20.0f) return ELight_TimeOfDay::Dusk;
    if (Hour >= 20.0f && Hour < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}
