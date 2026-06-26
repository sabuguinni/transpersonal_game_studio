#include "CretaceousLightingManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Morning preset — warm golden hour
    MorningSettings.SunPitch = -15.0f;
    MorningSettings.SunIntensity = 6.0f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.72f, 0.42f, 1.0f);
    MorningSettings.FogDensity = 0.045f;
    MorningSettings.FogColor = FLinearColor(0.65f, 0.72f, 0.9f, 1.0f);
    MorningSettings.SkyLightIntensity = 1.2f;

    // Midday preset — bright, slightly harsh
    MiddaySettings.SunPitch = -75.0f;
    MiddaySettings.SunIntensity = 18.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
    MiddaySettings.FogDensity = 0.012f;
    MiddaySettings.FogColor = FLinearColor(0.55f, 0.72f, 0.9f, 1.0f);
    MiddaySettings.SkyLightIntensity = 3.5f;

    // Dusk preset — deep amber/orange
    DuskSettings.SunPitch = -8.0f;
    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.45f, 0.18f, 1.0f);
    DuskSettings.FogDensity = 0.065f;
    DuskSettings.FogColor = FLinearColor(0.85f, 0.52f, 0.35f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.8f;
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();
    ApplyCretaceousPreset();
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDynamicDayCycle)
    {
        // Advance time — DayCycleSpeed=60 means 60 in-game seconds per real second
        TimeOfDayNormalized += (DeltaTime * DayCycleSpeed) / 86400.0f;
        if (TimeOfDayNormalized > 1.0f)
        {
            TimeOfDayNormalized -= 1.0f;
        }
        SetTimeOfDay(TimeOfDayNormalized);
    }
}

void ACretaceousLightingManager::SetTimeOfDay(float NormalizedTime)
{
    TimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    UpdateSunRotation();
    UpdateFogSettings();
    UpdateSkyLight();
}

ELight_TimeOfDay ACretaceousLightingManager::GetCurrentTimeOfDayEnum() const
{
    if (TimeOfDayNormalized < 0.15f || TimeOfDayNormalized > 0.92f) return ELight_TimeOfDay::Night;
    if (TimeOfDayNormalized < 0.22f) return ELight_TimeOfDay::Dawn;
    if (TimeOfDayNormalized < 0.35f) return ELight_TimeOfDay::Morning;
    if (TimeOfDayNormalized < 0.60f) return ELight_TimeOfDay::Midday;
    if (TimeOfDayNormalized < 0.75f) return ELight_TimeOfDay::Afternoon;
    return ELight_TimeOfDay::Dusk;
}

void ACretaceousLightingManager::ApplyCretaceousPreset()
{
    // Default: golden afternoon (TimeOfDayNormalized = 0.35)
    SetTimeOfDay(TimeOfDayNormalized);
}

void ACretaceousLightingManager::ApplyLightingInEditor()
{
    ApplyCretaceousPreset();
}

void ACretaceousLightingManager::UpdateSunRotation()
{
    if (!SunLight) return;

    UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (!SunComp) return;

    // Determine current phase settings by blending
    FLight_DayPhaseSettings CurrentSettings;

    // Simple 3-phase blend: morning (0.2-0.4), midday (0.4-0.65), dusk (0.65-0.85)
    float t = TimeOfDayNormalized;

    if (t < 0.3f)
    {
        CurrentSettings = MorningSettings;
    }
    else if (t < 0.55f)
    {
        float alpha = (t - 0.3f) / 0.25f;
        CurrentSettings = LerpDayPhase(MorningSettings, MiddaySettings, alpha);
    }
    else if (t < 0.75f)
    {
        CurrentSettings = MiddaySettings;
    }
    else
    {
        float alpha = (t - 0.75f) / 0.15f;
        CurrentSettings = LerpDayPhase(MiddaySettings, DuskSettings, FMath::Clamp(alpha, 0.0f, 1.0f));
    }

    SunLight->SetActorRotation(FRotator(CurrentSettings.SunPitch, 45.0f, 0.0f));
    SunComp->SetIntensity(CurrentSettings.SunIntensity);
    SunComp->SetLightColor(CurrentSettings.SunColor);
}

void ACretaceousLightingManager::UpdateFogSettings()
{
    if (!GetWorld()) return;

    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FogActors);

    if (FogActors.Num() == 0) return;

    AExponentialHeightFog* Fog = Cast<AExponentialHeightFog>(FogActors[0]);
    if (!Fog) return;

    UExponentialHeightFogComponent* FogComp = Fog->GetComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp) return;

    // Determine phase
    float t = TimeOfDayNormalized;
    FLight_DayPhaseSettings CurrentSettings;

    if (t < 0.3f)
    {
        CurrentSettings = MorningSettings;
    }
    else if (t < 0.55f)
    {
        float alpha = (t - 0.3f) / 0.25f;
        CurrentSettings = LerpDayPhase(MorningSettings, MiddaySettings, alpha);
    }
    else if (t < 0.75f)
    {
        CurrentSettings = MiddaySettings;
    }
    else
    {
        float alpha = (t - 0.75f) / 0.15f;
        CurrentSettings = LerpDayPhase(MiddaySettings, DuskSettings, FMath::Clamp(alpha, 0.0f, 1.0f));
    }

    FogComp->SetFogDensity(CurrentSettings.FogDensity);
    FogComp->SetFogInscatteringColor(CurrentSettings.FogColor);
}

void ACretaceousLightingManager::UpdateSkyLight()
{
    if (!SceneSkyLight) return;

    USkyLightComponent* SLComp = SceneSkyLight->GetComponentByClass<USkyLightComponent>();
    if (!SLComp) return;

    float t = TimeOfDayNormalized;
    FLight_DayPhaseSettings CurrentSettings;

    if (t < 0.3f)
    {
        CurrentSettings = MorningSettings;
    }
    else if (t < 0.55f)
    {
        float alpha = (t - 0.3f) / 0.25f;
        CurrentSettings = LerpDayPhase(MorningSettings, MiddaySettings, alpha);
    }
    else if (t < 0.75f)
    {
        CurrentSettings = MiddaySettings;
    }
    else
    {
        float alpha = (t - 0.75f) / 0.15f;
        CurrentSettings = LerpDayPhase(MiddaySettings, DuskSettings, FMath::Clamp(alpha, 0.0f, 1.0f));
    }

    SLComp->SetIntensity(CurrentSettings.SkyLightIntensity);
}

FLight_DayPhaseSettings ACretaceousLightingManager::LerpDayPhase(
    const FLight_DayPhaseSettings& A,
    const FLight_DayPhaseSettings& B,
    float Alpha) const
{
    FLight_DayPhaseSettings Result;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    return Result;
}
