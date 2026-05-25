#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_TerrainFeatureManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    FString FeatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    FString AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    FVector BaseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    float SpawnWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    EBiomeType BiomeType;

    FWorld_TerrainFeature()
    {
        FeatureName = TEXT("DefaultFeature");
        AssetPath = TEXT("");
        BaseScale = FVector(1.0f, 1.0f, 1.0f);
        SpawnWeight = 1.0f;
        BiomeType = EBiomeType::Savana;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeTerrainConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    TArray<FWorld_TerrainFeature> TerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    int32 MinFeaturesPerKm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    int32 MaxFeaturesPerKm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float HeightVariation;

    FWorld_BiomeTerrainConfig()
    {
        BiomeType = EBiomeType::Savana;
        MinFeaturesPerKm = 5;
        MaxFeaturesPerKm = 15;
        HeightVariation = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_TerrainFeatureManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_TerrainFeatureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<FWorld_BiomeTerrainConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    int32 MaxTerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float FeatureSpacing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    bool bAutoGenerateOnBeginPlay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime State")
    TArray<AActor*> SpawnedTerrainFeatures;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime State")
    int32 TotalFeaturesSpawned;

public:
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainFeatures();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainForBiome(EBiomeType BiomeType, FVector BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ClearAllTerrainFeatures();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    AActor* SpawnTerrainFeature(const FWorld_TerrainFeature& Feature, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    FWorld_TerrainFeature SelectRandomFeatureForBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void InitializeBiomeConfigs();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    FVector GetRandomLocationInBiome(FVector BiomeCenter, float BiomeRadius, float HeightBase);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogTerrainStats();

private:
    void LoadTerrainAssets();
    bool IsValidSpawnLocation(FVector Location, float MinDistance);
    FRotator GetRandomTerrainRotation();
    FVector GetRandomTerrainScale(const FWorld_TerrainFeature& Feature);
};