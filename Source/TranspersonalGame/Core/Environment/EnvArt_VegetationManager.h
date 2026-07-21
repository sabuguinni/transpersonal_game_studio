#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "EnvArt_VegetationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_VegetationCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 PlantCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    FEnvArt_VegetationCluster()
    {
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        PlantCount = 20;
        BiomeType = EBiomeType::Savana;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEnvArt_VegetationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEnvArt_VegetationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SpawnVegetationCluster(const FEnvArt_VegetationCluster& ClusterData);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void PopulateBiome(EBiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ClearVegetationInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
    void PopulateAllBiomes();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FEnvArt_VegetationCluster> VegetationClusters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TMap<EBiomeType, TArray<TSoftObjectPtr<UStaticMesh>>> BiomeVegetationAssets;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    TArray<TWeakObjectPtr<AActor>> SpawnedVegetationActors;

private:
    void LoadBiomeAssets();
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius);
    FRotator GetRandomRotation();
    UStaticMesh* GetRandomVegetationMesh(EBiomeType BiomeType);
};