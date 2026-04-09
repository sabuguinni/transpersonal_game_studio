// Copyright Transpersonal Game Studio. All Rights Reserved.
// PrehistoricArchitectureManager.h - Core architecture and interior management system

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataAsset.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "PCGComponent.h"
#include "PrehistoricArchitectureManager.generated.h"

class ULandscape;
class UMaterialParameterCollection;
class UNiagaraSystem;
class UAudioComponent;
class UPCGGraph;
class UBoxComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EArchitectureType : uint8
{
    Cave                 UMETA(DisplayName = "Cave"),
    RockShelter         UMETA(DisplayName = "Rock Shelter"),
    SimpleHut           UMETA(DisplayName = "Simple Hut"),
    TribalSettlement    UMETA(DisplayName = "Tribal Settlement"),
    SacredSite          UMETA(DisplayName = "Sacred Site"),
    BurialGround        UMETA(DisplayName = "Burial Ground"),
    ToolWorkshop        UMETA(DisplayName = "Tool Workshop"),
    FoodStorage         UMETA(DisplayName = "Food Storage"),
    WaterSource         UMETA(DisplayName = "Water Source"),
    DefensivePosition   UMETA(DisplayName = "Defensive Position"),
    AncientRuins        UMETA(DisplayName = "Ancient Ruins")
};

UENUM(BlueprintType)
enum class EConstructionMaterial : uint8
{
    Stone               UMETA(DisplayName = "Stone"),
    Wood                UMETA(DisplayName = "Wood"),
    Bone                UMETA(DisplayName = "Bone"),
    Hide                UMETA(DisplayName = "Hide"),
    Mud                 UMETA(DisplayName = "Mud"),
    Thatch              UMETA(DisplayName = "Thatch"),
    Clay                UMETA(DisplayName = "Clay"),
    Organic             UMETA(DisplayName = "Organic"),
    Natural             UMETA(DisplayName = "Natural")
};

UENUM(BlueprintType)
enum class EInteriorFunction : uint8
{
    Sleeping            UMETA(DisplayName = "Sleeping"),
    Cooking             UMETA(DisplayName = "Cooking"),
    ToolMaking          UMETA(DisplayName = "Tool Making"),
    FoodPreparation     UMETA(DisplayName = "Food Preparation"),
    Storage             UMETA(DisplayName = "Storage"),
    Ritual              UMETA(DisplayName = "Ritual"),
    Shelter             UMETA(DisplayName = "Shelter"),
    Gathering           UMETA(DisplayName = "Gathering"),
    Defense             UMETA(DisplayName = "Defense"),
    Healing             UMETA(DisplayName = "Healing")
};

UENUM(BlueprintType)
enum class EArchitecturalCondition : uint8
{
    Pristine            UMETA(DisplayName = "Pristine"),
    WellMaintained      UMETA(DisplayName = "Well Maintained"),
    Weathered           UMETA(DisplayName = "Weathered"),
    Damaged             UMETA(DisplayName = "Damaged"),
    Ruined              UMETA(DisplayName = "Ruined"),
    Abandoned           UMETA(DisplayName = "Abandoned"),
    Overgrown           UMETA(DisplayName = "Overgrown"),
    Collapsed           UMETA(DisplayName = "Collapsed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArchitecturalElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element")
    FString ElementName = TEXT("Architectural Element");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<UStaticMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TArray<TSoftObjectPtr<UStaticMesh>> VariationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector RelativeScale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float ScaleVariation = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float RotationVariation = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bIsRequired = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float SpawnProbability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableNanite = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullDistance = 3000.0f;

