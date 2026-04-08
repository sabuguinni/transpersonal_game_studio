// Copyright Transpersonal Game Studio. All Rights Reserved.
// Jurassic Architecture Core System - Every structure tells a story
// Agent #07 - Architecture & Interior Agent
// CYCLE_ID: REQ-20260408-MKT-003

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "PCGComponent.h"
#include "ArchitectureTypes.h"
#include "../Environment/JurassicEnvironmentArtCore.h"
#include "JurassicArchitectureCore.generated.h"

/**
 * ARCHITECTURE PHILOSOPHY:
 * Every structure is a document of human presence in the Jurassic world.
 * No interior is empty - every space tells the story of who lived there,
 * what they needed, what they feared, and what happened to them.
 * 
 * Stewart Brand's "How Buildings Learn" principle:
 * Buildings are not just structures - they are time made visible in layers.
 * 
 * Gaston Bachelard's "Poetics of Space" principle:
 * Inhabited space carries the memory and dreams of its inhabitants.
 */

UENUM(BlueprintType)
enum class EArchitecturalPeriod : uint8
{
    Emergency       UMETA(DisplayName = "Emergency Construction - First Days"),
    Survival        UMETA(DisplayName = "Survival Phase - Weeks to Months"),
    Settlement      UMETA(DisplayName = "Settlement Phase - Months to Years"),
    Established     UMETA(DisplayName = "Established Community - Years"),
    Abandoned       UMETA(DisplayName = "Abandoned - Nature Reclaiming"),
    Ruins           UMETA(DisplayName = "Ruins - Archaeological Remnants")
};

UENUM(BlueprintType)
enum class EStructuralIntegrity : uint8
{
    Perfect         UMETA(DisplayName = "Perfect - Recently Built"),
    Excellent       UMETA(DisplayName = "Excellent - Well Maintained"),
    Good           UMETA(DisplayName = "Good - Minor Wear"),
    Fair           UMETA(DisplayName = "Fair - Noticeable Damage"),
    Poor           UMETA(DisplayName = "Poor - Major Damage"),
    Failing        UMETA(DisplayName = "Failing - Structural Issues"),
    Collapsed      UMETA(DisplayName = "Collapsed - Partial Ruins"),
    Destroyed      UMETA(DisplayName = "Destroyed - Complete Ruins")
};

UENUM(BlueprintType)
enum class EInhabitationStory : uint8
{
    ActiveFamily        UMETA(DisplayName = "Active Family Home"),
    SingleSurvivor      UMETA(DisplayName = "Lone Survivor Shelter"),
    CommunitySpace      UMETA(DisplayName = "Community Gathering Place"),
    WorkshopCraft       UMETA(DisplayName = "Craft Workshop"),
    StorageCache        UMETA(DisplayName = "Resource Storage"),
    DefensiveOutpost    UMETA(DisplayName = "Defensive Outpost"),
    MedicalShelter      UMETA(DisplayName = "Medical/Recovery Shelter"),
    RitualSpace         UMETA(DisplayName = "Ritual/Spiritual Space"),
    EmergencyRefuge     UMETA(DisplayName = "Emergency Refuge"),
    AbandonedHurried    UMETA(DisplayName = "Abandoned in Haste"),
    AbandonedPlanned    UMETA(DisplayName = "Planned Abandonment"),
    TragedyScene        UMETA(DisplayName = "Site of Tragedy"),
    MysterySite         UMETA(DisplayName = "Mysterious Abandonment")
};

UENUM(BlueprintType)
enum class EInteriorDensity : uint8
{
    Minimal         UMETA(DisplayName = "Minimal - Emergency Shelter"),
    Basic          UMETA(DisplayName = "Basic - Essential Items Only"),
    Functional     UMETA(DisplayName = "Functional - Daily Life Items"),
    Comfortable    UMETA(DisplayName = "Comfortable - Well-Lived Space"),
    Cluttered      UMETA(DisplayName = "Cluttered - Accumulated Possessions"),
    Abandoned      UMETA(DisplayName = "Abandoned - Items Left Behind"),
    Ransacked      UMETA(DisplayName = "Ransacked - Disturbed/Searched")
};

