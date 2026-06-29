#include "LightDayNightCycle.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ALightDayNightCycle::ALightDayNightCycle()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20 ticks/sec is enough for lighting
    InitializePresets();
}

void ALightDayNightCycle::BeginPlay()
{
    Super::BeginPlay();
    AutoFindSceneReferences();
    // Apply initial lighting
    FLight_TimeOfDaySettings InitialSettings = GetSettingsForTime(TimeOfDayNormalized);
    ApplyLightingSettings(InitialSettings);
}

void ALightDayNightCycle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive || DayCycleSpeed <= 0.0f)
    {
        return;
    }

    // Advance time: DayCycleSpeed = game-seconds per real-second
    // A full day = 86400 game-seconds
    float DeltaNormalized = (DeltaTime * DayCycleSpeed) / 86400.0f;
    TimeOfDayNormalized = FMath::Fmod(TimeOfDayNormalized + DeltaNormalized, 1.0f);

    FLight_TimeOfDaySettings CurrentSettings = GetSettingsForTime(TimeOfDayNormalized);
    ApplyLightingSettings(CurrentSettings);
}

ELight_TimeOfDay ALightDayNightCycle::GetCurrentTimeOfDay() const
{
    // Map normalized time to enum
    // 0.0 = midnight, 0.25 = 6am, 0.5 = noon, 0.75 = 6pm, 1.0 = midnight
    if (TimeOfDayNormalized < 0.083f) return ELight_TimeOfDay::Midnight;
    if (TimeOfDayNormalized < 0.208f) return ELight_TimeOfDay::Night;
    if (TimeOfDayNormalized < 0.292f) return ELight_TimeOfDay::Dawn;
    if (TimeOfDayNormalized < 0.417f) return ELight_TimeOfDay::Morning;
    if (TimeOfDayNormalized < 0.542f) return ELight_TimeOfDay::Midday;
    if (TimeOfDayNormalized < 0.667f) return ELight_TimeOfDay::Afternoon;
    if (TimeOfDayNormalized < 0.750f) return ELight_TimeOfDay::GoldenHour;
    if (TimeOfDayNormalized < 0.833f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}

void ALightDayNightCycle::SetTimeOfDay(float NormalizedTime)
{
    TimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    FLight_TimeOfDaySettings Settings = GetSettingsForTime(TimeOfDayNormalized);
    ApplyLightingSettings(Settings);
}

void ALightDayNightCycle::JumpToTimeOfDay(ELight_TimeOfDay TargetTime)
{
    // Map enum to normalized time
    switch (TargetTime)
    {
        case ELight_TimeOfDay::Midnight:   SetTimeOfDay(0.0f);   break;
        case ELight_TimeOfDay::Night:      SetTimeOfDay(0.125f); break;
        case ELight_TimeOfDay::Dawn:       SetTimeOfDay(0.25f);  break;
        case ELight_TimeOfDay::Morning:    SetTimeOfDay(0.333f); break;
        case ELight_TimeOfDay::Midday:     SetTimeOfDay(0.5f);   break;
        case ELight_TimeOfDay::Afternoon:  SetTimeOfDay(0.583f); break;
        case ELight_TimeOfDay::GoldenHour: SetTimeOfDay(0.708f); break;
        case ELight_TimeOfDay::Dusk:       SetTimeOfDay(0.792f); break;
        default: break;
    }
}

void ALightDayNightCycle::ApplyLightingSettings(const FLight_TimeOfDaySettings& Settings)
{
    UpdateSunLight(Settings);
    UpdateFog(Settings);
}

void ALightDayNightCycle::AutoFindSceneReferences()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find sun directional light
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    // Find height fog
    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }

    // Find sky light
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }
}

void ALightDayNightCycle::UpdateSunLight(const FLight_TimeOfDaySettings& Settings)
{
    if (!SunLight) return;

    SunLight->SetActorRotation(FRotator(Settings.SunPitch, Settings.SunYaw, 0.0f));

    UDirectionalLightComponent* DLC = SunLight->GetComponent();
    if (DLC)
    {
        DLC->SetIntensity(Settings.SunIntensity);
        DLC->SetLightColor(Settings.SunColor);
    }
}

void ALightDayNightCycle::UpdateFog(const FLight_TimeOfDaySettings& Settings)
{
    if (!HeightFog) return;

    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    if (FogComp)
    {
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogComp->SetFogInscatteringColor(Settings.FogInscatteringColor);
    }
}

