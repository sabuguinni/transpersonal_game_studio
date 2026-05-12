#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Perf_TerrainDeformationOptimizer.generated.h"

// Forward declarations
class UCore_TerrainDeformationComponent;
class UCore_TerrainDeformationManager;

/**
 * Terrain Deformation Performance Metrics
 * Tracks performance data for terrain deformation operations
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainDeformationMetrics
{
    GENERATED_BODY()

    // Performance timing metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DeformationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance") 
    float LandscapeUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float ComponentUpdateTime;

    // Resource usage metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveDeformations;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PendingDeformations;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    // Quality metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LODLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float UpdateRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsOptimized;

    FPerf_TerrainDeformationMetrics()
        : DeformationTime(0.0f)
        , LandscapeUpdateTime(0.0f)
        , ComponentUpdateTime(0.0f)
        , ActiveDeformations(0)
        , PendingDeformations(0)
        , MemoryUsageMB(0.0f)
        , LODLevel(0)
        , UpdateRadius(1000.0f)
        , bIsOptimized(false)
    {}
};

/**
 * Terrain Deformation LOD Settings
 * Controls level-of-detail for terrain deformation based on distance and performance
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainDeformationLOD
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DeformationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float IntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableDeformation;

    FPerf_TerrainDeformationLOD()
        : Distance(1000.0f)
        , UpdateFrequency(60.0f)
        , DeformationRadius(500.0f)
        , IntensityMultiplier(1.0f)
        , bEnableDeformation(true)
    {}
};

/**
 * Terrain Deformation Performance Component
 * Monitors and optimizes terrain deformation performance for individual actors
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_TerrainDeformationOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_TerrainDeformationOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_TerrainDeformationMetrics GetPerformanceMetrics() const;

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODLevel(float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCurrentLODLevel() const;

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeDeformationRadius(float TargetFrameTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantDeformations(float MaxDistance);

    // Debug and profiling
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void RunPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void EnableDebugVisualization(bool bEnabled);

protected:
    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxDeformationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveDeformations;

    // LOD settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<FPerf_TerrainDeformationLOD> LODLevels;

    UPROPERTY(BlueprintReadOnly, Category = "LOD")
    int32 CurrentLODLevel;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_TerrainDeformationMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring;

    // Component references
    UPROPERTY()
    UCore_TerrainDeformationComponent* TerrainDeformationComponent;

private:
    // Performance timing
    float LastUpdateTime;
    float AccumulatedFrameTime;
    int32 FrameCount;

    // Optimization state
    bool bNeedsOptimization;
    float LastOptimizationTime;

    // Helper methods
    void UpdatePerformanceMetrics(float DeltaTime);
    void ApplyLODOptimizations();
    void CheckPerformanceThresholds();
    float CalculateDistanceToPlayer() const;
};

/**
 * Terrain Deformation Performance Manager
 * Global subsystem for managing terrain deformation performance across the world
 */
UCLASS()
class TRANSPERSONALGAME_API UPerf_TerrainDeformationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Component registration
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterOptimizer(UPerf_TerrainDeformationOptimizer* Optimizer);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterOptimizer(UPerf_TerrainDeformationOptimizer* Optimizer);

    // Global performance management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateGlobalPerformance(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_TerrainDeformationMetrics GetGlobalMetrics() const;

    // Performance controls
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGlobalOptimizationLevel(int32 Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceMode(bool bEnabled);

    // Debugging
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void RunGlobalPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPerformanceReport();

protected:
    // Registered optimizers
    UPROPERTY()
    TArray<UPerf_TerrainDeformationOptimizer*> RegisteredOptimizers;

    // Global performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GlobalTargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxGlobalDeformations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableGlobalOptimization;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_TerrainDeformationMetrics GlobalMetrics;

private:
    // Performance management
    void OptimizeGlobalPerformance();
    void UpdateGlobalLOD();
    void CullExcessiveDeformations();

    // Timing
    float LastGlobalUpdate;
    float GlobalUpdateInterval;
};