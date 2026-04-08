#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Landscape/Classes/Landscape.h"
#include "Components/SceneComponent.h"
#include "JurassicWorldGenerator.generated.h"

UENUM(BlueprintType)
enum class EJurassicBiomeType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverValley     UMETA(DisplayName = "River Valley"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RockyOutcrops   UMETA(DisplayName = "Rocky Outcrops"),
    SwampLands      UMETA(DisplayName = "Swamp Lands"),
    Coastline       UMETA(DisplayName = "Coastline")
};

USTRUCT(BlueprintType)
struct FJurassicBiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EJurassicBiomeType BiomeType = EJurassicBiomeType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DensityWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float ElevationMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float ElevationMax = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float SlopeMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float SlopeMax = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float WaterDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bNearWater = false;
};

USTRUCT(BlueprintType)
struct FJurassicTerrainSettings
{
    GENERATED_BODY()

    // Landscape dimensions (must follow UE5 valid heightmap sizes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 LandscapeSize = 2017; // 2017x2017 vertices (recommended size)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float WorldScale = 100.0f; // cm per unit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale = 256.0f; // Maximum height variation

    // Noise settings for base terrain
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    float BaseFrequency = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    float BaseAmplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    int32 NoiseOctaves = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    float NoisePersistence = 0.5f;

    // River generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rivers")
    int32 MainRiverCount = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rivers")
    int32 TributaryCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rivers")
    float RiverWidth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rivers")
    float RiverDepth = 200.0f;
};

/**
 * Main World Generator for the Jurassic survival game
 * Generates terrain, biomes, rivers, and basic geographic structure
 * Uses PCG Framework with World Partition for optimal performance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AJurassicWorldGenerator : public AActor
{
    GENERATED_BODY()

public:
    AJurassicWorldGenerator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPCGComponent* PCGComponent;

    // World generation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    FJurassicTerrainSettings TerrainSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TArray<FJurassicBiomeSettings> BiomeSettings;

    // PCG Graph for world generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    class UPCGGraph* WorldGenerationGraph;

    // Generated landscape reference
    UPROPERTY(BlueprintReadOnly, Category = "Generated")
    class ALandscape* GeneratedLandscape;

public:
    // Generation functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearGeneratedWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    ALandscape* CreateBaseLandscape();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRiverSystem();

    // Utility functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    EJurassicBiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    float GetElevationAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    float GetSlopeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    bool IsNearWater(FVector WorldLocation, float MaxDistance = 1000.0f) const;

private:
    // Internal generation helpers
    void SetupPCGComponent();
    void ConfigureWorldPartition();
    TArray<FVector> GenerateRiverSplinePoints(FVector StartPoint, FVector EndPoint, int32 SegmentCount);
    float CalculatePerlinNoise(float X, float Y, int32 Octaves, float Persistence, float Frequency) const;
};