    FArchitecturalElement()
    {
        ElementName = TEXT("Architectural Element");
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        RelativeScale = FVector::OneVector;
        ScaleVariation = 0.1f;
        RotationVariation = 5.0f;
        bIsRequired = true;
        SpawnProbability = 1.0f;
        bEnableNanite = true;
        CullDistance = 3000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteriorProp
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    FString PropName = TEXT("Interior Prop");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
    EInteriorFunction FunctionType = EInteriorFunction::Storage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<UStaticMesh> PropMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector RelativeScale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString StorytellingContext = TEXT("Generic prop");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bTellsStory = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bIsInteractable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionPrompt = TEXT("Examine");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float SpawnProbability = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bRequiresFloorSpace = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableNanite = true;

    FInteriorProp()
    {
        PropName = TEXT("Interior Prop");
        FunctionType = EInteriorFunction::Storage;
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        RelativeScale = FVector::OneVector;
        StorytellingContext = TEXT("Generic prop");
        bTellsStory = false;
        bIsInteractable = false;
        InteractionPrompt = TEXT("Examine");
        SpawnProbability = 0.8f;
        bRequiresFloorSpace = true;
        bEnableNanite = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuildingTemplate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FString BuildingName = TEXT("Prehistoric Structure");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    EArchitectureType ArchitectureType = EArchitectureType::SimpleHut;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    EConstructionMaterial PrimaryMaterial = EConstructionMaterial::Wood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<EConstructionMaterial> SecondaryMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    EArchitecturalCondition Condition = EArchitecturalCondition::WellMaintained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    TArray<FArchitecturalElement> StructuralElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FInteriorProp> InteriorProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions")
    FVector BuildingDimensions = FVector(400.0f, 400.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float MinSlopeAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float MaxSlopeAngle = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bRequiresWaterAccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float WaterAccessDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bRequiresDefensivePosition = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString CulturalContext = TEXT("Generic prehistoric dwelling");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString HistoricalNarrative = TEXT("A place where ancient peoples lived and worked");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseHierarchicalLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxDrawDistance = 5000.0f;

    FBuildingTemplate()
    {
        BuildingName = TEXT("Prehistoric Structure");
        ArchitectureType = EArchitectureType::SimpleHut;
        PrimaryMaterial = EConstructionMaterial::Wood;
        Condition = EArchitecturalCondition::WellMaintained;
        BuildingDimensions = FVector(400.0f, 400.0f, 300.0f);
        MinSlopeAngle = 0.0f;
        MaxSlopeAngle = 15.0f;
        bRequiresWaterAccess = false;
        WaterAccessDistance = 500.0f;
        bRequiresDefensivePosition = false;
        CulturalContext = TEXT("Generic prehistoric dwelling");
        HistoricalNarrative = TEXT("A place where ancient peoples lived and worked");
        bUseHierarchicalLOD = true;
        MaxDrawDistance = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSettlementLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    FString SettlementName = TEXT("Prehistoric Settlement");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buildings")
    TArray<FBuildingTemplate> Buildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    float SettlementRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    FVector CentralPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    int32 EstimatedPopulation = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    bool bHasCentralFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    bool bHasDefensiveWall = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    bool bHasWaterSource = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString SettlementStory = TEXT("A thriving prehistoric community");

    FSettlementLayout()
    {
        SettlementName = TEXT("Prehistoric Settlement");
        SettlementRadius = 1000.0f;
        CentralPoint = FVector::ZeroVector;
        EstimatedPopulation = 10;
        bHasCentralFirePit = true;
        bHasDefensiveWall = false;
        bHasWaterSource = true;
        SettlementStory = TEXT("A thriving prehistoric community");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPrehistoricArchitectureDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString DataAssetName = TEXT("Prehistoric Architecture Set");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Templates")
    TArray<FBuildingTemplate> BuildingTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlements")
    TArray<FSettlementLayout> SettlementLayouts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialParameterCollection> ArchitecturalMaterialParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<EConstructionMaterial, TSoftObjectPtr<UMaterialInterface>> MaterialLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<UPCGGraph> BuildingPlacementGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<UPCGGraph> InteriorDecorationGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableStreamingLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxBuildingsPerCluster = 20;
};

/**
 * APrehistoricArchitectureManager
 * 
 * Core actor responsible for managing all architectural structures and interior spaces
 * in the prehistoric world. Handles building placement, interior decoration, and
 * environmental storytelling through architecture.
 * 
 * Key Features:
 * - Procedural building placement using PCG
 * - Interior space management with contextual props
 * - Material weathering and aging systems
 * - Settlement layout generation
 * - Performance optimization for complex structures
 * - Environmental storytelling through architectural details
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrehistoricArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    APrehistoricArchitectureManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPCGComponent* BuildingPlacementPCG;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPCGComponent* InteriorDecorationPCG;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* ArchitecturalBounds;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Data")
    TSoftObjectPtr<UPrehistoricArchitectureDataAsset> ArchitectureDataAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    bool bAutoGenerateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float BuildingDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float SettlementProbability = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    int32 MaxBuildingsInArea = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bEnableEnvironmentalStorytelling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float StorytellingIntensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxDrawDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableOcclusion = true;

    // Runtime Data
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedBuildings;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TMap<AActor*, FBuildingTemplate> BuildingTemplateMap;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 CurrentBuildingCount = 0;

public:
    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllBuildings();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AActor* SpawnBuilding(const FBuildingTemplate& Template, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnSettlement(const FSettlementLayout& Layout, const FVector& CenterLocation);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void DecorateInterior(AActor* Building, const FBuildingTemplate& Template);

    UFUNCTION(BlueprintCallable, Category = "Materials")
    void ApplyMaterialWeathering(AActor* Building, EArchitecturalCondition Condition);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBuildingPerformance();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void ApplyEnvironmentalStorytelling(AActor* Building, const FString& StoryContext);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Placement")
    bool IsValidBuildingLocation(const FVector& Location, const FBuildingTemplate& Template);

    UFUNCTION(BlueprintCallable, Category = "Placement")
    FVector FindNearestWaterSource(const FVector& Location, float SearchRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    float CalculateTerrainSuitability(const FVector& Location, float Radius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Query")
    TArray<AActor*> GetBuildingsInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Query")
    FBuildingTemplate GetBuildingTemplate(AActor* Building);

protected:
    // Internal Functions
    void InitializeComponents();
    void LoadArchitectureData();
    void SetupPCGGraphs();
    void ValidatePlacementParameters();
    
    // Generation Helpers
    TArray<FVector> GenerateBuildingLocations(int32 Count, const FVector& CenterPoint, float Radius);
    FBuildingTemplate SelectAppropriateTemplate(const FVector& Location);
    void ApplyMaterialVariations(AActor* Building, EConstructionMaterial MaterialType);
    void SetupBuildingLOD(AActor* Building, const FBuildingTemplate& Template);
    
    // Performance Optimization
    void UpdateLODLevels();
    void CullDistantBuildings();
    void ManageInstancedComponents();

private:
    // Internal State
    bool bIsInitialized = false;
    float LastPerformanceUpdate = 0.0f;
    const float PerformanceUpdateInterval = 2.0f;
    
    // Cached References
    UPROPERTY()
    UPrehistoricArchitectureDataAsset* CachedArchitectureData;
    
    UPROPERTY()
    TMap<EConstructionMaterial, UMaterialInterface*> CachedMaterials;
    
    UPROPERTY()
    TArray<UInstancedStaticMeshComponent*> InstancedComponents;
};