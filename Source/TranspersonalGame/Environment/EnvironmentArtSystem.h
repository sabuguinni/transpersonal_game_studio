// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "FoliageType.h"
#include "InstancedFoliageActor.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "WorldPartition/WorldPartition.h"
#include "../WorldGeneration/JurassicWorldPCGSystem.h"
#include "../Performance/PerformanceTargets.h"
#include "EnvironmentArtSystem.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UFoliageType;
class AInstancedFoliageActor;
class UPCGComponent;
class ALandscape;

/**
 * @brief Environment Art System - Agent #6
 * 
 * Transforms generated terrain into living, breathing prehistoric worlds.
 * 
 * NARRATIVE ENVIRONMENTAL DESIGN:
 * Every placement tells a story. A cluster of broken fern fronds near a stream 
 * suggests a large herbivore drinking. Scratched bark at shoulder height hints 
 * at territorial marking. Scattered bones near a cave entrance warn of danger.
 * 
 * ARTISTIC PHILOSOPHY (Roger Deakins + RDR2 Environmental Team):
 * - Light and composition tell the story before any character speaks
 * - Every detail exists to create an illusion of history
 * - Players should feel they're in a place that existed before they arrived
 * - The world continues to exist after they leave
 * 
 * TECHNICAL EXCELLENCE:
 * - Nanite-enabled vegetation for film-quality detail
 * - Procedural variation ensures no two trees are identical
 * - Performance-aware LOD chains maintain 60fps
 * - Streaming-optimized for massive worlds
 * 
 * PREHISTORIC AUTHENTICITY:
 * - Cretaceous period flora (no grass, no flowers)
 * - Geologically accurate rock formations
 * - Realistic erosion patterns and sediment deposits
 * - Climate-appropriate vegetation distribution
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */

/** Vegetation types for Cretaceous period */
UENUM(BlueprintType)
enum class ECretaceousVegetation : uint8
{
    // Dominant trees
    ConiferTrees        UMETA(DisplayName = "Conifer Trees (Araucaria, Pine)"),
    CycadTrees          UMETA(DisplayName = "Cycad Trees (Palm-like)"),
    GinkgoTrees         UMETA(DisplayName = "Ginkgo Trees"),
    
    // Understory
    TreeFerns           UMETA(DisplayName = "Tree Ferns (Large)"),
    Ferns               UMETA(DisplayName = "Ground Ferns"),
    Horsetails          UMETA(DisplayName = "Horsetails (Equisetum)"),
    
    // Specialized plants
    CycadShrubs         UMETA(DisplayName = "Cycad Shrubs"),
    MossPatches         UMETA(DisplayName = "Moss Patches"),
    LichenRocks         UMETA(DisplayName = "Lichen-covered Rocks"),
    
    // Aquatic vegetation
    AquaticFerns        UMETA(DisplayName = "Aquatic Ferns"),
    Algae               UMETA(DisplayName = "Algae Mats"),
    
    // Dead/Decay
    FallenLogs          UMETA(DisplayName = "Fallen Logs"),
    DeadTrees           UMETA(DisplayName = "Dead Standing Trees"),
    RottenStumps        UMETA(DisplayName = "Rotting Stumps")
};

/** Rock and geological prop types */
UENUM(BlueprintType)
enum class EGeologicalProps : uint8
{
    // Primary rock formations
    SandstoneOutcrops   UMETA(DisplayName = "Sandstone Outcrops"),
    LimestoneCliffs     UMETA(DisplayName = "Limestone Cliffs"),
    VolcanicRocks       UMETA(DisplayName = "Volcanic Rock Formations"),
    
    // Boulders and stones
    LargeBoulders       UMETA(DisplayName = "Large Boulders"),
    MediumRocks         UMETA(DisplayName = "Medium Rocks"),
    SmallStones         UMETA(DisplayName = "Small Stones"),
    
    // Erosion features
    ErodedPillars       UMETA(DisplayName = "Eroded Rock Pillars"),
    NaturalArches       UMETA(DisplayName = "Natural Rock Arches"),
    CaveEntrances       UMETA(DisplayName = "Cave Entrances"),
    
    // Sediment deposits
    GravelBeds          UMETA(DisplayName = "Gravel Beds"),
    SandDeposits        UMETA(DisplayName = "Sand Deposits"),
    ClayBanks           UMETA(DisplayName = "Clay Banks"),
    
    // Mineral formations
    CrystalFormations   UMETA(DisplayName = "Crystal Formations"),
    IronOxideStains     UMETA(DisplayName = "Iron Oxide Stains"),
    SaltDeposits        UMETA(DisplayName = "Salt Deposits")
};

/** Environmental storytelling props */
UENUM(BlueprintType)
enum class EStorytellingProps : uint8
{
    // Dinosaur evidence
    BoneScatters        UMETA(DisplayName = "Scattered Bones"),
    Coprolites          UMETA(DisplayName = "Fossilized Dung"),
    NestSites           UMETA(DisplayName = "Nest Sites"),
    Footprints          UMETA(DisplayName = "Fossilized Footprints"),
    
