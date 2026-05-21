#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "World_TerrainManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 RockDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainRoughness;

    FWorld_BiomeData()
    {
        BiomeName = TEXT("Unknown");
        BiomeCenter = FVector::ZeroVector;
        BiomeRadius = 1000.0f;
        VegetationDensity = 50;
        RockDensity = 20;
        TerrainRoughness = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString FeatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString AssetPath;

    FWorld_TerrainFeature()
    {
        FeatureName = TEXT("Feature");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        AssetPath = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_TerrainManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_TerrainManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TArray<FWorld_BiomeData> BiomeConfigurations;

    // Terrain Features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TArray<FWorld_TerrainFeature> TerrainFeatures;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float MaxRenderDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1", ClampMax = "5"))
    int32 LODLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDistanceCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableFrustumCulling;

    // Generation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "1", ClampMax = "1000"))
    int32 MaxFeaturesPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float GenerationScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 RandomSeed;

    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrainForBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearTerrainFeatures();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FWorld_BiomeData GetBiomeByName(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FString GetBiomeAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODForDistance(float PlayerDistance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveFeatureCount();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePerformance();

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "World Generation")
    void InitializeDefaultBiomes();

    UFUNCTION(CallInEditor, Category = "World Generation")
    void GenerateTestTerrain();

    UFUNCTION(CallInEditor, Category = "Performance")
    void AnalyzePerformance();

private:
    // Internal state
    UPROPERTY()
    TArray<AActor*> SpawnedFeatures;

    UPROPERTY()
    float LastPerformanceCheck;

    UPROPERTY()
    bool bIsGenerating;

    // Internal methods
    void SpawnTerrainFeature(const FWorld_TerrainFeature& Feature);
    void UpdateFeatureLOD(AActor* Feature, float Distance);
    FVector GenerateRandomLocationInBiome(const FWorld_BiomeData& Biome);
    void CleanupInvalidFeatures();
    void InitializeBiomeDefaults();
};