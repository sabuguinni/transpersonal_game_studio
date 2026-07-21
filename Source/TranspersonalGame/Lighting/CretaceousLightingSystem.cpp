#include "CretaceousLightingSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// ACretaceousLightingSystem — Implementation
// Director of Photography for the Cretaceous world.
// Light doesn't just illuminate — it defines the emotional
// truth of each moment in the prehistoric survival experience.
// ============================================================

ACretaceousLightingSystem::ACretaceousLightingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;  // Update 10x/sec — smooth but not every frame

    // ── Dawn Preset ──────────────────────────────────────────
    DawnSettings.SunPitchAngle = -5.0f;
    DawnSettings.SunYawAngle = 90.0f;
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);  // Deep orange-red
    DawnSettings.SkyLightIntensity = 0.4f;
    DawnSettings.FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.bVolumetricFog = true;

    // ── Golden Hour Preset ───────────────────────────────────
    GoldenHourSettings.SunPitchAngle = -15.0f;
    GoldenHourSettings.SunYawAngle = 250.0f;
    GoldenHourSettings.SunIntensity = 8.5f;
    GoldenHourSettings.SunColor = FLinearColor(1.0f, 0.784f, 0.549f, 1.0f);  // Warm amber
    GoldenHourSettings.SkyLightIntensity = 1.2f;
    GoldenHourSettings.FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
    GoldenHourSettings.FogDensity = 0.02f;
    GoldenHourSettings.bVolumetricFog = true;

    // ── Midday Preset ────────────────────────────────────────
    MiddaySettings.SunPitchAngle = -75.0f;
    MiddaySettings.SunYawAngle = 180.0f;
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.97f, 0.9f, 1.0f);  // Near-white hot
    MiddaySettings.SkyLightIntensity = 1.8f;
    MiddaySettings.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    MiddaySettings.FogDensity = 0.01f;
    MiddaySettings.bVolumetricFog = false;

    // ── Night Preset ─────────────────────────────────────────
    NightSettings.SunPitchAngle = 30.0f;   // Below horizon
    NightSettings.SunYawAngle = 0.0f;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);  // Cool blue moonlight
    NightSettings.SkyLightIntensity = 0.15f;
    NightSettings.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    NightSettings.FogDensity = 0.05f;
    NightSettings.bVolumetricFog = true;
}

void ACretaceousLightingSystem::BeginPlay()
{
    Super::BeginPlay();

    // Auto-discover scene actors if not manually assigned
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

    if (!FogActor)
    {
        TArray<AActor*> FoundFog;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundFog);
        if (FoundFog.Num() > 0)
        {
            FogActor = Cast<AExponentialHeightFog>(FoundFog[0]);
        }
    }

    // Apply initial time of day
    SetTimeOfDay(CurrentTimeOfDayNormalized);
}

void ACretaceousLightingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDayNightCycleEnabled) return;

    // Advance time
    ElapsedDayTime += DeltaTime;
    if (ElapsedDayTime >= DayDurationSeconds)
    {
        ElapsedDayTime = 0.0f;
    }

    CurrentTimeOfDayNormalized = ElapsedDayTime / DayDurationSeconds;
    UpdateTimeOfDayEnum(CurrentTimeOfDayNormalized);
    UpdateSunPosition(CurrentTimeOfDayNormalized);
    UpdateSkyAndFog(CurrentTimeOfDayNormalized);

    // Weather transition
    if (CurrentWeather != TargetWeather)
    {
        WeatherTransitionAlpha = FMath::Clamp(
            WeatherTransitionAlpha + DeltaTime * WeatherTransitionSpeed, 0.0f, 1.0f);
        if (WeatherTransitionAlpha >= 1.0f)
        {
            CurrentWeather = TargetWeather;
            WeatherTransitionAlpha = 0.0f;
        }
    }
}

void ACretaceousLightingSystem::SetTimeOfDay(float NormalizedTime)
{
    CurrentTimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    ElapsedDayTime = CurrentTimeOfDayNormalized * DayDurationSeconds;
    UpdateTimeOfDayEnum(CurrentTimeOfDayNormalized);
    UpdateSunPosition(CurrentTimeOfDayNormalized);
    UpdateSkyAndFog(CurrentTimeOfDayNormalized);
}

void ACretaceousLightingSystem::SetWeatherState(ELight_WeatherState NewWeather)
{
    TargetWeather = NewWeather;
    WeatherTransitionAlpha = 0.0f;
}

void ACretaceousLightingSystem::ApplyLightingPreset(const FLight_TimeOfDaySettings& Settings)
{
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Settings.SunPitchAngle, Settings.SunYawAngle, 0.0f));
        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(Settings.SunIntensity);
            DLC->SetLightColor(Settings.SunColor);
        }
    }

    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(Settings.SkyLightIntensity);
        }
    }

    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
            FogComp->SetVolumetricFog(Settings.bVolumetricFog);
        }
    }
}

