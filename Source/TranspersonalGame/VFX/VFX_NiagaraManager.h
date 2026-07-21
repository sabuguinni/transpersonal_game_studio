#pragma once

// VFX_NiagaraManager.h
// Agent #17 — VFX Agent | PROD_CYCLE_AUTO_20260701_006
// Manages all Niagara particle systems for the prehistoric survival game.
// Categories: Environment (fire/rain/ash), Dinosaur (footstep/breath/blood), Combat (impact/wound)

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "VFX_NiagaraManager.generated.h"

// ── Enums (global scope — UHT requirement) ──────────────────────────────────

UENUM(BlueprintType)
enum class EVFX_EffectCategory : uint8
{
    Environment   UMETA(DisplayName = "Environment"),
    Dinosaur      UMETA(DisplayName = "Dinosaur"),
    Combat        UMETA(DisplayName = "Combat"),
    Weather       UMETA(DisplayName = "Weather"),
    World         UMETA(DisplayName = "World")
};

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    // Environment
    CampfireFire        UMETA(DisplayName = "Campfire Fire"),
    CampfireSmoke       UMETA(DisplayName = "Campfire Smoke"),
    CampfireEmbers      UMETA(DisplayName = "Campfire Embers"),
    // Dinosaur
    FootstepDust        UMETA(DisplayName = "Footstep Dust"),
    BreathVapour        UMETA(DisplayName = "Breath Vapour"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter"),
    RoarAirDistortion   UMETA(DisplayName = "Roar Air Distortion"),
    // Combat
    WeaponImpact        UMETA(DisplayName = "Weapon Impact"),
    WoundBleed          UMETA(DisplayName = "Wound Bleed"),
    SpearThrow          UMETA(DisplayName = "Spear Throw"),
    // Weather
    Rain                UMETA(DisplayName = "Rain"),
    Snow                UMETA(DisplayName = "Snow"),
    Fog                 UMETA(DisplayName = "Fog"),
    // World
    VolcanicAsh         UMETA(DisplayName = "Volcanic Ash"),
    WaterfallMist       UMETA(DisplayName = "Waterfall Mist"),
    PollenDrift         UMETA(DisplayName = "Pollen Drift"),
    AshFall             UMETA(DisplayName = "Ash Fall")
};

// ── Structs (global scope) ───────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FVFX_EffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::CampfireFire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD0_Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD1_Distance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD2_Distance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float CullDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoActivate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float SpawnRate = 1.0f;

    FVFX_EffectConfig() {}
};

USTRUCT(BlueprintType)
struct FVFX_ActiveEffect
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    int32 EffectID = -1;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::CampfireFire;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    float SpawnTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    bool bIsLooping = false;

    FVFX_ActiveEffect() {}
};

// ── Main Class ───────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (VFX), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AVFX_NiagaraManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_NiagaraManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Spawn VFX ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "VFX|Spawn")
    int32 SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, bool bLooping = false);

    UFUNCTION(BlueprintCallable, Category = "VFX|Spawn")
    int32 SpawnEffectAttached(EVFX_EffectType EffectType, USceneComponent* AttachTo, FName SocketName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX|Control")
    void StopEffect(int32 EffectID);

    UFUNCTION(BlueprintCallable, Category = "VFX|Control")
    void StopAllEffectsOfType(EVFX_EffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX|Control")
    void StopAllEffects();

    // ── Preset Spawners ───────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    int32 SpawnCampfire(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    int32 SpawnDinosaurFootstep(FVector Location, float DinosaurMassKg = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    int32 SpawnBloodImpact(FVector Location, FVector ImpactNormal, float DamageAmount = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    int32 SpawnWeaponImpact(FVector Location, FVector ImpactNormal, EVFX_EffectType WeaponType = EVFX_EffectType::WeaponImpact);

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    void SetWeatherEffect(EVFX_EffectType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    void ClearWeatherEffects();

    // ── LOD Control ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "VFX|LOD")
    void SetGlobalVFXQuality(float QualityScale);

    UFUNCTION(BlueprintCallable, Category = "VFX|LOD")
    void UpdateLODForCamera(FVector CameraLocation);

    // ── Configuration ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    TArray<FVFX_EffectConfig> EffectConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float GlobalVFXQuality = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    int32 MaxActiveEffects = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    bool bEnableLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float LODUpdateInterval = 0.5f;

    // ── State ─────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "VFX|State")
    TArray<FVFX_ActiveEffect> ActiveEffects;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|State")
    int32 TotalEffectsSpawnedThisSession = 0;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|State")
    int32 CurrentWeatherEffectID = -1;

private:
    int32 NextEffectID = 0;
    float LODUpdateTimer = 0.0f;

    FVFX_EffectConfig* FindConfigForType(EVFX_EffectType EffectType);
    void InitializeDefaultConfigs();
    void CullDistantEffects(FVector CameraLocation);
};
