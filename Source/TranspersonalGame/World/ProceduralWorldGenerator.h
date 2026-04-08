#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Landscape.h"
#include "ProceduralWorldGenerator.generated.h"

/**
 * Procedural World Generator for Jurassic Survival Game
 * Generates terrain, biomes, rivers, and geographical structure
 * Based on real geological principles and ecosystem logic
 */

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverValley     UMETA(DisplayName = "River Valley"),
    RockyOutcrop    UMETA(DisplayName = "Rocky Outcrop"),
    Wetlands        UMETA(DisplayName = "Wetlands"),
    ClearingMeadow  UMETA(DisplayName = "Clearing Meadow"),
    CaveSystem      UMETA(DisplayName = "Cave System")
};

UENUM(BlueprintType)
enum class ETerrainFeature : uint8
{
    River           UMETA(DisplayName = "River"),
    Hill            UMETA(DisplayName = "Hill"),
    Valley          UMETA(DisplayName = "Valley"),
    Plateau         UMETA(DisplayName = "Plateau"),
    Ravine          UMETA(DisplayName = "Ravine"),
    NaturalBridge   UMETA(DisplayName = "Natural Bridge")
};

USTRUCT(BlueprintType)
struct FBiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Elevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DrainageLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<ETerrainFeature> SupportedFeatures;

    FBiomeDefinition()
    {
        BiomeType = EBiomeType::DenseForest;
        Humidity = 0.7f;
        Temperature = 0.6f;
        Elevation = 0.5f;
        DrainageLevel = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FRiverSystemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SourceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MouthLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlowRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> TributaryPoints;

    FRiverSystemData()
    {
        SourceLocation = FVector::ZeroVector;
        MouthLocation = FVector::ZeroVector;
        Width = 500.0f;
        Depth = 200.0f;
        FlowRate = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UProceduralWorldGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UProceduralWorldGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // World Generation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    FVector2D WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 LandscapeResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float MaxElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float SeaLevel;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FBiomeDefinition> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeTransitionDistance;

    // River System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    TArray<FRiverSystemData> RiverSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    int32 MaxRiverBranches;

    // PCG References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<UPCGGraph> TerrainGenerationGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<UPCGGraph> BiomeDistributionGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<UPCGGraph> RiverGenerationGraph;

    // Generation Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrain();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRiverSystems();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateGeologicalFeatures();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EBiomeType GetBiomeAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetElevationAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool IsLocationNearRiver(FVector Location, float Threshold = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FVector FindNearestRiverPoint(FVector Location);

private:
    // Internal generation methods
    void InitializeHeightmapData();
    void ApplyGeologicalProcesses();
    void CalculateWaterFlow();
    void GenerateErosionPatterns();
    void PlaceLandmarks();
    
    // PCG Integration
    void SetupPCGComponents();
    void ConfigurePCGGraphs();
    
    // Landscape reference
    UPROPERTY()
    ALandscape* GeneratedLandscape;

    // Heightmap data
    TArray<float> HeightmapData;
    TArray<float> HumidityMap;
    TArray<float> TemperatureMap;
    TArray<float> DrainageMap;
};