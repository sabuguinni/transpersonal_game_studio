#include "CretaceousLightingManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
    DayProgress = 0.45f;

    // Default Dawn settings — cool blue-pink
    DawnSettings.SunPitchDegrees = -5.0f;
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.72f, 0.55f, 1.0f);
    DawnSettings.FogDensity = 0.06f;
    DawnSettings.FogColor = FLinearColor(0.6f, 0.65f, 0.9f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.8f;

    // Default Noon settings — bright white
    NoonSettings.SunPitchDegrees = -75.0f;
    NoonSettings.SunIntensity = 16.0f;
    NoonSettings.SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
    NoonSettings.FogDensity = 0.02f;
    NoonSettings.FogColor = FLinearColor(0.7f, 0.82f, 0.95f, 1.0f);
    NoonSettings.SkyLightIntensity = 3.5f;

    // Default Dusk settings — warm orange-red
    DuskSettings.SunPitchDegrees = -8.0f;
    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.45f, 0.18f, 1.0f);
    DuskSettings.FogDensity = 0.055f;
    DuskSettings.FogColor = FLinearColor(0.85f, 0.55f, 0.35f, 1.0f);
    DuskSettings.SkyLightIntensity = 1.0f;

    // Default Night settings — deep blue moonlight
    NightSettings.SunPitchDegrees = 30.0f;
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.35f, 0.7f, 1.0f);
    NightSettings.FogDensity = 0.045f;
    NightSettings.FogColor = FLinearColor(0.1f, 0.12f, 0.3f, 1.0f);
    NightSettings.SkyLightIntensity = 0.3f;
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();

    DayProgress = StartingDayProgress;
    AutoFindLightingActors();

    // Apply initial settings
    FLight_DayPhaseSettings InitialSettings = GetSettingsForProgress(DayProgress);
    ApplyDayPhaseSettings(InitialSettings);
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle && DayDurationSeconds > 0.0f)
    {
        AdvanceDayNightCycle(DeltaTime);
    }
}

void ACretaceousLightingManager::AdvanceDayNightCycle(float DeltaSeconds)
{
    DayProgress += DeltaSeconds / DayDurationSeconds;
    if (DayProgress >= 1.0f)
    {
        DayProgress -= 1.0f;
    }

    // Determine time of day from progress (0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk)
    if (DayProgress < 0.2f || DayProgress >= 0.85f)
    {
        CurrentTimeOfDay = ELight_TimeOfDay::Night;
    }
    else if (DayProgress < 0.3f)
    {
        CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
    }
    else if (DayProgress < 0.45f)
    {
        CurrentTimeOfDay = ELight_TimeOfDay::Morning;
    }
    else if (DayProgress < 0.6f)
    {
        CurrentTimeOfDay = ELight_TimeOfDay::Noon;
    }
    else if (DayProgress < 0.7f)
    {
        CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
    }
    else
    {
        CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
    }

    FLight_DayPhaseSettings CurrentSettings = GetSettingsForProgress(DayProgress);
    ApplyDayPhaseSettings(CurrentSettings);
}

void ACretaceousLightingManager::SetTimeOfDay(ELight_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;

    switch (NewTime)
    {
    case ELight_TimeOfDay::Dawn:
        DayProgress = 0.25f;
        break;
    case ELight_TimeOfDay::Morning:
        DayProgress = 0.35f;
        break;
    case ELight_TimeOfDay::Noon:
        DayProgress = 0.5f;
        break;
    case ELight_TimeOfDay::Afternoon:
        DayProgress = 0.55f;
        break;
    case ELight_TimeOfDay::Dusk:
        DayProgress = 0.72f;
        break;
    case ELight_TimeOfDay::Night:
        DayProgress = 0.1f;
        break;
    }

    FLight_DayPhaseSettings Settings = GetSettingsForProgress(DayProgress);
    ApplyDayPhaseSettings(Settings);
}

void ACretaceousLightingManager::ApplyDayPhaseSettings(const FLight_DayPhaseSettings& Settings)
{
    // Apply sun settings
    if (SunActor)
    {
        UpdateSunTransform(Settings.SunPitchDegrees);
        UDirectionalLightComponent* SunComp = SunActor->GetComponent();
        if (SunComp)
        {
            SunComp->SetIntensity(Settings.SunIntensity);
            SunComp->SetLightColor(Settings.SunColor);
        }
    }

    // Apply sky light
    if (SkyLightActor)
    {
        USkyLightComponent* SLComp = SkyLightActor->GetLightComponent();
        if (SLComp)
        {
            SLComp->SetIntensity(Settings.SkyLightIntensity);
        }
    }

    // Apply fog settings
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }
}

FLight_DayPhaseSettings ACretaceousLightingManager::GetSettingsForProgress(float Progress) const
{
    // Map progress to phase transitions
    // 0.0 = midnight, 0.25 = dawn, 0.5 = noon, 0.75 = dusk, 1.0 = midnight

    if (Progress < 0.2f)
    {
        // Night -> Dawn transition
        float Alpha = Progress / 0.2f;
        return LerpDayPhaseSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (Progress < 0.3f)
    {
        // Dawn -> Morning
        float Alpha = (Progress - 0.2f) / 0.1f;
        return LerpDayPhaseSettings(DawnSettings, NoonSettings, Alpha * 0.5f);
    }
    else if (Progress < 0.5f)
    {
        // Morning -> Noon
        float Alpha = (Progress - 0.3f) / 0.2f;
        return LerpDayPhaseSettings(DawnSettings, NoonSettings, 0.5f + Alpha * 0.5f);
    }
    else if (Progress < 0.65f)
    {
        // Noon -> Afternoon (use dusk as afternoon target)
        float Alpha = (Progress - 0.5f) / 0.15f;
        return LerpDayPhaseSettings(NoonSettings, DuskSettings, Alpha * 0.5f);
    }
    else if (Progress < 0.8f)
    {
        // Afternoon -> Dusk
        float Alpha = (Progress - 0.65f) / 0.15f;
        return LerpDayPhaseSettings(NoonSettings, DuskSettings, 0.5f + Alpha * 0.5f);
    }
    else
    {
        // Dusk -> Night
        float Alpha = (Progress - 0.8f) / 0.2f;
        return LerpDayPhaseSettings(DuskSettings, NightSettings, Alpha);
    }
}

FLight_DayPhaseSettings ACretaceousLightingManager::LerpDayPhaseSettings(
    const FLight_DayPhaseSettings& A,
    const FLight_DayPhaseSettings& B,
    float Alpha) const
{
    FLight_DayPhaseSettings Result;
    Result.SunPitchDegrees = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
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
    return Result;
}

void ACretaceousLightingManager::UpdateSunTransform(float PitchDegrees)
{
    if (SunActor)
    {
        FRotator CurrentRot = SunActor->GetActorRotation();
        SunActor->SetActorRotation(FRotator(PitchDegrees, CurrentRot.Yaw, 0.0f));
    }
}

void ACretaceousLightingManager::AutoFindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Auto-find DirectionalLight
    if (!SunActor)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunActor = *It;
            break;
        }
    }

    // Auto-find SkyLight
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }

    // Auto-find ExponentialHeightFog
    if (!FogActor)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            FogActor = *It;
            break;
        }
    }
}