FLight_TimeOfDaySettings ALightDayNightCycle::InterpolateSettings(
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
    Result.FogInscatteringColor = FLinearColor(
        FMath::Lerp(A.FogInscatteringColor.R, B.FogInscatteringColor.R, Alpha),
        FMath::Lerp(A.FogInscatteringColor.G, B.FogInscatteringColor.G, Alpha),
        FMath::Lerp(A.FogInscatteringColor.B, B.FogInscatteringColor.B, Alpha),
        1.0f
    );
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.VolumetricFogExtinctionScale = FMath::Lerp(A.VolumetricFogExtinctionScale, B.VolumetricFogExtinctionScale, Alpha);
    Result.VolumetricFogAlbedo = FLinearColor(
        FMath::Lerp(A.VolumetricFogAlbedo.R, B.VolumetricFogAlbedo.R, Alpha),
        FMath::Lerp(A.VolumetricFogAlbedo.G, B.VolumetricFogAlbedo.G, Alpha),
        FMath::Lerp(A.VolumetricFogAlbedo.B, B.VolumetricFogAlbedo.B, Alpha),
        1.0f
    );
    return Result;
}

FLight_TimeOfDaySettings ALightDayNightCycle::GetSettingsForTime(float NormalizedTime) const
{
    if (TimeOfDayPresets.Num() < 2)
    {
        // Return default midday if no presets
        FLight_TimeOfDaySettings Default;
        return Default;
    }

    // Find the two surrounding presets and interpolate
    // Presets are ordered by their TimeOfDay enum (0=Dawn...7=Midnight)
    int32 NumPresets = TimeOfDayPresets.Num();
    float SegmentSize = 1.0f / (float)NumPresets;
    int32 IndexA = FMath::FloorToInt(NormalizedTime / SegmentSize) % NumPresets;
    int32 IndexB = (IndexA + 1) % NumPresets;
    float Alpha = FMath::Fmod(NormalizedTime, SegmentSize) / SegmentSize;

    return InterpolateSettings(TimeOfDayPresets[IndexA], TimeOfDayPresets[IndexB], Alpha);
}

