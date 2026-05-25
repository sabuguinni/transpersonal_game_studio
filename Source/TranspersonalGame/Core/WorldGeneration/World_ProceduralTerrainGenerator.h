#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "World_ProceduralTerrainGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString FeatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TSoftObjectPtr<UStaticMesh> MeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EBiomeType BiomeType;

    FWorld_TerrainFeature()
    {
        FeatureName = TEXT("DefaultFeature");
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        BiomeType = EBiomeType::Savana;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> TerrainMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 FeatureDensity;

    FWorld_BiomeConfiguration()
    {
        BiomeType = EBiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        FeatureDensity = 50;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_ProceduralTerrainGenerator : public AActor
{
    GENERATED_BODY()

public:
    AWorld_ProceduralTerrainGenerator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TArray<FWorld_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TArray<FWorld_TerrainFeature> GeneratedFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxFeaturesPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinFeatureDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 RandomSeed;

public:
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ClearGeneratedFeatures();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    FVector GetRandomLocationInBiome(const FWorld_BiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    bool IsLocationValid(const FVector& Location, const FWorld_BiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain Generation")
    void EditorGenerateAllBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain Generation")
    void EditorClearFeatures();

protected:
    void InitializeBiomeConfigurations();
    void SpawnTerrainFeature(const FWorld_TerrainFeature& Feature);
    FWorld_TerrainFeature CreateRandomFeature(const FWorld_BiomeConfiguration& BiomeConfig);
    void LoadBiomeAssets();

private:
    TArray<AActor*> SpawnedActors;
    bool bIsGenerating;
};