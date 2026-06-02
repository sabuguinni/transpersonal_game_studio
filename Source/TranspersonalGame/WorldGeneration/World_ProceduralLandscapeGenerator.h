#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeInfo.h"
#include "Landscape/LandscapeComponent.h"
#include "Landscape/LandscapeStreamingProxy.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_ProceduralLandscapeGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LandscapeLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Layer")
    FString LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Layer")
    UTexture2D* HeightmapTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Layer")
    UTexture2D* WeightmapTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Layer")
    UMaterialInterface* LayerMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Layer")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Layer")
    float NoiseScale;

    FWorld_LandscapeLayer()
    {
        LayerName = TEXT("DefaultLayer");
        HeightmapTexture = nullptr;
        WeightmapTexture = nullptr;
        LayerMaterial = nullptr;
        BlendWeight = 1.0f;
        NoiseScale = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 ComponentSizeQuads;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 SectionsPerComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 QuadsPerSection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MaxHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MinHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 NoiseOctaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoisePersistence;

    FWorld_TerrainSettings()
    {
        ComponentSizeQuads = 63;
        SectionsPerComponent = 2;
        QuadsPerSection = 31;
        MaxHeight = 2000.0f;
        MinHeight = -500.0f;
        NoiseFrequency = 0.01f;
        NoiseOctaves = 4;
        NoisePersistence = 0.5f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_ProceduralLandscapeGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_ProceduralLandscapeGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Landscape Generation Functions
    UFUNCTION(BlueprintCallable, Category = "Landscape Generation")
    ALandscape* GenerateProceduralLandscape(const FVector& Location, const FWorld_TerrainSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Landscape Generation")
    void GenerateHeightmapData(TArray<uint16>& HeightData, const FWorld_TerrainSettings& Settings, int32 SizeX, int32 SizeY);

    UFUNCTION(BlueprintCallable, Category = "Landscape Generation")
    void ApplyPerlinNoise(TArray<uint16>& HeightData, const FWorld_TerrainSettings& Settings, int32 SizeX, int32 SizeY);

    UFUNCTION(BlueprintCallable, Category = "Landscape Generation")
    void CreateLandscapeLayers(ALandscape* Landscape, const TArray<FWorld_LandscapeLayer>& Layers);

    UFUNCTION(BlueprintCallable, Category = "Landscape Generation")
    void GenerateBiomeBasedTerrain(EBiomeType BiomeType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Landscape Generation")
    void CreateRiverBeds(ALandscape* Landscape, const TArray<FVector>& RiverPath);

    UFUNCTION(BlueprintCallable, Category = "Landscape Generation")
    void CreateMountainRanges(ALandscape* Landscape, const TArray<FVector>& MountainPeaks);

    UFUNCTION(BlueprintCallable, Category = "Landscape Generation")
    void CreateValleys(ALandscape* Landscape, const TArray<FVector>& ValleyPoints);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Landscape Utils")
    float GeneratePerlinNoise(float X, float Y, float Frequency, int32 Octaves, float Persistence);

    UFUNCTION(BlueprintCallable, Category = "Landscape Utils")
    float SampleHeightAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Landscape Utils")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Landscape Utils")
    void OptimizeLandscapeForPerformance(ALandscape* Landscape);

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void GenerateTestLandscape();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ClearAllLandscapes();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ValidateLandscapeGeneration();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    FWorld_TerrainSettings DefaultTerrainSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Layers")
    TArray<FWorld_LandscapeLayer> LandscapeLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    UMaterialInterface* DefaultLandscapeMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxLandscapeComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    bool bGenerateBiomeSpecificTerrain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    float BiomeTransitionDistance;

private:
    UPROPERTY()
    TArray<ALandscape*> GeneratedLandscapes;

    UPROPERTY()
    TMap<EBiomeType, FWorld_TerrainSettings> BiomeTerrainSettings;

    // Helper functions
    void InitializeBiomeTerrainSettings();
    float InterpolateHeight(float Height1, float Height2, float Alpha);
    void ApplyBiomeSpecificModifications(TArray<uint16>& HeightData, EBiomeType BiomeType, int32 SizeX, int32 SizeY);
    UTexture2D* CreateHeightmapTexture(const TArray<uint16>& HeightData, int32 SizeX, int32 SizeY);
    void SetupLandscapeMaterial(ALandscape* Landscape, EBiomeType BiomeType);
};