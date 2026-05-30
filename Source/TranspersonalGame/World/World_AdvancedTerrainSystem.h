#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Classes/Landscape.h"
#include "Landscape/Classes/LandscapeInfo.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialParameterCollection.h"
#include "World_AdvancedTerrainSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_TerrainType : uint8
{
    Plains      UMETA(DisplayName = "Plains"),
    Hills       UMETA(DisplayName = "Hills"),
    Mountains   UMETA(DisplayName = "Mountains"),
    Valleys     UMETA(DisplayName = "Valleys"),
    Cliffs      UMETA(DisplayName = "Cliffs"),
    Plateaus    UMETA(DisplayName = "Plateaus"),
    Canyons     UMETA(DisplayName = "Canyons"),
    Ridges      UMETA(DisplayName = "Ridges")
};

USTRUCT(BlueprintType)
struct FWorld_TerrainLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    class UMaterialInterface* LayerMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MinHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MaxHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Slope;

    FWorld_TerrainLayer()
    {
        LayerName = TEXT("DefaultLayer");
        LayerMaterial = nullptr;
        BlendWeight = 1.0f;
        MinHeight = 0.0f;
        MaxHeight = 1000.0f;
        Slope = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FWorld_BiomeTerrainConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Terrain")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Terrain")
    EWorld_TerrainType PrimaryTerrainType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Terrain")
    TArray<FWorld_TerrainLayer> TerrainLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Terrain")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Terrain")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Terrain")
    float HeightVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Terrain")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Terrain")
    int32 OctaveCount;

    FWorld_BiomeTerrainConfig()
    {
        BiomeName = TEXT("DefaultBiome");
        PrimaryTerrainType = EWorld_TerrainType::Plains;
        BiomeCenter = FVector::ZeroVector;
        BiomeRadius = 30000.0f;
        HeightVariation = 500.0f;
        NoiseScale = 0.001f;
        OctaveCount = 4;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_AdvancedTerrainSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_AdvancedTerrainSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core terrain generation
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateAdvancedTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateBiomeSpecificTerrain(const FWorld_BiomeTerrainConfig& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void CreateTerrainLayers();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ApplyTerrainMaterials();

    // Terrain modification
    UFUNCTION(BlueprintCallable, Category = "Terrain Modification")
    void ModifyTerrainHeight(FVector Location, float Radius, float Height, bool bAdditive = true);

    UFUNCTION(BlueprintCallable, Category = "Terrain Modification")
    void CreateRiver(const TArray<FVector>& RiverPoints, float RiverWidth, float RiverDepth);

    UFUNCTION(BlueprintCallable, Category = "Terrain Modification")
    void CreateLake(FVector Center, float Radius, float Depth);

    UFUNCTION(BlueprintCallable, Category = "Terrain Modification")
    void CreateCanyon(FVector StartPoint, FVector EndPoint, float Width, float Depth);

    // Terrain analysis
    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    float GetTerrainHeightAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    FVector GetTerrainNormalAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    float GetTerrainSlopeAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    EWorld_TerrainType GetTerrainTypeAtLocation(FVector WorldLocation);

    // Biome integration
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void SetupBiomeTerrainConfigs();

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void BlendBiomeTransitions();

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    FWorld_BiomeTerrainConfig GetBiomeConfigAtLocation(FVector WorldLocation);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateTerrainStreaming();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantTerrainDetails();

protected:
    // Terrain configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Config")
    TArray<FWorld_BiomeTerrainConfig> BiomeTerrainConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Config")
    class ALandscape* MainLandscape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Config")
    class UMaterialParameterCollection* TerrainParameterCollection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Config")
    class UTextureRenderTarget2D* HeightmapRenderTarget;

    // Generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 TerrainResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float GlobalHeightScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float TerrainScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxLODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float StreamingDistance;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableTerrainStreaming;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleComponents;

private:
    // Internal terrain data
    TArray<float> HeightData;
    TArray<FVector> NormalData;
    TArray<float> SlopeData;
    
    // Noise generation
    float GeneratePerlinNoise(float X, float Y, int32 Octaves, float Scale, float Persistence);
    float GenerateRidgedNoise(float X, float Y, int32 Octaves, float Scale);
    float GenerateBillowNoise(float X, float Y, int32 Octaves, float Scale);
    
    // Terrain utilities
    void CalculateNormals();
    void CalculateSlopes();
    void ApplyErosion(int32 Iterations, float Strength);
    void SmoothTerrain(int32 Iterations, float Strength);
    
    // Biome blending
    float CalculateBiomeInfluence(FVector Location, const FWorld_BiomeTerrainConfig& BiomeConfig);
    void BlendTerrainLayers(FVector Location, TArray<FWorld_TerrainLayer>& OutLayers);
};