#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Foliage/Public/FoliageType.h"
#include "EnvArt_FoliageSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_FoliageType : uint8
{
    None = 0,
    Ferns,
    Moss,
    Grass,
    Flowers,
    Vines,
    Mushrooms,
    Fallen_Logs,
    Rocks,
    Debris
};

USTRUCT(BlueprintType)
struct FEnvArt_FoliageCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    EEnvArt_FoliageType FoliageType = EEnvArt_FoliageType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    TSoftObjectPtr<UStaticMesh> MeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    TSoftObjectPtr<UMaterialInterface> MaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    FVector ScaleRange = FVector(0.8f, 1.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float ClusterRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    int32 MaxInstancesPerCluster = 50;

    FEnvArt_FoliageCluster()
    {
        FoliageType = EEnvArt_FoliageType::None;
        ScaleRange = FVector(0.8f, 1.2f, 1.0f);
        Density = 1.0f;
        ClusterRadius = 500.0f;
        MaxInstancesPerCluster = 50;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_FoliageSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_FoliageSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage System")
    TArray<FEnvArt_FoliageCluster> FoliageClusters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage System")
    float UpdateRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage System")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage System")
    float LODDistance2 = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage System")
    bool bEnableWind = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage System")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage System")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Foliage System")
    void GenerateFoliageInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Foliage System")
    void ClearFoliageInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Foliage System")
    void UpdateFoliageLOD();

    UFUNCTION(BlueprintCallable, Category = "Foliage System")
    void SetWindParameters(float Strength, const FVector& Direction);

protected:
    UPROPERTY()
    TMap<EEnvArt_FoliageType, UInstancedStaticMeshComponent*> InstancedMeshComponents;

    UPROPERTY()
    APawn* PlayerPawn;

    void InitializeFoliageComponents();
    void SpawnFoliageCluster(const FEnvArt_FoliageCluster& Cluster, const FVector& Center);
    bool IsValidSpawnLocation(const FVector& Location, EEnvArt_FoliageType FoliageType);
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius);
    float GetTerrainHeightAtLocation(const FVector& Location);
};