    // Feeding evidence
    StrippedBark        UMETA(DisplayName = "Stripped Tree Bark"),
    BrokenBranches      UMETA(DisplayName = "Broken Branches"),
    CrushedVegetation   UMETA(DisplayName = "Crushed Vegetation"),
    
    // Territorial markers
    ScratchMarks        UMETA(DisplayName = "Claw Scratch Marks"),
    RubbingPosts        UMETA(DisplayName = "Rubbing Posts"),
    
    // Natural hazards
    TarPits             UMETA(DisplayName = "Tar Pits"),
    QuicksandAreas      UMETA(DisplayName = "Quicksand Areas"),
    PoisonousPlants     UMETA(DisplayName = "Poisonous Plants"),
    
    // Resources
    FreshWaterSprings   UMETA(DisplayName = "Fresh Water Springs"),
    SaltLicks           UMETA(DisplayName = "Salt Licks"),
    ClayDeposits        UMETA(DisplayName = "Clay Deposits"),
    FlintNodes          UMETA(DisplayName = "Flint Nodes")
};

/** Vegetation density and distribution parameters */
USTRUCT(BlueprintType)
struct FVegetationDistribution
{
    GENERATED_BODY()

    /** Vegetation type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    ECretaceousVegetation VegetationType = ECretaceousVegetation::ConiferTrees;

    /** Static mesh to use */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    /** Density per square kilometer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float DensityPerSqKm = 100.0f;

    /** Size variation range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FVector2D SizeRange = FVector2D(0.8f, 1.2f);

    /** Rotation randomization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    bool bRandomizeRotation = true;

    /** Preferred biomes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology")
    TArray<EPrehistoricBiome> PreferredBiomes;

    /** Elevation range preference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology")
    FVector2D ElevationRange = FVector2D(-1000.0f, 5000.0f);

    /** Slope tolerance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology")
    float MaxSlope = 30.0f;

    /** Distance from water preference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology")
    FVector2D WaterDistanceRange = FVector2D(0.0f, 10000.0f);

    /** LOD distances */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<float> LODDistances = {5000.0f, 15000.0f, 25000.0f};

    /** Enable Nanite for this vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseNanite = true;

    /** Culling distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 50000.0f;
};

/** Rock and prop distribution parameters */
USTRUCT(BlueprintType)
struct FGeologicalPropDistribution
{
    GENERATED_BODY()

    /** Prop type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    EGeologicalProps PropType = EGeologicalProps::LargeBoulders;

    /** Static mesh to use */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    /** Material variations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;

    /** Density per square kilometer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float DensityPerSqKm = 50.0f;

    /** Size variation range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FVector2D SizeRange = FVector2D(0.5f, 2.0f);

    /** Geological formations this prop prefers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    TArray<ETectonicFormation> PreferredFormations;

    /** Slope preference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FVector2D SlopeRange = FVector2D(0.0f, 60.0f);

    /** Clustering factor (0 = scattered, 1 = highly clustered) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float ClusteringFactor = 0.3f;

    /** Enable physics simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnablePhysics = false;

    /** Enable Nanite */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseNanite = true;
};

/** Storytelling prop distribution */
USTRUCT(BlueprintType)
struct FStorytellingPropDistribution
{
    GENERATED_BODY()

    /** Story prop type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    EStorytellingProps PropType = EStorytellingProps::BoneScatters;

    /** Static mesh to use */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    /** Rarity (0 = common, 1 = extremely rare) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float Rarity = 0.5f;

    /** Preferred biomes for this story element */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<EPrehistoricBiome> PreferredBiomes;

    /** Requires proximity to specific features */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    TArray<EWaterSystemType> RequiredWaterProximity;

    /** Minimum distance from other story props */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float MinDistanceFromOthers = 500.0f;

    /** Maximum instances per square kilometer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    int32 MaxInstancesPerSqKm = 5;

    /** Story weight (affects narrative generation) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float StoryWeight = 1.0f;
};

/** Material blending parameters for terrain */
USTRUCT(BlueprintType)
struct FTerrainMaterialBlend
{
    GENERATED_BODY()

    /** Base material for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BaseMaterial;

    /** Detail materials for close-up viewing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> DetailMaterials;

    /** Biome this blend applies to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EPrehistoricBiome TargetBiome = EPrehistoricBiome::TropicalRainforest;

    /** Elevation-based blending */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    FVector2D ElevationBlendRange = FVector2D(0.0f, 1000.0f);

    /** Slope-based blending */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    FVector2D SlopeBlendRange = FVector2D(0.0f, 45.0f);

    /** Moisture-based blending */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    FVector2D MoistureBlendRange = FVector2D(0.0f, 1.0f);

