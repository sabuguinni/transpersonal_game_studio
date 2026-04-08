// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "../WorldGeneration/BiomeSystem.h"
#include "../Environment/EnvironmentArtSystem.h"
#include "ArchitectureSystem.generated.h"

/**
 * @brief Architecture & Interior System for Transpersonal Game Studio
 * 
 * Creates every structure built by human hands in the prehistoric world — each building
 * is a document of the civilization that created it. This system is guided by the conviction
 * that architecture is time made visible, and every interior tells the story of who lived there.
 * 
 * Core Philosophy (Stewart Brand + Gaston Bachelard):
 * - Buildings learn and adapt over time — they show layers of use and modification
 * - Every interior has memory — objects placed reveal the habits and fears of inhabitants
 * - No space is ever truly empty — even abandonment tells a story
 * - Architecture follows function, but function follows survival needs
 * 
 * Features:
 * - Procedural building generation with cultural authenticity
 * - Interior furnishing system that tells stories through object placement
 * - Architectural aging and weathering over time
 * - Cultural building styles based on environmental pressures
 * - Ruins and abandoned structures with narrative purpose
 * - Interactive elements that reveal past inhabitants
 * 
 * Technical Implementation:
 * - PCG-based building placement and generation
 * - Modular construction system for flexible architecture
 * - Interior decoration system with narrative logic
 * - Material aging and weathering shaders
 * - Nanite support for detailed architectural elements
 * - Blueprint-based interaction systems for storytelling
 * 
 * @author Architecture & Interior Agent — Agent #7
 * @version 1.0 — March 2026
 */

/** Building types found in the prehistoric world */
UENUM(BlueprintType)
enum class EBuildingType : uint8
{
    ShelterHut          UMETA(DisplayName = "Basic Shelter Hut"),
    LargeDwelling       UMETA(DisplayName = "Large Family Dwelling"),
    StorageStructure    UMETA(DisplayName = "Storage Structure"),
    CommunalHall        UMETA(DisplayName = "Communal Hall"),
    Watchtower          UMETA(DisplayName = "Watchtower"),
    Workshop            UMETA(DisplayName = "Crafting Workshop"),
    SacredSite          UMETA(DisplayName = "Sacred Site"),
    DefensiveWall       UMETA(DisplayName = "Defensive Wall"),
    Bridge              UMETA(DisplayName = "Bridge"),
    AbandonedRuin       UMETA(DisplayName = "Abandoned Ruin"),
    TemporaryOutpost    UMETA(DisplayName = "Temporary Outpost"),
    UndergroundShelter  UMETA(DisplayName = "Underground Shelter")
};

/** Construction materials available in prehistoric times */
UENUM(BlueprintType)
enum class EConstructionMaterial : uint8
{
    Wood                UMETA(DisplayName = "Wood"),
    Stone               UMETA(DisplayName = "Stone"),
    Mud                 UMETA(DisplayName = "Mud/Clay"),
    Bone                UMETA(DisplayName = "Bone"),
    Hide                UMETA(DisplayName = "Animal Hide"),
    Thatch              UMETA(DisplayName = "Thatch"),
    Bark                UMETA(DisplayName = "Tree Bark"),
    Bamboo              UMETA(DisplayName = "Bamboo"),
    Reed                UMETA(DisplayName = "Reed"),
    MixedMaterials      UMETA(DisplayName = "Mixed Materials")
};

/** Architectural styles based on environmental adaptation */
UENUM(BlueprintType)
enum class EArchitecturalStyle : uint8
{
    ForestAdapted       UMETA(DisplayName = "Forest Adapted"),
    PlainsDwelling      UMETA(DisplayName = "Plains Dwelling"),
    RiversideStructure  UMETA(DisplayName = "Riverside Structure"),
    HillsideBuilding    UMETA(DisplayName = "Hillside Building"),
    SwamplandHut        UMETA(DisplayName = "Swampland Hut"),
    DesertShelter       UMETA(DisplayName = "Desert Shelter"),
    MountainRefuge      UMETA(DisplayName = "Mountain Refuge"),
    CoastalStructure    UMETA(DisplayName = "Coastal Structure"),
    CaveExtension       UMETA(DisplayName = "Cave Extension"),
    NomadTemporary      UMETA(DisplayName = "Nomad Temporary")
};

