// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Foliage/Public/FoliageType.h"
#include "Foliage/Public/FoliageInstancedStaticMeshComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "../WorldGeneration/BiomeSystem.h"
#include "EnvironmentArtSystem.generated.h"

/**
 * @brief Environment Art System for Transpersonal Game Studio
 * 
 * Transforms the generated world into a living, breathing prehistoric environment.
 * Populates biomes with vegetation, rocks, props and environmental details that tell
 * the story of a world that existed before the player arrived.
 * 
 * Core Philosophy:
 * - Every detail has a narrative purpose
 * - The environment tells stories without words
 * - Players should feel they're in a place with history
 * - Composition and lighting guide the player's emotional journey
 * 
 * Features:
 * - Biome-specific vegetation placement using PCG and Foliage tools
 * - Procedural rock and geological feature distribution
 * - Environmental storytelling props (bones, abandoned nests, etc.)
 * - Dynamic material blending for terrain surfaces
 * - Atmospheric detail placement (fallen logs, scattered stones)
 * - Prehistoric flora with scientifically-inspired designs
 * 
 * Technical Implementation:
 * - Uses UE5 Foliage Mode for vegetation painting
 * - PCG Framework for procedural prop placement
 * - Hierarchical Instanced Static Meshes for performance
 * - Nanite-enabled high-detail vegetation
 * - Landscape material layering system
 * - LOD chains for distant vegetation
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */

/** Vegetation categories for the prehistoric world */
UENUM(BlueprintType)
enum class EPrehistoricVegetationType : uint8
{
    // Trees and Large Plants
    ConiferTrees        UMETA(DisplayName = "Conifer Trees"),
    CycadPalms          UMETA(DisplayName = "Cycad Palms"),
    GinkgoTrees         UMETA(DisplayName = "Ginkgo Trees"),
    FernTrees           UMETA(DisplayName = "Tree Ferns"),
    
    // Undergrowth
    LargeFerns          UMETA(DisplayName = "Large Ferns"),
    Horsetails          UMETA(DisplayName = "Horsetails"),
    Mosses              UMETA(DisplayName = "Mosses"),
    Liverworts          UMETA(DisplayName = "Liverworts"),
    
    // Ground Cover
    SmallFerns          UMETA(DisplayName = "Small Ferns"),
    PrimitiveFerns      UMETA(DisplayName = "Primitive Ferns"),
    Lichens             UMETA(DisplayName = "Lichens"),
    AlgaeMats           UMETA(DisplayName = "Algae Mats"),
    
    // Aquatic Plants
    AquaticFerns        UMETA(DisplayName = "Aquatic Ferns"),
    WaterLilies         UMETA(DisplayName = "Prehistoric Water Lilies"),
    Reeds               UMETA(DisplayName = "Primitive Reeds"),
    
    // Special/Rare
    CarnivorousPlants   UMETA(DisplayName = "Carnivorous Plants"),
    BioluminescentFungi UMETA(DisplayName = "Bioluminescent Fungi"),
    CrystalFormations   UMETA(DisplayName = "Crystal Formations")
};

/** Environmental prop categories */
UENUM(BlueprintType)
enum class EEnvironmentalPropType : uint8
{
    // Geological Features
    Boulders            UMETA(DisplayName = "Boulders"),
    RockOutcrops        UMETA(DisplayName = "Rock Outcrops"),
    FallenLogs          UMETA(DisplayName = "Fallen Logs"),
    DeadTrees           UMETA(DisplayName = "Dead Trees"),
    
    // Storytelling Elements
    DinosaurBones       UMETA(DisplayName = "Dinosaur Bones"),
    FossilizedRemains   UMETA(DisplayName = "Fossilized Remains"),
    AbandonedNests      UMETA(DisplayName = "Abandoned Nests"),
    ClawMarks           UMETA(DisplayName = "Claw Marks on Trees"),
    
    // Natural Debris
    ScatteredStones     UMETA(DisplayName = "Scattered Stones"),
    BrokenBranches      UMETA(DisplayName = "Broken Branches"),
    LeafLitter          UMETA(DisplayName = "Leaf Litter"),
    MudPatches          UMETA(DisplayName = "Mud Patches"),
    
    // Water Features
    RiverRocks          UMETA(DisplayName = "River Rocks"),
    Driftwood           UMETA(DisplayName = "Driftwood"),
    WaterPlants         UMETA(DisplayName = "Aquatic Vegetation"),
    
    // Atmospheric Details
    Mist                UMETA(DisplayName = "Ground Mist"),
    LightShafts         UMETA(DisplayName = "Light Shaft Markers"),
    SoundMarkers        UMETA(DisplayName = "Ambient Sound Markers")
};

/** Vegetation placement rules */
USTRUCT(BlueprintType)
struct FVegetationPlacementRules
{
    GENERATED_BODY()

