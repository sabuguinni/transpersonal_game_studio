// PCGBiomeZoneSystem.h
// Agent #05 — Procedural World Generator
// Biome zone classification and PCG rule dispatch for MinPlayableMap

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "PCGBiomeZoneSystem.generated.h"

// ── Biome Zone Types ──────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeZoneType : uint8
{
    TropicalJungle   UMETA(DisplayName = "Tropical Jungle"),
    VolcanicPlains   UMETA(DisplayName = "Volcanic Plains"),
    RiverDelta       UMETA(DisplayName = "River Delta"),
    OpenSavanna      UMETA(DisplayName = "Open Savanna"),
    RockyHighlands   UMETA(DisplayName = "Rocky Highlands"),
    COUNT            UMETA(Hidden)
};

// ── Per-zone PCG configuration ────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeZoneType ZoneType = EWorld_BiomeZoneType::OpenSavanna;

    /** World-space centre of this biome zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector ZoneCenter = FVector::ZeroVector;

    /** Radius in cm within which this biome rules apply */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ZoneRadius = 5000.f;

    /** Foliage density multiplier (0=barren, 1=normal, 2=dense) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 1.f;

    /** Dominant vegetation asset path (skeletal or static mesh) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString PrimaryVegetationPath = TEXT("");

    /** Secondary vegetation asset path */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString SecondaryVegetationPath = TEXT("");

    /** Rock/debris density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.5f;

    /** Water body present in this zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWater = false;

    /** Ambient temperature in Celsius (affects survival stats) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float AmbientTemperatureCelsius = 28.f;

    /** Humidity 0-1 (affects thirst drain rate) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float Humidity = 0.6f;

    /** Dinosaur species tags that prefer this zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Fauna")
    TArray<FName> PreferredDinoSpecies;
};

// ── Biome Zone Actor ──────────────────────────────────────────────────────────
UCLASS(ClassGroup = "TranspersonalGame|World", meta = (DisplayName = "PCG Biome Zone System"))
class TRANSPERSONALGAME_API APCGBiomeZoneSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeZoneSystem();

    // ── Zone registry ─────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    TArray<FWorld_BiomeZoneConfig> BiomeZones;

    // ── Runtime API ──────────────────────────────────────────────────────────

    /**
     * Returns the biome zone type at the given world location.
     * If multiple zones overlap, the one with the smallest radius wins.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Zones")
    EWorld_BiomeZoneType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns the full config for the biome zone at the given location.
     * Returns nullptr if no zone covers that location.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Zones")
    bool GetBiomeConfigAtLocation(const FVector& WorldLocation, FWorld_BiomeZoneConfig& OutConfig) const;

    /**
     * Returns foliage density multiplier at a world location.
     * Used by FoliageManager to scale instance counts.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Zones")
    float GetFoliageDensityAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns ambient temperature at a world location.
     * Used by survival system to calculate heat/cold damage.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Zones")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns humidity at a world location.
     * Used by survival system to scale thirst drain.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Zones")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    /**
     * Registers default biome zones for MinPlayableMap.
     * Call this once at BeginPlay or from a Blueprint setup node.
     */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome Zones")
    void InitializeDefaultBiomeZones();

    // ── Editor helpers ────────────────────────────────────────────────────────

    /** Draw debug spheres for all registered zones in the editor viewport */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome Zones|Debug")
    void DrawDebugZones();

protected:
    virtual void BeginPlay() override;

private:
    /** Sorted zone cache (smallest radius first for priority lookup) */
    TArray<FWorld_BiomeZoneConfig> SortedZoneCache;

    void RebuildSortedCache();
};
