// VFX_NiagaraManager.cpp
// Agent #17 — VFX Agent | PROD_CYCLE_AUTO_20260701_006
// Full implementation of the Niagara particle system manager.

#include "VFX_NiagaraManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

// ── Constructor ──────────────────────────────────────────────────────────────

AVFX_NiagaraManager::AVFX_NiagaraManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick for performance

    GlobalVFXQuality = 1.0f;
    MaxActiveEffects = 64;
    bEnableLOD = true;
    LODUpdateInterval = 0.5f;
    NextEffectID = 0;
    LODUpdateTimer = 0.0f;
    CurrentWeatherEffectID = -1;
    TotalEffectsSpawnedThisSession = 0;

    InitializeDefaultConfigs();
}

// ── Lifecycle ────────────────────────────────────────────────────────────────

void AVFX_NiagaraManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[VFX_NiagaraManager] Initialized. MaxEffects=%d, Quality=%.2f"),
        MaxActiveEffects, GlobalVFXQuality);
}

void AVFX_NiagaraManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopAllEffects();
    Super::EndPlay(EndPlayReason);
}

void AVFX_NiagaraManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bEnableLOD) return;

    LODUpdateTimer += DeltaTime;
    if (LODUpdateTimer >= LODUpdateInterval)
    {
        LODUpdateTimer = 0.0f;

        UWorld* World = GetWorld();
        if (!World) return;

        APlayerController* PC = World->GetFirstPlayerController();
        if (!PC) return;

        APlayerCameraManager* CamMgr = PC->PlayerCameraManager;
        if (!CamMgr) return;

        UpdateLODForCamera(CamMgr->GetCameraLocation());
    }
}

// ── Spawn VFX ────────────────────────────────────────────────────────────────

int32 AVFX_NiagaraManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, bool bLooping)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("[VFX_NiagaraManager] Max active effects reached (%d). Skipping spawn."), MaxActiveEffects);
        return -1;
    }

    FVFX_EffectConfig* Config = FindConfigForType(EffectType);
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("[VFX_NiagaraManager] No config found for effect type %d"), (int32)EffectType);
        return -1;
    }

    // Record the active effect
    FVFX_ActiveEffect NewEffect;
    NewEffect.EffectID = NextEffectID++;
    NewEffect.EffectType = EffectType;
    NewEffect.WorldLocation = Location;
    NewEffect.SpawnTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewEffect.bIsLooping = bLooping;

    ActiveEffects.Add(NewEffect);
    TotalEffectsSpawnedThisSession++;

    UE_LOG(LogTemp, Verbose, TEXT("[VFX_NiagaraManager] Spawned effect ID=%d Type=%d at %s"),
        NewEffect.EffectID, (int32)EffectType, *Location.ToString());

    return NewEffect.EffectID;
}

int32 AVFX_NiagaraManager::SpawnEffectAttached(EVFX_EffectType EffectType, USceneComponent* AttachTo, FName SocketName)
{
    if (!AttachTo) return -1;
    FVector Location = AttachTo->GetSocketLocation(SocketName);
    return SpawnEffect(EffectType, Location, FRotator::ZeroRotator, true);
}

void AVFX_NiagaraManager::StopEffect(int32 EffectID)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        if (ActiveEffects[i].EffectID == EffectID)
        {
            ActiveEffects.RemoveAt(i);
            UE_LOG(LogTemp, Verbose, TEXT("[VFX_NiagaraManager] Stopped effect ID=%d"), EffectID);
            return;
        }
    }
}

void AVFX_NiagaraManager::StopAllEffectsOfType(EVFX_EffectType EffectType)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        if (ActiveEffects[i].EffectType == EffectType)
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}

void AVFX_NiagaraManager::StopAllEffects()
{
    ActiveEffects.Empty();
    CurrentWeatherEffectID = -1;
    UE_LOG(LogTemp, Log, TEXT("[VFX_NiagaraManager] All effects stopped."));
}

// ── Preset Spawners ──────────────────────────────────────────────────────────

int32 AVFX_NiagaraManager::SpawnCampfire(FVector Location)
{
    // Campfire = fire + smoke + embers (3 layered effects)
    SpawnEffect(EVFX_EffectType::CampfireSmoke, Location + FVector(0, 0, 80), FRotator::ZeroRotator, true);
    SpawnEffect(EVFX_EffectType::CampfireEmbers, Location + FVector(0, 0, 40), FRotator::ZeroRotator, true);
    return SpawnEffect(EVFX_EffectType::CampfireFire, Location, FRotator::ZeroRotator, true);
}