USTRUCT(BlueprintType)
struct FArchitecturalMaterial
{
    GENERATED_BODY()

    // Primary construction material
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    EConstructionMaterial PrimaryMaterial = EConstructionMaterial::Wood;

    // Material quality and processing level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float MaterialQuality = 0.5f; // 0 = crude, 1 = mastercraft

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float ProcessingLevel = 0.3f; // 0 = raw materials, 1 = refined

    // Weathering and aging
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float WeatheringLevel = 0.2f; // 0 = new, 1 = completely weathered

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float MossGrowth = 0.1f; // 0 = clean, 1 = moss-covered

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float WaterDamage = 0.1f; // 0 = dry, 1 = water-damaged

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float InsectDamage = 0.05f; // 0 = intact, 1 = insect-eaten

    // Visual materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    UMaterialInterface* BaseMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    TArray<UMaterialInterface*> WeatheredVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    TArray<UMaterialInterface*> DamagedVariations;
};

USTRUCT(BlueprintType)
struct FInteriorProp
{
    GENERATED_BODY()

    // Prop identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    FString PropName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    UStaticMesh* PropMesh = nullptr;

    // Placement data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FTransform RelativeTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bAttachToWall = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bAttachToFloor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bAttachToCeiling = false;

    // Story context
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryContext; // What this prop tells about the inhabitants

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsClue = false; // Can be investigated for story information

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ClueText; // Text revealed when investigated

    // Condition and state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    float ConditionLevel = 1.0f; // 0 = broken, 1 = perfect

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    bool bIsUsable = true; // Can player interact with it

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    bool bIsMoveable = false; // Can be moved by player

    // Material override
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    FArchitecturalMaterial PropMaterial;

    // Spawning probability
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float SpawnProbability = 1.0f; // 0-1 chance to spawn

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 Priority = 1; // Higher priority spawns first
};

USTRUCT(BlueprintType)
struct FInteriorLayout
{
    GENERATED_BODY()

    // Layout identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    FString LayoutName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    EInhabitationStory StoryType = EInhabitationStory::ActiveFamily;

    // Space definition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space")
    FVector InteriorBounds = FVector(400.0f, 400.0f, 250.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space")
    EInteriorDensity Density = EInteriorDensity::Functional;

    // Essential props (always spawn)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<FInteriorProp> EssentialProps;

    // Optional props (spawn based on probability)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<FInteriorProp> OptionalProps;

    // Story-specific props
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<FInteriorProp> StoryProps;

    // Environmental storytelling elements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bHasFireplace = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bHasSleepingArea = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bHasWorkArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bHasStorageArea = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bHasCookingArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bHasBloodStains = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bHasStruggleSigns = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bHasPersonalItems = true;

    // Lighting and atmosphere
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AmbientLightLevel = 0.3f; // 0 = dark, 1 = bright

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor AmbientLightColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    USoundCue* AmbientSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<UParticleSystem*> AtmosphericEffects; // Dust motes, etc.
};

USTRUCT(BlueprintType)
struct FArchitecturalBlueprint
{
    GENERATED_BODY()

    // Structure identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EArchitectureType StructureType = EArchitectureType::BasicShelter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EArchitecturalPeriod Period = EArchitecturalPeriod::Survival;

    // Physical characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    UStaticMesh* ExteriorMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    TArray<UStaticMesh*> ExteriorVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    FVector StructureDimensions = FVector(500.0f, 500.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    FArchitecturalMaterial ConstructionMaterial;

    // Interior layouts (multiple possible layouts for variety)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FInteriorLayout> PossibleLayouts;

    // Environmental requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MinDistanceFromWater = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MaxDistanceFromWater = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MaxSlope = 15.0f; // Maximum terrain slope for placement

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MinClearance = 200.0f; // Minimum clear space around structure

