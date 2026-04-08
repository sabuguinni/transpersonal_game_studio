#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataAsset.h"
#include "Components/ActorComponent.h"
#include "WorldGeneration/ProceduralWorldManager.h"
#include "EnvironmentArtManager.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UPCGComponent;
class UPCGGraph;
class UInstancedStaticMeshComponent;
class UFoliageType;

/**
 * Vegetation categories for prehistoric environments
 * Based on Jurassic/Cretaceous flora
 */
UENUM(BlueprintType)
enum class EVegetationType : uint8
{
    None                    UMETA(DisplayName = "None"),
    // Trees
    Conifers                UMETA(DisplayName = "Conifers"),           // Pines, Firs, Araucarias
    Cycads                  UMETA(DisplayName = "Cycads"),             // Palm-like plants
    Ginkgos                 UMETA(DisplayName = "Ginkgos"),            // Ancient deciduous
    TreeFerns               UMETA(DisplayName = "Tree Ferns"),         // Large ferns
    // Understory
    Ferns                   UMETA(DisplayName = "Ferns"),              // Ground ferns
    Horsetails              UMETA(DisplayName = "Horsetails"),         // Equisetum
    Mosses                  UMETA(DisplayName = "Mosses"),             // Ground cover
    // Aquatic
    AquaticFerns            UMETA(DisplayName = "Aquatic Ferns"),      // Water plants
    Algae                   UMETA(DisplayName = "Algae"),              // Pond scum
    // Flowering (late Cretaceous only)
    EarlyFlowering          UMETA(DisplayName = "Early Flowering"),    // Primitive angiosperms
};

/**
 * Geological feature types for environmental storytelling
 */
UENUM(BlueprintType)
enum class EGeologyType : uint8
{
    None                    UMETA(DisplayName = "None"),
    // Rock formations
    Sandstone               UMETA(DisplayName = "Sandstone"),
    Limestone               UMETA(DisplayName = "Limestone"),
    Shale                   UMETA(DisplayName = "Shale"),
    VolcanicRock            UMETA(DisplayName = "Volcanic Rock"),
    // Features
    Boulders                UMETA(DisplayName = "Boulders"),
    RockOutcrops            UMETA(DisplayName = "Rock Outcrops"),
    Cliffs                  UMETA(DisplayName = "Cliffs"),
    Caves                   UMETA(DisplayName = "Caves"),
    // Fossil sites
    FossilBeds              UMETA(DisplayName = "Fossil Beds"),
    BoneYards               UMETA(DisplayName = "Bone Yards"),
};

/**
 * Environmental props that tell stories without words
 */
UENUM(BlueprintType)
enum class EEnvironmentalProp : uint8
{
    None                    UMETA(DisplayName = "None"),
    // Natural debris
    FallenLogs              UMETA(DisplayName = "Fallen Logs"),
    DeadTrees               UMETA(DisplayName = "Dead Trees"),
    BrokenBranches          UMETA(DisplayName = "Broken Branches"),
    // Animal traces
    DinosaurFootprints      UMETA(DisplayName = "Dinosaur Footprints"),
    NestSites               UMETA(DisplayName = "Nest Sites"),
    BoneScatters            UMETA(DisplayName = "Bone Scatters"),
    DroppingsDeposits       UMETA(DisplayName = "Droppings Deposits"),
    // Environmental storytelling
    ClawMarks               UMETA(DisplayName = "Claw Marks"),
    TeethMarks              UMETA(DisplayName = "Teeth Marks"),
    TramplePaths            UMETA(DisplayName = "Trample Paths"),
    WallowSites             UMETA(DisplayName = "Wallow Sites"),
    // Water features
    MudPools                UMETA(DisplayName = "Mud Pools"),
    SaltLicks               UMETA(DisplayName = "Salt Licks"),
    Springs                 UMETA(DisplayName = "Springs"),
};

/**
 * Atmospheric condition affecting visual appearance
 */
