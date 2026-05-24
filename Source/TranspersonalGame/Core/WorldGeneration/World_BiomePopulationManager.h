#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_BiomePopulationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomePopulationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Population")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Population")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Population")
    float PopulationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Population")
    int32 TargetActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Population")
    int32 CurrentActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Population")
    TArray<FString> PreferredAssetPaths;

    FWorld_BiomePopulationData()
    {
        BiomeType = EWorld_BiomeType::Savanna;
        CenterLocation = FVector::ZeroVector;
        PopulationRadius = 25000.0f;
        TargetActorCount = 500;
        CurrentActorCount = 0;
        PreferredAssetPaths.Empty();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainActorSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
    FVector SpawnScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
    FString AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
    EWorld_BiomeType TargetBiome;

    FWorld_TerrainActorSpawnData()
    {
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        SpawnScale = FVector::OneVector;
        AssetPath = TEXT("");
        TargetBiome = EWorld_BiomeType::Savanna;
    }
};

/**
 * World Biome Population Manager
 * Manages the procedural population of biomes with terrain actors, vegetation, and environmental elements.
 * Integrates with the physics performance system to optimize actor density based on performance metrics.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomePopulationManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomePopulationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core biome population data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Population")
    TArray<FWorld_BiomePopulationData> BiomePopulationData;

    // Performance integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxPhysicsActorsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceOptimizationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUsePerformanceBasedDensity;

    // Population control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population Control")
    float PopulationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population Control")
    int32 ActorsPerUpdateCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population Control")
    bool bAutoPopulateBiomes;

    // Asset management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<FString> AvailableTerrainAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<FString> AvailableVegetationAssets;

    // Biome population methods
    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    void InitializeBiomeData();

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    void PopulateBiome(EWorld_BiomeType BiomeType, int32 ActorCount);

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    void PopulateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    FVector GetBiomeCenterLocation(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    int32 GetBiomeActorCount(EWorld_BiomeType BiomeType) const;

    // Asset spawning methods
    UFUNCTION(BlueprintCallable, Category = "Asset Spawning")
    AStaticMeshActor* SpawnTerrainActor(const FWorld_TerrainActorSpawnData& SpawnData);

    UFUNCTION(BlueprintCallable, Category = "Asset Spawning")
    TArray<AStaticMeshActor*> SpawnActorsInRadius(const FVector& CenterLocation, float Radius, int32 ActorCount, EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Asset Spawning")
    FString SelectAssetForBiome(EWorld_BiomeType BiomeType) const;

    // Performance integration methods
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePopulationBasedOnPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetOptimalActorDensity(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomeActorCount(EWorld_BiomeType BiomeType);

    // Utility methods
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ClearBiomeActors(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void RefreshBiomePopulation(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<AActor*> GetBiomeActors(EWorld_BiomeType BiomeType) const;

private:
    // Internal state
    float LastPopulationUpdate;
    TMap<EWorld_BiomeType, TArray<AActor*>> SpawnedActorsByBiome;
    
    // Helper methods
    FVector GenerateRandomLocationInRadius(const FVector& Center, float Radius) const;
    FRotator GenerateRandomRotation() const;
    FVector GenerateRandomScale(float MinScale = 0.5f, float MaxScale = 2.0f) const;
    bool IsValidSpawnLocation(const FVector& Location) const;
    void UpdateBiomeActorCount(EWorld_BiomeType BiomeType);
};