#include "CretaceousLightingManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec — sufficient for smooth transitions

    // Initialize with golden hour as default
    TimeOfDay = 7.5f;
    CurrentDayPhase = ELight_DayPhase::GoldenHour;
    PreviousDayPhase = ELight_DayPhase::GoldenHour;
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultPresets();
    AutoDiscoverLightingActors();
    UpdateLightingForTime(TimeOfDay);
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDayCycleActive)
    {
        AdvanceDayCycle(DeltaTime);
    }
}

void ACretaceousLightingManager::AdvanceDayCycle(float DeltaTime)
{
    // Advance time: DayCycleSpeed = 60 means 1 real second = 1 game minute
    float HoursPerSecond = DayCycleSpeed / 3600.0f;
    TimeOfDay += HoursPerSecond * DeltaTime;

    // Wrap at 24 hours
    if (TimeOfDay >= 24.0f)
    {
        TimeOfDay -= 24.0f;
    }

    UpdateLightingForTime(TimeOfDay);
}

void ACretaceousLightingManager::UpdateLightingForTime(float Time)
{
    ELight_DayPhase NewPhase = ComputeDayPhase(Time);

    if (NewPhase != CurrentDayPhase)
    {
        PreviousDayPhase = CurrentDayPhase;
        CurrentDayPhase = NewPhase;
        OnDayPhaseChanged(CurrentDayPhase, PreviousDayPhase);
    }

    // Get the preset for current phase and apply
    FLight_DayPhasePreset Preset = GetDefaultPreset(CurrentDayPhase);

    // Override with user-defined preset if available
    for (const FLight_DayPhasePreset& P : DayPhasePresets)
    {
        if (P.Phase == CurrentDayPhase)
        {
            Preset = P;
            break;
        }
    }

    ApplyPreset(Preset);
}

ELight_DayPhase ACretaceousLightingManager::ComputeDayPhase(float Time) const
{
    if (Time >= 5.0f && Time < 6.5f)   return ELight_DayPhase::Dawn;
    if (Time >= 6.5f && Time < 9.0f)   return ELight_DayPhase::Morning;
    if (Time >= 9.0f && Time < 11.0f)  return ELight_DayPhase::GoldenHour;
    if (Time >= 11.0f && Time < 15.0f) return ELight_DayPhase::Midday;
    if (Time >= 15.0f && Time < 17.5f) return ELight_DayPhase::Afternoon;
    if (Time >= 17.5f && Time < 19.5f) return ELight_DayPhase::Dusk;
    if (Time >= 19.5f && Time < 22.0f) return ELight_DayPhase::Night;
    return ELight_DayPhase::DeepNight;
}

void ACretaceousLightingManager::ApplyPreset(const FLight_DayPhasePreset& Preset)
{
    // Apply to DirectionalLight (sun)
    if (SunLight)
    {
        FRotator CurrentRot = SunLight->GetActorRotation();
        FRotator NewRot(Preset.SunPitchDegrees, CurrentRot.Yaw, CurrentRot.Roll);
        SunLight->SetActorRotation(NewRot);

        UDirectionalLightComponent* DLComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLComp)
        {
            DLComp->SetIntensity(Preset.SunIntensity);
            DLComp->SetLightColor(Preset.SunColor.ToFColor(true));
        }
    }

    // Apply to SkyLight
    if (SceneSkyLight)
    {
        USkyLightComponent* SLComp = SceneSkyLight->GetComponentByClass<USkyLightComponent>();
        if (SLComp)
        {
            SLComp->SetIntensity(Preset.SkyLightIntensity);
        }
    }

    // Apply to ExponentialHeightFog
    if (SceneFog)
    {
        UExponentialHeightFogComponent* FogComp = SceneFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Preset.FogDensity);
            FogComp->SetFogInscatteringColor(Preset.FogColor);
            FogComp->SetFogHeightFalloff(Preset.FogHeightFalloff);
        }
    }
}