UENUM(BlueprintType)
enum class EAtmosphericCondition : uint8
{
    Clear                   UMETA(DisplayName = "Clear"),
    Misty                   UMETA(DisplayName = "Misty"),
    Humid                   UMETA(DisplayName = "Humid"),
    Dusty                   UMETA(DisplayName = "Dusty"),
    VolcanicAsh             UMETA(DisplayName = "Volcanic Ash"),
    Pollen                  UMETA(DisplayName = "Pollen"),
};

/**
 * Vegetation asset definition with growth parameters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVegetationAsset
{
    GENERATED_BODY()

    /** Static mesh for this vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
    TSoftObjectPtr<UStaticMesh> Mesh;

    /** Foliage type for painting */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
    TSoftObjectPtr<UFoliageType> FoliageType;

    /** Vegetation category */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
    EVegetationType VegetationType = EVegetationType::None;

    /** Density per square meter */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float Density = 1.0f;

    /** Scale variation range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    /** Rotation randomization */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    bool bRandomRotation = true;

    /** Slope tolerance in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Growth Conditions")
    FVector2D SlopeTolerance = FVector2D(0.0f, 30.0f);

    /** Altitude preference in meters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Growth Conditions")
    FVector2D AltitudeRange = FVector2D(0.0f, 1000.0f);

    /** Moisture requirement (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Growth Conditions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MoistureRequirement = 0.5f;

    /** Distance from water preference */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Growth Conditions")
    FVector2D WaterDistanceRange = FVector2D(0.0f, 10000.0f);

    /** Clustering tendency (0 = scattered, 1 = heavily clustered) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ClusteringFactor = 0.3f;

    /** Minimum distance between instances */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    float MinSpacing = 100.0f;

    /** LOD distances for performance */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    TArray<float> LODDistances = {1000.0f, 2000.0f, 4000.0f};

    /** Cull distance */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float CullDistance = 8000.0f;
};

/**
 * Geological feature asset definition
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FGeologyAsset
{
    GENERATED_BODY()

    /** Static mesh for this geological feature */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
    TSoftObjectPtr<UStaticMesh> Mesh;

    /** Geology type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
    EGeologyType GeologyType = EGeologyType::None;

    /** Scale variation range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.7f, 1.5f);

    /** Placement density per square kilometer */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Density = 5.0f;

    /** Slope preference for placement */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D PreferredSlope = FVector2D(15.0f, 60.0f);

    /** Altitude preference */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D AltitudeRange = FVector2D(0.0f, 2000.0f);

    /** Can be placed underwater */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    bool bCanPlaceUnderwater = false;

    /** Affects vegetation growth around it */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environmental Impact")
    float VegetationInfluenceRadius = 500.0f;

    /** Vegetation density multiplier in influence area */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environmental Impact")
    float VegetationDensityMultiplier = 0.5f;
};

/**
 * Environmental prop asset definition
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvironmentalPropAsset
{
    GENERATED_BODY()

    /** Static mesh for this prop */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
    TSoftObjectPtr<UStaticMesh> Mesh;

    /** Prop type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
    EEnvironmentalProp PropType = EEnvironmentalProp::None;

    /** Storytelling weight (higher = more narrative importance) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StorytellingWeight = 0.5f;

    /** Placement frequency per square kilometer */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float PlacementFrequency = 2.0f;

    /** Scale variation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    /** Requires specific biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    TArray<EBiomeType> RequiredBiomes;

    /** Requires proximity to water */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    bool bRequiresWaterProximity = false;

    /** Maximum distance from water if required */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements", meta = (EditCondition = "bRequiresWaterProximity"))
    float MaxWaterDistance = 1000.0f;

    /** Attracts specific dinosaur types */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Impact")
    TArray<FString> AttractedDinosaurTypes;

    /** Provides player resources */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Impact")
    bool bProvidesResources = false;
};

