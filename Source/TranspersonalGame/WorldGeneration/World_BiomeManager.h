#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
#include "World_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxProps = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 CurrentProps = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainRoughness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterPresence = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomePerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TotalActors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 VisibleActors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EWorld_PerformanceLevel CurrentPerformanceLevel = EWorld_PerformanceLevel::High;
};

/**
 * Advanced Biome Manager for procedural world generation
 * Manages distinct biome zones with performance optimization
 * Integrates with Physics Integration Optimizer for adaptive quality
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeTransitionDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    bool bEnableAutomaticGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    int32 MaxTotalActors = 20000;

    // Performance Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FWorld_BiomePerformanceMetrics PerformanceMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDistanceCulling = true;

    // Runtime Data
    UPROPERTY()
    float LastPerformanceUpdate = 0.0f;

    UPROPERTY()
    TArray<AActor*> SpawnedActors;

public:
    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void GenerateBiomeContent();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeZone* GetBiomeZoneAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void AddBiomeZone(const FWorld_BiomeZone& NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void RemoveBiomeZone(int32 BiomeIndex);

    // Content Generation
    UFUNCTION(BlueprintCallable, Category = "Generation")
    void SpawnBiomeProps(int32 BiomeIndex, int32 PropCount);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void SpawnTerrainFeatures(const FWorld_BiomeZone& Biome);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void SpawnVegetation(const FWorld_BiomeZone& Biome);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void SpawnWaterFeatures(const FWorld_BiomeZone& Biome);

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantActors(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustBiomeQuality(EWorld_PerformanceLevel TargetLevel);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToBiome(const FVector& Location, int32 BiomeIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsLocationInBiome(const FVector& Location, int32 BiomeIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void CleanupExcessActors();

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void VisualizeAllBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ClearAllSpawnedActors();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogBiomeStatistics() const;

private:
    // Internal helper functions
    void CreateDefaultBiomes();
    AActor* SpawnPropForBiome(const FWorld_BiomeZone& Biome, const FVector& Location);
    void UpdateBiomeActorCounts();
    bool CanSpawnMoreActors() const;
};