// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataAsset.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"
#include "../WorldGeneration/ProceduralWorldGenerator.h"
#include "ArchitectureManager.generated.h"

/**
 * @brief Architecture & Interior Manager for the Jurassic Survival Game
 * 
 * Creates every structure built by human hands in the prehistoric world.
 * Each building is a document of the civilization that created it.
 * Every interior tells the story of who lived there and what happened to them.
 * 
 * Core Philosophy (Stewart Brand + Gaston Bachelard):
 * - Buildings are layers of time - each structure shows how it was used and changed
 * - Inhabited space has memory - rooms remember their occupants through objects and wear
 * - No empty interiors - every space shows evidence of human presence and activity
 * - Architectural archaeology - each structure is an artifact waiting to be discovered
 * 
 * Structure Types:
 * - Primitive shelters (lean-tos, windbreaks, temporary camps)
 * - Permanent dwellings (huts, cabins, underground shelters)
 * - Community structures (gathering halls, storage pits, workshops)
 * - Defensive structures (palisades, watchtowers, hidden bunkers)
 * - Ceremonial sites (stone circles, burial mounds, ritual platforms)
 * - Abandoned ruins (collapsed buildings, overgrown settlements)
 * 
 * Interior Storytelling System:
 * - Personal belongings scattered by urgency of departure
 * - Tool marks on walls showing daily activities
 * - Wear patterns on floors revealing movement habits
 * - Food storage showing dietary patterns and seasons
 * - Sleeping arrangements revealing social structure
 * - Defensive modifications showing threat responses
 * 
 * Technical Implementation:
 * - Modular building system for flexible construction
 * - Procedural interior decoration based on inhabitant profiles
 * - Material aging and weathering systems
 * - Structural damage simulation for abandoned buildings
 * - Interactive object placement for environmental storytelling
 * - Performance optimization through instancing and LODs
 * 
 * @author Architecture & Interior Agent — Agent #7
 * @version 1.0 — March 2026
 */

/** Types of prehistoric human structures */
UENUM(BlueprintType)
enum class EStructureType : uint8
{
    // Temporary Shelters
    LeanToShelter       UMETA(DisplayName = "Lean-To Shelter"),
    Windbreak           UMETA(DisplayName = "Windbreak"),
    TemporaryCamp       UMETA(DisplayName = "Temporary Camp"),
    HuntingBlind        UMETA(DisplayName = "Hunting Blind"),
    
    // Permanent Dwellings
    PrimitiveHut        UMETA(DisplayName = "Primitive Hut"),
    LogCabin            UMETA(DisplayName = "Log Cabin"),
    StoneHouse          UMETA(DisplayName = "Stone House"),
    UndergroundShelter  UMETA(DisplayName = "Underground Shelter"),
    TreeHouse           UMETA(DisplayName = "Tree House"),
    CliffDwelling       UMETA(DisplayName = "Cliff Dwelling"),
    
    // Community Structures
    GatheringHall       UMETA(DisplayName = "Gathering Hall"),
    StoragePit          UMETA(DisplayName = "Storage Pit"),
    Workshop            UMETA(DisplayName = "Workshop"),
    Smokehouse          UMETA(DisplayName = "Smokehouse"),
    ToolShed            UMETA(DisplayName = "Tool Shed"),
    CommunalKitchen     UMETA(DisplayName = "Communal Kitchen"),
    
    // Defensive Structures
    Palisade            UMETA(DisplayName = "Palisade"),
    Watchtower          UMETA(DisplayName = "Watchtower"),
    HiddenBunker        UMETA(DisplayName = "Hidden Bunker"),
    TrapPit             UMETA(DisplayName = "Trap Pit"),
    Barricade           UMETA(DisplayName = "Barricade"),
    
    // Ceremonial Sites
    StoneCircle         UMETA(DisplayName = "Stone Circle"),
    BurialMound         UMETA(DisplayName = "Burial Mound"),
    RitualPlatform      UMETA(DisplayName = "Ritual Platform"),
    SacredGrove         UMETA(DisplayName = "Sacred Grove"),
    OfferingAltar       UMETA(DisplayName = "Offering Altar"),
    
