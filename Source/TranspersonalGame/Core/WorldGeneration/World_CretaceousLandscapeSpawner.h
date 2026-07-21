#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeEditorObject.h"
#include "SharedTypes.h"
#include "World_CretaceousLandscapeSpawner.generated.h"

/**
 * World_CretaceousLandscapeSpawner
 * 
 * Specialized actor for creating massive Cretaceous period landscapes in UE5.
 * Addresses purchase criteria #1: 10km x 10km landscape with 5 distinct biomes.
 * 
 * Key Features:
 * - Creates ALandscape actors with proper UE5 landscape system integration
 * - Supports massive 10km+ terrains using World Partition
 * - Biome-aware heightmap generation for realistic Cretaceous geography
 * - Performance-optimized landscape streaming and LOD management
 * - Integration with existing biome and climate systems
 * 
 * Technical Implementation:
 * - Uses UE5 Landscape API for proper terrain creation
 * - Implements heightmap generation using Perlin noise and geological rules
 * - Supports runtime landscape modification for dynamic events
 * - Integrates with World Partition for seamless streaming
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_CretaceousLandscapeSpawner : public AActor
{
    GENERATED_BODY()

public:
    AWorld_CretaceousLandscapeSpawner();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === LANDSCAPE CREATION METHODS ===

    /**
     * Create the main Cretaceous landscape with specified dimensions
     * @param SizeKm - Size in kilometers (minimum 10km for purchase criteria)
     * @param BiomeLayout - Layout configuration for the 5 biomes
     * @return Created landscape actor or nullptr if failed
     */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Landscape Creation")
    class ALandscape* CreateMainLandscape(float SizeKm = 10.0f, EWorld_BiomeType BiomeLayout = EWorld_BiomeType::CretaceousForest);

    /**
     * Generate heightmap data for Cretaceous terrain
     * @param Width - Heightmap width in pixels
     * @param Height - Heightmap height in pixels
     * @param TerrainType - Type of terrain to generate
     * @return Generated heightmap data
     */
    UFUNCTION(BlueprintCallable, Category = "Heightmap Generation")
    TArray<uint16> GenerateCretaceousHeightmap(int32 Width, int32 Height, EWorld_TerrainType TerrainType);

    /**
     * Apply biome-specific terrain modifications
     * @param Landscape - Target landscape to modify
     * @param BiomeType - Biome type to apply
     * @param BiomeBounds - World bounds for this biome
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void ApplyBiomeTerrainModifications(class ALandscape* Landscape, EWorld_BiomeType BiomeType, FBox BiomeBounds);

    /**
     * Create landscape streaming proxies for World Partition
     * @param MainLandscape - Main landscape to create proxies for
     * @param ProxySize - Size of each streaming proxy in world units
     */
    UFUNCTION(BlueprintCallable, Category = "World Partition")
    void CreateLandscapeStreamingProxies(class ALandscape* MainLandscape, float ProxySize = 200000.0f);

    // === LANDSCAPE PROPERTIES ===

    /** Target landscape size in kilometers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Settings", meta = (ClampMin = "1.0", ClampMax = "50.0"))
    float LandscapeSizeKm;

    /** Number of landscape components per axis */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Settings", meta = (ClampMin = "1", ClampMax = "32"))
    int32 ComponentCountPerAxis;

    /** Quads per landscape component */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Settings", meta = (ClampMin = "7", ClampMax = "255"))
    int32 QuadsPerComponent;

    /** Overall terrain height scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation", meta = (ClampMin = "1.0", ClampMax = "10000.0"))
    float TerrainHeightScale;

    /** Terrain roughness factor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float TerrainRoughness;

    /** Enable World Partition streaming */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableWorldPartition;

    /** Enable landscape LOD system */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLandscapeLOD;

    // === BIOME INTEGRATION ===

    /** Biome configuration for landscape regions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    TMap<EWorld_BiomeType, FWorld_BiomeConfig> BiomeConfigurations;

    /** Reference to biome manager for integration */
    UPROPERTY(BlueprintReadOnly, Category = "System Integration")
    class UWorld_BiomeManager* BiomeManager;

    /** Reference to climate system for weather integration */
    UPROPERTY(BlueprintReadOnly, Category = "System Integration")
    class UWorld_BiomeClimateSystem* ClimateSystem;

    // === LANDSCAPE REFERENCES ===

    /** Main created landscape reference */
    UPROPERTY(BlueprintReadOnly, Category = "Landscape State")
    class ALandscape* MainLandscape;

    /** Streaming proxy landscapes for World Partition */
    UPROPERTY(BlueprintReadOnly, Category = "Landscape State")
    TArray<class ALandscapeStreamingProxy*> StreamingProxies;

    /** Landscape creation status */
    UPROPERTY(BlueprintReadOnly, Category = "Landscape State")
    bool bLandscapeCreated;

    /** Last creation error message */
    UPROPERTY(BlueprintReadOnly, Category = "Landscape State")
    FString LastErrorMessage;

    // === EDITOR UTILITIES ===

    /**
     * Validate landscape creation parameters
     * @return True if parameters are valid for landscape creation
     */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    bool ValidateLandscapeParameters();

    /**
     * Calculate memory requirements for landscape
     * @return Estimated memory usage in MB
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float CalculateLandscapeMemoryUsage();

    /**
     * Get landscape world bounds
     * @return World bounds of the created landscape
     */
    UFUNCTION(BlueprintCallable, Category = "Landscape Info")
    FBox GetLandscapeWorldBounds();

protected:
    // === INTERNAL METHODS ===

    /** Initialize landscape creation parameters */
    void InitializeLandscapeParameters();

    /** Create heightmap using Perlin noise and geological rules */
    TArray<uint16> GeneratePerlinHeightmap(int32 Width, int32 Height, float Scale, int32 Octaves);

    /** Apply geological features to heightmap */
    void ApplyGeologicalFeatures(TArray<uint16>& Heightmap, int32 Width, int32 Height);

    /** Setup landscape material and layers */
    void SetupLandscapeMaterials(class ALandscape* Landscape);

    /** Configure landscape collision */
    void ConfigureLandscapeCollision(class ALandscape* Landscape);

private:
    /** Cached landscape editor object for creation */
    UPROPERTY()
    class ULandscapeEditorObject* LandscapeEditorObject;

    /** Performance monitoring */
    float LastCreationTime;
    int32 CreationAttempts;
};