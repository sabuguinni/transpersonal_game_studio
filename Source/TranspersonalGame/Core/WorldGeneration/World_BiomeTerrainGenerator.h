#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_BiomeTerrainGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationAssetPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> TerrainFeatureAssetPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 TerrainFeatureDensity;

    FWorld_BiomeConfiguration()
    {
        BiomeName = TEXT("DefaultBiome");
        CenterLocation = FVector::ZeroVector;
        BiomeRadius = 10000.0f;
        VegetationDensity = 50;
        TerrainFeatureDensity = 25;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString FeatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString AssetPath;

    FWorld_TerrainFeature()
    {
        FeatureName = TEXT("DefaultFeature");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        AssetPath = TEXT("");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeTerrainGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeTerrainGenerator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TArray<FWorld_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TArray<FWorld_TerrainFeature> GeneratedTerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 RandomSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    bool bAutoGenerateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float MinDistanceBetweenFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float TerrainVariationScale;

public:
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrainForBiome(const FWorld_BiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateAllBiomeTerrain();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearGeneratedTerrain();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FVector GetRandomLocationInBiome(const FWorld_BiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool IsLocationValidForFeature(const FVector& Location, float MinDistance);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    AStaticMeshActor* SpawnTerrainFeature(const FString& AssetPath, const FVector& Location, const FRotator& Rotation, const FVector& Scale);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SetBiomeConfiguration(const TArray<FWorld_BiomeConfiguration>& NewConfigurations);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<FWorld_BiomeConfiguration> GetBiomeConfigurations() const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    int32 GetTerrainFeatureCount() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Generation")
    void EditorGenerateTerrain();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Generation")
    void EditorClearTerrain();

private:
    void SetupDefaultBiomes();
    float GetPerlinNoise(float X, float Y, float Scale) const;
    FRotator GetRandomRotationVariation() const;
    FVector GetRandomScaleVariation() const;
};