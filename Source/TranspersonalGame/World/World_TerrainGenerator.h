#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_TerrainGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_TerrainType TerrainType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Roughness;

    FWorld_TerrainFeature()
    {
        TerrainType = EWorld_TerrainType::Hill;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        Height = 100.0f;
        Roughness = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterBody
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    EWorld_WaterType WaterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector2D Size;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bIsSwimmable;

    FWorld_WaterBody()
    {
        WaterType = EWorld_WaterType::Lake;
        Location = FVector::ZeroVector;
        Size = FVector2D(1000.0f, 1000.0f);
        Depth = 200.0f;
        FlowSpeed = 0.0f;
        bIsSwimmable = true;
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

    // Terrain generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    int32 TerrainSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float TerrainScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float MaxHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float NoiseFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    int32 TerrainResolution;

    // Biome-specific terrain features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Features")
    TArray<FWorld_TerrainFeature> TerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Features")
    TArray<FWorld_WaterBody> WaterBodies;

    // Generation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinFeatureDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxFeaturesPerBiome;

public:
    // Terrain generation methods
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainForBiome(EWorld_BiomeType BiomeType, FVector BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateWaterFeatures(EWorld_BiomeType BiomeType, FVector BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainFeatures(EWorld_BiomeType BiomeType, FVector BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ClearGeneratedTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    FVector GetOptimalLocationForFeature(EWorld_TerrainType TerrainType, FVector BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    float CalculateTerrainHeight(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    bool IsLocationSuitableForWater(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void RegenerateAllTerrain();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void PreviewTerrainGeneration();

protected:
    // Internal generation helpers
    void GenerateHill(FVector Location, float Height, float Radius);
    void GenerateValley(FVector Location, float Depth, float Radius);
    void GenerateRockFormation(FVector Location, float Scale);
    void GenerateWaterBody(FWorld_WaterBody WaterBodyData);

    // Noise generation
    float GeneratePerlinNoise(float X, float Y, float Frequency);
    float GenerateRidgedNoise(float X, float Y, float Frequency);

    // Utility methods
    bool IsValidTerrainLocation(FVector Location);
    FVector SnapToTerrain(FVector Location);

private:
    TArray<AActor*> GeneratedActors;
    bool bIsGenerating;
};