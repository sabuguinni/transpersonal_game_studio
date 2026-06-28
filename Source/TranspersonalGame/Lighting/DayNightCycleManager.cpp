#include "DayNightCycleManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth transitions
    InitializeDefaultPhaseSettings();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoDiscoverLightingActors();
    ApplyLightingSettings(GetInterpolatedSettings(CurrentTimeOfDay));
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive) return;

    // Advance time — RealDayDurationSeconds maps to 24 hours
    float HoursPerSecond = 24.0f / RealDayDurationSeconds;
    CurrentTimeOfDay += HoursPerSecond * DeltaTime;
    if (CurrentTimeOfDay >= 24.0f) CurrentTimeOfDay -= 24.0f;

    // Update phase
    CurrentPhase = GetPhaseForTime(CurrentTimeOfDay);

    // Apply interpolated lighting
    FLight_DayPhaseSettings Settings = GetInterpolatedSettings(CurrentTimeOfDay);
    ApplyLightingSettings(Settings);
}

ELight_DayPhase ADayNightCycleManager::GetPhaseForTime(float TimeOfDay) const
{
    if (TimeOfDay >= 5.0f  && TimeOfDay < 7.0f)  return ELight_DayPhase::Dawn;
    if (TimeOfDay >= 7.0f  && TimeOfDay < 10.0f) return ELight_DayPhase::Morning;
    if (TimeOfDay >= 10.0f && TimeOfDay < 14.0f) return ELight_DayPhase::Midday;
    if (TimeOfDay >= 14.0f && TimeOfDay < 17.0f) return ELight_DayPhase::Afternoon;
    if (TimeOfDay >= 17.0f && TimeOfDay < 19.0f) return ELight_DayPhase::Dusk;
    if (TimeOfDay >= 19.0f && TimeOfDay < 21.0f) return ELight_DayPhase::Twilight;
    if (TimeOfDay >= 21.0f || TimeOfDay < 2.0f)  return ELight_DayPhase::Night;
    return ELight_DayPhase::DeepNight;
}

FLight_DayPhaseSettings ADayNightCycleManager::GetInterpolatedSettings(float TimeOfDay) const
{
    ELight_DayPhase Phase = GetPhaseForTime(TimeOfDay);
    const FLight_DayPhaseSettings* Current = PhaseSettings.Find(Phase);
    if (!Current)
    {
        FLight_DayPhaseSettings Default;
        return Default;
    }
    return *Current;
}

void ADayNightCycleManager::ApplyLightingSettings(const FLight_DayPhaseSettings& Settings)
{
    // Apply to DirectionalLight (sun)
    if (SunLight)
    {
        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(Settings.SunIntensity);
            DLC->SetLightColor(Settings.SunColor);
        }

        float Pitch = GetSunPitchForTime(CurrentTimeOfDay);
        float Yaw   = GetSunYawForTime(CurrentTimeOfDay);
        SunLight->SetActorRotation(FRotator(Pitch, Yaw, 0.0f));
    }

    // Apply to ExponentialHeightFog
    if (SceneFog)
    {
        UExponentialHeightFogComponent* FogComp = SceneFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }

    // Apply to SkyLight
    if (SceneSkyLight)
    {
        USkyLightComponent* SLC = SceneSkyLight->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(Settings.SkyLightIntensity);
        }
    }
}

void ADayNightCycleManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    CurrentPhase = GetPhaseForTime(CurrentTimeOfDay);
    ApplyLightingSettings(GetInterpolatedSettings(CurrentTimeOfDay));
}

void ADayNightCycleManager::AutoDiscoverLightingActors()
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

    // Find ExponentialHeightFog
    if (!SceneFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            SceneFog = *It;
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
}

float ADayNightCycleManager::GetSunPitchForTime(float TimeOfDay) const
{
    // Sun rises at 6am, peaks at noon (-90 pitch = overhead), sets at 6pm
    // Night: sun is below horizon (positive pitch = below)
    if (TimeOfDay >= 6.0f && TimeOfDay <= 18.0f)
    {
        // Daytime arc: 6am = horizon (0), noon = overhead (-90), 6pm = horizon (0)
        float DayProgress = (TimeOfDay - 6.0f) / 12.0f; // 0 to 1
        float Angle = FMath::Sin(DayProgress * PI) * -90.0f;
        return FMath::Clamp(Angle, -90.0f, -5.0f);
    }
    else
    {
        // Nighttime — sun below horizon
        return 10.0f;
    }
}

float ADayNightCycleManager::GetSunYawForTime(float TimeOfDay) const
{
    // Sun moves from East (90) to West (270) over the day
    float DayProgress = TimeOfDay / 24.0f;
    return 90.0f + DayProgress * 180.0f;
}

