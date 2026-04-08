// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "PCGSettings.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Foliage/FoliageType.h"
#include "WorldGeneration/JurassicBiomeManager.h"
#include "EnvironmentArtSystem.generated.h"

class UPCGGraph;
class UPCGComponent;
class ALandscape;
class UStaticMesh;
class UMaterialInterface;
class UFoliageType;
class AJurassicBiomeManager;

/**
 * @brief Environment Art System - Transpersonal Game Studio Agent #6
 * 
 * Transforms generated terrain into living, breathing prehistoric worlds.
 * Every detail tells a story. Every prop has a reason to exist.
 * 
 * NARRATIVE ENVIRONMENTAL DESIGN:
 * - Each cluster of vegetation tells a story about soil, water, and history
 * - Props are placed with archaeological precision - bones near water, tools near shelter
 * - Erosion patterns, fallen logs, and rock formations create believable history
 * - Lighting and composition guide the player's emotional journey
 * 
 * TECHNICAL EXCELLENCE:
 * - Nanite-enabled high-poly vegetation and rocks for film-quality detail
 * - Hierarchical LOD system: Hero assets → Standard detail → Background fill
 * - GPU-driven instancing for massive vegetation density
 * - Procedural variation within artistic control
 * - Performance-conscious: every triangle earns its place
 * 
 * PREHISTORIC AUTHENTICITY:
 * - Cretaceous period flora: Ferns, cycads, conifers, early flowering plants
 * - Geological accuracy: Rock types appropriate for Mesozoic formations
 * - Ecosystem storytelling: Predator paths, herbivore feeding areas, nesting sites
 * - Weathering and erosion patterns that feel millions of years old
 * 
 * DESIGN PHILOSOPHY (Roger Deakins + RDR2 Environmental Team):
 * - Light and composition tell the story before any character speaks
 * - Players should feel they're in a place that existed before they arrived
 * - Every detail serves the illusion of a living, breathing world
 * - Beauty and function are inseparable
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */

/** Vegetation types for Cretaceous period */
UENUM(BlueprintType)
enum class ECretaceousVegetationType : uint8
{
    // Ferns and primitive plants
    TreeFern            UMETA(DisplayName = "Tree Fern"),
    RoyalFern           UMETA(DisplayName = "Royal Fern"),
    BrackenFern         UMETA(DisplayName = "Bracken Fern"),
    Horsetail           UMETA(DisplayName = "Horsetail"),
    ClubMoss            UMETA(DisplayName = "Club Moss"),
    
    // Gymnosperms (dominant in Cretaceous)
    Araucaria           UMETA(DisplayName = "Araucaria (Monkey Puzzle)"),
    Sequoia             UMETA(DisplayName = "Sequoia"),
    Ginkgo              UMETA(DisplayName = "Ginkgo"),
    Cycad               UMETA(DisplayName = "Cycad"),
    Bennettites         UMETA(DisplayName = "Bennettites"),
    
    // Early angiosperms (flowering plants emerging)
    Magnolia            UMETA(DisplayName = "Magnolia"),
    Laurel              UMETA(DisplayName = "Laurel"),
    Sycamore            UMETA(DisplayName = "Sycamore"),
    WillowLike          UMETA(DisplayName = "Willow-like"),
    
    // Aquatic and wetland
    WaterLily           UMETA(DisplayName = "Water Lily"),
    Cattail             UMETA(DisplayName = "Cattail"),
    Moss                UMETA(DisplayName = "Moss"),
    Algae               UMETA(DisplayName = "Algae"),
    
    // Ground cover
    FernUndergrowth     UMETA(DisplayName = "Fern Undergrowth"),
    MossGround          UMETA(DisplayName = "Moss Ground Cover"),
    LichenRock          UMETA(DisplayName = "Lichen on Rock"),
    DeadVegetation      UMETA(DisplayName = "Dead Vegetation")
};

