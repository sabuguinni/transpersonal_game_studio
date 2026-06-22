#include "LightingAtmosphereSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Math/UnrealMathUtility.h"

ALightingAtmosphereSystem::ALightingAtmosphereSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for performance
}

void ALightingAtmosphereSystem::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightActors();
    ApplyAtmosphereSettings();
}

void ALightingAtmosphereSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDayCycleActive)
    {
        UpdateSunPosition(DeltaTime);
    }
}

void ALightingAtmosphereSystem::AutoFindLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light (sun)
    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }

    // Find sky light
    if (!SkyLightActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
        }
    }

    // Find height fog
    if (!HeightFogActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            HeightFogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
    }
}

void ALightingAtmosphereSystem::ApplyAtmosphereSettings()
{
    // Apply sun settings
    if (SunLight)
    {
        UDirectionalLightComponent* DirComp = SunLight->GetComponent();
        if (DirComp)
        {
            DirComp->SetIntensity(AtmosphereSettings.SunIntensity);
            DirComp->SetLightColor(AtmosphereSettings.SunColor);
        }
        // Ensure pitch is negative (sun points down)
        FRotator CurrentRot = SunLight->GetActorRotation();
        float SafePitch = FMath::Min(AtmosphereSettings.SunPitch, -1.0f);
        SunLight->SetActorRotation(FRotator(SafePitch, CurrentRot.Yaw, 0.0f));
    }

    // Apply sky light settings
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
        if (SkyComp)
        {
            SkyComp->SetIntensity(AtmosphereSettings.SkyLightIntensity);
        }
    }

    // Apply fog settings
    if (HeightFogActor)
    {
        UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(AtmosphereSettings.FogDensity);
            FogComp->SetFogHeightFalloff(AtmosphereSettings.FogHeightFalloff);
        }
    }
}

void ALightingAtmosphereSystem::SetTimeOfDay(float NewTime)
{
    TimeOfDayNormalized = FMath::Clamp(NewTime, 0.0f, 1.0f);
    AtmosphereSettings = GetSettingsForTime(TimeOfDayNormalized);
    ApplyAtmosphereSettings();
}

