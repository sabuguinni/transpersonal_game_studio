// Copyright Transpersonal Game Studio. All Rights Reserved.
// Jurassic Architecture System - Creates authentic prehistoric structures
// Agent #7 - Architecture & Interior Agent

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"
#include "PCGComponent.h"
#include "../WorldGeneration/JurassicBiomeManager.h"
#include "../Environment/JurassicEnvironmentArtCore.h"
#include "JurassicArchitectureSystem.generated.h"

/**
 * Architecture System for Prehistoric Structures
 * Creates authentic structures that tell the story of who lived there
 * Every building is a historical document of the civilization that built it
 */

UENUM(BlueprintType)
enum class EPrehistoricStructureType : uint8
{
    // Shelter Types
    SimpleHut           UMETA(DisplayName = "Simple Hut - Basic survival shelter"),
    FamilyCave          UMETA(DisplayName = "Family Cave - Extended dwelling"),
    CommunalShelter     UMETA(DisplayName = "Communal Shelter - Group living"),
    ChieftainHut        UMETA(DisplayName = "Chieftain Hut - Leadership dwelling"),
    
    // Functional Structures
    FoodStorage         UMETA(DisplayName = "Food Storage - Preservation structure"),
    ToolWorkshop        UMETA(DisplayName = "Tool Workshop - Crafting area"),
    FirePit             UMETA(DisplayName = "Fire Pit - Cooking and warmth"),
    WaterWell           UMETA(DisplayName = "Water Well - Water collection"),
    
    // Defensive Structures
    Watchtower          UMETA(DisplayName = "Watchtower - Observation post"),
    Palisade            UMETA(DisplayName = "Palisade - Defensive wall"),
    TrapPit             UMETA(DisplayName = "Trap Pit - Defensive trap"),
    
    // Spiritual/Cultural
    RitualCircle        UMETA(DisplayName = "Ritual Circle - Ceremonial space"),
    AncestorShrine      UMETA(DisplayName = "Ancestor Shrine - Spiritual site"),
    CaveArt             UMETA(DisplayName = "Cave Art - Cultural expression"),
    
    // Ruins and Abandoned
    AbandonedHut        UMETA(DisplayName = "Abandoned Hut - Tells a story"),
    CollapsedShelter    UMETA(DisplayName = "Collapsed Shelter - Time's passage"),
    OvergrownRuins      UMETA(DisplayName = "Overgrown Ruins - Nature reclaims"),
    
    // Special/Mystery
    GemaSite            UMETA(DisplayName = "Gema Site - Mysterious energy"),
    AncientMonument     UMETA(DisplayName = "Ancient Monument - Unknown origin")
};

UENUM(BlueprintType)
enum class EStructureCondition : uint8
{
    Perfect         UMETA(DisplayName = "Perfect - Just built"),
    WellMaintained  UMETA(DisplayName = "Well Maintained - Cared for"),
    Weathered       UMETA(DisplayName = "Weathered - Shows age"),
    Damaged         UMETA(DisplayName = "Damaged - Partial destruction"),
    Ruined          UMETA(DisplayName = "Ruined - Mostly destroyed"),
    Overgrown       UMETA(DisplayName = "Overgrown - Nature reclaiming"),
    Mysterious      UMETA(DisplayName = "Mysterious - Unnatural preservation")
};

UENUM(BlueprintType)
enum class EInhabitantType : uint8
{
    None            UMETA(DisplayName = "Uninhabited"),
    Hunter          UMETA(DisplayName = "Hunter - Survival specialist"),
    Gatherer        UMETA(DisplayName = "Gatherer - Food collector"),
    Toolmaker       UMETA(DisplayName = "Toolmaker - Craft specialist"),
    Shaman          UMETA(DisplayName = "Shaman - Spiritual leader"),
    Chieftain       UMETA(DisplayName = "Chieftain - Tribal leader"),
    Family          UMETA(DisplayName = "Family - Multiple inhabitants"),
    Mysterious      UMETA(DisplayName = "Mysterious - Unknown inhabitant")
};

