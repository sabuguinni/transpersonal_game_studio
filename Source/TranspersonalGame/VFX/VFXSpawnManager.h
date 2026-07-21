#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/EngineTypes.h"
#include "VFXSpawnManager.generated.h"

// ============================================================
// VFX Spawn Manager — Agent #17 (Cycle AUTO_20260630_011)
// Manages runtime spawning of Niagara VFX systems in the world.
// Handles: campfire, footstep dust, blood splatter, rain splash,
//          volcanic ash, breath vapor, crafting sparks.
// ============================================================

UENUM(BlueprintType)
enum class EVFX_SpawnCategory : uint8
{
    Environment     UMETA(DisplayName = "Environment"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Player          UMETA(DisplayName = "Player"),
    Combat          UMETA(DisplayName = "Combat"),
    Weather         UMETA(DisplayName = "Weather"),
    Volcanic        UMETA(DisplayName = "Volcanic")
};

UENUM(BlueprintType)
enum class EVFX_LODLevel : uint8
{
    High    UMETA(DisplayName = "High (< 800 units)"),
    Medium  UMETA(DisplayName = "Medium (800-2500 units)"),
    Low     UMETA(DisplayName = "Low (2500-5000 units)"),
    Culled  UMETA(DisplayName = "Culled (> 5000 units)")
};

USTRUCT(BlueprintType)
struct FVFX_SpawnRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_SpawnCategory Category = EVFX_SpawnCategory::Environment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LifetimeSeconds = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    AActor* AttachTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName AttachSocketName = NAME_None;
};

USTRUCT(BlueprintType)
struct FVFX_ActiveEffect
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    int32 EffectID = -1;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    EVFX_SpawnCategory Category = EVFX_SpawnCategory::Environment;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    float SpawnTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    float LifetimeSeconds = 3.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    EVFX_LODLevel CurrentLOD = EVFX_LODLevel::High;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    AActor* EffectActor = nullptr;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UVFX_SpawnManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // --- Core Spawn API ---

    UFUNCTION(BlueprintCallable, Category = "VFX|Spawn")
    int32 SpawnEffect(const FVFX_SpawnRequest& Request);

    UFUNCTION(BlueprintCallable, Category = "VFX|Spawn")
    void StopEffect(int32 EffectID, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "VFX|Spawn")
    void StopAllEffects(EVFX_SpawnCategory Category);

    UFUNCTION(BlueprintCallable, Category = "VFX|Spawn")
    void StopAllEffectsImmediate();

    // --- LOD Management ---

    UFUNCTION(BlueprintCallable, Category = "VFX|LOD")
    EVFX_LODLevel GetLODForDistance(float DistanceFromCamera) const;

    UFUNCTION(BlueprintCallable, Category = "VFX|LOD")
    void UpdateAllEffectLODs();

    // --- Convenience Spawners ---

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    int32 SpawnCampfireEffect(FVector Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    int32 SpawnFootstepDust(FVector ImpactLocation, float DinoMassKg = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    int32 SpawnBloodSplatter(FVector ImpactLocation, FVector ImpactNormal, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    int32 SpawnRainSplash(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    int32 SpawnVolcanicAsh(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    int32 SpawnBreathVapor(AActor* DinoActor, FName MouthSocket);

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    int32 SpawnCraftingSparks(FVector Location);

    // --- State Queries ---

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VFX|Query")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VFX|Query")
    int32 GetActiveEffectCountByCategory(EVFX_SpawnCategory Category) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VFX|Query")
    bool IsEffectActive(int32 EffectID) const;

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    int32 MaxConcurrentEffects = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float LODDistanceHigh = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float LODDistanceMedium = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float LODDistanceLow = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    bool bEnableLODScaling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    bool bEnableEffectPooling = true;

private:
    TMap<int32, FVFX_ActiveEffect> ActiveEffects;
    int32 NextEffectID = 1;

    int32 GenerateEffectID();
    void CleanupExpiredEffects();
    float GetDistanceToCamera(const FVector& Location) const;
    void ApplyLODToEffect(FVFX_ActiveEffect& Effect, EVFX_LODLevel NewLOD);
};
