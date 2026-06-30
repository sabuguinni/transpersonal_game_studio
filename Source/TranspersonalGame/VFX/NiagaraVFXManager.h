#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraVFXManager.generated.h"

// ============================================================
// VFX LOD Level — 3-tier chain for performance
// ============================================================
UENUM(BlueprintType)
enum class EVFX_LODTier : uint8
{
    High    UMETA(DisplayName = "High (< 20m)"),
    Medium  UMETA(DisplayName = "Medium (20-60m)"),
    Low     UMETA(DisplayName = "Low (60-120m)"),
    Culled  UMETA(DisplayName = "Culled (> 120m)")
};

// ============================================================
// VFX Category — maps to Niagara system asset slot
// ============================================================
UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    CampfireFlame       UMETA(DisplayName = "Campfire Flame"),
    CampfireSmoke       UMETA(DisplayName = "Campfire Smoke"),
    CampfireEmbers      UMETA(DisplayName = "Campfire Embers"),
    PlayerFootstepDust  UMETA(DisplayName = "Player Footstep Dust"),
    DinoFootstepDust    UMETA(DisplayName = "Dino Footstep Dust (Small)"),
    TRexFootstepDust    UMETA(DisplayName = "T-Rex Footstep Dust (Large)"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter"),
    BloodDrip           UMETA(DisplayName = "Blood Drip"),
    RainSplash          UMETA(DisplayName = "Rain Splash"),
    DustAmbient         UMETA(DisplayName = "Ambient Dust Motes"),
    VolcanicAsh         UMETA(DisplayName = "Volcanic Ash Fall"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    BreathVapour        UMETA(DisplayName = "Breath Vapour (Cold)"),
    SparkCraft          UMETA(DisplayName = "Crafting Sparks"),
    SmokeCook           UMETA(DisplayName = "Cooking Smoke")
};

// ============================================================
// Spawn parameters struct
// ============================================================
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_SpawnRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::PlayerFootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    /** If set, the effect attaches to this component */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TObjectPtr<USceneComponent> AttachTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping = false;

    /** Override LOD — Auto means distance-based */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_LODTier LODOverride = EVFX_LODTier::High;
};

// ============================================================
// LOD distance thresholds (cm)
// ============================================================
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_LODDistances
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float HighToMedium = 2000.0f;   // 20m

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float MediumToLow = 6000.0f;    // 60m

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LowToCull = 12000.0f;     // 120m
};

// ============================================================
// NiagaraVFXManager — ActorComponent managing all particle FX
// ============================================================
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UNiagaraVFXManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNiagaraVFXManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Public Spawn API ----

    /** Spawn a one-shot or looping VFX at world location */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(const FVFX_SpawnRequest& Request);

    /** Convenience: spawn campfire at location (flame + smoke + embers) */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfire(FVector Location);

    /** Convenience: spawn player footstep dust */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnPlayerFootstep(FVector Location, float SpeedScale = 1.0f);

    /** Convenience: spawn T-Rex footstep dust cloud */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnTRexFootstep(FVector Location, float MassScale = 1.0f);

    /** Convenience: spawn blood splatter at impact point */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatter(FVector Location, FVector ImpactNormal, float DamageAmount);

    /** Convenience: spawn breath vapour (cold biomes) */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBreathVapour(USceneComponent* MouthSocket);

    /** Stop and destroy a looping effect */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* NiagaraComp);

    // ---- Niagara Asset References ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TObjectPtr<UNiagaraSystem> NS_CampfireFlame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TObjectPtr<UNiagaraSystem> NS_CampfireSmoke;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TObjectPtr<UNiagaraSystem> NS_CampfireEmbers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TObjectPtr<UNiagaraSystem> NS_PlayerFootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TObjectPtr<UNiagaraSystem> NS_DinoFootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TObjectPtr<UNiagaraSystem> NS_TRexFootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TObjectPtr<UNiagaraSystem> NS_BloodSplatter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TObjectPtr<UNiagaraSystem> NS_RainSplash;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TObjectPtr<UNiagaraSystem> NS_VolcanicAsh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TObjectPtr<UNiagaraSystem> NS_BreathVapour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TObjectPtr<UNiagaraSystem> NS_SparkCraft;

    // ---- LOD Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    FVFX_LODDistances LODDistances;

    /** Max simultaneous active Niagara components before oldest is culled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    int32 MaxActiveEffects = 32;

    /** Current active effect count (read-only) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Performance")
    int32 ActiveEffectCount = 0;

private:
    /** Compute LOD tier based on distance to player camera */
    EVFX_LODTier ComputeLODTier(const FVector& EffectLocation) const;

    /** Get the correct Niagara system for effect type + LOD tier */
    UNiagaraSystem* ResolveNiagaraSystem(EVFX_EffectType EffectType, EVFX_LODTier LOD) const;

    /** Pool of active looping components for management */
    UPROPERTY()
    TArray<TObjectPtr<UNiagaraComponent>> ActiveLoopingEffects;

    /** Evict oldest effect when pool is full */
    void EvictOldestEffect();
};
