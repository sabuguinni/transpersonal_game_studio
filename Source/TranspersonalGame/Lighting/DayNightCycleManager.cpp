#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Dawn: warm pink-orange light, low sun
    DawnSettings.SunPitch = -10.0f;
    DawnSettings.SunYaw = -90.0f;
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.5f;
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogColor = FLinearColor(0.6f, 0.5f, 0.7f, 1.0f);
    DawnSettings.AmbientTemperature = 15.0f;

    // Midday: bright white-yellow light, high sun
    MiddaySettings.SunPitch = -75.0f;
    MiddaySettings.SunYaw = 0.0f;
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    MiddaySettings.SkyLightIntensity = 2.0f;
    MiddaySettings.FogDensity = 0.01f;
    MiddaySettings.FogColor = FLinearColor(0.5f, 0.7f, 0.9f, 1.0f);
    MiddaySettings.AmbientTemperature = 32.0f;

    // Dusk: warm amber-red light, low sun
    DuskSettings.SunPitch = -8.0f;
    DuskSettings.SunYaw = 90.0f;
    DuskSettings.SunIntensity = 2.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.4f;
    DuskSettings.FogDensity = 0.05f;
    DuskSettings.FogColor = FLinearColor(0.7f, 0.4f, 0.3f, 1.0f);
    DuskSettings.AmbientTemperature = 24.0f;

    // Night: dim blue moonlight
    NightSettings.SunPitch = -45.0f;
    NightSettings.SunYaw = 180.0f;
    NightSettings.SunIntensity = 0.3f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    NightSettings.SkyLightIntensity = 0.15f;
    NightSettings.FogDensity = 0.03f;
    NightSettings.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    NightSettings.AmbientTemperature = 12.0f;
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

    // Apply initial lighting
    ApplyLightingForTime(CurrentTimeOfDay);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive) return;

    ElapsedSeconds += DeltaTime;

    // Advance time: TimeScale seconds of game time per real second
    float HoursPerSecond = TimeScale / 3600.0f;
    CurrentTimeOfDay += DeltaTime * HoursPerSecond;

    // Wrap around 24 hours
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    // Apply lighting every frame (Lumen handles GI updates)
    ApplyLightingForTime(CurrentTimeOfDay);
}

void ADayNightCycleManager::SetTimeOfDay(float NewHour)
{
    CurrentTimeOfDay = FMath::Clamp(NewHour, 0.0f, 23.99f);
    ApplyLightingForTime(CurrentTimeOfDay);
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)  return ELight_TimeOfDay::Morning;
    if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 14.0f) return ELight_TimeOfDay::Midday;
    if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 18.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentTimeOfDay >= 20.0f && CurrentTimeOfDay < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

bool ADayNightCycleManager::IsNightTime() const
{
    return CurrentTimeOfDay >= 20.0f || CurrentTimeOfDay < 5.0f;
}

float ADayNightCycleManager::GetNormalizedDayProgress() const
{
    return CurrentTimeOfDay / 24.0f;
}

void ADayNightCycleManager::ApplyLightingForTime(float Hour)
{
    if (!SunLight) return;

    UDirectionalLightComponent* LightComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (!LightComp) return;

    FLight_TimeOfDaySettings Settings;

    // Interpolate between presets based on hour
    if (Hour >= 5.0f && Hour < 12.0f)
    {
        // Dawn → Midday
        float Alpha = (Hour - 5.0f) / 7.0f;
        Settings = InterpolateSettings(DawnSettings, MiddaySettings, Alpha);
    }
    else if (Hour >= 12.0f && Hour < 19.0f)
    {
        // Midday → Dusk
        float Alpha = (Hour - 12.0f) / 7.0f;
        Settings = InterpolateSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else if (Hour >= 19.0f && Hour < 21.0f)
    {
        // Dusk → Night
        float Alpha = (Hour - 19.0f) / 2.0f;
        Settings = InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }
    else
    {
        // Night (21:00 - 05:00)
        Settings = NightSettings;
    }

    // Apply sun rotation
    FRotator SunRotation(Settings.SunPitch, Settings.SunYaw, 0.0f);
    SunLight->SetActorRotation(SunRotation);

    // Apply light properties
    LightComp->SetIntensity(Settings.SunIntensity);
    LightComp->SetLightColor(Settings.SunColor);
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
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.AmbientTemperature = FMath::Lerp(A.AmbientTemperature, B.AmbientTemperature, Alpha);
    return Result;
}
