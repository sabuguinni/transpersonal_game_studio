#include "CretaceousLightingManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20fps tick — sufficient for smooth lighting

    InitDefaultPresets();
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();
    DiscoverLightingActors();
    UpdateAtmosphereForTime();
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDayCycleActive)
    {
        UpdateDayCycle(DeltaTime);
    }

    if (bTransitioningWeather)
    {
        WeatherTransitionAlpha += DeltaTime / FMath::Max(WeatherTransitionDuration, 0.1f);
        WeatherTransitionAlpha = FMath::Clamp(WeatherTransitionAlpha, 0.0f, 1.0f);

        FLight_AtmosphereConfig BlendedConfig = InterpolateConfigs(TransitionFromConfig, TransitionToConfig, WeatherTransitionAlpha);
        ApplyAtmosphereConfig(BlendedConfig);

        if (WeatherTransitionAlpha >= 1.0f)
        {
            bTransitioningWeather = false;
            CurrentWeather = TargetWeather;
        }
    }
}

void ACretaceousLightingManager::DiscoverLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Discover DirectionalLight (Sun)
    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        SunActor = *It;
        break;
    }

    // Discover SkyLight
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        SkyLightActor = *It;
        break;
    }

    // Discover ExponentialHeightFog
    for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
    {
        FogActor = *It;
        break;
    }

    // Discover PostProcessVolume
    for (TActorIterator<APostProcessVolume> It(World); It; ++It)
    {
        PostProcessActor = *It;
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: Discovered Sun=%s SkyLight=%s Fog=%s PPV=%s"),
        SunActor ? *SunActor->GetName() : TEXT("MISSING"),
        SkyLightActor ? *SkyLightActor->GetName() : TEXT("MISSING"),
        FogActor ? *FogActor->GetName() : TEXT("MISSING"),
        PostProcessActor ? *PostProcessActor->GetName() : TEXT("MISSING"));
}

void ACretaceousLightingManager::ApplyAtmosphereConfig(const FLight_AtmosphereConfig& Config)
{
    // Apply to DirectionalLight
    if (SunActor)
    {
        FRotator SunRot = SunActor->GetActorRotation();
        SunRot.Pitch = Config.SunPitch;
        SunActor->SetActorRotation(SunRot);

        UDirectionalLightComponent* DirComp = SunActor->GetComponent();
        if (DirComp)
        {
            DirComp->SetIntensity(Config.SunIntensity);
            DirComp->SetLightColor(Config.SunColor);
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        ULightComponent* SkyComp = SkyLightActor->GetLightComponent();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Config.SkyLightIntensity);
        }
    }

    // Apply to ExponentialHeightFog
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Config.FogDensity);
            FogComp->SetFogInscatteringColor(Config.FogColor);
            FogComp->SetVolumetricFog(Config.bVolumetricFog);
        }
    }

    // Apply to PostProcessVolume
    if (PostProcessActor)
    {
        FPostProcessSettings& PPSettings = PostProcessActor->Settings;
        PPSettings.BloomIntensity = Config.BloomIntensity;
        PPSettings.AmbientOcclusionIntensity = Config.AmbientOcclusionIntensity;
        PPSettings.bOverride_BloomIntensity = true;
        PPSettings.bOverride_AmbientOcclusionIntensity = true;
    }
}

void ACretaceousLightingManager::TransitionToWeather(ELight_WeatherState NewWeather, float TransitionDuration)
{
    if (NewWeather == CurrentWeather) return;

    TransitionFromConfig = GetCurrentAtmosphereConfig();
    TargetWeather = NewWeather;

    // Build target config based on weather type
    switch (NewWeather)
    {
    case ELight_WeatherState::Stormy:
        TransitionToConfig = StormConfig;
        break;
    case ELight_WeatherState::Foggy:
        TransitionToConfig = MorningConfig;
        TransitionToConfig.FogDensity = 0.12f;
        TransitionToConfig.SunIntensity = 4.0f;
        break;
    case ELight_WeatherState::Overcast:
        TransitionToConfig = MiddayConfig;
        TransitionToConfig.SunIntensity = 5.0f;
        TransitionToConfig.SkyLightIntensity = 3.0f;
        TransitionToConfig.FogDensity = 0.06f;
        break;
    case ELight_WeatherState::Rainy:
        TransitionToConfig = StormConfig;
        TransitionToConfig.FogDensity = 0.08f;
        TransitionToConfig.FogColor = FLinearColor(0.4f, 0.5f, 0.6f, 1.0f);
        break;
    default:
        TransitionToConfig = GetCurrentAtmosphereConfig();
        break;
    }

    WeatherTransitionAlpha = 0.0f;
    WeatherTransitionDuration = FMath::Max(TransitionDuration, 0.1f);
    bTransitioningWeather = true;
}

