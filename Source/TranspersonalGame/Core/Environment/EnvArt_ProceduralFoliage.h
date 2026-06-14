#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "EnvArt_ProceduralFoliage.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_FoliageType
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    UStaticMesh* Mesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    FString TypeName = "DefaultFoliage";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SpawnDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MinScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MaxScale = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MinSlope = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MaxSlope = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MinAltitude = -1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MaxAltitude = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EBiomeType> AllowedBiomes;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_FoliageCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    int32 InstanceCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    EBiomeType BiomeType = EBiomeType::Temperate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    TArray<int32> FoliageTypeIndices;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_ProceduralFoliage : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_ProceduralFoliage();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Foliage configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Types")
    TArray<FEnvArt_FoliageType> FoliageTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float GenerationRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxInstancesPerType = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float ClusterSize = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxClustersPerBiome = 20;

    // Runtime data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<FEnvArt_FoliageCluster> ActiveClusters;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TMap<int32, UInstancedStaticMeshComponent*> InstancedMeshComponents;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void GenerateFoliageForBiome(EBiomeType BiomeType, const FVector& CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void ClearAllFoliage();

    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void AddFoliageType(const FEnvArt_FoliageType& NewType);

    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void CreatePrehistoricForest();

    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void CreateSwamplandVegetation();

    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void CreateGrasslandVegetation();

    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void CreateDesertVegetation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void RegenerateAllFoliage();

private:
    // Internal generation functions
    void SpawnFoliageCluster(const FEnvArt_FoliageType& FoliageType, const FVector& ClusterCenter, float ClusterRadius, EBiomeType BiomeType);
    bool IsValidSpawnLocation(const FVector& Location, const FEnvArt_FoliageType& FoliageType, EBiomeType BiomeType);
    FVector GetRandomPointInCluster(const FVector& ClusterCenter, float Radius);
    float GetTerrainSlope(const FVector& Location);
    EBiomeType GetBiomeAtLocation(const FVector& Location);
    void InitializeDefaultFoliageTypes();
    UInstancedStaticMeshComponent* GetOrCreateInstancedMeshComponent(int32 FoliageTypeIndex);
};