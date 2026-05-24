#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_ProceduralBiomeSpawner.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_ProceduralBiomeSpawner : public AActor
{
    GENERATED_BODY()

public:
    AWorld_ProceduralBiomeSpawner();

protected:
    virtual void BeginPlay() override;

    // Biome configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TArray<FWorld_BiomeZoneData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    float BiomeSpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    int32 MaxBiomeActorsPerZone = 50;

    // Water system configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    TArray<FWorld_WaterBodyData> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    float WaterLevel = 45.0f;

    // Vegetation configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FWorld_VegetationCluster> VegetationClusters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationDensity = 0.7f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxDrawDistance = 10000.0f;

public:
    // Core spawning functions
    UFUNCTION(BlueprintCallable, Category = "Biome Spawning")
    void SpawnAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Spawning")
    void SpawnBiomeZone(const FWorld_BiomeZoneData& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void SpawnWaterSystems();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    AActor* SpawnWaterBody(const FWorld_WaterBodyData& WaterData);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnVegetationClusters();

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnVegetationCluster(const FWorld_VegetationCluster& ClusterData);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Biome Utilities")
    FVector GetRandomLocationInBiome(const FWorld_BiomeZoneData& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Biome Utilities")
    bool IsLocationInBiome(const FVector& Location, const FWorld_BiomeZoneData& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetTotalSpawnedActors() const;

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ClearAllBiomes();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void RegenerateAllBiomes();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ValidateBiomeConfiguration();

private:
    // Internal tracking
    UPROPERTY()
    TArray<AActor*> SpawnedBiomeActors;

    UPROPERTY()
    TArray<AActor*> SpawnedWaterActors;

    UPROPERTY()
    TArray<AActor*> SpawnedVegetationActors;

    // Internal helper functions
    AActor* SpawnStaticMeshActor(UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale);
    void ConfigureBiomeActor(AActor* Actor, const FWorld_BiomeZoneData& ZoneData);
    void ConfigureWaterActor(AActor* Actor, const FWorld_WaterBodyData& WaterData);
    void ConfigureVegetationActor(AActor* Actor, const FWorld_VegetationCluster& ClusterData);
    
    // Performance optimization helpers
    void SetActorLODSettings(AActor* Actor, float DrawDistance);
    void OptimizeActorCollision(AActor* Actor, bool bComplexCollision);
};