FLinearColor ADayNightCycleManager::LerpColor(const FLinearColor& A, const FLinearColor& B, float Alpha) const
{
    return FLinearColor(
        FMath::Lerp(A.R, B.R, Alpha),
        FMath::Lerp(A.G, B.G, Alpha),
        FMath::Lerp(A.B, B.B, Alpha),
        1.0f
    );
}

void ADayNightCycleManager::InitializeDefaultPhaseSettings()
{
    // DAWN (5-7am) — pale pink/orange, hope, vulnerability
    FLight_DayPhaseSettings Dawn;
    Dawn.SunColor       = FLinearColor(1.0f, 0.60f, 0.40f, 1.0f);
    Dawn.SunIntensity   = 3.0f;
    Dawn.FogColor       = FLinearColor(0.80f, 0.65f, 0.70f, 1.0f);
    Dawn.FogDensity     = 0.06f;
    Dawn.SkyLightIntensity = 0.8f;
    PhaseSettings.Add(ELight_DayPhase::Dawn, Dawn);

    // MORNING (7-10am) — warm golden, energy, alertness
    FLight_DayPhaseSettings Morning;
    Morning.SunColor       = FLinearColor(1.0f, 0.85f, 0.60f, 1.0f);
    Morning.SunIntensity   = 6.0f;
    Morning.FogColor       = FLinearColor(0.65f, 0.78f, 0.90f, 1.0f);
    Morning.FogDensity     = 0.04f;
    Morning.SkyLightIntensity = 1.4f;
    PhaseSettings.Add(ELight_DayPhase::Morning, Morning);

    // MIDDAY (10am-2pm) — harsh white, danger, exposure
    FLight_DayPhaseSettings Midday;
    Midday.SunColor       = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    Midday.SunIntensity   = 10.0f;
    Midday.FogColor       = FLinearColor(0.50f, 0.65f, 0.80f, 1.0f);
    Midday.FogDensity     = 0.025f;
    Midday.SkyLightIntensity = 2.2f;
    PhaseSettings.Add(ELight_DayPhase::Midday, Midday);

    // AFTERNOON (2-5pm) — warm amber, fatigue, long shadows
    FLight_DayPhaseSettings Afternoon;
    Afternoon.SunColor       = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);
    Afternoon.SunIntensity   = 8.5f;
    Afternoon.FogColor       = FLinearColor(0.55f, 0.72f, 0.85f, 1.0f);
    Afternoon.FogDensity     = 0.035f;
    Afternoon.SkyLightIntensity = 1.8f;
    PhaseSettings.Add(ELight_DayPhase::Afternoon, Afternoon);

    // DUSK (5-7pm) — deep orange/red, tension, predator time
    FLight_DayPhaseSettings Dusk;
    Dusk.SunColor       = FLinearColor(1.0f, 0.45f, 0.20f, 1.0f);
    Dusk.SunIntensity   = 4.0f;
    Dusk.FogColor       = FLinearColor(0.70f, 0.45f, 0.35f, 1.0f);
    Dusk.FogDensity     = 0.055f;
    Dusk.SkyLightIntensity = 1.0f;
    PhaseSettings.Add(ELight_DayPhase::Dusk, Dusk);

    // TWILIGHT (7-9pm) — deep purple/blue, transition, unease
    FLight_DayPhaseSettings Twilight;
    Twilight.SunColor       = FLinearColor(0.30f, 0.20f, 0.50f, 1.0f);
    Twilight.SunIntensity   = 1.5f;
    Twilight.FogColor       = FLinearColor(0.25f, 0.20f, 0.45f, 1.0f);
    Twilight.FogDensity     = 0.07f;
    Twilight.SkyLightIntensity = 0.5f;
    PhaseSettings.Add(ELight_DayPhase::Twilight, Twilight);

    // NIGHT (9pm-2am) — deep blue/black, terror, survival instinct
    FLight_DayPhaseSettings Night;
    Night.SunColor       = FLinearColor(0.05f, 0.05f, 0.15f, 1.0f);
    Night.SunIntensity   = 0.1f;
    Night.FogColor       = FLinearColor(0.05f, 0.08f, 0.18f, 1.0f);
    Night.FogDensity     = 0.09f;
    Night.SkyLightIntensity = 0.15f;
    PhaseSettings.Add(ELight_DayPhase::Night, Night);

    // DEEP NIGHT (2-5am) — absolute darkness, maximum danger
    FLight_DayPhaseSettings DeepNight;
    DeepNight.SunColor       = FLinearColor(0.02f, 0.02f, 0.08f, 1.0f);
    DeepNight.SunIntensity   = 0.05f;
    DeepNight.FogColor       = FLinearColor(0.02f, 0.03f, 0.10f, 1.0f);
    DeepNight.FogDensity     = 0.12f;
    DeepNight.SkyLightIntensity = 0.08f;
    PhaseSettings.Add(ELight_DayPhase::DeepNight, DeepNight);
}