ELight_TimeOfDay ALightingAtmosphereSystem::GetCurrentTimeOfDay() const
{
    if (TimeOfDayNormalized < 0.1f || TimeOfDayNormalized > 0.95f) return ELight_TimeOfDay::Night;
    if (TimeOfDayNormalized < 0.2f) return ELight_TimeOfDay::Dawn;
    if (TimeOfDayNormalized < 0.35f) return ELight_TimeOfDay::Morning;
    if (TimeOfDayNormalized < 0.6f) return ELight_TimeOfDay::Midday;
    if (TimeOfDayNormalized < 0.75f) return ELight_TimeOfDay::Afternoon;
    if (TimeOfDayNormalized < 0.9f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}

void ALightingAtmosphereSystem::UpdateSunPosition(float DeltaTime)
{
    // Advance time
    float DayLengthSeconds = 86400.0f / DayCycleSpeed;
    TimeOfDayNormalized += DeltaTime / DayLengthSeconds;
    if (TimeOfDayNormalized > 1.0f) TimeOfDayNormalized -= 1.0f;

    // Sun arc: pitch goes from -5 (horizon) at dawn to -75 (zenith) at noon, back to -5 at dusk
    // Night: sun is below horizon (positive pitch = underground)
    float SunAngle = TimeOfDayNormalized * 360.0f - 90.0f; // 0=midnight, 90=dawn, 180=noon, 270=dusk
    float SunPitch = -FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 80.0f;
    // Clamp: only negative pitch means sun is above horizon
    bool bIsDay = (SunPitch < 0.0f);

    if (SunLight)
    {
        UDirectionalLightComponent* DirComp = SunLight->GetComponent();
        if (DirComp)
        {
            float Yaw = 45.0f + TimeOfDayNormalized * 90.0f; // Sun sweeps east to west
            SunLight->SetActorRotation(FRotator(SunPitch, Yaw, 0.0f));

            // Intensity: full at noon, zero at night
            float DayFactor = FMath::Clamp(-SunPitch / 80.0f, 0.0f, 1.0f);
            DirComp->SetIntensity(AtmosphereSettings.SunIntensity * DayFactor);

            // Color: warm orange at dawn/dusk, white at midday
            float NoonFactor = FMath::Clamp(DayFactor, 0.0f, 1.0f);
            FLinearColor DawnColor(1.0f, 0.5f, 0.2f, 1.0f);
            FLinearColor NoonColor(1.0f, 0.98f, 0.9f, 1.0f);
            DirComp->SetLightColor(FLinearColor::LerpUsingHSV(DawnColor, NoonColor, NoonFactor));
        }
    }

    // Update fog and sky periodically
    UpdateFogForTimeOfDay();
    UpdateSkyLightForTimeOfDay();
}

void ALightingAtmosphereSystem::UpdateFogForTimeOfDay()
{
    if (!HeightFogActor) return;
    UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponent();
    if (!FogComp) return;

    // More fog at dawn/dusk, less at midday
    ELight_TimeOfDay TOD = GetCurrentTimeOfDay();
    float TargetDensity = AtmosphereSettings.FogDensity;
    switch (TOD)
    {
        case ELight_TimeOfDay::Dawn:
        case ELight_TimeOfDay::Dusk:
            TargetDensity = AtmosphereSettings.FogDensity * 2.5f;
            break;
        case ELight_TimeOfDay::Night:
            TargetDensity = AtmosphereSettings.FogDensity * 1.5f;
            break;
        case ELight_TimeOfDay::Midday:
            TargetDensity = AtmosphereSettings.FogDensity * 0.5f;
            break;
        default:
            break;
    }
    FogComp->SetFogDensity(TargetDensity);
}

void ALightingAtmosphereSystem::UpdateSkyLightForTimeOfDay()
{
    if (!SkyLightActor) return;
    USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
    if (!SkyComp) return;

    ELight_TimeOfDay TOD = GetCurrentTimeOfDay();
    float TargetIntensity = AtmosphereSettings.SkyLightIntensity;
    switch (TOD)
    {
        case ELight_TimeOfDay::Night:
            TargetIntensity = 0.1f;
            break;
        case ELight_TimeOfDay::Dawn:
        case ELight_TimeOfDay::Dusk:
            TargetIntensity = AtmosphereSettings.SkyLightIntensity * 0.6f;
            break;
        case ELight_TimeOfDay::Midday:
            TargetIntensity = AtmosphereSettings.SkyLightIntensity * 1.2f;
            break;
        default:
            break;
    }
    SkyComp->SetIntensity(TargetIntensity);
}

FLight_AtmosphereSettings ALightingAtmosphereSystem::GetSettingsForTime(float NormalizedTime) const
{
    FLight_AtmosphereSettings Settings;

    // Dawn (0.1 - 0.2)
    if (NormalizedTime >= 0.1f && NormalizedTime < 0.2f)
    {
        Settings.SunIntensity = 3.0f;
        Settings.SunPitch = -10.0f;
        Settings.SkyLightIntensity = 0.8f;
        Settings.FogDensity = 0.05f;
        Settings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    }
    // Midday (0.35 - 0.6)
    else if (NormalizedTime >= 0.35f && NormalizedTime < 0.6f)
    {
        Settings.SunIntensity = 12.0f;
        Settings.SunPitch = -75.0f;
        Settings.SkyLightIntensity = 2.0f;
        Settings.FogDensity = 0.01f;
        Settings.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    }
    // Dusk (0.75 - 0.9)
    else if (NormalizedTime >= 0.75f && NormalizedTime < 0.9f)
    {
        Settings.SunIntensity = 4.0f;
        Settings.SunPitch = -8.0f;
        Settings.SkyLightIntensity = 0.7f;
        Settings.FogDensity = 0.04f;
        Settings.SunColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
    }
    // Night
    else if (NormalizedTime < 0.1f || NormalizedTime >= 0.9f)
    {
        Settings.SunIntensity = 0.0f;
        Settings.SunPitch = 30.0f; // Below horizon
        Settings.SkyLightIntensity = 0.1f;
        Settings.FogDensity = 0.03f;
        Settings.SunColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    }

    return Settings;
}
