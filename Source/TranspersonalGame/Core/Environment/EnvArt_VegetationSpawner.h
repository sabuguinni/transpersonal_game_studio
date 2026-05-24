#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "EnvArt_VegetationSpawner.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_VegetationCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector ClusterCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float ClusterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 DensityCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EBiomeType BiomeType;

    FEnvArt_VegetationCluster()
    {
        ClusterCenter = FVector::ZeroVector;
        ClusterRadius = 1000.0f;
        DensityCount = 50;
        BiomeType = EBiomeType::Savana;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_VegetationSpawner : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_VegetationSpawner();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation System")
    TArray<FEnvArt_VegetationCluster> VegetationClusters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation System")
    bool bAutoSpawnOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation System")
    float GlobalDensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVegetationActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullDistance;

    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void SpawnVegetationClusters();

    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void SpawnClusterAtLocation(const FEnvArt_VegetationCluster& Cluster);

    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void ClearAllVegetation();

    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void LoadBiomeVegetationAssets(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    int32 GetSpawnedVegetationCount() const;

private:
    UPROPERTY()
    TArray<AActor*> SpawnedVegetationActors;

    void SpawnVegetationAtPoint(UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation);
    FVector GetRandomPointInCluster(const FEnvArt_VegetationCluster& Cluster) const;
    FRotator GetRandomVegetationRotation() const;
};