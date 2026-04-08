// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerVolume.h"
#include "../WorldGeneration/BiomeSystem.h"
#include "ArchitectureSystem.generated.h"

/**
 * @brief Architecture System for Transpersonal Game Studio
 * 
 * Every structure is a document of human civilization. Every interior tells the story
 * of who lived there, what they feared, what they needed. This system creates
 * architectural archaeology — buildings that feel lived-in, abandoned with purpose,
 * weathered by time and elements.
 * 
 * Core Philosophy (Stewart Brand + Gaston Bachelard):
 * - Buildings learn over time — they show their history in layers
 * - Space has memory — every room remembers its inhabitants
 * - No empty interiors — every space shows signs of use and abandonment
 * - Architecture as environmental storytelling
 * 
 * Structure Types:
 * - Primitive Shelters: Basic survival structures (lean-tos, windbreaks)
 * - Cave Dwellings: Natural caves modified for habitation
 * - Elevated Platforms: Tree houses and raised structures for safety
 * - Stone Circles: Ritual/gathering spaces
 * - Burial Mounds: Memorial structures
 * - Tool Caches: Hidden storage areas
 * - Observation Posts: Elevated lookout points
 * - Defensive Structures: Barriers and traps
 * 
 * Technical Features:
 * - Procedural structure generation using PCG Framework
 * - Modular building components for variety
 * - Interior furnishing system with narrative props
 * - Weathering and aging materials
 * - Structural integrity simulation
 * - Integration with biome-specific materials
 * - Nanite support for detailed architecture
 * - Performance optimization through LOD and culling
 * 
 * @author Architecture & Interior Agent — Agent #7
 * @version 1.0 — March 2026
 */

/** Primitive structure types found in prehistoric world */
UENUM(BlueprintType)
enum class EStructureType : uint8
{
    PrimitiveShelter    UMETA(DisplayName = "Primitive Shelter"),
    CaveDwelling        UMETA(DisplayName = "Cave Dwelling"),
    ElevatedPlatform    UMETA(DisplayName = "Elevated Platform"),
    StoneCircle         UMETA(DisplayName = "Stone Circle"),
    BurialMound         UMETA(DisplayName = "Burial Mound"),
    ToolCache           UMETA(DisplayName = "Tool Cache"),
    ObservationPost     UMETA(DisplayName = "Observation Post"),
    DefensiveBarrier    UMETA(DisplayName = "Defensive Barrier"),
    FoodStorage         UMETA(DisplayName = "Food Storage"),
    WaterCollection     UMETA(DisplayName = "Water Collection"),
    FirePit             UMETA(DisplayName = "Fire Pit"),
    WorkArea            UMETA(DisplayName = "Work Area"),
    SleepingArea        UMETA(DisplayName = "Sleeping Area"),
    RitualSite          UMETA(DisplayName = "Ritual Site"),
    TrapSystem          UMETA(DisplayName = "Trap System")
};

/** Construction materials available in prehistoric times */
UENUM(BlueprintType)
enum class EConstructionMaterial : uint8
{
    Wood                UMETA(DisplayName = "Wood"),
    Stone               UMETA(DisplayName = "Stone"),
    Bone                UMETA(DisplayName = "Bone"),
    Hide                UMETA(DisplayName = "Animal Hide"),
    Plant               UMETA(DisplayName = "Plant Fiber"),
    Mud                 UMETA(DisplayName = "Mud/Clay"),
    Thatch              UMETA(DisplayName = "Thatch"),
    Bamboo              UMETA(DisplayName = "Bamboo"),
    Vine                UMETA(DisplayName = "Vine"),
    Bark                UMETA(DisplayName = "Tree Bark"),
    Reed                UMETA(DisplayName = "Reed"),
    Moss                UMETA(DisplayName = "Moss")
};

/** Structural condition states */
UENUM(BlueprintType)
enum class EStructuralCondition : uint8
{
    Perfect             UMETA(DisplayName = "Perfect Condition"),
    WellMaintained      UMETA(DisplayName = "Well Maintained"),
    SlightlyWorn        UMETA(DisplayName = "Slightly Worn"),
    Weathered           UMETA(DisplayName = "Weathered"),
    Damaged             UMETA(DisplayName = "Damaged"),
    Deteriorating       UMETA(DisplayName = "Deteriorating"),
    Ruined              UMETA(DisplayName = "Ruined"),
    Collapsed           UMETA(DisplayName = "Collapsed")
};

