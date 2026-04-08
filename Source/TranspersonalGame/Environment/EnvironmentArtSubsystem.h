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

/** Quality levels for environment art */
UENUM(BlueprintType)
enum class EEnvironmentArtQuality : uint8
{
    Low                     UMETA(DisplayName = "Low Quality"),
    Medium                  UMETA(DisplayName = "Medium Quality"),
    High                    UMETA(DisplayName = "High Quality"),
    Ultra                   UMETA(DisplayName = "Ultra Quality")
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

    /** World bounds for generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBox WorldBounds = FBox(FVector(-100000), FVector(100000));
};

/** Vegetation placement settings */
USTRUCT(BlueprintType)
struct FVegetationPlacementSettings
{
    GENERATED_BODY()

    /** Target biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBiomeType TargetBiome = EBiomeType::TropicalRainforest;

    /** Vegetation density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DensityMultiplier = 1.0f;

    /** Size variation range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D SizeVariation = FVector2D(0.8f, 1.2f);

    /** Placement area */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBox PlacementArea = FBox(FVector(-50000), FVector(50000));
};

/** Performance statistics */
USTRUCT(BlueprintType)
struct FEnvironmentArtPerformanceStats
{
    GENERATED_BODY()

    /** Total vegetation instances */
    UPROPERTY(BlueprintReadOnly)
    int32 TotalVegetationInstances = 0;

    /** Total rock instances */
    UPROPERTY(BlueprintReadOnly)
    int32 TotalRockInstances = 0;

    /** Total storytelling props */
    UPROPERTY(BlueprintReadOnly)
    int32 TotalStorytellingProps = 0;

    /** Memory usage in MB */
    UPROPERTY(BlueprintReadOnly)
    float MemoryUsageMB = 0.0f;

    /** Current frame time impact in ms */
    UPROPERTY(BlueprintReadOnly)
    float FrameTimeImpactMS = 0.0f;
};

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

    /** Performance monitoring */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEnvironmentArtPerformanceStats GetPerformanceStats() const;

protected:
    /** Current environment art settings */
    UPROPERTY()
    FEnvironmentArtSettings CurrentSettings;

    /** Performance statistics */
    UPROPERTY()
    FEnvironmentArtPerformanceStats PerformanceStats;

    /** Vegetation instances */
    UPROPERTY()
    TArray<UHierarchicalInstancedStaticMeshComponent*> VegetationComponents;

    /** Rock instances */
    UPROPERTY()
    TArray<UHierarchicalInstancedStaticMeshComponent*> RockComponents;

private:
    /** Initialize vegetation system */
    void InitializeVegetationSystem();

    /** Initialize rock placement system */
    void InitializeRockSystem();

    /** Update performance statistics */
    void UpdatePerformanceStats();
};