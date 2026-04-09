#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataAsset.h"
#include "FoliageType.h"
#include "InstancedFoliageActor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../WorldGeneration/ProceduralWorldGenerator.h"
#include "VegetationSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVegetationSystem, Log, All);

/**
 * Vegetation Types for Jurassic Environment
 */
UENUM(BlueprintType)
enum class EVegetationType : uint8
{
    // Trees
    ConiferousTree         UMETA(DisplayName = "Coniferous Tree"),
    BroadleafTree         UMETA(DisplayName = "Broadleaf Tree"),
    PalmTree              UMETA(DisplayName = "Palm Tree"),
    CycadTree             UMETA(DisplayName = "Cycad Tree"),
    GinkgoTree            UMETA(DisplayName = "Ginkgo Tree"),
    
    // Ferns and Undergrowth
    TreeFern              UMETA(DisplayName = "Tree Fern"),
    SmallFern             UMETA(DisplayName = "Small Fern"),
    Horsetail             UMETA(DisplayName = "Horsetail"),
    Moss                  UMETA(DisplayName = "Moss"),
    
    // Aquatic Plants
    WaterLily             UMETA(DisplayName = "Water Lily"),
    Algae                 UMETA(DisplayName = "Algae"),
    ReedGrass             UMETA(DisplayName = "Reed Grass"),
    
    // Ground Cover
    LowShrub              UMETA(DisplayName = "Low Shrub"),
    Grass                 UMETA(DisplayName = "Grass"),
    Flowers               UMETA(DisplayName = "Flowers")
};

/**
 * Vegetation Density Levels
 */
UENUM(BlueprintType)
enum class EVegetationDensity : uint8
{
    Sparse                UMETA(DisplayName = "Sparse"),
    Light                 UMETA(DisplayName = "Light"),
    Medium                UMETA(DisplayName = "Medium"),
    Dense                 UMETA(DisplayName = "Dense"),
    VeryDense             UMETA(DisplayName = "Very Dense")
};

/**
 * Growth Conditions for Vegetation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVegetationGrowthConditions
{
    GENERATED_BODY()

    /** Minimum elevation for growth (meters) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevation")
    float MinElevation = 0.0f;

    /** Maximum elevation for growth (meters) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevation")
    float MaxElevation = 2000.0f;

    /** Minimum slope angle (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MinSlope = 0.0f;

    /** Maximum slope angle (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MaxSlope = 45.0f;

    /** Distance from water sources (meters) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydrology")
    float WaterDistance = 1000.0f;

    /** Requires water proximity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydrology")
    bool bRequiresWater = false;

    /** Preferred biomes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EBiomeType> PreferredBiomes;

    /** Minimum temperature (Celsius) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float MinTemperature = 15.0f;

    /** Maximum temperature (Celsius) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float MaxTemperature = 35.0f;

    /** Minimum humidity (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float MinHumidity = 0.3f;

    /** Maximum humidity (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float MaxHumidity = 1.0f;
};

/**
 * Vegetation Species Configuration
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UVegetationSpeciesAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Species name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species")
    FString SpeciesName = TEXT("Unknown Species");

    /** Vegetation type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species")
    EVegetationType VegetationType = EVegetationType::Grass;

    /** Static mesh for this species */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
    TSoftObjectPtr<UStaticMesh> SpeciesMesh;

    /** LOD meshes for distance rendering */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
    TArray<TSoftObjectPtr<UStaticMesh>> LODMeshes;

    /** Material overrides */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialOverrides;

    /** Growth conditions */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Growth")
    FVegetationGrowthConditions GrowthConditions;

    /** Base density per square kilometer */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Density", meta = (ClampMin = "0.0"))
    float BaseDensity = 100.0f;

    /** Size variation range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FVector2D SizeVariation = FVector2D(0.8f, 1.2f);

    /** Rotation randomness */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    bool bRandomRotation = true;

    /** Wind responsiveness (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WindResponsiveness = 0.5f;

    /** Seasonal variation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Seasonal")
    bool bHasSeasonalVariation = false;

    /** Collision enabled */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    bool bEnableCollision = true;

    /** Cull distance */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float CullDistance = 10000.0f;

    /** Shadow casting */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendering")
    bool bCastShadows = true;

    /** Nanite enabled */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendering")
    bool bNaniteEnabled = false;
};

/**
 * Vegetation Cluster Configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVegetationCluster
{
    GENERATED_BODY()

    /** Center position of cluster */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
    FVector ClusterCenter = FVector::ZeroVector;

    /** Cluster radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size")
    float ClusterRadius = 500.0f;

    /** Primary species in cluster */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TSoftObjectPtr<UVegetationSpeciesAsset> PrimarySpecies;

    /** Secondary species (undergrowth, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TArray<TSoftObjectPtr<UVegetationSpeciesAsset>> SecondarySpecies;

    /** Density multiplier for this cluster */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density", meta = (ClampMin = "0.0"))
    float DensityMultiplier = 1.0f;

    /** Biome type for this cluster */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EBiomeType BiomeType = EBiomeType::TropicalRainforest;
};

