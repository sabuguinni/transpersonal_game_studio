#include "CretaceousLightingManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// ============================================================
// Constructor
// ============================================================

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;  // 10Hz — sufficient for day/night

    // Default ruin light configs at arch gateway (2000, 2000)
    FLight_RuinLightConfig AltarLight;
    AltarLight.Location = FVector(2000.0f, 2000.0f, 120.0f);
    AltarLight.Color = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
    AltarLight.Intensity = 800.0f;
    AltarLight.AttenuationRadius = 600.0f;
    AltarLight.Temperature = 2800.0f;
    RuinLightConfigs.Add(AltarLight);

    FLight_RuinLightConfig PillarL;
    PillarL.Location = FVector(1850.0f, 2000.0f, 200.0f);
    PillarL.Color = FLinearColor(0.9f, 0.5f, 0.15f, 1.0f);
    PillarL.Intensity = 400.0f;
    PillarL.AttenuationRadius = 500.0f;
    PillarL.Temperature = 2800.0f;
    RuinLightConfigs.Add(PillarL);

    FLight_RuinLightConfig PillarR;
    PillarR.Location = FVector(2150.0f, 2000.0f, 200.0f);
    PillarR.Color = FLinearColor(0.9f, 0.5f, 0.15f, 1.0f);
    PillarR.Intensity = 400.0f;
    PillarR.AttenuationRadius = 500.0f;
    PillarR.Temperature = 2800.0f;
    RuinLightConfigs.Add(PillarR);

    FLight_RuinLightConfig MossA;
    MossA.Location = FVector(2000.0f, 1800.0f, 150.0f);
    MossA.Color = FLinearColor(0.4f, 0.8f, 0.5f, 1.0f);
    MossA.Intensity = 300.0f;
    MossA.AttenuationRadius = 400.0f;
    MossA.Temperature = 4200.0f;
    RuinLightConfigs.Add(MossA);
}

// ============================================================
// BeginPlay
// ============================================================

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();
    FindLightActorsInWorld();
    ApplyTimeOfDay(CurrentTimeOfDay);
    if (bRuinLightsEnabled)
    {
        SpawnRuinLights();
    }
}

// ============================================================
// Tick
// ============================================================

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

// ============================================================
// FindLightActorsInWorld
// ============================================================

void ACretaceousLightingManager::FindLightActorsInWorld()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light
    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        SunLight = *It;
        break;
    }

    // Find sky light
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        SkyLightActor = *It;
        break;
    }

    // Find fog
    for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
    {
        FogActor = *It;
        break;
    }
}

// ============================================================
// GetSettingsForTimeOfDay
// ============================================================

FLight_TimeOfDaySettings ACretaceousLightingManager::GetSettingsForTimeOfDay(ELight_TimeOfDay TimeOfDay) const
{
    FLight_TimeOfDaySettings Settings;

    switch (TimeOfDay)
    {
    case ELight_TimeOfDay::Dawn:
        Settings.SunPitch = -5.0f;
        Settings.SunYaw = 90.0f;
        Settings.SunIntensity = 3.0f;
        Settings.SunColor = FLinearColor(1.0f, 0.65f, 0.4f, 1.0f);
        Settings.SunTemperature = 3200.0f;
        Settings.SkyLightIntensity = 0.8f;
        Settings.FogDensity = 0.06f;
        Settings.FogColor = FLinearColor(0.7f, 0.6f, 0.8f, 1.0f);
        break;

    case ELight_TimeOfDay::Morning:
        Settings.SunPitch = -20.0f;
        Settings.SunYaw = 70.0f;
        Settings.SunIntensity = 6.0f;
        Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.75f, 1.0f);
        Settings.SunTemperature = 5000.0f;
        Settings.SkyLightIntensity = 1.5f;
        Settings.FogDensity = 0.04f;
        Settings.FogColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);
        break;

    case ELight_TimeOfDay::Midday:
        Settings.SunPitch = -75.0f;
        Settings.SunYaw = 0.0f;
        Settings.SunIntensity = 10.0f;
        Settings.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
        Settings.SunTemperature = 6500.0f;
        Settings.SkyLightIntensity = 2.5f;
        Settings.FogDensity = 0.02f;
        Settings.FogColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
        break;

    case ELight_TimeOfDay::GoldenHour:
    default:
        Settings.SunPitch = -35.0f;
        Settings.SunYaw = 45.0f;
        Settings.SunIntensity = 8.0f;
        Settings.SunColor = FLinearColor(1.0f, 0.88f, 0.65f, 1.0f);
        Settings.SunTemperature = 5500.0f;
        Settings.SkyLightIntensity = 2.0f;
        Settings.FogDensity = 0.035f;
        Settings.FogColor = FLinearColor(0.55f, 0.75f, 0.95f, 1.0f);
        break;

    case ELight_TimeOfDay::Dusk:
        Settings.SunPitch = -8.0f;
        Settings.SunYaw = -45.0f;
        Settings.SunIntensity = 4.0f;
        Settings.SunColor = FLinearColor(1.0f, 0.45f, 0.2f, 1.0f);
        Settings.SunTemperature = 2700.0f;
        Settings.SkyLightIntensity = 0.6f;
        Settings.FogDensity = 0.055f;
        Settings.FogColor = FLinearColor(0.8f, 0.4f, 0.3f, 1.0f);
        break;

    case ELight_TimeOfDay::Night:
        Settings.SunPitch = 30.0f;
        Settings.SunYaw = 180.0f;
        Settings.SunIntensity = 0.5f;
        Settings.SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
        Settings.SunTemperature = 8000.0f;
        Settings.SkyLightIntensity = 0.3f;
        Settings.FogDensity = 0.07f;
        Settings.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
        break;
    }

    return Settings;
}

