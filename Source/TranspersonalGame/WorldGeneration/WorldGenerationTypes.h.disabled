#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WorldGenerationTypes.generated.h"

/**
 * Biome types for world generation
 */
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Grassland   UMETA(DisplayName = "Grassland"), 
    Mountain    UMETA(DisplayName = "Mountain"),
    River       UMETA(DisplayName = "River"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Desert      UMETA(DisplayName = "Desert"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Coastal     UMETA(DisplayName = "Coastal")
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
struct TRANSPERSONALGAME_API FWorld_BiomeConfig : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureMax = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityMax = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ElevationMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ElevationMax = 1.0f;

    FWorld_BiomeConfig()
    {
        BiomeType = EWorld_BiomeType::Forest;
        BiomeColor = FLinearColor::Green;
        TemperatureMin = 0.0f;
        TemperatureMax = 1.0f;
        HumidityMin = 0.0f;
        HumidityMax = 1.0f;
        ElevationMin = 0.0f;
        ElevationMax = 1.0f;
    }
};

/**
 * River generation parameters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float MinLength = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float MaxLength = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Width = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Depth = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    int32 SegmentCount = 20;

    FWorld_RiverParams()
    {
        MinLength = 5000.0f;
        MaxLength = 20000.0f;
        Width = 500.0f;
        Depth = 200.0f;
        SegmentCount = 20;
    }
};

/**
 * Settlement generation data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_SettlementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    EWorld_BiomeType PreferredBiome = EWorld_BiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    int32 Population = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    FString SettlementName = TEXT("Unnamed");

    FWorld_SettlementData()
    {
        Location = FVector::ZeroVector;
        Radius = 2000.0f;
        PreferredBiome = EWorld_BiomeType::Grassland;
        Population = 100;
        SettlementName = TEXT("Unnamed");
    }
};