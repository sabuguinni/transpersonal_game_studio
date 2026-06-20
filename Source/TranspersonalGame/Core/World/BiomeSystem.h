// BiomeSystem.h
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260620_002
// Biome classification, terrain feature data, and PCG spawn rules for MinPlayableMap.
// Prefix: World_ (collision-safe per RULE 2)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS (global scope — RULE 1)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Cretaceous Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    RockyRidge  UMETA(DisplayName = "Rocky Ridge"),
    River       UMETA(DisplayName = "River Valley"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic Field"),
    COUNT       UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EWorld_TerrainFeature : uint8
{
    RockCluster     UMETA(DisplayName = "Rock Cluster"),
    WaterBody       UMETA(DisplayName = "Water Body"),
    CliffFace       UMETA(DisplayName = "Cliff Face"),
    DenseVegetation UMETA(DisplayName = "Dense Vegetation"),
    OpenGround      UMETA(DisplayName = "Open Ground"),
    MudFlat         UMETA(DisplayName = "Mud Flat"),
    LavaBed         UMETA(DisplayName = "Lava Bed")
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Ash         UMETA(DisplayName = "Volcanic Ash")
};

// ─────────────────────────────────────────────────────────────────────────────
// STRUCTS (global scope — RULE 1)
// ─────────────────────────────────────────────────────────────────────────────

/** Per-biome spawn density and terrain rules */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeSpawnRules
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    /** Max static mesh actors allowed in this biome (budget gate) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    int32 MaxStaticMeshActors = 80;

    /** Max foliage instances (grass/ferns/cycads) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    int32 MaxFoliageInstances = 2000;

    /** Tree density per 10000 cm² (0.0–1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TreeDensity = 0.6f;

    /** Rock density per 10000 cm² (0.0–1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RockDensity = 0.2f;

    /** Dominant terrain feature */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_TerrainFeature DominantFeature = EWorld_TerrainFeature::DenseVegetation;

    /** Ambient audio cue asset path (for Audio Agent #16) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FString AmbientAudioPath;

    /** Fog density multiplier for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float FogDensityMultiplier = 1.0f;

    /** Dinosaur species that prefer this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    TArray<FName> PreferredDinoSpecies;
};

/** Runtime snapshot of a biome zone in the world */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZoneSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    float RadiusCm = 3000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    int32 CurrentActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    bool bIsPlayerInside = false;

    UPROPERTY(BlueprintReadOnly, Category = "World|Biome")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;
};

/** PCG terrain feature placement record */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeatureRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "World|PCG")
    FString ActorLabel;

    UPROPERTY(BlueprintReadOnly, Category = "World|PCG")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "World|PCG")
    EWorld_TerrainFeature FeatureType = EWorld_TerrainFeature::RockCluster;

    UPROPERTY(BlueprintReadOnly, Category = "World|PCG")
    EWorld_BiomeType OwningBiome = EWorld_BiomeType::Forest;

    UPROPERTY(BlueprintReadOnly, Category = "World|PCG")
    bool bIsPlaced = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// UCLASS
// ─────────────────────────────────────────────────────────────────────────────

/**
 * UWorld_BiomeSystem
 * Manages biome zone definitions, spawn rules, and runtime queries.
 * Instantiated as a UObject owned by the GameInstance or PCGWorldGenerator.
 * Agent #06 (Environment Artist) reads spawn rules from this system.
 * Agent #16 (Audio) reads AmbientAudioPath from BiomeSpawnRules.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "World Biome System"))
class TRANSPERSONALGAME_API UWorld_BiomeSystem : public UObject
{
    GENERATED_BODY()

public:
    UWorld_BiomeSystem();

    // ── Biome Registry ────────────────────────────────────────────────────────

    /** Register default biome rules for all 6 biome types */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    void InitializeDefaultBiomes();

    /** Get spawn rules for a given biome type */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    FWorld_BiomeSpawnRules GetBiomeRules(EWorld_BiomeType BiomeType) const;

    /** Determine which biome a world-space location belongs to */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    /** Get all registered biome zone snapshots */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    TArray<FWorld_BiomeZoneSnapshot> GetAllBiomeZones() const;

    // ── PCG Terrain Features ──────────────────────────────────────────────────

    /** Register a placed terrain feature (called by PCG spawn scripts) */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    void RegisterTerrainFeature(const FWorld_TerrainFeatureRecord& Record);

    /** Get all terrain features for a specific biome */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    TArray<FWorld_TerrainFeatureRecord> GetFeaturesForBiome(EWorld_BiomeType BiomeType) const;

    /** Total placed feature count (for budget validation) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World|PCG")
    int32 GetTotalFeatureCount() const;

    // ── Weather ───────────────────────────────────────────────────────────────

    /** Set weather state for a biome zone */
    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    void SetBiomeWeather(EWorld_BiomeType BiomeType, EWorld_WeatherState NewWeather);

    /** Get current weather in the biome containing the given location */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World|Weather")
    EWorld_WeatherState GetWeatherAtLocation(FVector WorldLocation) const;

    // ── Budget Gate (integrates with PerformanceBudget from Agent #04) ────────

    /** Returns true if spawning N more actors in this biome is within budget */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World|Budget")
    bool IsBiomeWithinBudget(EWorld_BiomeType BiomeType, int32 AdditionalActors = 1) const;

private:
    /** Biome rules registry — indexed by EWorld_BiomeType */
    UPROPERTY()
    TArray<FWorld_BiomeSpawnRules> BiomeRules;

    /** All placed terrain feature records */
    UPROPERTY()
    TArray<FWorld_TerrainFeatureRecord> PlacedFeatures;

    /** Runtime biome zone snapshots */
    UPROPERTY()
    TArray<FWorld_BiomeZoneSnapshot> BiomeZones;

    /** Per-biome weather state */
    UPROPERTY()
    TArray<EWorld_WeatherState> BiomeWeatherStates;

    /** Initialize zone center/radius for each biome (matches MinPlayableMap layout) */
    void InitializeBiomeZones();
};