    // Spawning parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnProbability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 MaxInstancesPerBiome = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float MinDistanceBetweenSameType = 1000.0f;
};

/**
 * Data Asset that defines all architectural blueprints for the game
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UJurassicArchitectureDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // All available architectural blueprints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArchitecturalBlueprint> ArchitecturalBlueprints;

    // Material libraries
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<EConstructionMaterial, FArchitecturalMaterial> MaterialLibrary;

    // Prop libraries organized by function
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TMap<FString, TArray<FInteriorProp>> PropLibrary;

    // Story templates for different scenarios
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TMap<EInhabitationStory, FInteriorLayout> StoryTemplates;

    // Get blueprint by type
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArchitecturalBlueprint GetBlueprintByType(EArchitectureType Type) const;

    // Get random blueprint for period
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArchitecturalBlueprint GetRandomBlueprintForPeriod(EArchitecturalPeriod Period) const;

    // Get material configuration
    UFUNCTION(BlueprintCallable, Category = "Materials")
    FArchitecturalMaterial GetMaterialConfig(EConstructionMaterial Material) const;
};

/**
 * Main Architecture System Actor
 * Manages all architectural elements in the world
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AJurassicArchitectureCore : public AActor
{
    GENERATED_BODY()

public:
    AJurassicArchitectureCore();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPCGComponent* PCGComponent;

    // Architecture data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    UJurassicArchitectureDataAsset* ArchitectureData;

    // Generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float StructureDensity = 0.05f; // Structures per square kilometer

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinDistanceBetweenStructures = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MaxDistanceBetweenStructures = 2000.0f;

    // Environmental influence weights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Influence")
    float WaterProximityWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Influence")
    float ElevationWeight = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Influence")
    float VegetationDensityWeight = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Influence")
    float SlopeWeight = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Influence")
    float DefensibilityWeight = 0.7f;

    // Narrative parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float AbandonmentRate = 0.4f; // Percentage of structures that are abandoned

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TragedyRate = 0.1f; // Percentage that show signs of tragedy

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float MysteryRate = 0.05f; // Percentage with mysterious circumstances

public:
    // Architecture generation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateArchitectureInArea(FVector Center, float Radius, int32 MaxStructures = 10);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearArchitectureInArea(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegenerateAllArchitecture();

    // Structure spawning
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    class AJurassicStructure* SpawnStructureAtLocation(const FVector& Location, 
                                                       const FArchitecturalBlueprint& Blueprint,
                                                       const FRotator& Rotation = FRotator::ZeroRotator);

    // Environmental analysis
    UFUNCTION(BlueprintCallable, Category = "Analysis")
    float AnalyzePlacementSuitability(const FVector& Location, EArchitectureType StructureType) const;

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    EArchitectureType DetermineOptimalStructureType(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    FRotator CalculateOptimalOrientation(const FVector& Location) const;

    // Story generation
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    EInhabitationStory GenerateStoryForLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FInteriorLayout GenerateInteriorLayout(EInhabitationStory Story, EArchitectureType StructureType) const;

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<FVector> FindValidPlacementLocations(FVector Center, float Radius, int32 MaxLocations) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsLocationValidForStructure(const FVector& Location, EArchitectureType StructureType) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetTerrainSlope(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToNearestWater(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetVegetationDensity(const FVector& Location) const;

protected:
    // Internal generation helpers
    void InitializeArchitectureData();
    void SetupPCGGeneration();
    bool ValidateLocationForPlacement(const FVector& Location, const FArchitecturalBlueprint& Blueprint) const;
    void ApplyEnvironmentalInfluences(FVector& Location, FRotator& Rotation) const;

private:
    // Cached data for performance
    TArray<FVector> CachedWaterLocations;
    TArray<FVector> CachedStructureLocations;
    bool bDataCacheValid = false;
    float LastCacheUpdateTime = 0.0f;
    static constexpr float CacheUpdateInterval = 30.0f; // Update cache every 30 seconds
};