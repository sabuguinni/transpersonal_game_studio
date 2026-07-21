#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DinosaurSpawnManager.generated.h"

class ADinosaurBase;
class ATranspersonalCharacter;

/**
 * FPerf_DinoSpawnEntry — tracks a single dinosaur spawn slot.
 * Manages distance-based activation: dinos only tick when within ActivationRadius of player.
 */
USTRUCT(BlueprintType)
struct FPerf_DinoSpawnEntry
{
    GENERATED_BODY()

    /** World location where this dino spawns */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector SpawnLocation = FVector::ZeroVector;

    /** Class of dinosaur to spawn at this slot */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    TSubclassOf<ADinosaurBase> DinoClass;

    /** Currently spawned actor (null if not active) */
    UPROPERTY(BlueprintReadOnly, Category = "Spawn")
    ADinosaurBase* SpawnedActor = nullptr;

    /** Distance at which this dino activates (starts ticking) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float ActivationRadius = 3000.f;

    /** Distance at which this dino despawns to save memory */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float DespawnRadius = 5000.f;

    /** Whether this slot is currently active */
    UPROPERTY(BlueprintReadOnly, Category = "Spawn")
    bool bIsActive = false;
};

/**
 * APerf_DinosaurSpawnManager — distance-based dinosaur activation system.
 *
 * Performance contract:
 * - Only dinosaurs within ActivationRadius (default 3000 UU) of the player tick each frame.
 * - Dinosaurs beyond DespawnRadius (default 5000 UU) are despawned and replaced with a
 *   lightweight spawn entry that re-activates when the player approaches again.
 * - This manager itself ticks at 5Hz (0.2s interval) — not every frame.
 * - Supports up to 50 registered spawn slots with zero performance cost when all are distant.
 *
 * Target: 60fps PC / 30fps console with up to 20 simultaneous active dinosaurs.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dinosaur Spawn Manager"))
class TRANSPERSONALGAME_API APerf_DinosaurSpawnManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_DinosaurSpawnManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Register a new spawn slot at runtime */
    UFUNCTION(BlueprintCallable, Category = "Performance|Spawning")
    void RegisterSpawnSlot(FVector Location, TSubclassOf<ADinosaurBase> DinoClass, float ActivationRadius = 3000.f);

    /** Force activate all slots within radius of a point (e.g., for cutscenes) */
    UFUNCTION(BlueprintCallable, Category = "Performance|Spawning")
    void ForceActivateRadius(FVector Center, float Radius);

    /** Despawn all active dinosaurs immediately (e.g., on player death) */
    UFUNCTION(BlueprintCallable, Category = "Performance|Spawning")
    void DespawnAll();

    /** Returns count of currently active (ticking) dinosaurs */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance|Spawning")
    int32 GetActiveDinoCount() const;

    /** Returns count of registered spawn slots */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance|Spawning")
    int32 GetTotalSlotCount() const;

    // --- Configuration ---

    /** All registered spawn slots */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    TArray<FPerf_DinoSpawnEntry> SpawnSlots;

    /** Global activation radius override (0 = use per-slot radius) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    float GlobalActivationRadius = 0.f;

    /** Maximum simultaneous active dinosaurs — hard cap for performance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    int32 MaxActiveDinos = 20;

    /** How often (in seconds) the manager checks player distance — lower = more responsive but more CPU */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    float CheckInterval = 0.2f;

protected:
    /** Cached reference to the player character */
    UPROPERTY()
    ATranspersonalCharacter* CachedPlayer = nullptr;

    /** Time accumulator for check interval */
    float TimeSinceLastCheck = 0.f;

    /** Find and cache the player reference */
    void CachePlayerReference();

    /** Activate a spawn slot — spawns the dinosaur actor */
    void ActivateSlot(FPerf_DinoSpawnEntry& Slot);

    /** Deactivate a spawn slot — despawns the dinosaur actor */
    void DeactivateSlot(FPerf_DinoSpawnEntry& Slot);

    /** Main distance-check loop — called at CheckInterval Hz */
    void UpdateActivation();
};
