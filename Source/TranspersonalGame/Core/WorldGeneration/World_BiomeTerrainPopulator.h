#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_BiomeTerrainPopulator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeatureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    FString FeatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    float Density;

    FWorld_TerrainFeatureData()
    {
        FeatureName = TEXT("DefaultFeature");
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        BiomeType = EBiomeType::Savanna;
        Density = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomePopulationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    int32 TargetActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    float SpreadRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    TArray<FString> PreferredAssetPaths;

    FWorld_BiomePopulationSettings()
    {
        BiomeType = EBiomeType::Savanna;
        TargetActorCount = 500;
        SpreadRadius = 10000.0f;
        BiomeCenter = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeTerrainPopulator : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeTerrainPopulator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Population")
    TArray<FWorld_BiomePopulationSettings> BiomeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Population")
    TArray<FWorld_TerrainFeatureData> SpawnedFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Population")
    bool bAutoPopulateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Population")
    int32 MaxActorsPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Population")
    float PopulationRadius;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    void PopulateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    void PopulateBiome(EBiomeType BiomeType, const FVector& Center, int32 ActorCount);

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    void ClearBiomePopulation(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    int32 GetBiomeActorCount(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    TArray<AActor*> GetBiomeActors(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    void SpawnTerrainFeature(const FWorld_TerrainFeatureData& FeatureData);

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    void InitializeBiomeSettings();

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    FVector GetRandomLocationInBiome(const FVector& BiomeCenter, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    void ValidateBiomePopulation();

private:
    void SpawnActorsInBiome(const FWorld_BiomePopulationSettings& Settings);
    TArray<UStaticMesh*> LoadAvailableStaticMeshes();
    UStaticMesh* SelectMeshForBiome(EBiomeType BiomeType, const TArray<UStaticMesh*>& AvailableMeshes);
    FVector GenerateRandomOffset(float Radius) const;
    bool IsValidSpawnLocation(const FVector& Location) const;
};