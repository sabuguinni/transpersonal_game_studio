#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFXNiagaraController.generated.h"

// Forward declarations — Niagara module (added to Build.cs separately)
class UNiagaraSystem;
class UNiagaraComponent;

// ============================================================
// EVFX_NiagaraEffectType — all Niagara effect categories
// ============================================================
UENUM(BlueprintType)
enum class EVFX_NiagaraEffectType : uint8
{
    None            UMETA(DisplayName = "None"),
    // CATEGORY 1 — ENVIRONMENT
    CampfireFire    UMETA(DisplayName = "Campfire Fire"),
    CampfireSmoke   UMETA(DisplayName = "Campfire Smoke"),
    CampfireEmbers  UMETA(DisplayName = "Campfire Embers"),
    RainFall        UMETA(DisplayName = "Rain Fall"),
    SnowFall        UMETA(DisplayName = "Snow Fall"),
    Fog             UMETA(DisplayName = "Ground Fog"),
    WindParticles   UMETA(DisplayName = "Wind Particles"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash"),
    // CATEGORY 2 — DINOSAURS
    DinoFootstepDust    UMETA(DisplayName = "Dino Footstep Dust"),
    DinoBreathVapor     UMETA(DisplayName = "Dino Breath Vapor"),
    DinoBloodImpact     UMETA(DisplayName = "Dino Blood Impact"),
    DinoRoarAirwave     UMETA(DisplayName = "Dino Roar Airwave"),
    // CATEGORY 3 — PLAYER & COMBAT
    WeaponImpactDirt    UMETA(DisplayName = "Weapon Impact Dirt"),
    WeaponImpactBlood   UMETA(DisplayName = "Weapon Impact Blood"),
    CraftingSparkFlint  UMETA(DisplayName = "Crafting Spark Flint"),
    CraftingSmokeCook   UMETA(DisplayName = "Crafting Smoke Cook"),
    // CATEGORY 4 — WORLD
    WaterfallSpray      UMETA(DisplayName = "Waterfall Spray"),
    GodRays             UMETA(DisplayName = "God Rays"),
    InsectSwarm         UMETA(DisplayName = "Insect Swarm"),
    PollenDrift         UMETA(DisplayName = "Pollen Drift"),
};

// ============================================================
// EVFX_NiagaraLODLevel — 3-level LOD chain
// ============================================================
UENUM(BlueprintType)
enum class EVFX_NiagaraLODLevel : uint8
{
    High    UMETA(DisplayName = "High (< 1500 UU)"),
    Medium  UMETA(DisplayName = "Medium (1500-4000 UU)"),
    Low     UMETA(DisplayName = "Low (4000-8000 UU)"),
    Culled  UMETA(DisplayName = "Culled (> 8000 UU)"),
};

// ============================================================
// FVFX_NiagaraEffectSlot — runtime tracking of a live effect
// ============================================================
USTRUCT(BlueprintType)
struct FVFX_NiagaraEffectSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    EVFX_NiagaraEffectType EffectType = EVFX_NiagaraEffectType::None;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    EVFX_NiagaraLODLevel CurrentLOD = EVFX_NiagaraLODLevel::High;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    float SpawnTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    bool bIsLooping = false;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    float Duration = -1.0f; // -1 = infinite loop

    // Raw pointer — Niagara component (not UPROPERTY to avoid cross-module issues)
    UNiagaraComponent* NiagaraComp = nullptr;
};

// ============================================================
// FVFX_NiagaraSpawnRequest — data to spawn a new effect
// ============================================================
USTRUCT(BlueprintType)
struct FVFX_NiagaraSpawnRequest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    EVFX_NiagaraEffectType EffectType = EVFX_NiagaraEffectType::None;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector::OneVector;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    bool bLooping = false;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    float Duration = 3.0f;

    UPROPERTY(BlueprintReadWrite, Category = "VFX")
    float IntensityScale = 1.0f;
};

// ============================================================
// AVFX_NiagaraController — main Niagara VFX controller actor
// ============================================================
UCLASS(BlueprintType, Blueprintable, ClassGroup = "VFX")
class TRANSPERSONALGAME_API AVFX_NiagaraController : public AActor
{
    GENERATED_BODY()

public:
    AVFX_NiagaraController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // --------------------------------------------------------
    // LOD THRESHOLDS
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LODHighDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LODMediumDistance = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LODLowDistance = 8000.0f;