/** Interior prop types that tell stories */
UENUM(BlueprintType)
enum class EInteriorPropType : uint8
{
    SleepingFur         UMETA(DisplayName = "Sleeping Fur"),
    StoneTools          UMETA(DisplayName = "Stone Tools"),
    FoodRemains         UMETA(DisplayName = "Food Remains"),
    WaterContainer      UMETA(DisplayName = "Water Container"),
    FireAsh             UMETA(DisplayName = "Fire Ash"),
    BoneNeedles         UMETA(DisplayName = "Bone Needles"),
    PlantMedicine       UMETA(DisplayName = "Plant Medicine"),
    HuntingGear         UMETA(DisplayName = "Hunting Gear"),
    CookingStones       UMETA(DisplayName = "Cooking Stones"),
    PersonalOrnaments   UMETA(DisplayName = "Personal Ornaments"),
    ChildToys           UMETA(DisplayName = "Child Toys"),
    RitualObjects       UMETA(DisplayName = "Ritual Objects"),
    WeaponCache         UMETA(DisplayName = "Weapon Cache"),
    FoodStorage         UMETA(DisplayName = "Food Storage"),
    ClothingRemnants    UMETA(DisplayName = "Clothing Remnants")
};

/** Building component for modular construction */
USTRUCT(BlueprintType)
struct FBuildingComponent
{
    GENERATED_BODY()

    /** Component name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
    FString ComponentName;

    /** Static mesh for this component */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
    TSoftObjectPtr<UStaticMesh> ComponentMesh;

    /** Material for this component */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
    TSoftObjectPtr<UMaterialInterface> ComponentMaterial;

    /** Relative transform from structure origin */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FTransform RelativeTransform;

    /** Required construction material */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Construction")
    EConstructionMaterial RequiredMaterial = EConstructionMaterial::Wood;

    /** Material quantity needed */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Construction", meta = (ClampMin = "1", ClampMax = "100"))
    int32 MaterialQuantity = 5;

    /** Structural importance (affects collapse behavior) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StructuralImportance = 0.5f;

    /** Durability against weather */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Durability", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherResistance = 0.5f;

    /** Durability against dinosaur damage */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Durability", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DinosaurResistance = 0.3f;

    /** Can this component be repaired? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maintenance")
    bool bCanBeRepaired = true;

    /** Time to repair this component (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maintenance", meta = (ClampMin = "1.0"))
    float RepairTime = 30.0f;
};

/** Interior prop configuration */
USTRUCT(BlueprintType)
struct FInteriorPropConfig
{
    GENERATED_BODY()

    /** Prop type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    EInteriorPropType PropType = EInteriorPropType::SleepingFur;

    /** Prop mesh */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    TSoftObjectPtr<UStaticMesh> PropMesh;

    /** Prop material variations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;

    /** Narrative story this prop tells */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling", meta = (MultiLine = true))
    FString NarrativeStory;

    /** Spawn probability in appropriate structures */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnProbability = 0.5f;

    /** Preferred placement areas within structure */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<FString> PreferredAreas;

    /** Scale variation range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    /** Rotation variation (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation", meta = (ClampMin = "0.0", ClampMax = "360.0"))
    float RotationVariation = 45.0f;

    /** Age/weathering level */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatheringLevel = 0.5f;

    /** Can player interact with this prop? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool bIsInteractable = false;

    /** Resources obtained from interaction */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    TMap<FString, int32> InteractionRewards;

    /** Interaction description for UI */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    FText InteractionDescription;
};

/** Structure configuration data */
USTRUCT(BlueprintType)
struct FStructureConfig
{
    GENERATED_BODY()

    /** Structure type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    EStructureType StructureType = EStructureType::PrimitiveShelter;

    /** Human-readable structure name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    FText StructureName;

    /** Historical/narrative description */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling", meta = (MultiLine = true))
    FString HistoricalDescription;

    /** Building components that make up this structure */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Construction")
    TArray<FBuildingComponent> BuildingComponents;

    /** Interior props for this structure type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interior")
    TArray<FInteriorPropConfig> InteriorProps;

    /** Preferred biomes for this structure */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<EBiomeType> PreferredBiomes;

    /** Minimum distance from other structures */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "100.0"))
    float MinDistanceFromOthers = 500.0f;

    /** Spawn probability per km² */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnProbability = 0.05f;

    /** Preferred elevation range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    FVector2D ElevationRange = FVector2D(0.0f, 1000.0f);

    /** Slope tolerance for placement */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    FVector2D SlopeRange = FVector2D(0.0f, 30.0f);

    /** Proximity to water preference */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaterProximityPreference = 0.5f;

    /** Shelter value (protection from weather) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ShelterValue = 0.7f;

    /** Security value (protection from predators) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SecurityValue = 0.5f;

    /** Storage capacity (number of items) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "0", ClampMax = "100"))
    int32 StorageCapacity = 10;

    /** Construction time required (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Construction", meta = (ClampMin = "60.0"))
    float ConstructionTime = 300.0f;

    /** Maintenance interval (game days) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maintenance", meta = (ClampMin = "1.0"))
    float MaintenanceInterval = 7.0f;

    /** Structural integrity (affects collapse) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StructuralIntegrity = 1.0f;

    /** Degradation rate per day */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maintenance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DegradationRate = 0.01f;
};

