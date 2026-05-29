#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "EnvArt_VegetationManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_VegetationType : uint8
{
    Fern,
    Cycad,
    Conifer,
    Moss,
    Grass,
    Vine,
    Mushroom,
    DeadTree,
    FallenLog
};

UENUM(BlueprintType)
enum class EEnvArt_BiomeType : uint8
{
    Savana,
    Forest,
    Desert,
    Swamp,
    Mountain
};

USTRUCT(BlueprintType)
struct FEnvArt_VegetationSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EEnvArt_VegetationType VegetationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> VegetationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float Density;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bRandomRotation;

    FEnvArt_VegetationSpawnData()
    {
        VegetationType = EEnvArt_VegetationType::Fern;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        Density = 1.0f;
        bRandomRotation = true;
    }
};

UCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_VegetationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_VegetationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Settings")
    EEnvArt_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Settings")
    TArray<FEnvArt_VegetationSpawnData> VegetationSpawnData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Settings")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Settings")
    int32 MaxVegetationCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Settings")
    bool bAutoSpawnOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Settings")
    float MinDistanceBetweenVegetation;

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnVegetationInRadius(const FVector& CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnVegetationAtLocation(const FVector& Location, EEnvArt_VegetationType Type);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void ClearAllVegetation();

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SetBiomeType(EEnvArt_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    TArray<AActor*> GetSpawnedVegetation() const;

private:
    UPROPERTY()
    TArray<AActor*> SpawnedVegetationActors;

    UFUNCTION()
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius) const;

    UFUNCTION()
    bool IsLocationValid(const FVector& Location) const;

    UFUNCTION()
    UStaticMesh* GetMeshForVegetationType(EEnvArt_VegetationType Type) const;
};