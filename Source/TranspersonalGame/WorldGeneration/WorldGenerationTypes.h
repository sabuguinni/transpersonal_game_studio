#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "WorldGenerationTypes.generated.h"

/**
 * Biome types for procedural world generation
 */
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"), 
    Mountain    UMETA(DisplayName = "Mountain"),
    River       UMETA(DisplayName = "River"),
    Grassland   UMETA(DisplayName = "Grassland"),
    Wetland     UMETA(DisplayName = "Wetland"),
    Canyon      UMETA(DisplayName = "Canyon"),
    Plateau     UMETA(DisplayName = "Plateau")
};

/**
 * Terrain generation parameters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 Octaves = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Persistence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Lacunarity = 2.0f;

    FWorld_TerrainParams()
    {
        HeightScale = 1000.0f;
        NoiseScale = 0.01f;
        Octaves = 4;
        Persistence = 0.5f;
        Lacunarity = 2.0f;
    }
};

/**
 * Biome configuration data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<class UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    FWorld_BiomeConfig()
    {
        BiomeType = EWorld_BiomeType::Forest;
        Temperature = 20.0f;
        Humidity = 0.5f;
        Elevation = 0.0f;
        BiomeColor = FLinearColor::Green;
        VegetationDensity = 1.0f;
    }
};

/**
 * River system configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Width = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Depth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    int32 Segments = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Meandering = 0.3f;

    FWorld_RiverConfig()
    {
        Width = 500.0f;
        Depth = 100.0f;
        FlowSpeed = 100.0f;
        Segments = 50;
        Meandering = 0.3f;
    }
};

/**
 * World generation settings
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_GenerationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 WorldSize = 20480; // 20km x 20km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 ChunkSize = 2048; // 2km x 2km chunks

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    FWorld_TerrainParams TerrainParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    FWorld_RiverConfig RiverConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 RandomSeed = 12345;

    FWorld_GenerationSettings()
    {
        WorldSize = 20480;
        ChunkSize = 2048;
        RandomSeed = 12345;
    }
};