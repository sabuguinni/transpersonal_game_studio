// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"
#include "ArchitectureSubsystem.generated.h"

class UStaticMesh;
class UMaterialInterface;
class ALandscape;
class APCGWorldActor;

/**
 * @brief Architecture & Interior Subsystem for Transpersonal Game Studio
 * 
 * Creates every structure built by humans in the prehistoric world — edifícios, interiores, 
 * ruínas, monumentos — with the conviction that each structure is a historical document 
 * of the civilization that built it.
 * 
 * Core Philosophy:
 * "Buildings tell time in layers" (Stewart Brand) - cada estrutura mostra camadas de uso
 * "Inhabited space has memory" (Gaston Bachelard) - nenhum interior está vazio de história
 * 
 * Every interior must answer: who lived here, and what happened to them?
 * 
 * Architecture Types:
 * - Emergency Shelters: Quick survival structures (lean-tos, debris huts)
 * - Temporary Habitats: Semi-permanent camps (elevated platforms, thatched huts)
 * - Permanent Bases: Long-term settlements (stone foundations, defensive walls)
 * - Utility Structures: Specialized buildings (storage, workshops, watchtowers)
 * - Ruins: Abandoned structures showing passage of time
 * - Monuments: Ceremonial or memorial structures
 * 
 * Interior Design Principles:
 * - Every object has a purpose and tells a story
 * - Personal belongings reveal character and needs
 * - Wear patterns show daily routines
 * - Damage patterns reveal threats and conflicts
 * - Empty spaces are lies - humans leave traces everywhere
 * 
 * Technical Implementation:
 * - Modular building system with procedural variation
 * - Interior furnishing system with narrative props
 * - Weathering and decay simulation
 * - Structural integrity system
 * - Performance-optimized LOD chains
 * - Integration with PCG for placement
 * 
 * @author Architecture & Interior Agent — Agent #7
 * @version 1.0 — March 2026
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArchitectureSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UArchitectureSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    /** Main architecture functions */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateArchitecture(const FArchitectureGenerationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RefreshRegion(const FBox& WorldBounds);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllStructures();

    /** Building construction */
    UFUNCTION(BlueprintCallable, Category = "Building")
    AActor* ConstructBuilding(const FBuildingConstructionData& ConstructionData);

    UFUNCTION(BlueprintCallable, Category = "Building")
    void FurnishInterior(AActor* Building, const FInteriorFurnishingData& FurnishingData);

    /** Ruins and decay */
    UFUNCTION(BlueprintCallable, Category = "Ruins")
    void ConvertToRuins(AActor* Building, const FRuinsConversionData& ConversionData);

    UFUNCTION(BlueprintCallable, Category = "Ruins")
    void ApplyWeathering(AActor* Structure, float WeatheringIntensity);

    /** Settlement generation */
    UFUNCTION(BlueprintCallable, Category = "Settlement")
    void GenerateSettlement(const FSettlementData& SettlementData);

    /** Performance monitoring */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FArchitecturePerformanceStats GetPerformanceStats() const;

protected:
    /** Architecture types */
    UENUM(BlueprintType)
    enum class EArchitectureType : uint8
    {
        // Survival structures
        DebrisHut               UMETA(DisplayName = "Debris Hut"),
        LeanToShelter           UMETA(DisplayName = "Lean-To Shelter"),
        RockShelter             UMETA(DisplayName = "Rock Shelter"),
        TreePlatform            UMETA(DisplayName = "Tree Platform"),
        
        // Temporary habitats
        ThatchedHut             UMETA(DisplayName = "Thatched Hut"),
        ElevatedPlatform        UMETA(DisplayName = "Elevated Platform"),
        SemiSubterranean        UMETA(DisplayName = "Semi-Subterranean"),
        BambooStructure         UMETA(DisplayName = "Bamboo Structure"),
        
        // Permanent bases
        StoneFoundationHouse    UMETA(DisplayName = "Stone Foundation House"),
        LogCabin                UMETA(DisplayName = "Log Cabin"),
        CliffDwelling           UMETA(DisplayName = "Cliff Dwelling"),
        UndergroundBunker       UMETA(DisplayName = "Underground Bunker"),
        
        // Utility structures
        Watchtower              UMETA(DisplayName = "Watchtower"),
        StorageCache            UMETA(DisplayName = "Storage Cache"),
        Workshop                UMETA(DisplayName = "Workshop"),
        Smokehouse              UMETA(DisplayName = "Smokehouse"),
        WaterCistern            UMETA(DisplayName = "Water Cistern"),
        DefensiveWall           UMETA(DisplayName = "Defensive Wall"),
        
        // Ruins and monuments
        AncientRuins            UMETA(DisplayName = "Ancient Ruins"),
        CollapseStructure       UMETA(DisplayName = "Collapsed Structure"),
        StoneMonument           UMETA(DisplayName = "Stone Monument"),
        BurialCairn             UMETA(DisplayName = "Burial Cairn")
    };

    /** Structural condition states */
    UENUM(BlueprintType)
    enum class EStructuralCondition : uint8
    {
        Pristine        UMETA(DisplayName = "Pristine - Just Built"),
        Excellent       UMETA(DisplayName = "Excellent - Well Maintained"),
        Good            UMETA(DisplayName = "Good - Minor Wear"),
        Fair            UMETA(DisplayName = "Fair - Noticeable Wear"),
        Poor            UMETA(DisplayName = "Poor - Significant Damage"),
        Deteriorating   UMETA(DisplayName = "Deteriorating - Major Issues"),
        Ruined          UMETA(DisplayName = "Ruined - Barely Standing"),
        Collapsed       UMETA(DisplayName = "Collapsed - Destroyed")
    };

    /** Interior furnishing categories */
    UENUM(BlueprintType)
    enum class EInteriorCategory : uint8
    {
        // Basic needs
        Sleeping                UMETA(DisplayName = "Sleeping Area"),
        Cooking                 UMETA(DisplayName = "Cooking Area"),
        Storage                 UMETA(DisplayName = "Storage"),
        WorkArea                UMETA(DisplayName = "Work Area"),
        
        // Personal items
        PersonalBelongings      UMETA(DisplayName = "Personal Belongings"),
        Clothing                UMETA(DisplayName = "Clothing"),
        Tools                   UMETA(DisplayName = "Tools"),
        Weapons                 UMETA(DisplayName = "Weapons"),
        
        // Comfort items
        Seating                 UMETA(DisplayName = "Seating"),
        Lighting                UMETA(DisplayName = "Lighting"),
        Decoration              UMETA(DisplayName = "Decoration"),
        
        // Survival items
        FoodPreservation        UMETA(DisplayName = "Food Preservation"),
        WaterStorage            UMETA(DisplayName = "Water Storage"),
        MedicalSupplies         UMETA(DisplayName = "Medical Supplies"),
        
        // Signs of abandonment
        AbandonmentSigns        UMETA(DisplayName = "Abandonment Signs"),
        DamageEvidence          UMETA(DisplayName = "Damage Evidence"),
        NatureReclaim           UMETA(DisplayName = "Nature Reclaiming")
    };

    /** Building construction data */
    USTRUCT(BlueprintType)
    struct FBuildingConstructionData
    {
        GENERATED_BODY()

        /** Type of building to construct */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EArchitectureType BuildingType = EArchitectureType::DebrisHut;

        /** Location to build */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FVector Location = FVector::ZeroVector;

        /** Building rotation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FRotator Rotation = FRotator::ZeroRotator;

        /** Building scale variation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FVector Scale = FVector::OneVector;

        /** Structural condition */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EStructuralCondition Condition = EStructuralCondition::Good;

        /** Age of structure (affects weathering) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float AgeInDays = 30.0f;

        /** Whether to generate interior */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bGenerateInterior = true;

        /** Whether structure is abandoned */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bIsAbandoned = false;

        /** Abandonment reason (affects interior state) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString AbandonmentReason = TEXT("Unknown");

        /** Custom materials override */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<UMaterialInterface*> CustomMaterials;
    };

    /** Interior furnishing data */
    USTRUCT(BlueprintType)
    struct FInteriorFurnishingData
    {
        GENERATED_BODY()

        /** Categories to furnish */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<EInteriorCategory> CategoriesToFurnish;

        /** Density of furnishing (0-1) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float FurnishingDensity = 0.7f;

        /** Personal story elements */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString OccupantPersonality = TEXT("Practical Survivor");

        /** Signs of struggle or conflict */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bShowConflictSigns = false;

        /** Time since last inhabited */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float DaysSinceAbandoned = 0.0f;

        /** Specific props to include */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FString> SpecificProps;

        /** Narrative elements to include */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FString> NarrativeElements;
    };

    /** Ruins conversion data */
    USTRUCT(BlueprintType)
    struct FRuinsConversionData
    {
        GENERATED_BODY()

        /** Destruction cause */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString DestructionCause = TEXT("Time and Weather");

        /** Percentage of structure remaining */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float StructuralIntegrity = 0.3f;

        /** Nature reclamation level */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float NatureReclamationLevel = 0.5f;

        /** Scatter debris around ruins */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bScatterDebris = true;

        /** Add vegetation growth */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bAddVegetationGrowth = true;

        /** Preserve some interior elements */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bPreserveSomeInterior = true;
    };

    /** Settlement generation data */
    USTRUCT(BlueprintType)
    struct FSettlementData
    {
        GENERATED_BODY()

        /** Settlement center location */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FVector CenterLocation = FVector::ZeroVector;

        /** Settlement radius */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Radius = 5000.0f; // 50 meters

        /** Number of buildings */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 BuildingCount = 8;

        /** Settlement type influences building types */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString SettlementType = TEXT("Temporary Camp");

        /** Population estimate (affects building sizes) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 EstimatedPopulation = 15;

        /** Settlement age (affects condition) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float AgeInDays = 60.0f;

        /** Defensive features */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bIncludeDefenses = true;

        /** Communal areas */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bIncludeCommunalAreas = true;

        /** Settlement story/background */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString SettlementStory = TEXT("A group of survivors establishing a base");
    };

    /** Architecture generation settings */
    USTRUCT(BlueprintType)
    struct FArchitectureGenerationSettings
    {
        GENERATED_BODY()

        /** Overall density of structures */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float StructureDensity = 0.1f;

        /** Minimum distance between structures */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MinStructureDistance = 1000.0f; // 10 meters

        /** Generate abandoned structures */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bGenerateAbandonedStructures = true;

        /** Abandonment probability */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float AbandonmentProbability = 0.3f;

        /** Generate defensive structures */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bGenerateDefensiveStructures = true;

        /** Generate settlements */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bGenerateSettlements = true;

        /** Settlement probability */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float SettlementProbability = 0.05f;

        /** Quality level for architecture */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 QualityLevel = 3; // 1-5 scale

        /** Enable interior generation */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bGenerateInteriors = true;

        /** Enable storytelling elements */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bEnableStorytellingElements = true;
    };

    /** Performance statistics */
    USTRUCT(BlueprintType)
    struct FArchitecturePerformanceStats
    {
        GENERATED_BODY()

        /** Total structures generated */
        UPROPERTY(BlueprintReadOnly)
        int32 TotalStructures = 0;

        /** Total interior props */
        UPROPERTY(BlueprintReadOnly)
        int32 TotalInteriorProps = 0;

        /** Memory usage in MB */
        UPROPERTY(BlueprintReadOnly)
        float MemoryUsageMB = 0.0f;

        /** Generation time in seconds */
        UPROPERTY(BlueprintReadOnly)
        float GenerationTimeSeconds = 0.0f;

        /** Average FPS impact */
        UPROPERTY(BlueprintReadOnly)
        float AverageFPSImpact = 0.0f;
    };

    /** Internal data structures */
    UPROPERTY()
    TArray<AActor*> GeneratedStructures;

    UPROPERTY()
    TArray<UHierarchicalInstancedStaticMeshComponent*> InteriorPropComponents;

    /** Asset references */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    UDataTable* BuildingMeshTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    UDataTable* InteriorPropsTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    UDataTable* RuinsVariationsTable;

    /** Material references */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> WoodMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> StoneMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> ThatchMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> WeatheredMaterials;

    /** Performance tracking */
    UPROPERTY()
    FArchitecturePerformanceStats CurrentPerformanceStats;

private:
    /** Internal helper functions */
    void LoadAssetReferences();
    void InitializePerformanceTracking();
    
    AActor* CreateBuildingActor(const FBuildingConstructionData& ConstructionData);
    void GenerateInteriorProps(AActor* Building, const FInteriorFurnishingData& FurnishingData);
    void ApplyWeatheringEffects(AActor* Structure, float WeatheringIntensity);
    void CreateRuinsFromBuilding(AActor* Building, const FRuinsConversionData& ConversionData);
    
    bool ValidateBuildingLocation(const FVector& Location, float MinDistance);
    FVector FindSuitableBuildingLocation(const FVector& PreferredLocation, float SearchRadius);
    
    void UpdatePerformanceStats();
    void OptimizeStructuresForPerformance();
};