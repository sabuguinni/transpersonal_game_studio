// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "PCGPoint.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerVolume.h"
#include "../WorldGeneration/BiomeSystem.h"
#include "ArchitectureSystem.h"
#include "StructureGenerator.generated.h"

/**
 * @brief Structure Generator for Transpersonal Game Studio
 * 
 * Procedurally generates prehistoric structures using PCG Framework.
 * Each structure is placed with intention - near water sources, on elevated
 * ground for defense, in sheltered areas for protection from weather.
 * 
 * Generation Rules:
 * - Structures follow natural human settlement patterns
 * - Defensive structures on high ground with clear sightlines
 * - Shelters in protected areas with wind breaks
 * - Water collection near streams and rivers
 * - Tool caches hidden but accessible
 * - Ritual sites in prominent, sacred-feeling locations
 * 
 * Technical Implementation:
 * - Uses UE5 PCG Framework for placement logic
 * - Modular component system for variety
 * - Biome-specific material and construction choices
 * - Structural integrity simulation
 * - Weathering and aging systems
 * 
 * @author Architecture & Interior Agent — Agent #7
 * @version 1.0 — March 2026
 */

/** Structure placement context data */
USTRUCT(BlueprintType)
struct FStructurePlacementContext
{
    GENERATED_BODY()

    /** World location for structure */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector WorldLocation = FVector::ZeroVector;

    /** Terrain slope at location (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float TerrainSlope = 0.0f;

    /** Distance to nearest water source */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float DistanceToWater = 1000.0f;

    /** Elevation above sea level */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float Elevation = 0.0f;

    /** Vegetation density around location (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float VegetationDensity = 0.5f;

    /** Wind exposure level (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float WindExposure = 0.5f;

    /** Visibility range from location */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float VisibilityRange = 500.0f;

    /** Biome type at this location */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    EBiomeType BiomeType = EBiomeType::DenseJungle;

    /** Soil type/drainage quality */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float DrainageQuality = 0.5f;

    /** Natural shelter availability */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float NaturalShelter = 0.3f;

    /** Threat level (predator activity) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float ThreatLevel = 0.5f;
};

/** Structure generation parameters */
USTRUCT(BlueprintType)
struct FStructureGenerationParams
{
    GENERATED_BODY()

    /** Structure type to generate */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    EStructureType StructureType = EStructureType::PrimitiveShelter;

    /** Size category (0=small, 1=medium, 2=large) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    int32 SizeCategory = 1;

    /** Construction quality level (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    float ConstructionQuality = 0.7f;

    /** Age of structure in days */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    float StructureAge = 30.0f;

    /** Maintenance level (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    float MaintenanceLevel = 0.6f;

    /** Primary construction material */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    EConstructionMaterial PrimaryMaterial = EConstructionMaterial::Wood;

    /** Secondary construction material */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    EConstructionMaterial SecondaryMaterial = EConstructionMaterial::Plant;

    /** Current structural condition */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    EStructuralCondition StructuralCondition = EStructuralCondition::WellMaintained;

    /** Randomization seed */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    int32 RandomSeed = 12345;

    /** Include interior furnishing? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    bool bIncludeInterior = true;

    /** Interior story theme */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    FString InteriorStoryTheme = "Family Shelter";

    /** Number of inhabitants suggested by interior */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    int32 InhabitantCount = 3;
};

/** Generated structure data */
USTRUCT(BlueprintType)
struct FGeneratedStructureData
{
    GENERATED_BODY()

    /** Unique structure identifier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    FGuid StructureID;

    /** Structure type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    EStructureType StructureType = EStructureType::PrimitiveShelter;

    /** World transform */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    FTransform WorldTransform;

    /** Bounding box of structure */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    FBox StructureBounds;

    /** Generated building components */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    TArray<FBuildingComponent> BuildingComponents;

    /** Generated interior props */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    TArray<FInteriorPropConfig> InteriorProps;

    /** Placement context used for generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    FStructurePlacementContext PlacementContext;

    /** Generation parameters used */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    FStructureGenerationParams GenerationParams;

    /** Narrative story told by this structure */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure", meta = (MultiLine = true))
    FString StructureNarrative;

    /** Environmental integration notes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure", meta = (MultiLine = true))
    FString EnvironmentalNotes;

    /** Gameplay interaction points */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    TArray<FVector> InteractionPoints;

    /** Resource cache locations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    TMap<FVector, FString> ResourceCaches;
};

/** Structure template for PCG generation */
USTRUCT(BlueprintType)
struct FStructureTemplate
{
    GENERATED_BODY()

    /** Template name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    FString TemplateName = "Basic Shelter";

    /** Structure type this template generates */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    EStructureType StructureType = EStructureType::PrimitiveShelter;

    /** Required building components */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    TArray<FBuildingComponent> RequiredComponents;

    /** Optional building components */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    TArray<FBuildingComponent> OptionalComponents;

    /** Interior prop templates */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    TArray<FInteriorPropConfig> InteriorPropTemplates;

    /** Biome suitability scores */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    TMap<EBiomeType, float> BiomeSuitability;

