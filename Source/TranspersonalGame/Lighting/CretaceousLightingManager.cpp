#include "CretaceousLightingManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth cycle

    InitializeDefaultPresets();
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightActors();
    ApplyCurrentSettings();
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (DayNightConfig.bEnableDynamicCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void ACretaceousLightingManager::InitializeDefaultPresets()
{
    // Golden Hour — warm amber prehistoric sunset
    GoldenHourSettings.SunPitchDegrees = -25.0f;
    GoldenHourSettings.SunYawDegrees = -60.0f;
    GoldenHourSettings.SunIntensity = 8.5f;
    GoldenHourSettings.SunColor = FLinearColor(1.0f, 0.86f, 0.63f, 1.0f);
    GoldenHourSettings.SkyLightIntensity = 1.2f;
    GoldenHourSettings.FogDensity = 0.04f;
    GoldenHourSettings.FogColor = FLinearColor(0.85f, 0.7f, 0.5f, 1.0f);
    GoldenHourSettings.VolumetricFogExtinction = 1.5f;
    GoldenHourSettings.BloomIntensity = 1.2f;
    GoldenHourSettings.VignetteIntensity = 0.4f;

    // Midnight — deep blue prehistoric night
    MidnightSettings.SunPitchDegrees = 60.0f;
    MidnightSettings.SunYawDegrees = 120.0f;
    MidnightSettings.SunIntensity = 0.1f;
    MidnightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    MidnightSettings.SkyLightIntensity = 0.15f;
    MidnightSettings.FogDensity = 0.08f;
    MidnightSettings.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    MidnightSettings.VolumetricFogExtinction = 2.0f;
    MidnightSettings.BloomIntensity = 0.3f;
    MidnightSettings.VignetteIntensity = 0.7f;

    // Storm — dark overcast Cretaceous storm
    StormSettings.SunPitchDegrees = -45.0f;
    StormSettings.SunYawDegrees = -90.0f;
    StormSettings.SunIntensity = 2.0f;
    StormSettings.SunColor = FLinearColor(0.6f, 0.65f, 0.7f, 1.0f);
    StormSettings.SkyLightIntensity = 0.4f;
    StormSettings.FogDensity = 0.12f;
    StormSettings.FogColor = FLinearColor(0.4f, 0.45f, 0.5f, 1.0f);
    StormSettings.VolumetricFogExtinction = 3.0f;
    StormSettings.BloomIntensity = 0.2f;
    StormSettings.VignetteIntensity = 0.6f;
}

void ACretaceousLightingManager::UpdateDayNightCycle(float DeltaTime)
{
    AccumulatedTime += DeltaTime;

    float TotalCycleLength = DayNightConfig.DayLengthSeconds + DayNightConfig.NightLengthSeconds;
    if (TotalCycleLength <= 0.0f) return;

    // Advance normalized time
    float TimeAdvance = DeltaTime / TotalCycleLength;
    CurrentTimeNormalized = FMath::Fmod(CurrentTimeNormalized + TimeAdvance, 1.0f);

    CurrentTimeOfDay = NormalizedTimeToEnum(CurrentTimeNormalized);
    SetTimeOfDay(CurrentTimeNormalized);
}

void ACretaceousLightingManager::SetTimeOfDay(float NormalizedTime)
{
    CurrentTimeNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    CurrentTimeOfDay = NormalizedTimeToEnum(CurrentTimeNormalized);

    if (!SunLight) return;

    // Map normalized time to sun angle (0=midnight, 0.5=noon, 1=midnight)
    // Sun travels from east to west
    float SunAngle = (CurrentTimeNormalized - 0.25f) * 360.0f; // offset so noon is at 0.5
    float SunPitch = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * -80.0f;
    float SunYaw = -60.0f + CurrentTimeNormalized * 180.0f;

    // Clamp pitch to valid range
    SunPitch = FMath::Clamp(SunPitch, -85.0f, 85.0f);

    SunLight->SetActorRotation(FRotator(SunPitch, SunYaw, 0.0f));

    // Interpolate color and intensity based on time
    FLinearColor DayColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    FLinearColor GoldenColor = FLinearColor(1.0f, 0.7f, 0.3f, 1.0f);
    FLinearColor NightColor = FLinearColor(0.2f, 0.25f, 0.5f, 1.0f);

    UDirectionalLightComponent* LightComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (LightComp)
    {
        // Day phase: 0.2 to 0.8
        bool bIsDay = (CurrentTimeNormalized >= 0.2f && CurrentTimeNormalized <= 0.8f);
        bool bIsGoldenHour = (CurrentTimeNormalized >= 0.15f && CurrentTimeNormalized <= 0.25f) ||
                             (CurrentTimeNormalized >= 0.75f && CurrentTimeNormalized <= 0.85f);

        float Intensity = bIsDay ? 8.5f : 0.05f;
        if (bIsGoldenHour) Intensity = 5.0f;

        FLinearColor Color = bIsDay ? DayColor : NightColor;
        if (bIsGoldenHour) Color = GoldenColor;

        LightComp->SetIntensity(Intensity);
        LightComp->SetLightColor(Color);
    }
}

void ACretaceousLightingManager::ApplyGoldenHourPreset()
{
    ApplyTimeOfDaySettings(GoldenHourSettings);
    CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;
    CurrentTimeNormalized = 0.8f;
}

void ACretaceousLightingManager::ApplyMidnightPreset()
{
    ApplyTimeOfDaySettings(MidnightSettings);
    CurrentTimeOfDay = ELight_TimeOfDay::Midnight;
    CurrentTimeNormalized = 0.0f;
}

void ACretaceousLightingManager::ApplyStormPreset()
{
    ApplyTimeOfDaySettings(StormSettings);
    CurrentWeather = ELight_WeatherState::Stormy;
}

void ACretaceousLightingManager::SetWeatherState(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;

    switch (NewWeather)
    {
        case ELight_WeatherState::Stormy:
            ApplyStormPreset();
            break;
        case ELight_WeatherState::Foggy:
        {
            FLight_TimeOfDaySettings FogSettings = GoldenHourSettings;
            FogSettings.FogDensity = 0.15f;
            FogSettings.VolumetricFogExtinction = 4.0f;
            ApplyTimeOfDaySettings(FogSettings);
            break;
        }
        case ELight_WeatherState::Clear:
        {
            FLight_TimeOfDaySettings ClearSettings = GoldenHourSettings;
            ClearSettings.FogDensity = 0.01f;
            ClearSettings.VolumetricFogExtinction = 0.5f;
            ApplyTimeOfDaySettings(ClearSettings);
            break;
        }
        default:
            ApplyTimeOfDaySettings(GoldenHourSettings);
            break;
    }
}

void ACretaceousLightingManager::ApplyTimeOfDaySettings(const FLight_TimeOfDaySettings& Settings)
{
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Settings.SunPitchDegrees, Settings.SunYawDegrees, 0.0f));
        UDirectionalLightComponent* LightComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (LightComp)
        {
            LightComp->SetIntensity(Settings.SunIntensity);
            LightComp->SetLightColor(Settings.SunColor);
        }
    }

    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Settings.SkyLightIntensity);
        }
    }

    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }
}