    /** Minimum distance from water sources (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float MinDistanceFromWater = 100.0f;

    /** Maximum distance from water sources (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float MaxDistanceFromWater = 2000.0f;

    /** Preferred slope range (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D PreferredSlopeRange = FVector2D(0.0f, 15.0f);

    /** Elevation range preference (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D ElevationRange = FVector2D(-500.0f, 1500.0f);

    /** Clustering tendency (0 = scattered, 1 = heavily clustered) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ClusteringTendency = 0.3f;

    /** Cluster size range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D ClusterSizeRange = FVector2D(3.0f, 12.0f);

    /** Avoids other vegetation types */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<EPrehistoricVegetationType> AvoidVegetationTypes;

    /** Prefers to grow near these vegetation types */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<EPrehistoricVegetationType> PreferVegetationTypes;

    /** Can grow on these biome types */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<EBiomeType> SuitableBiomes;
};

/** Vegetation asset with placement and rendering data */
USTRUCT(BlueprintType)
struct FPrehistoricVegetationAsset
{
    GENERATED_BODY()

    /** Vegetation type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    EPrehistoricVegetationType VegetationType = EPrehistoricVegetationType::LargeFerns;

    /** Display name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    FString DisplayName = "Large Fern";

    /** Static mesh asset */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> VegetationMesh;

    /** Alternative mesh variations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> MeshVariations;

    /** Material overrides for seasonal/biome variations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TMap<EBiomeType, TSoftObjectPtr<UMaterialInterface>> BiomeMaterials;

    /** Foliage type for UE5 foliage system */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TSoftObjectPtr<UFoliageType> FoliageType;

    /** Placement rules */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVegetationPlacementRules PlacementRules;

    /** Scale variation range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    /** Rotation randomization */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    bool bRandomizeRotation = true;

    /** Tilt randomization range (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation", meta = (ClampMin = "0.0", ClampMax = "45.0"))
    float MaxTiltAngle = 5.0f;

    /** Density per square meter */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Density", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float DensityPerSquareMeter = 0.5f;

    /** LOD distances */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    TArray<float> LODDistances = {500.0f, 1500.0f, 3000.0f};

    /** Cull distance */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "1000.0", ClampMax = "10000.0"))
    float CullDistance = 5000.0f;

    /** Can be destroyed by dinosaurs */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bCanBeDestroyed = true;

    /** Provides food for herbivores */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bProvidesFood = false;

    /** Provides hiding spots for player */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bProvidesConcealment = false;

    /** Sound effects when interacted with */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> InteractionSound;

    /** Particle effects when destroyed */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TSoftObjectPtr<UParticleSystem> DestructionEffect;
};

/** Environmental prop asset data */
USTRUCT(BlueprintType)
struct FEnvironmentalPropAsset
{
    GENERATED_BODY()

    /** Prop type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    EEnvironmentalPropType PropType = EEnvironmentalPropType::Boulders;

    /** Display name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    FString DisplayName = "Boulder";

    /** Static mesh asset */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    TSoftObjectPtr<UStaticMesh> PropMesh;

    /** Mesh variations for diversity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    TArray<TSoftObjectPtr<UStaticMesh>> MeshVariations;

    /** Material variations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;

    /** Spawn probability in suitable areas */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnProbability = 0.1f;

    /** Suitable biomes for this prop */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<EBiomeType> SuitableBiomes;

    /** Preferred terrain slope range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D PreferredSlopeRange = FVector2D(0.0f, 30.0f);

    /** Scale variation range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    FVector2D ScaleRange = FVector2D(0.7f, 1.3f);

    /** Clustering behavior */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    bool bCanCluster = true;

    /** Cluster size if clustering enabled */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "2", ClampMax = "20"))
    int32 ClusterSize = 5;

    /** Minimum distance between instances */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "50.0", ClampMax = "2000.0"))
    float MinSpacing = 200.0f;

    /** Collision settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    bool bHasCollision = true;

    /** Can be used as cover by player */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bProvidesPlayerCover = false;

    /** Can be climbed by player */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bCanBeClimbed = false;

    /** Narrative significance (for storytelling props) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (MultiLine = true))
    FString NarrativeDescription;
};

/** Biome art configuration */
USTRUCT(BlueprintType)
struct FBiomeArtConfiguration
{
    GENERATED_BODY()

    /** Biome this configuration applies to */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::DenseJungle;

    /** Primary vegetation assets for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FPrehistoricVegetationAsset> PrimaryVegetation;

    /** Secondary/undergrowth vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FPrehistoricVegetationAsset> SecondaryVegetation;

    /** Ground cover vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FPrehistoricVegetationAsset> GroundCover;

    /** Environmental props for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<FEnvironmentalPropAsset> EnvironmentalProps;

    /** Landscape material layers */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> PrimaryLandscapeMaterial;

    /** Material blend masks for terrain variation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<UTexture2D>> MaterialBlendMasks;

    /** Overall vegetation density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Density", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float VegetationDensityMultiplier = 1.0f;

    /** Prop density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Density", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float PropDensityMultiplier = 1.0f;

    /** Color grading for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    FLinearColor BiomeColorTint = FLinearColor::White;

    /** Fog density modifier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float FogDensityMultiplier = 1.0f;

    /** Ambient sound assets */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TArray<TSoftObjectPtr<USoundBase>> AmbientSounds;

