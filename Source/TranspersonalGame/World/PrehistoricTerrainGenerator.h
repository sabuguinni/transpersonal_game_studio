// Copyright Transpersonal Game Studio. All Rights Reserved.
// PrehistoricTerrainGenerator.h - Advanced terrain generation for Jurassic/Cretaceous environments

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "PrehistoricTerrainGenerator.generated.h"

class ULandscapeComponent;
class ULandscapeInfo;
class ALandscape;
class UTexture2D;
class UMaterialParameterCollection;

UENUM(BlueprintType)
enum class ETerrainFeature : uint8
{
    RiverValley         UMETA(DisplayName = "River Valley"),
    MountainRidge       UMETA(DisplayName = "Mountain Ridge"),
    VolcanicCone        UMETA(DisplayName = "Volcanic Cone"),
    CanyonSystem        UMETA(DisplayName = "Canyon System"),
    PlateauFormation    UMETA(DisplayName = "Plateau Formation"),
    CoastalCliffs       UMETA(DisplayName = "Coastal Cliffs"),
    GeothermalVents     UMETA(DisplayName = "Geothermal Vents"),
    AncientCrater       UMETA(DisplayName = "Ancient Crater")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTerrainFeatureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature")
    ETerrainFeature FeatureType = ETerrainFeature::RiverValley;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature")
    float Radius = 5000.0f; // 50 meters default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature")
    float Falloff = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature")
    bool bActive = true;

