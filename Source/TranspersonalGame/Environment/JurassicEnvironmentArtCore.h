// Copyright Transpersonal Game Studio. All Rights Reserved.
// Environment Art Core System - Central hub for all environmental art generation
// Agent #6 - Environment Artist

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Engine/Texture2D.h"
#include "Landscape.h"
#include "ProceduralFoliageSpawner.h"
#include "PCGComponent.h"
#include "../WorldGeneration/JurassicBiomeManager.h"
#include "JurassicEnvironmentArtCore.generated.h"

/**
 * Comprehensive Environment Art System for Jurassic World
 * Handles vegetation, rocks, props, materials, and environmental storytelling
 * Integrates with PCG Framework for performance and scalability
 */

UENUM(BlueprintType)
enum class EEnvironmentArtLayer : uint8
{
    Terrain         UMETA(DisplayName = "Terrain Base"),
    Vegetation      UMETA(DisplayName = "Vegetation"),
    Props           UMETA(DisplayName = "Environment Props"),
    Rocks           UMETA(DisplayName = "Rock Formations"),
    Water           UMETA(DisplayName = "Water Features"),
    Atmosphere      UMETA(DisplayName = "Atmospheric Effects"),
    Storytelling    UMETA(DisplayName = "Narrative Elements"),
    Performance     UMETA(DisplayName = "Performance Optimization")
};

UENUM(BlueprintType)
enum class EDetailLevel : uint8
{
    Hero        UMETA(DisplayName = "Hero - Maximum Detail"),
    High        UMETA(DisplayName = "High - Near Player"),
    Medium      UMETA(DisplayName = "Medium - Mid Distance"),
    Low         UMETA(DisplayName = "Low - Far Distance"),
    Culled      UMETA(DisplayName = "Culled - Not Rendered")
};

UENUM(BlueprintType)
enum class EEnvArt_NarrativeTheme : uint8
{
    Ancient             UMETA(DisplayName = "Ancient Geological"),
    DinosaurActivity    UMETA(DisplayName = "Dinosaur Activity"),
    PlayerStory         UMETA(DisplayName = "Player Story Clues"),
    GemaClues           UMETA(DisplayName = "Gema Mystery"),
    NaturalBeauty       UMETA(DisplayName = "Natural Beauty"),
    Danger              UMETA(DisplayName = "Environmental Danger"),
    Sanctuary           UMETA(DisplayName = "Safe Haven")
};

USTRUCT(BlueprintType)
struct FEnvironmentMaterial
{
    GENERATED_BODY()

    // Base material
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    UMaterialInterface* BaseMaterial = nullptr;

    // Material variations for diversity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    TArray<UMaterialInterface*> MaterialVariations;

    // Weathering materials (wet, dry, moss-covered, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    TArray<UMaterialInterface*> WeatheringStates;

    // Blend weights for different environmental conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float MoistureInfluence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float SunlightInfluence = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float AgeInfluence = 0.2f;
};

USTRUCT(BlueprintType)
struct FVegetationCluster
{
    GENERATED_BODY()

    // Vegetation meshes in this cluster
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<UStaticMesh*> VegetationMeshes;

    // Cluster density and distribution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float ClusterRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 MinInstances = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 MaxInstances = 20;

    // Growth patterns
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bFollowTerrain = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector2D ScaleRange = FVector2D(0.7f, 1.3f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bRandomRotation = true;

    // Environmental requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MinMoisture = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MaxMoisture = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MinSlope = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MaxSlope = 45.0f;

    // Materials for this vegetation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    FEnvironmentMaterial VegetationMaterials;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bCastShadows = true;
};

USTRUCT(BlueprintType)
struct FRockFormationCluster
{
    GENERATED_BODY()

    // Rock meshes for this formation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    TArray<UStaticMesh*> RockMeshes;

    // Formation characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    int32 MinRocks = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    int32 MaxRocks = 12;

    // Size and rotation variation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector2D ScaleRange = FVector2D(0.5f, 2.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bRandomRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bPartialBurial = true; // Some rocks partially buried

    // Geological storytelling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bShowGeologicalAge = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bShowWeathering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bAllowVegetationGrowth = true;

    // Materials for different rock types and weathering
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    FEnvironmentMaterial RockMaterials;

    // Moss and lichen growth on rocks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<UStaticMesh*> RockVegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationCoverage = 0.3f; // 0-1 percentage
};

USTRUCT(BlueprintType)
struct FEnvironmentProp
{
    GENERATED_BODY()