/** Interior room types and their functions */
UENUM(BlueprintType)
enum class ERoomType : uint8
{
    MainLiving          UMETA(DisplayName = "Main Living Area"),
    Sleeping            UMETA(DisplayName = "Sleeping Area"),
    Storage             UMETA(DisplayName = "Storage Room"),
    Cooking             UMETA(DisplayName = "Cooking Area"),
    Crafting            UMETA(DisplayName = "Crafting Workshop"),
    Gathering           UMETA(DisplayName = "Gathering Space"),
    Sacred              UMETA(DisplayName = "Sacred/Ritual Space"),
    Defense             UMETA(DisplayName = "Defense Position"),
    AnimalShelter       UMETA(DisplayName = "Animal Shelter"),
    FoodPreparation     UMETA(DisplayName = "Food Preparation"),
    ToolMaking          UMETA(DisplayName = "Tool Making"),
    Entrance            UMETA(DisplayName = "Entrance/Foyer")
};

/** Weathering and aging states */
UENUM(BlueprintType)
enum class EWeatheringState : uint8
{
    New                 UMETA(DisplayName = "Recently Built"),
    Lived               UMETA(DisplayName = "Well Lived-In"),
    Worn                UMETA(DisplayName = "Showing Wear"),
    Weathered           UMETA(DisplayName = "Weather Damaged"),
    Deteriorating       UMETA(DisplayName = "Deteriorating"),
    Abandoned           UMETA(DisplayName = "Recently Abandoned"),
    Ruined              UMETA(DisplayName = "Ruined"),
    Ancient             UMETA(DisplayName = "Ancient Ruin")
};

/** Interior furnishing object types */
UENUM(BlueprintType)
enum class EFurnitureType : uint8
{
    SleepingFur         UMETA(DisplayName = "Sleeping Fur"),
    FirePit             UMETA(DisplayName = "Fire Pit"),
    CookingStone        UMETA(DisplayName = "Cooking Stone"),
    ToolRack            UMETA(DisplayName = "Tool Rack"),
    StorageBasket       UMETA(DisplayName = "Storage Basket"),
    WaterContainer      UMETA(DisplayName = "Water Container"),
    FoodStorage         UMETA(DisplayName = "Food Storage"),
    WorkSurface         UMETA(DisplayName = "Work Surface"),
    Seating             UMETA(DisplayName = "Seating"),
    Decoration          UMETA(DisplayName = "Decoration"),
    RitualObject        UMETA(DisplayName = "Ritual Object"),
    PersonalBelonging   UMETA(DisplayName = "Personal Belonging"),
    Weapon              UMETA(DisplayName = "Weapon"),
    ClothingStorage     UMETA(DisplayName = "Clothing Storage"),
    CraftingTool        UMETA(DisplayName = "Crafting Tool")
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

    /** Material variations for different weathering states */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
    TMap<EWeatheringState, TSoftObjectPtr<UMaterialInterface>> WeatheringMaterials;

    /** Attachment points for connecting to other components */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
    TArray<FVector> AttachmentPoints;

    /** Component type (wall, roof, floor, door, window, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
    FString ComponentType;

    /** Construction material */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
    EConstructionMaterial Material = EConstructionMaterial::Wood;

    /** Structural integrity (affects weathering) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StructuralIntegrity = 1.0f;

    /** Can this component be damaged/destroyed? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
    bool bCanBeDamaged = true;

    /** Collision settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
    bool bHasCollision = true;
};

/** Complete building configuration */
USTRUCT(BlueprintType)
struct FBuildingConfig
{
    GENERATED_BODY()

