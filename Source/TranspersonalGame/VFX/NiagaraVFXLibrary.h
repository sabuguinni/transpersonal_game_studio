// NiagaraVFXLibrary.h
// VFX Agent #17 — Transpersonal Game Studio
// Typed Niagara VFX system library for prehistoric survival game.
// All effects are physically plausible in a real prehistoric world.
// NO spiritual, mystical, or energy-based effects.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraVFXLibrary.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS — VFX Categories
// ─────────────────────────────────────────────────────────────────────────────

/** LOD level for VFX spawning — 3-tier chain as per GDD performance spec */
UENUM(BlueprintType)
enum class EVFX_LODLevel : uint8
{
    High    UMETA(DisplayName = "High (< 20m)"),
    Medium  UMETA(DisplayName = "Medium (20-60m)"),
    Low     UMETA(DisplayName = "Low (60-150m)"),
    Culled  UMETA(DisplayName = "Culled (> 150m)")
};

/** VFX category for budget tracking */
UENUM(BlueprintType)
enum class EVFX_Category : uint8
{
    Environment     UMETA(DisplayName = "Environment"),    // Fire, rain, fog, wind
    Dinosaur        UMETA(DisplayName = "Dinosaur"),       // Footstep, breath, blood
    Combat          UMETA(DisplayName = "Combat"),         // Weapon impacts, wounds
    World           UMETA(DisplayName = "World"),          // Volcano, waterfall, insects
    Weather         UMETA(DisplayName = "Weather")         // Rain, snow, storm
};

// ─────────────────────────────────────────────────────────────────────────────
// STRUCTS — VFX Spawn Parameters
// ─────────────────────────────────────────────────────────────────────────────

/** Parameters for spawning a single VFX instance */
USTRUCT(BlueprintType)
struct FVFX_SpawnParams
{
    GENERATED_BODY()

    /** World location to spawn the effect */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    /** Rotation of the effect (e.g. align dust to surface normal) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator Rotation = FRotator::ZeroRotator;

    /** Scale multiplier (1.0 = default size) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale = 1.0f;

    /** LOD override — if Culled, effect is skipped entirely */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_LODLevel LODLevel = EVFX_LODLevel::High;

    /** Category for budget enforcement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_Category Category = EVFX_Category::Environment;

    /** If true, effect loops until explicitly stopped */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping = false;

    /** Actor to attach this effect to (nullptr = world-space) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    AActor* AttachTarget = nullptr;

    /** Socket name for attachment */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName AttachSocket = NAME_None;
};

/** Runtime VFX instance tracking entry */
USTRUCT(BlueprintType)
struct FVFX_ActiveInstance
{
    GENERATED_BODY()

    /** Unique ID for this instance */
    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    int32 InstanceID = -1;

    /** The spawned Niagara component */
    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    UNiagaraComponent* Component = nullptr;

    /** Category for budget tracking */
    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    EVFX_Category Category = EVFX_Category::Environment;

    /** Time this instance was spawned */
    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    float SpawnTime = 0.0f;

    /** Whether this is a looping effect */
    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    bool bIsLooping = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// UVFX_NiagaraLibrary — Asset Reference Container
// ─────────────────────────────────────────────────────────────────────────────

/**
 * UVFX_NiagaraLibrary
 * Data asset that holds soft references to all Niagara systems used in the game.
 * Assign in Project Settings → Game → VFX Library.
 * All effect names follow convention: NS_[Category]_[Effect]
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_NiagaraLibrary : public UObject
{
    GENERATED_BODY()

public:
    UVFX_NiagaraLibrary();

    // ── ENVIRONMENT ──────────────────────────────────────────────────────────

    /** NS_Fire_Campfire — looping campfire with embers and smoke */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Environment")
    TSoftObjectPtr<UNiagaraSystem> NS_Fire_Campfire;

    /** NS_Fire_CampfireEmbers — ember particles drifting upward */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Environment")
    TSoftObjectPtr<UNiagaraSystem> NS_Fire_CampfireEmbers;

    /** NS_Fire_CampfireSmoke — volumetric smoke column */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Environment")
    TSoftObjectPtr<UNiagaraSystem> NS_Fire_CampfireSmoke;

