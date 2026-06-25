#include "CretaceousLightingManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Math/UnrealMathUtility.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick for performance

    // --- Golden Hour Preset (Default) ---
    GoldenHourConfig.SunPitchDegrees = -38.0f;
    GoldenHourConfig.SunYawDegrees = 45.0f;
    GoldenHourConfig.SunIntensity = 12.0f;
    GoldenHourConfig.SunColor = FLinearColor(1.0f, 0.878f, 0.62f, 1.0f);
    GoldenHourConfig.FogDensity = 0.025f;
    GoldenHourConfig.FogColor = FLinearColor(0.45f, 0.65f, 0.85f, 1.0f);
    GoldenHourConfig.SkyLightIntensity = 2.0f;
    GoldenHourConfig.bVolumetricFog = true;

    // --- Midnight Preset ---
    MidnightConfig.SunPitchDegrees = 15.0f;
    MidnightConfig.SunYawDegrees = 225.0f;
    MidnightConfig.SunIntensity = 0.5f;
    MidnightConfig.SunColor = FLinearColor(0.3f, 0.35f, 0.6f, 1.0f);
    MidnightConfig.FogDensity = 0.04f;
    MidnightConfig.FogColor = FLinearColor(0.05f, 0.08f, 0.15f, 1.0f);
    MidnightConfig.SkyLightIntensity = 0.3f;
    MidnightConfig.bVolumetricFog = true;

    // --- Dawn Preset ---
    DawnConfig.SunPitchDegrees = -5.0f;
    DawnConfig.SunYawDegrees = 90.0f;
    DawnConfig.SunIntensity = 4.0f;
    DawnConfig.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DawnConfig.FogDensity = 0.04f;
    DawnConfig.FogColor = FLinearColor(0.7f, 0.5f, 0.4f, 1.0f);
    DawnConfig.SkyLightIntensity = 0.8f;
    DawnConfig.bVolumetricFog = true;

    // Default weather: clear Cretaceous day
    CurrentWeather.WeatherState = ELight_WeatherState::Clear;
    CurrentWeather.CloudCoverage = 0.1f;
    CurrentWeather.RainIntensity = 0.0f;
    CurrentWeather.FogDensityMultiplier = 1.0f;
    CurrentWeather.LightIntensityMultiplier = 1.0f;
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();

    // Apply Lumen console vars on game start
    ApplyLumenConsoleVars();

    // Apply golden hour by default
    ApplyTimeOfDayPreset(CurrentTimeOfDay);
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        TickDayNightCycle(DeltaTime);
    }

    if (bTransitioningWeather)
    {
        TickWeatherTransition(DeltaTime);
    }
}

void ACretaceousLightingManager::ApplyLumenConsoleVars()
{
    UWorld* World = GetWorld();
    if (!World) return;

    auto ExecCmd = [&](const FString& Cmd)
    {
        GEngine->Exec(World, *Cmd);
    };

    ExecCmd(TEXT("r.Lumen.Reflections.Allow 1"));
    ExecCmd(TEXT("r.Lumen.GlobalIllumination.Allow 1"));
    ExecCmd(TEXT("r.Lumen.HardwareRayTracing 0"));
    ExecCmd(TEXT("r.Lumen.TraceMeshSDFs 1"));
    ExecCmd(TEXT("r.Lumen.DiffuseIndirect.Allow 1"));
    ExecCmd(FString::Printf(TEXT("r.Lumen.GlobalIllumination.MaxTraceDistance %f"), LumenMaxTraceDistance));
    ExecCmd(FString::Printf(TEXT("r.Lumen.Reflections.MaxRoughnessToTrace %f"), LumenReflectionsMaxRoughness));
    ExecCmd(TEXT("r.VolumetricFog 1"));
    ExecCmd(TEXT("r.VolumetricFog.GridPixelSize 8"));
    ExecCmd(TEXT("r.VolumetricFog.GridSizeZ 64"));
    ExecCmd(TEXT("r.VolumetricFog.HistoryWeight 0.9"));
    ExecCmd(TEXT("r.Atmosphere 1"));
    ExecCmd(TEXT("r.SkyAtmosphere 1"));
    ExecCmd(TEXT("r.SkyLight.RealTimeReflectionCapture 1"));
}