    // Ruins and Abandoned
    CollapsedHut        UMETA(DisplayName = "Collapsed Hut"),
    OvergrownRuins      UMETA(DisplayName = "Overgrown Ruins"),
    BurnedStructure     UMETA(DisplayName = "Burned Structure"),
    FloodedRuins        UMETA(DisplayName = "Flooded Ruins"),
    AbandonedCamp       UMETA(DisplayName = "Abandoned Camp")
};

/** Construction materials used in prehistoric times */
UENUM(BlueprintType)
enum class EConstructionMaterial : uint8
{
    // Organic Materials
    Wood                UMETA(DisplayName = "Wood"),
    Bark                UMETA(DisplayName = "Bark"),
    Leaves              UMETA(DisplayName = "Leaves"),
    Grass               UMETA(DisplayName = "Grass"),
    Vines               UMETA(DisplayName = "Vines"),
    AnimalHides         UMETA(DisplayName = "Animal Hides"),
    Bones               UMETA(DisplayName = "Bones"),
    
    // Mineral Materials
    Stone               UMETA(DisplayName = "Stone"),
    Clay                UMETA(DisplayName = "Clay"),
    Mud                 UMETA(DisplayName = "Mud"),
    Sand                UMETA(DisplayName = "Sand"),
    Flint               UMETA(DisplayName = "Flint"),
    
    // Composite Materials
    WattleAndDaub       UMETA(DisplayName = "Wattle and Daub"),
    ThatchRoof          UMETA(DisplayName = "Thatch Roof"),
    LogAndChink         UMETA(DisplayName = "Log and Chink"),
    StoneAndMortar      UMETA(DisplayName = "Stone and Mortar")
};

/** Condition states for structures */
UENUM(BlueprintType)
enum class EStructureCondition : uint8
{
    Pristine            UMETA(DisplayName = "Pristine"),
    WellMaintained      UMETA(DisplayName = "Well Maintained"),
    SlightWear          UMETA(DisplayName = "Slight Wear"),
    Weathered           UMETA(DisplayName = "Weathered"),
    Damaged             UMETA(DisplayName = "Damaged"),
    Deteriorating       UMETA(DisplayName = "Deteriorating"),
    Ruined              UMETA(DisplayName = "Ruined"),
    Collapsed           UMETA(DisplayName = "Collapsed")
};

/** Inhabitant types that affect interior design */
UENUM(BlueprintType)
enum class EInhabitantType : uint8
{
    // Individual Types
    SolitaryHunter      UMETA(DisplayName = "Solitary Hunter"),
    Gatherer            UMETA(DisplayName = "Gatherer"),
    Toolmaker           UMETA(DisplayName = "Toolmaker"),
    Shaman              UMETA(DisplayName = "Shaman"),
    Elder               UMETA(DisplayName = "Elder"),
    
    // Family Units
    SmallFamily         UMETA(DisplayName = "Small Family"),
    LargeFamily         UMETA(DisplayName = "Large Family"),
    ExtendedFamily      UMETA(DisplayName = "Extended Family"),
    
    // Community Groups
    HuntingParty        UMETA(DisplayName = "Hunting Party"),
    TradingGroup        UMETA(DisplayName = "Trading Group"),
    Tribe               UMETA(DisplayName = "Tribe"),
    
    // Special Cases
    Refugees            UMETA(DisplayName = "Refugees"),
    Outcasts            UMETA(DisplayName = "Outcasts"),
    Unknown             UMETA(DisplayName = "Unknown"),
    Abandoned           UMETA(DisplayName = "Abandoned")
};

/** Interior object types for storytelling */
UENUM(BlueprintType)
enum class EInteriorObjectType : uint8
{
    // Survival Essentials
    FirePit             UMETA(DisplayName = "Fire Pit"),
    SleepingFurs        UMETA(DisplayName = "Sleeping Furs"),
    WaterContainer      UMETA(DisplayName = "Water Container"),
    FoodStorage         UMETA(DisplayName = "Food Storage"),
    
