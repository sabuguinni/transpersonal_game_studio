#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Math/UnrealMathUtility.h"
#include "SharedTypes.h"
#include "World_TerrainGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_ErosionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float WaterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float SedimentCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float ErosionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float DepositionRate;

    FWorld_ErosionData()
    {
        WaterLevel = 0.0f;
        SedimentCapacity = 1.0f;
        ErosionRate = 0.1f;
        DepositionRate = 0.05f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeRegion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation;

    FWorld_BiomeRegion()
    {
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        BiomeType = EBiomeType::Grassland;
        Temperature = 20.0f;
        Humidity = 0.5f;
        Elevation = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector StartPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector EndPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowRate;

    FWorld_RiverSegment()
    {
        StartPoint = FVector::ZeroVector;
        EndPoint = FVector::ZeroVector;
        Width = 100.0f;
        Depth = 50.0f;
        FlowRate = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_TerrainGenerator : public AActor
{
    GENERATED_BODY()

public:
    AWorld_TerrainGenerator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TerrainMeshComponent;

    // Terrain Generation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float TerrainSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float HeightScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    int32 TerrainResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    int32 NoiseOctaves;

    // Erosion Simulation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    int32 ErosionIterations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float RainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float EvaporationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float SedimentCapacityConstant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float MinSedimentCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float DepositSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float ErodeSpeed;

    // River Generation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    float RiverDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    int32 MaxRiverLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    float RiverWidthMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    float RiverDepthMultiplier;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeRegion> BiomeRegions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeTransitionSmoothness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float TemperatureGradient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float HumidityVariation;

    // Generated Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generated Data")
    TArray<FWorld_RiverSegment> GeneratedRivers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generated Data")
    TArray<FWorld_ErosionData> ErosionMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generated Data")
    bool bTerrainGenerated;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Generation Functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ApplyErosionSimulation();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateRiverNetwork();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ApplyBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void FinalizeTerrainGeneration();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    float GetHeightAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    EBiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    float GetHumidityAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    bool IsLocationNearRiver(FVector WorldLocation, float Threshold = 200.0f) const;

    // Erosion Simulation Functions
    UFUNCTION(BlueprintCallable, Category = "Erosion")
    void SimulateWaterDroplet(FVector StartLocation, int32 MaxSteps);

    UFUNCTION(BlueprintCallable, Category = "Erosion")
    void CalculateFlowMap();

    UFUNCTION(BlueprintCallable, Category = "Erosion")
    void ApplySedimentTransport();

    // River Generation Functions
    UFUNCTION(BlueprintCallable, Category = "Rivers")
    TArray<FVector> TraceRiverPath(FVector StartLocation, FVector TargetDirection);

    UFUNCTION(BlueprintCallable, Category = "Rivers")
    void CreateRiverMesh(const TArray<FVector>& RiverPath, float Width, float Depth);

    UFUNCTION(BlueprintCallable, Category = "Rivers")
    void ConnectRiverSystems();

    // Biome Functions
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void CreateBiomeTransitions();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void ApplyBiomeSpecificFeatures();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void GenerateGeologicalFeatures();

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void RegenerateTerrainInEditor();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ClearGeneratedTerrain();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ExportTerrainData();

protected:
    // Internal Helper Functions
    float GeneratePerlinNoise(float X, float Y, float Scale, int32 Octaves) const;
    FVector CalculateGradient(FVector Location) const;
    float InterpolateBiomeValue(FVector Location, float BiomeValue1, float BiomeValue2, float Distance) const;
    void UpdateErosionData(int32 Index, const FWorld_ErosionData& NewData);
    bool ValidateTerrainParameters() const;
    void InitializeDefaultBiomes();
    void SetupTerrainMesh();
};