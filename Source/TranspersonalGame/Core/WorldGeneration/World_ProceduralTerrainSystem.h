#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "World_ProceduralTerrainSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    FString FeatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    TSoftObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    float LODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    bool bIsActive;

    FWorld_TerrainFeature()
    {
        FeatureName = TEXT("DefaultFeature");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        Mesh = nullptr;
        BiomeType = EBiomeType::Savana;
        LODDistance = 5000.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeTerrainConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    TArray<TSoftObjectPtr<UStaticMesh>> TerrainMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    int32 FeatureDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float MinFeatureScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float MaxFeatureScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float HeightVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    bool bEnableWaterFeatures;

    FWorld_BiomeTerrainConfig()
    {
        BiomeType = EBiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        BiomeRadius = 10000.0f;
        FeatureDensity = 50;
        MinFeatureScale = 0.5f;
        MaxFeatureScale = 2.0f;
        HeightVariation = 500.0f;
        bEnableWaterFeatures = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterBody
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    FString WaterBodyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    TArray<FVector> WaterPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    float WaterWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    float WaterDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    bool bIsRiver;

    FWorld_WaterBody()
    {
        WaterBodyName = TEXT("DefaultWater");
        WaterWidth = 500.0f;
        WaterDepth = 100.0f;
        FlowSpeed = 1.0f;
        bIsRiver = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_ProceduralTerrainSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_ProceduralTerrainSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core terrain generation
    UFUNCTION(BlueprintCallable, Category = "Procedural Terrain")
    void GenerateProceduralTerrain();

    UFUNCTION(BlueprintCallable, Category = "Procedural Terrain")
    void GenerateBiomeTerrain(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Procedural Terrain")
    void ClearExistingTerrain();

    // Feature placement
    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    void PlaceTerrainFeatures(const FWorld_BiomeTerrainConfig& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    AActor* SpawnTerrainFeature(const FWorld_TerrainFeature& FeatureData);

    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    void RemoveTerrainFeature(AActor* FeatureActor);

    // Water system
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateRiver(const FWorld_WaterBody& RiverData);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateLake(FVector Location, float Radius);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateFeatureLOD(AActor* FeatureActor, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float CalculateOptimalLODDistance(const FWorld_TerrainFeature& Feature);

    // Landscape integration
    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void CreateBaseLandscape();

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void ModifyLandscapeHeight(FVector Location, float Radius, float HeightDelta);

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    ALandscape* GetMainLandscape();

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiomeConfigs();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FWorld_BiomeTerrainConfig GetBiomeConfig(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    EBiomeType GetBiomeAtLocation(FVector Location);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<AActor*> GetTerrainFeaturesInRadius(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SaveTerrainConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void LoadTerrainConfiguration();

protected:
    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Terrain data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Data")
    TArray<FWorld_TerrainFeature> ActiveTerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Data")
    TArray<FWorld_BiomeTerrainConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Data")
    TArray<FWorld_WaterBody> WaterBodies;

    // Generation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    int32 TerrainSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    int32 MaxTerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    bool bAutoGenerateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    bool bEnableRealTimeOptimization;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleFeatures;

    // Runtime data
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float LastLODUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 CurrentActiveFeatures;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    ALandscape* MainLandscape;

private:
    // Internal helper functions
    void InitializeDefaultBiomes();
    FVector GenerateRandomLocationInBiome(const FWorld_BiomeTerrainConfig& BiomeConfig);
    float GeneratePerlinNoise(float X, float Y, float Scale);
    void CleanupInvalidFeatures();
    void UpdatePerformanceMetrics();
};