    // Prop mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    UStaticMesh* PropMesh = nullptr;

    // Prop category for organization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    FString PropCategory = "General";

    // Spawn probability
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnProbability = 1.0f;

    // Size variation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    // Rotation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    bool bRandomRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    bool bAlignToSurface = true;

    // Environmental requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MinSlope = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MaxSlope = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WaterDistancePreference = 0.0f; // Negative = near water, positive = away

    // Clustering behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering")
    bool bClusterSpawning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering")
    int32 ClusterSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering")
    float ClusterRadius = 200.0f;

    // Narrative context
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    EEnvArt_NarrativeTheme NarrativeTheme = EEnvArt_NarrativeTheme::NaturalBeauty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString StoryContext;

    // Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    FEnvironmentMaterial PropMaterials;

    // Performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EDetailLevel DetailLevel = EDetailLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullDistance = 3000.0f;
};

USTRUCT(BlueprintType)
struct FNarrativeEnvironmentCluster
{
    GENERATED_BODY()

    // Cluster identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString ClusterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    EEnvArt_NarrativeTheme Theme = EEnvArt_NarrativeTheme::NaturalBeauty;

    // Story this cluster tells
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString StoryDescription;

    // Props that compose this story
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<FEnvironmentProp> StoryProps;

    // Decals for ground storytelling (footprints, blood, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decals")
    TArray<UMaterialInterface*> StoryDecals;

    // Audio elements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* AmbientSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<USoundCue*> InteractiveAudio;

    // Particle effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TArray<UParticleSystem*> StoryParticles;

    // Cluster properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    float ClusterRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    float StoryIntensity = 1.0f; // Affects prop density and detail

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    bool bPlayerDiscoverable = true;

    // Gema clue integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gema")
    bool bContainsGemaClue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gema")
    int32 GemaClueIntensity = 0; // 0 = no clue, 5 = strong clue
};

USTRUCT(BlueprintType)
struct FBiomeEnvironmentProfile
{
    GENERATED_BODY()

    // Biome this profile applies to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EJurassicBiomeType BiomeType = EJurassicBiomeType::DenseForest;

    // Vegetation clusters for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FVegetationCluster> VegetationClusters;

    // Rock formations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    TArray<FRockFormationCluster> RockFormations;

    // Environment props
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<FEnvironmentProp> EnvironmentProps;

    // Ground scatter (small details)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Scatter")
    TArray<FEnvironmentProp> GroundScatter;

    // Water-related features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    TArray<FEnvironmentProp> WaterFeatures;

    // Narrative clusters for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarrativeEnvironmentCluster> NarrativeClusters;

    // Atmospheric settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<UParticleSystem*> AtmosphericParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor AmbientLightTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AmbientLightIntensity = 1.0f;

    // Fog and weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bUseFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.02f;

