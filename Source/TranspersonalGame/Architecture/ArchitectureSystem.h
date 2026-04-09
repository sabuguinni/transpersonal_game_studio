// Copyright Transpersonal Game Studio. All Rights Reserved.
// ArchitectureSystem.h - Prehistoric Architecture Generation System

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Actor.h"
#include "../World/PCGWorldGenerator.h"
#include "ArchitectureSystem.generated.h"

class UStaticMesh;
class UMaterialInterface;
class AStaticMeshActor;

/**
 * Prehistoric Structure Types
 * Based on early human and pre-civilization architecture
 */
UENUM(BlueprintType)
enum class EPrehistoricStructureType : uint8
{
    // Shelters
    CaveEntrance_Natural        UMETA(DisplayName = "Natural Cave Entrance"),
    CaveEntrance_Modified       UMETA(DisplayName = "Modified Cave Entrance"),
    RockShelter_Simple          UMETA(DisplayName = "Simple Rock Shelter"),
    RockShelter_Extended        UMETA(DisplayName = "Extended Rock Shelter"),
    
    // Primitive Dwellings
    HutFrame_Basic              UMETA(DisplayName = "Basic Hut Frame"),
    HutFrame_Advanced           UMETA(DisplayName = "Advanced Hut Frame"),
    LeanTo_Simple               UMETA(DisplayName = "Simple Lean-To"),
    LeanTo_Complex              UMETA(DisplayName = "Complex Lean-To"),
    
    // Stone Structures
    StoneCircle_Small           UMETA(DisplayName = "Small Stone Circle"),
    StoneCircle_Large           UMETA(DisplayName = "Large Stone Circle"),
    Dolmen_Simple               UMETA(DisplayName = "Simple Dolmen"),
    Dolmen_Complex              UMETA(DisplayName = "Complex Dolmen"),
    Menhir_Single               UMETA(DisplayName = "Single Menhir"),
    Menhir_Cluster              UMETA(DisplayName = "Menhir Cluster"),
    
    // Functional Structures
    FirePit_Simple              UMETA(DisplayName = "Simple Fire Pit"),
    FirePit_Elaborate           UMETA(DisplayName = "Elaborate Fire Pit"),
    ToolCache_Hidden            UMETA(DisplayName = "Hidden Tool Cache"),
    ToolCache_Visible           UMETA(DisplayName = "Visible Tool Cache"),
    FoodStorage_Underground     UMETA(DisplayName = "Underground Food Storage"),
    FoodStorage_Elevated        UMETA(DisplayName = "Elevated Food Storage"),
    
    // Defensive Structures
    Palisade_Simple             UMETA(DisplayName = "Simple Palisade"),
    Palisade_Reinforced         UMETA(DisplayName = "Reinforced Palisade"),
    Barricade_Stone             UMETA(DisplayName = "Stone Barricade"),
    Barricade_Wood              UMETA(DisplayName = "Wood Barricade"),
    Watchtower_Basic            UMETA(DisplayName = "Basic Watchtower"),
    
    // Ritual/Sacred Structures
    SacredGrove_Marked          UMETA(DisplayName = "Marked Sacred Grove"),
    AltarStone_Simple           UMETA(DisplayName = "Simple Altar Stone"),
    AltarStone_Elaborate        UMETA(DisplayName = "Elaborate Altar Stone"),
    BurialCairn_Small           UMETA(DisplayName = "Small Burial Cairn"),
    BurialCairn_Large           UMETA(DisplayName = "Large Burial Cairn"),
    
    // Ancient Ruins
    RuinWall_Collapsed          UMETA(DisplayName = "Collapsed Ruin Wall"),
    RuinWall_Standing           UMETA(DisplayName = "Standing Ruin Wall"),
    RuinFoundation_Stone        UMETA(DisplayName = "Stone Ruin Foundation"),
    RuinPillar_Broken           UMETA(DisplayName = "Broken Ruin Pillar"),
    RuinPillar_Standing         UMETA(DisplayName = "Standing Ruin Pillar"),
    
    // Mysterious Structures
    CrystalFormation_Natural    UMETA(DisplayName = "Natural Crystal Formation"),
    CrystalFormation_Arranged   UMETA(DisplayName = "Arranged Crystal Formation"),
    StrangeMonolith_Ancient     UMETA(DisplayName = "Ancient Strange Monolith"),
    GeometricPattern_Ground     UMETA(DisplayName = "Ground Geometric Pattern")
};