// ============================================================
// ApplySettings
// ============================================================

void ACretaceousLightingManager::ApplySettings(const FLight_TimeOfDaySettings& Settings)
{
    // Sun
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Settings.SunPitch, Settings.SunYaw, 0.0f));
        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(Settings.SunIntensity);
            DLC->SetLightColor(Settings.SunColor);
            DLC->SetTemperature(Settings.SunTemperature);
        }
    }

    // Sky Light
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(Settings.SkyLightIntensity);
        }
    }

    // Fog
    if (FogActor)
    {
        UExponentialHeightFogComponent* FC = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FC)
        {
            FC->SetFogDensity(Settings.FogDensity);
            FC->SetFogInscatteringColor(Settings.FogColor);
        }
    }
}

// ============================================================
// ApplyTimeOfDay
// ============================================================

void ACretaceousLightingManager::ApplyTimeOfDay(ELight_TimeOfDay TimeOfDay)
{
    CurrentTimeOfDay = TimeOfDay;
    FLight_TimeOfDaySettings Settings = GetSettingsForTimeOfDay(TimeOfDay);
    ApplySettings(Settings);
}

// ============================================================
// ApplyWeather
// ============================================================

void ACretaceousLightingManager::ApplyWeather(ELight_WeatherState Weather)
{
    CurrentWeather = Weather;
    if (!FogActor) return;

    UExponentialHeightFogComponent* FC = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
    if (!FC) return;

    switch (Weather)
    {
    case ELight_WeatherState::Clear:
        FC->SetFogDensity(0.02f);
        break;
    case ELight_WeatherState::Overcast:
        FC->SetFogDensity(0.05f);
        if (SunLight)
        {
            UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
            if (DLC) DLC->SetIntensity(4.0f);
        }
        break;
    case ELight_WeatherState::Stormy:
        FC->SetFogDensity(0.08f);
        FC->SetFogInscatteringColor(FLinearColor(0.3f, 0.3f, 0.4f, 1.0f));
        if (SunLight)
        {
            UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
            if (DLC) DLC->SetIntensity(2.0f);
        }
        break;
    case ELight_WeatherState::Foggy:
        FC->SetFogDensity(0.12f);
        FC->SetFogInscatteringColor(FLinearColor(0.8f, 0.85f, 0.9f, 1.0f));
        break;
    case ELight_WeatherState::HeatHaze:
        FC->SetFogDensity(0.03f);
        FC->SetFogInscatteringColor(FLinearColor(0.9f, 0.8f, 0.6f, 1.0f));
        break;
    }
}

// ============================================================
// SetSunPosition
// ============================================================

void ACretaceousLightingManager::SetSunPosition(float Pitch, float Yaw)
{
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Pitch, Yaw, 0.0f));
    }
}

