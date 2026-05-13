#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Perf_TerrainPhysicsPerformanceIntegrator.generated.h"

// Forward declarations
class UCore_TerrainPhysicsSystem;
class ALandscapeProxy;

/**
 * Performance metrics for terrain physics operations
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainPhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TerrainDeformationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float SurfaceMaterialDetectionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CollisionCalculationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveDeformationPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaterialCacheHits;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaterialCacheMisses;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FPerf_TerrainPhysicsMetrics()
    {
        TerrainDeformationTime = 0.0f;
        SurfaceMaterialDetectionTime = 0.0f;
        CollisionCalculationTime = 0.0f;
        ActiveDeformationPoints = 0;
        MaterialCacheHits = 0;
        MaterialCacheMisses = 0;
        MemoryUsageMB = 0.0f;
    }
};

/**
 * Optimization levels for terrain physics performance
 */
UENUM(BlueprintType)
enum class EPerf_TerrainOptimizationLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Minimal     UMETA(DisplayName = "Minimal Quality")
};

/**
 * Advanced Terrain Physics Performance Integration System
 * 
 * Monitors and optimizes terrain physics performance in real-time,
 * providing adaptive quality scaling and comprehensive metrics tracking.
 */
UCLASS(BlueprintType, Blueprintable, Category = "Performance|Terrain Physics")
class TRANSPERSONALGAME_API UPerf_TerrainPhysicsPerformanceIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_TerrainPhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === PERFORMANCE MONITORING ===

    /**
     * Get current terrain physics performance metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    FPerf_TerrainPhysicsMetrics GetTerrainPhysicsMetrics() const;

    /**
     * Start performance profiling for terrain physics systems
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void StartTerrainPhysicsProfiler();

    /**
     * Stop performance profiling and generate report
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void StopTerrainPhysicsProfiler();

    /**
     * Check if terrain physics performance is within acceptable limits
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    bool IsTerrainPhysicsPerformanceOptimal() const;

    // === OPTIMIZATION CONTROL ===

    /**
     * Set terrain physics optimization level
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void SetTerrainOptimizationLevel(EPerf_TerrainOptimizationLevel Level);

    /**
     * Get current terrain physics optimization level
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    EPerf_TerrainOptimizationLevel GetTerrainOptimizationLevel() const;

    /**
     * Enable or disable adaptive terrain physics optimization
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void SetAdaptiveTerrainOptimization(bool bEnabled);

    /**
     * Optimize terrain physics for current performance conditions
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void OptimizeTerrainPhysicsPerformance();

    // === DEFORMATION OPTIMIZATION ===

    /**
     * Set maximum number of active terrain deformation points
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void SetMaxDeformationPoints(int32 MaxPoints);

    /**
     * Set terrain deformation quality level
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void SetDeformationQuality(float QualityLevel);

    /**
     * Enable or disable terrain deformation culling based on distance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void SetDeformationCulling(bool bEnabled, float CullingDistance = 5000.0f);

    // === MATERIAL CACHE OPTIMIZATION ===

    /**
     * Optimize surface material cache for better performance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void OptimizeMaterialCache();

    /**
     * Clear and rebuild surface material cache
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void RebuildMaterialCache();

    /**
     * Set material cache size limit
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void SetMaterialCacheSize(int32 CacheSize);

    // === COLLISION OPTIMIZATION ===

    /**
     * Set terrain collision LOD level based on distance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void SetTerrainCollisionLOD(int32 LODLevel);

    /**
     * Enable or disable terrain collision optimization
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void SetCollisionOptimization(bool bEnabled);

    /**
     * Optimize terrain collision meshes for performance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Terrain Physics")
    void OptimizeTerrainCollision();

protected:
    // === PERFORMANCE METRICS ===

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_TerrainPhysicsMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_TerrainOptimizationLevel OptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bAdaptiveOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bProfilingEnabled;

    // === DEFORMATION SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation", meta = (AllowPrivateAccess = "true"))
    int32 MaxDeformationPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation", meta = (AllowPrivateAccess = "true"))
    float DeformationQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation", meta = (AllowPrivateAccess = "true"))
    bool bDeformationCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation", meta = (AllowPrivateAccess = "true"))
    float DeformationCullingDistance;

    // === MATERIAL CACHE SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Cache", meta = (AllowPrivateAccess = "true"))
    int32 MaterialCacheSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Cache", meta = (AllowPrivateAccess = "true"))
    bool bMaterialCacheOptimization;

    // === COLLISION SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    int32 TerrainCollisionLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    bool bCollisionOptimization;

    // === PERFORMANCE THRESHOLDS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (AllowPrivateAccess = "true"))
    float MaxDeformationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (AllowPrivateAccess = "true"))
    float MaxMaterialDetectionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (AllowPrivateAccess = "true"))
    float MaxCollisionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (AllowPrivateAccess = "true"))
    float MaxMemoryUsage;

    // === INTERNAL STATE ===

    UPROPERTY()
    TArray<TWeakObjectPtr<ALandscapeProxy>> TrackedLandscapes;

    UPROPERTY()
    TArray<TWeakObjectPtr<UCore_TerrainPhysicsSystem>> TrackedTerrainSystems;

    float LastProfilerUpdate;
    float ProfilerStartTime;
    bool bProfilerActive;

private:
    // Internal optimization methods
    void UpdatePerformanceMetrics();
    void ApplyOptimizationLevel();
    void CheckAdaptiveOptimization();
    void OptimizeBasedOnMetrics();
    void UpdateTerrainSystemReferences();
    void CollectTerrainPhysicsData();
    float CalculateOptimalQualityLevel() const;
    void ApplyDeformationOptimizations();
    void ApplyMaterialCacheOptimizations();
    void ApplyCollisionOptimizations();
};