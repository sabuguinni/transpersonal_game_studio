// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Foliage/Public/FoliageType.h"
#include "Foliage/Public/FoliageInstancedStaticMeshComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EnvironmentArtSubsystem.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UFoliageType;
class UPCGComponent;
class ALandscape;

/**
 * @brief Environment Art Subsystem for Transpersonal Game Studio
 * 
 * Transforms the procedurally generated terrain into a living, breathing prehistoric world.
 * Adds vegetation, rocks, props, materials and environmental storytelling details that make
 * the world feel inhabited and authentic to the Jurassic/Cretaceous period.
 * 
 * Core Responsibilities:
 * - Vegetation placement and variation (trees, ferns, cycads, conifers)
 * - Rock and geological prop placement (boulders, outcrops, fossil deposits)
 * - Environmental storytelling props (bones, nests, territorial markings)
 * - Material application and blending for terrain surfaces
 * - Atmospheric details (fallen logs, debris, water effects)
 * - Performance-optimized LOD chains and culling systems
 * 
 * Design Philosophy:
 * - Every prop tells a story - nothing exists without narrative purpose
 * - Layered detail from macro (forest composition) to micro (individual leaves)
 * - Biome-specific authenticity - each environment has unique character
 * - Procedural variation to avoid repetition while maintaining artistic control
 * - Performance-first approach with aggressive LOD and culling systems
 * 
 * Technical Implementation:
 * - Uses UE5 Foliage system for vegetation with custom density maps
 * - Hierarchical Instanced Static Meshes for performance
 * - PCG integration for procedural placement rules
 * - Material layering system for terrain surface variation
 * - Custom LOD chains optimized for prehistoric vegetation
 * - GPU-based culling for massive instance counts
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnvironmentArtSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEnvironmentArtSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    /** Main environment art functions */
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PopulateEnvironment(const FEnvironmentArtSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void RefreshRegion(const FBox& WorldBounds);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ClearEnvironmentArt();

    /** Vegetation system */
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void PlaceVegetation(const FVegetationPlacementSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void UpdateVegetationDensity(EBiomeType BiomeType, float DensityMultiplier);

    /** Rock and geological features */
    UFUNCTION(BlueprintCallable, Category = "Geology")
    void PlaceRocksAndOutcrops(const FRockPlacementSettings& Settings);

    /** Environmental storytelling props */
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlaceStorytellingProps(const FStorytellingSettings& Settings);

    /** Material system */
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void ApplyTerrainMaterials(const FTerrainMaterialSettings& Settings);

    /** Performance monitoring */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEnvironmentArtPerformanceStats GetPerformanceStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForPerformance(const FPerformanceOptimizationSettings& Settings);

protected:
    /** Biome types from World Generation Subsystem */
    UENUM(BlueprintType)
    enum class EBiomeType : uint8
    {
        TropicalRainforest      UMETA(DisplayName = "Tropical Rainforest"),
        TemperateForest         UMETA(DisplayName = "Temperate Forest"),
        ConiferousForest        UMETA(DisplayName = "Coniferous Forest"),
        Grassland               UMETA(DisplayName = "Grassland"),
        Savanna                 UMETA(DisplayName = "Savanna"),
        Desert                  UMETA(DisplayName = "Desert"),
        Wetland                 UMETA(DisplayName = "Wetland"),
        CoastalPlain            UMETA(DisplayName = "Coastal Plain"),
        MountainousRegion       UMETA(DisplayName = "Mountainous Region"),
        VolcanicRegion          UMETA(DisplayName = "Volcanic Region"),
        RiverDelta              UMETA(DisplayName = "River Delta"),
        LakeShore               UMETA(DisplayName = "Lake Shore")
    };

    /** Vegetation types for prehistoric world */
    UENUM(BlueprintType)
    enum class EVegetationType : uint8
    {
        // Trees
        ConiferLarge            UMETA(DisplayName = "Large Conifer (Araucaria)"),
        ConiferMedium           UMETA(DisplayName = "Medium Conifer"),
        ConiferSmall            UMETA(DisplayName = "Small Conifer"),
        CycadLarge              UMETA(DisplayName = "Large Cycad"),
        CycadMedium             UMETA(DisplayName = "Medium Cycad"),
        FernTree                UMETA(DisplayName = "Tree Fern"),
        GinkgoTree              UMETA(DisplayName = "Ginkgo Tree"),
        
        // Understory
        FernLarge               UMETA(DisplayName = "Large Fern"),
        FernMedium              UMETA(DisplayName = "Medium Fern"),
        FernSmall               UMETA(DisplayName = "Small Fern"),
        Horsetail               UMETA(DisplayName = "Horsetail"),
        Moss                    UMETA(DisplayName = "Moss Patches"),
        
        // Ground cover
        GrassPrehistoric        UMETA(DisplayName = "Prehistoric Grass"),
        Sedge                   UMETA(DisplayName = "Sedge"),
        Liverwort               UMETA(DisplayName = "Liverwort"),
        
        // Aquatic
        WaterFern               UMETA(DisplayName = "Water Fern"),
        Algae                   UMETA(DisplayName = "Algae"),
        ReedGrass               UMETA(DisplayName = "Reed Grass")
    };

    /** Rock and geological prop types */
    UENUM(BlueprintType)
    enum class ERockType : uint8
    {
        BoulderLarge            UMETA(DisplayName = "Large Boulder"),
        BoulderMedium           UMETA(DisplayName = "Medium Boulder"),
        BoulderSmall            UMETA(DisplayName = "Small Boulder"),
        RockOutcrop             UMETA(DisplayName = "Rock Outcrop"),
        CliffFace               UMETA(DisplayName = "Cliff Face"),
        SedimentaryLayer        UMETA(DisplayName = "Sedimentary Layer"),
        VolcanicRock            UMETA(DisplayName = "Volcanic Rock"),
        FossilDeposit           UMETA(DisplayName = "Fossil Deposit"),
        CrystalFormation        UMETA(DisplayName = "Crystal Formation"),
        Pebbles                 UMETA(DisplayName = "Pebble Scatter")
    };

    /** Storytelling prop types */
    UENUM(BlueprintType)
    enum class EStorytellingPropType : uint8
    {
        // Dinosaur traces
        DinosaurBones           UMETA(DisplayName = "Dinosaur Bones"),
        DinosaurNest            UMETA(DisplayName = "Dinosaur Nest"),
        DinosaurFootprints      UMETA(DisplayName = "Dinosaur Footprints"),
        DinosaurScat            UMETA(DisplayName = "Dinosaur Droppings"),
        ClawMarks               UMETA(DisplayName = "Claw Marks on Trees"),
        
        // Environmental details
        FallenLog               UMETA(DisplayName = "Fallen Log"),
        DeadTree                UMETA(DisplayName = "Dead Tree"),
        RottenStump             UMETA(DisplayName = "Rotten Stump"),
        BrokenBranches          UMETA(DisplayName = "Broken Branches"),
        
        // Water features
        MudPuddle               UMETA(DisplayName = "Mud Puddle"),
        StreamRocks             UMETA(DisplayName = "Stream Rocks"),
        WaterLilies             UMETA(DisplayName = "Water Lilies"),
        
        // Atmospheric details
        FlyingInsects           UMETA(DisplayName = "Flying Insects"),
        ButterflySwarm          UMETA(DisplayName = "Butterfly Swarm"),
        BirdNest                UMETA(DisplayName = "Bird Nest"),
        SpiderWeb               UMETA(DisplayName = "Spider Web")
    };

    /** Environment art settings structure */
    USTRUCT(BlueprintType)
    struct FEnvironmentArtSettings
    {
        GENERATED_BODY()

        /** Overall density multiplier (0-2) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float OverallDensity = 1.0f;

        /** Quality level for environment art */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EEnvironmentArtQuality Quality = EEnvironmentArtQuality::High;

        /** Enable procedural variation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bEnableProceduralVariation = true;

        /** Enable storytelling props */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bEnableStorytellingProps = true;

        /** Performance budget for environment art */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FEnvironmentArtPerformanceBudget PerformanceBudget;

        /** Vegetation placement settings */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FVegetationPlacementSettings Vegetation;

        /** Rock placement settings */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FRockPlacementSettings Rocks;

        /** Storytelling prop settings */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FStorytellingSettings Storytelling;

        /** Terrain material settings */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FTerrainMaterialSettings Materials;
    };

    /** Environment art quality levels */
    UENUM(BlueprintType)
    enum class EEnvironmentArtQuality : uint8
    {
        Low         UMETA(DisplayName = "Low Quality"),
        Medium      UMETA(DisplayName = "Medium Quality"),
        High        UMETA(DisplayName = "High Quality"),
        Epic        UMETA(DisplayName = "Epic Quality"),
        Cinematic   UMETA(DisplayName = "Cinematic Quality")
    };

    /** Vegetation placement settings */
    USTRUCT(BlueprintType)
    struct FVegetationPlacementSettings
    {
        GENERATED_BODY()

        /** Base vegetation density per biome */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<EBiomeType, float> BiomeDensityMap;

        /** Vegetation type distribution per biome */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<EBiomeType, TArray<FVegetationTypeWeight>> BiomeVegetationMap;

        /** Minimum distance between large trees */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float LargeTreeSpacing = 800.0f;

        /** Maximum slope for tree placement (degrees) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxTreeSlope = 35.0f;

        /** Water avoidance distance for most vegetation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float WaterAvoidanceDistance = 200.0f;

        /** Clustering factor for natural grouping */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float ClusteringFactor = 0.7f;

        /** Random seed for vegetation placement */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 VegetationSeed = 13579;

        /** Enable seasonal variation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bEnableSeasonalVariation = true;

        /** Enable wind animation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bEnableWindAnimation = true;

        /** Wind strength multiplier */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float WindStrength = 1.0f;
    };

    /** Vegetation type weight for biome distribution */
    USTRUCT(BlueprintType)
    struct FVegetationTypeWeight
    {
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EVegetationType VegetationType = EVegetationType::ConiferMedium;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Weight = 1.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MinElevation = -1000.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxElevation = 1000.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MinSlope = 0.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxSlope = 90.0f;
    };

    /** Rock placement settings */
    USTRUCT(BlueprintType)
    struct FRockPlacementSettings
    {
        GENERATED_BODY()

        /** Base rock density per biome */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<EBiomeType, float> BiomeRockDensityMap;

        /** Rock type distribution per biome */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<EBiomeType, TArray<FRockTypeWeight>> BiomeRockMap;

        /** Prefer steep slopes for rock placement */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float PreferredSlope = 25.0f;

        /** Slope preference strength */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float SlopePreferenceStrength = 2.0f;

        /** Clustering factor for natural rock grouping */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float RockClusteringFactor = 0.8f;

        /** Random seed for rock placement */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 RockSeed = 24680;

        /** Enable geological storytelling */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bEnableGeologicalStorytelling = true;
    };

    /** Rock type weight for biome distribution */
    USTRUCT(BlueprintType)
    struct FRockTypeWeight
    {
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        ERockType RockType = ERockType::BoulderMedium;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Weight = 1.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MinElevation = -1000.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxElevation = 1000.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MinSlope = 0.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxSlope = 90.0f;
    };

    /** Storytelling prop settings */
    USTRUCT(BlueprintType)
    struct FStorytellingSettings
    {
        GENERATED_BODY()

        /** Enable dinosaur traces */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bEnableDinosaurTraces = true;

        /** Enable environmental storytelling */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bEnableEnvironmentalStorytelling = true;

        /** Storytelling prop density */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float StorytellingDensity = 0.3f;

        /** Prop type distribution */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FStorytellingPropWeight> PropWeights;

        /** Random seed for storytelling placement */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 StorytellingS