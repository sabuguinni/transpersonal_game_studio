#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "../Performance/Perf_PhysicsArchitectIntegrator.h"
#include "../SharedTypes.h"
#include "World_MilestoneTerrainOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainOptimizationSettings
{
    GENERATED_BODY()

    // Performance-aware terrain settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Optimization")
    int32 MaxTerrainLODLevels = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Optimization") 
    float TerrainCullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Optimization")
    int32 MaxVisibleTerrainChunks = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Optimization")
    bool bEnableTerrainStreaming = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Optimization")
    float TerrainStreamingRadius = 3000.0f;

    FWorld_TerrainOptimizationSettings()
    {
        MaxTerrainLODLevels = 4;
        TerrainCullingDistance = 5000.0f;
        MaxVisibleTerrainChunks = 16;
        bEnableTerrainStreaming = true;
        TerrainStreamingRadius = 3000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomePerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    int32 ActiveBiomeActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    float BiomeRenderTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    int32 VisibleVegetationCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    float BiomeMemoryUsage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    bool bBiomeOptimizationActive = false;

    FWorld_BiomePerformanceMetrics()
    {
        ActiveBiomeActors = 0;
        BiomeRenderTime = 0.0f;
        VisibleVegetationCount = 0;
        BiomeMemoryUsage = 0.0f;
        bBiomeOptimizationActive = false;
    }
};

/**
 * World Milestone Terrain Optimizer - Specialized terrain optimization for Milestone 1 "WALK AROUND"
 * Integrates with Performance Optimizer's physics integration system to ensure terrain generation
 * maintains 60fps PC / 30fps console targets while providing diverse biome experiences.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_MilestoneTerrainOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_MilestoneTerrainOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Performance Integration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Integration")
    TObjectPtr<UPerf_PhysicsArchitectIntegrator> PerformanceIntegrator;

    // Terrain Optimization Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    FWorld_TerrainOptimizationSettings TerrainSettings;

    // Current Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FWorld_BiomePerformanceMetrics CurrentMetrics;

    // Terrain Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain Management")
    TArray<TObjectPtr<ALandscape>> ManagedLandscapes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain Management")
    TArray<TObjectPtr<AActor>> BiomeMarkerActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain Management")
    TArray<TObjectPtr<AActor>> VegetationActors;

public:
    // Milestone 1 Terrain Optimization
    UFUNCTION(BlueprintCallable, Category = "Milestone Terrain")
    void OptimizeTerrainForMilestone();

    UFUNCTION(BlueprintCallable, Category = "Milestone Terrain")
    void CreateMilestoneTerrainLayout();

    UFUNCTION(BlueprintCallable, Category = "Milestone Terrain")
    void OptimizeBiomeTransitions();

    // Performance Integration
    UFUNCTION(BlueprintCallable, Category = "Performance Integration")
    void IntegrateWithPerformanceSystem();

    UFUNCTION(BlueprintCallable, Category = "Performance Integration")
    void UpdateTerrainPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Integration")
    bool IsTerrainPerformanceOptimal() const;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void CreateForestBiome(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void CreatePlainsBiome(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void CreateRockyBiome(const FVector& Location, float Radius);

    // Vegetation Optimization
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void OptimizeVegetationLOD();

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void CullDistantVegetation();

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void UpdateVegetationVisibility();

    // Water System Integration
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateOptimizedWaterFeatures();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void OptimizeWaterRendering();

    // Debug and Validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateTerrainOptimization();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugTerrainPerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ResetTerrainOptimization();

protected:
    // Internal optimization methods
    void UpdateTerrainLOD();
    void CullDistantTerrain();
    void OptimizeTerrainMemory();
    void UpdateBiomeMetrics();
    void ValidatePerformanceTargets();

private:
    // Performance tracking
    float LastOptimizationTime;
    float TerrainOptimizationInterval;
    bool bOptimizationEnabled;
    int32 CurrentOptimizationLevel;
};