    FTerrainFeatureData()
    {
        FeatureType = ETerrainFeature::RiverValley;
        Location = FVector::ZeroVector;
        Intensity = 1.0f;
        Radius = 5000.0f;
        Falloff = 0.5f;
        bActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FGeologicalLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    FString LayerName = TEXT("Sedimentary");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    float Thickness = 100.0f; // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    float Hardness = 0.5f; // 0-1 scale

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    FLinearColor LayerColor = FLinearColor::Brown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    TSoftObjectPtr<UMaterialInterface> LayerMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    float ErosionResistance = 0.5f;

    FGeologicalLayer()
    {
        LayerName = TEXT("Sedimentary");
        Thickness = 100.0f;
        Hardness = 0.5f;
        LayerColor = FLinearColor::Brown;
        ErosionResistance = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPrehistoricTerrainSettings
{
    GENERATED_BODY()

    // Base terrain parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Terrain")
    int32 WorldSizeKm = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Terrain")
    float MaxElevation = 1000.0f; // meters

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Terrain")
    float SeaLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Terrain")
    float BaseRoughness = 0.6f;

    // Noise settings for realistic terrain
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Generation")
    int32 NoiseOctaves = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Generation")
    float NoiseFrequency = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Generation")
    float NoisePersistence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Generation")
    float NoiseLacunarity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Generation")
    int32 TerrainSeed = 42;

    // Geological features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    TArray<FGeologicalLayer> GeologicalLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    float TectonicActivity = 0.3f; // 0-1 scale

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geology")
    float VolcanicActivity = 0.2f; // 0-1 scale

    // Erosion simulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    bool bSimulateErosion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    int32 ErosionIterations = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float RainfallIntensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float EvaporationRate = 0.3f;

    // Climate parameters (affects erosion and vegetation)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float AverageTemperature = 28.0f; // Celsius - warm Mesozoic climate

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float AverageHumidity = 0.8f; // High humidity in Mesozoic

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float CO2Level = 1800.0f; // ppm - much higher than modern

    FPrehistoricTerrainSettings()
    {
        WorldSizeKm = 4;
        MaxElevation = 1000.0f;
        SeaLevel = 0.0f;
        BaseRoughness = 0.6f;
        NoiseOctaves = 8;
        NoiseFrequency = 0.001f;
        NoisePersistence = 0.5f;
        NoiseLacunarity = 2.0f;
        TerrainSeed = 42;
        TectonicActivity = 0.3f;
        VolcanicActivity = 0.2f;
        bSimulateErosion = true;
        ErosionIterations = 100;
        RainfallIntensity = 0.7f;
        EvaporationRate = 0.3f;
        AverageTemperature = 28.0f;
        AverageHumidity = 0.8f;
        CO2Level = 1800.0f;
    }
};

/**
 * APrehistoricTerrainGenerator
 * 
 * Advanced terrain generation system specifically designed for Jurassic/Cretaceous environments.
 * Creates geologically accurate prehistoric landscapes with proper erosion simulation,
 * geological layering, and climate-appropriate features.
 * 
 * Key Features:
 * - Multi-octave Perlin noise for realistic terrain
 * - Geological layer simulation
 * - Hydraulic erosion simulation
 * - Tectonic and volcanic activity modeling
 * - Climate-based terrain modification
 * - Real-time terrain feature placement
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrehistoricTerrainGenerator : public AActor
{
    GENERATED_BODY()

public:
    APrehistoricTerrainGenerator();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Main generation interface
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainAsync();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void RegenerateArea(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void AddTerrainFeature(const FTerrainFeatureData& FeatureData);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void RemoveTerrainFeature(int32 FeatureIndex);

    // Terrain analysis
    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    float GetElevationAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    FVector GetTerrainNormalAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    float GetSlopeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    FGeologicalLayer GetGeologicalLayerAtLocation(const FVector& WorldLocation) const;

    // Climate and environmental data
    UFUNCTION(BlueprintCallable, Category = "Climate")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Climate")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Climate")
    float GetRainfallAtLocation(const FVector& WorldLocation) const;

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawTerrainFeatures(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawGeologicalLayers(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ExportHeightmapToTexture(const FString& FilePath);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTerrainGenerationComplete, bool, bSuccess);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerrainFeatureAdded, ETerrainFeature, FeatureType, FVector, Location);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTerrainGenerationComplete OnTerrainGenerationComplete;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTerrainFeatureAdded OnTerrainFeatureAdded;

protected:
    // Terrain generation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    FPrehistoricTerrainSettings TerrainSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Features")
    TArray<FTerrainFeatureData> TerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TSoftObjectPtr<ALandscape> TargetLandscape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TSoftObjectPtr<UMaterialParameterCollection> TerrainParameterCollection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bGenerateAsync = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GenerationBudgetMs = 16.0f; // 60fps budget

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ChunkSize = 64; // Process terrain in chunks

private:
    // Core generation functions
    void GenerateBaseHeightmap();
    void ApplyGeologicalLayers();
    void SimulateErosion();
    void ApplyTerrainFeatures();
    void ApplyClimateEffects();

    // Noise generation
    float GenerateMultiOctaveNoise(float X, float Y, const FPrehistoricTerrainSettings& Settings) const;
    float GenerateRidgedNoise(float X, float Y, int32 Octaves, float Frequency, float Persistence) const;
    float GenerateBillowNoise(float X, float Y, int32 Octaves, float Frequency, float Persistence) const;

    // Erosion simulation
    void SimulateHydraulicErosion(TArray<float>& HeightData, int32 Width, int32 Height);
    void SimulateThermalErosion(TArray<float>& HeightData, int32 Width, int32 Height);
    void DropletErosion(TArray<float>& HeightData, int32 Width, int32 Height, int32 Iterations);

    // Feature generation
    void ApplyRiverValley(const FTerrainFeatureData& Feature, TArray<float>& HeightData, int32 Width, int32 Height);
    void ApplyMountainRidge(const FTerrainFeatureData& Feature, TArray<float>& HeightData, int32 Width, int32 Height);
    void ApplyVolcanicCone(const FTerrainFeatureData& Feature, TArray<float>& HeightData, int32 Width, int32 Height);
    void ApplyCanyonSystem(const FTerrainFeatureData& Feature, TArray<float>& HeightData, int32 Width, int32 Height);

    // Utility functions
    float SampleHeightData(const TArray<float>& HeightData, int32 Width, int32 Height, float X, float Y) const;
    FVector CalculateNormal(const TArray<float>& HeightData, int32 Width, int32 Height, int32 X, int32 Y) const;
    float CalculateSlope(const FVector& Normal) const;

    // Generation state
    bool bIsGenerating = false;
    bool bGenerationComplete = false;
    float GenerationProgress = 0.0f;
    int32 CurrentChunk = 0;
    int32 TotalChunks = 0;

    // Cached data
    TArray<float> CachedHeightData;
    TArray<FVector> CachedNormalData;
    TArray<FGeologicalLayer> CachedLayerData;
    int32 CachedDataWidth = 0;
    int32 CachedDataHeight = 0;

    // Debug visualization
    bool bDebugDrawFeatures = false;
    bool bDebugDrawLayers = false;
    TArray<FVector> DebugFeaturePoints;
    TArray<FColor> DebugFeatureColors;

    // Performance tracking
    double GenerationStartTime = 0.0;
    TArray<float> ChunkProcessingTimes;
};