void ALightDayNightCycle::InitializePresets()
{
    TimeOfDayPresets.Empty();

    // Dawn preset
    FLight_TimeOfDaySettings Dawn;
    Dawn.TimeOfDay = ELight_TimeOfDay::Dawn;
    Dawn.SunPitch = -5.0f;
    Dawn.SunYaw = 90.0f;
    Dawn.SunIntensity = 2.0f;
    Dawn.SunColor = FLinearColor(1.0f, 0.55f, 0.25f, 1.0f);
    Dawn.FogInscatteringColor = FLinearColor(0.85f, 0.55f, 0.35f, 1.0f);
    Dawn.FogDensity = 0.045f;
    Dawn.FogHeightFalloff = 0.12f;
    Dawn.VolumetricFogExtinctionScale = 1.8f;
    Dawn.VolumetricFogAlbedo = FLinearColor(0.95f, 0.75f, 0.55f, 1.0f);
    TimeOfDayPresets.Add(Dawn);

    // Morning preset
    FLight_TimeOfDaySettings Morning;
    Morning.TimeOfDay = ELight_TimeOfDay::Morning;
    Morning.SunPitch = -30.0f;
    Morning.SunYaw = 120.0f;
    Morning.SunIntensity = 7.0f;
    Morning.SunColor = FLinearColor(1.0f, 0.88f, 0.72f, 1.0f);
    Morning.FogInscatteringColor = FLinearColor(0.7f, 0.78f, 0.9f, 1.0f);
    Morning.FogDensity = 0.025f;
    Morning.FogHeightFalloff = 0.18f;
    Morning.VolumetricFogExtinctionScale = 1.2f;
    Morning.VolumetricFogAlbedo = FLinearColor(0.9f, 0.88f, 0.82f, 1.0f);
    TimeOfDayPresets.Add(Morning);

    // Midday preset
    FLight_TimeOfDaySettings Midday;
    Midday.TimeOfDay = ELight_TimeOfDay::Midday;
    Midday.SunPitch = -75.0f;
    Midday.SunYaw = 180.0f;
    Midday.SunIntensity = 12.0f;
    Midday.SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    Midday.FogInscatteringColor = FLinearColor(0.55f, 0.68f, 0.88f, 1.0f);
    Midday.FogDensity = 0.015f;
    Midday.FogHeightFalloff = 0.22f;
    Midday.VolumetricFogExtinctionScale = 0.8f;
    Midday.VolumetricFogAlbedo = FLinearColor(0.88f, 0.9f, 0.95f, 1.0f);
    TimeOfDayPresets.Add(Midday);

    // Afternoon preset
    FLight_TimeOfDaySettings Afternoon;
    Afternoon.TimeOfDay = ELight_TimeOfDay::Afternoon;
    Afternoon.SunPitch = -45.0f;
    Afternoon.SunYaw = 220.0f;
    Afternoon.SunIntensity = 9.0f;
    Afternoon.SunColor = FLinearColor(1.0f, 0.92f, 0.75f, 1.0f);
    Afternoon.FogInscatteringColor = FLinearColor(0.65f, 0.72f, 0.85f, 1.0f);
    Afternoon.FogDensity = 0.02f;
    Afternoon.FogHeightFalloff = 0.2f;
    Afternoon.VolumetricFogExtinctionScale = 1.0f;
    Afternoon.VolumetricFogAlbedo = FLinearColor(0.9f, 0.88f, 0.78f, 1.0f);
    TimeOfDayPresets.Add(Afternoon);

    // Golden Hour preset
    FLight_TimeOfDaySettings GoldenHour;
    GoldenHour.TimeOfDay = ELight_TimeOfDay::GoldenHour;
    GoldenHour.SunPitch = -12.0f;
    GoldenHour.SunYaw = 270.0f;
    GoldenHour.SunIntensity = 5.5f;
    GoldenHour.SunColor = FLinearColor(1.0f, 0.62f, 0.18f, 1.0f);
    GoldenHour.FogInscatteringColor = FLinearColor(0.85f, 0.52f, 0.22f, 1.0f);
    GoldenHour.FogDensity = 0.035f;
    GoldenHour.FogHeightFalloff = 0.18f;
    GoldenHour.VolumetricFogExtinctionScale = 1.4f;
    GoldenHour.VolumetricFogAlbedo = FLinearColor(0.92f, 0.78f, 0.55f, 1.0f);
    TimeOfDayPresets.Add(GoldenHour);

    // Dusk preset
    FLight_TimeOfDaySettings Dusk;
    Dusk.TimeOfDay = ELight_TimeOfDay::Dusk;
    Dusk.SunPitch = -2.0f;
    Dusk.SunYaw = 290.0f;
    Dusk.SunIntensity = 1.5f;
    Dusk.SunColor = FLinearColor(0.95f, 0.38f, 0.12f, 1.0f);
    Dusk.FogInscatteringColor = FLinearColor(0.72f, 0.35f, 0.28f, 1.0f);
    Dusk.FogDensity = 0.05f;
    Dusk.FogHeightFalloff = 0.14f;
    Dusk.VolumetricFogExtinctionScale = 2.0f;
    Dusk.VolumetricFogAlbedo = FLinearColor(0.88f, 0.62f, 0.42f, 1.0f);
    TimeOfDayPresets.Add(Dusk);

    // Night preset
    FLight_TimeOfDaySettings Night;
    Night.TimeOfDay = ELight_TimeOfDay::Night;
    Night.SunPitch = 20.0f; // Below horizon
    Night.SunYaw = 0.0f;
    Night.SunIntensity = 0.0f;
    Night.SunColor = FLinearColor(0.1f, 0.15f, 0.35f, 1.0f);
    Night.FogInscatteringColor = FLinearColor(0.05f, 0.08f, 0.22f, 1.0f);
    Night.FogDensity = 0.06f;
    Night.FogHeightFalloff = 0.1f;
    Night.VolumetricFogExtinctionScale = 2.5f;
    Night.VolumetricFogAlbedo = FLinearColor(0.15f, 0.18f, 0.35f, 1.0f);
    TimeOfDayPresets.Add(Night);

    // Midnight preset
    FLight_TimeOfDaySettings Midnight;
    Midnight.TimeOfDay = ELight_TimeOfDay::Midnight;
    Midnight.SunPitch = 45.0f; // Deep below horizon
    Midnight.SunYaw = 0.0f;
    Midnight.SunIntensity = 0.0f;
    Midnight.SunColor = FLinearColor(0.05f, 0.08f, 0.2f, 1.0f);
    Midnight.FogInscatteringColor = FLinearColor(0.02f, 0.04f, 0.12f, 1.0f);
    Midnight.FogDensity = 0.07f;
    Midnight.FogHeightFalloff = 0.08f;
    Midnight.VolumetricFogExtinctionScale = 3.0f;
    Midnight.VolumetricFogAlbedo = FLinearColor(0.08f, 0.1f, 0.22f, 1.0f);
    TimeOfDayPresets.Add(Midnight);
}
