// Copyright Transpersonal Game Studio. All Rights Reserved.
// ProceduralWorldSubsystem.h - Core world generation subsystem for prehistoric environments

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataAsset.h"
#include "Components/ActorComponent.h"
// DISABLED: // DISABLED: #include "TranspersonalGameSharedTypes.h"
#include "ProceduralWorldSubsystem.generated.h"

class ULandscape;
class APCGVolume;
class UPCGGraph;
class UWorldPartitionSubsystem;
class APhysicsPerformanceProfiler;

UENUM(BlueprintType)
enum class EWorld_PrehistoricBiome_B7A : uint8
{
    DenseJungle         UMETA(DisplayName = "Dense Jungle"),
    OpenPlains          UMETA(DisplayName = "Open Plains"),
    RiverDelta          UMETA(DisplayName = "River Delta"),
    MountainousRegion   UMETA(DisplayName = "Mountainous Region"),
    CoastalArea         UMETA(DisplayName = "Coastal Area"),
    Swampland          UMETA(DisplayName = "Swampland"),
    VolcanicRegion     UMETA(DisplayName = "Volcanic Region"),
    CaveSystem         UMETA(DisplayName = "Cave System")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeGenerationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_PrehistoricBiome_B7A BiomeType = EWorld_PrehistoricBiome_B7A::DenseJungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.7f; // 0-1 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f; // Meters above sea level

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.8f; // 0-1 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> BiomeMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TSoftObjectPtr<UMaterialInterface> TerrainMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TSoftObjectPtr<UPCGGraph> BiomePCGGraph;

    FBiomeGenerationData()
    {
        Temperature = 25.0f;
        Humidity = 0.7f;
        Elevation = 0.0f;
        VegetationDensity = 0.8f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTerrainGenerationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 WorldSizeKm = 4; // 4km x 4km world

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 HeightmapResolution = 2017; // UE5 recommended size

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MaxElevation = 500.0f; // Maximum height in meters

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float SeaLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 NoiseOctaves = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseFrequency = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoisePersistence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 RandomSeed = 12345;

    FTerrainGenerationSettings()
    {
        WorldSizeKm = 4;
        HeightmapResolution = 2017;
        MaxElevation = 500.0f;
        SeaLevel = 0.0f;
        NoiseOctaves = 6;
        NoiseFrequency = 0.01f;
        NoisePersistence = 0.5f;
        RandomSeed = 12345;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UProceduralWorldDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    FTerrainGenerationSettings TerrainSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TArray<FBiomeGenerationData> AvailableBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TSoftObjectPtr<UPCGGraph> MasterWorldPCGGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    bool bUseWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    bool bEnableRuntimeGeneration = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxConcurrentPCGTasks = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PCGGenerationBudgetMs = 16.0f; // 60fps budget
};

/**
 * UProceduralWorldSubsystem
 * 
 * Core subsystem responsible for generating the prehistoric world using PCG Framework.
 * Handles terrain generation, biome placement, and performance optimization.
 * 
 * Key Features:
 * - Procedural terrain generation using Perlin noise
 * - Biome-based vegetation and asset placement
 * - PCG integration with World Partition
 * - Performance monitoring and optimization
 * - Runtime generation support for large worlds
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UProceduralWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UProceduralWorldSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // World Generation Interface
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld(UProceduralWorldDataAsset* WorldData);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrain(const FTerrainGenerationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomes(const TArray<FBiomeGenerationData>& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PlacePCGVolumes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EWorld_PrehistoricBiome_B7A GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Performance Interface
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceBudget(float BudgetMs);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentGenerationTime() const { return CurrentGenerationTimeMs; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsGenerationInProgress() const { return bIsGenerating; }

    // Debug Interface
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawBiomes(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void RegenerateArea(const FVector& Center, float Radius);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldGenerationComplete, bool, bSuccess);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBiomeGenerated, EWorld_PrehistoricBiome_B7A, BiomeType, float, CompletionPercent);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnWorldGenerationComplete OnWorldGenerationComplete;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBiomeGenerated OnBiomeGenerated;

protected:
    // Internal generation functions
    void GenerateHeightmap(const FTerrainGenerationSettings& Settings);
    void ApplyBiomeDistribution();
    void CreatePCGComponents();
    void OptimizePerformance();

    // Noise generation
    float GeneratePerlinNoise(float X, float Y, int32 Octaves, float Frequency, float Persistence, int32 Seed) const;
    float SampleNoise(float X, float Y) const;

    // Biome logic
    EWorld_PrehistoricBiome_B7A DetermineBiomeType(float Height, float Temperature, float Humidity) const;
    FBiomeGenerationData GetBiomeData(EWorld_PrehistoricBiome_B7A BiomeType) const;

    // Performance monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceBudget();

private:
// [UHT-FIX]     UPROPERTY()
    TObjectPtr<ULandscape> GeneratedLandscape;

    UPROPERTY()
    TArray<TObjectPtr<APCGVolume>> PCGVolumes;

    UPROPERTY()
    TObjectPtr<UProceduralWorldDataAsset> CurrentWorldData;

// [UHT-FIX]     UPROPERTY()
    TObjectPtr<APhysicsPerformanceProfiler> PerformanceProfiler;

    // Generation state
    bool bIsGenerating = false;
    float CurrentGenerationTimeMs = 0.0f;
    float PerformanceBudgetMs = 16.0f; // 60fps target
    int32 GeneratedBiomeCount = 0;
    int32 TotalBiomeCount = 0;

    // Cached biome map for fast lookups
    TArray<TArray<EWorld_PrehistoricBiome_B7A>> BiomeMap;
    int32 BiomeMapResolution = 512;

    // Performance tracking
    double GenerationStartTime = 0.0;
    TArray<float> FrameTimeHistory;
    static constexpr int32 MaxFrameTimeHistory = 60;

    // PCG task management
    TArray<FGuid> ActivePCGTasks;
    int32 MaxConcurrentTasks = 4;

    // Debug visualization
    bool bDebugDrawBiomes = false;
    TArray<FVector> DebugBiomePoints;
    TArray<FColor> DebugBiomeColors;
};