    /** Texture tiling scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
    float TextureScale = 1.0f;

    /** Normal map intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
    float NormalIntensity = 1.0f;

    /** Roughness variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
    FVector2D RoughnessRange = FVector2D(0.3f, 0.8f);
};

/**
 * @brief Main Environment Art System Component
 * 
 * Orchestrates the placement of all environmental art elements:
 * - Vegetation distribution and variation
 * - Geological props and rock formations  
 * - Storytelling elements and narrative props
 * - Terrain material blending and detail
 * 
 * Works in conjunction with the Procedural World Generator to create
 * believable, performance-optimized prehistoric environments.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvironmentArtSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvironmentArtSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Initialize the environment art system */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void InitializeEnvironmentArt();

    /** Generate vegetation for a specific world partition cell */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void GenerateVegetationForCell(const FVector& CellCenter, float CellSize);

    /** Generate geological props for a specific area */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void GenerateGeologicalPropsForArea(const FVector& AreaCenter, float AreaSize, ETectonicFormation FormationType);

    /** Place storytelling props based on biome and narrative requirements */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PlaceStorytellingProps(const FVector& Location, EPrehistoricBiome Biome, float Radius);

    /** Apply terrain materials based on biome data */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ApplyTerrainMaterials(ALandscape* TargetLandscape, const TArray<EPrehistoricBiome>& BiomeMap);

    /** Update environment art based on performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceOptimizations();

    /** Get vegetation density for a specific location */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    float GetVegetationDensityAtLocation(const FVector& Location, ECretaceousVegetation VegetationType) const;

    /** Check if a location is suitable for a specific prop type */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    bool IsLocationSuitableForProp(const FVector& Location, EGeologicalProps PropType) const;

protected:
    /** Vegetation distribution configurations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FVegetationDistribution> VegetationTypes;

    /** Geological prop configurations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geological Props")
    TArray<FGeologicalPropDistribution> GeologicalProps;

    /** Storytelling prop configurations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FStorytellingPropDistribution> StorytellingProps;

    /** Terrain material blending configurations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<FTerrainMaterialBlend> TerrainMaterials;

    /** Performance budget for environment art */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerformanceBudget PerformanceBudget;

    /** Maximum instances per type per cell */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxInstancesPerTypePerCell = 1000;

    /** LOD bias for vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float VegetationLODBias = 1.0f;

    /** Enable dynamic batching */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDynamicBatching = true;

    /** Use GPU-based culling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseGPUCulling = true;

    /** Streaming distance multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingDistanceMultiplier = 1.0f;

private:
    /** Reference to the world generation system */
    UPROPERTY()
    TObjectPtr<class AJurassicWorldPCGSystem> WorldGenerationSystem;

    /** Cached foliage actor for vegetation placement */
    UPROPERTY()
    TObjectPtr<AInstancedFoliageActor> FoliageActor;

    /** Instanced static mesh components for props */
    UPROPERTY()
    TMap<EGeologicalProps, TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> PropComponents;

    /** Dynamic material instances for terrain blending */
    UPROPERTY()
    TArray<TObjectPtr<UMaterialInstanceDynamic>> TerrainMaterialInstances;

    /** Performance monitoring */
    float LastPerformanceUpdate = 0.0f;
    int32 CurrentInstanceCount = 0;
    float CurrentFrameTime = 0.0f;

    // Internal generation methods
    void GenerateVegetationCluster(const FVector& Center, ECretaceousVegetation VegetationType, int32 Count);
    void PlaceGeologicalProp(const FVector& Location, EGeologicalProps PropType, float Scale = 1.0f);
    void CreateStorytellingElement(const FVector& Location, EStorytellingProps PropType);
    bool ValidatePlacementLocation(const FVector& Location, float MinSlope, float MaxSlope) const;
    FVector2D GetBiomeInfluenceAtLocation(const FVector& Location) const;
    void OptimizeLODDistances();
    void UpdateInstanceCulling();
};

/**
 * @brief Environment Art Manager Actor
 * 
 * World-level manager that coordinates environment art generation
 * across multiple world partition cells and biomes.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentArtManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentArtManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    /** Initialize environment art for the entire world */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void InitializeWorldEnvironmentArt();

    /** Regenerate environment art for a specific biome */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void RegenerateEnvironmentForBiome(EPrehistoricBiome Biome);

    /** Get environment art statistics */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetEnvironmentArtStats() const;

protected:
    /** Environment art system component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UEnvironmentArtSystem> EnvironmentArtSystem;

    /** PCG component for procedural generation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UPCGComponent> PCGComponent;

    /** World generation system reference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TObjectPtr<AJurassicWorldPCGSystem> WorldGenerationSystem;

    /** Enable real-time updates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableRealTimeUpdates = false;

    /** Update frequency in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 1.0f;

private:
    float LastUpdateTime = 0.0f;
    TMap<EPrehistoricBiome, int32> BiomeInstanceCounts;
};