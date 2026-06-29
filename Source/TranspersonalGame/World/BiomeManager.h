#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

/**
 * EEng_BiomeType — Prehistoric biome classifications
 * Prefixed with Eng_ to avoid collision with other agents' types.
 */
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Jungle         UMETA(DisplayName = "Jungle"),
    Savanna        UMETA(DisplayName = "Savanna"),
    Swamp          UMETA(DisplayName = "Swamp"),
    Volcanic       UMETA(DisplayName = "Volcanic"),
    Coastal        UMETA(DisplayName = "Coastal"),
    Forest         UMETA(DisplayName = "Forest"),
    Plains         UMETA(DisplayName = "Plains"),
    Desert         UMETA(DisplayName = "Desert"),
    Unknown        UMETA(DisplayName = "Unknown")
};

/**
 * FEng_BiomeData — Runtime data for a single biome region.
 * Stored per-cell in the biome grid.
 */
USTRUCT(BlueprintType)
struct FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Unknown;

    /** Average temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    /** Rainfall in mm/year (affects vegetation density) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Rainfall = 1200.0f;

    /** Elevation in metres above sea level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    /** Danger level 0-1 (affects dinosaur spawn density) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DangerLevel = 0.5f;

    /** World-space origin of this biome cell */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WorldOrigin = FVector::ZeroVector;

    /** Radius of this biome cell in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 50000.0f;
};

/**
 * ABiomeManager — World actor that owns and queries the biome grid.
 *
 * Responsibilities:
 *   - Maintains a flat array of FEng_BiomeData cells covering the playable world
 *   - Provides GetBiomeAtLocation() for any world-space query
 *   - Exposes biome data to Blueprint for environment/AI systems
 *   - Integrates with PCGWorldGenerator (forward-declared; no hard dependency)
 *
 * Architecture rules:
 *   - Single instance per world (enforced via BeginPlay check)
 *   - All cross-system queries go through this actor, never direct array access
 *   - DinosaurBase, FoliageManager, and WeatherSystem query this actor for biome context
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|World")
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ─── Biome Query API ─────────────────────────────────────────────────────

    /**
     * Returns the biome data for the cell closest to WorldLocation.
     * Returns a default Unknown biome if no cells are registered.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    FEng_BiomeData GetBiomeAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns the biome type enum for a world location (convenience wrapper).
     */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    EEng_BiomeType GetBiomeTypeAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns the danger level (0-1) at a world location.
     * Used by DinosaurBase to scale spawn frequency.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    // ─── Biome Registration ───────────────────────────────────────────────────

    /**
     * Registers a biome cell. Called by PCGWorldGenerator during world init.
     * Safe to call at runtime — adds to the BiomeCells array.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome|Setup")
    void RegisterBiomeCell(const FEng_BiomeData& BiomeData);

    /**
     * Clears all registered biome cells. Use when regenerating the world.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome|Setup")
    void ClearAllBiomeCells();

    /**
     * Returns the total number of registered biome cells.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    int32 GetBiomeCellCount() const;

    // ─── Default Biome Setup (Editor / Prototype) ─────────────────────────────

    /**
     * Populates a default set of biome cells for the MinPlayableMap.
     * Called in BeginPlay if BiomeCells is empty (prototype fallback).
     */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Setup")
    void InitialiseDefaultBiomes();

    // ─── Properties ──────────────────────────────────────────────────────────

    /** All registered biome cells. Populated by PCGWorldGenerator or InitialiseDefaultBiomes. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Data")
    TArray<FEng_BiomeData> BiomeCells;

    /** World extents in cm used for biome grid queries (default: 200km × 200km). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float WorldExtentCm = 200000.0f;

    /** If true, draws debug spheres for each biome cell origin in-editor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDrawDebugBiomes = false;

private:
    /** Finds the index of the nearest biome cell to WorldLocation. Returns -1 if empty. */
    int32 FindNearestBiomeCellIndex(const FVector& WorldLocation) const;
};
