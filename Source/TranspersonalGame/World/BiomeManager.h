#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// ─── Biome weather state ─────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EEng_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Drought     UMETA(DisplayName = "Drought"),
};

// ─── Per-biome runtime data ───────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FEng_BiomeRuntimeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Grassland;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_WeatherState CurrentWeather = EEng_WeatherState::Clear;

    /** 0-1 moisture level — affects vegetation density and water sources */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float MoistureLevel = 0.5f;

    /** Celsius — affects stamina drain and animal behaviour */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float AmbientTemperature = 28.0f;

    /** Dominant dinosaur species allowed to spawn in this biome */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    TArray<EDinosaurSpecies> AllowedSpecies;

    /** World-space bounds of this biome region */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FBox BiomeBounds = FBox(ForceInit);

    /** Foliage density multiplier (0=barren, 1=dense) */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float VegetationDensity = 0.7f;
};

// ─── Biome transition event ───────────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FEng_OnBiomeChanged,
    EBiomeType, OldBiome,
    EBiomeType, NewBiome);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FEng_OnWeatherChanged,
    EEng_WeatherState, OldWeather,
    EEng_WeatherState, NewWeather);

// ─── BiomeManager ─────────────────────────────────────────────────────────────
/**
 * ABiomeManager
 *
 * Singleton actor that owns the biome map for the current level.
 * Tracks which biome the player occupies, drives weather transitions,
 * and exposes data to the foliage, AI, and audio systems.
 *
 * Architecture rules (Engine Architect #02):
 *  - One instance per level, placed in MinPlayableMap.
 *  - All cross-system queries go through GetBiomeAtLocation().
 *  - Weather is driven by a Markov chain — no random jumps.
 *  - Biome bounds are registered at BeginPlay by PCGWorldGenerator.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ── Lifecycle ─────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Biome query API (used by AI, Audio, VFX, Foliage) ────────────────────

    /** Returns the biome type at the given world location. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns full runtime data for the biome at the given location. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeRuntimeData GetBiomeDataAtLocation(const FVector& WorldLocation) const;

    /** Returns the biome the player is currently in. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetPlayerCurrentBiome() const;

    /** Returns current weather in the player's biome. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_WeatherState GetCurrentWeather() const;

    /** Returns ambient temperature at location (°C). */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    // ── Biome registration (called by PCGWorldGenerator at BeginPlay) ─────────

    /** Register a biome region. PCGWorldGenerator calls this for each generated chunk. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeRegion(EBiomeType BiomeType, const FBox& Bounds, float VegetationDensity, float MoistureLevel);

    /** Clear all registered biome regions (called before PCG re-generation). */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void ClearAllBiomeRegions();

    // ── Weather control ───────────────────────────────────────────────────────

    /** Force a specific weather state (for scripted events / cutscenes). */
    UFUNCTION(BlueprintCallable, Category = "Biome|Weather")
    void ForceWeatherState(EEng_WeatherState NewWeather);

    /** How many seconds between weather evaluation ticks. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Weather")
    float WeatherTickInterval = 300.0f;

    // ── Events ────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Biome|Events")
    FEng_OnBiomeChanged OnBiomeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Biome|Events")
    FEng_OnWeatherChanged OnWeatherChanged;

    // ── Debug ─────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDrawBiomeBounds = false;

    UFUNCTION(CallInEditor, Category = "Biome|Debug")
    void PrintBiomeReport();

private:
    // Registered biome regions
    UPROPERTY()
    TArray<FEng_BiomeRuntimeData> BiomeRegions;

    // Current player biome
    UPROPERTY()
    EBiomeType CurrentPlayerBiome = EBiomeType::Grassland;

    // Current weather
    UPROPERTY()
    EEng_WeatherState ActiveWeather = EEng_WeatherState::Clear;

    // Accumulated time since last weather tick
    float WeatherTickAccumulator = 0.0f;

    // Internal helpers
    void TickWeatherTransition();
    void UpdatePlayerBiome();
    EEng_WeatherState AdvanceWeatherMarkov(EEng_WeatherState Current, EBiomeType Biome) const;
    float GetBaseTemperatureForBiome(EBiomeType Biome) const;
};
