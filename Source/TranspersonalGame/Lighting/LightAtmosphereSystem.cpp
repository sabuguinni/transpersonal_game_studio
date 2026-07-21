#include "LightAtmosphereSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// ULightAtmosphereComponent
// ============================================================

ULightAtmosphereComponent::ULightAtmosphereComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Golden hour defaults
    GoldenHourConfig.SunPitch = -35.0f;
    GoldenHourConfig.SunIntensity = 8.0f;
    GoldenHourConfig.SunColor = FLinearColor(1.0f, 0.92f, 0.75f, 1.0f);
    GoldenHourConfig.SkyLightIntensity = 2.5f;
    GoldenHourConfig.FogDensity = 0.035f;
    GoldenHourConfig.FogColor = FLinearColor(0.45f, 0.65f, 0.85f, 1.0f);

    // Noon defaults
    NoonConfig.SunPitch = -80.0f;
    NoonConfig.SunIntensity = 12.0f;
    NoonConfig.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
    NoonConfig.SkyLightIntensity = 3.5f;
    NoonConfig.FogDensity = 0.015f;
    NoonConfig.FogColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    // Night defaults
    NightConfig.SunPitch = 15.0f;
    NightConfig.SunIntensity = 0.5f;
    NightConfig.SunColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    NightConfig.SkyLightIntensity = 0.3f;
    NightConfig.FogDensity = 0.06f;
    NightConfig.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
}

void ULightAtmosphereComponent::BeginPlay()
{
    Super::BeginPlay();
    ElapsedTime = CurrentTimeNormalized * DayDurationSeconds;
}

void ULightAtmosphereComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableDayNightCycle)
    {
        AdvanceDayNightCycle(DeltaTime);
    }
}

void ULightAtmosphereComponent::AdvanceDayNightCycle(float DeltaTime)
{
    if (DayDurationSeconds <= 0.0f) return;

    ElapsedTime += DeltaTime;
    if (ElapsedTime > DayDurationSeconds)
    {
        ElapsedTime -= DayDurationSeconds;
    }

    CurrentTimeNormalized = ElapsedTime / DayDurationSeconds;

    // Map normalized time to time of day enum
    if (CurrentTimeNormalized < 0.1f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
    else if (CurrentTimeNormalized < 0.25f)
        CurrentTimeOfDay = ELight_TimeOfDay::Morning;
    else if (CurrentTimeNormalized < 0.45f)
        CurrentTimeOfDay = ELight_TimeOfDay::Noon;
    else if (CurrentTimeNormalized < 0.6f)
        CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeNormalized < 0.75f)
        CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;
    else if (CurrentTimeNormalized < 0.85f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
    else
        CurrentTimeOfDay = ELight_TimeOfDay::Night;
}

void ULightAtmosphereComponent::SetTimeOfDay(ELight_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;

    // Map to normalized time
    switch (NewTime)
    {
    case ELight_TimeOfDay::Dawn:        CurrentTimeNormalized = 0.05f; break;
    case ELight_TimeOfDay::Morning:     CurrentTimeNormalized = 0.18f; break;
    case ELight_TimeOfDay::Noon:        CurrentTimeNormalized = 0.35f; break;
    case ELight_TimeOfDay::Afternoon:   CurrentTimeNormalized = 0.52f; break;
    case ELight_TimeOfDay::GoldenHour:  CurrentTimeNormalized = 0.67f; break;
    case ELight_TimeOfDay::Dusk:        CurrentTimeNormalized = 0.80f; break;
    case ELight_TimeOfDay::Night:       CurrentTimeNormalized = 0.92f; break;
    }

    ElapsedTime = CurrentTimeNormalized * DayDurationSeconds;
}

void ULightAtmosphereComponent::SetWeatherState(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
}

FLight_DayPhaseConfig ULightAtmosphereComponent::GetCurrentPhaseConfig() const
{
    switch (CurrentTimeOfDay)
    {
    case ELight_TimeOfDay::GoldenHour:
    case ELight_TimeOfDay::Dusk:
    case ELight_TimeOfDay::Dawn:
        return GoldenHourConfig;
    case ELight_TimeOfDay::Night:
        return NightConfig;
    default:
        return NoonConfig;
    }
}

float ULightAtmosphereComponent::GetCurrentSunAngle() const
{
    // Sun travels from -90 (below horizon) to -80 (noon) to 15 (night)
    // Simplified: map 0-1 normalized time to pitch angle
    const float NightPitch = 15.0f;
    const float NoonPitch = -80.0f;

    if (CurrentTimeNormalized < 0.5f)
    {
        // Rising: night -> noon
        return FMath::Lerp(NightPitch, NoonPitch, CurrentTimeNormalized * 2.0f);
    }
    else
    {
        // Setting: noon -> night
        return FMath::Lerp(NoonPitch, NightPitch, (CurrentTimeNormalized - 0.5f) * 2.0f);
    }
}

FLight_DayPhaseConfig ULightAtmosphereComponent::InterpolateConfigs(
    const FLight_DayPhaseConfig& A,
    const FLight_DayPhaseConfig& B,
    float Alpha) const
{
    FLight_DayPhaseConfig Result;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    return Result;
}

// ============================================================
// ALightAtmosphereSystem
// ============================================================

ALightAtmosphereSystem::ALightAtmosphereSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    AtmosphereComponent = CreateDefaultSubobject<ULightAtmosphereComponent>(TEXT("AtmosphereComponent"));
}

void ALightAtmosphereSystem::BeginPlay()
{
    Super::BeginPlay();
    // Apply initial golden hour preset on game start
    SetGoldenHourPreset();
}

void ALightAtmosphereSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!AtmosphereComponent || !AtmosphereComponent->bEnableDayNightCycle) return;

    // Apply current config to scene lights every frame
    FLight_DayPhaseConfig Config = AtmosphereComponent->GetCurrentPhaseConfig();
    ApplyLightingConfig(Config);
}

void ALightAtmosphereSystem::ApplyLightingConfig(const FLight_DayPhaseConfig& Config)
{
    // Apply to directional light (sun)
    if (SunLight)
    {
        UDirectionalLightComponent* DLC = SunLight->GetComponent();
        if (DLC)
        {
            DLC->SetIntensity(Config.SunIntensity);
            DLC->SetLightColor(Config.SunColor);
        }
        FRotator CurrentRot = SunLight->GetActorRotation();
        SunLight->SetActorRotation(FRotator(Config.SunPitch, CurrentRot.Yaw, CurrentRot.Roll));
    }

    // Apply to sky light
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetLightComponent();
        if (SLC)
        {
            SLC->SetIntensity(Config.SkyLightIntensity);
        }
    }

    // Apply to fog
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Config.FogDensity);
            FogComp->SetFogInscatteringColor(Config.FogColor);
        }
    }
}

void ALightAtmosphereSystem::SetGoldenHourPreset()
{
    if (!AtmosphereComponent) return;
    AtmosphereComponent->SetTimeOfDay(ELight_TimeOfDay::GoldenHour);
    ApplyLightingConfig(AtmosphereComponent->GoldenHourConfig);
}

void ALightAtmosphereSystem::SetNightPreset()
{
    if (!AtmosphereComponent) return;
    AtmosphereComponent->SetTimeOfDay(ELight_TimeOfDay::Night);
    ApplyLightingConfig(AtmosphereComponent->NightConfig);
}

void ALightAtmosphereSystem::ApplyGoldenHourInEditor()
{
    SetGoldenHourPreset();
}
