#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Perf_TerrainPhysicsPerformanceIntegrator.generated.h"

// Forward declarations
class UCore_TerrainDeformationSystem;
class UCore_PhysicsManager;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainPhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Performance")
    float DeformationProcessingTime;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Performance")
    float MaterialDetectionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Performance")
    float CollisionCalculationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Performance")
    float HeightmapUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Performance")
    int32 ActiveDeformations;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Performance")
    int32 QueuedDeformations;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Performance")
    bool bPerformanceThresholdExceeded;

    FPerf_TerrainPhysicsMetrics()
        : DeformationProcessingTime(0.0f)
        , MaterialDetectionTime(0.0f)
        , CollisionCalculationTime(0.0f)
        , HeightmapUpdateTime(0.0f)
        , ActiveDeformations(0)
        , QueuedDeformations(0)
        , MemoryUsageMB(0.0f)
        , AverageFrameTime(0.0f)
        , bPerformanceThresholdExceeded(false)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxDeformationProcessingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxConcurrentDeformations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float DeformationCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAdaptiveQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PerformanceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableSpatialPartitioning;

    FPerf_TerrainOptimizationSettings()
        : MaxDeformationProcessingTime(2.0f)
        , MaxConcurrentDeformations(10)
        , DeformationCullingDistance(5000.0f)
        , bEnableAdaptiveQuality(true)
        , PerformanceThreshold(16.67f)
        , bEnableSpatialPartitioning(true)
    {
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_TerrainPhysicsPerformanceIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_TerrainPhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_TerrainPhysicsMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceCounters();

    // Terrain system integration
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterTerrainDeformationSystem(UCore_TerrainDeformationSystem* TerrainSystem);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterPhysicsManager(UCore_PhysicsManager* PhysicsManager);

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetOptimizationSettings(const FPerf_TerrainOptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    FPerf_TerrainOptimizationSettings GetOptimizationSettings() const;

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void AnalyzeDeformationPerformance();

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    bool IsPerformanceOptimal() const;

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugTerrainPerformance();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPerformanceMetrics();

protected:
    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FPerf_TerrainPhysicsMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_TerrainOptimizationSettings OptimizationSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float MonitoringStartTime;

    // System references
    UPROPERTY()
    UCore_TerrainDeformationSystem* TerrainDeformationSystem;

    UPROPERTY()
    UCore_PhysicsManager* PhysicsManager;

    // Performance data
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    TArray<float> DeformationTimeHistory;

    UPROPERTY()
    int32 PerformanceFrameCounter;

    UPROPERTY()
    float TotalDeformationTime;

    UPROPERTY()
    float TotalFrameTime;

    // Timer handles
    FTimerHandle PerformanceAnalysisTimer;
    FTimerHandle OptimizationTimer;

private:
    // Internal performance tracking
    void UpdatePerformanceMetrics(float DeltaTime);
    void AnalyzeFramePerformance();
    void ApplyPerformanceOptimizations();
    void CullDistantDeformations();
    void OptimizeSpatialPartitioning();
    float CalculateAverageFrameTime() const;
    float CalculateMemoryUsage() const;
    void WritePerformanceLog() const;
};