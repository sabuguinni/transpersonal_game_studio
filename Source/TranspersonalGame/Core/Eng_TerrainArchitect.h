#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Landscape/Landscape.h"
#include "LandscapeInfo.h"
#include "SharedTypes.h"
#include "Eng_TerrainArchitect.generated.h"

/**
 * Engine Architect - Terrain Architecture System
 * Manages terrain generation, height maps, and landscape streaming
 * Coordinates with BiomeSystemManager for environmental placement
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_TerrainArchitect : public AActor
{
    GENERATED_BODY()

public:
    AEng_TerrainArchitect();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === TERRAIN CONFIGURATION ===
    
    /** World size in Unreal units (target: 10km x 10km) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector2D WorldSize;

    /** Landscape component size */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 ComponentSize;

    /** Number of landscape components per axis */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 ComponentsPerAxis;

    /** Height scale for terrain generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale;

    /** Current landscape reference */
    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    ALandscape* MainLandscape;

    // === HEIGHT MAP GENERATION ===

    /** Noise scale for terrain generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Generation")
    float NoiseScale;

    /** Noise octaves for detail */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Generation")
    int32 NoiseOctaves;

    /** Noise persistence for roughness */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Generation")
    float NoisePersistence;

    /** Random seed for terrain generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Generation")
    int32 TerrainSeed;

    // === BIOME INTEGRATION ===

    /** Height thresholds for biome placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    TMap<EBiomeType, FVector2D> BiomeHeightRanges;

    /** Slope thresholds for biome placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    TMap<EBiomeType, float> BiomeSlopeThresholds;

    /** Moisture map for biome distribution */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    TMap<EBiomeType, float> BiomeMoistureRequirements;

    // === STREAMING SYSTEM ===

    /** Enable landscape streaming for large worlds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnableLandscapeStreaming;

    /** Streaming distance in Unreal units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingDistance;

    /** Number of landscape proxies for streaming */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 LandscapeProxyCount;

    // === CORE FUNCTIONS ===

    /** Initialize terrain architecture system */
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void InitializeTerrainSystem();

    /** Generate height map data for landscape */
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    TArray<uint16> GenerateHeightMapData(int32 Width, int32 Height);

    /** Create landscape with generated height data */
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    ALandscape* CreateLandscapeFromHeightMap(const TArray<uint16>& HeightData, int32 Width, int32 Height);

    /** Get terrain height at world coordinates */
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetTerrainHeightAtLocation(FVector WorldLocation);

    /** Get terrain slope at world coordinates */
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetTerrainSlopeAtLocation(FVector WorldLocation);

    /** Get terrain normal at world coordinates */
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    FVector GetTerrainNormalAtLocation(FVector WorldLocation);

    // === BIOME INTEGRATION FUNCTIONS ===

    /** Determine optimal biome for terrain location */
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    EBiomeType GetOptimalBiomeForTerrain(FVector WorldLocation, float Height, float Slope);

    /** Apply biome-specific terrain modifications */
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void ApplyBiomeTerrainModifications(EBiomeType BiomeType, FVector CenterLocation, float Radius);

    /** Generate moisture map for biome distribution */
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void GenerateMoistureMap();

    // === STREAMING FUNCTIONS ===

    /** Setup landscape streaming system */
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void SetupLandscapeStreaming();

    /** Update landscape streaming based on player location */
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateLandscapeStreaming(FVector PlayerLocation);

    /** Load landscape proxy at coordinates */
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void LoadLandscapeProxy(FIntPoint ProxyCoordinates);

    /** Unload landscape proxy at coordinates */
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UnloadLandscapeProxy(FIntPoint ProxyCoordinates);

    // === VALIDATION FUNCTIONS ===

    /** Validate terrain system integrity */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    bool ValidateTerrainSystem();

    /** Get terrain system status report */
    UFUNCTION(BlueprintCallable, Category = "Validation")
    FString GetTerrainSystemStatus();

    /** Calculate terrain statistics */
    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_TerrainStats CalculateTerrainStatistics();

protected:
    /** Setup default terrain parameters */
    void SetupDefaultTerrainParameters();

    /** Setup biome height and slope requirements */
    void SetupBiomeTerrainRequirements();

    /** Generate Perlin noise for height maps */
    float GeneratePerlinNoise(float X, float Y, int32 Octaves, float Persistence, float Scale);

    /** Apply smoothing to height map data */
    void SmoothHeightMapData(TArray<uint16>& HeightData, int32 Width, int32 Height, int32 Iterations);

    /** Convert world coordinates to landscape coordinates */
    FVector2D WorldToLandscapeCoordinates(FVector WorldLocation);

    /** Convert landscape coordinates to world coordinates */
    FVector LandscapeToWorldCoordinates(FVector2D LandscapeCoords);

private:
    /** Cached landscape info for performance */
    ULandscapeInfo* CachedLandscapeInfo;

    /** Moisture map data */
    TArray<float> MoistureMapData;

    /** Moisture map dimensions */
    FIntPoint MoistureMapSize;

    /** Currently loaded landscape proxies */
    TSet<FIntPoint> LoadedProxies;

    /** Terrain generation timer */
    float TerrainUpdateTimer;

    /** Terrain update interval */
    float TerrainUpdateInterval;
};

/**
 * Terrain statistics structure for validation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_TerrainStats
{
    GENERATED_BODY()

    /** Average terrain height */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float AverageHeight;

    /** Maximum terrain height */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float MaxHeight;

    /** Minimum terrain height */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float MinHeight;

    /** Average terrain slope */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float AverageSlope;

    /** Terrain coverage area in square units */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float CoverageArea;

    /** Number of landscape components */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 ComponentCount;

    /** Memory usage in MB */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float MemoryUsageMB;

    FEng_TerrainStats()
    {
        AverageHeight = 0.0f;
        MaxHeight = 0.0f;
        MinHeight = 0.0f;
        AverageSlope = 0.0f;
        CoverageArea = 0.0f;
        ComponentCount = 0;
        MemoryUsageMB = 0.0f;
    }
};