#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "World_ProceduralBiomeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurAssetPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 DinosaurDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 TerrainFeatureDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityLevel;

    FWorld_BiomeConfiguration()
    {
        BiomeCenter = FVector::ZeroVector;
        BiomeRadius = 5000.0f;
        BiomeType = EBiomeType::Savana;
        DinosaurDensity = 5;
        TerrainFeatureDensity = 20;
        TemperatureRange = 25.0f;
        HumidityLevel = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    ETerrainFeatureType FeatureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString AssetPath;

    FWorld_TerrainFeature()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        FeatureType = ETerrainFeatureType::Rock;
        AssetPath = TEXT("");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_ProceduralBiomeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_ProceduralBiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void PopulateBiome(const FWorld_BiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void ClearBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    // Dinosaur Population
    UFUNCTION(BlueprintCallable, Category = "Dinosaur System")
    void SpawnDinosaursInBiome(const FWorld_BiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur System")
    AActor* SpawnDinosaurAtLocation(const FString& AssetPath, const FVector& Location, const FRotator& Rotation);

    // Terrain Features
    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void GenerateTerrainFeatures(const FWorld_BiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    AActor* SpawnTerrainFeature(const FWorld_TerrainFeature& Feature);

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomePerformance(const FVector& PlayerLocation, float CullingDistance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODDistances(float StaticMeshLOD, float SkeletalMeshLOD);

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyBiomeEnvironmentalEffects(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateWeatherSystem(float DeltaTime);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TArray<FWorld_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StaticMeshCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float SkeletalMeshCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float EnvironmentalUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WeatherTransitionSpeed;

private:
    // Internal state
    TArray<AActor*> SpawnedDinosaurs;
    TArray<AActor*> SpawnedTerrainFeatures;
    float LastEnvironmentalUpdate;
    
    // Helper functions
    FVector GenerateRandomLocationInBiome(const FVector& BiomeCenter, float BiomeRadius) const;
    FRotator GenerateRandomRotation() const;
    FVector GenerateRandomScale(float MinScale = 0.5f, float MaxScale = 2.0f) const;
    bool IsLocationValid(const FVector& Location) const;
    void CleanupInvalidActors();
};