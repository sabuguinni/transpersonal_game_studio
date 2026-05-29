#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "ProceduralWorldManager.generated.h"

class UPCGComponent;
class UPCGGraph;
class ALandscape;
class AWaterBody;

/**
 * Biome types for the prehistoric world
 * Based on Jurassic/Cretaceous period environments
 */
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    None                UMETA(DisplayName = "None"),
    DenseForest         UMETA(DisplayName = "Dense Forest"),
    OpenWoodland        UMETA(DisplayName = "Open Woodland"),
    Swampland           UMETA(DisplayName = "Swampland"),
    RiverDelta          UMETA(DisplayName = "River Delta"),
    CoastalPlains       UMETA(DisplayName = "Coastal Plains"),
    VolcanicRegion      UMETA(DisplayName = "Volcanic Region"),
    Highlands           UMETA(DisplayName = "Highlands"),
    CanyonLands         UMETA(DisplayName = "Canyon Lands")
};

/**
 * Climate zones affecting vegetation and dinosaur distribution
 */
UENUM(BlueprintType)
enum class EClimateZone : uint8
{
    Tropical            UMETA(DisplayName = "Tropical"),
    Subtropical         UMETA(DisplayName = "Subtropical"),
    Temperate           UMETA(DisplayName = "Temperate"),
    Arid                UMETA(DisplayName = "Arid")
};

/**
 * Data asset defining biome characteristics
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBiomeDefinition : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Biome type identifier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    /** Climate zone for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EClimateZone ClimateZone = EClimateZone::Tropical;

    /** PCG Graph for terrain generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    TSoftObjectPtr<UPCGGraph> TerrainGraph;

    /** PCG Graph for vegetation generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    TSoftObjectPtr<UPCGGraph> VegetationGraph;

    /** PCG Graph for rock/geological features */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    TSoftObjectPtr<UPCGGraph> GeologyGraph;

    /** Height range for this biome (min, max in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    FVector2D HeightRange = FVector2D(0.0f, 50000.0f);

    /** Slope range in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    FVector2D SlopeRange = FVector2D(0.0f, 45.0f);

    /** Moisture level (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MoistureLevel = 0.5f;

    /** Temperature range in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    FVector2D TemperatureRange = FVector2D(20.0f, 35.0f);

    /** Vegetation density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float VegetationDensity = 1.0f;

    /** Dominant tree species for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> DominantTrees;

    /** Understory vegetation meshes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> UnderstoryPlants;

    /** Ground cover vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> GroundCover;

    /** Rock and geological feature meshes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geology")
    TArray<TSoftObjectPtr<UStaticMesh>> RockFormations;
};

/**
 * World generation configuration
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UWorldGenerationConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    /** World size in kilometers */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Scale")
    float WorldSizeKm = 16.0f;

    /** Landscape resolution (vertices per side) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape")
    int32 LandscapeResolution = 8129;

    /** Component size for World Partition */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Partition")
    int32 ComponentSize = 254;

    /** Sections per component */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Partition")
    int32 SectionsPerComponent = 4;

    /** Z scale for heightmap import */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape")
    float HeightmapZScale = 100.0f;

    /** Available biome definitions */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biomes")
    TArray<TSoftObjectPtr<UBiomeDefinition>> BiomeDefinitions;

    /** Noise settings for biome distribution */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Distribution")
    float BiomeNoiseScale = 0.001f;

    /** Biome transition blend distance in meters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Distribution")
    float BiomeBlendDistance = 1000.0f;

    /** River generation settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water Systems")
    bool bGenerateRivers = true;

    /** Number of major rivers */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water Systems", meta = (EditCondition = "bGenerateRivers"))
    int32 MajorRiverCount = 3;

    /** Minimum river length in meters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water Systems", meta = (EditCondition = "bGenerateRivers"))
    float MinRiverLength = 5000.0f;
};

/**
 * Main manager for procedural world generation
 * Coordinates PCG systems, biome distribution, and World Partition
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API AProceduralWorldManager : public AActor
{
    GENERATED_BODY()

public:
    AProceduralWorldManager();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    /** World generation configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TSoftObjectPtr<UWorldGenerationConfig> WorldConfig;

    /** Main PCG component for world generation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TObjectPtr<UPCGComponent> MainPCGComponent;

    /** Generated landscape reference */
    UPROPERTY(BlueprintReadOnly, Category = "Generated Content")
    TObjectPtr<ALandscape> GeneratedLandscape;

    /** Generated water bodies */
    UPROPERTY(BlueprintReadOnly, Category = "Generated Content")
    TArray<TObjectPtr<AWaterBody>> GeneratedWaterBodies;

public:
    /** Generate the complete world */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld();

    /** Generate only the base terrain */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrain();

    /** Generate biome distribution */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomes();

    /** Generate water systems (rivers, lakes) */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWaterSystems();

    /** Generate vegetation for all biomes */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateVegetation();

    /** Clear all generated content */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearGeneratedContent();

    /** Get biome type at world location */
    UFUNCTION(BlueprintCallable, Category = "World Query")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get biome definition for a biome type */
    UFUNCTION(BlueprintCallable, Category = "World Query")
    UBiomeDefinition* GetBiomeDefinition(EWorld_BiomeType BiomeType) const;

    /** Check if location is suitable for dinosaur spawning */
    UFUNCTION(BlueprintCallable, Category = "World Query")
    bool IsLocationSuitableForDinosaurs(const FVector& WorldLocation, float RequiredClearance = 500.0f) const;

    /** Find suitable locations for player base construction */
    UFUNCTION(BlueprintCallable, Category = "World Query")
    TArray<FVector> FindBaseBuildingLocations(int32 MaxLocations = 10, float MinDistanceBetween = 2000.0f) const;

private:
    /** Internal biome noise generation */
    float GenerateBiomeNoise(const FVector2D& Location) const;

    /** Calculate biome weights at location */
    TMap<EWorld_BiomeType, float> CalculateBiomeWeights(const FVector& WorldLocation) const;

    /** Setup World Partition for the generated world */
    void SetupWorldPartition();

    /** Cached biome definitions for quick access */
    UPROPERTY()
    TMap<EWorld_BiomeType, TObjectPtr<UBiomeDefinition>> CachedBiomeDefinitions;
};