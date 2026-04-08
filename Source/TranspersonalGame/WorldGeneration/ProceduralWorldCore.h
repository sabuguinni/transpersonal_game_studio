// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "Landscape/Classes/Landscape.h"
#include "Engine/DataTable.h"
#include "Math/UnrealMathUtility.h"
#include "ProceduralWorldCore.generated.h"

/**
 * @brief Core Procedural World Generation System
 * 
 * Generates the Jurassic survival world with the following specifications:
 * - Regional scale: ~200km² of explorable terrain
 * - 5 distinct biomes: Forest (starting), Swamp, Savana, Desert, Snowy Rockside
 * - Impassable mountain borders creating natural boundaries
 * - Dynamic river system flowing from mountains to center
 * - Transition zones between biomes for natural blending
 * - World Partition integration for streaming large worlds
 * - Hierarchical PCG generation for performance optimization
 * 
 * Design Philosophy:
 * "Um mundo não é um cenário pintado — é um sistema físico com razões para cada vale, 
 * para cada rio, para cada estrada. Quando o Miguel caminha no mundo que criaste, 
 * deve sentir que aquele mundo existia antes de ele chegar."
 * 
 * @author Procedural World Generator — Agent #5
 * @version 1.0 — March 2026
 */

/** Biome types following the Geographic Guide */
UENUM(BlueprintType)
enum class EJurassicBiome : uint8
{
    Forest          UMETA(DisplayName = "Forest (Tropical Forest)"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Savana          UMETA(DisplayName = "Savana"),
    Desert          UMETA(DisplayName = "Desert"),
    SnowyRockside   UMETA(DisplayName = "Snowy Rockside"),
    MountainBorder  UMETA(DisplayName = "Mountain Border (Impassable)"),
    Transition      UMETA(DisplayName = "Transition Zone")
};

/** Terrain generation method for different areas */
UENUM(BlueprintType)
enum class ETerrainGenerationType : uint8
{
    Heightfield     UMETA(DisplayName = "Heightfield (Mountains, Hills)"),
    Flat            UMETA(DisplayName = "Flat (Plains, Swamps)"),
    Valley          UMETA(DisplayName = "Valley (River Channels)"),
    Plateau         UMETA(DisplayName = "Plateau (Desert Mesas)"),
    Coastal         UMETA(DisplayName = "Coastal (Lake Shores)")
};

/** Water flow characteristics per biome */
UENUM(BlueprintType)
enum class EWaterFlowType : uint8
{
    FastRiver       UMETA(DisplayName = "Fast River (Forest)"),
    SlowRiver       UMETA(DisplayName = "Slow River (Swamp)"),
    SeasonalStream  UMETA(DisplayName = "Seasonal Stream (Savana)"),
    DryBed          UMETA(DisplayName = "Dry Riverbed (Desert)"),
    FrozenStream    UMETA(DisplayName = "Frozen Stream (Snowy)")
};

/** Biome configuration data */
USTRUCT(BlueprintType)
struct FBiomeGenerationSettings
{
    GENERATED_BODY()

    /** Biome type identifier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EJurassicBiome BiomeType = EJurassicBiome::Forest;

    /** Elevation range for this biome (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (ClampMin = "0", ClampMax = "300000"))
    FVector2D ElevationRange = FVector2D(0.0f, 50000.0f);

    /** Slope angle range (in degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (ClampMin = "0", ClampMax = "90"))
    FVector2D SlopeRange = FVector2D(0.0f, 30.0f);

    /** Distance from water sources (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector2D WaterDistanceRange = FVector2D(0.0f, 500000.0f);

    /** Terrain generation method for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    ETerrainGenerationType TerrainType = ETerrainGenerationType::Heightfield;

    /** Water flow characteristics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    EWaterFlowType WaterFlowType = EWaterFlowType::FastRiver;

    /** Noise parameters for terrain generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float NoiseFrequency = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float NoiseAmplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    int32 NoiseOctaves = 6;

    /** Transition zone width to adjacent biomes (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionWidth = 200000.0f; // 2km transition zones

    /** PCG Graph for this biome's content generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<class UPCGGraph> BiomePCGGraph;

    /** Climate settings affecting generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Temperature = 25.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Humidity = 0.6f; // 0-1 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float WindStrength = 0.3f; // 0-1 range

    /** Performance settings for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 PCGGridSize = 12800; // cm, for hierarchical generation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StreamingDistance = 500000.0f; // 5km streaming distance
};

/** World generation configuration */
USTRUCT(BlueprintType)
struct FWorldGenerationConfig
{
    GENERATED_BODY()