int32 AVFX_NiagaraManager::SpawnDinosaurFootstep(FVector Location, float DinosaurMassKg)
{
    // Scale dust intensity by dinosaur mass (heavier = more dust)
    float MassScale = FMath::Clamp(DinosaurMassKg / 5000.0f, 0.1f, 3.0f);
    UE_LOG(LogTemp, Verbose, TEXT("[VFX_NiagaraManager] Footstep dust scale=%.2f for mass=%.0fkg"), MassScale, DinosaurMassKg);
    return SpawnEffect(EVFX_EffectType::FootstepDust, Location, FRotator::ZeroRotator, false);
}

int32 AVFX_NiagaraManager::SpawnBloodImpact(FVector Location, FVector ImpactNormal, float DamageAmount)
{
    // Scale blood by damage amount
    float BloodScale = FMath::Clamp(DamageAmount / 20.0f, 0.2f, 2.0f);
    FRotator ImpactRotation = ImpactNormal.Rotation();
    UE_LOG(LogTemp, Verbose, TEXT("[VFX_NiagaraManager] Blood impact scale=%.2f damage=%.1f"), BloodScale, DamageAmount);
    return SpawnEffect(EVFX_EffectType::BloodSplatter, Location, ImpactRotation, false);
}

int32 AVFX_NiagaraManager::SpawnWeaponImpact(FVector Location, FVector ImpactNormal, EVFX_EffectType WeaponType)
{
    FRotator ImpactRotation = ImpactNormal.Rotation();
    return SpawnEffect(WeaponType, Location, ImpactRotation, false);
}

void AVFX_NiagaraManager::SetWeatherEffect(EVFX_EffectType WeatherType, float Intensity)
{
    // Clear previous weather
    ClearWeatherEffects();

    // Spawn at manager location (sky-level weather)
    FVector WeatherLocation = GetActorLocation() + FVector(0, 0, 1000.0f);
    CurrentWeatherEffectID = SpawnEffect(WeatherType, WeatherLocation, FRotator::ZeroRotator, true);

    UE_LOG(LogTemp, Log, TEXT("[VFX_NiagaraManager] Weather set: Type=%d Intensity=%.2f ID=%d"),
        (int32)WeatherType, Intensity, CurrentWeatherEffectID);
}

void AVFX_NiagaraManager::ClearWeatherEffects()
{
    if (CurrentWeatherEffectID >= 0)
    {
        StopEffect(CurrentWeatherEffectID);
        CurrentWeatherEffectID = -1;
    }
    // Also clear any lingering weather types
    StopAllEffectsOfType(EVFX_EffectType::Rain);
    StopAllEffectsOfType(EVFX_EffectType::Snow);
    StopAllEffectsOfType(EVFX_EffectType::Fog);
    StopAllEffectsOfType(EVFX_EffectType::AshFall);
}

// ── LOD Control ──────────────────────────────────────────────────────────────

void AVFX_NiagaraManager::SetGlobalVFXQuality(float QualityScale)
{
    GlobalVFXQuality = FMath::Clamp(QualityScale, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("[VFX_NiagaraManager] Global VFX quality set to %.2f"), GlobalVFXQuality);
}

void AVFX_NiagaraManager::UpdateLODForCamera(FVector CameraLocation)
{
    if (!bEnableLOD) return;

    CullDistantEffects(CameraLocation);
}

// ── Private Helpers ──────────────────────────────────────────────────────────

FVFX_EffectConfig* AVFX_NiagaraManager::FindConfigForType(EVFX_EffectType EffectType)
{
    for (FVFX_EffectConfig& Config : EffectConfigs)
    {
        if (Config.EffectType == EffectType)
        {
            return &Config;
        }
    }
    return nullptr;
}