/** Rock and geological prop types */
UENUM(BlueprintType)
enum class EGeologicalPropType : uint8
{
    // Sedimentary rocks (common in Cretaceous)
    Limestone           UMETA(DisplayName = "Limestone"),
    Sandstone           UMETA(DisplayName = "Sandstone"),
    Shale               UMETA(DisplayName = "Shale"),
    Mudstone            UMETA(DisplayName = "Mudstone"),
    
    // Igneous rocks
    Basalt              UMETA(DisplayName = "Basalt"),
    Granite             UMETA(DisplayName = "Granite"),
    VolcanicRock        UMETA(DisplayName = "Volcanic Rock"),
    
    // Formations
    Boulder             UMETA(DisplayName = "Boulder"),
    RockOutcrop         UMETA(DisplayName = "Rock Outcrop"),
    CliffFace           UMETA(DisplayName = "Cliff Face"),
    Pebbles             UMETA(DisplayName = "Pebbles"),
    
    // Erosion features
    ErodedRock          UMETA(DisplayName = "Eroded Rock"),
    RockArch            UMETA(DisplayName = "Rock Arch"),
    Hoodoo              UMETA(DisplayName = "Hoodoo Formation"),
    
    // Special formations
    FossilBearing       UMETA(DisplayName = "Fossil-Bearing Rock"),
    CrystalFormation    UMETA(DisplayName = "Crystal Formation"),
    IronOxide           UMETA(DisplayName = "Iron Oxide Staining")
};

/** Environmental storytelling props */
UENUM(BlueprintType)
enum class EStorytellingPropType : uint8
{
    // Dinosaur traces
    DinosaurBones       UMETA(DisplayName = "Dinosaur Bones"),
    DinosaurSkull       UMETA(DisplayName = "Dinosaur Skull"),
    Footprints          UMETA(DisplayName = "Fossilized Footprints"),
    EggShells           UMETA(DisplayName = "Egg Shells"),
    Coprolites          UMETA(DisplayName = "Coprolites"),
    
    // Natural debris
    FallenLog           UMETA(DisplayName = "Fallen Log"),
    DeadTree            UMETA(DisplayName = "Dead Tree"),
    BrokenBranch        UMETA(DisplayName = "Broken Branch"),
    
    // Water features
    Driftwood           UMETA(DisplayName = "Driftwood"),
    RiverStones         UMETA(DisplayName = "River Stones"),
    MudCracks           UMETA(DisplayName = "Mud Cracks"),
    
    // Geological storytelling
    Landslide           UMETA(DisplayName = "Landslide Debris"),
    RockFall            UMETA(DisplayName = "Rock Fall"),
    ErosionChannel      UMETA(DisplayName = "Erosion Channel"),
    
    // Atmospheric elements
    MistVents           UMETA(DisplayName = "Mist Vents"),
    HotSprings          UMETA(DisplayName = "Hot Springs"),
    GeyserRocks         UMETA(DisplayName = "Geyser Rocks")
};

/** Vegetation density levels for performance management */
UENUM(BlueprintType)
enum class EVegetationDensity : uint8
{
    Sparse              UMETA(DisplayName = "Sparse (Background)"),
    Medium              UMETA(DisplayName = "Medium (Standard)"),
    Dense               UMETA(DisplayName = "Dense (Hero Areas)"),
    UltraDense          UMETA(DisplayName = "Ultra Dense (Showcase)")
};

/** Artistic detail levels */
UENUM(BlueprintType)
enum class EDetailLevel : uint8
{
    Background          UMETA(DisplayName = "Background (Distant)"),
    Standard            UMETA(DisplayName = "Standard (Mid-range)"),
    Hero                UMETA(DisplayName = "Hero (Close-up)"),
    Cinematic           UMETA(DisplayName = "Cinematic (Showcase)")
};

/** Vegetation asset configuration */
USTRUCT(BlueprintType)
struct FVegetationAssetData
{
    GENERATED_BODY()

