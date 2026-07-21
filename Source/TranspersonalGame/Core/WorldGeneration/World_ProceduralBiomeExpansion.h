#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_ProceduralBiomeExpansion.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeExpansionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Expansion")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Expansion")
    float ExpansionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Expansion")
    int32 VegetationDensity = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Expansion")
    float TerrainVariation = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Expansion")
    bool bIncludeWaterFeatures = true;

    FWorld_BiomeExpansionConfig()
    {
        BiomeType = EBiomeType::Forest;
        ExpansionRadius = 2000.0f;
        VegetationDensity = 50;
        TerrainVariation = 100.0f;
        bIncludeWaterFeatures = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VegetationSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EVegetationType VegetationType = EVegetationType::Tree;

    FWorld_VegetationSpawnData()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        VegetationType = EVegetationType::Tree;
    }
};

/**
 * Procedural Biome Expansion System
 * Dynamically creates and expands biome areas with appropriate vegetation and terrain features
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_ProceduralBiomeExpansion : public AActor
{
    GENERATED_BODY()

public:
    AWorld_ProceduralBiomeExpansion();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TArray<FWorld_BiomeExpansionConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expansion Settings")
    float ExpansionUpdateInterval = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expansion Settings")
    int32 MaxVegetationPerUpdate = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Features")
    bool bGenerateRivers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Features")
    bool bGenerateLakes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Features")
    float WaterLevel = 50.0f;

private:
    UPROPERTY()
    TArray<AActor*> SpawnedVegetation;

    UPROPERTY()
    TArray<AActor*> SpawnedWaterFeatures;

    FTimerHandle ExpansionTimerHandle;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Expansion")
    void InitializeBiomeExpansion();

    UFUNCTION(BlueprintCallable, Category = "Biome Expansion")
    void ExpandBiome(EBiomeType BiomeType, FVector CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Expansion")
    void GenerateVegetationCluster(const FWorld_BiomeExpansionConfig& Config, FVector CenterLocation);

    UFUNCTION(BlueprintCallable, Category = "Water Features")
    void CreateRiverSystem(FVector StartLocation, FVector EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Water Features")
    void CreateLake(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void ModifyTerrainHeight(FVector Location, float Radius, float HeightDelta);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void ClearBiomeArea(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    TArray<FWorld_VegetationSpawnData> GenerateVegetationLayout(const FWorld_BiomeExpansionConfig& Config, FVector CenterLocation);

protected:
    UFUNCTION()
    void UpdateBiomeExpansion();

    UFUNCTION()
    FVector GetRandomLocationInRadius(FVector Center, float Radius);

    UFUNCTION()
    bool IsLocationSuitableForVegetation(FVector Location, EVegetationType VegetationType);

    UFUNCTION()
    AActor* SpawnVegetationActor(const FWorld_VegetationSpawnData& SpawnData);

    UFUNCTION()
    AActor* SpawnWaterPlane(FVector Location, FVector Scale);
};