/**
 * Structure Condition/Age
 */
UENUM(BlueprintType)
enum class EStructureCondition : uint8
{
    New         UMETA(DisplayName = "New/Recently Built"),
    Good        UMETA(DisplayName = "Good Condition"),
    Weathered   UMETA(DisplayName = "Weathered"),
    Damaged     UMETA(DisplayName = "Damaged"),
    Ruined      UMETA(DisplayName = "Ruined"),
    Ancient     UMETA(DisplayName = "Ancient/Mysterious")
};

/**
 * Interior Space Types
 */
UENUM(BlueprintType)
enum class EInteriorSpaceType : uint8
{
    Shelter_Basic           UMETA(DisplayName = "Basic Shelter"),
    Shelter_Family          UMETA(DisplayName = "Family Shelter"),
    Workshop_Tool           UMETA(DisplayName = "Tool Workshop"),
    Workshop_Food           UMETA(DisplayName = "Food Preparation"),
    Storage_General         UMETA(DisplayName = "General Storage"),
    Storage_Sacred          UMETA(DisplayName = "Sacred Storage"),
    Ritual_Small            UMETA(DisplayName = "Small Ritual Space"),
    Ritual_Large            UMETA(DisplayName = "Large Ritual Space"),
    Cave_Natural            UMETA(DisplayName = "Natural Cave"),
    Cave_Modified           UMETA(DisplayName = "Modified Cave"),
    Ruin_Chamber            UMETA(DisplayName = "Ruined Chamber"),
    Ruin_Hall               UMETA(DisplayName = "Ruined Hall")
};