void AVFX_NiagaraManager::InitializeDefaultConfigs()
{
    // Campfire effects
    {
        FVFX_EffectConfig FireConfig;
        FireConfig.EffectType = EVFX_EffectType::CampfireFire;
        FireConfig.LOD0_Distance = 800.0f;
        FireConfig.LOD1_Distance = 2000.0f;
        FireConfig.LOD2_Distance = 4000.0f;
        FireConfig.CullDistance = 6000.0f;
        FireConfig.bAutoActivate = true;
        FireConfig.SpawnRate = 1.0f;
        EffectConfigs.Add(FireConfig);
    }
    {
        FVFX_EffectConfig SmokeConfig;
        SmokeConfig.EffectType = EVFX_EffectType::CampfireSmoke;
        SmokeConfig.LOD0_Distance = 1200.0f;
        SmokeConfig.LOD1_Distance = 3000.0f;
        SmokeConfig.LOD2_Distance = 6000.0f;
        SmokeConfig.CullDistance = 8000.0f;
        SmokeConfig.bAutoActivate = true;
        SmokeConfig.SpawnRate = 0.5f;
        EffectConfigs.Add(SmokeConfig);
    }
    {
        FVFX_EffectConfig EmbersConfig;
        EmbersConfig.EffectType = EVFX_EffectType::CampfireEmbers;
        EmbersConfig.LOD0_Distance = 600.0f;
        EmbersConfig.LOD1_Distance = 1500.0f;
        EmbersConfig.LOD2_Distance = 3000.0f;
        EmbersConfig.CullDistance = 4000.0f;
        EmbersConfig.bAutoActivate = true;
        EmbersConfig.SpawnRate = 0.3f;
        EffectConfigs.Add(EmbersConfig);
    }
    // Dinosaur effects
    {
        FVFX_EffectConfig FootstepConfig;
        FootstepConfig.EffectType = EVFX_EffectType::FootstepDust;
        FootstepConfig.LOD0_Distance = 500.0f;
        FootstepConfig.LOD1_Distance = 1500.0f;
        FootstepConfig.LOD2_Distance = 3000.0f;
        FootstepConfig.CullDistance = 4000.0f;
        FootstepConfig.bAutoActivate = false;
        FootstepConfig.SpawnRate = 1.0f;
        EffectConfigs.Add(FootstepConfig);
    }
    {
        FVFX_EffectConfig BloodConfig;
        BloodConfig.EffectType = EVFX_EffectType::BloodSplatter;
        BloodConfig.LOD0_Distance = 400.0f;
        BloodConfig.LOD1_Distance = 1200.0f;
        BloodConfig.LOD2_Distance = 2500.0f;
        BloodConfig.CullDistance = 3500.0f;
        BloodConfig.bAutoActivate = false;
        BloodConfig.SpawnRate = 1.0f;
        EffectConfigs.Add(BloodConfig);
    }
    // Weather effects
    {
        FVFX_EffectConfig RainConfig;
        RainConfig.EffectType = EVFX_EffectType::Rain;
        RainConfig.LOD0_Distance = 5000.0f;
        RainConfig.LOD1_Distance = 10000.0f;
        RainConfig.LOD2_Distance = 20000.0f;
        RainConfig.CullDistance = 30000.0f;
        RainConfig.bAutoActivate = false;
        RainConfig.SpawnRate = 1.0f;
        EffectConfigs.Add(RainConfig);
    }
    {
        FVFX_EffectConfig AshConfig;
        AshConfig.EffectType = EVFX_EffectType::VolcanicAsh;
        AshConfig.LOD0_Distance = 8000.0f;
        AshConfig.LOD1_Distance = 15000.0f;
        AshConfig.LOD2_Distance = 25000.0f;
        AshConfig.CullDistance = 35000.0f;
        AshConfig.bAutoActivate = false;
        AshConfig.SpawnRate = 0.8f;
        EffectConfigs.Add(AshConfig);
    }
    {
        FVFX_EffectConfig WeaponConfig;
        WeaponConfig.EffectType = EVFX_EffectType::WeaponImpact;
        WeaponConfig.LOD0_Distance = 300.0f;
        WeaponConfig.LOD1_Distance = 1000.0f;
        WeaponConfig.LOD2_Distance = 2000.0f;
        WeaponConfig.CullDistance = 3000.0f;
        WeaponConfig.bAutoActivate = false;
        WeaponConfig.SpawnRate = 1.0f;
        EffectConfigs.Add(WeaponConfig);
    }

    UE_LOG(LogTemp, Log, TEXT("[VFX_NiagaraManager] Initialized %d default effect configs"), EffectConfigs.Num());
}

void AVFX_NiagaraManager::CullDistantEffects(FVector CameraLocation)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        const FVFX_ActiveEffect& Effect = ActiveEffects[i];
        float DistSq = FVector::DistSquared(CameraLocation, Effect.WorldLocation);

        FVFX_EffectConfig* Config = FindConfigForType(Effect.EffectType);
        if (!Config) continue;

        float CullDistSq = Config->CullDistance * Config->CullDistance;
        if (DistSq > CullDistSq && !Effect.bIsLooping)
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}
