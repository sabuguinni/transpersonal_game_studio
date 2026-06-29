#include "LightingAtmosphereManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ALightingAtmosphereManager::ALightingAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20 fps tick — lightweight

    InitDefaultPhaseConfigs();
}

void ALightingAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightingActors();
    ApplyPhaseConfig(GetCurrentPhase());
}

void ALightingAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bDayNightCycleActive)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void ALightingAtmosphereManager::InitDefaultPhaseConfigs()
{
    // Dawn — amber/orange, low sun, heavy fog
    FLight_SunConfig DawnConfig;
    DawnConfig.SunPitch = -8.0f;
    DawnConfig.SunIntensity = 3.0f;
    DawnConfig.SunColor = FLinearColor(1.0f, 0.55f, 0.15f, 1.0f);
    DawnConfig.FogDensity = 0.04f;
    DawnConfig.FogInscatteringColor = FLinearColor(0.8f, 0.5f, 0.2f, 1.0f);
    PhaseConfigs.Add(ELight_TimeOfDay::Dawn, DawnConfig);

    // Morning — warm gold, rising sun
    FLight_SunConfig MorningConfig;
    MorningConfig.SunPitch = -30.0f;
    MorningConfig.SunIntensity = 8.0f;
    MorningConfig.SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    MorningConfig.FogDensity = 0.025f;
    MorningConfig.FogInscatteringColor = FLinearColor(0.6f, 0.65f, 0.8f, 1.0f);
    PhaseConfigs.Add(ELight_TimeOfDay::Morning, MorningConfig);

    // Midday — white/neutral, high sun
    FLight_SunConfig MiddayConfig;
    MiddayConfig.SunPitch = -75.0f;
    MiddayConfig.SunIntensity = 15.0f;
    MiddayConfig.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
    MiddayConfig.FogDensity = 0.01f;
    MiddayConfig.FogInscatteringColor = FLinearColor(0.5f, 0.6f, 0.9f, 1.0f);
    PhaseConfigs.Add(ELight_TimeOfDay::Midday, MiddayConfig);

    // Afternoon — golden hour approaching
    FLight_SunConfig AfternoonConfig;
    AfternoonConfig.SunPitch = -40.0f;
    AfternoonConfig.SunIntensity = 10.0f;
    AfternoonConfig.SunColor = FLinearColor(1.0f, 0.75f, 0.4f, 1.0f);
    AfternoonConfig.FogDensity = 0.02f;
    AfternoonConfig.FogInscatteringColor = FLinearColor(0.7f, 0.55f, 0.35f, 1.0f);
    PhaseConfigs.Add(ELight_TimeOfDay::Afternoon, AfternoonConfig);

    // Dusk — deep orange/red, very low sun
    FLight_SunConfig DuskConfig;
    DuskConfig.SunPitch = -5.0f;
    DuskConfig.SunIntensity = 2.0f;
    DuskConfig.SunColor = FLinearColor(1.0f, 0.3f, 0.05f, 1.0f);
    DuskConfig.FogDensity = 0.05f;
    DuskConfig.FogInscatteringColor = FLinearColor(0.9f, 0.35f, 0.1f, 1.0f);
    PhaseConfigs.Add(ELight_TimeOfDay::Dusk, DuskConfig);

    // Night — deep blue, moonlight simulation
    FLight_SunConfig NightConfig;
    NightConfig.SunPitch = -90.0f; // Below horizon
    NightConfig.SunIntensity = 0.5f;
    NightConfig.SunColor = FLinearColor(0.15f, 0.2f, 0.5f, 1.0f);
    NightConfig.FogDensity = 0.03f;
    NightConfig.FogInscatteringColor = FLinearColor(0.05f, 0.08f, 0.25f, 1.0f);
    PhaseConfigs.Add(ELight_TimeOfDay::Night, NightConfig);
}

