#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_TerrainAssetManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainAssetData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Asset")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Asset")
    FString AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Asset")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Asset")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Asset")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Asset")
    float SpawnProbability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Asset")
    bool bIsLandmark;

    FWorld_TerrainAssetData()
    {
        AssetName = TEXT("");
        AssetPath = TEXT("");
        BiomeType = EBiomeType::Savana;
        SpawnLocation = FVector::ZeroVector;
        Scale = FVector::OneVector;
        SpawnProbability = 1.0f;
        bIsLandmark = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeTerrainConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    TArray<FWorld_TerrainAssetData> TerrainAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    int32 MinAssetsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    int32 MaxAssetsPerBiome;

    FWorld_BiomeTerrainConfig()
    {
        BiomeType = EBiomeType::Savana;
        BiomeCenter = FVector::ZeroVector;
        BiomeRadius = 25000.0f;
        MinAssetsPerBiome = 100;
        MaxAssetsPerBiome = 500;
    }
};

/**
 * Manages terrain asset spawning and distribution across different biomes
 * Integrates with Meshy API pipeline for procedural 3D asset generation
 * Handles biome-specific terrain placement and optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_TerrainAssetManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_TerrainAssetManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Management")
    TArray<FWorld_BiomeTerrainConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Management")
    TArray<AActor*> SpawnedTerrainActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Pipeline")
    FString MeshyAssetDirectory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Pipeline")
    bool bAutoLoadMeshyAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxSpawnDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveTerrainActors;

public:
    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void InitializeBiomeConfigs();

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void SpawnTerrainForBiome(EBiomeType BiomeType, int32 AssetCount = 100);

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void SpawnAllBiomeTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void ClearTerrainForBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void ClearAllTerrain();

    UFUNCTION(BlueprintCallable, Category = "Asset Pipeline")
    void LoadMeshyAssetsFromDirectory();

    UFUNCTION(BlueprintCallable, Category = "Asset Pipeline")
    bool RegisterTerrainAsset(const FString& AssetName, const FString& AssetPath, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    FVector GetBiomeCenter(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    TArray<AActor*> GetTerrainActorsInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void CreateDebugTerrainMarkers();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogBiomeTerrainStats();

private:
    void SpawnTerrainAsset(const FWorld_TerrainAssetData& AssetData, const FVector& Location);
    FVector GenerateRandomLocationInBiome(EBiomeType BiomeType) const;
    bool IsLocationValidForTerrain(const FVector& Location) const;
    void UpdateTerrainLOD(AActor* TerrainActor, float DistanceToPlayer);
};