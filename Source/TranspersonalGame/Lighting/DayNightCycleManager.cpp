#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default: start at 8:00 AM (morning)
    CurrentTimeOfDay = 8.0f;
    TimeScale = 60.0f; // 1 real minute = 1 game hour
    bCycleActive = true;

    CurrentTimeOfDayEnum = ELight_TimeOfDay::Morning;
    PreviousTimeOfDayEnum = ELight_TimeOfDay::Morning;
    TimeSinceLastSunriseCheck = 0.0f;

    // Dawn preset — warm pink/orange, low sun
    DawnSettings.SunColor = FLinearColor(1.0f, 0.55f, 0.3f, 1.0f);
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunPitchAngle = -5.0f;
    DawnSettings.FogColor = FLinearColor(0.9f, 0.6f, 0.5f, 1.0f);
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.AmbientIntensity = 0.5f;

    // Midday preset — bright white/yellow, high sun
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    MiddaySettings.SunIntensity = 10.0f;
    MiddaySettings.SunPitchAngle = -75.0f;
    MiddaySettings.FogColor = FLinearColor(0.78f, 0.85f, 0.92f, 1.0f);
    MiddaySettings.FogDensity = 0.01f;
    MiddaySettings.AmbientIntensity = 1.5f;

    // Dusk preset — deep amber/orange, low sun
    DuskSettings.SunColor = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);
    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunPitchAngle = -8.0f;
    DuskSettings.FogColor = FLinearColor(0.85f, 0.5f, 0.3f, 1.0f);
    DuskSettings.FogDensity = 0.05f;
    DuskSettings.AmbientIntensity = 0.4f;

    // Night preset — deep blue/indigo, moon light
    NightSettings.SunColor = FLinearColor(0.15f, 0.2f, 0.45f, 1.0f);
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunPitchAngle = -180.0f;
    NightSettings.FogColor = FLinearColor(0.05f, 0.05f, 0.15f, 1.0f);
    NightSettings.FogDensity = 0.03f;
    NightSettings.AmbientIntensity = 0.1f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find sun light if not set
    if (!SunLight)
    {
        TArray<AActor*> FoundLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundLights);
        if (FoundLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundLights[0]);
        }
    }

    ApplyLightingForCurrentTime();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive) return;

    UpdateSunPosition(DeltaTime);
    UpdateLightingColors();

    // Check for time of day transitions
    ELight_TimeOfDay NewTOD = ClassifyTimeOfDay(CurrentTimeOfDay);
    if (NewTOD != CurrentTimeOfDayEnum)
    {
        PreviousTimeOfDayEnum = CurrentTimeOfDayEnum;
        CurrentTimeOfDayEnum = NewTOD;
        OnTimeOfDayChanged(NewTOD);

        // Fire sunrise/sunset events
        if (NewTOD == ELight_TimeOfDay::Dawn)
        {
            OnSunrise();
        }
        else if (NewTOD == ELight_TimeOfDay::Dusk)
        {
            OnSunset();
        }
    }
}

void ADayNightCycleManager::UpdateSunPosition(float DeltaTime)
{
    // Advance time
    float HoursPerSecond = TimeScale / 3600.0f;
    CurrentTimeOfDay += DeltaTime * HoursPerSecond;

    // Wrap around 24 hours
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    if (!SunLight) return;

    // Map 0-24h to sun pitch angle
    // 6:00 = sunrise (pitch -5), 12:00 = noon (pitch -75), 18:00 = sunset (pitch -5), 0:00 = midnight (below horizon)
    float NormalizedTime = CurrentTimeOfDay / 24.0f;
    float SunAngle = NormalizedTime * 360.0f - 90.0f; // -90 at midnight, 90 at noon
    float SunPitch = -FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 75.0f;

    // Clamp: never go above -5 degrees (always casting downward shadows)
    SunPitch = FMath::Clamp(SunPitch, -85.0f, -5.0f);

    FRotator CurrentRot = SunLight->GetActorRotation();
    SunLight->SetActorRotation(FRotator(SunPitch, CurrentRot.Yaw, CurrentRot.Roll));
}

void ADayNightCycleManager::UpdateLightingColors()
{
    if (!SunLight) return;

    UDirectionalLightComponent* LightComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (!LightComp) return;

    // Determine which presets to blend between
    FLight_TimeOfDaySettings TargetSettings;

    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 8.0f)
    {
        // Dawn to Morning
        float Alpha = (CurrentTimeOfDay - 5.0f) / 3.0f;
        TargetSettings = InterpolateSettings(DawnSettings, MiddaySettings, Alpha * 0.5f);
    }
    else if (CurrentTimeOfDay >= 8.0f && CurrentTimeOfDay < 16.0f)
    {
        // Morning to Afternoon
        float Alpha = (CurrentTimeOfDay - 8.0f) / 8.0f;
        TargetSettings = InterpolateSettings(MiddaySettings, DuskSettings, Alpha * 0.3f);
    }
    else if (CurrentTimeOfDay >= 16.0f && CurrentTimeOfDay < 20.0f)
    {
        // Afternoon to Night
        float Alpha = (CurrentTimeOfDay - 16.0f) / 4.0f;
        TargetSettings = InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }
    else
    {
        // Night
        TargetSettings = NightSettings;
    }

    LightComp->SetIntensity(TargetSettings.SunIntensity);
    LightComp->SetLightColor(TargetSettings.SunColor);
}

ELight_TimeOfDay ADayNightCycleManager::ClassifyTimeOfDay(float TimeHours) const
{
    if (TimeHours >= 5.0f && TimeHours < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (TimeHours >= 7.0f && TimeHours < 11.0f)  return ELight_TimeOfDay::Morning;
    if (TimeHours >= 11.0f && TimeHours < 14.0f) return ELight_TimeOfDay::Midday;
    if (TimeHours >= 14.0f && TimeHours < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (TimeHours >= 17.0f && TimeHours < 20.0f) return ELight_TimeOfDay::Dusk;
    if (TimeHours >= 20.0f && TimeHours < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

FLight_TimeOfDaySettings ADayNightCycleManager::InterpolateSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunPitchAngle = FMath::Lerp(A.SunPitchAngle, B.SunPitchAngle, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.AmbientIntensity = FMath::Lerp(A.AmbientIntensity, B.AmbientIntensity, Alpha);
    return Result;
}

void ADayNightCycleManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    ApplyLightingForCurrentTime();
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDayEnum() const
{
    return CurrentTimeOfDayEnum;
}

float ADayNightCycleManager::GetNormalizedTime() const
{
    return CurrentTimeOfDay / 24.0f;
}

void ADayNightCycleManager::ApplyLightingForCurrentTime()
{
    UpdateLightingColors();
    CurrentTimeOfDayEnum = ClassifyTimeOfDay(CurrentTimeOfDay);
}