    /** Total world size (landscape vertices) - using UE5 recommended size */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    int32 LandscapeSize = 8129; // 8129x8129 for ~200km² at 25m resolution

    /** Scale factor (cm per landscape unit) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    float WorldScale = 2500.0f; // 25m per landscape unit for 200km² total

    /** Maximum height variation (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    float MaxElevation = 300000.0f; // 3km mountain peaks

    /** Mountain border settings (impassable boundaries) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Border")
    float BorderWidth = 1000000.0f; // 10km impassable border

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Border")
    float BorderHeight = 250000.0f; // 2.5km high mountains

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Border")
    float BorderSteepness = 0.9f; // Very steep, impassable

    /** River system configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    int32 MainRiverCount = 4; // Major rivers from mountains

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    int32 TributaryCount = 12; // Secondary rivers

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    float RiverWidth = 5000.0f; // 50m wide rivers

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    float RiverDepth = 500.0f; // 5m deep rivers

    /** Lake generation in transition zones */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lakes")
    int32 LakeCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lakes")
    FVector2D LakeSizeRange = FVector2D(50000.0f, 200000.0f); // 0.5-2km diameter

    /** Player spawn location (always in Forest biome) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Spawn")
    FVector PlayerSpawnOffset = FVector(0.0f, 0.0f, 0.0f); // Offset from forest center

    /** Performance optimization settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseHierarchicalGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 WorldPartitionCellSize = 128000; // 1.28km cells

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StreamingRange = 640000.0f; // 6.4km streaming range

    /** Biome distribution settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    float ForestCenterRadius = 2000000.0f; // 20km forest in center

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    float BiomeTransitionSmoothness = 0.7f; // How smooth transitions are

    /** Generation seed for reproducible worlds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 WorldSeed = 12345;

    /** Enable debug visualization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowBiomeDebugColors = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowRiverDebugLines = false;
};

/**
 * @brief Core component for procedural world generation
 * 
 * This component handles the high-level orchestration of world generation:
 * 1. Terrain heightfield generation using noise functions
 * 2. Biome placement based on elevation, slope, and distance rules
 * 3. River system generation from mountains to center
 * 4. Lake placement in transition zones
 * 5. PCG graph execution for each biome
 * 6. World Partition setup for streaming
 */
UCLASS(ClassGroup=(WorldGeneration), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UProceduralWorldCore : public UActorComponent
{
    GENERATED_BODY()

public:
    UProceduralWorldCore();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Initialize world generation with configuration
     * 
     * Sets up the world generation parameters and prepares PCG components
     * 
     * @param Config World generation configuration
     */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeWorldGeneration(const FWorldGenerationConfig& Config);

    /**
     * @brief Generate the complete world
     * 
     * Executes the full world generation pipeline:
     * 1. Generate base terrain heightfield
     * 2. Create mountain borders
     * 3. Generate river system
     * 4. Place biomes based on rules
     * 5. Create transition zones
     * 6. Execute PCG graphs for each biome
     */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld();

    /**
     * @brief Generate only the base terrain heightfield
     * 
     * Creates the underlying terrain structure without biome-specific content
     */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBaseTerrain();

    /**
     * @brief Generate the river system
     * 
     * Creates rivers flowing from mountain borders toward the center,
     * with different characteristics in each biome
     */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRiverSystem();

    /**
     * @brief Place biomes according to geographic rules
     * 
     * Assigns biome types to terrain areas based on:
     * - Distance from center (Forest in center)
     * - Elevation (Desert on plateaus, Swamp in lowlands)
     * - Water proximity (Swamp near rivers)
     * - Climate gradients (Snow in mountains)
     */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PlaceBiomes();

    /**
     * @brief Generate lakes in transition zones
     * 
     * Places lakes at biome boundaries to create natural transitions
     * and provide water sources for dinosaur behavior
     */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateLakes();

    /**
     * @brief Execute PCG graphs for all biomes
     * 
     * Runs the biome-specific PCG graphs to populate each area with
     * appropriate vegetation, rocks, and other environmental features
     */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ExecuteBiomePCGGraphs();

    /**
     * @brief Get biome type at world location
     * 
     * @param WorldLocation Location to query
     * @return Biome type at that location
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    EJurassicBiome GetBiomeAtLocation(const FVector& WorldLocation) const;

    /**
     * @brief Get terrain elevation at world location
     * 
     * @param WorldLocation Location to query
     * @return Elevation in cm
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    float GetElevationAtLocation(const FVector& WorldLocation) const;

    /**
     * @brief Get nearest water source distance
     * 
     * @param WorldLocation Location to query
     * @return Distance to nearest river or lake in cm
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    float GetWaterDistanceAtLocation(const FVector& WorldLocation) const;

    /**
     * @brief Get player spawn location in forest biome
     * 
     * @return Safe spawn location in the forest biome center
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    FVector GetPlayerSpawnLocation() const;

    /**
     * @brief Regenerate specific biome
     * 
     * @param BiomeType Biome to regenerate
     * @param bClearExisting Whether to clear existing content first
     */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void RegenerateBiome(EJurassicBiome BiomeType, bool bClearExisting = true);

    /**
     * @brief Setup World Partition for streaming
     * 
     * Configures World Partition cells and streaming settings
     * for optimal performance with large worlds
     */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SetupWorldPartition();

    /**
     * @brief Enable/disable debug visualization
     * 
     * @param bShowBiomes Show biome color overlay
     * @param bShowRivers Show river debug lines
     * @param bShowElevation Show elevation heatmap
     */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void SetDebugVisualization(bool bShowBiomes, bool bShowRivers, bool bShowElevation);

protected:
    /** World generation configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    FWorldGenerationConfig WorldConfig;

    /** Biome generation settings for each biome type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TMap<EJurassicBiome, FBiomeGenerationSettings> BiomeSettings;

    /** Master PCG component for world-level generation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPCGComponent* MasterPCGComponent;

    /** Generated landscape reference */
    UPROPERTY(BlueprintReadOnly, Category = "Generated Content")
    class ALandscape* GeneratedLandscape;

    /** Generated water bodies */
    UPROPERTY(BlueprintReadOnly, Category = "Generated Content")
    TArray<class AWaterBody*> GeneratedRivers;

    UPROPERTY(BlueprintReadOnly, Category = "Generated Content")
    TArray<class AWaterBody*> GeneratedLakes;

    /** Biome data texture for runtime queries */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    class UTexture2D* BiomeDataTexture;

    /** Elevation data texture for runtime queries */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    class UTexture2D* ElevationDataTexture;

    /** Water distance data texture for runtime queries */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    class UTexture2D* WaterDistanceTexture;

private:
    /** Generate heightfield using noise functions */
    void GenerateHeightfield();

    /** Create impassable mountain borders */
    void GenerateMountainBorders();

    /** Calculate biome weights for each location */
    TMap<EJurassicBiome, float> CalculateBiomeWeights(const FVector2D& Location) const;

    /** Generate noise value at location */
    float GenerateNoise(const FVector2D& Location, float Frequency, int32 Octaves, float Persistence) const;

    /** Create biome data texture for runtime queries */
    void CreateBiomeDataTexture();

    /** Create elevation data texture for runtime queries */
    void CreateElevationDataTexture();

    /** Create water distance texture for runtime queries */
    void CreateWaterDistanceTexture();

    /** Convert world location to texture coordinates */
    FVector2D WorldToTextureCoordinates(const FVector& WorldLocation) const;

    /** Convert texture coordinates to world location */
    FVector TextureToWorldCoordinates(const FVector2D& TextureCoords) const;

    /** Generation state tracking */
    bool bIsGenerating = false;
    bool bTerrainGenerated = false;
    bool bRiversGenerated = false;
    bool bBiomesPlaced = false;
    bool bLakesGenerated = false;
    bool bPCGExecuted = false;

    /** Performance tracking */
    float GenerationStartTime = 0.0f;
    FString CurrentGenerationStep = TEXT("Idle");
};