void ACretaceousLightingSystem::ApplyGoldenHourPreset()
{
    ApplyLightingPreset(GoldenHourSettings);
}

void ACretaceousLightingSystem::ApplyNightPreset()
{
    ApplyLightingPreset(NightSettings);
}

void ACretaceousLightingSystem::ApplyStormPreset()
{
    FLight_TimeOfDaySettings StormSettings;
    StormSettings.SunPitchAngle = -40.0f;
    StormSettings.SunYawAngle = 180.0f;
    StormSettings.SunIntensity = 1.5f;
    StormSettings.SunColor = FLinearColor(0.5f, 0.55f, 0.6f, 1.0f);  // Desaturated grey
    StormSettings.SkyLightIntensity = 0.3f;
    StormSettings.FogColor = FLinearColor(0.4f, 0.45f, 0.5f, 1.0f);
    StormSettings.FogDensity = 0.08f;
    StormSettings.bVolumetricFog = true;
    ApplyLightingPreset(StormSettings);
    SetWeatherState(ELight_WeatherState::HeavyStorm);
}

void ACretaceousLightingSystem::UpdateSunPosition(float T)
{
    if (!SunLight) return;

    // Sun arc: rises from east (yaw=90) sets in west (yaw=270)
    // T=0 midnight, T=0.25 dawn, T=0.5 noon, T=0.75 dusk, T=1.0 midnight
    float SunYaw = FMath::Lerp(90.0f, 270.0f, T);

    // Pitch: -90 at noon (directly overhead), +30 at midnight (below horizon)
    // Smooth sine curve
    float SunPitch = -90.0f * FMath::Sin(T * PI);
    // Clamp: don't go below -80 during day, don't go above +35 at night
    SunPitch = FMath::Clamp(SunPitch, -80.0f, 35.0f);

    SunLight->SetActorRotation(FRotator(SunPitch, SunYaw, 0.0f));

    // Intensity: peak at noon, near-zero at night
    float DayFactor = FMath::Max(0.0f, FMath::Sin(T * PI));
    float Intensity = FMath::Lerp(0.05f, 12.0f, DayFactor);

    // Color: warm at dawn/dusk, white at noon, blue at night
    FLinearColor SunColor;
    if (DayFactor > 0.8f)
    {
        // Midday — near white
        SunColor = FLinearColor(1.0f, 0.97f, 0.9f, 1.0f);
    }
    else if (DayFactor > 0.3f)
    {
        // Morning/afternoon — warm amber
        SunColor = FLinearColor(1.0f, 0.784f, 0.549f, 1.0f);
    }
    else if (DayFactor > 0.05f)
    {
        // Dawn/dusk — deep orange
        SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    }
    else
    {
        // Night — cool blue moonlight
        SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
        Intensity = 0.08f;
    }

    UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (DLC)
    {
        DLC->SetIntensity(Intensity);
        DLC->SetLightColor(SunColor);
    }
}

void ACretaceousLightingSystem::UpdateSkyAndFog(float T)
{
    float DayFactor = FMath::Max(0.0f, FMath::Sin(T * PI));

    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            float SkyIntensity = FMath::Lerp(0.1f, 1.8f, DayFactor);
            SLC->SetIntensity(SkyIntensity);
        }
    }

    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            // Morning mist, clear midday, evening haze
            float FogDensity;
            FLinearColor FogColor;

            if (DayFactor > 0.7f)
            {
                // Clear midday
                FogDensity = 0.01f;
                FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
            }
            else if (DayFactor > 0.2f)
            {
                // Morning/afternoon haze
                FogDensity = 0.02f;
                FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
            }
            else
            {
                // Dawn/dusk/night mist
                FogDensity = 0.04f;
                FogColor = FLinearColor(0.3f, 0.35f, 0.5f, 1.0f);
            }

            FogComp->SetFogDensity(FogDensity);
            FogComp->SetFogInscatteringColor(FogColor);
        }
    }
}

void ACretaceousLightingSystem::UpdateTimeOfDayEnum(float T)
{
    if (T < 0.1f || T >= 0.95f)
        CurrentTimeOfDay = ELight_TimeOfDay::Midnight;
    else if (T < 0.2f)
        CurrentTimeOfDay = ELight_TimeOfDay::Night;
    else if (T < 0.28f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
    else if (T < 0.38f)
        CurrentTimeOfDay = ELight_TimeOfDay::Morning;
    else if (T < 0.55f)
        CurrentTimeOfDay = ELight_TimeOfDay::Midday;
    else if (T < 0.65f)
        CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
    else if (T < 0.75f)
        CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;
    else
        CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
}

FLight_TimeOfDaySettings ACretaceousLightingSystem::InterpolateSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunPitchAngle = FMath::Lerp(A.SunPitchAngle, B.SunPitchAngle, Alpha);
    Result.SunYawAngle = FMath::Lerp(A.SunYawAngle, B.SunYawAngle, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.bVolumetricFog = Alpha < 0.5f ? A.bVolumetricFog : B.bVolumetricFog;
    return Result;
}