    /** NS_Wind_VegetationDisturbance — grass/leaf movement from wind */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Environment")
    TSoftObjectPtr<UNiagaraSystem> NS_Wind_VegetationDisturbance;

    // ── WEATHER ──────────────────────────────────────────────────────────────

    /** NS_Weather_Rain — rain streaks with ground splash */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    TSoftObjectPtr<UNiagaraSystem> NS_Weather_Rain;

    /** NS_Weather_Snow — snowfall particles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    TSoftObjectPtr<UNiagaraSystem> NS_Weather_Snow;

    /** NS_Weather_Fog — ground-level fog wisps */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    TSoftObjectPtr<UNiagaraSystem> NS_Weather_Fog;

    /** NS_Weather_Lightning — lightning flash burst */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    TSoftObjectPtr<UNiagaraSystem> NS_Weather_Lightning;

    // ── DINOSAUR ─────────────────────────────────────────────────────────────

    /** NS_Dino_FootstepDust — dust cloud on heavy footstep impact */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_FootstepDust;

    /** NS_Dino_FootstepMud — mud splatter in wet terrain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_FootstepMud;

    /** NS_Dino_BreathVapor — visible breath in cold environments */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_BreathVapor;

    /** NS_Dino_RoarDistortion — air distortion wave from roar */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_RoarDistortion;

    /** NS_Dino_BloodImpact — blood burst on hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_BloodImpact;

    /** NS_Dino_RunningDust — continuous dust trail while running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_RunningDust;

    // ── COMBAT ───────────────────────────────────────────────────────────────

    /** NS_Combat_SpearImpact — debris and dust on spear hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    TSoftObjectPtr<UNiagaraSystem> NS_Combat_SpearImpact;

    /** NS_Combat_ArrowFlight — arrow trail in flight */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    TSoftObjectPtr<UNiagaraSystem> NS_Combat_ArrowFlight;

    /** NS_Combat_RockImpact — rock chip particles on stone strike */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    TSoftObjectPtr<UNiagaraSystem> NS_Combat_RockImpact;

    /** NS_Combat_BloodWound — sustained blood drip from wound */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    TSoftObjectPtr<UNiagaraSystem> NS_Combat_BloodWound;

    /** NS_Combat_CraftingSparks — sparks when knapping flint */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    TSoftObjectPtr<UNiagaraSystem> NS_Combat_CraftingSparks;

    // ── WORLD ────────────────────────────────────────────────────────────────

    /** NS_World_VolcanicAsh — ash particles drifting from eruption */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|World")
    TSoftObjectPtr<UNiagaraSystem> NS_World_VolcanicAsh;

    /** NS_World_WaterfallMist — mist spray at base of waterfall */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|World")
    TSoftObjectPtr<UNiagaraSystem> NS_World_WaterfallMist;

    /** NS_World_RiverRipples — water surface disturbance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|World")
    TSoftObjectPtr<UNiagaraSystem> NS_World_RiverRipples;

    /** NS_World_InsectSwarm — ambient insect particle cloud */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|World")
    TSoftObjectPtr<UNiagaraSystem> NS_World_InsectSwarm;

    /** NS_World_PollenDrift — pollen particles in sunlight */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|World")
    TSoftObjectPtr<UNiagaraSystem> NS_World_PollenDrift;

    /** NS_World_AshFall — volcanic ash falling from sky */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|World")
    TSoftObjectPtr<UNiagaraSystem> NS_World_AshFall;

    // ── UTILITY ──────────────────────────────────────────────────────────────

    /**
     * Get the appropriate LOD system for a given distance.
     * Returns nullptr if distance exceeds cull range (150m).
     */
    UFUNCTION(BlueprintCallable, Category = "VFX|Utility")
    UNiagaraSystem* GetSystemForDistance(TSoftObjectPtr<UNiagaraSystem> HighLOD, float DistanceCm) const;

    /**
     * Compute LOD level from camera distance.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VFX|Utility")
    static EVFX_LODLevel ComputeLODLevel(float DistanceCm);

    /**
     * Per-category budget limits (max simultaneous active instances).
     * Environment=8, Dinosaur=16, Combat=24, World=4, Weather=2
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VFX|Utility")
    static int32 GetBudgetForCategory(EVFX_Category Category);
};
