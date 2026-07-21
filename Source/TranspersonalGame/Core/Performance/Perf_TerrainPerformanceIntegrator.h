#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Perf_TerrainPerformanceIntegrator.generated.h"

/**
 * Advanced terrain performance integration system that optimizes terrain physics
 * and rendering performance for the prehistoric world. Integrates with Core_TerrainPhysicsSystem
 * to provide comprehensive terrain performance management.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_TerrainPerformanceIntegrator : public AActor
{
    GENERATED_BODY()

public:
    APerf_TerrainPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    // Terrain Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Performance")
    float TerrainLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Performance")
    int32 MaxTerrainChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Performance")
    float TerrainCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Performance")
    bool bEnableTerrainLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Performance")
    bool bEnableTerrainCulling;

    // Physics Integration Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    float PhysicsUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    int32 MaxPhysicsObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    bool bEnablePhysicsLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    float PhysicsLODDistance;

    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CurrentTerrainFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 ActiveTerrainChunks;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 ActivePhysicsObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float TerrainMemoryUsage;

    // Biome-Specific Performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Performance")
    TMap<EBiomeType, float> BiomeLODMultipliers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Performance")
    TMap<EBiomeType, int32> BiomeMaxObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Performance")
    bool bEnableBiomeSpecificOptimization;

public:
    // Performance Control Functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Performance")
    void OptimizeTerrainPerformance();

    UFUNCTION(BlueprintCallable, Category = "Terrain Performance")
    void SetTerrainLODLevel(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Terrain Performance")
    void UpdateTerrainCulling();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetPhysicsLODLevel(int32 LODLevel);

    // Performance Monitoring Functions
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    float GetTerrainPerformanceScore();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    bool IsPerformanceOptimal();

    // Biome Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Biome Performance")
    void OptimizeBiomePerformance(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Performance")
    void SetBiomeLODMultiplier(EBiomeType BiomeType, float Multiplier);

    UFUNCTION(BlueprintCallable, Category = "Biome Performance")
    float GetBiomePerformanceScore(EBiomeType BiomeType);

private:
    // Internal Performance Management
    void UpdatePerformanceMetrics();
    void ApplyTerrainOptimizations();
    void ApplyPhysicsOptimizations();
    void ApplyBiomeOptimizations();
    void MonitorFrameTime();
    void ManageMemoryUsage();

    // Performance Thresholds
    float TargetFrameTime;
    float MaxMemoryUsage;
    int32 MaxDrawCalls;

    // Timing
    float LastUpdateTime;
    float UpdateInterval;
    
    // Performance History
    TArray<float> FrameTimeHistory;
    TArray<float> MemoryUsageHistory;
    int32 MaxHistorySize;
};