void ALightingAtmosphereManager::UpdateDayNightCycle(float DeltaTime)
{
    // Advance time
    const float DayLengthSeconds = 86400.0f / DayNightCycleSpeed;
    TimeOfDayNormalized += DeltaTime / DayLengthSeconds;
    if (TimeOfDayNormalized >= 1.0f)
    {
        TimeOfDayNormalized -= 1.0f;
    }

    ELight_TimeOfDay NewPhase = NormalizedTimeToPhase(TimeOfDayNormalized);
    if (NewPhase != CurrentPhase)
    {
        CurrentPhase = NewPhase;
        ApplyPhaseConfig(CurrentPhase);
    }

    UpdateSunRotation();
}

void ALightingAtmosphereManager::UpdateSunRotation()
{
    if (!SunLight) return;

    // Map normalized time to sun pitch: 0.0 = midnight (-90), 0.25 = dawn (-5), 0.5 = noon (-75), 0.75 = dusk (-5), 1.0 = midnight (-90)
    float SunAngle = 0.0f;
    if (TimeOfDayNormalized < 0.25f)
    {
        // Midnight to dawn
        SunAngle = FMath::Lerp(-90.0f, -5.0f, TimeOfDayNormalized / 0.25f);
    }
    else if (TimeOfDayNormalized < 0.5f)
    {
        // Dawn to noon
        SunAngle = FMath::Lerp(-5.0f, -75.0f, (TimeOfDayNormalized - 0.25f) / 0.25f);
    }
    else if (TimeOfDayNormalized < 0.75f)
    {
        // Noon to dusk
        SunAngle = FMath::Lerp(-75.0f, -5.0f, (TimeOfDayNormalized - 0.5f) / 0.25f);
    }
    else
    {
        // Dusk to midnight
        SunAngle = FMath::Lerp(-5.0f, -90.0f, (TimeOfDayNormalized - 0.75f) / 0.25f);
    }

    FRotator CurrentRot = SunLight->GetActorRotation();
    SunLight->SetActorRotation(FRotator(SunAngle, CurrentRot.Yaw, CurrentRot.Roll));
}

void ALightingAtmosphereManager::UpdateFogForPhase(const FLight_SunConfig& Config)
{
    if (!HeightFog) return;

    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    if (!FogComp) return;

    FogComp->SetFogDensity(Config.FogDensity);
    FogComp->SetFogInscatteringColor(Config.FogInscatteringColor);
}

ELight_TimeOfDay ALightingAtmosphereManager::NormalizedTimeToPhase(float T) const
{
    if (T < 0.1f || T >= 0.95f) return ELight_TimeOfDay::Night;
    if (T < 0.2f)  return ELight_TimeOfDay::Dawn;
    if (T < 0.35f) return ELight_TimeOfDay::Morning;
    if (T < 0.6f)  return ELight_TimeOfDay::Midday;
    if (T < 0.75f) return ELight_TimeOfDay::Afternoon;
    return ELight_TimeOfDay::Dusk;
}

ELight_TimeOfDay ALightingAtmosphereManager::GetCurrentPhase() const
{
    return NormalizedTimeToPhase(TimeOfDayNormalized);
}

void ALightingAtmosphereManager::SetTimeOfDay(float NormalizedTime)
{
    TimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    CurrentPhase = GetCurrentPhase();
    ApplyPhaseConfig(CurrentPhase);
    UpdateSunRotation();
}

void ALightingAtmosphereManager::ApplyPhaseConfig(ELight_TimeOfDay Phase)
{
    const FLight_SunConfig* Config = PhaseConfigs.Find(Phase);
    if (!Config) return;

    // Apply sun settings
    if (SunLight)
    {
        UDirectionalLightComponent* DirComp = SunLight->GetComponent();
        if (DirComp)
        {
            DirComp->SetIntensity(Config->SunIntensity);
            DirComp->SetLightColor(Config->SunColor);
        }
    }

    // Apply fog settings
    UpdateFogForPhase(*Config);
}

void ALightingAtmosphereManager::AutoFindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light (sun)
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
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

    // Find height fog
    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }
}
