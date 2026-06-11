#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// Forward declarations
class UPCGWorldGenerator;
class UFoliageManager;
class AStaticMeshActor;

/**
 * Biome Data Structure - defines characteristics of each biome type
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData : public FTableRowBase
{
    GENERATED_BODY()

    FEng_BiomeData()
        : BiomeType(EBiomeType::Temperate_Forest)
        , Temperature(20.0f)
        , Humidity(0.5f)
        , Elevation(100.0f)
        , VegetationDensity(0.7f)
        , WaterPresence(0.3f)
        , DinosaurSpawnRate(1.0f)
        , ResourceAbundance(0.6f)
        , DangerLevel(0.4f)
    {}

    /** Type of biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType;

    /** Average temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float Temperature;

    /** Humidity level (0.0 = arid, 1.0 = very humid) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float Humidity;

    /** Typical elevation in meters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float Elevation;

    /** Vegetation density (0.0 = barren, 1.0 = dense forest) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    float VegetationDensity;

    /** Water presence (0.0 = desert, 1.0 = swamp/lake) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float WaterPresence;

    /** Dinosaur spawn rate multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wildlife")
    float DinosaurSpawnRate;

    /** Resource abundance (0.0 = scarce, 1.0 = abundant) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
    float ResourceAbundance;

    /** Danger level for player (0.0 = safe, 1.0 = extremely dangerous) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    float DangerLevel;

    /** Primary foliage types for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> PrimaryFoliage;

    /** Secondary foliage types for variety */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> SecondaryFoliage;

    /** Ground material for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> GroundMaterial;
};

/**
 * Biome Transition Zone - defines how biomes blend into each other
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    FEng_BiomeTransition()
        : FromBiome(EBiomeType::Temperate_Forest)
        , ToBiome(EBiomeType::Grassland)
        , TransitionDistance(500.0f)
        , BlendCurve(nullptr)
    {}

    /** Source biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    EBiomeType FromBiome;

    /** Target biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    EBiomeType ToBiome;

    /** Distance over which transition occurs (in UE units) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    float TransitionDistance;

    /** Curve defining how the blend occurs */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    TSoftObjectPtr<UCurveFloat> BlendCurve;
};

/**
 * BiomeManager - World Subsystem responsible for biome generation, management and transitions
 * This is the core architecture component that coordinates all biome-related systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core biome management functions
    
    /** Initialize biome system with world data */
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiomeSystem();

    /** Get biome type at specific world location */
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get biome data for a specific biome type */
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FEng_BiomeData GetBiomeData(EBiomeType BiomeType) const;

    /** Generate biome map for given world bounds */
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void GenerateBiomeMap(const FVector2D& WorldSize, int32 Resolution = 512);

    /** Apply biome characteristics to world region */
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void ApplyBiomeToRegion(const FVector& Center, float Radius, EBiomeType BiomeType);

    /** Get all biomes within a radius of location */
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    TArray<EBiomeType> GetBiomesInRadius(const FVector& Center, float Radius) const;

    /** Calculate biome transition blend factor between two points */
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    float CalculateTransitionBlend(const FVector& LocationA, const FVector& LocationB) const;

    // Integration with other systems

    /** Register PCG World Generator for biome-based generation */
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RegisterWorldGenerator(UPCGWorldGenerator* WorldGen);

    /** Register Foliage Manager for vegetation placement */
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RegisterFoliageManager(UFoliageManager* FoliageMgr);

    /** Notify biome manager of terrain changes */
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void OnTerrainModified(const FVector& Location, float Radius);

    // Debug and visualization

    /** Draw biome boundaries for debugging */
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DrawBiomeBoundaries(bool bEnabled = true);

    /** Get biome statistics for current world */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    TMap<EBiomeType, float> GetBiomeStatistics() const;

    /** Validate biome configuration */
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    bool ValidateBiomeConfiguration() const;

protected:
    /** Data table containing all biome configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TSoftObjectPtr<UDataTable> BiomeDataTable;

    /** Array of biome transition rules */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    /** Resolution of the biome map (pixels per world unit) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float BiomeMapResolution;

    /** Maximum distance for biome influence */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float MaxBiomeInfluenceDistance;

    /** Whether to use noise-based biome generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    bool bUseNoiseGeneration;

    /** Noise scale for biome generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation", meta = (EditCondition = "bUseNoiseGeneration"))
    float NoiseScale;

    /** Noise octaves for biome generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation", meta = (EditCondition = "bUseNoiseGeneration"))
    int32 NoiseOctaves;

private:
    // Internal biome map storage
    TArray<TArray<EBiomeType>> BiomeMap;
    FVector2D BiomeMapSize;
    int32 BiomeMapResolutionInt;

    // System references
    UPROPERTY()
    TObjectPtr<UPCGWorldGenerator> WorldGenerator;

    UPROPERTY()
    TObjectPtr<UFoliageManager> FoliageManager;

    // Internal helper functions
    void LoadBiomeDataTable();
    void GenerateNoiseBiomeMap();
    void ApplyBiomeTransitions();
    EBiomeType SampleBiomeFromNoise(float NoiseValue, const FVector& Location) const;
    void CacheBiomeData();

    // Cached biome data for performance
    TMap<EBiomeType, FEng_BiomeData> CachedBiomeData;
    bool bBiomeDataCached;

    // Debug visualization
    bool bDrawBiomeBoundaries;
    TArray<FVector> BiomeBoundaryPoints;
};