/** Weathering effect configuration */
USTRUCT(BlueprintType)
struct FWeatheringEffect
{
    GENERATED_BODY()

    /** Effect name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weathering")
    FString EffectName;

    /** Material parameter to modify */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weathering")
    FString MaterialParameter;

    /** Weathering intensity (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weathering", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatheringIntensity = 0.5f;

    /** Affected construction materials */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weathering")
    TArray<EConstructionMaterial> AffectedMaterials;

    /** Weather conditions that cause this effect */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    TArray<FString> CausingWeatherConditions;

    /** Rate of effect application */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weathering", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EffectRate = 0.1f;
};

/**
 * @brief Architecture Data Asset
 * 
 * Defines architectural styles and construction methods for prehistoric structures.
 * Each asset represents a complete building system with components, materials,
 * and narrative elements.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArchitectureData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UArchitectureData();

    /** Architecture style name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture Info")
    FText ArchitectureName;

    /** Cultural/historical description */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture Info", meta = (MultiLine = true))
    FString CulturalDescription;

    /** Time period this architecture represents */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture Info")
    FString TimePeriod;

    /** Available structure types in this architectural style */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structures")
    TArray<FStructureConfig> AvailableStructures;

    /** Common construction materials for this style */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<EConstructionMaterial> CommonMaterials;

    /** Weathering effects for this architectural style */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weathering")
    TArray<FWeatheringEffect> WeatheringEffects;

    /** Biomes where this architecture is commonly found */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Distribution")
    TArray<EBiomeType> NativeBiomes;

    /** Overall condition range for structures of this style */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
    FVector2D ConditionRange = FVector2D(0.3f, 0.8f);

    /** Abandonment probability (structures found empty) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AbandonmentProbability = 0.7f;

    /** Signs of struggle probability (damaged/destroyed structures) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StruggleProbability = 0.3f;
};

/**
 * @brief Architecture System Subsystem
 * 
 * Manages the procedural placement and generation of prehistoric structures
 * throughout the world. Integrates with the biome system and environment art
 * to create cohesive architectural storytelling.
 */
UCLASS()
class TRANSPERSONALGAME_API UArchitectureSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UArchitectureSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Generate structures for a specific biome area */
    UFUNCTION(BlueprintCallable, Category = "Architecture System")
    void GenerateStructuresForBiome(EBiomeType BiomeType, const FVector& AreaCenter, float AreaRadius);

    /** Place a specific structure at a location */
    UFUNCTION(BlueprintCallable, Category = "Architecture System")
    class AStructureActor* PlaceStructure(const FStructureConfig& StructureConfig, const FVector& Location, float Rotation = 0.0f);

    /** Weather a structure over time */
    UFUNCTION(BlueprintCallable, Category = "Architecture System")
    void ApplyWeathering(class AStructureActor* Structure, float WeatheringAmount);

    /** Repair a damaged structure */
    UFUNCTION(BlueprintCallable, Category = "Architecture System")
    bool RepairStructure(class AStructureActor* Structure, const TArray<EConstructionMaterial>& AvailableMaterials);

    /** Get suitable structure types for a biome */
    UFUNCTION(BlueprintPure, Category = "Architecture System")
    TArray<FStructureConfig> GetStructureTypesForBiome(EBiomeType BiomeType) const;

    /** Check if location is suitable for structure placement */
    UFUNCTION(BlueprintPure, Category = "Architecture System")
    bool IsLocationSuitableForStructure(const FVector& Location, const FStructureConfig& StructureConfig) const;

protected:
    /** Available architecture data assets */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<TSoftObjectPtr<UArchitectureData>> ArchitectureDataAssets;

    /** Maximum structures per km² */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MaxStructuresPerKm2 = 5.0f;

    /** Minimum distance between structures */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation", meta = (ClampMin = "100.0"))
    float MinStructureDistance = 1000.0f;

    /** Enable structure weathering over time */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation")
    bool bEnableWeathering = true;

    /** Weathering update interval (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation", meta = (ClampMin = "60.0"))
    float WeatheringUpdateInterval = 3600.0f;

    /** Enable structural integrity simulation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation")
    bool bEnableStructuralIntegrity = true;

private:
    /** Timer for weathering updates */
    FTimerHandle WeatheringTimer;

    /** Currently spawned structures */
    UPROPERTY()
    TArray<TWeakObjectPtr<class AStructureActor>> SpawnedStructures;

    /** Cached architecture data */
    UPROPERTY()
    TArray<UArchitectureData*> LoadedArchitectureData;

    /** Update weathering for all structures */
    void UpdateWeathering();

    /** Load architecture data assets */
    void LoadArchitectureData();

    /** Calculate structure placement score */
    float CalculatePlacementScore(const FVector& Location, const FStructureConfig& StructureConfig) const;
};