    // Tools and Weapons
    StoneTools          UMETA(DisplayName = "Stone Tools"),
    WoodenSpears        UMETA(DisplayName = "Wooden Spears"),
    BoneKnives          UMETA(DisplayName = "Bone Knives"),
    HuntingBow          UMETA(DisplayName = "Hunting Bow"),
    Arrows              UMETA(DisplayName = "Arrows"),
    
    // Crafting Materials
    KnappingStone       UMETA(DisplayName = "Knapping Stone"),
    WoodShavings        UMETA(DisplayName = "Wood Shavings"),
    AnimalBones         UMETA(DisplayName = "Animal Bones"),
    DriedPlants         UMETA(DisplayName = "Dried Plants"),
    
    // Personal Items
    Clothing            UMETA(DisplayName = "Clothing"),
    Jewelry             UMETA(DisplayName = "Jewelry"),
    PersonalTotems      UMETA(DisplayName = "Personal Totems"),
    
    // Evidence of Activities
    CookingRemnants     UMETA(DisplayName = "Cooking Remnants"),
    ToolMarks           UMETA(DisplayName = "Tool Marks"),
    WearPatterns        UMETA(DisplayName = "Wear Patterns"),
    
    // Signs of Departure
    ScatteredBelongings UMETA(DisplayName = "Scattered Belongings"),
    HurriedPacking      UMETA(DisplayName = "Hurried Packing"),
    AbandonedProjects   UMETA(DisplayName = "Abandoned Projects"),
    
    // Mysterious Elements
    StrangeSymbols      UMETA(DisplayName = "Strange Symbols"),
    UnknownObjects      UMETA(DisplayName = "Unknown Objects"),
    HiddenCaches        UMETA(DisplayName = "Hidden Caches")
};

/** Building component data */
USTRUCT(BlueprintType)
struct FBuildingComponent
{
    GENERATED_BODY()

    /** Component mesh */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    TSoftObjectPtr<UStaticMesh> ComponentMesh;

    /** Material for this component */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    TSoftObjectPtr<UMaterialInterface> ComponentMaterial;

    /** Component type name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    FString ComponentName = "Wall";

    /** Snap points for connecting to other components */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    TArray<FVector> SnapPoints;

    /** Compatible connection types */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    TArray<FString> CompatibleConnections;

    /** Size in building grid units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions")
    FIntVector GridSize = FIntVector(1, 1, 1);

    /** Structural integrity value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    /** Weather resistance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability")
    float WeatherResistance = 50.0f;

    /** Construction material type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    EConstructionMaterial MaterialType = EConstructionMaterial::Wood;

    /** Can this component decay over time? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aging")
    bool bCanDecay = true;

    /** Decay rate per year */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aging")
    float DecayRate = 0.1f;
};

/** Interior decoration profile */
USTRUCT(BlueprintType)
struct FInteriorProfile
{
    GENERATED_BODY()

    /** Type of inhabitant */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inhabitant")
    EInhabitantType InhabitantType = EInhabitantType::SmallFamily;

    /** Number of occupants */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inhabitant")
    int32 OccupantCount = 3;

    /** How long was this place inhabited? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    float InhabitationDuration = 2.0f; // years

    /** How long since abandonment? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    float AbandonmentTime = 0.5f; // years

    /** Reason for abandonment */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    FString AbandonmentReason = "Unknown";

    /** Wealth level (affects object quality) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float WealthLevel = 0.5f; // 0-1 scale

    /** Skill level (affects tool sophistication) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SkillLevel = 0.5f; // 0-1 scale

    /** Primary activities performed here */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activities")
    TArray<FString> PrimaryActivities;

    /** Seasonal occupation pattern */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temporal")
    FString SeasonalPattern = "Year-round";

    /** Objects that should be present */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objects")
    TMap<EInteriorObjectType, int32> RequiredObjects;

    /** Objects that might be present */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objects")
    TMap<EInteriorObjectType, float> OptionalObjects; // probability 0-1

    /** Specific story elements to include */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> StoryElements;

    /** Wear patterns to apply */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aging")
    TArray<FString> WearPatterns;
};

