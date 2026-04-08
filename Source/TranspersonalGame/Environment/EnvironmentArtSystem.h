// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "FoliageType.h"
#include "InstancedFoliageActor.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "EnvironmentArtSystem.generated.h"

class UPCGGraph;
class UMaterialInterface;
class UStaticMesh;
class UFoliageType;
class AInstancedFoliageActor;

/**
 * @brief Environment Art System for Jurassic World
 * 
 * Creates rich, detailed prehistoric environments with:
 * - Layered vegetation systems (canopy, understory, ground cover)
 * - Geological features (rock formations, outcrops, caves)
 * - Environmental storytelling props (fallen logs, bones, nests)
 * - Atmospheric details (particle effects, ambient audio)
 * - Performance-optimized LOD systems
 * 
 * Design Philosophy:
 * - Every detail tells a story - props hint at the world's history
 * - Natural chaos over perfect order - imperfection creates believability
 * - Light guides the eye - composition follows cinematographic principles
 * - Performance is art - beautiful worlds that run smoothly
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */

/** Environment detail types for categorization */
UENUM(BlueprintType)
enum class EEnvironmentDetailType : uint8
{
    Vegetation          UMETA(DisplayName = \"Vegetation\"),
    RockFormation       UMETA(DisplayName = \"Rock Formation\"),
    FallenLog           UMETA(DisplayName = \"Fallen Log\"),
    AnimalBones         UMETA(DisplayName = \"Animal Bones\"),
    DinosaurNest        UMETA(DisplayName = \"Dinosaur Nest\"),
    WaterFeature        UMETA(DisplayName = \"Water Feature\"),
    GroundClutter       UMETA(DisplayName = \"Ground Clutter\"),
    AtmosphericEffect   UMETA(DisplayName = \"Atmospheric Effect\")
};

/** Prehistoric vegetation types */
UENUM(BlueprintType)
enum class EPrehistoricVegetationType : uint8
{
    // Canopy Layer
    Araucaria           UMETA(DisplayName = \"Araucaria (Monkey Puzzle)\"),
    Ginkgo              UMETA(DisplayName = \"Ginkgo\"),
    Sequoia             UMETA(DisplayName = \"Sequoia\"),
    CycadPalm           UMETA(DisplayName = \"Cycad Palm\"),
    
    // Understory Layer
    TreeFern            UMETA(DisplayName = \"Tree Fern\"),
    Horsetail           UMETA(DisplayName = \"Horsetail\"),
    Bennettitales       UMETA(DisplayName = \"Bennettitales\"),
    
    // Ground Layer
    Fern                UMETA(DisplayName = \"Fern\"),
    Moss                UMETA(DisplayName = \"Moss\"),
    Liverwort           UMETA(DisplayName = \"Liverwort\"),
    PrimitiveGrass      UMETA(DisplayName = \"Primitive Grass\"),
    
    // Aquatic
    WaterFern           UMETA(DisplayName = \"Water Fern\"),
    Algae               UMETA(DisplayName = \"Algae\"),
    ReedPlant           UMETA(DisplayName = \"Reed Plant\")
};

/** Rock formation types for geological storytelling */
UENUM(BlueprintType)
enum class ERockFormationType : uint8
{
    Outcrop             UMETA(DisplayName = \"Rock Outcrop\"),
    Boulder             UMETA(DisplayName = \"Boulder\"),
    CliffFace           UMETA(DisplayName = \"Cliff Face\"),
    CaveEntrance        UMETA(DisplayName = \"Cave Entrance\"),
    Stalactite          UMETA(DisplayName = \"Stalactite\"),
    Stalagmite          UMETA(DisplayName = \"Stalagmite\"),
    ErodedRock          UMETA(DisplayName = \"Eroded Rock\"),
    VolcanicRock        UMETA(DisplayName = \"Volcanic Rock\")
};

/** Environmental storytelling prop types */
UENUM(BlueprintType)
enum class EStorytellingPropType : uint8
{
    FallenTree          UMETA(DisplayName = \"Fallen Tree\"),
    DinosaurBones       UMETA(DisplayName = \"Dinosaur Bones\"),
    DinosaurSkull       UMETA(DisplayName = \"Dinosaur Skull\"),
    DinosaurNest        UMETA(DisplayName = \"Dinosaur Nest\"),
    DinosaurEggs        UMETA(DisplayName = \"Dinosaur Eggs\"),
    DinosaurFootprint   UMETA(DisplayName = \"Dinosaur Footprint\"),
    AncientArtifact     UMETA(DisplayName = \"Ancient Artifact\"),
    FossilizedWood      UMETA(DisplayName = \"Fossilized Wood\"),
    CrystalFormation    UMETA(DisplayName = \"Crystal Formation\")
};

/** Vegetation layer configuration */
USTRUCT(BlueprintType)
struct FVegetationLayerConfig
{
    GENERATED_BODY()

    /** Vegetation type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Vegetation\")
    EPrehistoricVegetationType VegetationType;

    /** Static mesh for this vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Mesh\")
    TObjectPtr<UStaticMesh> VegetationMesh;

    /** Foliage type for advanced settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Foliage\")
    TObjectPtr<UFoliageType> FoliageType;

    /** Density per 100m² */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Distribution\", meta = (ClampMin = 0.0f, ClampMax = 1000.0f))
    float Density = 50.0f;

    /** Size variation range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Variation\")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);\n\n    /** Height range where this vegetation grows */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Environment\")\n    FVector2D AltitudeRange = FVector2D(-500.0f, 2000.0f);\n\n    /** Maximum slope for placement (degrees) */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Environment\", meta = (ClampMin = 0.0f, ClampMax = 90.0f))\n    float MaxSlope = 30.0f;\n\n    /** Distance from water preference (-1 = avoid, 0 = neutral, 1 = prefer) */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Environment\", meta = (ClampMin = -1.0f, ClampMax = 1.0f))\n    float WaterPreference = 0.0f;\n\n    /** Sunlight preference (0 = shade, 1 = full sun) */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Environment\", meta = (ClampMin = 0.0f, ClampMax = 1.0f))\n    float SunlightPreference = 0.5f;\n\n    /** LOD distances for performance */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")\n    TArray<float> LODDistances = {1000.0f, 3000.0f, 8000.0f};\n\n    /** Cull distance */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")\n    float CullDistance = 15000.0f;\n\n    FVegetationLayerConfig()\n    {\n        VegetationType = EPrehistoricVegetationType::Fern;\n        VegetationMesh = nullptr;\n        FoliageType = nullptr;\n    }\n};\n\n/** Rock formation configuration */\nUSTRUCT(BlueprintType)\nstruct FRockFormationConfig\n{\n    GENERATED_BODY()\n\n    /** Rock formation type */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Rock\")\n    ERockFormationType FormationType;\n\n    /** Static mesh for this rock formation */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Mesh\")\n    TObjectPtr<UStaticMesh> RockMesh;\n\n    /** Material for the rock */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Material\")\n    TObjectPtr<UMaterialInterface> RockMaterial;\n\n    /** Placement density per km² */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Distribution\", meta = (ClampMin = 0.0f, ClampMax = 100.0f))\n    float Density = 5.0f;\n\n    /** Size variation range */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Variation\")\n    FVector2D ScaleRange = FVector2D(0.7f, 1.5f);\n\n    /** Rotation variation (degrees) */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Variation\")\n    FVector RotationVariation = FVector(15.0f, 360.0f, 15.0f);\n\n    /** Preferred slope range for placement */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Environment\")\n    FVector2D SlopeRange = FVector2D(0.0f, 45.0f);\n\n    /** Geological formation preference */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Environment\")\n    TArray<FString> PreferredFormations;\n\n    FRockFormationConfig()\n    {\n        FormationType = ERockFormationType::Boulder;\n        RockMesh = nullptr;\n        RockMaterial = nullptr;\n    }\n};\n\n/** Storytelling prop configuration */\nUSTRUCT(BlueprintType)\nstruct FStorytellingPropConfig\n{\n    GENERATED_BODY()\n\n    /** Prop type */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Prop\")\n    EStorytellingPropType PropType;\n\n    /** Static mesh for this prop */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Mesh\")\n    TObjectPtr<UStaticMesh> PropMesh;\n\n    /** Material for the prop */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Material\")\n    TObjectPtr<UMaterialInterface> PropMaterial;\n\n    /** Rarity (0 = common, 1 = very rare) */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Distribution\", meta = (ClampMin = 0.0f, ClampMax = 1.0f))\n    float Rarity = 0.5f;\n\n    /** Cluster size (how many spawn together) */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Distribution\")\n    FIntPoint ClusterSize = FIntPoint(1, 3);\n\n    /** Cluster radius */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Distribution\")\n    float ClusterRadius = 500.0f;\n\n    /** Narrative context (what story this prop tells) */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Storytelling\")\n    FString NarrativeContext;\n\n    /** Associated sound effect */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Audio\")\n    TObjectPtr<class USoundCue> AmbientSound;\n\n    /** Associated particle effect */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"VFX\")\n    TObjectPtr<class UParticleSystem> ParticleEffect;\n\n    FStorytellingPropConfig()\n    {\n        PropType = EStorytellingPropType::FallenTree;\n        PropMesh = nullptr;\n        PropMaterial = nullptr;\n    }\n};\n\n/** Biome environment configuration */\nUSTRUCT(BlueprintType)\nstruct FBiomeEnvironmentConfig\n{\n    GENERATED_BODY()\n\n    /** Biome name */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Biome\")\n    FString BiomeName;\n\n    /** Vegetation layers for this biome */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Vegetation\")\n    TArray<FVegetationLayerConfig> VegetationLayers;\n\n    /** Rock formations for this biome */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Geology\")\n    TArray<FRockFormationConfig> RockFormations;\n\n    /** Storytelling props for this biome */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Storytelling\")\n    TArray<FStorytellingPropConfig> StorytellingProps;\n\n    /** Landscape material for terrain */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Materials\")\n    TObjectPtr<UMaterialInterface> LandscapeMaterial;\n\n    /** Ambient lighting color tint */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Atmosphere\")\n    FLinearColor AmbientTint = FLinearColor::White;\n\n    /** Fog density multiplier */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Atmosphere\")\n    float FogDensityMultiplier = 1.0f;\n\n    /** Wind strength for vegetation */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Atmosphere\")\n    float WindStrength = 1.0f;\n\n    /** Ambient sound for this biome */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Audio\")\n    TObjectPtr<class USoundCue> BiomeAmbientSound;\n\n    FBiomeEnvironmentConfig()\n    {\n        BiomeName = TEXT(\"Default Biome\");\n        LandscapeMaterial = nullptr;\n        BiomeAmbientSound = nullptr;\n    }\n};\n\n/**\n * @brief Environment Art Data Asset\n * \n * Contains all configuration data for environment art generation\n * Allows artists to define biome characteristics without code changes\n */\nUCLASS(BlueprintType, Blueprintable)\nclass TRANSPERSONALGAME_API UEnvironmentArtData : public UDataAsset\n{\n    GENERATED_BODY()\n\npublic:\n    /** All biome configurations */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Biomes\")\n    TArray<FBiomeEnvironmentConfig> BiomeConfigs;\n\n    /** Global vegetation settings */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Global Settings\")\n    float GlobalVegetationDensityMultiplier = 1.0f;\n\n    /** Global rock density multiplier */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Global Settings\")\n    float GlobalRockDensityMultiplier = 1.0f;\n\n    /** Global storytelling prop density */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Global Settings\")\n    float GlobalStorytellingDensity = 1.0f;\n\n    /** Performance settings */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")\n    int32 MaxInstancesPerCell = 50000;\n\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")\n    float CullingDistanceMultiplier = 1.0f;\n\n    /** PCG Graph for environment generation */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"PCG\")\n    TObjectPtr<UPCGGraph> EnvironmentPCGGraph;\n};\n\n/**\n * @brief Main Environment Art System Component\n * \n * Manages the complete environment art pipeline:\n * 1. Vegetation placement using advanced foliage systems\n * 2. Rock formation generation with geological accuracy\n * 3. Environmental storytelling prop placement\n * 4. Material blending and atmospheric effects\n * 5. Performance optimization with LOD and culling\n */\nUCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))\nclass TRANSPERSONALGAME_API UEnvironmentArtSystem : public UActorComponent\n{\n    GENERATED_BODY()\n\npublic:\n    UEnvironmentArtSystem();\n\nprotected:\n    virtual void BeginPlay() override;\n    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;\n\npublic:\n    /** Generate environment art for a specific area */\n    UFUNCTION(BlueprintCallable, Category = \"Environment Art\")\n    void GenerateEnvironmentInArea(FVector Center, float Radius, const FString& BiomeName);\n\n    /** Clear environment art in area (for paths, clearings) */\n    UFUNCTION(BlueprintCallable, Category = \"Environment Art\")\n    void ClearEnvironmentInArea(FVector Center, float Radius, float ClearancePercentage = 1.0f);\n\n    /** Place storytelling props in area */\n    UFUNCTION(BlueprintCallable, Category = \"Storytelling\")\n    void PlaceStorytellingPropsInArea(FVector Center, float Radius, const FString& BiomeName, int32 PropCount = 5);\n\n    /** Update vegetation for season/time changes */\n    UFUNCTION(BlueprintCallable, Category = \"Environment Art\")\n    void UpdateVegetationForSeason(float SeasonProgress);\n\n    /** Optimize performance in area */\n    UFUNCTION(BlueprintCallable, Category = \"Performance\")\n    void OptimizePerformanceInArea(FVector Center, float Radius);\n\n    /** Get biome configuration by name */\n    UFUNCTION(BlueprintCallable, Category = \"Environment Art\")\n    FBiomeEnvironmentConfig GetBiomeConfig(const FString& BiomeName);\n\nprotected:\n    /** Environment art data asset */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Configuration\")\n    TObjectPtr<UEnvironmentArtData> EnvironmentData;\n\n    /** PCG component for procedural generation */\n    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"PCG\")\n    TObjectPtr<UPCGComponent> PCGComponent;\n\n    /** Instanced foliage actor reference */\n    UPROPERTY()\n    TObjectPtr<AInstancedFoliageActor> FoliageActor;\n\n    /** Performance monitoring */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")\n    float TargetFrameTime = 16.67f; // 60 FPS\n\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")\n    float LODDistanceMultiplier = 1.0f;\n\n    /** Debug visualization */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Debug\")\n    bool bShowDebugInfo = false;\n\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Debug\")\n    bool bShowVegetationBounds = false;\n\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Debug\")\n    bool bShowRockFormationBounds = false;\n\nprivate:\n    /** Internal generation functions */\n    void GenerateVegetationLayer(const FVegetationLayerConfig& LayerConfig, FVector Center, float Radius);\n    void GenerateRockFormations(const TArray<FRockFormationConfig>& RockConfigs, FVector Center, float Radius);\n    void GenerateStorytellingProps(const TArray<FStorytellingPropConfig>& PropConfigs, FVector Center, float Radius);\n    \n    /** Placement validation */\n    bool IsLocationSuitableForVegetation(FVector Location, const FVegetationLayerConfig& Config);\n    bool IsLocationSuitableForRock(FVector Location, const FRockFormationConfig& Config);\n    bool IsLocationSuitableForProp(FVector Location, const FStorytellingPropConfig& Config);\n    \n    /** Environmental queries */\n    float GetTerrainSlope(FVector Location);\n    float GetDistanceToWater(FVector Location);\n    float GetSunlightExposure(FVector Location);\n    FString GetGeologicalFormation(FVector Location);\n    \n    /** Performance optimization */\n    void UpdateLODDistances(float FrameTime);\n    void CullDistantInstances(FVector ViewerLocation);\n    \n    /** Cached data for performance */\n    TMap<FString, FBiomeEnvironmentConfig> BiomeConfigCache;\n    TArray<FVector> RecentlyGeneratedAreas;\n    float LastPerformanceCheck = 0.0f;\n};\n\n/**\n * @brief Environment Art Manager Actor\n * \n * World-level manager for environment art systems\n * Coordinates between multiple environment art components\n * Handles streaming and world partition integration\n */\nUCLASS(BlueprintType, Blueprintable)\nclass TRANSPERSONALGAME_API AEnvironmentArtManager : public AActor\n{\n    GENERATED_BODY()\n\npublic:\n    AEnvironmentArtManager();\n\nprotected:\n    virtual void BeginPlay() override;\n    virtual void Tick(float DeltaTime) override;\n\npublic:\n    /** Initialize environment art for the entire world */\n    UFUNCTION(BlueprintCallable, Category = \"Environment Management\")\n    void InitializeWorldEnvironmentArt();\n\n    /** Stream in environment art for area */\n    UFUNCTION(BlueprintCallable, Category = \"Environment Management\")\n    void StreamInEnvironmentArt(FVector Location, float Radius);\n\n    /** Stream out environment art for area */\n    UFUNCTION(BlueprintCallable, Category = \"Environment Management\")\n    void StreamOutEnvironmentArt(FVector Location, float Radius);\n\n    /** Update environment art based on player location */\n    UFUNCTION(BlueprintCallable, Category = \"Environment Management\")\n    void UpdateEnvironmentForPlayer(FVector PlayerLocation);\n\nprotected:\n    /** Main environment art system */\n    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"Components\")\n    TObjectPtr<UEnvironmentArtSystem> EnvironmentArtSystem;\n\n    /** Streaming settings */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Streaming\")\n    float StreamingRadius = 5000.0f;\n\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Streaming\")\n    float UnloadRadius = 8000.0f;\n\n    /** Performance monitoring */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")\n    float PerformanceCheckInterval = 1.0f;\n\n    /** World partition integration */\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"World Partition\")\n    bool bUseWorldPartition = true;\n\n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"World Partition\")\n    float CellSize = 25600.0f; // 256m cells\n\nprivate:\n    /** Cached player location for streaming */\n    FVector LastPlayerLocation = FVector::ZeroVector;\n    float LastPerformanceCheck = 0.0f;\n    \n    /** Active environment art areas */\n    TSet<FIntPoint> ActiveCells;\n    TMap<FIntPoint, float> CellLoadTimes;\n};