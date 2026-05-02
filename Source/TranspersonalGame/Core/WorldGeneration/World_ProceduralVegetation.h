#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "World_ProceduralVegetation.generated.h"

UENUM(BlueprintType)
enum class EWorld_VegetationType : uint8
{
    SwampLog        UMETA(DisplayName = "Swamp Log"),
    ForestTree      UMETA(DisplayName = "Forest Tree"),
    SavannaTree     UMETA(DisplayName = "Savanna Tree"),
    TerrainRock     UMETA(DisplayName = "Terrain Rock"),
    WaterBody       UMETA(DisplayName = "Water Body"),
    Undergrowth     UMETA(DisplayName = "Undergrowth")
};

USTRUCT(BlueprintType)
struct FWorld_VegetationSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EWorld_VegetationType VegetationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EWorld_BiomeType BiomeType;

    FWorld_VegetationSpawnData()
    {
        VegetationType = EWorld_VegetationType::ForestTree;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        BiomeType = EWorld_BiomeType::Forest;
    }
};

USTRUCT(BlueprintType)
struct FWorld_BiomeVegetationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    int32 VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float MinScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float MaxScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    TArray<EWorld_VegetationType> AllowedVegetationTypes;

    FWorld_BiomeVegetationConfig()
    {
        BiomeType = EWorld_BiomeType::Forest;
        VegetationDensity = 10;
        MinScale = 0.8f;
        MaxScale = 1.5f;
        SpawnRadius = 1000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_ProceduralVegetation : public AActor
{
    GENERATED_BODY()

public:
    AWorld_ProceduralVegetation();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VegetationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EWorld_VegetationType VegetationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float GrowthStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bCanBeHarvested;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float HarvestYield;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SetVegetationType(EWorld_VegetationType NewType);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SetBiomeType(EWorld_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void UpdateMeshForType();

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    bool CanPlayerHarvest() const;

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    float HarvestResource();

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void ApplyBiomeSpecificProperties();

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    FVector GetRandomScaleForType() const;

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SetGrowthStage(float NewGrowthStage);

    UFUNCTION(BlueprintPure, Category = "Vegetation")
    bool IsFullyGrown() const;

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SimulateGrowth(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void OnPlayerInteract();

    UFUNCTION(BlueprintImplementableEvent, Category = "Vegetation")
    void OnHarvested();

    UFUNCTION(BlueprintImplementableEvent, Category = "Vegetation")
    void OnGrowthStageChanged(float NewStage);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorld_VegetationManager : public UObject
{
    GENERATED_BODY()

public:
    UWorld_VegetationManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Manager")
    TArray<FWorld_BiomeVegetationConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Manager")
    TArray<FWorld_VegetationSpawnData> SpawnedVegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Manager")
    int32 MaxVegetationActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Manager")
    float VegetationUpdateInterval;

    UFUNCTION(BlueprintCallable, Category = "Vegetation Manager")
    void GenerateVegetationForBiome(EWorld_BiomeType BiomeType, FVector CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Vegetation Manager")
    void ClearVegetationInArea(FVector CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Vegetation Manager")
    TArray<AWorld_ProceduralVegetation*> GetVegetationInRadius(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Vegetation Manager")
    void UpdateVegetationLOD(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Vegetation Manager")
    FWorld_BiomeVegetationConfig GetConfigForBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Vegetation Manager")
    void SetBiomeConfig(EWorld_BiomeType BiomeType, const FWorld_BiomeVegetationConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Vegetation Manager")
    bool SpawnVegetationActor(const FWorld_VegetationSpawnData& SpawnData);

    UFUNCTION(BlueprintCallable, Category = "Vegetation Manager")
    void OptimizeVegetationPerformance();

    UFUNCTION(BlueprintCallable, Category = "Vegetation Manager")
    int32 GetVegetationCount() const;

    UFUNCTION(BlueprintCallable, Category = "Vegetation Manager")
    void InitializeDefaultConfigs();
};