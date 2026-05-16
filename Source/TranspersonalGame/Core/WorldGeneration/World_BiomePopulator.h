#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_BiomePopulator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 TargetActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> TerrainAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainFeatureDensity;

    FWorld_BiomeData()
    {
        BiomeType = EBiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        TargetActorCount = 500;
        VegetationDensity = 0.7f;
        TerrainFeatureDensity = 0.3f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_SpawnPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MinDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float ClusterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 ClusterSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float RandomRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float ScaleVariation;

    FWorld_SpawnPattern()
    {
        MinDistance = 500.0f;
        MaxDistance = 2000.0f;
        ClusterRadius = 1000.0f;
        ClusterSize = 5;
        RandomRotation = 360.0f;
        ScaleVariation = 0.2f;
    }
};

/**
 * Biome Populator Component
 * Handles procedural population of biomes with vegetation, terrain features, and environmental actors
 * Integrates with the 3D asset pipeline for dynamic content generation
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomePopulator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomePopulator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    FWorld_SpawnPattern DefaultSpawnPattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PopulationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseAsyncSpawning;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedActors;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 TotalActorsSpawned;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    bool bPopulationComplete;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    void PopulateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    void PopulateBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    void ClearBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Population")
    void ClearAllBiomes();

    // Asset Management
    UFUNCTION(BlueprintCallable, Category = "Asset Management")
    void LoadBiomeAssets(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Asset Management")
    TArray<UStaticMesh*> GetAvailableAssetsForBiome(EBiomeType BiomeType);

    // Spawn Functions
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    AActor* SpawnActorInBiome(UStaticMesh* Mesh, const FVector& Location, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    TArray<FVector> GenerateSpawnLocations(const FWorld_BiomeData& BiomeData, int32 Count);

    // Validation and Metrics
    UFUNCTION(BlueprintCallable, Category = "Validation")
    int32 GetActorCountInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateBiomePopulation(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FString GetPopulationReport();

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomeActors(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODDistances(AActor* Actor, float LOD1Distance, float LOD2Distance);

private:
    // Internal Functions
    void InitializeBiomeConfigurations();
    FVector GetRandomLocationInBiome(const FWorld_BiomeData& BiomeData);
    bool IsValidSpawnLocation(const FVector& Location, const FWorld_BiomeData& BiomeData);
    void ApplyBiomeSpecificSettings(AActor* Actor, EBiomeType BiomeType);
    
    // Async Spawning
    void ProcessAsyncSpawning();
    TArray<FVector> PendingSpawnLocations;
    TArray<UStaticMesh*> PendingSpawnMeshes;
    int32 CurrentSpawnIndex;
    
    // Performance Tracking
    float LastSpawnTime;
    int32 ActorsSpawnedThisFrame;
};