/**
 * Structure Instance Data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FStructureInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EPrehistoricStructureType StructureType = EPrehistoricStructureType::HutFrame_Basic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    EStructureCondition Condition = EStructureCondition::Good;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    int32 VariationSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasInterior = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EInteriorSpaceType InteriorType = EInteriorSpaceType::Shelter_Basic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occupation")
    bool bCurrentlyOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occupation")
    float AbandonmentTime = 0.0f; // Days since abandonment

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StructureStory; // What happened here?

    FStructureInstanceData()
    {
        VariationSeed = FMath::Rand();
    }
};

/**
 * Interior Prop Data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteriorPropData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    FString PropName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TObjectPtr<UStaticMesh> PropMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector RelativeScale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString PropStory; // What is this item's story?

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanBePickedUp = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanBeExamined = true;
};

/**
 * Interior Configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteriorConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space")
    EInteriorSpaceType SpaceType = EInteriorSpaceType::Shelter_Basic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<FInteriorPropData> Props;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FVector FirePitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString InteriorStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float DustLevel = 0.5f; // 0.0 = clean, 1.0 = very dusty

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MoistureLevel = 0.3f; // 0.0 = dry, 1.0 = damp

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bHasSpiderWebs = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bHasAnimalSigns = false; // Droppings, scratches, etc.
};

/**
 * Structure Asset Data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FStructureAssetData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TObjectPtr<UStaticMesh> StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TObjectPtr<UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    TArray<TObjectPtr<UStaticMesh>> VariationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TObjectPtr<UStaticMesh> InteriorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FInteriorConfiguration DefaultInteriorConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bHasCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bCanBeDestroyed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
    FVector MinScale = FVector(0.9f, 0.9f, 0.9f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
    FVector MaxScale = FVector(1.1f, 1.1f, 1.1f);
};

/**
 * Settlement Configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSettlementConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size")
    int32 MinStructures = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size")
    int32 MaxStructures = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    float StructureSpacing = 500.0f; // Minimum distance between structures

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Types")
    TMap<EPrehistoricStructureType, float> StructureTypeWeights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    float AbandonmentChance = 0.3f; // Chance a structure is abandoned

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    float RuinChance = 0.1f; // Chance a structure is ruined

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    bool bHasCentralFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    bool bHasDefensiveStructures = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    bool bHasRitualArea = false;

    FSettlementConfig()
    {
        // Default small settlement configuration
        StructureTypeWeights.Add(EPrehistoricStructureType::HutFrame_Basic, 0.4f);
        StructureTypeWeights.Add(EPrehistoricStructureType::HutFrame_Advanced, 0.2f);
        StructureTypeWeights.Add(EPrehistoricStructureType::FoodStorage_Elevated, 0.15f);
        StructureTypeWeights.Add(EPrehistoricStructureType::ToolCache_Visible, 0.1f);
        StructureTypeWeights.Add(EPrehistoricStructureType::FirePit_Elaborate, 0.1f);
        StructureTypeWeights.Add(EPrehistoricStructureType::AltarStone_Simple, 0.05f);
    }
};

/**
 * Architecture System Component
 * Manages procedural architecture generation and interior design
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Architecture), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArchitectureSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchitectureSystem();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Core architecture functions
    UFUNCTION(BlueprintCallable, Category = "Architecture Generation")
    void GenerateStructuresForBiome(EPrehistoricBiome BiomeType, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture Generation")
    void GenerateSettlement(const FVector& Center, const FSettlementConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Architecture Generation")
    void GenerateIsolatedStructures(const FVector& Center, float Radius, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Architecture Generation")
    void ClearStructuresInArea(const FBox& Area);

    // Structure placement functions
    UFUNCTION(BlueprintCallable, Category = "Structure Placement")
    AActor* PlaceStructureAtLocation(EPrehistoricStructureType StructureType, 
                                   const FVector& Location, 
                                   const FRotator& Rotation = FRotator::ZeroRotator,
                                   const FVector& Scale = FVector::OneVector);

    UFUNCTION(BlueprintCallable, Category = "Structure Placement")
    bool RemoveStructureAtLocation(const FVector& Location, float SearchRadius = 200.0f);

    UFUNCTION(BlueprintCallable, Category = "Structure Placement")
    void ModifyStructureCondition(AActor* Structure, EStructureCondition NewCondition);

    // Interior design functions
    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    void GenerateInteriorForStructure(AActor* Structure, EInteriorSpaceType SpaceType);

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    void PopulateInteriorWithProps(AActor* Structure, const FInteriorConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    void CreateStorytellingProps(AActor* Structure, const FString& Story);

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    void AddAbandonmentDetails(AActor* Structure, float AbandonmentTime);

    // Biome-specific generation
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void GenerateForestSettlement(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void GenerateCaveComplex(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void GenerateAncientRuins(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void GenerateRitualSite(const FVector& Center, float Radius);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Architecture Utility")
    bool IsLocationSuitableForStructure(const FVector& Location, EPrehistoricStructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture Utility")
    FVector FindBestLocationForStructure(const FVector& SearchCenter, float SearchRadius, 
                                       EPrehistoricStructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture Utility")
    TArray<FVector> GenerateSettlementLayout(const FVector& Center, const FSettlementConfig& Config);

    // Story generation
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FString GenerateStructureStory(EPrehistoricStructureType StructureType, EStructureCondition Condition);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FString GenerateInteriorStory(EInteriorSpaceType SpaceType, bool bIsAbandoned);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void ApplyStorytellingDetails(AActor* Structure, const FString& Story);

protected:
    // Asset management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TMap<EPrehistoricStructureType, FStructureAssetData> StructureAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TMap<EInteriorSpaceType, FInteriorConfiguration> InteriorConfigurations;

    // Generation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinDistanceBetweenStructures = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MaxSlopeForStructures = 30.0f; // Degrees

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAvoidWaterBodies = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinDistanceFromWater = 100.0f;

    // Storytelling settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FString> StructureStoryTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FString> InteriorStoryTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FString> AbandonmentReasons;

    // Runtime data
    UPROPERTY()
    TArray<AActor*> GeneratedStructures;

    UPROPERTY()
    TMap<AActor*, FStructureInstanceData> StructureData;

private:
    // Internal generation functions
    void GenerateStructureVariation(AActor* Structure, const FStructureInstanceData& Data);
    void ApplyWeatheringEffects(AActor* Structure, EStructureCondition Condition);
    void SetupStructureCollision(AActor* Structure, EPrehistoricStructureType StructureType);
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius);
    bool IsLocationValid(const FVector& Location, EPrehistoricStructureType StructureType);
    void LoadStructureAssets();
    void SetupDefaultInteriorConfigurations();
    void SetupStoryTemplates();
};

/**
 * Architecture Manager Actor
 * Placed in level to manage all architectural generation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitectureManager();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UArchitectureSystem> ArchitectureSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateOnBeginPlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float GenerationRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TArray<FSettlementConfig> SettlementConfigs;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegenerateAllStructures();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllStructures();
};