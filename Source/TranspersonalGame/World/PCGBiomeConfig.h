#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PCGBiomeConfig.generated.h"

/**
 * Biome types for the Cretaceous prehistoric world.
 * Agent #05 — Procedural World Generator
 */
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    CretaceousForest    UMETA(DisplayName = "Cretaceous Forest"),
    OpenPlains          UMETA(DisplayName = "Open Plains"),
    RiverValley         UMETA(DisplayName = "River Valley"),
    VolcanicHighlands   UMETA(DisplayName = "Volcanic Highlands"),
    CoastalWetlands     UMETA(DisplayName = "Coastal Wetlands"),
    RockyEscarpment     UMETA(DisplayName = "Rocky Escarpment"),
};

/**
 * Terrain height configuration per biome.
 */
USTRUCT(BlueprintType)
struct FWorld_BiomeHeightConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float MinHeightCM = -1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float MaxHeightCM = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float HillFrequency = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float HillAmplitude = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float RiverDepthCM = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float RiverWidthCM = 1500.0f;
};

/**
 * Foliage density configuration per biome.
 * Respects DefaultScalability.ini foliage.DensityScale tiers.
 */
USTRUCT(BlueprintType)
struct FWorld_BiomeFoliageConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Foliage")
    float TreeDensityPerHectare = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Foliage")
    float FernDensityPerHectare = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Foliage")
    float RockDensityPerHectare = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Foliage")
    float CycadDensityPerHectare = 40.0f;

    /** LOD bias — 0=full quality, 1=reduced, 2=minimal. Matches scalability tiers. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Foliage")
    int32 LODBias = 0;

    /** Max draw distance in cm for foliage instances */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Foliage")
    float MaxDrawDistanceCM = 15000.0f;
};

/**
 * Complete biome definition for PCG world generation.
 */
USTRUCT(BlueprintType)
struct FWorld_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FString BiomeName = TEXT("Open Plains");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FWorld_BiomeHeightConfig HeightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FWorld_BiomeFoliageConfig FoliageConfig;

    /** World Partition streaming cell size in cm. Minimum 12800cm (128m) per perf budget. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Streaming")
    float StreamingCellSizeCM = 12800.0f;

    /** Ambient temperature in Celsius — affects survival system */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Survival")
    float AmbientTemperatureC = 28.0f;

    /** Humidity 0-1 — affects thirst drain rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Survival")
    float Humidity = 0.6f;

    /** Dinosaur spawn weight multiplier for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Dinosaurs")
    float DinoSpawnWeightMultiplier = 1.0f;
};

/**
 * PCG Biome Configuration Asset — defines all biomes for the Cretaceous world.
 * Agent #05 — Procedural World Generator
 * Cycle: PROD_CYCLE_AUTO_20260622_012
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPCGBiomeConfig : public UObject
{
    GENERATED_BODY()

public:
    UPCGBiomeConfig();

    /** All biome definitions for the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    TArray<FWorld_BiomeDefinition> BiomeDefinitions;

    /** World size in cm (8km = 800000cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Scale")
    float WorldSizeCM = 800000.0f;

    /** Landscape resolution — 4033x4033 for 8km² at 200cm/vertex */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    int32 LandscapeResolution = 4033;

    /** Global foliage density scale — synced with DefaultScalability.ini */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Foliage")
    float GlobalFoliageDensityScale = 1.0f;

    /** Global grass density scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Foliage")
    float GlobalGrassDensityScale = 1.0f;

    /** Get biome definition by type */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    FWorld_BiomeDefinition GetBiomeDefinition(EWorld_BiomeType BiomeType) const;

    /** Get biome type at world position */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    EWorld_BiomeType GetBiomeAtPosition(FVector WorldPosition) const;

    /** Initialize default Cretaceous biome set */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biomes")
    void InitializeDefaultBiomes();
};