USTRUCT(BlueprintType)
struct FStructureMaterial
{
    GENERATED_BODY()

    // Primary construction materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    UMaterialInterface* WoodMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    UMaterialInterface* StoneMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    UMaterialInterface* ThatchMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    UMaterialInterface* HideMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    UMaterialInterface* BoneMaterial = nullptr;

    // Weathering and aging materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    TArray<UMaterialInterface*> WeatheredVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    UMaterialInterface* MossyMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    UMaterialInterface* RottenMaterial = nullptr;

    // Material blend weights based on condition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float WeatheringIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float MoistureEffect = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float AgeEffect = 0.7f;
};

USTRUCT(BlueprintType)
struct FInteriorLayout
{
    GENERATED_BODY()

    // Interior props that tell the inhabitant's story
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<UStaticMesh*> SleepingArea; // Bed, furs, sleeping arrangements

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<UStaticMesh*> CookingArea; // Fire pit, cooking stones, pots

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<UStaticMesh*> ToolStorage; // Weapon racks, tool containers

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<UStaticMesh*> FoodStorage; // Storage baskets, dried meat

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<UStaticMesh*> PersonalItems; // Clothing, ornaments, crafts

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<UStaticMesh*> SpiritualItems; // Totems, ritual objects

    // Layout parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    FVector InteriorSize = FVector(400.0f, 400.0f, 250.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    bool bHasCentralFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    int32 MaxOccupants = 2;

    // Story elements - what happened here?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bShowSignsOfStruggle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bShowHastyDeparture = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bShowLongAbandonment = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bShowRecentActivity = true;

    // Clutter and wear patterns
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
    float ClutterDensity = 0.5f; // How lived-in it looks

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
    float WearPatterns = 0.3f; // Paths, worn spots

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
    float DirtAccumulation = 0.2f; // Dust, debris
};

USTRUCT(BlueprintType)
struct FStructureBlueprint
{
    GENERATED_BODY()

    // Structure identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EPrehistoricStructureType StructureType = EPrehistoricStructureType::SimpleHut;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EInhabitantType InhabitantType = EInhabitantType::Hunter;

    // Physical structure
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    TArray<UStaticMesh*> StructuralElements; // Walls, roof, supports

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    UStaticMesh* FoundationMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    UStaticMesh* RoofMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    TArray<UStaticMesh*> DoorMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    TArray<UStaticMesh*> WindowMeshes;