/** Structure blueprint data */
USTRUCT(BlueprintType)
struct FStructureBlueprint
{
    GENERATED_BODY()

    /** Structure type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EStructureType StructureType = EStructureType::PrimitiveHut;

    /** Display name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName = "Primitive Hut";

    /** Description */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString Description = "A simple dwelling made from available materials";

    /** Building components required */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    TArray<FBuildingComponent> RequiredComponents;

    /** Optional components */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    TArray<FBuildingComponent> OptionalComponents;

    /** Foundation requirements */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FString FoundationType = "Level Ground";

    /** Minimum terrain slope for placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float MinSlope = 0.0f;

    /** Maximum terrain slope for placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float MaxSlope = 15.0f;

    /** Preferred biomes for this structure */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<EBiomeType> PreferredBiomes;

    /** Distance from water preference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FVector2D WaterDistanceRange = FVector2D(50.0f, 500.0f);

    /** Typical interior profile */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FInteriorProfile TypicalInterior;

    /** Construction difficulty (affects placement frequency) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rarity")
    float ConstructionDifficulty = 1.0f;

    /** Cultural significance (affects placement near other structures) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float CulturalSignificance = 1.0f;
};

/** Interior object asset data */
USTRUCT(BlueprintType)
struct FInteriorObjectAsset
{
    GENERATED_BODY()

    /** Object mesh */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TSoftObjectPtr<UStaticMesh> ObjectMesh;

    /** Material variations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;

    /** Object type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
    EInteriorObjectType ObjectType = EInteriorObjectType::StoneTools;

    /** Object name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
    FString ObjectName = "Stone Tool";

    /** Scale variation range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    /** Rotation variation (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float RotationVariation = 360.0f;

    /** Placement rules */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    TArray<FString> PlacementRules;

    /** Required nearby objects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<EInteriorObjectType> RequiredNearbyObjects;

    /** Incompatible objects (won't place near these) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<EInteriorObjectType> IncompatibleObjects;

    /** Story weight (importance for narrative) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float StoryWeight = 1.0f;

    /** Interaction possibilities */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TArray<FString> PossibleInteractions;

    /** Condition when placed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EStructureCondition InitialCondition = EStructureCondition::WellMaintained;

    /** Can this object decay? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aging")
    bool bCanDecay = true;

    /** Decay rate per year */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aging")
    float DecayRate = 0.2f;
};

/** Main Architecture Manager class */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Generate structures for the world */
    UFUNCTION(BlueprintCallable, Category = "Architecture Generation")
    void GenerateWorldStructures(UWorld* World, const FWorldGenerationParams& WorldParams);

