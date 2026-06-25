// PCGWorldGenerator.h
// Procedural World Generator — Agent #5
// Transpersonal Game Studio — Dinosaur Survival Game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCGWorldGenerator.generated.h"

// ============================================================
// EWorld_BiomeType — biome classification enum
// ============================================================
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    JungleForest    UMETA(DisplayName = "Jungle Forest"),
    OpenSavanna     UMETA(DisplayName = "Open Savanna"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    RiverDelta      UMETA(DisplayName = "River Delta"),
    CoastalFlats    UMETA(DisplayName = "Coastal Flats"),
};

// ============================================================
// FWorld_BiomeCell — data for a single biome region
// ============================================================
USTRUCT(BlueprintType)
struct FWorld_BiomeCell
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenSavanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 10000.0f;

    /** Temperature in Celsius (Cretaceous era range: 18-35°C) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 28.0f;

    /** Humidity 0.0 (arid) to 1.0 (saturated) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    /** Vegetation density 0.0 (bare) to 1.0 (dense canopy) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.4f;
};

// ============================================================
// APCGWorldGenerator — main world generation actor
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "PCG World Generator"))
class TRANSPERSONALGAME_API APCGWorldGenerator : public AActor
{
    GENERATED_BODY()

public:
    APCGWorldGenerator();

protected:
    virtual void BeginPlay() override;

public:
    // --------------------------------------------------------
    // Generation Parameters
    // --------------------------------------------------------

    /** Random seed for deterministic world generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 BiomeSeed;

    /** World size in Unreal units (X axis) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float WorldSizeX;

    /** World size in Unreal units (Y axis) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float WorldSizeY;

    /** Maximum terrain height variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float TerrainHeightScale;

    /** If true, generate world automatically on BeginPlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    bool bGenerateOnBeginPlay;

    /** Biome type to weight mapping (must sum to ~1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TMap<EWorld_BiomeType, float> BiomeWeights;

    // --------------------------------------------------------
    // Runtime Data
    // --------------------------------------------------------

    /** All generated biome cells */
    UPROPERTY(BlueprintReadOnly, Category = "World Generation|Runtime")
    TArray<FWorld_BiomeCell> ActiveBiomes;

    /** River source points for spline generation */
    UPROPERTY(BlueprintReadOnly, Category = "World Generation|Runtime")
    TArray<FVector> RiverSourcePoints;

    /** Landmark positions (volcanoes, rock formations) */
    UPROPERTY(BlueprintReadOnly, Category = "World Generation|Runtime")
    TArray<FVector> LandmarkLocations;

    // --------------------------------------------------------
    // Generation Functions
    // --------------------------------------------------------

    /** Trigger full world generation */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Generation")
    void GenerateWorld();

    /** Generate biome map using Voronoi-style placement */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomeMap();

    /** Generate river network flowing from highlands to coast */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRiverNetwork();

    /** Place terrain landmarks: volcanic peaks, rock formations */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PlaceTerrainLandmarks();

    /** Query biome type at a world-space location */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    // --------------------------------------------------------
    // Biome Property Helpers
    // --------------------------------------------------------

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation|Biome")
    float GetBiomeTemperature(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation|Biome")
    float GetBiomeHumidity(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation|Biome")
    float GetBiomeVegetationDensity(EWorld_BiomeType BiomeType) const;
};