    /** PCG Graph for procedural placement */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Procedural")
    TSoftObjectPtr<UPCGGraph> BiomeArtPCGGraph;
};

/**
 * @brief Environment Art Data Asset
 * 
 * Contains all art assets and configuration for populating the procedurally
 * generated world with vegetation, props, and environmental details.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnvironmentArtData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UEnvironmentArtData();

    /** Art configurations for each biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Art")
    TArray<FBiomeArtConfiguration> BiomeConfigurations;

    /** Global vegetation assets library */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Library")
    TArray<FPrehistoricVegetationAsset> VegetationLibrary;

    /** Global environmental props library */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Library")
    TArray<FEnvironmentalPropAsset> PropsLibrary;

    /** Master PCG Graph for environment art placement */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Procedural Generation")
    TSoftObjectPtr<UPCGGraph> MasterEnvironmentPCGGraph;

    /** Performance settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxVegetationInstancesPerTile = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxPropInstancesPerTile = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float GlobalCullDistance = 8000.0f;

    /** Quality settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quality")
    bool bUseNaniteVegetation = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quality")
    bool bUseLODChains = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quality")
    int32 MaxLODLevels = 4;

    /** Get biome configuration by type */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    const FBiomeArtConfiguration* GetBiomeConfiguration(EBiomeType BiomeType) const;

    /** Get vegetation assets for biome */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    TArray<FPrehistoricVegetationAsset> GetVegetationForBiome(EBiomeType BiomeType) const;

    /** Get environmental props for biome */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    TArray<FEnvironmentalPropAsset> GetPropsForBiome(EBiomeType BiomeType) const;
};

/**
 * @brief Environment Art System
 * 
 * Main system responsible for populating the world with vegetation and environmental art.
 * Works in conjunction with the Procedural World Generator to create a living ecosystem.
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEnvironmentArtSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEnvironmentArtSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Initialize the environment art system with data */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void InitializeEnvironmentArt(UEnvironmentArtData* InArtData);

    /** Populate a biome area with vegetation and props */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PopulateBiomeArea(EBiomeType BiomeType, const FVector& Center, float Radius);

    /** Clear vegetation and props in an area */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ClearArea(const FVector& Center, float Radius);

    /** Regenerate vegetation in an area (for dynamic destruction) */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void RegenerateVegetationInArea(const FVector& Center, float Radius, float RegenerationRate = 0.1f);

    /** Get vegetation density at location */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    float GetVegetationDensityAtLocation(const FVector& Location) const;

    /** Check if location provides player concealment */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    bool DoesLocationProvideConcealment(const FVector& Location, float CheckRadius = 200.0f) const;

    /** Get nearest climbable objects */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    TArray<AActor*> GetClimbableObjectsNearLocation(const FVector& Location, float SearchRadius = 500.0f) const;

protected:
    /** Environment art data asset */
    UPROPERTY(BlueprintReadOnly, Category = "Environment Art")
    TObjectPtr<UEnvironmentArtData> EnvironmentArtData;

    /** Foliage components for different vegetation types */
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TMap<EPrehistoricVegetationType, TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> VegetationComponents;

    /** Prop components for environmental objects */
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TMap<EEnvironmentalPropType, TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> PropComponents;

    /** PCG components for procedural generation */
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TArray<TObjectPtr<UPCGComponent>> PCGComponents;

    /** Active biome areas being managed */
    UPROPERTY(BlueprintReadOnly, Category = "Management")
    TMap<FVector, EBiomeType> ActiveBiomeAreas;

    /** Performance tracking */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentVegetationInstances = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentPropInstances = 0;

private:
    /** Create vegetation component for type */
    UHierarchicalInstancedStaticMeshComponent* CreateVegetationComponent(EPrehistoricVegetationType VegType);

    /** Create prop component for type */
    UHierarchicalInstancedStaticMeshComponent* CreatePropComponent(EEnvironmentalPropType PropType);

    /** Place vegetation instances using PCG */
    void PlaceVegetationWithPCG(const FBiomeArtConfiguration& BiomeConfig, const FVector& Center, float Radius);

    /** Place environmental props */
    void PlaceEnvironmentalProps(const FBiomeArtConfiguration& BiomeConfig, const FVector& Center, float Radius);

    /** Apply landscape materials for biome */
    void ApplyLandscapeMaterials(EBiomeType BiomeType, const FVector& Center, float Radius);

    /** Update LOD levels based on distance */
    void UpdateLODLevels();

    /** Cull distant instances for performance */
    void CullDistantInstances();

    /** Validate placement location */
    bool IsValidPlacementLocation(const FVector& Location, const FVegetationPlacementRules& Rules) const;

    /** Get terrain slope at location */
    float GetTerrainSlope(const FVector& Location) const;

    /** Get distance to nearest water */
    float GetDistanceToNearestWater(const FVector& Location) const;
};