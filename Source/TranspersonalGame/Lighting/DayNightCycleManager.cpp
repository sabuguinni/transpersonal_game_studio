#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Math/UnrealMathUtility.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = UpdateInterval;

    // Dawn preset — warm pink/orange light, low sun
    DawnSettings.SunPitch = -8.0f;
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DawnSettings.FogDensity = 0.08f;
    DawnSettings.FogColor = FLinearColor(0.6f, 0.5f, 0.7f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.5f;
    DawnSettings.ExposureBias = 0.5f;

    // Midday preset — bright white light, high sun
    MiddaySettings.SunPitch = -75.0f;
    MiddaySettings.SunIntensity = 15.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
    MiddaySettings.FogDensity = 0.02f;
    MiddaySettings.FogColor = FLinearColor(0.5f, 0.65f, 0.8f, 1.0f);
    MiddaySettings.SkyLightIntensity = 2.0f;
    MiddaySettings.ExposureBias = 1.5f;

    // Dusk preset — golden orange, low sun opposite side
    DuskSettings.SunPitch = -10.0f;
    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DuskSettings.FogDensity = 0.06f;
    DuskSettings.FogColor = FLinearColor(0.7f, 0.4f, 0.3f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.8f;
    DuskSettings.ExposureBias = 0.8f;

    // Night preset — deep blue, moonlight
    NightSettings.SunPitch = 20.0f; // Below horizon
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    NightSettings.FogDensity = 0.05f;
    NightSettings.FogColor = FLinearColor(0.05f, 0.08f, 0.2f, 1.0f);
    NightSettings.SkyLightIntensity = 0.2f;
    NightSettings.ExposureBias = -1.0f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    FindAndCacheLightActors();
    SetTimeOfDay(TimeOfDayNormalized);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive)
        return;

    // Advance time
    const float RealSecondsPerGameDay = 86400.0f / CycleSpeedMultiplier;
    TimeOfDayNormalized += DeltaTime / RealSecondsPerGameDay;
    if (TimeOfDayNormalized >= 1.0f)
        TimeOfDayNormalized -= 1.0f;

    UpdateLighting(DeltaTime);
}

void ADayNightCycleManager::SetTimeOfDay(float NormalizedTime)
{
    TimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    CurrentTimeOfDay = CalculateTimeOfDayEnum(TimeOfDayNormalized);
    UpdateLighting(0.0f);
}

void ADayNightCycleManager::SetTimePreset(ELight_TimeOfDay Preset)
{
    switch (Preset)
    {
        case ELight_TimeOfDay::Dawn:        SetTimeOfDay(0.25f); break;
        case ELight_TimeOfDay::Morning:     SetTimeOfDay(0.30f); break;
        case ELight_TimeOfDay::Midday:      SetTimeOfDay(0.50f); break;
        case ELight_TimeOfDay::Afternoon:   SetTimeOfDay(0.65f); break;
        case ELight_TimeOfDay::Dusk:        SetTimeOfDay(0.75f); break;
        case ELight_TimeOfDay::Night:       SetTimeOfDay(0.85f); break;
        case ELight_TimeOfDay::Midnight:    SetTimeOfDay(0.0f);  break;
    }
}

float ADayNightCycleManager::GetCurrentHour() const
{
    return TimeOfDayNormalized * 24.0f;
}

ELight_TimeOfDay ADayNightCycleManager::GetTimeOfDayEnum() const
{
    return CurrentTimeOfDay;
}

void ADayNightCycleManager::FindAndCacheLightActors()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    // Find DirectionalLight
    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find ExponentialHeightFog
    if (!HeightFog)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
            HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    // Find SkyLight
    if (!SceneSkyLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
            SceneSkyLight = Cast<ASkyLight>(FoundActors[0]);
    }
}

void ADayNightCycleManager::UpdateLighting(float DeltaTime)
{
    CurrentTimeOfDay = CalculateTimeOfDayEnum(TimeOfDayNormalized);

    // Determine which two presets to interpolate between
    FLight_TimeOfDaySettings CurrentSettings;
    float T = TimeOfDayNormalized;

    if (T < 0.25f)
    {
        // Midnight → Dawn
        float Alpha = T / 0.25f;
        CurrentSettings = InterpolateSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (T < 0.50f)
    {
        // Dawn → Midday
        float Alpha = (T - 0.25f) / 0.25f;
        CurrentSettings = InterpolateSettings(DawnSettings, MiddaySettings, Alpha);
    }
    else if (T < 0.75f)
    {
        // Midday → Dusk
        float Alpha = (T - 0.50f) / 0.25f;
        CurrentSettings = InterpolateSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else
    {
        // Dusk → Night
        float Alpha = (T - 0.75f) / 0.25f;
        CurrentSettings = InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }

    ApplyLightingSettings(CurrentSettings);
}

void ADayNightCycleManager::ApplyLightingSettings(const FLight_TimeOfDaySettings& Settings)
{
    // Apply to DirectionalLight (Sun)
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Settings.SunPitch, 45.0f, 0.0f));
        UDirectionalLightComponent* LightComp = SunLight->GetComponent();
        if (LightComp)
        {
            LightComp->SetIntensity(Settings.SunIntensity);
            LightComp->SetLightColor(Settings.SunColor);
        }
    }

    // Apply to ExponentialHeightFog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }

    // Apply to SkyLight
    if (SceneSkyLight)
    {
        USkyLightComponent* SkyComp = SceneSkyLight->GetLightComponent();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Settings.SkyLightIntensity);
        }
    }
}

FLight_TimeOfDaySettings ADayNightCycleManager::InterpolateSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.ExposureBias = FMath::Lerp(A.ExposureBias, B.ExposureBias, Alpha);
    return Result;
}

ELight_TimeOfDay ADayNightCycleManager::CalculateTimeOfDayEnum(float NormalizedTime) const
{
    if (NormalizedTime < 0.22f) return ELight_TimeOfDay::Midnight;
    if (NormalizedTime < 0.28f) return ELight_TimeOfDay::Dawn;
    if (NormalizedTime < 0.40f) return ELight_TimeOfDay::Morning;
    if (NormalizedTime < 0.58f) return ELight_TimeOfDay::Midday;
    if (NormalizedTime < 0.70f) return ELight_TimeOfDay::Afternoon;
    if (NormalizedTime < 0.80f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}