void ACretaceousLightingManager::ApplyTimeOfDayPreset(ELight_TimeOfDay TimeOfDay)
{
    CurrentTimeOfDay = TimeOfDay;

    FLight_TimeOfDayConfig Config;
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::GoldenHour:
            Config = GoldenHourConfig;
            CurrentTimeNormalized = 0.72f;
            break;
        case ELight_TimeOfDay::Dawn:
            Config = DawnConfig;
            CurrentTimeNormalized = 0.25f;
            break;
        case ELight_TimeOfDay::Night:
        case ELight_TimeOfDay::Midnight:
            Config = MidnightConfig;
            CurrentTimeNormalized = 0.0f;
            break;
        case ELight_TimeOfDay::Morning:
            Config = LerpConfigs(DawnConfig, GoldenHourConfig, 0.4f);
            CurrentTimeNormalized = 0.35f;
            break;
        case ELight_TimeOfDay::Midday:
        {
            FLight_TimeOfDayConfig MiddayConfig;
            MiddayConfig.SunPitchDegrees = -75.0f;
            MiddayConfig.SunYawDegrees = 180.0f;
            MiddayConfig.SunIntensity = 15.0f;
            MiddayConfig.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
            MiddayConfig.FogDensity = 0.015f;
            MiddayConfig.FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);
            MiddayConfig.SkyLightIntensity = 3.0f;
            Config = MiddayConfig;
            CurrentTimeNormalized = 0.5f;
            break;
        }
        case ELight_TimeOfDay::Afternoon:
            Config = LerpConfigs(GoldenHourConfig, MidnightConfig, 0.2f);
            CurrentTimeNormalized = 0.62f;
            break;
        case ELight_TimeOfDay::Dusk:
            Config = LerpConfigs(GoldenHourConfig, MidnightConfig, 0.5f);
            CurrentTimeNormalized = 0.8f;
            break;
        default:
            Config = GoldenHourConfig;
            break;
    }

    // Apply weather multipliers
    Config.SunIntensity *= CurrentWeather.LightIntensityMultiplier;
    Config.FogDensity *= CurrentWeather.FogDensityMultiplier;

    ApplyConfigToScene(Config);
}

void ACretaceousLightingManager::SetWeatherState(ELight_WeatherState NewWeather, float TransitionTime)
{
    TargetWeather.WeatherState = NewWeather;
    TargetWeather.TransitionDuration = TransitionTime;

    switch (NewWeather)
    {
        case ELight_WeatherState::Clear:
            TargetWeather.CloudCoverage = 0.1f;
            TargetWeather.RainIntensity = 0.0f;
            TargetWeather.FogDensityMultiplier = 1.0f;
            TargetWeather.LightIntensityMultiplier = 1.0f;
            break;
        case ELight_WeatherState::Hazy:
            TargetWeather.CloudCoverage = 0.4f;
            TargetWeather.RainIntensity = 0.0f;
            TargetWeather.FogDensityMultiplier = 2.0f;
            TargetWeather.LightIntensityMultiplier = 0.8f;
            break;
        case ELight_WeatherState::Overcast:
            TargetWeather.CloudCoverage = 0.85f;
            TargetWeather.RainIntensity = 0.0f;
            TargetWeather.FogDensityMultiplier = 1.5f;
            TargetWeather.LightIntensityMultiplier = 0.5f;
            break;
        case ELight_WeatherState::Stormy:
            TargetWeather.CloudCoverage = 1.0f;
            TargetWeather.RainIntensity = 0.8f;
            TargetWeather.FogDensityMultiplier = 3.0f;
            TargetWeather.LightIntensityMultiplier = 0.3f;
            break;
        case ELight_WeatherState::Rain:
            TargetWeather.CloudCoverage = 0.7f;
            TargetWeather.RainIntensity = 0.6f;
            TargetWeather.FogDensityMultiplier = 2.5f;
            TargetWeather.LightIntensityMultiplier = 0.4f;
            break;
        case ELight_WeatherState::Foggy:
            TargetWeather.CloudCoverage = 0.3f;
            TargetWeather.RainIntensity = 0.0f;
            TargetWeather.FogDensityMultiplier = 5.0f;
            TargetWeather.LightIntensityMultiplier = 0.6f;
            break;
    }

    WeatherTransitionElapsed = 0.0f;
    bTransitioningWeather = true;
}

void ACretaceousLightingManager::SetTimeNormalized(float TimeNormalized)
{
    CurrentTimeNormalized = FMath::Clamp(TimeNormalized, 0.0f, 1.0f);
    CurrentTimeOfDay = TimeNormalizedToEnum(CurrentTimeNormalized);
    UpdateSunFromTimeNormalized(CurrentTimeNormalized);
}

void ACretaceousLightingManager::ForceGoldenHour()
{
    ApplyTimeOfDayPreset(ELight_TimeOfDay::GoldenHour);
}

void ACretaceousLightingManager::ForceNight()
{
    ApplyTimeOfDayPreset(ELight_TimeOfDay::Night);
}

// --- PRIVATE ---

void ACretaceousLightingManager::TickDayNightCycle(float DeltaTime)
{
    if (DayDurationSeconds <= 0.0f) return;

    CurrentTimeNormalized += DeltaTime / DayDurationSeconds;
    if (CurrentTimeNormalized > 1.0f) CurrentTimeNormalized -= 1.0f;

    CurrentTimeOfDay = TimeNormalizedToEnum(CurrentTimeNormalized);
    UpdateSunFromTimeNormalized(CurrentTimeNormalized);
}

