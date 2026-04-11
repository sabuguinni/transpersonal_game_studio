#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldGenerationManager.generated.h"

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Coastal     UMETA(DisplayName = "Coastal"),
    Grassland   UMETA(DisplayName = "Grassland"),
    River       UMETA(DisplayName = "River"),
    Mountain    UMETA(DisplayName = "Mountain"),
    Desert      UMETA(DisplayName = "Desert")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VegetationDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UPCGGraphAsset> PCGGraph = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TObjectPtr<class UStaticMesh>> BiomeMeshes;

    FBiomeData()
    {
        BiomeType = EBiomeType::Forest;
        Temperature = 20.0f;
        Humidity = 50.0f;
        Elevation = 0.0f;
        VegetationDensity = 0.7f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorldGenerationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WorldSizeKm = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BiomeResolution = 512;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NoiseScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RandomSeed = 12345;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StreamingCellSize = 25600.0f; // 256m cells

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FBiomeData> BiomeDefinitions;

    FWorldGenerationSettings()
    {
        WorldSizeKm = 100;
        BiomeResolution = 512;
        NoiseScale = 0.01f;
        RandomSeed = 12345;
        bUseWorldPartition = true;
        StreamingCellSize = 25600.0f;
    }
};

/**
 * World Generation Manager - Controls procedural generation of the entire game world
 * Manages biome distribution, terrain generation, and integration with World Partition
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorldGenerationManager : public UObject
{
    GENERATED_BODY()

public:
    UWorldGenerationManager();

    // Core Generation Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeWorldGeneration(UWorld* TargetWorld);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomeMap();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRiverSystems();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateSettlements();

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FBiomeData GetBiomeData(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void SetBiomeAtLocation(const FVector& WorldLocation, EBiomeType BiomeType);

    // PCG Integration
    UFUNCTION(BlueprintCallable, Category = "PCG")
    void CreatePCGVolumeForBiome(const FVector& Location, const FVector& Extent, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "PCG")
    void RefreshPCGGeneration();

    // World Partition Integration
    UFUNCTION(BlueprintCallable, Category = "World Partition")
    void SetupWorldPartition();

    UFUNCTION(BlueprintCallable, Category = "World Partition")
    void CreateStreamingCells();

    // Noise and Terrain
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetHeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FWorldGenerationSettings GenerationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TObjectPtr<class UPCGGraphAsset> MasterDistributionGraph;

protected:
    // Internal state
    UPROPERTY()
    TObjectPtr<UWorld> TargetWorld;

    UPROPERTY()
    TArray<TArray<EBiomeType>> BiomeMap;

    UPROPERTY()
    TArray<TObjectPtr<class APCGVolume>> PCGVolumes;

    UPROPERTY()
    TObjectPtr<class UWorldPartition> WorldPartition;

    // Internal generation functions
    void InitializeBiomeDefinitions();
    float GenerateNoise(float X, float Y, float Scale = 1.0f, int32 Octaves = 4) const;
    EBiomeType DetermineBiomeType(float Temperature, float Humidity, float Elevation) const;
    void CreateRiverPath(const FVector& StartLocation, const FVector& EndLocation);
    FVector FindNearestWaterSource(const FVector& Location) const;

private:
    // Noise generation state
    mutable class FRandomStream RandomStream;
    
    // Performance tracking
    double LastGenerationTime = 0.0;
    int32 GeneratedCellCount = 0;
};

/**
 * World Generation Data Asset - Stores presets and configurations
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UWorldGenerationDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    FWorldGenerationSettings DefaultSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FBiomeData> BiomePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TObjectPtr<class UPCGGraphAsset> DefaultTerrainGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TMap<EBiomeType, TObjectPtr<class UPCGGraphAsset>> BiomeGraphs;
};