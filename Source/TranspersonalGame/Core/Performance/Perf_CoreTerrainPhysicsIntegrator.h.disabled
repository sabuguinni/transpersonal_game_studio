#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "HAL/Platform.h"
#include "Misc/DateTime.h"
#include "SharedTypes.h"
#include "Perf_CoreTerrainPhysicsIntegrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainPhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Physics Metrics")
    float TerrainDeformationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Physics Metrics")
    float MaterialDetectionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Physics Metrics")
    float CollisionCalculationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Physics Metrics")
    float PhysicsSimulationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Physics Metrics")
    int32 ActiveTerrainColliders;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Physics Metrics")
    int32 TerrainMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Physics Metrics")
    float TerrainLODUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Physics Metrics")
    float TerrainStreamingTime;

    FPerf_TerrainPhysicsMetrics()
    {
        TerrainDeformationTime = 0.0f;
        MaterialDetectionTime = 0.0f;
        CollisionCalculationTime = 0.0f;
        PhysicsSimulationTime = 0.0f;
        ActiveTerrainColliders = 0;
        TerrainMemoryUsageMB = 0;
        TerrainLODUpdateTime = 0.0f;
        TerrainStreamingTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainPhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics Optimization")
    float MaxTerrainDeformationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics Optimization")
    float MaxMaterialDetectionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics Optimization")
    float MaxCollisionCalculationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics Optimization")
    int32 MaxActiveTerrainColliders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics Optimization")
    int32 MaxTerrainMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics Optimization")
    bool bEnableTerrainLODOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics Optimization")
    bool bEnableTerrainStreamingOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics Optimization")
    float TerrainOptimizationRadius;

    FPerf_TerrainPhysicsOptimizationSettings()
    {
        MaxTerrainDeformationTime = 2.0f;
        MaxMaterialDetectionTime = 1.0f;
        MaxCollisionCalculationTime = 3.0f;
        MaxActiveTerrainColliders = 500;
        MaxTerrainMemoryUsageMB = 512;
        bEnableTerrainLODOptimization = true;
        bEnableTerrainStreamingOptimization = true;
        TerrainOptimizationRadius = 5000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_CoreTerrainPhysicsIntegrator : public AActor
{
    GENERATED_BODY()

public:
    APerf_CoreTerrainPhysicsIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics Performance")
    FPerf_TerrainPhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Physics Performance")
    FPerf_TerrainPhysicsMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    bool bEnablePerformanceMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float MonitoringUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    bool bEnableDetailedLogging;

private:
    float LastMonitoringUpdate;
    TArray<float> TerrainDeformationHistory;
    TArray<float> MaterialDetectionHistory;
    TArray<float> CollisionCalculationHistory;
    TArray<int32> TerrainMemoryHistory;

public:
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Performance")
    void StartTerrainPhysicsMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Performance")
    void StopTerrainPhysicsMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Performance")
    FPerf_TerrainPhysicsMetrics GetCurrentTerrainPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Performance")
    void OptimizeTerrainPhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Performance")
    void UpdateTerrainPhysicsLOD(float PlayerDistance);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Performance")
    void ManageTerrainPhysicsStreaming(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Performance")
    bool IsTerrainPhysicsPerformanceOptimal() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Performance")
    void ResetTerrainPhysicsOptimization();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Performance")
    float GetAverageTerrainDeformationTime() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics Performance")
    float GetAverageTerrainMemoryUsage() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance Testing")
    void RunTerrainPhysicsPerformanceTest();

private:
    void UpdateTerrainPhysicsMetrics();
    void ApplyTerrainPhysicsOptimizations();
    void LogTerrainPhysicsPerformance();
    void CheckTerrainPhysicsThresholds();
    void OptimizeTerrainColliders();
    void OptimizeTerrainMaterials();
    void OptimizeTerrainDeformation();
    void ManageTerrainMemory();
};