    // --------------------------------------------------------
    // BUDGET
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Budget")
    int32 MaxActiveEffects = 64;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|Budget")
    int32 CurrentActiveEffects = 0;

    // --------------------------------------------------------
    // NIAGARA SYSTEM REFERENCES (assign in Blueprint/Editor)
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|Environment")
    UNiagaraSystem* NS_CampfireFire = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|Environment")
    UNiagaraSystem* NS_CampfireSmoke = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|Environment")
    UNiagaraSystem* NS_CampfireEmbers = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|Environment")
    UNiagaraSystem* NS_RainFall = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|Environment")
    UNiagaraSystem* NS_VolcanicAsh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|Environment")
    UNiagaraSystem* NS_WindParticles = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|Dinosaurs")
    UNiagaraSystem* NS_DinoFootstepDust = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|Dinosaurs")
    UNiagaraSystem* NS_DinoBreathVapor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|Dinosaurs")
    UNiagaraSystem* NS_DinoBloodImpact = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|Dinosaurs")
    UNiagaraSystem* NS_DinoRoarAirwave = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|Combat")
    UNiagaraSystem* NS_WeaponImpactDirt = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|Combat")
    UNiagaraSystem* NS_WeaponImpactBlood = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|Combat")
    UNiagaraSystem* NS_CraftingSparkFlint = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|World")
    UNiagaraSystem* NS_WaterfallSpray = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|World")
    UNiagaraSystem* NS_InsectSwarm = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems|World")
    UNiagaraSystem* NS_PollenDrift = nullptr;

    // --------------------------------------------------------
    // WEATHER STATE
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    bool bRainActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    bool bVolcanicAshActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float WeatherIntensity = 1.0f;

    // --------------------------------------------------------
    // PUBLIC API — UFUNCTION
    // --------------------------------------------------------

    /** Spawn a one-shot or looping VFX effect at world location */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 SpawnEffect(const FVFX_NiagaraSpawnRequest& Request);

    /** Stop a specific active effect by slot index */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(int32 SlotIndex);

    /** Stop all active effects */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    /** Convenience: spawn campfire at location (fire + smoke + embers) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Convenience")
    void SpawnCampfireEffect(FVector Location);

    /** Convenience: spawn dinosaur footstep dust at impact location */
    UFUNCTION(BlueprintCallable, Category = "VFX|Convenience")
    void SpawnDinoFootstepDust(FVector ImpactLocation, float DinoMassScale = 1.0f);

    /** Convenience: spawn blood impact at hit location */
    UFUNCTION(BlueprintCallable, Category = "VFX|Convenience")
    void SpawnBloodImpact(FVector HitLocation, FVector HitNormal);

    /** Convenience: spawn weapon impact (dirt or blood based on surface) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Convenience")
    void SpawnWeaponImpact(FVector HitLocation, FVector HitNormal, bool bHitFlesh);

    /** Convenience: spawn dino roar airwave (screen distortion ring) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Convenience")
    void SpawnDinoRoarAirwave(FVector DinoMouthLocation, float RoarIntensity = 1.0f);

    /** Enable/disable weather VFX */
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherVFX(bool bRain, bool bAsh, float Intensity);

    /** Get LOD level for a given world distance */
    UFUNCTION(BlueprintCallable, Category = "VFX|LOD")
    EVFX_NiagaraLODLevel GetLODForDistance(float Distance) const;

    /** Get current active effect count */
    UFUNCTION(BlueprintCallable, Category = "VFX|Budget")
    int32 GetActiveEffectCount() const { return CurrentActiveEffects; }

private:
    // Active effect slots
    TArray<FVFX_NiagaraEffectSlot> ActiveEffects;

    // Weather Niagara components (persistent looping)
    UNiagaraComponent* ActiveRainComponent = nullptr;
    UNiagaraComponent* ActiveAshComponent = nullptr;

    // Cached player location for LOD
    FVector CachedPlayerLocation = FVector::ZeroVector;

    // LOD update timer
    float LODUpdateTimer = 0.0f;
    static constexpr float LODUpdateInterval = 0.25f;

    // Internal helpers
    UNiagaraSystem* GetSystemForType(EVFX_NiagaraEffectType EffectType) const;
    void UpdateLODForAllEffects();
    void CleanupExpiredEffects(float CurrentTime);
    int32 FindFreeSlot() const;
    void ApplyLODToComponent(UNiagaraComponent* Comp, EVFX_NiagaraLODLevel LOD, EVFX_NiagaraEffectType EffectType);
};
