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
#include "FoliageType_InstancedStaticMesh.h"
#include "../WorldGeneration/BiomeSystem.h"
#include "VegetationSystem.generated.h"

/**
 * @brief Advanced Vegetation System for Prehistoric World
 * 
 * Creates living, breathing ecosystems that tell environmental stories.
 * Every plant placement follows ecological principles and narrative purpose.
 * 
 * Core Philosophy:
 * - Vegetation reveals the history of the land
 * - Each cluster tells a story about water, soil, and time
 * - Realistic distribution based on real-world botanical principles
 * - Performance-first approach with Nanite and instancing
 * 
 * Key Features:
 * - Biome-specific vegetation distribution
 * - Realistic clustering and competition patterns
 * - Environmental storytelling through plant health and distribution
 * - Dynamic wind response and seasonal variation
 * - Nanite-optimized high-density foliage
 * - PCG-driven procedural placement with artistic control
 * 
 * @author Environment Artist — Agent #6
 * @version 1.0 — March 2026
 */

/** Vegetation layer types for hierarchical distribution */
UENUM(BlueprintType)
enum class EVegetationLayer : uint8
{
    Canopy          UMETA(DisplayName = "Canopy Layer"),      // 20-40m height
    Understory      UMETA(DisplayName = "Understory"),       // 5-20m height
    Shrub           UMETA(DisplayName = "Shrub Layer"),      // 1-5m height
    Herbaceous      UMETA(DisplayName = "Herbaceous"),       // 0-1m height
    Ground          UMETA(DisplayName = "Ground Cover"),     // 0-0.3m height
    Emergent        UMETA(DisplayName = "Emergent"),         // 40m+ height
    Aquatic         UMETA(DisplayName = "Aquatic"),          // Water plants
    Epiphytic       UMETA(DisplayName = "Epiphytic")         // Growing on trees
};

/** Plant health states for environmental storytelling */
UENUM(BlueprintType)
enum class EPlantHealthState : uint8
{
    Thriving        UMETA(DisplayName = "Thriving"),
    Healthy         UMETA(DisplayName = "Healthy"),
    Stressed        UMETA(DisplayName = "Stressed"),
    Struggling      UMETA(DisplayName = "Struggling"),
    Dying           UMETA(DisplayName = "Dying"),
    Dead            UMETA(DisplayName = "Dead"),
    Fossilized      UMETA(DisplayName = "Fossilized")
};

/** Vegetation interaction types with dinosaurs */
UENUM(BlueprintType)
enum class EVegetationInteraction : uint8
{
    None            UMETA(DisplayName = "No Interaction"),
    Edible          UMETA(DisplayName = "Edible"),
    Shelter         UMETA(DisplayName = "Provides Shelter"),
    NestingMaterial UMETA(DisplayName = "Nesting Material"),
    Destructible    UMETA(DisplayName = "Can Be Destroyed"),
    Climbable       UMETA(DisplayName = "Climbable"),
    Medicinal       UMETA(DisplayName = "Medicinal Properties")
};

/** Detailed vegetation species data */
USTRUCT(BlueprintType)
struct FVegetationSpeciesData
{
    GENERATED_BODY()

    /** Species identification */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString SpeciesName = "Prehistoric Fern";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString ScientificName = "Cyathea jurassica";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FText Description = NSLOCTEXT("Vegetation", "FernDesc", "A large tree fern common in humid prehistoric forests");