void ACretaceousLightingManager::ApplyLumenSettings()
{
    if (UWorld* World = GetWorld())
    {
        if (bEnableLumenGI)
        {
            GEngine->Exec(World, TEXT("r.Lumen.GlobalIllumination.Allow 1"));
            GEngine->Exec(World, TEXT("r.DynamicGlobalIlluminationMethod 1"));
        }
        if (bEnableLumenReflections)
        {
            GEngine->Exec(World, TEXT("r.Lumen.Reflections.Allow 1"));
            GEngine->Exec(World, TEXT("r.ReflectionMethod 1"));
        }
        if (bEnableVirtualShadowMaps)
        {
            GEngine->Exec(World, TEXT("r.Shadow.Virtual.Enable 1"));
        }
        GEngine->Exec(World, TEXT("r.VolumetricFog 1"));
        GEngine->Exec(World, TEXT("r.SkyAtmosphere.FastSkyLUT 1"));
    }
}

void ACretaceousLightingManager::AutoFindLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        SunLight = *It;
        break;
    }

    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        SkyLightActor = *It;
        break;
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if ((*It)->GetComponentByClass<UExponentialHeightFogComponent>())
        {
            FogActor = *It;
            break;
        }
    }
}

void ACretaceousLightingManager::ApplyCurrentSettings()
{
    ApplyLumenSettings();
    ApplyGoldenHourPreset();
}

ELight_TimeOfDay ACretaceousLightingManager::NormalizedTimeToEnum(float NormalizedTime) const
{
    if (NormalizedTime < 0.05f || NormalizedTime >= 0.97f) return ELight_TimeOfDay::Midnight;
    if (NormalizedTime < 0.15f) return ELight_TimeOfDay::Night;
    if (NormalizedTime < 0.22f) return ELight_TimeOfDay::Dawn;
    if (NormalizedTime < 0.35f) return ELight_TimeOfDay::Morning;
    if (NormalizedTime < 0.6f)  return ELight_TimeOfDay::Midday;
    if (NormalizedTime < 0.72f) return ELight_TimeOfDay::Afternoon;
    if (NormalizedTime < 0.82f) return ELight_TimeOfDay::GoldenHour;
    if (NormalizedTime < 0.9f)  return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}

FLight_TimeOfDaySettings ACretaceousLightingManager::InterpolateSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunPitchDegrees = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
    Result.SunYawDegrees = FMath::Lerp(A.SunYawDegrees, B.SunYawDegrees, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.VolumetricFogExtinction = FMath::Lerp(A.VolumetricFogExtinction, B.VolumetricFogExtinction, Alpha);
    Result.BloomIntensity = FMath::Lerp(A.BloomIntensity, B.BloomIntensity, Alpha);
    Result.VignetteIntensity = FMath::Lerp(A.VignetteIntensity, B.VignetteIntensity, Alpha);
    return Result;
}

#if WITH_EDITOR
void ACretaceousLightingManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    ApplyCurrentSettings();
}
#endif