void ACretaceousLightingManager::SetTimeOfDay(float NormalizedTime)
{
    TimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    CurrentTimePhase = ResolveTimePhase(TimeOfDayNormalized);
    UpdateAtmosphereForTime();
}

float ACretaceousLightingManager::GetCurrentSunPitch() const
{
    if (SunActor)
    {
        return SunActor->GetActorRotation().Pitch;
    }
    return GetCurrentAtmosphereConfig().SunPitch;
}

FLight_AtmosphereConfig ACretaceousLightingManager::GetCurrentAtmosphereConfig() const
{
    switch (CurrentTimePhase)
    {
    case ELight_TimeOfDay::Dawn:        return DawnConfig;
    case ELight_TimeOfDay::Morning:     return MorningConfig;
    case ELight_TimeOfDay::Midday:      return MiddayConfig;
    case ELight_TimeOfDay::Afternoon:   return MorningConfig; // reuse morning, mirrored
    case ELight_TimeOfDay::Dusk:        return DuskConfig;
    case ELight_TimeOfDay::Night:       return NightConfig;
    case ELight_TimeOfDay::StormApproach: return StormConfig;
    default:                            return MorningConfig;
    }
}

// ── Private ──────────────────────────────────────────────────────────────────

void ACretaceousLightingManager::UpdateDayCycle(float DeltaTime)
{
    // Advance time (DayCycleSpeed=60 → full day in 24 real minutes)
    const float DayDurationSeconds = 24.0f * 60.0f / FMath::Max(DayCycleSpeed, 0.01f);
    TimeOfDayNormalized += DeltaTime / DayDurationSeconds;
    if (TimeOfDayNormalized > 1.0f) TimeOfDayNormalized -= 1.0f;

    ELight_TimeOfDay NewPhase = ResolveTimePhase(TimeOfDayNormalized);
    if (NewPhase != CurrentTimePhase)
    {
        CurrentTimePhase = NewPhase;
    }

    UpdateSunTransform();
    UpdateAtmosphereForTime();
}

void ACretaceousLightingManager::UpdateSunTransform()
{
    if (!SunActor) return;

    // Map normalized time to sun pitch: 0.0=midnight(-90), 0.25=dawn(-15), 0.5=noon(-80), 0.75=dusk(-15), 1.0=midnight(-90)
    float SunAngle = 0.0f;
    if (TimeOfDayNormalized < 0.25f)
    {
        // Night to dawn
        SunAngle = FMath::Lerp(-90.0f, -15.0f, TimeOfDayNormalized / 0.25f);
    }
    else if (TimeOfDayNormalized < 0.5f)
    {
        // Dawn to noon
        SunAngle = FMath::Lerp(-15.0f, -80.0f, (TimeOfDayNormalized - 0.25f) / 0.25f);
    }
    else if (TimeOfDayNormalized < 0.75f)
    {
        // Noon to dusk
        SunAngle = FMath::Lerp(-80.0f, -15.0f, (TimeOfDayNormalized - 0.5f) / 0.25f);
    }
    else
    {
        // Dusk to night
        SunAngle = FMath::Lerp(-15.0f, -90.0f, (TimeOfDayNormalized - 0.75f) / 0.25f);
    }

    FRotator NewRot = SunActor->GetActorRotation();
    NewRot.Pitch = SunAngle;
    SunActor->SetActorRotation(NewRot);
}

void ACretaceousLightingManager::UpdateAtmosphereForTime()
{
    if (!bTransitioningWeather)
    {
        FLight_AtmosphereConfig CurrentConfig = GetCurrentAtmosphereConfig();
        ApplyAtmosphereConfig(CurrentConfig);
    }
}

