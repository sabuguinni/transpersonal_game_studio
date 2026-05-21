#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "EnvArt_VegetationSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_VegetationType : uint8
{
    Fern            UMETA(DisplayName = "Prehistoric Fern"),
    ConiferTree     UMETA(DisplayName = "Conifer Tree"),
    CycadPlant      UMETA(DisplayName = "Cycad Plant"),
    Moss            UMETA(DisplayName = "Moss Patch"),
    DeadLog         UMETA(DisplayName = "Fallen Log"),
    Mushroom        UMETA(DisplayName = "Mushroom Cluster")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_VegetationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EEnvArt_VegetationType VegetationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> VegetationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TSoftObjectPtr<UMaterialInterface> VegetationMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector ScaleRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float SpawnDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float WindIntensity;

    FEnvArt_VegetationData()
    {
        VegetationType = EEnvArt_VegetationType::Fern;
        ScaleRange = FVector(0.8f, 1.2f, 1.0f);
        SpawnDensity = 1.0f;
        WindIntensity = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_VegetationSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_VegetationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* FernInstances;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* TreeInstances;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* CycadInstances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Config")
    TArray<FEnvArt_VegetationData> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Config")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Config")
    int32 MaxInstancesPerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector WindDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableCulling;

public:
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnVegetationInRadius(const FVector& Center, float Radius, EEnvArt_VegetationType Type, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void ClearAllVegetation();

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void UpdateWindParameters(float NewStrength, const FVector& NewDirection);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    int32 GetTotalInstanceCount() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void GenerateRandomVegetation();

private:
    void InitializeVegetationData();
    void UpdateWindEffect(float DeltaTime);
    FVector GetRandomSpawnLocation(const FVector& Center, float Radius) const;
    FRotator GetRandomRotation() const;
    FVector GetRandomScale(const FVector& ScaleRange) const;
    UInstancedStaticMeshComponent* GetInstancedMeshForType(EEnvArt_VegetationType Type) const;

    float WindTimer;
    FVector CurrentWindOffset;
};