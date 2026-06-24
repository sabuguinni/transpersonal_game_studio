#include "CretaceousLightingManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update every second

    CachedSunActor = nullptr;
    CachedSkyLightActor = nullptr;
    CachedFogActor = nullptr;
    CachedPostProcessVolume = nullptr;
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();
    FindLightingActors();
    ApplyLightingConfiguration();
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (DayNightCycleSpeed > 0.0f)
    {
        CurrentDayProgress += DeltaTime * DayNightCycleSpeed * (1.0f / 86400.0f);
        if (CurrentDayProgress > 1.0f)
        {
            CurrentDayProgress -= 1.0f;
        }

        // Map day progress to time of day
        if (CurrentDayProgress < 0.1f || CurrentDayProgress > 0.9f)
        {
            SetTimeOfDay(ELight_TimeOfDay::Night);
        }
        else if (CurrentDayProgress < 0.2f)
        {
            SetTimeOfDay(ELight_TimeOfDay::Dawn);
        }
        else if (CurrentDayProgress < 0.35f)
        {
            SetTimeOfDay(ELight_TimeOfDay::Morning);
        }
        else if (CurrentDayProgress < 0.6f)
        {
            SetTimeOfDay(ELight_TimeOfDay::Midday);
        }
        else if (CurrentDayProgress < 0.75f)
        {
            SetTimeOfDay(ELight_TimeOfDay::GoldenHour);
        }
        else
        {
            SetTimeOfDay(ELight_TimeOfDay::Dusk);
        }
    }
}

void ACretaceousLightingManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight
    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        CachedSunActor = *It;
        break;
    }

    // Find SkyLight
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        CachedSkyLightActor = *It;
        break;
    }

    // Find ExponentialHeightFog
    for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
    {
        CachedFogActor = *It;
        break;
    }

    // Find PostProcessVolume
    for (TActorIterator<APostProcessVolume> It(World); It; ++It)
    {
        CachedPostProcessVolume = *It;
        break;
    }
}

void ACretaceousLightingManager::ApplyLightingConfiguration()
{
    if (!GetWorld()) return;

    FindLightingActors();

    // Apply sun configuration
    if (CachedSunActor)
    {
        UDirectionalLightComponent* SunComp =
            CachedSunActor->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(SunConfig.SunIntensity);
            SunComp->SetLightColor(SunConfig.SunColor);
            SunComp->bAtmosphereSunLight = SunConfig.bAtmosphereSunLight;
        }
        CachedSunActor->SetActorRotation(
            FRotator(SunConfig.SunPitch, 45.0f, 0.0f));
    }

    // Apply sky light configuration
    if (CachedSkyLightActor)
    {
        USkyLightComponent* SkyComp =
            CachedSkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(SkyLightIntensity);
        }
    }

    // Apply fog configuration
    if (CachedFogActor)
    {
        UExponentialHeightFogComponent* FogComp =
            CachedFogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(FogConfig.FogDensity);
            FogComp->SetFogHeightFalloff(FogConfig.FogHeightFalloff);
            FogComp->SetFogInscatteringColor(FogConfig.FogInscatteringColor);
            FogComp->SetVolumetricFog(FogConfig.bVolumetricFog);
        }
    }
}

void ACretaceousLightingManager::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    UpdateSunForTimeOfDay(NewTimeOfDay);
}

float ACretaceousLightingManager::GetCurrentSunPitch() const
{
    return SunConfig.SunPitch;
}

FString ACretaceousLightingManager::GetTimeOfDayName() const
{
    switch (CurrentTimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:       return TEXT("Dawn");
        case ELight_TimeOfDay::Morning:    return TEXT("Morning");
        case ELight_TimeOfDay::Midday:     return TEXT("Midday");
        case ELight_TimeOfDay::GoldenHour: return TEXT("Golden Hour");
        case ELight_TimeOfDay::Dusk:       return TEXT("Dusk");
        case ELight_TimeOfDay::Night:      return TEXT("Night");
        default:                           return TEXT("Unknown");
    }
}

void ACretaceousLightingManager::SetVolumetricFogEnabled(bool bEnabled)
{
    FogConfig.bVolumetricFog = bEnabled;
    if (CachedFogActor)
    {
        UExponentialHeightFogComponent* FogComp =
            CachedFogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetVolumetricFog(bEnabled);
        }
    }
}

void ACretaceousLightingManager::UpdateSunForTimeOfDay(ELight_TimeOfDay TimeOfDay)
{
    SunConfig.SunPitch     = GetSunPitchForTimeOfDay(TimeOfDay);
    SunConfig.SunColor     = GetSunColorForTimeOfDay(TimeOfDay);
    SunConfig.SunIntensity = GetSunIntensityForTimeOfDay(TimeOfDay);
    ApplyLightingConfiguration();
}

float ACretaceousLightingManager::GetSunPitchForTimeOfDay(ELight_TimeOfDay TimeOfDay) const
{
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:       return -8.0f;
        case ELight_TimeOfDay::Morning:    return -25.0f;
        case ELight_TimeOfDay::Midday:     return -75.0f;
        case ELight_TimeOfDay::GoldenHour: return -38.0f;
        case ELight_TimeOfDay::Dusk:       return -5.0f;
        case ELight_TimeOfDay::Night:      return 30.0f;  // below horizon
        default:                           return -38.0f;
    }
}

FLinearColor ACretaceousLightingManager::GetSunColorForTimeOfDay(ELight_TimeOfDay TimeOfDay) const
{
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            return FLinearColor(1.0f, 0.65f, 0.35f, 1.0f);   // warm orange-pink
        case ELight_TimeOfDay::Morning:
            return FLinearColor(1.0f, 0.92f, 0.75f, 1.0f);   // soft warm white
        case ELight_TimeOfDay::Midday:
            return FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);   // near-white, slightly warm
        case ELight_TimeOfDay::GoldenHour:
            return FLinearColor(1.0f, 0.88f, 0.62f, 1.0f);   // golden amber
        case ELight_TimeOfDay::Dusk:
            return FLinearColor(1.0f, 0.55f, 0.25f, 1.0f);   // deep orange-red
        case ELight_TimeOfDay::Night:
            return FLinearColor(0.1f, 0.15f, 0.35f, 1.0f);   // cool moonlight blue
        default:
            return FLinearColor(1.0f, 0.88f, 0.62f, 1.0f);
    }
}

float ACretaceousLightingManager::GetSunIntensityForTimeOfDay(ELight_TimeOfDay TimeOfDay) const
{
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:       return 4.0f;
        case ELight_TimeOfDay::Morning:    return 8.0f;
        case ELight_TimeOfDay::Midday:     return 15.0f;
        case ELight_TimeOfDay::GoldenHour: return 12.0f;
        case ELight_TimeOfDay::Dusk:       return 3.0f;
        case ELight_TimeOfDay::Night:      return 0.1f;
        default:                           return 12.0f;
    }
}
