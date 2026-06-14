#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "EnvArt_RockFormationSpawner.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_RockFormationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    TArray<TSoftObjectPtr<UStaticMesh>> RockMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    FVector2D ScaleRange = FVector2D(0.8f, 2.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    float ClusterRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    int32 MinRocksPerCluster = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    int32 MaxRocksPerCluster = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    float SurfaceAlignmentStrength = 0.7f;

    FEnvArt_RockFormationData()
    {
        RockMeshes.Empty();
        ScaleRange = FVector2D(0.8f, 2.5f);
        ClusterRadius = 500.0f;
        MinRocksPerCluster = 3;
        MaxRocksPerCluster = 8;
        SurfaceAlignmentStrength = 0.7f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_RockFormationSpawner : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_RockFormationSpawner();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    FEnvArt_RockFormationData RockFormationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    TMap<EBiomeType, FEnvArt_RockFormationData> BiomeSpecificRocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 MaxFormations = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float MinDistanceBetweenFormations = 800.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedRockActors;

    UFUNCTION(BlueprintCallable, Category = "Rock Formation")
    void SpawnRockFormations();

    UFUNCTION(BlueprintCallable, Category = "Rock Formation")
    void SpawnSingleFormation(const FVector& Location, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Rock Formation")
    void ClearAllRockFormations();

    UFUNCTION(BlueprintCallable, Category = "Rock Formation")
    void UpdateFormationsForBiome(EBiomeType NewBiome);

private:
    bool IsValidSpawnLocation(const FVector& Location) const;
    FVector GetSurfaceLocation(const FVector& StartLocation) const;
    FRotator CalculateRockRotation(const FVector& SurfaceNormal) const;
    UStaticMesh* SelectRandomRockMesh(EBiomeType BiomeType) const;
    void InitializeBiomeRockData();
};