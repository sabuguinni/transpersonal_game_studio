#include "LightAtmosphereManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ALightAtmosphereManager::ALightAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update twice per second
}

void ALightAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    FindLightingActors();
    ApplyLightingConfig();
}

void ALightAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (DayCycleSpeed > 0.0f)
    {
        CurrentDayProgress += DeltaTime * DayCycleSpeed * (1.0f / 86400.0f);
        if (CurrentDayProgress > 1.0f) CurrentDayProgress -= 1.0f;
        UpdateSunForTimeOfDay();
    }
}

void ALightAtmosphereManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight
    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        SunActor = *It;
        break;
    }

    // Find SkyLight
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        SkyLightActor = *It;
        break;
    }

    // Find ExponentialHeightFog
    for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
    {
        FogActor = *It;
        break;
    }
}

void ALightAtmosphereManager::ApplyLightingConfig()
{
    // Apply Sun config
    if (SunActor)
    {
        UDirectionalLightComponent* SunComp = SunActor->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(SunConfig.SunIntensity);
            SunComp->SetLightColor(SunConfig.SunColor);
            SunComp->VolumetricScatteringIntensity = SunConfig.VolumetricScatteringIntensity;
        }
        SunActor->SetActorRotation(FRotator(SunConfig.SunPitchAngle, SunConfig.SunYawAngle, 0.0f));
    }

    // Apply SkyLight config
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->Intensity = SkyLightIntensity;
            SkyComp->bRealTimeCapture = true;
        }
    }

    // Apply Fog config
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(FogConfig.FogDensity);
            FogComp->SetFogHeightFalloff(FogConfig.FogHeightFalloff);
            FogComp->SetFogInscatteringColor(FogConfig.FogColor);
            FogComp->SetVolumetricFog(FogConfig.bVolumetricFog);
        }
    }
}

void ALightAtmosphereManager::SetTimeOfDay(ELight_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;
    SunConfig = GetSunConfigForTime(NewTime);
    ApplyLightingConfig();
}

float ALightAtmosphereManager::GetNormalizedTime() const
{
    return CurrentDayProgress;
}

void ALightAtmosphereManager::UpdateSunForTimeOfDay()
{
    // Map day progress (0-1) to sun pitch (-90 to +90 and back)
    float SunAngle = FMath::Sin(CurrentDayProgress * 2.0f * PI) * 90.0f;

    if (SunActor)
    {
        SunActor->SetActorRotation(FRotator(SunAngle, SunConfig.SunYawAngle, 0.0f));
    }

    // Determine time of day from progress
    if (CurrentDayProgress < 0.1f || CurrentDayProgress > 0.9f)
        CurrentTimeOfDay = ELight_TimeOfDay::Night;
    else if (CurrentDayProgress < 0.2f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
    else if (CurrentDayProgress < 0.3f)
        CurrentTimeOfDay = ELight_TimeOfDay::Morning;
    else if (CurrentDayProgress < 0.6f)
        CurrentTimeOfDay = ELight_TimeOfDay::Noon;
    else if (CurrentDayProgress < 0.75f)
        CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;
    else
        CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
}

FLight_SunConfig ALightAtmosphereManager::GetSunConfigForTime(ELight_TimeOfDay Time) const
{
    FLight_SunConfig Config;

    switch (Type)
    {
        case ELight_TimeOfDay::Dawn:
            Config.SunPitchAngle = -10.0f;
            Config.SunIntensity = 3.0f;
            Config.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
            break;

        case ELight_TimeOfDay::Morning:
            Config.SunPitchAngle = -35.0f;
            Config.SunIntensity = 7.0f;
            Config.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
            break;

        case ELight_TimeOfDay::GoldenHour:
            Config.SunPitchAngle = -25.0f;
            Config.SunIntensity = 10.0f;
            Config.SunColor = FLinearColor(1.0f, 0.92f, 0.75f, 1.0f);
            Config.VolumetricScatteringIntensity = 4.0f;
            break;

        case ELight_TimeOfDay::Noon:
            Config.SunPitchAngle = -75.0f;
            Config.SunIntensity = 12.0f;
            Config.SunColor = FLinearColor(1.0f, 1.0f, 0.98f, 1.0f);
            Config.VolumetricScatteringIntensity = 2.0f;
            break;

        case ELight_TimeOfDay::Dusk:
            Config.SunPitchAngle = -8.0f;
            Config.SunIntensity = 4.0f;
            Config.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
            Config.VolumetricScatteringIntensity = 5.0f;
            break;

        case ELight_TimeOfDay::Night:
            Config.SunPitchAngle = 30.0f;
            Config.SunIntensity = 0.5f;
            Config.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
            Config.VolumetricScatteringIntensity = 1.0f;
            break;

        default:
            break;
    }

    return Config;
}
