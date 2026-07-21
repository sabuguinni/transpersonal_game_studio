// PCGWorldGenerator.h — Procedural World Generator #05
// Cycle: PROD_CYCLE_AUTO_20260626_008
// Biome system, terrain variation, river generation for Cretaceous world

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PCGWorldGenerator.generated.h"

// === BIOME TYPES ===
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    TropicalForest    UMETA(DisplayName = "Tropical Forest"),
    OpenPlains        UMETA(DisplayName = "Open Plains"),
    RockyHighlands    UMETA(DisplayName = "Rocky Highlands"),
    RiverValley       UMETA(DisplayName = "River Valley"),
    CoastalSwamp      UMETA(DisplayName = "Coastal Swamp"),
    VolcanicField     UMETA(DisplayName = "Volcanic Field")
};

// === BIOME ZONE DATA ===
USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float Radius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float GroundHeightOffset = 0.f;
};

// === TERRAIN SAMPLE ===
USTRUCT(BlueprintType)
struct FWorld_TerrainSample
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "World|Terrain")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "World|Terrain")
    float Height = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "World|Terrain")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenPlains;

    UPROPERTY(BlueprintReadOnly, Category = "World|Terrain")
    float SlopeAngle = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "World|Terrain")
    bool bIsWater = false;
};

// === PCG WORLD GENERATOR ===
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|World")
class TRANSPERSONALGAME_API UPCGWorldGenerator : public UObject
{
    GENERATED_BODY()

public:
    UPCGWorldGenerator();

    // === INITIALIZATION ===
    UFUNCTION(BlueprintCallable, Category = "World|Generation")
    void InitializeWorld(int32 Seed);

    // === BIOME QUERIES ===
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    TArray<FWorld_BiomeZone> GetAllBiomeZones() const;

    // === TERRAIN QUERIES ===
    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    float GetTerrainHeightAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    FWorld_TerrainSample SampleTerrain(FVector Location) const;

    // === RIVER SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "World|Rivers")
    int32 GetRiverCount() const;

    // === CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Config")
    int32 BiomeSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Config")
    float TerrainScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Config")
    int32 RiverCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Config")
    bool bEnableBiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Config")
    float WorldExtentXY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Config")
    float HeightVariationScale;

private:
    void GenerateBiomeMap();
    void GenerateRivers();

    TArray<FWorld_BiomeZone> BiomeZones;
    TArray<TArray<FVector>> RiverPaths;
    FRandomStream RandomStream;
};