    /** Building type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    EBuildingType BuildingType = EBuildingType::ShelterHut;

    /** Architectural style */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    EArchitecturalStyle ArchitecturalStyle = EArchitecturalStyle::ForestAdapted;

    /** Primary construction material */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    EConstructionMaterial PrimaryMaterial = EConstructionMaterial::Wood;

    /** Secondary construction material */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    EConstructionMaterial SecondaryMaterial = EConstructionMaterial::Thatch;

    /** Building components that make up this structure */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    TArray<FBuildingComponent> BuildingComponents;

    /** Room layout for interior generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interior")
    TArray<ERoomType> RoomLayout;

    /** Current weathering state */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    EWeatheringState WeatheringState = EWeatheringState::Lived;

    /** Building size category */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    FVector BuildingSize = FVector(400.0f, 400.0f, 300.0f);

    /** Preferred biomes for this building type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<EBiomeType> PreferredBiomes;

    /** Minimum distance from water sources */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0"))
    float MinWaterDistance = 100.0f;

    /** Maximum distance from water sources */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0"))
    float MaxWaterDistance = 1000.0f;

    /** Slope tolerance for building placement */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D SlopeTolerance = FVector2D(0.0f, 15.0f);

    /** Narrative description of this building's purpose and history */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling", meta = (MultiLine = true))
    FString BuildingNarrative;

    /** Number of inhabitants this building was designed for */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling", meta = (ClampMin = "1", ClampMax = "50"))
    int32 DesignedInhabitants = 1;

    /** Age of the building (affects weathering) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling", meta = (ClampMin = "0.0"))
    float BuildingAge = 1.0f; // Years

    /** Has this building been abandoned? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling")
    bool bIsAbandoned = false;

    /** Time since abandonment (if abandoned) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling", meta = (ClampMin = "0.0"))
    float TimeSinceAbandonment = 0.0f; // Years
};

/** Interior furnishing configuration */
USTRUCT(BlueprintType)
struct FFurnitureConfig
{
    GENERATED_BODY()

    /** Furniture type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Furniture")
    EFurnitureType FurnitureType = EFurnitureType::SleepingFur;

    /** Static mesh for this furniture */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Furniture")
    TSoftObjectPtr<UStaticMesh> FurnitureMesh;

    /** Material variations for different conditions */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Furniture")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;

    /** Room types where this furniture is appropriate */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<ERoomType> AppropriateRooms;

    /** Placement priority (higher = more likely to be placed) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PlacementPriority = 0.5f;

    /** Size requirements for placement */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector RequiredSpace = FVector(100.0f, 100.0f, 100.0f);

    /** Must be placed against walls? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    bool bRequiresWallPlacement = false;

    /** Must be placed near other specific furniture? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<EFurnitureType> RequiredNearbyFurniture;

    /** Cannot be placed near these furniture types */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    TArray<EFurnitureType> ConflictingFurniture;

    /** Narrative purpose of this furniture */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling", meta = (MultiLine = true))
    FString NarrativePurpose;

    /** Signs of use/wear this furniture shows */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling")
    TArray<FString> WearSigns;

    /** Can the player interact with this furniture? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bIsInteractable = false;

    /** Interaction type (examine, use, move, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    FString InteractionType;

    /** Resources provided when interacted with */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    TMap<FString, int32> InteractionRewards;
};

/** Room configuration for interior generation */
USTRUCT(BlueprintType)
struct FRoomConfig
{
    GENERATED_BODY()

    /** Room type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
    ERoomType RoomType = ERoomType::MainLiving;

    /** Room dimensions */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
    FVector RoomSize = FVector(300.0f, 300.0f, 250.0f);

