// Copyright Transpersonal Game Studio. All Rights Reserved.
// VegetationSystem.h - Prehistoric Vegetation Generation System

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "FoliageType.h"
#include "InstancedFoliageActor.h"
#include "../World/PCGWorldGenerator.h"
#include "VegetationSystem.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UFoliageType;
class AInstancedFoliageActor;

/**
 * Prehistoric Plant Types
 * Based on Jurassic/Cretaceous period flora
 */
UENUM(BlueprintType)
enum class EPrehistoricPlantType : uint8
{
    // Trees
    ConiferTree_Giant       UMETA(DisplayName = "Giant Conifer Tree"),
    ConiferTree_Medium      UMETA(DisplayName = "Medium Conifer Tree"),
    ConiferTree_Small       UMETA(DisplayName = "Small Conifer Tree"),
    CycadTree_Large         UMETA(DisplayName = "Large Cycad Tree"),
    CycadTree_Medium        UMETA(DisplayName = "Medium Cycad Tree"),
    GinkgoTree_Ancient      UMETA(DisplayName = "Ancient Ginkgo Tree"),
    
    // Ferns
    TreeFern_Giant          UMETA(DisplayName = "Giant Tree Fern"),
    TreeFern_Medium         UMETA(DisplayName = "Medium Tree Fern"),
    GroundFern_Large        UMETA(DisplayName = "Large Ground Fern"),
    GroundFern_Small        UMETA(DisplayName = "Small Ground Fern"),
    RoyalFern_Cluster       UMETA(DisplayName = "Royal Fern Cluster"),
    
    // Undergrowth
    Horsetail_Tall          UMETA(DisplayName = "Tall Horsetail"),
    Horsetail_Short         UMETA(DisplayName = "Short Horsetail"),
    Moss_Carpet             UMETA(DisplayName = "Moss Carpet"),
    Liverwort_Patch         UMETA(DisplayName = "Liverwort Patch"),
    
    // Aquatic
    WaterFern_Floating      UMETA(DisplayName = "Floating Water Fern"),
    Algae_Cluster           UMETA(DisplayName = "Algae Cluster"),
    
    // Specialized
    CarnivorousPlant_Large  UMETA(DisplayName = "Large Carnivorous Plant"),
    VolcanicPlant_Heat      UMETA(DisplayName = "Heat-Resistant Volcanic Plant"),
    
    // Decorative
    FloweringPlant_Early    UMETA(DisplayName = "Early Flowering Plant"),
    SeedFern_Decorative     UMETA(DisplayName = "Decorative Seed Fern")
};

/**
 * Vegetation Density Settings
 */
UENUM(BlueprintType)
enum class EVegetationDensity : uint8
{
    Sparse      UMETA(DisplayName = "Sparse (10-30%)"),
    Light       UMETA(DisplayName = "Light (30-50%)"),
    Medium      UMETA(DisplayName = "Medium (50-70%)"),
    Dense       UMETA(DisplayName = "Dense (70-85%)"),
    VeryDense   UMETA(DisplayName = "Very Dense (85-95%)"),
    Jungle      UMETA(DisplayName = "Jungle (95-100%)")
};