ELight_TimeOfDay ACretaceousLightingManager::ResolveTimePhase(float NormalizedTime) const
{
    if (NormalizedTime < 0.18f) return ELight_TimeOfDay::Night;
    if (NormalizedTime < 0.25f) return ELight_TimeOfDay::Dawn;
    if (NormalizedTime < 0.38f) return ELight_TimeOfDay::Morning;
    if (NormalizedTime < 0.62f) return ELight_TimeOfDay::Midday;
    if (NormalizedTime < 0.75f) return ELight_TimeOfDay::Afternoon;
    if (NormalizedTime < 0.82f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}

FLight_AtmosphereConfig ACretaceousLightingManager::InterpolateConfigs(
    const FLight_AtmosphereConfig& A,
    const FLight_AtmosphereConfig& B,
    float Alpha) const
{
    FLight_AtmosphereConfig Result;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.bVolumetricFog = Alpha < 0.5f ? A.bVolumetricFog : B.bVolumetricFog;
    Result.BloomIntensity = FMath::Lerp(A.BloomIntensity, B.BloomIntensity, Alpha);
    Result.AmbientOcclusionIntensity = FMath::Lerp(A.AmbientOcclusionIntensity, B.AmbientOcclusionIntensity, Alpha);
    return Result;
}

void ACretaceousLightingManager::InitDefaultPresets()
{
    // Dawn — warm pink/orange horizon, low sun, heavy atmosphere
    DawnConfig.SunPitch = -12.0f;
    DawnConfig.SunIntensity = 3.5f;
    DawnConfig.SunColor = FLinearColor(1.0f, 0.55f, 0.25f, 1.0f);
    DawnConfig.SkyLightIntensity = 0.8f;
    DawnConfig.FogDensity = 0.065f;
    DawnConfig.FogColor = FLinearColor(0.8f, 0.55f, 0.4f, 1.0f);
    DawnConfig.bVolumetricFog = true;
    DawnConfig.BloomIntensity = 0.8f;
    DawnConfig.AmbientOcclusionIntensity = 0.5f;

    // Morning — warm gold, moderate sun, Cretaceous haze
    MorningConfig.SunPitch = -38.0f;
    MorningConfig.SunIntensity = 12.0f;
    MorningConfig.SunColor = FLinearColor(1.0f, 0.878f, 0.62f, 1.0f);
    MorningConfig.SkyLightIntensity = 2.0f;
    MorningConfig.FogDensity = 0.035f;
    MorningConfig.FogColor = FLinearColor(0.55f, 0.72f, 0.85f, 1.0f);
    MorningConfig.bVolumetricFog = true;
    MorningConfig.BloomIntensity = 0.45f;
    MorningConfig.AmbientOcclusionIntensity = 0.6f;

    // Midday — bright white sun, minimal fog, harsh shadows
    MiddayConfig.SunPitch = -78.0f;
    MiddayConfig.SunIntensity = 18.0f;
    MiddayConfig.SunColor = FLinearColor(1.0f, 0.97f, 0.9f, 1.0f);
    MiddayConfig.SkyLightIntensity = 3.5f;
    MiddayConfig.FogDensity = 0.015f;
    MiddayConfig.FogColor = FLinearColor(0.65f, 0.78f, 0.9f, 1.0f);
    MiddayConfig.bVolumetricFog = true;
    MiddayConfig.BloomIntensity = 0.3f;
    MiddayConfig.AmbientOcclusionIntensity = 0.7f;

    // Dusk — deep orange/red, long shadows, dramatic
    DuskConfig.SunPitch = -8.0f;
    DuskConfig.SunIntensity = 4.0f;
    DuskConfig.SunColor = FLinearColor(1.0f, 0.35f, 0.1f, 1.0f);
    DuskConfig.SkyLightIntensity = 0.6f;
    DuskConfig.FogDensity = 0.08f;
    DuskConfig.FogColor = FLinearColor(0.9f, 0.45f, 0.2f, 1.0f);
    DuskConfig.bVolumetricFog = true;
    DuskConfig.BloomIntensity = 1.2f;
    DuskConfig.AmbientOcclusionIntensity = 0.4f;

    // Night — moonlight blue, heavy fog, low visibility
    NightConfig.SunPitch = -90.0f;
    NightConfig.SunIntensity = 0.5f;
    NightConfig.SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    NightConfig.SkyLightIntensity = 0.15f;
    NightConfig.FogDensity = 0.09f;
    NightConfig.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    NightConfig.bVolumetricFog = true;
    NightConfig.BloomIntensity = 0.2f;
    NightConfig.AmbientOcclusionIntensity = 0.8f;

    // Storm — dark green-grey, dense fog, dramatic
    StormConfig.SunPitch = -45.0f;
    StormConfig.SunIntensity = 2.0f;
    StormConfig.SunColor = FLinearColor(0.5f, 0.55f, 0.45f, 1.0f);
    StormConfig.SkyLightIntensity = 0.4f;
    StormConfig.FogDensity = 0.1f;
    StormConfig.FogColor = FLinearColor(0.35f, 0.4f, 0.35f, 1.0f);
    StormConfig.bVolumetricFog = true;
    StormConfig.BloomIntensity = 0.1f;
    StormConfig.AmbientOcclusionIntensity = 0.9f;
}