    /** Visual assets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<UStaticMesh> HealthyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<UStaticMesh> StressedMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<UStaticMesh> DeadMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<TSoftObjectPtr<UMaterialInterface>> SeasonalMaterials;

    /** Ecological requirements */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology")
    EVegetationLayer PreferredLayer = EVegetationLayer::Understory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology")
    FVector2D OptimalTemperatureRange = FVector2D(18.0f, 28.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology")
    FVector2D OptimalHumidityRange = FVector2D(60.0f, 90.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology")
    FVector2D WaterDistancePreference = FVector2D(50.0f, 500.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology")
    FVector2D SoilRichnessRange = FVector2D(0.3f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology")
    FVector2D SlopeToleranceRange = FVector2D(0.0f, 35.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecology")
    float SunlightRequirement = 0.6f; // 0 = shade tolerant, 1 = full sun

    /** Growth characteristics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
    FVector2D MatureHeightRange = FVector2D(300.0f, 800.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
    FVector2D MatureWidthRange = FVector2D(200.0f, 600.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
    float GrowthRate = 1.0f; // Relative to other species

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
    int32 LifespanYears = 50;

    /** Distribution patterns */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float ClusteringTendency = 0.4f; // 0 = random, 1 = highly clustered

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float CompetitionRadius = 300.0f; // Distance affecting other plants

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float DensityPerSquareMeter = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    TArray<FString> CompanionSpecies; // Species that grow well together

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    TArray<FString> CompetitorSpecies; // Species that compete for resources

    /** Biome preferences */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EBiomeType, float> BiomeAffinities; // 0-1 preference for each biome

    /** Dinosaur interactions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    TArray<EVegetationInteraction> InteractionTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    float NutritionalValue = 0.5f; // For herbivores

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    float ToxicityLevel = 0.0f; // 0 = safe, 1 = deadly

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    TArray<FString> AttractedDinosaurTypes;

    /** Performance settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseNanite = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<float> LODDistances = {1000.0f, 2500.0f, 5000.0f};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxInstancesPerComponent = 1000;

    /** Wind and animation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float WindResponseStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float WindFrequencyMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bHasSeasonalVariation = true;
};

/** Vegetation cluster data for realistic distribution */
USTRUCT(BlueprintType)
struct FVegetationCluster
{
    GENERATED_BODY()

    /** Cluster center location */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    FVector ClusterCenter = FVector::ZeroVector;

    /** Cluster radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    float ClusterRadius = 1000.0f;

    /** Primary species in this cluster */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    FString DominantSpecies;

    /** Secondary species with their relative abundance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    TMap<FString, float> SecondarySpecies;

    /** Cluster health state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    EPlantHealthState ClusterHealth = EPlantHealthState::Healthy;

    /** Environmental story this cluster tells */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EnvironmentalStory = "Thriving ecosystem near water source";

    /** Age of this cluster (affects appearance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float ClusterAgeYears = 25.0f;

    /** Recent disturbance events */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> DisturbanceHistory;
};

/** Main vegetation system class */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVegetationSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UVegetationSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Initialize vegetation system with world data */
    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void InitializeVegetationSystem(UWorld* World);

    /** Generate vegetation for a specific biome area */
    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void GenerateVegetationForBiome(const FVector& WorldLocation, float Radius, EBiomeType BiomeType);

    /** Create a vegetation cluster at specific location */
    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void CreateVegetationCluster(const FVegetationCluster& ClusterData);

    /** Update vegetation based on environmental changes */
    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void UpdateVegetationHealth(const FVector& Location, float Radius, EPlantHealthState NewHealth);

    /** Get vegetation species data */
    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    FVegetationSpeciesData GetSpeciesData(const FString& SpeciesName) const;

    /** Register a new vegetation species */
    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void RegisterVegetationSpecies(const FString& SpeciesName, const FVegetationSpeciesData& SpeciesData);

    /** Get all vegetation in area */
    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    TArray<AActor*> GetVegetationInArea(const FVector& Center, float Radius) const;

    /** Remove vegetation in area (for dinosaur destruction) */
    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void RemoveVegetationInArea(const FVector& Center, float Radius, bool bGradual = true);

    /** Spawn environmental storytelling props */
    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void SpawnEnvironmentalProps(const FVector& Location, const FString& StoryType);

protected:
    /** All registered vegetation species */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Database")
    TMap<FString, FVegetationSpeciesData> VegetationDatabase;

    /** Active vegetation clusters in the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World State")
    TArray<FVegetationCluster> ActiveClusters;

    /** PCG components for each biome type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TMap<EBiomeType, TSoftObjectPtr<UPCGGraph>> BiomeVegetationGraphs;

    /** Foliage components for instanced rendering */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    TMap<FString, UHierarchicalInstancedStaticMeshComponent*> VegetationComponents;

    /** Current world reference */
    UPROPERTY()
    TWeakObjectPtr<UWorld> CurrentWorld;

private:
    /** Initialize default vegetation species */
    void InitializeDefaultSpecies();

    /** Calculate optimal placement for vegetation */
    TArray<FVector> CalculateVegetationPlacements(const FVegetationSpeciesData& Species, 
                                                  const FVector& AreaCenter, 
                                                  float AreaRadius, 
                                                  EBiomeType BiomeType);

    /** Evaluate site suitability for species */
    float EvaluateSiteSuitability(const FVegetationSpeciesData& Species, 
                                  const FVector& Location, 
                                  EBiomeType BiomeType);

    /** Create instanced mesh component for species */
    UHierarchicalInstancedStaticMeshComponent* CreateVegetationComponent(const FString& SpeciesName);

    /** Apply environmental storytelling to vegetation placement */
    void ApplyEnvironmentalNarrative(FVegetationCluster& Cluster);
};