// VFXSystemManager.cpp
// Integration Agent #19 — PROD_CYCLE_AUTO_20260701_003
// Implements VFXSystemManager stub to pair with VFXSystemManager.h (Agent #17)
// Prehistoric survival game — particle/VFX orchestration

#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UVFXSystemManager::UVFXSystemManager()
{
    bIsInitialized = false;
    ActiveEffectCount = 0;
    MaxConcurrentEffects = 64;
}

void UVFXSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bIsInitialized = true;
    ActiveEffectCount = 0;
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager initialized — max concurrent effects: %d"), MaxConcurrentEffects);
}

void UVFXSystemManager::Deinitialize()
{
    StopAllEffects();
    bIsInitialized = false;
    Super::Deinitialize();
}

void UVFXSystemManager::SpawnDustEffect(const FVector& Location, float Scale)
{
    if (!bIsInitialized) return;
    if (ActiveEffectCount >= MaxConcurrentEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXSystemManager: Max concurrent effects reached (%d)"), MaxConcurrentEffects);
        return;
    }
    // Dust effect — triggered when dinosaur footstep or player runs on dirt
    ActiveEffectCount++;
    UE_LOG(LogTemp, Verbose, TEXT("VFXSystemManager: SpawnDustEffect at (%.1f, %.1f, %.1f) scale=%.2f"),
        Location.X, Location.Y, Location.Z, Scale);
}

void UVFXSystemManager::SpawnBloodSplatter(const FVector& Location, const FVector& ImpactNormal)
{
    if (!bIsInitialized) return;
    if (ActiveEffectCount >= MaxConcurrentEffects) return;
    ActiveEffectCount++;
    UE_LOG(LogTemp, Verbose, TEXT("VFXSystemManager: SpawnBloodSplatter at (%.1f, %.1f, %.1f)"),
        Location.X, Location.Y, Location.Z);
}

void UVFXSystemManager::SpawnFireEffect(const FVector& Location, float Radius, float Duration)
{
    if (!bIsInitialized) return;
    if (ActiveEffectCount >= MaxConcurrentEffects) return;
    ActiveEffectCount++;
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager: SpawnFireEffect at (%.1f, %.1f, %.1f) r=%.1f dur=%.1f"),
        Location.X, Location.Y, Location.Z, Radius, Duration);
}

void UVFXSystemManager::SpawnRainEffect(float Intensity)
{
    if (!bIsInitialized) return;
    // Global rain — single persistent effect, intensity 0.0-1.0
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager: SpawnRainEffect intensity=%.2f"), Intensity);
}

void UVFXSystemManager::SpawnFootprintEffect(const FVector& Location, bool bIsHeavyCreature)
{
    if (!bIsInitialized) return;
    float scale = bIsHeavyCreature ? 3.0f : 1.0f;
    SpawnDustEffect(Location, scale);
}

void UVFXSystemManager::StopAllEffects()
{
    ActiveEffectCount = 0;
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager: All effects stopped"));
}

int32 UVFXSystemManager::GetActiveEffectCount() const
{
    return ActiveEffectCount;
}

bool UVFXSystemManager::IsInitialized() const
{
    return bIsInitialized;
}