    /** Generate a single structure at specified location */
    UFUNCTION(BlueprintCallable, Category = "Architecture Generation")
    class AStructureActor* GenerateStructure(UWorld* World, EStructureType StructureType, 
        const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    /** Populate interior of existing structure */
    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    void PopulateInterior(class AStructureActor* Structure, const FInteriorProfile& Profile);

    /** Apply aging and weathering to structure */
    UFUNCTION(BlueprintCallable, Category = "Aging System")
    void ApplyAging(class AStructureActor* Structure, float TimeYears);

    /** Find suitable locations for structure type */
    UFUNCTION(BlueprintCallable, Category = "Placement Analysis")
    TArray<FVector> FindSuitableLocations(UWorld* World, EStructureType StructureType, 
        int32 MaxLocations = 10);

    /** Get structure blueprint data */
    UFUNCTION(BlueprintCallable, Category = "Data Access")
    FStructureBlueprint GetStructureBlueprint(EStructureType StructureType) const;

    /** Register custom structure blueprint */
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void RegisterStructureBlueprint(EStructureType StructureType, const FStructureBlueprint& Blueprint);

    /** Get interior object asset data */
    UFUNCTION(BlueprintCallable, Category = "Data Access")
    FInteriorObjectAsset GetInteriorObjectAsset(EInteriorObjectType ObjectType) const;

protected:
    /** Initialize default structure blueprints */
    void InitializeStructureBlueprints();

    /** Initialize default interior object assets */
    void InitializeInteriorObjectAssets();

    /** Create settlement clusters */
    void GenerateSettlements(UWorld* World, const FWorldGenerationParams& WorldParams);

    /** Create isolated structures */
    void GenerateIsolatedStructures(UWorld* World, const FWorldGenerationParams& WorldParams);

    /** Create ruins and abandoned sites */
    void GenerateRuins(UWorld* World, const FWorldGenerationParams& WorldParams);

    /** Analyze terrain suitability for structure placement */
    float AnalyzeTerrainSuitability(UWorld* World, const FVector& Location, 
        EStructureType StructureType) const;

    /** Generate interior layout for structure */
    void GenerateInteriorLayout(class AStructureActor* Structure, const FInteriorProfile& Profile);

    /** Place interior objects based on profile */
    void PlaceInteriorObjects(class AStructureActor* Structure, const FInteriorProfile& Profile);

    /** Apply wear patterns to interior */
    void ApplyWearPatterns(class AStructureActor* Structure, const FInteriorProfile& Profile);

    /** Create environmental storytelling elements */
    void CreateStorytellingElements(class AStructureActor* Structure, const FInteriorProfile& Profile);

private:
    /** Structure blueprint database */
    UPROPERTY()
    TMap<EStructureType, FStructureBlueprint> StructureBlueprints;

    /** Interior object asset database */
    UPROPERTY()
    TMap<EInteriorObjectType, FInteriorObjectAsset> InteriorObjectAssets;

    /** Reference to world generator for biome data */
    UPROPERTY()
    class UProceduralWorldGenerator* WorldGenerator;

    /** PCG component for structure placement */
    UPROPERTY()
    class UPCGComponent* StructurePlacementPCG;

    /** Performance tracking */
    UPROPERTY()
    int32 TotalStructuresGenerated;

    UPROPERTY()
    int32 TotalInteriorObjectsPlaced;

    /** Random stream for consistent generation */
    FRandomStream RandomStream;
};

/** Structure Actor class */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStructureActor : public AActor
{
    GENERATED_BODY()

public:
    AStructureActor();

protected:
    virtual void BeginPlay() override;

public:
    /** Structure type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EStructureType StructureType = EStructureType::PrimitiveHut;

    /** Current condition */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EStructureCondition CurrentCondition = EStructureCondition::WellMaintained;

    /** Interior profile used for this structure */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FInteriorProfile InteriorProfile;

    /** Age of structure in years */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aging")
    float StructureAge = 0.0f;

    /** Time since abandonment */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aging")
    float AbandonmentTime = 0.0f;

    /** Building components */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<class UStaticMeshComponent*> BuildingComponents;

    /** Interior objects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<class UStaticMeshComponent*> InteriorObjects;

    /** Collision component for interior space */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* InteriorBounds;

    /** Apply aging effects */
    UFUNCTION(BlueprintCallable, Category = "Aging")
    void ApplyAgingEffects(float TimeYears);

    /** Get interior bounds */
    UFUNCTION(BlueprintCallable, Category = "Interior")
    FBox GetInteriorBounds() const;

    /** Check if point is inside structure */
    UFUNCTION(BlueprintCallable, Category = "Interior")
    bool IsPointInside(const FVector& Point) const;

    /** Get all interior objects of specific type */
    UFUNCTION(BlueprintCallable, Category = "Interior")
    TArray<class UStaticMeshComponent*> GetInteriorObjectsOfType(EInteriorObjectType ObjectType) const;

protected:
    /** Update visual condition based on age and damage */
    void UpdateVisualCondition();

    /** Apply material aging effects */
    void ApplyMaterialAging(float AgingFactor);

    /** Update structural integrity */
    void UpdateStructuralIntegrity();

private:
    /** Material instances for aging effects */
    UPROPERTY()
    TArray<class UMaterialInstanceDynamic*> DynamicMaterials;

    /** Original materials before aging */
    UPROPERTY()
    TArray<class UMaterialInterface*> OriginalMaterials;

    /** Structural integrity value */
    float StructuralIntegrity = 100.0f;
};