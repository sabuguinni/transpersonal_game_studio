#include "VFXManager.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

UVFX_Manager::UVFX_Manager()
{
    PrimaryComponentTick.bCanEverTick = true;

    GlobalVFXQuality = 1.0f;
    bEnableBloodEffects = true;
    bEnableWeatherEffects = true;
    MaxConcurrentEffects = 32;
    CurrentWeatherIntensity = 0.0f;
    ActiveEffectCount = 0;
}

void UVFX_Manager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: Initialized. MaxEffects=%d Quality=%.1f"),
        MaxConcurrentEffects, GlobalVFXQuality);
}

void UVFX_Manager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    CleanupExpiredEffects();
}

void UVFX_Manager::SpawnEffect(const FVFX_EffectConfig& Config)
{
    if (ActiveEffectCount >= MaxConcurrentEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_Manager: Max concurrent effects reached (%d). Skipping."), MaxConcurrentEffects);
        return;
    }

    switch (Config.EffectType)
    {
        case EVFX_EffectType::Campfire:
            SpawnCampfireEffect(Config.SpawnLocation);
            break;
        case EVFX_EffectType::DinoFootstep:
            SpawnDinoFootstepEffect(Config.SpawnLocation, 1000.0f);
            break;
        case EVFX_EffectType::BloodImpact:
            SpawnBloodImpactEffect(Config.SpawnLocation, FVector::UpVector);
            break;
        case EVFX_EffectType::DustCloud:
            SpawnDustCloudEffect(Config.SpawnLocation, Config.Scale * 100.0f);
            break;
        case EVFX_EffectType::Rain:
            SpawnRainEffect(true);
            break;
        default:
            UE_LOG(LogTemp, Log, TEXT("VFX_Manager: SpawnEffect type %d at %s"),
                (int32)Config.EffectType, *Config.SpawnLocation.ToString());
            break;
    }

    ActiveEffectCount++;
}

void UVFX_Manager::SpawnCampfireEffect(FVector Location)
{
    // Campfire VFX: fire + embers + smoke
    // In production: spawn NS_Fire_Campfire Niagara system here
    // Placeholder: log the spawn for now
    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: Campfire effect at %s"), *Location.ToString());

    UWorld* World = GetWorld();
    if (!World) return;

    // TODO: Replace with actual Niagara spawn when NS_Fire_Campfire is created
    // UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, CampfireNiagaraSystem, Location);
}

void UVFX_Manager::SpawnDinoFootstepEffect(FVector Location, float DinoMass)
{
    // Scale dust cloud by dino mass
    float DustRadius = FMath::Clamp(DinoMass / 500.0f, 50.0f, 400.0f);
    float DustIntensity = FMath::Clamp(DinoMass / 1000.0f, 0.1f, 2.0f);

    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: Dino footstep at %s | Mass=%.0f Radius=%.0f Intensity=%.2f"),
        *Location.ToString(), DinoMass, DustRadius, DustIntensity);

    // TODO: Spawn NS_Dino_Footstep Niagara system
    // Camera shake for heavy dinos
    if (DinoMass > 5000.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_Manager: Heavy dino footstep — triggering camera shake"));
    }
}

void UVFX_Manager::SpawnBloodImpactEffect(FVector Location, FVector ImpactNormal)
{
    if (!bEnableBloodEffects) return;

    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: Blood impact at %s | Normal=%s"),
        *Location.ToString(), *ImpactNormal.ToString());

    // TODO: Spawn NS_Combat_BloodImpact Niagara system
    // Decal blood splatter on surface
}

void UVFX_Manager::SpawnRainEffect(bool bEnable)
{
    if (!bEnableWeatherEffects) return;

    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: Rain effect %s"), bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));

    // TODO: Toggle NS_Weather_Rain Niagara system
    // Adjust ExponentialHeightFog density for rain atmosphere
}

void UVFX_Manager::SpawnDustCloudEffect(FVector Location, float Radius)
{
    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: Dust cloud at %s | Radius=%.0f"), *Location.ToString(), Radius);

    // TODO: Spawn NS_Ambient_Dust Niagara system with radius parameter
}

void UVFX_Manager::SetWeatherIntensity(float Intensity)
{
    CurrentWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: Weather intensity set to %.2f"), CurrentWeatherIntensity);

    // Trigger rain if intensity > 0.5
    if (CurrentWeatherIntensity > 0.5f)
    {
        SpawnRainEffect(true);
    }
    else
    {
        SpawnRainEffect(false);
    }
}

void UVFX_Manager::CleanupExpiredEffects()
{
    // Decrement active count over time (simplified — real impl tracks per-effect timers)
    if (ActiveEffectCount > 0)
    {
        // Effects naturally expire — decrement slowly
        // In production: track FTimerHandle per effect and decrement on callback
    }
}
