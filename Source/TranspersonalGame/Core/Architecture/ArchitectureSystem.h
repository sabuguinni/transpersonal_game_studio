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

/** Inhabitant types that lived in structures */
UENUM(BlueprintType)
enum class EInhabitantType : uint8
{
    SolitaryHunter      UMETA(DisplayName = "Solitary Hunter"),
    SmallFamily         UMETA(DisplayName = "Small Family"),
    HunterGroup         UMETA(DisplayName = "Hunter Group"),
    Shaman              UMETA(DisplayName = "Shaman"),
    Craftsperson        UMETA(DisplayName = "Craftsperson"),
    Elder               UMETA(DisplayName = "Elder"),
    ChildGroup          UMETA(DisplayName = "Child Group"),
    WarriorBand         UMETA(DisplayName = "Warrior Band"),
    Gatherers           UMETA(DisplayName = "Gatherers"),
    Refugees            UMETA(DisplayName = "Refugees"),
    Outcasts            UMETA(DisplayName = "Outcasts"),
    Traders             UMETA(DisplayName = "Traders")
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

    /** Collision component for interaction */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    bool bHasCollision = true;

    /** Component can be destroyed */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction")
    bool bCanBeDestroyed = true;

    /** Health points for destruction */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction", meta = (ClampMin = "1.0"))
    float HealthPoints = 100.0f;
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
    TMap<FString, int32> InteractionResources;

    /** Interaction text displayed to player */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (MultiLine = true))
    FString InteractionText;

    /** Sound played when interacting */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> InteractionSound;

    /** Particle effect when interacting */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TSoftObjectPtr<UParticleSystem> InteractionVFX;
};

/** Structure blueprint defining complete building */
USTRUCT(BlueprintType)
struct FStructureBlueprint
{
    GENERATED_BODY()

    /** Structure type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    EStructureType StructureType = EStructureType::PrimitiveShelter;

    /** Structure name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    FString StructureName;

    /** Description of the structure */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure", meta = (MultiLine = true))
    FString StructureDescription;

    /** Building components that make up this structure */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<FBuildingComponent> BuildingComponents;

    /** Interior props for this structure */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interior")
    TArray<FInteriorPropConfig> InteriorProps;

    /** Who lived here? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    EInhabitantType FormerInhabitant = EInhabitantType::SmallFamily;

    /** Story of what happened here */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (MultiLine = true))
    FString StructureStory;

    /** How long was it inhabited? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (ClampMin = "1", ClampMax = "365"))
    int32 InhabitationDays = 30;

    /** How long has it been abandoned? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (ClampMin = "0", ClampMax = "3650"))
    int32 AbandonedDays = 90;

    /** Reason for abandonment */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    FString AbandonmentReason;

    /** Preferred biomes for this structure */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<EBiomeType> PreferredBiomes;

    /** Minimum distance from water (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float MinWaterDistance = 500.0f;

    /** Maximum distance from water (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float MaxWaterDistance = 5000.0f;

    /** Elevation preference (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D ElevationRange = FVector2D(0.0f, 2000.0f);

    /** Slope tolerance (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D SlopeRange = FVector2D(0.0f, 30.0f);

    /** Structure footprint size (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dimensions")
    FVector2D FootprintSize = FVector2D(400.0f, 400.0f);

    /** Structure height (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dimensions")
    float StructureHeight = 250.0f;

    /** Interior space definition */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interior")
    TArray<FString> InteriorAreas;

    /** Current structural condition */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
    EStructuralCondition CurrentCondition = EStructuralCondition::Weathered;

    /** Fire damage level */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FireDamage = 0.0f;

    /** Water damage level */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaterDamage = 0.2f;

    /** Animal damage level */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AnimalDamage = 0.1f;

    /** Wind damage level */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WindDamage = 0.3f;

    /** Can player use this structure for shelter? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bProvidesShel ter = true;

    /** Protection from weather (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherProtection = 0.7f;

    /** Protection from dinosaurs (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DinosaurProtection = 0.4f;

    /** Can player repair this structure? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bCanBeRepaired = true;

    /** Resources required for full repair */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    TMap<EConstructionMaterial, int32> RepairResources;

    /** Time to fully repair (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "60.0"))
    float RepairTime = 300.0f;

    /** Spawn probability in world */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnProbability = 0.1f;

    /** Minimum distance between structures of same type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    float MinSpacing = 2000.0f;

    /** Can form clusters with other structures */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    bool bCanFormClusters = false;

    /** Cluster size range */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    FVector2D ClusterSize = FVector2D(1.0f, 3.0f);
};

