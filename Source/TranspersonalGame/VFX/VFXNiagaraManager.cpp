// VFXNiagaraManager.cpp
// QA Agent #18 — Stub implementation to satisfy RULE 1 (every .h must have a matching .cpp)
// Full Niagara implementation deferred to VFX Agent #17 next cycle.

#include "VFXNiagaraManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UVFXNiagaraManager::UVFXNiagaraManager()
{
    // Default constructor — no Niagara assets loaded at CDO time (null-safe)
    bCampfireActive = false;
    bRainActive = false;
    bDustActive = false;
    CampfireIntensity = 1.0f;
    RainDensity = 0.5f;
    DustOpacity = 0.3f;
}

void UVFXNiagaraManager::InitializeVFXSystems()
{
    // Stub — Niagara system initialization
    // TODO (A#17): Load NiagaraSystem assets and cache component references
    UE_LOG(LogTemp, Log, TEXT("VFXNiagaraManager: InitializeVFXSystems called (stub)"));
}

void UVFXNiagaraManager::SpawnCampfireVFX(FVector Location)
{
    // Stub — spawn campfire particle effect at location
    // TODO (A#17): Spawn NS_Campfire Niagara system at Location
    UE_LOG(LogTemp, Log, TEXT("VFXNiagaraManager: SpawnCampfireVFX at (%f, %f, %f) (stub)"),
        Location.X, Location.Y, Location.Z);
    bCampfireActive = true;
}

void UVFXNiagaraManager::SpawnRainVFX(float Density)
{
    // Stub — activate rain particle system
    // TODO (A#17): Activate NS_Rain Niagara system with Density parameter
    UE_LOG(LogTemp, Log, TEXT("VFXNiagaraManager: SpawnRainVFX density=%f (stub)"), Density);
    RainDensity = Density;
    bRainActive = true;
}

void UVFXNiagaraManager::SpawnDustVFX(FVector Location, float Opacity)
{
    // Stub — spawn dust/impact particle effect
    // TODO (A#17): Spawn NS_DinosaurDust Niagara system at Location
    UE_LOG(LogTemp, Log, TEXT("VFXNiagaraManager: SpawnDustVFX at (%f, %f, %f) opacity=%f (stub)"),
        Location.X, Location.Y, Location.Z, Opacity);
    DustOpacity = Opacity;
    bDustActive = true;
}

void UVFXNiagaraManager::StopAllVFX()
{
    // Stub — deactivate all active particle systems
    // TODO (A#17): Deactivate all cached Niagara components
    UE_LOG(LogTemp, Log, TEXT("VFXNiagaraManager: StopAllVFX called (stub)"));
    bCampfireActive = false;
    bRainActive = false;
    bDustActive = false;
}

void UVFXNiagaraManager::SetCampfireIntensity(float Intensity)
{
    CampfireIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("VFXNiagaraManager: CampfireIntensity set to %f"), CampfireIntensity);
}
