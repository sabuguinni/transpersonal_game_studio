// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "FoliageType.h"
#include "InstancedFoliageActor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "../Performance/PerformanceTargets.h"
#include "../WorldGeneration/ProceduralWorldGeneratorV5.h"
#include "EnvironmentArtSystem.generated.h"

class ALandscape;
class UStaticMesh;
class UMaterialInterface;
class UFoliageType;
class AInstancedFoliageActor;
class UPCGComponent;

/**
 * @brief Environment Art System — Transpersonal Game Studio
 * 
 * Transforms generated terrain into a living, breathing prehistoric world.
 * Populates the world with vegetation, rocks, props and environmental storytelling.
 * 
 * CORE PHILOSOPHY:
 * - Every prop tells a story of what happened before the player arrived
 * - Vegetation grows where it would naturally thrive
 * - Rocks and debris follow geological and erosion patterns
 * - The world feels lived-in, not artificially placed
 * - Light and composition guide the player's eye and emotions
 * 
 * TECHNICAL APPROACH:
 * - Uses UE5 Foliage System with GPU instancing for massive vegetation
 * - PCG-driven placement for intelligent prop distribution
 * - Nanite-enabled high-detail rocks and environmental meshes
 * - Dynamic material blending for natural surface variation
 * - Performance-aware LOD and culling systems
 * 
 * ARTISTIC VISION:
 * - Inspired by Roger Deakins' environmental storytelling through light
 * - RDR2-level environmental narrative detail
 * - Every cluster of objects implies a history
 * - Prehistoric authenticity with dramatic visual impact
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnvironmentArtSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEnvironmentArtSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    /** Main environment art population entry point */
    UFUNCTION(BlueprintCallable, Category = "Environment Art", CallInEditor = true)
    void PopulatePrehistoricEnvironment(const FEnvironmentArtConfig& Config);

    /** Vegetation systems */
    UFUNCTION(BlueprintCallable, Category = "Vegetation", CallInEditor = true)
    void GeneratePrehistoricVegetation(const FVegetationConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Vegetation", CallInEditor = true)
    void PlaceAncientTrees(const FTreePlacementConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Vegetation", CallInEditor = true)
    void GenerateUndergrowth(const FUndergrowthConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Vegetation", CallInEditor = true)
    void CreateFernGroves(const FFernGroveConfig& Config);

    /** Rock and geological features */
    UFUNCTION(BlueprintCallable, Category = "Geology", CallInEditor = true)
    void PlaceGeologicalFormations(const FRockFormationConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Geology", CallInEditor = true)
    void ScatterNaturalDebris(const FDebrisConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Geology", CallInEditor = true)
    void CreateRockOutcrops(const FOutcropConfig& Config);

    /** Environmental storytelling props */
    UFUNCTION(BlueprintCallable, Category = "Storytelling", CallInEditor = true)
    void PlaceEnvironmentalNarratives(const FNarrativePropsConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Storytelling", CallInEditor = true)
    void CreateDinosaurTrails(const FTrailConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Storytelling", CallInEditor = true)
    void PlaceAbandonedCampsites(const FCampsiteConfig& Config);

    /** Material and surface systems */
    UFUNCTION(BlueprintCallable, Category = "Materials", CallInEditor = true)
    void ApplyLandscapeMaterials(const FLandscapeMaterialConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Materials", CallInEditor = true)
    void BlendNaturalSurfaces(const FSurfaceBlendConfig& Config);

    /** Performance optimization */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeEnvironmentForPerformance(const FPerformanceBudget& Budget);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEnvironmentPerformanceStats GetEnvironmentPerformanceStats() const;

    /** Runtime environment management */
    UFUNCTION(BlueprintCallable, Category = "Runtime")
    void UpdateEnvironmentLOD(const FVector& ViewerLocation, float ViewDistance);

    UFUNCTION(BlueprintCallable, Category = "Runtime")
    void StreamEnvironmentContent(const FBox& StreamingBounds);

protected:
    /** Environment art configuration */
    USTRUCT(BlueprintType)
    struct FEnvironmentArtConfig
    {
        GENERATED_BODY()

        /** Art quality level */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
        EEnvironmentQuality Quality = EEnvironmentQuality::High;

        /** Vegetation density multiplier */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density", meta = (ClampMin = "0.1", ClampMax = "2.0"))
        float VegetationDensity = 1.0f;

        /** Rock formation density */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density", meta = (ClampMin = "0.1", ClampMax = "2.0"))
        float RockDensity = 1.0f;

        /** Environmental storytelling intensity */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float StorytellingIntensity = 1.0f;

        /** Use Nanite for environment meshes */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
        bool bUseNaniteForEnvironment = true;

        /** Enable GPU-driven foliage culling */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
        bool bUseGPUFoliageCulling = true;

        /** Maximum view distance for environment details */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
        float MaxViewDistance = 50000.0f; // 500m

        /** Vegetation configuration */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
        FVegetationConfig Vegetation;

        /** Rock formation configuration */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
        FRockFormationConfig RockFormations;

        /** Environmental narrative configuration */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
        FNarrativePropsConfig NarrativeProps;

        /** Landscape material configuration */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
        FLandscapeMaterialConfig LandscapeMaterials;
    };

    /** Environment quality levels */
    UENUM(BlueprintType)
    enum class EEnvironmentQuality : uint8
    {
        Low         UMETA(DisplayName = "Low - Performance Optimized"),
        Medium      UMETA(DisplayName = "Medium - Balanced"),
        High        UMETA(DisplayName = "High - Detailed"),
        Epic        UMETA(DisplayName = "Epic - Maximum Detail"),
        Cinematic   UMETA(DisplayName = "Cinematic - Film Quality")
    };

    /** Prehistoric vegetation types */
    UENUM(BlueprintType)
    enum class EPrehistoricVegetationType : uint8
    {
        Conifers            UMETA(DisplayName = "Conifers (Araucaria, Podocarpus)"),
        Ferns               UMETA(DisplayName = "Tree Ferns (Cyathea, Dicksonia)"),
        Cycads              UMETA(DisplayName = "Cycads (Cycas, Zamia)"),
        Ginkgos             UMETA(DisplayName = "Ginkgos (Ginkgo biloba ancestors)"),
        Horsetails          UMETA(DisplayName = "Giant Horsetails (Equisetum)"),
        Mosses              UMETA(DisplayName = "Mosses and Liverworts"),
        Algae               UMETA(DisplayName = "Freshwater Algae"),
        Flowering           UMETA(DisplayName = "Early Flowering Plants")
    };

    /** Vegetation configuration */
    USTRUCT(BlueprintType)
    struct FVegetationConfig
    {
        GENERATED_BODY()

        /** Enable different vegetation types */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Types")
        TMap<EPrehistoricVegetationType, bool> EnabledTypes;

        /** Vegetation density per type */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
        TMap<EPrehistoricVegetationType, float> TypeDensities;

        /** Biome-specific vegetation rules */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
        TMap<FString, FBiomeVegetationRules> BiomeRules;

        /** Seasonal variation (for dynamic seasons) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasons")
        bool bEnableSeasonalVariation = false;

        /** Wind interaction strength */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float WindInteractionStrength = 1.0f;

        /** Use GPU instancing for small vegetation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
        bool bUseGPUInstancing = true;

        /** Maximum instances per cluster */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
        int32 MaxInstancesPerCluster = 10000;

        FVegetationConfig()
        {
            // Default enabled vegetation types for Late Cretaceous
            EnabledTypes.Add(EPrehistoricVegetationType::Conifers, true);
            EnabledTypes.Add(EPrehistoricVegetationType::Ferns, true);
            EnabledTypes.Add(EPrehistoricVegetationType::Cycads, true);
            EnabledTypes.Add(EPrehistoricVegetationType::Ginkgos, true);
            EnabledTypes.Add(EPrehistoricVegetationType::Horsetails, true);
            EnabledTypes.Add(EPrehistoricVegetationType::Flowering, true);

            // Default densities
            TypeDensities.Add(EPrehistoricVegetationType::Conifers, 0.8f);
            TypeDensities.Add(EPrehistoricVegetationType::Ferns, 1.2f);
            TypeDensities.Add(EPrehistoricVegetationType::Cycads, 0.6f);
            TypeDensities.Add(EPrehistoricVegetationType::Ginkgos, 0.4f);
            TypeDensities.Add(EPrehistoricVegetationType::Horsetails, 1.0f);
            TypeDensities.Add(EPrehistoricVegetationType::Flowering, 0.7f);
        }
    };

    /** Biome-specific vegetation rules */
    USTRUCT(BlueprintType)
    struct FBiomeVegetationRules
    {
        GENERATED_BODY()

        /** Dominant vegetation type for this biome */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dominance")
        EPrehistoricVegetationType DominantType = EPrehistoricVegetationType::Conifers;

        /** Secondary vegetation types */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
        TArray<EPrehistoricVegetationType> SecondaryTypes;

        /** Vegetation size variation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size")
        FVector2D SizeRange = FVector2D(0.8f, 1.2f);

        /** Clustering behavior */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
        float ClusteringStrength = 0.7f;

        /** Elevation preference */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
        FVector2D ElevationRange = FVector2D(-1000.0f, 50000.0f);

        /** Slope tolerance */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
        float MaxSlope = 35.0f;

        /** Water proximity preference */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
        float WaterProximityBonus = 1.5f;
    };

    /** Tree placement configuration */
    USTRUCT(BlueprintType)
    struct FTreePlacementConfig
    {
        GENERATED_BODY()

        /** Ancient tree species to place */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
        TArray<TSoftObjectPtr<UStaticMesh>> AncientTreeMeshes;

        /** Tree density per square kilometer */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
        float TreesPerSqKm = 150.0f;

        /** Size variation for ancient trees */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size")
        FVector2D TreeSizeRange = FVector2D(0.8f, 1.5f);

        /** Age variation (affects model choice) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Age")
        FVector2D AgeRange = FVector2D(0.3f, 1.0f);

        /** Forest clustering strength */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
        float ForestClusteringStrength = 0.8f;

        /** Clearings and gaps */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
        float ClearingProbability = 0.15f;

        /** Enable fallen trees for storytelling */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
        bool bIncludeFallenTrees = true;

        /** Fallen tree probability */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
        float FallenTreeProbability = 0.05f;
    };

    /** Rock formation configuration */
    USTRUCT(BlueprintType)
    struct FRockFormationConfig
    {
        GENERATED_BODY()

        /** Rock types for different geological periods */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
        TMap<FString, TSoftObjectPtr<UStaticMesh>> RockMeshes;

        /** Rock density per square kilometer */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
        float RocksPerSqKm = 50.0f;

        /** Size variation for rocks */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size")
        FVector2D RockSizeRange = FVector2D(0.5f, 2.0f);

        /** Geological clustering (rocks form in groups) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
        float GeologicalClustering = 0.9f;

        /** Erosion patterns (affects placement) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
        float ErosionInfluence = 0.7f;

        /** Slope preference for rock placement */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
        float SlopePreference = 0.6f;

        /** Enable boulder fields */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
        bool bCreateBoulderFields = true;

        /** Enable rock arches and formations */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
        bool bCreateRockFormations = true;
    };

    /** Environmental storytelling props configuration */
    USTRUCT(BlueprintType)
    struct FNarrativePropsConfig
    {
        GENERATED_BODY()

        /** Enable different narrative elements */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elements")
        bool bPlaceDinosaurBones = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elements")
        bool bCreateDinosaurNests = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elements")
        bool bAddPrehistoricArtifacts = false; // No human artifacts in this period

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elements")
        bool bCreateNaturalShelters = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elements")
        bool bAddWeatherDamage = true;

        /** Storytelling intensity per area */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
        float NarrativeElementsPerSqKm = 5.0f;

        /** Narrative clustering (stories happen in groups) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
        float NarrativeClustering = 0.8f;

        /** Age of narrative elements (weathering) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Age")
        FVector2D ElementAgeRange = FVector2D(0.2f, 1.0f);
    };

    /** Landscape material configuration */
    USTRUCT(BlueprintType)
    struct FLandscapeMaterialConfig
    {
        GENERATED_BODY()

        /** Base landscape material */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
        TSoftObjectPtr<UMaterialInterface> BaseLandscapeMaterial;

        /** Material layers for different terrain types */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layers")
        TMap<FString, TSoftObjectPtr<UMaterialInterface>> TerrainLayers;

        /** Texture resolution for landscape materials */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
        int32 TextureResolution = 2048;

        /** Enable material blending */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
        bool bEnableAdvancedBlending = true;

        /** Blending sharpness */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending", meta = (ClampMin = "0.1", ClampMax = "2.0"))
        float BlendingSharpness = 1.0f;

        /** Enable dynamic weathering */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
        bool bEnableDynamicWeathering = true;

        /** Weathering intensity */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering", meta = (ClampMin = "0.0", ClampMax = "2.0"))
        float WeatheringIntensity = 0.8f;
    };

    /** Performance statistics */
    USTRUCT(BlueprintType)
    struct FEnvironmentPerformanceStats
    {
        GENERATED_BODY()

        /** Current foliage instance count */
        UPROPERTY(BlueprintReadOnly, Category = "Foliage")
        int32 FoliageInstanceCount = 0;

        /** Current rock instance count */
        UPROPERTY(BlueprintReadOnly, Category = "Rocks")
        int32 RockInstanceCount = 0;

        /** Current prop instance count */
        UPROPERTY(BlueprintReadOnly, Category = "Props")
        int32 PropInstanceCount = 0;

        /** Memory usage in MB */
        UPROPERTY(BlueprintReadOnly, Category = "Memory")
        float MemoryUsageMB = 0.0f;

        /** Current frame time impact in ms */
        UPROPERTY(BlueprintReadOnly, Category = "Performance")
        float FrameTimeImpactMS = 0.0f;

        /** LOD efficiency percentage */
        UPROPERTY(BlueprintReadOnly, Category = "Performance")
        float LODEfficiency = 100.0f;
    };

private:
    /** World generation system reference */
    UPROPERTY()
    TObjectPtr<UProceduralWorldGeneratorV5> WorldGenerator;

    /** Current performance budget */
    UPROPERTY()
    FPerformanceBudget CurrentBudget;

    /** Foliage actor for vegetation management */
    UPROPERTY()
    TObjectPtr<AInstancedFoliageActor> FoliageActor;

    /** PCG components for procedural placement */
    UPROPERTY()
    TArray<TObjectPtr<UPCGComponent>> PCGComponents;

    /** Current environment statistics */
    UPROPERTY()
    FEnvironmentPerformanceStats CurrentStats;

    /** Internal generation methods */
    void GenerateVegetationForBiome(const FString& BiomeName, const FBiomeVegetationRules& Rules, const FBox& Bounds);
    void PlaceRockFormationCluster(const FVector& Location, const FRockFormationConfig& Config);
    void CreateNarrativeScene(const FVector& Location, const FNarrativePropsConfig& Config);
    void ApplyMaterialToLandscapeRegion(const FBox& Region, UMaterialInterface* Material);
    void OptimizeFoliageInstances();
    void UpdateLODDistances();
    
    /** Performance monitoring */
    void UpdatePerformanceStats();
    bool IsWithinPerformanceBudget() const;
};