    // Size and scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions")
    FVector BaseSize = FVector(500.0f, 500.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions")
    FVector2D ScaleVariation = FVector2D(0.8f, 1.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions")
    bool bAllowRotation = true;

    // Materials for this structure type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    FStructureMaterial StructureMaterials;

    // Interior configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FInteriorLayout InteriorLayout;

    // Environmental requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<EJurassicBiomeType> PreferredBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MinSlope = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MaxSlope = 15.0f; // Structures need relatively flat ground

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WaterProximityPreference = 0.0f; // -1 = near water, +1 = away from water

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bRequiresClearance = true; // Needs space around it

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float ClearanceRadius = 300.0f;

    // Spawning rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnProbability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 MaxInstancesPerBiome = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float MinDistanceBetweenStructures = 1000.0f;

    // Clustering - some structures appear together
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering")
    bool bCanFormSettlements = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering")
    TArray<EPrehistoricStructureType> ComplementaryStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering")
    float SettlementRadius = 500.0f;

    // Condition and aging
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    EStructureCondition DefaultCondition = EStructureCondition::WellMaintained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    TArray<EStructureCondition> PossibleConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    float AgeVariation = 0.5f; // How much aging varies

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullDistance = 8000.0f; // Structures are important landmarks

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bCastShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxLODLevel = 3;
};

USTRUCT(BlueprintType)
struct FSettlementCluster
{
    GENERATED_BODY()

    // Settlement identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    FString SettlementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    int32 Population = 5; // Estimated inhabitants

    // Core structures that define the settlement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
    TArray<EPrehistoricStructureType> CoreStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
    TArray<EPrehistoricStructureType> OptionalStructures;

    // Settlement layout
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    float SettlementRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    bool bHasCentralArea = true; // Common gathering space

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    bool bHasDefensivePerimeter = false;

    // Settlement story - what kind of community was this?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bWasProsperous = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bWasAbandoned = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bWasAttacked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bWasMigration = false;

    // Environmental integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bFollowsTopography = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bUsesNaturalFeatures = true; // Incorporates rocks, trees, water

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float TerrainAdaptation = 0.8f; // How well it fits the landscape
};

/**
 * Data Asset containing all structure blueprints and settlement patterns
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UJurassicArchitectureDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // All available structure blueprints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
    TArray<FStructureBlueprint> StructureBlueprints;

    // Settlement patterns
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlements")
    TArray<FSettlementCluster> SettlementTypes;

    // Global architecture settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalStructureDensity = 0.1f; // Structures per square kilometer

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float SettlementProbability = 0.3f; // Chance of clustered vs isolated structures

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float AbandonmentRate = 0.4f; // Percentage of structures that are abandoned

    // Material aging parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aging")
    float GlobalWeatheringRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aging")
    float MoistureWeatheringMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aging")
    float VegetationOvergrowthRate = 0.3f;
};

/**
 * Main Architecture System Actor
 * Generates and manages all prehistoric structures in the world
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AJurassicArchitectureSystem : public AActor
{
    GENERATED_BODY()

public:
    AJurassicArchitectureSystem();

protected:
    virtual void BeginPlay() override;

    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPCGComponent* ArchitecturePCGComponent;

    // System configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    class UJurassicArchitectureDataAsset* ArchitectureDataAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    class AJurassicBiomeManager* BiomeManager;

    // Generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bGenerateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bUseHierarchicalGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float GenerationRadius = 10000.0f; // Generate structures within this radius

    // World bounds for structure placement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Bounds")
    FVector WorldOrigin = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Bounds")
    FVector WorldSize = FVector(201600.0f, 201600.0f, 25600.0f);

public:
    // Structure generation functions
    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateAllStructures();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateStructuresInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void ClearAllStructures();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void RefreshStructureGeneration();

    // Structure spawning
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    AActor* SpawnStructure(EPrehistoricStructureType StructureType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, EStructureCondition Condition = EStructureCondition::WellMaintained);

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void SpawnSettlement(FVector CenterLocation, const FSettlementCluster& SettlementData);

    // Query functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
    TArray<AActor*> GetStructuresInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
    AActor* GetNearestStructure(FVector Location) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
    bool IsLocationSuitableForStructure(FVector Location, EPrehistoricStructureType StructureType) const;

    // Interior decoration
    UFUNCTION(BlueprintCallable, Category = "Interior")
    void PopulateStructureInterior(AActor* StructureActor, const FInteriorLayout& Layout, EInhabitantType InhabitantType, EStructureCondition Condition);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void ApplyStorytellingElements(AActor* StructureActor, const FInteriorLayout& Layout);

private:
    // Internal generation helpers
    void SetupPCGArchitectureGeneration();
    bool ValidateStructurePlacement(FVector Location, const FStructureBlueprint& Blueprint) const;
    FStructureBlueprint GetStructureBlueprintByType(EPrehistoricStructureType StructureType) const;
    void ApplyStructureCondition(AActor* StructureActor, EStructureCondition Condition);
    void ApplyWeatheringEffects(AActor* StructureActor, float WeatheringIntensity, EJurassicBiomeType BiomeType);
    FVector FindSuitableLocationForStructure(const FStructureBlueprint& Blueprint, FVector PreferredLocation, float SearchRadius) const;
    void CreateSettlementLayout(const FSettlementCluster& Settlement, FVector CenterLocation, TArray<FVector>& StructureLocations) const;

    // Cached data for performance
    UPROPERTY()
    TArray<AActor*> SpawnedStructures;

    UPROPERTY()
    TMap<EPrehistoricStructureType, FStructureBlueprint> StructureBlueprintCache;
};