    // Performance settings for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float BiomeLODDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float BiomeCullDistance = 15000.0f;
};

/**
 * Data Asset containing all environment profiles for different biomes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UJurassicEnvironmentDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    // Environment profiles for each biome type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Profiles")
    TMap<EJurassicBiomeType, FBiomeEnvironmentProfile> BiomeProfiles;

    // Global environment settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalVegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalPropDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalRockDensity = 1.0f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxInstancesPerCluster = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GlobalLODDistanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseNaniteForVegetation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseVirtualShadowMaps = true;

    // Narrative settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float NarrativeClusterFrequency = 0.1f; // Clusters per square kilometer

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float GemaClueFrequency = 0.01f; // Gema clues per square kilometer

    // Get environment profile for a specific biome
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment")
    FBiomeEnvironmentProfile GetBiomeProfile(EJurassicBiomeType BiomeType) const;

    // Check if biome profile exists
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment")
    bool HasBiomeProfile(EJurassicBiomeType BiomeType) const;
};

/**
 * Core Environment Art Manager
 * Central system that coordinates all environmental art generation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AJurassicEnvironmentArtCore : public AActor
{
    GENERATED_BODY()

public:
    AJurassicEnvironmentArtCore();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime);

    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPCGComponent* EnvironmentPCGComponent;

    // Environment database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Database")
    UJurassicEnvironmentDatabase* EnvironmentDatabase;

    // Biome manager reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    AJurassicBiomeManager* BiomeManager;

    // Landscape reference for terrain queries
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    ALandscape* TargetLandscape;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateRadius = 5000.0f; // Radius around player to update

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 2.0f; // Updates per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxInstancesPerFrame = 100; // Max instances to spawn per frame

public:
    // Main environment population functions
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PopulateEnvironmentArea(FVector Center, float Radius, EDetailLevel DetailLevel = EDetailLevel::High);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PopulateBiomeArea(FVector Center, float Radius, EJurassicBiomeType BiomeType, EDetailLevel DetailLevel = EDetailLevel::High);

    // Specialized generation functions
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void GenerateVegetation(FVector Center, float Radius, EJurassicBiomeType BiomeType, float DensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Rock Formations")
    void GenerateRockFormations(FVector Center, float Radius, EJurassicBiomeType BiomeType, int32 FormationCount = -1);

    UFUNCTION(BlueprintCallable, Category = "Environment Props")
    void GenerateEnvironmentProps(FVector Center, float Radius, EJurassicBiomeType BiomeType, EDetailLevel DetailLevel);

    UFUNCTION(BlueprintCallable, Category = "Ground Scatter")
    void GenerateGroundScatter(FVector Center, float Radius, EJurassicBiomeType BiomeType, float DensityMultiplier = 1.0f);

    // Narrative environment functions
    UFUNCTION(BlueprintCallable, Category = "Narrative Environment")
    void CreateNarrativeCluster(FVector Location, const FNarrativeEnvironmentCluster& ClusterData);

    UFUNCTION(BlueprintCallable, Category = "Narrative Environment")
    void GenerateNarrativeClusters(FVector Center, float Radius, EJurassicBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Narrative Environment")
    void PlaceGemaClues(FVector Center, float Radius, int32 ClueCount = 1);

    // Environmental storytelling
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateDinosaurActivitySigns(FVector Location, TSubclassOf<class ACharacter> DinosaurClass, float ActivityAge = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateAbandonedCampsite(FVector Location, float AgeInDays = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateBattleAftermath(FVector Location, float BattleIntensity = 1.0f);

    // Performance and optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODLevels(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantEnvironment(FVector PlayerLocation, float CullDistance = 10000.0f);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeEnvironmentForPerformance();

    // Utility functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment Art")
    EJurassicBiomeType GetBiomeAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment Art")
    bool IsLocationSuitableForProp(FVector Location, const FEnvironmentProp& PropData) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment Art")
    float GetTerrainSlope(FVector Location) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment Art")
    float GetDistanceToWater(FVector Location) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment Art")
    float GetMoistureLevel(FVector Location) const;

    // Clear functions
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ClearEnvironmentArea(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ClearAllEnvironment();

    // Material application
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void ApplyBiomeMaterials(FVector Center, float Radius, EJurassicBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Materials")
    void UpdateMaterialsForWeather(FVector Center, float Radius, float MoistureLevel, float Temperature);

protected:
    // Internal generation helpers
    void SpawnVegetationCluster(const FVegetationCluster& Cluster, FVector Center, float Radius);
    void SpawnRockFormation(const FRockFormationCluster& Formation, FVector Center);
    void SpawnEnvironmentProp(const FEnvironmentProp& Prop, FVector Location);
    void SpawnGroundScatterAtLocation(const TArray<FEnvironmentProp>& ScatterProps, FVector Location, float Density);

    // Material helpers
    UMaterialInterface* SelectMaterialVariation(const FEnvironmentMaterial& MaterialData, FVector Location) const;
    void ApplyWeatheringToMaterial(UMaterialInterface* Material, FVector Location, float WeatheringAmount) const;

    // Performance tracking
    UPROPERTY()
    TArray<UHierarchicalInstancedStaticMeshComponent*> VegetationComponents;

    UPROPERTY()
    TArray<UHierarchicalInstancedStaticMeshComponent*> RockComponents;

    UPROPERTY()
    TArray<UHierarchicalInstancedStaticMeshComponent*> PropComponents;

    // Update tracking
    FVector LastPlayerLocation = FVector::ZeroVector;
    float LastUpdateTime = 0.0f;
    int32 InstancesSpawnedThisFrame = 0;

    // Performance statistics
    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 TotalVegetationInstances = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 TotalRockInstances = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 TotalPropInstances = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float LastGenerationTime = 0.0f;
};