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
    Fern            UMETA(DisplayName = "Fern"),
    Cycad           UMETA(DisplayName = "Cycad"),
    Conifer         UMETA(DisplayName = "Conifer"),
    Moss            UMETA(DisplayName = "Moss"),
    Lichen          UMETA(DisplayName = "Lichen"),
    Mushroom        UMETA(DisplayName = "Mushroom"),
    Grass           UMETA(DisplayName = "Grass"),
    Vine            UMETA(DisplayName = "Vine")
};

USTRUCT(BlueprintType)
struct FEnvArt_VegetationSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EEnvArt_VegetationType VegetationType = EEnvArt_VegetationType::Fern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    UStaticMesh* VegetationMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    UMaterialInterface* VegetationMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector MinScale = FVector(0.8f, 0.8f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector MaxScale = FVector(1.2f, 1.2f, 1.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float SpawnDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float MinDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bAlignToSurface = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bRandomRotation = true;
};

USTRUCT(BlueprintType)
struct FEnvArt_BiomeVegetation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Forest");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FEnvArt_VegetationSpec> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float OverallDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 10000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_VegetationSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_VegetationSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FEnvArt_BiomeVegetation> BiomeConfigurations;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Instanced Meshes")
    TMap<EEnvArt_VegetationType, UInstancedStaticMeshComponent*> VegetationInstances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxInstancesPerType = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float GenerationRadius = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bUseHeightmapSampling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinSpawnHeight = -1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MaxSpawnHeight = 2000.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void GenerateVegetationForBiome(const FEnvArt_BiomeVegetation& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void ClearAllVegetation();

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void RegenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Vegetation", CallInEditor)
    void PreviewVegetationPlacement();

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    int32 GetTotalVegetationCount() const;

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SetBiomeDensity(const FString& BiomeName, float NewDensity);

private:
    void InitializeVegetationInstances();
    void InitializeDefaultBiomes();
    FVector GetRandomPointInBiome(const FEnvArt_BiomeVegetation& Biome) const;
    bool IsValidSpawnLocation(const FVector& Location, const FEnvArt_VegetationSpec& Spec) const;
    FTransform GenerateVegetationTransform(const FVector& Location, const FEnvArt_VegetationSpec& Spec) const;
};