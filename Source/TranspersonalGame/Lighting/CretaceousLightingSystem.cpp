#include "CretaceousLightingSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Math/UnrealMathUtility.h"

ACretaceousLightingSystem::ACretaceousLightingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f;  // 20 Hz — sufficient for smooth day cycle

    // ── Golden Hour Preset (default Cretaceous look) ──────────────────
    GoldenHourPreset.SunPitch         = -38.0f;
    GoldenHourPreset.SunYaw           = 45.0f;
    GoldenHourPreset.SunIntensity     = 12.0f;
    GoldenHourPreset.SunColor         = FLinearColor(1.0f, 0.878f, 0.62f, 1.0f);
    GoldenHourPreset.SkyLightIntensity = 2.0f;
    GoldenHourPreset.FogDensity       = 0.025f;
    GoldenHourPreset.FogColor         = FLinearColor(0.85f, 0.72f, 0.55f, 1.0f);
    GoldenHourPreset.ExposureBias     = 0.5f;

    // ── Midday Preset ─────────────────────────────────────────────────
    MiddayPreset.SunPitch         = -75.0f;
    MiddayPreset.SunYaw           = 0.0f;
    MiddayPreset.SunIntensity     = 18.0f;
    MiddayPreset.SunColor         = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
    MiddayPreset.SkyLightIntensity = 3.0f;
    MiddayPreset.FogDensity       = 0.015f;
    MiddayPreset.FogColor         = FLinearColor(0.7f, 0.82f, 0.95f, 1.0f);
    MiddayPreset.ExposureBias     = 0.0f;

    // ── Night Preset ──────────────────────────────────────────────────
    NightPreset.SunPitch         = 20.0f;   // Below horizon
    NightPreset.SunYaw           = 180.0f;
    NightPreset.SunIntensity     = 0.5f;    // Moon
    NightPreset.SunColor         = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    NightPreset.SkyLightIntensity = 0.3f;
    NightPreset.FogDensity       = 0.04f;
    NightPreset.FogColor         = FLinearColor(0.1f, 0.12f, 0.25f, 1.0f);
    NightPreset.ExposureBias     = -1.5f;
}

void ACretaceousLightingSystem::BeginPlay()
{
    Super::BeginPlay();
    AutoDiscoverLightingActors();
    RefreshLighting();
}

void ACretaceousLightingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDayCycleActive)
    {
        TickDayCycle(DeltaTime);
    }
}

void ACretaceousLightingSystem::TickDayCycle(float DeltaTime)
{
    // Advance time of day
    const float RealSecondsPerGameDay = 86400.0f / DayCycleSpeed;
    const float DeltaNormalized = DeltaTime / RealSecondsPerGameDay;
    TimeOfDayNormalized = FMath::Fmod(TimeOfDayNormalized + DeltaNormalized, 1.0f);

    // Only update lighting if time has changed meaningfully (avoid micro-updates)
    if (FMath::Abs(TimeOfDayNormalized - PreviousTimeOfDay) > 0.001f)
    {
        RefreshLighting();
        PreviousTimeOfDay = TimeOfDayNormalized;
    }
}

void ACretaceousLightingSystem::AutoDiscoverLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    // Find SkyLight
    if (!SceneSkyLight)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SceneSkyLight = *It;
            break;
        }
    }

    // Find ExponentialHeightFog
    if (!SceneFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            SceneFog = *It;
            break;
        }
    }

    // Find PostProcessVolume
    if (!ScenePostProcess)
    {
        for (TActorIterator<APostProcessVolume> It(World); It; ++It)
        {
            ScenePostProcess = *It;
            break;
        }
    }
}

void ACretaceousLightingSystem::RefreshLighting()
{
    FLight_TimePreset CurrentPreset = GetPresetForTime(TimeOfDayNormalized);
    ApplyPresetToScene(CurrentPreset);
}

void ACretaceousLightingSystem::ApplyPresetToScene(const FLight_TimePreset& Preset)
{
    // ── Sun ───────────────────────────────────────────────────────────
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Preset.SunPitch, Preset.SunYaw, 0.0f));
        if (UDirectionalLightComponent* DC = SunLight->GetComponent())
        {
            DC->SetIntensity(Preset.SunIntensity);
            DC->SetLightColor(Preset.SunColor);
        }
    }

    // ── SkyLight ──────────────────────────────────────────────────────
    if (SceneSkyLight)
    {
        if (USkyLightComponent* SLC = SceneSkyLight->GetLightComponent())
        {
            SLC->SetIntensity(Preset.SkyLightIntensity);
        }
    }

    // ── Fog ───────────────────────────────────────────────────────────
    if (SceneFog)
    {
        if (UExponentialHeightFogComponent* FC = SceneFog->GetComponent())
        {
            FC->SetFogDensity(Preset.FogDensity);
            FC->SetFogInscatteringColor(Preset.FogColor);
        }
    }
}