/**
 * Plant Instance Data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPlantInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant")
    EPrehistoricPlantType PlantType = EPrehistoricPlantType::GroundFern_Small;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float HealthFactor = 1.0f; // 0.0 = dead, 1.0 = healthy

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    float AgeFactor = 1.0f; // 0.0 = young, 1.0 = mature

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    int32 VariationSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MoistureFactor = 0.5f; // 0.0 = dry, 1.0 = wet

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float SunlightFactor = 0.5f; // 0.0 = shade, 1.0 = full sun

    FPlantInstanceData()
    {
        VariationSeed = FMath::Rand();
    }
};

/**
 * Biome Vegetation Configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeVegetationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
    EVegetationDensity OverallDensity = EVegetationDensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant Distribution")
    TMap<EPrehistoricPlantType, float> PlantTypeWeights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layers")
    float CanopyDensity = 0.3f; // Large trees

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layers")
    float UnderstoryDensity = 0.5f; // Medium plants

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layers")
    float GroundCoverDensity = 0.8f; // Small plants and ferns

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float MoistureLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float TemperatureLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float SoilRichness = 0.5f;

    FBiomeVegetationConfig()
    {
        // Default tropical rainforest configuration
        PlantTypeWeights.Add(EPrehistoricPlantType::ConiferTree_Giant, 0.1f);
        PlantTypeWeights.Add(EPrehistoricPlantType::ConiferTree_Medium, 0.15f);
        PlantTypeWeights.Add(EPrehistoricPlantType::TreeFern_Giant, 0.2f);
        PlantTypeWeights.Add(EPrehistoricPlantType::TreeFern_Medium, 0.25f);
        PlantTypeWeights.Add(EPrehistoricPlantType::GroundFern_Large, 0.3f);
    }
};

/**
 * Plant Asset Reference
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPlantAssetData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TObjectPtr<UStaticMesh> PlantMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TObjectPtr<UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    TArray<TObjectPtr<UStaticMesh>> VariationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TObjectPtr<UStaticMesh> LOD1Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TObjectPtr<UStaticMesh> LOD2Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bHasCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bCanBeDestroyed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
    FVector MinScale = FVector(0.8f, 0.8f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
    FVector MaxScale = FVector(1.2f, 1.2f, 1.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    bool bAffectedByWind = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindStrength = 1.0f;
};

/**
 * Vegetation System Component
 * Manages procedural vegetation generation and placement
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVegetationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVegetationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Core vegetation functions
    UFUNCTION(BlueprintCallable, Category = "Vegetation Generation")
    void GenerateVegetationForBiome(EPrehistoricBiome BiomeType, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Vegetation Generation")
    void GenerateVegetationInArea(const FBox& Area, const FBiomeVegetationConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Vegetation Generation")
    void ClearVegetationInArea(const FBox& Area);

    UFUNCTION(BlueprintCallable, Category = "Vegetation Generation")
    void RefreshVegetationLOD(const FVector& ViewerLocation);

    // Plant placement functions
    UFUNCTION(BlueprintCallable, Category = "Plant Placement")
    void PlacePlantAtLocation(EPrehistoricPlantType PlantType, const FVector& Location, 
                             const FRotator& Rotation = FRotator::ZeroRotator, 
                             const FVector& Scale = FVector::OneVector);

    UFUNCTION(BlueprintCallable, Category = "Plant Placement")
    void PlacePlantCluster(EPrehistoricPlantType PlantType, const FVector& Center, 
                          float Radius, int32 Count, float RandomSpread = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Plant Placement")
    bool RemovePlantAtLocation(const FVector& Location, float SearchRadius = 100.0f);

    // Biome-specific generation
    UFUNCTION(BlueprintCallable, Category = "Biome Vegetation")
    void GenerateTropicalRainforestVegetation(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Vegetation")
    void GenerateConiferousForestVegetation(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Vegetation")
    void GenerateFernPrairieVegetation(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Vegetation")
    void GenerateSwampVegetation(const FVector& Center, float Radius);

    // Environmental queries
    UFUNCTION(BlueprintCallable, Category = "Environment Queries")
    TArray<FPlantInstanceData> GetPlantsInArea(const FBox& Area) const;

    UFUNCTION(BlueprintCallable, Category = "Environment Queries")
    EPrehistoricPlantType GetDominantPlantType(const FVector& Location, float SearchRadius) const;

    UFUNCTION(BlueprintCallable, Category = "Environment Queries")
    float GetVegetationDensityAtLocation(const FVector& Location, float SearchRadius) const;

    // Performance and optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetVegetationLODDistance(float NewLODDistance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableDynamicBatching(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCullingDistance(float NewCullingDistance);

protected:
    // Plant asset registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant Assets")
    TMap<EPrehistoricPlantType, FPlantAssetData> PlantAssets;

    // Biome configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configs")
    TMap<EPrehistoricBiome, FBiomeVegetationConfig> BiomeConfigurations;

    // Instanced mesh components for performance
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    TMap<EPrehistoricPlantType, TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> PlantInstanceComponents;

    // Generation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 VegetationSeed = 54321;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float PlacementGridSize = 200.0f; // 2m grid for placement

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bUseNaturalClustering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float ClusteringStrength = 0.7f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance = 5000.0f; // 50m LOD transition

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 15000.0f; // 150m culling distance

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseDynamicBatching = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxInstancesPerComponent = 10000;

    // Environmental factors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bConsiderSlope = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MaxPlacementSlope = 45.0f; // degrees

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bAvoidWater = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WaterAvoidanceDistance = 50.0f;

private:
    // Internal generation functions
    void InitializePlantAssets();
    void SetupBiomeConfigurations();
    void CreateInstancedMeshComponents();
    
    // Placement algorithms
    TArray<FVector> GeneratePoissonDiskSampling(const FBox& Area, float MinDistance, int32 MaxAttempts = 30);
    TArray<FVector> GenerateClusteredPlacements(const FVector& Center, float Radius, int32 Count, float ClusterSize);
    bool IsValidPlacementLocation(const FVector& Location, EPrehistoricPlantType PlantType) const;
    
    // Environmental analysis
    float CalculateSuitability(const FVector& Location, EPrehistoricPlantType PlantType, EPrehistoricBiome BiomeType) const;
    float GetSlopeAtLocation(const FVector& Location) const;
    float GetMoistureAtLocation(const FVector& Location) const;
    float GetSunlightAtLocation(const FVector& Location) const;
    
    // Instance management
    void AddPlantInstance(EPrehistoricPlantType PlantType, const FPlantInstanceData& InstanceData);
    void RemovePlantInstance(EPrehistoricPlantType PlantType, int32 InstanceIndex);
    void UpdateInstanceLOD(EPrehistoricPlantType PlantType, const FVector& ViewerLocation);

    // Performance monitoring
    UPROPERTY()
    int32 TotalPlantInstances;

    UPROPERTY()
    float LastGenerationTime;

public:
    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVegetationGenerated, EPrehistoricBiome, BiomeType, FVector, Location, int32, PlantCount);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnVegetationGenerated OnVegetationGenerated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlantPlaced, EPrehistoricPlantType, PlantType, FVector, Location);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlantPlaced OnPlantPlaced;
};