#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraVFXManager.generated.h"

// ============================================================
// EVFX_LODTier — 4-tier LOD for Niagara particle systems
// ============================================================
UENUM(BlueprintType)
enum class EVFX_LODTier : uint8
{
    High    UMETA(DisplayName = "High   (0–1500m)"),
    Medium  UMETA(DisplayName = "Medium (1500–4000m)"),
    Low     UMETA(DisplayName = "Low    (4000–8000m)"),
    Culled  UMETA(DisplayName = "Culled (>8000m)")
};

// ============================================================
// EVFX_EffectType — All prehistoric VFX effect categories
// ============================================================
UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    // Category 1 — Natural Environment
    CampfireFlame       UMETA(DisplayName = "Campfire Flame"),
    CampfireSmoke       UMETA(DisplayName = "Campfire Smoke"),
    CampfireEmbers      UMETA(DisplayName = "Campfire Embers"),
    RainSplash          UMETA(DisplayName = "Rain Splash"),
    VolcanicAsh         UMETA(DisplayName = "Volcanic Ash"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    WindDust            UMETA(DisplayName = "Wind Dust"),

    // Category 2 — Dinosaurs
    DinoFootstepDust    UMETA(DisplayName = "Dino Footstep Dust"),
    DinoBreathVapor     UMETA(DisplayName = "Dino Breath Vapor"),
    DinoBloodSplatter   UMETA(DisplayName = "Dino Blood Splatter"),
    DinoRoarDistortion  UMETA(DisplayName = "Dino Roar Air Distortion"),

    // Category 3 — Player & Combat
    PlayerFootstepDust  UMETA(DisplayName = "Player Footstep Dust"),
    PlayerBreathVapor   UMETA(DisplayName = "Player Breath Vapor"),
    WeaponImpactSpark   UMETA(DisplayName = "Weapon Impact Spark"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter"),
    CraftingSpark       UMETA(DisplayName = "Crafting Spark"),

    // Category 4 — World
    GodRay              UMETA(DisplayName = "God Ray / Volumetric Light"),
    VolcanicEruption    UMETA(DisplayName = "Volcanic Eruption"),
    WaterfallSpray      UMETA(DisplayName = "Waterfall Spray"),
    InsectSwarm         UMETA(DisplayName = "Insect Swarm"),
    PollenDrift         UMETA(DisplayName = "Pollen Drift")
};

// ============================================================
// FVFX_ActiveEffect — Runtime tracking entry for a live effect
// ============================================================
USTRUCT(BlueprintType)
struct FVFX_ActiveEffect
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UNiagaraComponent> NiagaraComponent = nullptr;

    UPROPERTY()
    EVFX_EffectType EffectType = EVFX_EffectType::CampfireFlame;

    UPROPERTY()
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY()
    float SpawnTime = 0.0f;

    UPROPERTY()
    EVFX_LODTier CurrentLOD = EVFX_LODTier::High;
};

// ============================================================
// UVFX_NiagaraVFXManager — World Subsystem
// Manages all Niagara particle effects with LOD and pooling.
// ============================================================
UCLASS(BlueprintType, meta = (DisplayName = "VFX Niagara Manager"))
class TRANSPERSONALGAME_API UVFX_NiagaraVFXManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UVFX_NiagaraVFXManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Spawn / Stop ---
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(
        EVFX_EffectType EffectType,
        const FVector& Location,
        const FRotator& Rotation,
        AActor* AttachTarget = nullptr
    );

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* NiagaraComponent, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects(bool bImmediate = false);

    // --- Registration ---
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterEffectSystem(EVFX_EffectType EffectType, UNiagaraSystem* System);

    // --- LOD ---
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateLOD(const FVector& CameraLocation);

    // --- Queries ---
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VFX")
    int32 GetActiveEffectCount() const;

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LODHighMaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LODMediumMaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LODLowMaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Pool")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bVFXEnabled;

private:
    UPROPERTY()
    TArray<FVFX_ActiveEffect> ActiveEffects;

    UPROPERTY()
    TMap<EVFX_EffectType, TObjectPtr<UNiagaraSystem>> EffectSystemMap;

    EVFX_LODTier ComputeLODTier(float Distance) const;
    void ApplyLODToComponent(UNiagaraComponent* Component, EVFX_LODTier LODTier);
    UNiagaraSystem* GetSystemForEffect(EVFX_EffectType EffectType) const;
    void CullOldestEffect();
};
