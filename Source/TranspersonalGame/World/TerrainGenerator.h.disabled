// Copyright Transpersonal Game Studio. All Rights Reserved.
// TerrainGenerator.h - Advanced Terrain Generation System

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Texture2D.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "Materials/MaterialInterface.h"
#include "TerrainGenerator.generated.h"

class ALandscape;
class ULandscapeLayerInfoObject;
class UMaterialInterface;

/**
 * Terrain Layer Types for Prehistoric World
 */
UENUM(BlueprintType)
enum class ETerrainLayer : uint8
{
    Bedrock             UMETA(DisplayName = "Bedrock"),
    RichSoil            UMETA(DisplayName = "Rich Soil"),
    Sand                UMETA(DisplayName = "Sand"),
    Clay                UMETA(DisplayName = "Clay"),
    Gravel              UMETA(DisplayName = "Gravel"),
    Mud                 UMETA(DisplayName = "Mud"),
    VolcanicAsh         UMETA(DisplayName = "Volcanic Ash"),
    Limestone           UMETA(DisplayName = "Limestone")
};

/**
 * Geological Formation Types
 */
UENUM(BlueprintType)
enum class EWorld_GeologicalFormation : uint8
{
    Plains              UMETA(DisplayName = "Plains"),
    Hills               UMETA(DisplayName = "Hills"),
    Mountains           UMETA(DisplayName = "Mountains"),
    Valleys             UMETA(DisplayName = "Valleys"),
    Canyons             UMETA(DisplayName = "Canyons"),
    Plateaus            UMETA(DisplayName = "Plateaus"),
    RiverBeds           UMETA(DisplayName = "River Beds"),
    LakeBeds            UMETA(DisplayName = "Lake Beds"),
    VolcanicCones       UMETA(DisplayName = "Volcanic Cones"),
    Cliffs              UMETA(DisplayName = "Cliffs")
};

/**
 * Terrain Generation Settings
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainGenerationSettings
{
    GENERATED_BODY()

    // Heightmap settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
    int32 HeightmapWidth = 2017;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
    int32 HeightmapHeight = 2017;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
    float MaxHeight = 25600.0f; // 256m max height

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
    float MinHeight = -2560.0f; // -25.6m min height (underwater)

    // Noise parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float BaseNoiseScale = 0.005f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float DetailNoiseScale = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    int32 NoiseOctaves = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float NoisePersistence = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float NoiseLacunarity = 2.0f;

    // Geological features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    float MountainFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    float ValleyDepth = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    float RiverFrequency = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    float CliffSteepness = 0.8f;

    // Erosion simulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    bool bEnableErosion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    int32 ErosionIterations = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float RainAmount = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float EvaporationRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float SedimentCapacity = 0.3f;

    FWorld_TerrainGenerationSettings()
    {
        // Default constructor with reasonable values
    }
};

/**
 * Terrain Layer Information
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTerrainLayerInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    ETerrainLayer LayerType = ETerrainLayer::RichSoil;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    TObjectPtr<ULandscapeLayerInfoObject> LayerInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    float MinHeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    float MaxHeight = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    float MinSlope = 0.0f; // In degrees

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    float MaxSlope = 90.0f; // In degrees

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    float BlendRange = 100.0f;

    FTerrainLayerInfo()
    {
        // Default constructor
    }
};

/**
 * Advanced Terrain Generator Component
 * Handles complex terrain generation with geological realism
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(WorldGeneration), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTerrainGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UTerrainGenerator();

protected:
    virtual void BeginPlay() override;

public:
    // Main generation functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateHeightmap();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ApplyErosion();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainLayers();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void CreateLandscape();

    // Geological formation generators
    UFUNCTION(BlueprintCallable, Category = "Geological Features")
    void GenerateMountainRange(const FVector& StartPoint, const FVector& EndPoint, float Height, float Width);

    UFUNCTION(BlueprintCallable, Category = "Geological Features")
    void GenerateValley(const FVector& StartPoint, const FVector& EndPoint, float Depth, float Width);

    UFUNCTION(BlueprintCallable, Category = "Geological Features")
    void GenerateRiverBed(const TArray<FVector>& RiverPath, float Width, float Depth);

    UFUNCTION(BlueprintCallable, Category = "Geological Features")
    void GenerateVolcanicCone(const FVector& Location, float BaseRadius, float Height);

    UFUNCTION(BlueprintCallable, Category = "Geological Features")
    void GenerateCliffFace(const FVector& StartPoint, const FVector& EndPoint, float Height);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    float GetHeightAtWorldLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    float GetSlopeAtWorldLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    EWorld_GeologicalFormation GetFormationAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    ETerrainLayer GetTerrainLayerAtLocation(const FVector& WorldLocation) const;

    // Advanced features
    UFUNCTION(BlueprintCallable, Category = "Advanced Features")
    void SimulateWeatherErosion(int32 Years);

    UFUNCTION(BlueprintCallable, Category = "Advanced Features")
    void ApplyTectonicForces(const FVector& ForceDirection, float Magnitude);

    UFUNCTION(BlueprintCallable, Category = "Advanced Features")
    void CreateSedimentLayers();

    // Performance and optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTerrainLOD(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainMesh();

protected:
    // Generation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FWorld_TerrainGenerationSettings GenerationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 TerrainSeed = 42;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FVector TerrainScale = FVector(100.0f, 100.0f, 100.0f);

    // Terrain layers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layers")
    TArray<FTerrainLayerInfo> TerrainLayers;

    // Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialInterface> LandscapeMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<ETerrainLayer, UMaterialInterface*> LayerMaterials;

    // Generated landscape reference
    UPROPERTY()
    TWeakObjectPtr<ALandscape> GeneratedLandscape;

    // Heightmap data
    UPROPERTY()
    TArray<float> HeightmapData;

    UPROPERTY()
    TArray<uint8> LayerWeightData;

private:
    // Internal generation functions
    void InitializeHeightmapData();
    void GenerateBaseNoise();
    void ApplyGeologicalFeatures();
    void SimulateHydraulicErosion();
    void SimulateThermalErosion();
    void CalculateTerrainLayers();
    void ApplyLayerWeights();

    // Noise generation utilities
    float GenerateRidgedNoise(float X, float Y, int32 Octaves) const;
    float GenerateBillowNoise(float X, float Y, int32 Octaves) const;
    float GenerateVoronoiNoise(float X, float Y) const;
    float GenerateFBMNoise(float X, float Y, int32 Octaves, float Persistence, float Lacunarity) const;

    // Geological utilities
    void CreateMountainFormation(int32 X, int32 Y, float Intensity);
    void CreateValleyFormation(int32 X, int32 Y, float Intensity);
    void CreatePlateauFormation(int32 X, int32 Y, float Intensity);
    void CreateRiverFormation(int32 X, int32 Y, float Intensity);

    // Erosion simulation
    struct FWaterDroplet
    {
        FVector2D Position;
        FVector2D Velocity;
        float Water;
        float Sediment;
        float Speed;
    };

    void SimulateWaterDroplet(FWaterDroplet& Droplet);
    float CalculateSedimentCapacity(float Speed, float Water) const;

    // Layer calculation utilities
    float CalculateLayerWeight(const FTerrainLayerInfo& LayerInfo, float Height, float Slope) const;
    void BlendLayerWeights(int32 X, int32 Y);

public:
    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTerrainGenerated, bool, bSuccess);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTerrainGenerated OnTerrainGenerated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGeologicalFeatureCreated, EWorld_GeologicalFormation, FormationType, FVector, Location);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnGeologicalFeatureCreated OnGeologicalFeatureCreated;
};