void ACretaceousLightingManager::TickWeatherTransition(float DeltaTime)
{
    if (TargetWeather.TransitionDuration <= 0.0f)
    {
        CurrentWeather = TargetWeather;
        bTransitioningWeather = false;
        return;
    }

    WeatherTransitionElapsed += DeltaTime;
    float Alpha = FMath::Clamp(WeatherTransitionElapsed / TargetWeather.TransitionDuration, 0.0f, 1.0f);

    CurrentWeather.CloudCoverage = FMath::Lerp(CurrentWeather.CloudCoverage, TargetWeather.CloudCoverage, Alpha);
    CurrentWeather.RainIntensity = FMath::Lerp(CurrentWeather.RainIntensity, TargetWeather.RainIntensity, Alpha);
    CurrentWeather.FogDensityMultiplier = FMath::Lerp(CurrentWeather.FogDensityMultiplier, TargetWeather.FogDensityMultiplier, Alpha);
    CurrentWeather.LightIntensityMultiplier = FMath::Lerp(CurrentWeather.LightIntensityMultiplier, TargetWeather.LightIntensityMultiplier, Alpha);

    if (Alpha >= 1.0f)
    {
        CurrentWeather = TargetWeather;
        bTransitioningWeather = false;
    }
}

void ACretaceousLightingManager::UpdateSunFromTimeNormalized(float T)
{
    FLight_TimeOfDayConfig Config = GetConfigForTime(T);
    Config.SunIntensity *= CurrentWeather.LightIntensityMultiplier;
    Config.FogDensity *= CurrentWeather.FogDensityMultiplier;
    ApplyConfigToScene(Config);
}

FLight_TimeOfDayConfig ACretaceousLightingManager::GetConfigForTime(float T) const
{
    // T: 0=midnight, 0.25=dawn, 0.5=midday, 0.72=golden hour, 1.0=midnight
    if (T < 0.25f)
    {
        float A = T / 0.25f;
        return LerpConfigs(MidnightConfig, DawnConfig, A);
    }
    else if (T < 0.5f)
    {
        float A = (T - 0.25f) / 0.25f;
        return LerpConfigs(DawnConfig, GoldenHourConfig, A);
    }
    else if (T < 0.72f)
    {
        float A = (T - 0.5f) / 0.22f;
        return LerpConfigs(GoldenHourConfig, GoldenHourConfig, A);
    }
    else if (T < 0.9f)
    {
        float A = (T - 0.72f) / 0.18f;
        return LerpConfigs(GoldenHourConfig, MidnightConfig, A);
    }
    else
    {
        float A = (T - 0.9f) / 0.1f;
        return LerpConfigs(MidnightConfig, MidnightConfig, A);
    }
}

FLight_TimeOfDayConfig ACretaceousLightingManager::LerpConfigs(const FLight_TimeOfDayConfig& A, const FLight_TimeOfDayConfig& B, float Alpha) const
{
    FLight_TimeOfDayConfig Out;
    Out.SunPitchDegrees = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
    Out.SunYawDegrees = FMath::Lerp(A.SunYawDegrees, B.SunYawDegrees, Alpha);
    Out.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Out.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Out.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Out.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Out.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Out.bVolumetricFog = A.bVolumetricFog || B.bVolumetricFog;
    return Out;
}

void ACretaceousLightingManager::ApplyConfigToScene(const FLight_TimeOfDayConfig& Config)
{
    // Apply to DirectionalLight
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Config.SunPitchDegrees, Config.SunYawDegrees, 0.0f));
        UDirectionalLightComponent* DL = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DL)
        {
            DL->SetIntensity(Config.SunIntensity);
            DL->SetLightColor(Config.SunColor.ToFColor(true));
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SL = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SL)
        {
            SL->SetIntensity(Config.SkyLightIntensity);
        }
    }

    // Apply to HeightFog
    if (HeightFogActor)
    {
        UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Config.FogDensity);
            FogComp->SetFogInscatteringColor(Config.FogColor.ToFColor(true));
        }
    }
}

ELight_TimeOfDay ACretaceousLightingManager::TimeNormalizedToEnum(float T) const
{
    if (T < 0.15f) return ELight_TimeOfDay::Midnight;
    if (T < 0.28f) return ELight_TimeOfDay::Dawn;
    if (T < 0.42f) return ELight_TimeOfDay::Morning;
    if (T < 0.58f) return ELight_TimeOfDay::Midday;
    if (T < 0.68f) return ELight_TimeOfDay::Afternoon;
    if (T < 0.78f) return ELight_TimeOfDay::GoldenHour;
    if (T < 0.88f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}
