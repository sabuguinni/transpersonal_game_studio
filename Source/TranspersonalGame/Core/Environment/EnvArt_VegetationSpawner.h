#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EnvArt_VegetationSpawner.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_VegetationCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> MeshVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float ClusterRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 MaxInstancesPerCluster = 50;

    FEnvArt_VegetationCluster()
    {
        Density = 1.0f;
        ClusterRadius = 500.0f;
        MaxInstancesPerCluster = 50;
        ScaleRange = FVector2D(0.8f, 1.2f);
    }
};

UCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_VegetationSpawner : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_VegetationSpawner();

protected:
    virtual void BeginPlay() override;

public:
    // Vegetation clusters for different biome types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Vegetation")
    FEnvArt_VegetationCluster ForestTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Vegetation")
    FEnvArt_VegetationCluster ForestUndergrowth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swampland Vegetation")
    FEnvArt_VegetationCluster SwampCypress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swampland Vegetation")
    FEnvArt_VegetationCluster SwampMoss;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canyon Vegetation")
    FEnvArt_VegetationCluster DesertCacti;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canyon Vegetation")
    FEnvArt_VegetationCluster RockFormations;

    // Spawning parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 MaxVegetationInstances = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    bool bUseRandomSeed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (EditCondition = "!bUseRandomSeed"))
    int32 RandomSeed = 12345;

    // Biome detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Detection")
    EBiomeType CurrentBiome = EBiomeType::Forest;

    // Spawning functions
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnVegetationForBiome(EBiomeType BiomeType, const FVector& CenterLocation);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnVegetationCluster(const FEnvArt_VegetationCluster& ClusterConfig, const FVector& CenterLocation);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void ClearAllVegetation();

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void RefreshVegetation();

    // Editor tools
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void SpawnTestVegetation();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ClearTestVegetation();

private:
    // Internal spawning helpers
    void SpawnForestVegetation(const FVector& CenterLocation);
    void SpawnSwamplandVegetation(const FVector& CenterLocation);
    void SpawnCanyonVegetation(const FVector& CenterLocation);
    
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius);
    bool IsValidSpawnLocation(const FVector& Location);
    void SpawnMeshInstance(UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale);

    // Spawned actors tracking
    UPROPERTY()
    TArray<AActor*> SpawnedVegetationActors;

    // Random stream for consistent results
    FRandomStream RandomStream;
};