FLight_TimePreset ACretaceousLightingSystem::GetPresetForTime(float NormalizedTime) const
{
    // Map 0-1 time to sun angle and blend between presets
    // 0.0  = midnight
    // 0.25 = dawn
    // 0.5  = noon
    // 0.75 = dusk
    // 1.0  = midnight

    if (NormalizedTime < 0.25f)
    {
        // Midnight → Dawn
        float Alpha = NormalizedTime / 0.25f;
        return InterpolatePresets(NightPreset, GoldenHourPreset, Alpha * 0.3f);
    }
    else if (NormalizedTime < 0.4f)
    {
        // Dawn → Morning
        float Alpha = (NormalizedTime - 0.25f) / 0.15f;
        return InterpolatePresets(GoldenHourPreset, MiddayPreset, Alpha * 0.5f);
    }
    else if (NormalizedTime < 0.6f)
    {
        // Morning → Midday → Afternoon
        float Alpha = (NormalizedTime - 0.4f) / 0.2f;
        return InterpolatePresets(MiddayPreset, MiddayPreset, Alpha);
    }
    else if (NormalizedTime < 0.75f)
    {
        // Afternoon → Golden Hour
        float Alpha = (NormalizedTime - 0.6f) / 0.15f;
        return InterpolatePresets(MiddayPreset, GoldenHourPreset, Alpha);
    }
    else if (NormalizedTime < 0.85f)
    {
        // Golden Hour → Dusk
        float Alpha = (NormalizedTime - 0.75f) / 0.1f;
        return InterpolatePresets(GoldenHourPreset, NightPreset, Alpha * 0.5f);
    }
    else
    {
        // Dusk → Night
        float Alpha = (NormalizedTime - 0.85f) / 0.15f;
        return InterpolatePresets(GoldenHourPreset, NightPreset, Alpha);
    }
}

FLight_TimePreset ACretaceousLightingSystem::InterpolatePresets(
    const FLight_TimePreset& A, const FLight_TimePreset& B, float Alpha)
{
    FLight_TimePreset Result;
    Result.SunPitch          = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunYaw            = FMath::Lerp(A.SunYaw, B.SunYaw, Alpha);
    Result.SunIntensity      = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor          = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity        = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor          = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f);
    Result.ExposureBias      = FMath::Lerp(A.ExposureBias, B.ExposureBias, Alpha);
    return Result;
}

void ACretaceousLightingSystem::ApplyTimePreset(ELight_TimeOfDay TimeOfDay)
{
    switch (TimeOfDay)
    {
    case ELight_TimeOfDay::GoldenHour:
        TimeOfDayNormalized = 0.7f;
        break;
    case ELight_TimeOfDay::Midday:
        TimeOfDayNormalized = 0.5f;
        break;
    case ELight_TimeOfDay::Dawn:
        TimeOfDayNormalized = 0.25f;
        break;
    case ELight_TimeOfDay::Dusk:
        TimeOfDayNormalized = 0.8f;
        break;
    case ELight_TimeOfDay::Night:
        TimeOfDayNormalized = 0.9f;
        break;
    case ELight_TimeOfDay::Midnight:
        TimeOfDayNormalized = 0.0f;
        break;
    case ELight_TimeOfDay::Morning:
        TimeOfDayNormalized = 0.35f;
        break;
    case ELight_TimeOfDay::Afternoon:
        TimeOfDayNormalized = 0.6f;
        break;
    default:
        break;
    }
    RefreshLighting();
}

void ACretaceousLightingSystem::SetWeather(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;

    if (!SceneFog) return;
    UExponentialHeightFogComponent* FC = SceneFog->GetComponent();
    if (!FC) return;

    switch (CurrentWeather)
    {
    case ELight_WeatherState::Clear:
        FC->SetFogDensity(0.015f);
        break;
    case ELight_WeatherState::Hazy:
        FC->SetFogDensity(0.03f);
        break;
    case ELight_WeatherState::Overcast:
        FC->SetFogDensity(0.05f);
        if (SunLight)
        {
            if (UDirectionalLightComponent* DC = SunLight->GetComponent())
                DC->SetIntensity(6.0f);
        }
        break;
    case ELight_WeatherState::Stormy:
        FC->SetFogDensity(0.08f);
        if (SunLight)
        {
            if (UDirectionalLightComponent* DC = SunLight->GetComponent())
                DC->SetIntensity(3.0f);
        }
        break;
    case ELight_WeatherState::Foggy:
        FC->SetFogDensity(0.12f);
        break;
    }
}

float ACretaceousLightingSystem::GetCurrentSunPitch() const
{
    FLight_TimePreset Preset = GetPresetForTime(TimeOfDayNormalized);
    return Preset.SunPitch;
}

FLinearColor ACretaceousLightingSystem::GetCurrentSkyColor() const
{
    FLight_TimePreset Preset = GetPresetForTime(TimeOfDayNormalized);
    return Preset.SunColor;
}

#if WITH_EDITOR
void ACretaceousLightingSystem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName PropName = PropertyChangedEvent.GetPropertyName();
    if (PropName == GET_MEMBER_NAME_CHECKED(ACretaceousLightingSystem, TimeOfDayNormalized) ||
        PropName == GET_MEMBER_NAME_CHECKED(ACretaceousLightingSystem, CurrentWeather))
    {
        AutoDiscoverLightingActors();
        RefreshLighting();
    }
}
#endif