    /** Vegetation type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    ECretaceousVegetationType VegetationType = ECretaceousVegetationType::TreeFern;

    /** Static mesh for this vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    /** Material variations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;

    /** Size variation range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FVector2D SizeRange = FVector2D(0.8f, 1.2f);

    /** Rotation randomization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    bool bRandomRotation = true;

    /** Slope tolerance in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FVector2D SlopeRange = FVector2D(0.0f, 45.0f);

    /** Preferred biomes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EJurassicBiomeType> PreferredBiomes;

    /** Density per square meter */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
    float DensityPerSquareMeter = 0.1f;

    /** Clustering factor (0 = random, 1 = highly clustered) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float ClusteringFactor = 0.3f;

    /** Distance from water influence */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FVector2D WaterDistanceRange = FVector2D(0.0f, 1000.0f);

    /** Elevation preference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FVector2D ElevationRange = FVector2D(0.0f, 1000.0f);

    /** LOD distances */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FVector4 LODDistances = FVector4(500.0f, 1500.0f, 5000.0f, 15000.0f);

    /** Wind response strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float WindResponseStrength = 1.0f;

    /** Seasonal variation (future feature) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasons")
    bool bSeasonalVariation = false;
};

/** Rock and prop asset configuration */
USTRUCT(BlueprintType)
struct FGeologicalPropData
{
    GENERATED_BODY()

    /** Prop type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    EGeologicalPropType PropType = EGeologicalPropType::Limestone;

    /** Static mesh */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    /** Material variations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;

    /** Size variation range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FVector2D SizeRange = FVector2D(0.5f, 2.0f);

    /** Preferred geological formations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    TArray<ETectonicFormation> PreferredFormations;

    /** Slope requirements */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FVector2D SlopeRange = FVector2D(0.0f, 90.0f);

    /** Density per square kilometer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
    float DensityPerSquareKm = 50.0f;

    /** Clustering factor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float ClusteringFactor = 0.7f;

    /** Weathering intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float WeatheringIntensity = 0.5f;

    /** Erosion resistance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float ErosionResistance = 0.8f;
};

/** Storytelling prop configuration */
USTRUCT(BlueprintType)
struct FStorytellingPropData
{
    GENERATED_BODY()

    /** Prop type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    EStorytellingPropType PropType = EStorytellingPropType::DinosaurBones;

    /** Static mesh */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    /** Material variations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;

    /** Narrative context */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NarrativeContext;

    /** Preferred placement contexts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<FString> PlacementContexts;

    /** Rarity (0 = common, 1 = extremely rare) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rarity")
    float Rarity = 0.5f;

    /** Discovery reward value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    int32 DiscoveryValue = 10;

    /** Associated audio cues */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<TSoftObjectPtr<USoundBase>> AudioCues;
};

/** Environment art generation parameters */
USTRUCT(BlueprintType)
struct FEnvironmentArtGenerationData
{
    GENERATED_BODY()

    /** Overall vegetation density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float VegetationDensityMultiplier = 1.0f;

    /** Rock and prop density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float PropDensityMultiplier = 1.0f;

    /** Storytelling prop frequency */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float StorytellingPropFrequency = 0.3f;

    /** Artistic detail level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    EDetailLevel DetailLevel = EDetailLevel::Standard;

    /** Performance target */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EVegetationDensity PerformanceTarget = EVegetationDensity::Medium;