    /** Required furniture for this room type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
    TArray<EFurnitureType> RequiredFurniture;

    /** Optional furniture for this room type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
    TArray<EFurnitureType> OptionalFurniture;

    /** Floor material for this room */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
    TSoftObjectPtr<UMaterialInterface> FloorMaterial;

    /** Wall material for this room */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
    TSoftObjectPtr<UMaterialInterface> WallMaterial;

    /** Lighting configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
    bool bHasFirePit = false;

    /** Natural light sources (windows, openings) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
    int32 NaturalLightSources = 1;

    /** Privacy level (affects door placement) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PrivacyLevel = 0.5f;

    /** Activity level (affects wear and furniture placement) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ActivityLevel = 0.5f;

    /** Narrative description of this room's purpose and history */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling", meta = (MultiLine = true))
    FString RoomNarrative;
};

/** Data asset containing all building configurations */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildingDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    /** All available building configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buildings")
    TArray<FBuildingConfig> BuildingConfigurations;

    /** All available furniture configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Furniture")
    TArray<FFurnitureConfig> FurnitureConfigurations;

    /** All available room configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms")
    TArray<FRoomConfig> RoomConfigurations;

    /** Building components library */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<FBuildingComponent> ComponentLibrary;

    /** Get building configurations for a specific biome */
    UFUNCTION(BlueprintCallable, Category = "Buildings")
    TArray<FBuildingConfig> GetBuildingsForBiome(EBiomeType BiomeType) const;

    /** Get furniture configurations for a specific room type */
    UFUNCTION(BlueprintCallable, Category = "Furniture")
    TArray<FFurnitureConfig> GetFurnitureForRoom(ERoomType RoomType) const;

    /** Get appropriate architectural style for biome and location */
    UFUNCTION(BlueprintCallable, Category = "Buildings")
    EArchitecturalStyle GetStyleForLocation(EBiomeType BiomeType, float DistanceToWater, float Elevation) const;
};

/** Main architecture system subsystem */
UCLASS()
class TRANSPERSONALGAME_API UArchitectureSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Initialize the architecture system with building database */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeSystem(UBuildingDatabase* InBuildingDatabase);

    /** Generate buildings for a specific area */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateBuildingsInArea(const FVector& AreaCenter, float AreaRadius, EBiomeType BiomeType);

    /** Generate a specific building at a location */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    class ABuilding* GenerateBuilding(const FVector& Location, const FBuildingConfig& BuildingConfig);

    /** Generate interior for an existing building */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateInterior(class ABuilding* Building, const TArray<FRoomConfig>& RoomConfigs);

    /** Update weathering for all buildings in the world */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateBuildingWeathering(float DeltaTime);

    /** Find suitable building locations in an area */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FVector> FindBuildingLocations(const FVector& AreaCenter, float AreaRadius, const FBuildingConfig& BuildingConfig);

    /** Get building database */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    UBuildingDatabase* GetBuildingDatabase() const { return BuildingDatabase; }

protected:
    /** Building database containing all configurations */
    UPROPERTY()
    UBuildingDatabase* BuildingDatabase;

    /** All buildings currently in the world */
    UPROPERTY()
    TArray<class ABuilding*> WorldBuildings;

    /** Performance settings */
    UPROPERTY()
    float MaxBuildingsPerArea = 10.0f;

    UPROPERTY()
    float BuildingUpdateInterval = 60.0f; // Seconds between weathering updates

    /** Timer for building updates */
    FTimerHandle BuildingUpdateTimer;

    /** Internal methods */
    void UpdateBuildingWeatheringInternal();
    bool IsLocationSuitableForBuilding(const FVector& Location, const FBuildingConfig& BuildingConfig) const;
    EArchitecturalStyle DetermineArchitecturalStyle(EBiomeType BiomeType, const FVector& Location) const;
    void PlaceFurnitureInRoom(class ABuilding* Building, const FRoomConfig& RoomConfig, const FVector& RoomCenter, const FVector& RoomSize);
};