    /** Placement requirements */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    float MinWaterDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    float MaxWaterDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    float MinElevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    float MaxElevation = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    float MaxSlope = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    float PreferredVegetationDensity = 0.5f;

    /** Construction time in game hours */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    float ConstructionTime = 8.0f;

    /** Required materials and quantities */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    TMap<EConstructionMaterial, int32> RequiredMaterials;

    /** Narrative themes this template supports */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    TArray<FString> SupportedNarrativeThemes;
};

/**
 * @brief Structure Generator Component
 * 
 * Handles procedural generation of prehistoric structures using PCG Framework.
 * Integrates with biome system to create contextually appropriate buildings.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UStructureGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UStructureGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Generate structures in specified area using PCG */
    UFUNCTION(BlueprintCallable, Category = "Structure Generation")
    TArray<FGeneratedStructureData> GenerateStructuresInArea(
        const FVector& AreaCenter,
        float AreaRadius,
        int32 MaxStructures = 10,
        const TArray<EStructureType>& AllowedTypes = {}
    );

    /** Generate single structure at specific location */
    UFUNCTION(BlueprintCallable, Category = "Structure Generation")
    FGeneratedStructureData GenerateStructureAtLocation(
        const FVector& Location,
        const FStructureGenerationParams& Params
    );

    /** Analyze location suitability for structure type */
    UFUNCTION(BlueprintCallable, Category = "Structure Generation")
    float AnalyzeLocationSuitability(
        const FVector& Location,
        EStructureType StructureType
    );

    /** Get placement context for location */
    UFUNCTION(BlueprintCallable, Category = "Structure Generation")
    FStructurePlacementContext GetPlacementContext(const FVector& Location);

    /** Update structure condition based on time and weather */
    UFUNCTION(BlueprintCallable, Category = "Structure Maintenance")
    void UpdateStructureCondition(
        UPARAM(ref) FGeneratedStructureData& StructureData,
        float DeltaTime,
        float WeatherIntensity = 0.5f
    );

    /** Repair structure (player action) */
    UFUNCTION(BlueprintCallable, Category = "Structure Maintenance")
    bool RepairStructure(
        UPARAM(ref) FGeneratedStructureData& StructureData,
        const TMap<EConstructionMaterial, int32>& AvailableMaterials
    );

    /** Generate interior narrative and props */
    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    void GenerateInteriorNarrative(
        UPARAM(ref) FGeneratedStructureData& StructureData,
        const FString& NarrativeTheme = "Abandoned Family Home"
    );

    /** Spawn structure in world from data */
    UFUNCTION(BlueprintCallable, Category = "Structure Spawning")
    AActor* SpawnStructureInWorld(
        const FGeneratedStructureData& StructureData,
        UWorld* TargetWorld = nullptr
    );

    /** Remove structure from world */
    UFUNCTION(BlueprintCallable, Category = "Structure Spawning")
    void RemoveStructureFromWorld(const FGuid& StructureID);

protected:
    /** PCG Component for structure generation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TObjectPtr<UPCGComponent> PCGComponent;

    /** Structure templates database */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Templates")
    TArray<FStructureTemplate> StructureTemplates;

    /** Default building components */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<FBuildingComponent> DefaultBuildingComponents;

    /** Default interior props */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<FInteriorPropConfig> DefaultInteriorProps;

    /** Material library for different biomes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TMap<EBiomeType, TArray<TSoftObjectPtr<UMaterialInterface>>> BiomeMaterials;

    /** Mesh library for building components */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Meshes")
    TMap<EConstructionMaterial, TArray<TSoftObjectPtr<UStaticMesh>>> ComponentMeshes;

    /** Generated structures registry */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Registry")
    TMap<FGuid, FGeneratedStructureData> GeneratedStructures;

    /** Spawned structure actors */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Registry")
    TMap<FGuid, TWeakObjectPtr<AActor>> SpawnedStructures;

private:
    /** Initialize structure templates */
    void InitializeStructureTemplates();

    /** Initialize building components */
    void InitializeBuildingComponents();

    /** Initialize interior props */
    void InitializeInteriorProps();

    /** Select appropriate template for context */
    FStructureTemplate SelectTemplateForContext(
        EStructureType StructureType,
        const FStructurePlacementContext& Context
    );

    /** Generate building components for structure */
    TArray<FBuildingComponent> GenerateBuildingComponents(
        const FStructureTemplate& Template,
        const FStructureGenerationParams& Params,
        const FStructurePlacementContext& Context
    );

    /** Generate interior props for structure */
    TArray<FInteriorPropConfig> GenerateInteriorProps(
        const FStructureTemplate& Template,
        const FStructureGenerationParams& Params,
        const FStructurePlacementContext& Context
    );

    /** Apply weathering to structure */
    void ApplyWeathering(
        UPARAM(ref) FGeneratedStructureData& StructureData,
        float WeatheringAmount
    );

    /** Calculate structural integrity */
    float CalculateStructuralIntegrity(const FGeneratedStructureData& StructureData);

    /** Generate narrative story for structure */
    FString GenerateStructureStory(
        const FStructureTemplate& Template,
        const FStructureGenerationParams& Params,
        const FStructurePlacementContext& Context
    );
};