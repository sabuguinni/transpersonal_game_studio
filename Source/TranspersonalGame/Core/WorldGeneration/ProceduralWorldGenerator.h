#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataAsset.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Landscape.h"
#include "ProceduralWorldGenerator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogProceduralWorld, Log, All);

// Forward declarations
class UPCGComponent;
class UPCGGraph;
class ALandscape;
class ULandscapeComponent;
class UWorldPartitionSubsystem;

/**
 * Biome Types for the Jurassic World
 */
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    TropicalRainforest      UMETA(DisplayName = "Tropical Rainforest"),
    FloodPlains            UMETA(DisplayName = "Flood Plains"),
    CoastalWetlands        UMETA(DisplayName = "Coastal Wetlands"),
    VolcanicRegions        UMETA(DisplayName = "Volcanic Regions"),
    RiverDeltas            UMETA(DisplayName = "River Deltas"),
    UplandForests          UMETA(DisplayName = "Upland Forests"),
    OpenWoodlands          UMETA(DisplayName = "Open Woodlands"),
    LagoonSystems          UMETA(DisplayName = "Lagoon Systems")
};

/**
 * Terrain Feature Types
 */
UENUM(BlueprintType)
enum class ETerrainFeature : uint8
{
    River                  UMETA(DisplayName = "River"),
    Lake                   UMETA(DisplayName = "Lake"),
    Mountain               UMETA(DisplayName = "Mountain"),
    Valley                 UMETA(DisplayName = "Valley"),
    Canyon                 UMETA(DisplayName = "Canyon"),
    Plateau                UMETA(DisplayName = "Plateau"),
    Cave                   UMETA(DisplayName = "Cave"),
    Waterfall              UMETA(DisplayName = "Waterfall")
};

/**
 * Biome Configuration Data Asset
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBiomeConfigurationAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Biome type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::TropicalRainforest;

    /** PCG Graph for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    TSoftObjectPtr<UPCGGraph> BiomePCGGraph;

    /** Vegetation density (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VegetationDensity = 0.7f;

    /** Average temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate")
    float Temperature = 25.0f;

    /** Humidity level (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity = 0.8f;

    /** Elevation range for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    FVector2D ElevationRange = FVector2D(0.0f, 500.0f);

    /** Allowed terrain features */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    TArray<ETerrainFeature> AllowedTerrainFeatures;
};

/**
 * World Generation Parameters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorldGenerationParams
{
    GENERATED_BODY()

    /** World size in kilometers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    FVector2D WorldSizeKm = FVector2D(20.0f, 20.0f);

    /** Heightmap resolution */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 HeightmapResolution = 4033;

    /** Maximum elevation in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MaxElevation = 2000.0f;

    /** Sea level in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float SeaLevel = 0.0f;

    /** Number of major rivers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydrology")
    int32 MajorRivers = 3;

    /** Number of lakes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydrology")
    int32 Lakes = 8;

    /** Erosion strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ErosionStrength = 0.3f;

    /** Seed for random generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 Seed = 12345;
};

/**
 * Procedural World Generator
 * Generates vast Jurassic landscapes using PCG Framework and World Partition
 */
UCLASS()
class TRANSPERSONALGAME_API UProceduralWorldGenerator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Get the procedural world generator instance */
    UFUNCTION(BlueprintPure, Category = "World Generation")
    static UProceduralWorldGenerator* Get(const UObject* WorldContext);

    /** Generate the world with specified parameters */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld(const FWorldGenerationParams& Params);

    /** Generate terrain heightmap */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrain(const FWorldGenerationParams& Params);

    /** Generate biome distribution */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomes();

    /** Generate river systems */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRivers(int32 NumRivers);

    /** Generate lake systems */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateLakes(int32 NumLakes);

    /** Apply erosion to terrain */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ApplyErosion(float Strength, int32 Iterations = 10);

    /** Setup World Partition for large world streaming */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SetupWorldPartition();

    /** Setup PCG components for procedural content */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SetupPCGComponents();

    /** Generate vegetation using PCG */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateVegetation();

    /** Generate geological features */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateGeologicalFeatures();

    /** Get biome at world position */
    UFUNCTION(BlueprintPure, Category = "World Generation")
    EBiomeType GetBiomeAtPosition(const FVector& WorldPosition) const;

    /** Get elevation at world position */
    UFUNCTION(BlueprintPure, Category = "World Generation")
    float GetElevationAtPosition(const FVector& WorldPosition) const;

    /** Check if position is near water */
    UFUNCTION(BlueprintPure, Category = "World Generation")
    bool IsNearWater(const FVector& WorldPosition, float Range = 1000.0f) const;

    /** Get generation progress (0.0 to 1.0) */
    UFUNCTION(BlueprintPure, Category = "World Generation")
    float GetGenerationProgress() const;

protected:
    /** Current world generation parameters */
    UPROPERTY()
    FWorldGenerationParams CurrentParams;

    /** Generated landscape actor */
    UPROPERTY()
    TObjectPtr<ALandscape> GeneratedLandscape;

    /** PCG components for different biomes */
    UPROPERTY()
    TMap<EBiomeType, TObjectPtr<UPCGComponent>> BiomePCGComponents;

    /** Biome configuration assets */
    UPROPERTY()
    TMap<EBiomeType, TObjectPtr<UBiomeConfigurationAsset>> BiomeConfigurations;

    /** River spline components */
    UPROPERTY()
    TArray<TObjectPtr<class USplineComponent>> RiverSplines;

    /** Lake actors */
    UPROPERTY()
    TArray<TObjectPtr<AActor>> LakeActors;

    /** Generation progress */
    UPROPERTY()
    float GenerationProgress = 0.0f;

    /** Is generation in progress */
    UPROPERTY()
    bool bGenerationInProgress = false;

private:
    // Internal generation methods
    void GenerateHeightmapData(TArray<uint16>& HeightData, int32 Resolution);
    void GenerateBiomeMap(TArray<uint8>& BiomeData, int32 Resolution);
    void CreateLandscapeActor(const TArray<uint16>& HeightData, int32 Resolution);
    void SetupLandscapeMaterials();
    void GenerateRiverSpline(const FVector& StartPoint, const FVector& EndPoint);
    void PlaceLake(const FVector& Position, float Radius);
    void ApplyHydraulicErosion(TArray<uint16>& HeightData, int32 Resolution, float Strength);
    void SetupBiomePCG(EBiomeType BiomeType);
    
    // Noise generation
    float GeneratePerlinNoise(float X, float Y, float Scale, int32 Octaves = 4) const;
    float GenerateRidgedNoise(float X, float Y, float Scale) const;
    float GenerateVoronoiNoise(float X, float Y, float Scale) const;
    
    // Utility functions
    FVector2D WorldToHeightmapCoords(const FVector& WorldPos) const;
    FVector HeightmapToWorldCoords(const FVector2D& HeightmapCoords) const;
    
    FTimerHandle GenerationTimer;
};