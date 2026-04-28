#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "PCGSettings.h"
#include "PCGBiomeSystemV43.generated.h"

UENUM(BlueprintType)
enum class EPCGBiomeGenerationType : uint8
{
    Surface         UMETA(DisplayName = "Surface Generation"),
    Volume          UMETA(DisplayName = "Volume Generation"),
    Spline          UMETA(DisplayName = "Spline Generation"),
    Landscape       UMETA(DisplayName = "Landscape Generation"),
    Hierarchical    UMETA(DisplayName = "Hierarchical Generation")
};

UENUM(BlueprintType)
enum class EPCGVegetationDensity : uint8
{
    Sparse      UMETA(DisplayName = "Sparse (10-30%)"),
    Medium      UMETA(DisplayName = "Medium (30-60%)"),
    Dense       UMETA(DisplayName = "Dense (60-85%)"),
    VeryDense   UMETA(DisplayName = "Very Dense (85-100%)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPCGBiomeVegetationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> TreeMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> ShrubMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> GrassMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> FernMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EPCGVegetationDensity VegetationDensity = EPCGVegetationDensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float TreeSpawnRate = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float ShrubSpawnRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float GrassSpawnRate = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float FernSpawnRate = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector2D TreeScaleRange = FVector2D(0.8f, 1.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector2D ShrubScaleRange = FVector2D(0.6f, 1.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector2D GrassScaleRange = FVector2D(0.5f, 1.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPCGBiomeRockData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    TArray<TSoftObjectPtr<UStaticMesh>> LargeRockMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    TArray<TSoftObjectPtr<UStaticMesh>> MediumRockMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    TArray<TSoftObjectPtr<UStaticMesh>> SmallRockMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    TArray<TSoftObjectPtr<UStaticMesh>> BoulderMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    float RockDensity = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    float LargeRockSpawnRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    float MediumRockSpawnRate = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    float SmallRockSpawnRate = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    FVector2D RockScaleRange = FVector2D(0.7f, 1.8f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPCGBiomeGenerationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    EPCGBiomeGenerationType GenerationType = EPCGBiomeGenerationType::Surface;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 PointsPerSquareMeter = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinPointSpacing = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MaxPointSpacing = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bUseHierarchicalGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 HierarchicalGridSize = 512;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bEnableRuntimeGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float GenerationRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float CleanupRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filtering")
    float MinSlopeAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filtering")
    float MaxSlopeAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filtering")
    float MinHeight = -1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filtering")
    float MaxHeight = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filtering")
    bool bAvoidWaterBodies = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filtering")
    float WaterAvoidanceDistance = 200.0f;
};

/**
 * PCG Biome Data Asset V43
 * 
 * Defines complete biome configuration for PCG generation including
 * vegetation, rocks, generation settings, and filtering parameters.
 * Designed for Jurassic environments with prehistoric flora and geology.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPCGBiomeDataAssetV43 : public UDataAsset
{
    GENERATED_BODY()

public:
    UPCGBiomeDataAssetV43();

    // Biome Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Identity")
    FString BiomeName = TEXT("Jurassic Biome");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Identity")
    FText BiomeDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Identity")
    TSoftObjectPtr<UTexture2D> BiomeIcon;

    // PCG Graph References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    TSoftObjectPtr<UPCGGraph> MainGenerationGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    TSoftObjectPtr<UPCGGraph> VegetationGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    TSoftObjectPtr<UPCGGraph> RockPlacementGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    TSoftObjectPtr<UPCGGraph> DetailGraph;

    // Biome Content
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Content")
    FPCGBiomeVegetationData VegetationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Content")
    FPCGBiomeRockData RockData;

    // Generation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    FPCGBiomeGenerationSettings GenerationSettings;

    // Environmental Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Humidity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Temperature = 25.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WindStrength = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FLinearColor AmbientColor = FLinearColor(0.8f, 0.9f, 0.7f, 1.0f);

    // Material Overrides
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> TerrainMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> VegetationMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> RockMaterial;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<class USoundBase> BiomeAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientSoundVolume = 0.5f;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxInstancesPerChunk = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance3 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseCullingVolumes = true;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Biome Utilities")
    TArray<UStaticMesh*> GetAllVegetationMeshes() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Utilities")
    TArray<UStaticMesh*> GetAllRockMeshes() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Utilities")
    float GetVegetationDensityMultiplier() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Utilities")
    bool ShouldSpawnAtLocation(FVector WorldLocation, float SlopeAngle) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Utilities")
    FVector GetRandomVegetationScale(int32 MeshType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Utilities")
    UStaticMesh* SelectRandomMesh(int32 MeshCategory, float RandomValue) const;

#if WITH_EDITOR
    // Editor Utilities
    UFUNCTION(CallInEditor, Category = "Editor")
    void ValidateBiomeData();

    UFUNCTION(CallInEditor, Category = "Editor")
    void GeneratePreviewThumbnail();
#endif
};

/**
 * PCG Biome Manager Component V43
 * 
 * Manages multiple biomes and their interactions within a world region.
 * Handles biome transitions, LOD management, and runtime optimization.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(PCG), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPCGBiomeManagerV43 : public UActorComponent
{
    GENERATED_BODY()

public:
    UPCGBiomeManagerV43();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void AddBiome(UPCGBiomeDataAssetV43* BiomeAsset, FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void RemoveBiome(int32 BiomeIndex);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void ClearAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    UPCGBiomeDataAssetV43* GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    TArray<UPCGBiomeDataAssetV43*> GetBiomesInRadius(FVector WorldLocation, float Radius) const;

    // Generation Control
    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateBiome(int32 BiomeIndex);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void RegenerateBiomesInRadius(FVector WorldLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void CleanupBiomesOutsideRadius(FVector WorldLocation, float Radius);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODLevels(FVector ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomeGeneration();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetTotalGeneratedInstances() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetGenerationPerformanceMetric() const;

    // Biome Transitions
    UFUNCTION(BlueprintCallable, Category = "Transitions")
    void CreateBiomeTransitions();

    UFUNCTION(BlueprintCallable, Category = "Transitions")
    float GetBiomeBlendWeight(FVector WorldLocation, int32 BiomeIndex) const;

protected:
    // Biome Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<UPCGBiomeDataAssetV43*> BiomeAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FVector> BiomeLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<float> BiomeRadii;

    // PCG Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TArray<UPCGComponent*> BiomePCGComponents;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveBiomes = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceMonitoring = true;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 ActiveBiomeCount;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector LastViewerLocation;

private:
    // Internal Management
    void UpdateActiveBiomes(FVector ViewerLocation);
    void CreateBiomePCGComponent(int32 BiomeIndex);
    void DestroyBiomePCGComponent(int32 BiomeIndex);
    float CalculateBiomeDistance(FVector Location, int32 BiomeIndex) const;
    void UpdatePerformanceMetrics();

    FTimerHandle UpdateTimerHandle;
    FTimerHandle PerformanceTimerHandle;
};