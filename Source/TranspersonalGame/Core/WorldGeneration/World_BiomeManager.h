#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;

    FWorld_BiomeData()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        TerrainHeight = 100.0f;
        VegetationDensity = 0.5f;
        Temperature = 25.0f;
        Humidity = 0.5f;
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
    FString FeatureType;

    FWorld_TerrainFeature()
    {
        FeatureName = TEXT("Feature");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        FeatureType = TEXT("Rock");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<FWorld_TerrainFeature> TerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxFeaturesPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float FeatureSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateOnBeginPlay;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FWorld_BiomeData GetBiomeAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    TArray<FWorld_BiomeData> GetAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainFeatures();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void SpawnTerrainFeature(const FWorld_TerrainFeature& Feature);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    float GetBiomeInfluenceAtLocation(FVector Location, const FWorld_BiomeData& Biome);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void ClearAllTerrainFeatures();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorGenerateBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorClearBiomes();

private:
    UPROPERTY()
    TArray<AActor*> SpawnedFeatureActors;

    void SetupDefaultBiomes();
    FVector GetRandomLocationInBiome(const FWorld_BiomeData& Biome);
    FRotator GetRandomRotation();
    FVector GetRandomScale(float MinScale, float MaxScale);
};