/**
 * Vegetation System - Manages procedural vegetation placement and rendering
 * Uses UE5 Foliage system with Nanite support for massive vegetation scenes
 */
UCLASS()
class TRANSPERSONALGAME_API UVegetationSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Get the vegetation system instance */
    UFUNCTION(BlueprintPure, Category = "Vegetation")
    static UVegetationSystem* Get(const UObject* WorldContext);

    /** Generate vegetation for the entire world */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void GenerateWorldVegetation();

    /** Generate vegetation for a specific biome */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void GenerateBiomeVegetation(EBiomeType BiomeType, const FVector& Center, float Radius);

    /** Place vegetation cluster at location */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void PlaceVegetationCluster(const FVegetationCluster& ClusterConfig);

    /** Remove vegetation in area */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void ClearVegetationInArea(const FVector& Center, float Radius);

    /** Update vegetation LOD based on camera position */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void UpdateVegetationLOD(const FVector& ViewerPosition);

    /** Get vegetation density at position */
    UFUNCTION(BlueprintPure, Category = "Vegetation")
    float GetVegetationDensityAtPosition(const FVector& WorldPosition) const;

    /** Check if position is suitable for vegetation type */
    UFUNCTION(BlueprintPure, Category = "Vegetation")
    bool IsPositionSuitableForVegetation(const FVector& WorldPosition, EVegetationType VegetationType) const;

    /** Get dominant vegetation type at position */
    UFUNCTION(BlueprintPure, Category = "Vegetation")
    EVegetationType GetDominantVegetationAtPosition(const FVector& WorldPosition) const;

    /** Set vegetation density multiplier for biome */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SetBiomeDensityMultiplier(EBiomeType BiomeType, float Multiplier);

    /** Enable/disable vegetation rendering */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SetVegetationRenderingEnabled(bool bEnabled);

    /** Get vegetation rendering statistics */
    UFUNCTION(BlueprintPure, Category = "Vegetation")
    FString GetVegetationStats() const;

protected:
    /** Vegetation species database */
    UPROPERTY()
    TMap<EVegetationType, TObjectPtr<UVegetationSpeciesAsset>> VegetationSpecies;

    /** Foliage actor for instanced rendering */
    UPROPERTY()
    TObjectPtr<AInstancedFoliageActor> FoliageActor;

    /** Foliage types for each vegetation species */
    UPROPERTY()
    TMap<EVegetationType, TObjectPtr<UFoliageType>> FoliageTypes;

    /** Vegetation clusters */
    UPROPERTY()
    TArray<FVegetationCluster> VegetationClusters;

    /** Biome density multipliers */
    UPROPERTY()
    TMap<EBiomeType, float> BiomeDensityMultipliers;

    /** Current LOD level */
    UPROPERTY()
    int32 CurrentLODLevel = 0;

    /** Vegetation rendering enabled */
    UPROPERTY()
    bool bVegetationRenderingEnabled = true;

    /** Generation progress */
    UPROPERTY()
    float GenerationProgress = 0.0f;

private:
    // Internal generation methods
    void LoadVegetationSpecies();
    void SetupFoliageActor();
    void GenerateVegetationForBiome(EBiomeType BiomeType, const FVector& Center, float Radius);
    void PlaceVegetationInstances(EVegetationType VegetationType, const TArray<FVector>& Positions, const TArray<FRotator>& Rotations, const TArray<FVector>& Scales);
    
    // Placement algorithms
    TArray<FVector> GeneratePoissonDiskSampling(const FVector& Center, float Radius, float MinDistance, int32 MaxAttempts = 30);
    TArray<FVector> GenerateClusteredPlacement(const FVector& Center, float Radius, int32 NumClusters, float ClusterRadius);
    TArray<FVector> GenerateRandomPlacement(const FVector& Center, float Radius, int32 Count);
    
    // Validation methods
    bool ValidateGrowthConditions(const FVector& Position, const FVegetationGrowthConditions& Conditions) const;
    float CalculateVegetationSuitability(const FVector& Position, EVegetationType VegetationType) const;
    
    // LOD management
    void UpdateInstanceLOD(UFoliageType* FoliageType, const FVector& ViewerPosition);
    int32 CalculateLODLevel(float Distance, const TArray<float>& LODDistances) const;
    
    // Performance optimization
    void OptimizeFoliageInstances();
    void CullDistantVegetation(const FVector& ViewerPosition);
    
    // Utility functions
    FRotator GenerateRandomRotation(bool bRandomYaw = true, bool bRandomPitch = false, bool bRandomRoll = false) const;
    FVector GenerateRandomScale(const FVector2D& ScaleRange) const;
    float GetTerrainSlope(const FVector& Position) const;
    
    FTimerHandle LODUpdateTimer;
    FTimerHandle OptimizationTimer;
    
    // Reference to world generator
    UPROPERTY()
    TObjectPtr<UProceduralWorldGenerator> WorldGenerator;
};