#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_ProceduralBiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 TerrainFeatureCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;

    FWorld_BiomeConfiguration()
    {
        BiomeName = TEXT("Default");
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        TerrainFeatureCount = 5;
        VegetationDensity = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString FeatureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 LODLevel;

    FWorld_TerrainFeature()
    {
        Location = FVector::ZeroVector;
        FeatureType = TEXT("Rock");
        Scale = 1.0f;
        LODLevel = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_ProceduralBiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_ProceduralBiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_TerrainFeature> GeneratedFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxFeaturesPerFrame;

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateTerrainFeatures(const FWorld_BiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void PopulateBiomeWithDinosaurs(const FWorld_BiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomePerformance();

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FWorld_BiomeConfiguration GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void RegenerateBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void ClearAllBiomes();

protected:
    UFUNCTION()
    void CreateBiomeMarker(const FWorld_BiomeConfiguration& BiomeConfig);

    UFUNCTION()
    void SpawnTerrainFeature(const FVector& Location, const FString& FeatureType, float Scale, int32 LODLevel);

    UFUNCTION()
    AActor* SpawnDinosaurInBiome(const FString& DinosaurSpecies, const FVector& Location);

private:
    UPROPERTY()
    TArray<AActor*> SpawnedBiomeActors;

    UPROPERTY()
    TArray<AActor*> SpawnedFeatureActors;

    UPROPERTY()
    TArray<AActor*> SpawnedDinosaurActors;

    float LastOptimizationTime;
    int32 CurrentFeatureIndex;
};