// ============================================================
// SetFogDensity
// ============================================================

void ACretaceousLightingManager::SetFogDensity(float Density)
{
    if (FogActor)
    {
        UExponentialHeightFogComponent* FC = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FC) FC->SetFogDensity(Density);
    }
}

// ============================================================
// EnableLumen
// ============================================================

void ACretaceousLightingManager::EnableLumen(bool bGI, bool bReflections)
{
    bLumenGIEnabled = bGI;
    bLumenReflectionsEnabled = bReflections;
    // Lumen is controlled via console variables and post-process settings
    // These are applied via UE5 console commands at startup
}

// ============================================================
// SpawnRuinLights
// ============================================================

void ACretaceousLightingManager::SpawnRuinLights()
{
    UWorld* World = GetWorld();
    if (!World) return;

    RuinPointLights.Empty();

    for (const FLight_RuinLightConfig& Config : RuinLightConfigs)
    {
        APointLight* PL = World->SpawnActor<APointLight>(Config.Location, FRotator::ZeroRotator);
        if (PL)
        {
            UPointLightComponent* PLC = PL->GetComponentByClass<UPointLightComponent>();
            if (PLC)
            {
                PLC->SetIntensity(Config.Intensity);
                PLC->SetLightColor(Config.Color);
                PLC->SetAttenuationRadius(Config.AttenuationRadius);
                PLC->SetTemperature(Config.Temperature);
                PLC->bUseTemperature = true;
                PLC->CastShadows = true;
            }
            RuinPointLights.Add(PL);
        }
    }
}

// ============================================================
// UpdateDayNightCycle
// ============================================================

void ACretaceousLightingManager::UpdateDayNightCycle(float DeltaTime)
{
    if (DayDurationSeconds <= 0.0f) return;

    CurrentTimeNormalized += DeltaTime / DayDurationSeconds;
    if (CurrentTimeNormalized > 1.0f) CurrentTimeNormalized -= 1.0f;

    // Map normalized time to sun pitch: 0=midnight(90 above), 0.5=noon(-75)
    float SunPitch = FMath::Lerp(90.0f, -75.0f, FMath::Sin(CurrentTimeNormalized * PI));
    float SunYaw = CurrentTimeNormalized * 360.0f;
    SetSunPosition(SunPitch, SunYaw);

    // Determine time of day bucket
    ELight_TimeOfDay NewTimeOfDay;
    if (CurrentTimeNormalized < 0.2f || CurrentTimeNormalized > 0.9f)
        NewTimeOfDay = ELight_TimeOfDay::Night;
    else if (CurrentTimeNormalized < 0.3f)
        NewTimeOfDay = ELight_TimeOfDay::Dawn;
    else if (CurrentTimeNormalized < 0.45f)
        NewTimeOfDay = ELight_TimeOfDay::Morning;
    else if (CurrentTimeNormalized < 0.6f)
        NewTimeOfDay = ELight_TimeOfDay::Midday;
    else if (CurrentTimeNormalized < 0.75f)
        NewTimeOfDay = ELight_TimeOfDay::GoldenHour;
    else
        NewTimeOfDay = ELight_TimeOfDay::Dusk;

    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        ApplyTimeOfDay(NewTimeOfDay);
    }
}

// ============================================================
// CallInEditor presets
// ============================================================

void ACretaceousLightingManager::ApplyGoldenHourPreset()
{
    FindLightActorsInWorld();
    ApplyTimeOfDay(ELight_TimeOfDay::GoldenHour);
}

void ACretaceousLightingManager::ApplyMidnightPreset()
{
    FindLightActorsInWorld();
    ApplyTimeOfDay(ELight_TimeOfDay::Night);
}

void ACretaceousLightingManager::ApplyStormPreset()
{
    FindLightActorsInWorld();
    ApplyTimeOfDay(ELight_TimeOfDay::Midday);
    ApplyWeather(ELight_WeatherState::Stormy);
}

// ============================================================
// PostEditChangeProperty
// ============================================================

#if WITH_EDITOR
void ACretaceousLightingManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    FindLightActorsInWorld();
    ApplyTimeOfDay(CurrentTimeOfDay);
    ApplyWeather(CurrentWeather);
}
#endif