/** Data asset containing structure blueprints */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UStructureBlueprintLibrary : public UDataAsset
{
    GENERATED_BODY()

public:
    /** All available structure blueprints */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structures")
    TArray<FStructureBlueprint> StructureBlueprints;

    /** Get structures by type */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FStructureBlueprint> GetStructuresByType(EStructureType StructureType) const;

    /** Get structures suitable for biome */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FStructureBlueprint> GetStructuresForBiome(EBiomeType BiomeType) const;

    /** Get structures by inhabitant type */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FStructureBlueprint> GetStructuresByInhabitant(EInhabitantType InhabitantType) const;
};

/** Architecture System Subsystem */
UCLASS()
class TRANSPERSONALGAME_API UArchitectureSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Initialize architecture system */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeArchitectureSystem();

    /** Generate structures for a region */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateStructuresForRegion(const FVector& RegionCenter, float RegionRadius, EBiomeType BiomeType);

    /** Spawn a specific structure at location */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    class AStructureActor* SpawnStructure(const FStructureBlueprint& Blueprint, const FVector& Location, const FRotator& Rotation);

    /** Get structure blueprint by name */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool GetStructureBlueprintByName(const FString& StructureName, FStructureBlueprint& OutBlueprint) const;

    /** Update structure condition over time */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructureConditions(float DeltaTime);

    /** Repair structure */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RepairStructure(class AStructureActor* Structure, const TMap<EConstructionMaterial, int32>& AvailableResources);

    /** Get all structures in radius */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<class AStructureActor*> GetStructuresInRadius(const FVector& Center, float Radius) const;

    /** Check if location is suitable for structure type */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationSuitableForStructure(const FVector& Location, const FStructureBlueprint& Blueprint) const;

    /** Get narrative information about structure */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FString GetStructureNarrative(const class AStructureActor* Structure) const;

protected:
    /** Structure blueprint library */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    TSoftObjectPtr<UStructureBlueprintLibrary> StructureLibrary;

    /** Active structures in world */
    UPROPERTY()
    TArray<class AStructureActor*> ActiveStructures;

    /** Structure generation settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    float StructureDensity = 0.1f;

    /** Maximum structures per region */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    int32 MaxStructuresPerRegion = 10;

    /** Minimum structure spacing */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    float MinStructureSpacing = 1000.0f;

    /** Structure decay rate per day */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation")
    float StructureDecayRate = 0.01f;

    /** Weather damage multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation")
    float WeatherDamageMultiplier = 1.0f;

private:
    /** Generate interior props for structure */
    void GenerateInteriorProps(class AStructureActor* Structure, const FStructureBlueprint& Blueprint);

    /** Apply weathering to structure */
    void ApplyWeathering(class AStructureActor* Structure, float WeatheringAmount);

    /** Calculate structure stability */
    float CalculateStructureStability(const class AStructureActor* Structure) const;

    /** Find suitable spawn locations */
    TArray<FVector> FindSuitableSpawnLocations(const FVector& RegionCenter, float RegionRadius, const FStructureBlueprint& Blueprint, int32 MaxLocations) const;

    /** Validate structure placement */
    bool ValidateStructurePlacement(const FVector& Location, const FStructureBlueprint& Blueprint) const;
};

/** Delegate for structure events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStructureEvent, class AStructureActor*, Structure, const FString&, EventType);

/** Architecture Manager Component */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArchitectureManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchitectureManagerComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Structure event delegate */
    UPROPERTY(BlueprintAssignable, Category = "Architecture")
    FOnStructureEvent OnStructureEvent;

    /** Register structure with manager */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterStructure(class AStructureActor* Structure);

    /** Unregister structure from manager */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UnregisterStructure(class AStructureActor* Structure);

    /** Update all managed structures */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateManagedStructures(float DeltaTime);

    /** Get nearest structure to location */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    class AStructureActor* GetNearestStructure(const FVector& Location) const;

    /** Get structures by type */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<class AStructureActor*> GetStructuresByType(EStructureType StructureType) const;

protected:
    /** Managed structures */
    UPROPERTY()
    TArray<class AStructureActor*> ManagedStructures;

    /** Update frequency for structure conditions */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float UpdateFrequency = 5.0f;

    /** Time since last update */
    float TimeSinceLastUpdate = 0.0f;
};