FLight_DayPhasePreset ACretaceousLightingManager::GetDefaultPreset(ELight_DayPhase Phase) const
{
    FLight_DayPhasePreset Preset;
    Preset.Phase = Phase;

    switch (Phase)
    {
    case ELight_DayPhase::Dawn:
        Preset.SunPitchDegrees = -5.0f;
        Preset.SunIntensity = 2.0f;
        Preset.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
        Preset.SkyLightIntensity = 0.8f;
        Preset.FogDensity = 0.06f;
        Preset.FogColor = FLinearColor(0.7f, 0.55f, 0.45f, 1.0f);
        Preset.FogHeightFalloff = 0.12f;
        break;

    case ELight_DayPhase::Morning:
        Preset.SunPitchDegrees = -20.0f;
        Preset.SunIntensity = 5.0f;
        Preset.SunColor = FLinearColor(1.0f, 0.88f, 0.7f, 1.0f);
        Preset.SkyLightIntensity = 1.8f;
        Preset.FogDensity = 0.04f;
        Preset.FogColor = FLinearColor(0.8f, 0.75f, 0.65f, 1.0f);
        Preset.FogHeightFalloff = 0.15f;
        break;

    case ELight_DayPhase::GoldenHour:
        Preset.SunPitchDegrees = -28.0f;
        Preset.SunIntensity = 8.0f;
        Preset.SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);
        Preset.SkyLightIntensity = 2.5f;
        Preset.FogDensity = 0.035f;
        Preset.FogColor = FLinearColor(0.85f, 0.72f, 0.55f, 1.0f);
        Preset.FogHeightFalloff = 0.18f;
        break;

    case ELight_DayPhase::Midday:
        Preset.SunPitchDegrees = -75.0f;
        Preset.SunIntensity = 12.0f;
        Preset.SunColor = FLinearColor(1.0f, 0.97f, 0.9f, 1.0f);
        Preset.SkyLightIntensity = 3.5f;
        Preset.FogDensity = 0.02f;
        Preset.FogColor = FLinearColor(0.75f, 0.82f, 0.9f, 1.0f);
        Preset.FogHeightFalloff = 0.22f;
        break;

    case ELight_DayPhase::Afternoon:
        Preset.SunPitchDegrees = -35.0f;
        Preset.SunIntensity = 7.0f;
        Preset.SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
        Preset.SkyLightIntensity = 2.2f;
        Preset.FogDensity = 0.03f;
        Preset.FogColor = FLinearColor(0.82f, 0.7f, 0.5f, 1.0f);
        Preset.FogHeightFalloff = 0.16f;
        break;

    case ELight_DayPhase::Dusk:
        Preset.SunPitchDegrees = -8.0f;
        Preset.SunIntensity = 3.5f;
        Preset.SunColor = FLinearColor(1.0f, 0.45f, 0.2f, 1.0f);
        Preset.SkyLightIntensity = 1.2f;
        Preset.FogDensity = 0.055f;
        Preset.FogColor = FLinearColor(0.65f, 0.35f, 0.25f, 1.0f);
        Preset.FogHeightFalloff = 0.1f;
        break;

    case ELight_DayPhase::Night:
        Preset.SunPitchDegrees = 20.0f;
        Preset.SunIntensity = 0.3f;
        Preset.SunColor = FLinearColor(0.3f, 0.35f, 0.6f, 1.0f);
        Preset.SkyLightIntensity = 0.4f;
        Preset.FogDensity = 0.07f;
        Preset.FogColor = FLinearColor(0.1f, 0.12f, 0.22f, 1.0f);
        Preset.FogHeightFalloff = 0.08f;
        break;

    case ELight_DayPhase::DeepNight:
        Preset.SunPitchDegrees = 60.0f;
        Preset.SunIntensity = 0.1f;
        Preset.SunColor = FLinearColor(0.2f, 0.22f, 0.45f, 1.0f);
        Preset.SkyLightIntensity = 0.2f;
        Preset.FogDensity = 0.09f;
        Preset.FogColor = FLinearColor(0.05f, 0.06f, 0.15f, 1.0f);
        Preset.FogHeightFalloff = 0.06f;
        break;

    default:
        break;
    }

    return Preset;
}

void ACretaceousLightingManager::InitializeDefaultPresets()
{
    // Populate DayPhasePresets with defaults if empty
    if (DayPhasePresets.Num() == 0)
    {
        TArray<ELight_DayPhase> AllPhases = {
            ELight_DayPhase::Dawn,
            ELight_DayPhase::Morning,
            ELight_DayPhase::GoldenHour,
            ELight_DayPhase::Midday,
            ELight_DayPhase::Afternoon,
            ELight_DayPhase::Dusk,
            ELight_DayPhase::Night,
            ELight_DayPhase::DeepNight,
        };

        for (ELight_DayPhase Phase : AllPhases)
        {
            DayPhasePresets.Add(GetDefaultPreset(Phase));
        }
    }
}

void ACretaceousLightingManager::AutoDiscoverLightingActors()
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
}

void ACretaceousLightingManager::SetTimeOfDay(float NewTime)
{
    TimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateLightingForTime(TimeOfDay);
}

void ACretaceousLightingManager::SetWeatherState(ELight_WeatherState NewWeather)
{
    if (CurrentWeather != NewWeather)
    {
        CurrentWeather = NewWeather;
        OnWeatherChanged(CurrentWeather);
    }
}

void ACretaceousLightingManager::ApplyGoldenHourPreset()
{
    SetTimeOfDay(9.5f);
}

void ACretaceousLightingManager::ApplyMiddayPreset()
{
    SetTimeOfDay(12.0f);
}

void ACretaceousLightingManager::ApplyNightPreset()
{
    SetTimeOfDay(22.0f);
}