/**
 * Biome art configuration defining visual characteristics
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBiomeArtConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Biome this configuration applies to */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EBiomeType TargetBiome = EBiomeType::None;

    /** Dominant vegetation assets */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FVegetationAsset> DominantVegetation;

    /** Understory vegetation assets */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FVegetationAsset> UnderstoryVegetation;

    /** Ground cover vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FVegetationAsset> GroundCover;

    /** Geological features for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geology")
    TArray<FGeologyAsset> GeologicalFeatures;

    /** Environmental props for storytelling */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environmental Storytelling")
    TArray<FEnvironmentalPropAsset> EnvironmentalProps;

    /** Landscape material for terrain */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> LandscapeMaterial;

    /** Material layers for blending */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TMap<FString, TSoftObjectPtr<UMaterialInterface>> MaterialLayers;

    /** Atmospheric conditions typical for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    TArray<EAtmosphericCondition> TypicalConditions;

    /** Color grading LUT for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Post Processing")
    TSoftObjectPtr<class UTexture> ColorGradingLUT;

    /** Ambient sound for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<class USoundBase> AmbientSound;

    /** PCG Graph for vegetation distribution */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PCG")
    TSoftObjectPtr<UPCGGraph> VegetationDistributionGraph;

    /** PCG Graph for prop placement */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PCG")
    TSoftObjectPtr<UPCGGraph> PropPlacementGraph;
};

/**
 * Main manager for environment art systems
 * Transforms procedural terrain into visually rich prehistoric environments
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API AEnvironmentArtManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentArtManager();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    /** Reference to the world generation manager */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dependencies")
    TSoftObjectPtr<AProceduralWorldManager> WorldManager;

    /** Biome art configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<TSoftObjectPtr<UBiomeArtConfig>> BiomeArtConfigs;

    /** Main PCG component for environment art */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TObjectPtr<UPCGComponent> EnvironmentPCGComponent;

    /** Performance settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MaxDrawDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxInstancesPerComponent = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bUseNaniteForVegetation = true;

public:
    /** Populate all biomes with vegetation and props */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PopulateEnvironment();

    /** Populate specific biome */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PopulateBiome(EBiomeType BiomeType);

    /** Clear all environment art */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ClearEnvironmentArt();

    /** Apply materials to landscape based on biomes */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ApplyLandscapeMaterials();

    /** Place storytelling props in specific area */
    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void PlaceStorytellingProps(const FVector& CenterLocation, float Radius, int32 MaxProps = 5);

    /** Create a dinosaur path with environmental clues */
    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void CreateDinosaurPath(const TArray<FVector>& PathPoints, float PathWidth = 500.0f);

    /** Create a feeding area with appropriate props */
    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void CreateFeedingArea(const FVector& Location, float Radius = 1000.0f);

    /** Create a nesting site */
    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void CreateNestingSite(const FVector& Location, int32 NestCount = 3);

    /** Get vegetation density at location */
    UFUNCTION(BlueprintCallable, Category = "Environment Query")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    /** Check if location is suitable for specific vegetation type */
    UFUNCTION(BlueprintCallable, Category = "Environment Query")
    bool IsLocationSuitableForVegetation(const FVector& WorldLocation, EVegetationType VegetationType) const;

    /** Find clearings suitable for player camps */
    UFUNCTION(BlueprintCallable, Category = "Environment Query")
    TArray<FVector> FindClearings(float MinRadius = 500.0f, int32 MaxResults = 10) const;

    /** Get biome art configuration for biome type */
    UFUNCTION(BlueprintCallable, Category = "Environment Query")
    UBiomeArtConfig* GetBiomeArtConfig(EBiomeType BiomeType) const;

private:
    /** Generate vegetation for a specific area */
    void GenerateVegetationInArea(const FVector& CenterLocation, float Radius, EBiomeType BiomeType);

    /** Place geological features */
    void PlaceGeologicalFeatures(EBiomeType BiomeType);

    /** Create environmental narrative clusters */
    void CreateNarrativeClusters();

    /** Calculate vegetation placement based on environmental factors */
    bool CalculateVegetationPlacement(const FVector& Location, const FVegetationAsset& VegetationAsset) const;

    /** Apply weathering and aging effects to props */
    void ApplyWeatheringEffects(UStaticMeshComponent* Component, float WeatheringIntensity) const;

    /** Cached biome art configurations */
    UPROPERTY()
    TMap<EBiomeType, TObjectPtr<UBiomeArtConfig>> CachedArtConfigs;

    /** Instanced static mesh components for performance */
    UPROPERTY()
    TMap<UStaticMesh*, TObjectPtr<UInstancedStaticMeshComponent>> InstancedMeshComponents;
};