    /** Seasonal time (0-1, for future seasonal variation) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasons", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SeasonalTime = 0.5f;

    /** Weather influence on placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherInfluence = 0.7f;

    /** Erosion pattern strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float ErosionPatternStrength = 1.0f;

    /** Biodiversity factor (species variation) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float BiodiversityFactor = 1.0f;
};

/**
 * Main Environment Art System Actor
 * Manages all environmental art placement and generation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentArtSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentArtSystem();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPCGComponent* VegetationPCGComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPCGComponent* PropsPCGComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPCGComponent* StorytellingPCGComponent;

    // Asset libraries
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Libraries")
    TArray<FVegetationAssetData> VegetationLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Libraries")
    TArray<FGeologicalPropData> GeologicalPropLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Libraries")
    TArray<FStorytellingPropData> StorytellingPropLibrary;

    // Generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    FEnvironmentArtGenerationData GenerationParameters;

    // World references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World References")
    TSoftObjectPtr<AJurassicBiomeManager> BiomeManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World References")
    TSoftObjectPtr<ALandscape> TargetLandscape;

    // PCG Graphs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<UPCGGraph> VegetationPCGGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<UPCGGraph> PropsPCGGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<UPCGGraph> StorytellingPCGGraph;

    // Performance monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 TotalInstancesGenerated = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastGenerationTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MemoryUsageMB = 0;

public:
    // Main generation functions
    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateEnvironmentArt();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateVegetation();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateGeologicalProps();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateStorytellingProps();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void ClearAllGeneration();

    // Biome-specific generation
    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateForBiome(EJurassicBiomeType BiomeType, FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void RefreshBiomeArea(EJurassicBiomeType BiomeType, FVector Center, float Radius);

    // Asset management
    UFUNCTION(BlueprintCallable, Category = "Assets")
    void LoadAssetLibraries();

    UFUNCTION(BlueprintCallable, Category = "Assets")
    void UnloadAssetLibraries();

    UFUNCTION(BlueprintCallable, Category = "Assets")
    void RefreshAssetLibraries();

    // Query functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
    TArray<FVegetationAssetData> GetVegetationForBiome(EJurassicBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
    TArray<FGeologicalPropData> GetPropsForFormation(ETectonicFormation Formation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
    FVegetationAssetData GetRandomVegetationForContext(EJurassicBiomeType Biome, float Moisture, float Elevation) const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    bool IsWithinPerformanceBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetDetailLevel(EDetailLevel NewDetailLevel);

    // Narrative functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlaceStorytellingProps(const TArray<FVector>& Locations, const TArray<FString>& NarrativeContexts);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CreateNarrativeCluster(FVector Center, float Radius, const FString& StoryContext);

private:
    // Internal generation helpers
    void GenerateVegetationForBiome(EJurassicBiomeType BiomeType, const FBox& Bounds);
    void GeneratePropsForFormation(ETectonicFormation Formation, const FBox& Bounds);
    void PlaceStorytellingElement(const FStorytellingPropData& PropData, FVector Location, FRotator Rotation);
    
    // Asset selection helpers
    FVegetationAssetData SelectVegetationAsset(EJurassicBiomeType Biome, float Moisture, float Temperature, float Elevation) const;
    FGeologicalPropData SelectGeologicalProp(ETectonicFormation Formation, float Slope, float Elevation) const;
    FStorytellingPropData SelectStorytellingProp(const FString& Context, float Rarity) const;
    
    // Performance helpers
    void UpdatePerformanceMetrics();
    bool ShouldPlaceInstanceAtLOD(float Distance, EDetailLevel DetailLevel) const;
    void CullDistantInstances();
    
    // PCG integration
    void SetupPCGGeneration();
    void ConfigurePCGParameters();
    void ExecutePCGGeneration();
    
    // Biome integration
    AJurassicBiomeManager* GetBiomeManager() const;
    bool IsValidPlacementLocation(FVector Location, const FVegetationAssetData& VegData) const;
    bool IsValidPlacementLocation(FVector Location, const FGeologicalPropData& PropData) const;
    
    // Artistic helpers
    FRotator CalculateArtisticRotation(FVector Location, FVector Normal, bool bRandomize) const;
    FVector CalculateArtisticScale(const FVector2D& SizeRange, FVector Location) const;
    float CalculateArtisticDensity(FVector Location, EJurassicBiomeType Biome) const;
    
    // Memory management
    void ManageMemoryUsage();
    void UnloadDistantAssets();
    void PreloadNearbyAssets(FVector PlayerLocation);
};