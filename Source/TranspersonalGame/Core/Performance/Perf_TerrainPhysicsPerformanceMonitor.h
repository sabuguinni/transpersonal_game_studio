#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "Perf_TerrainPhysicsPerformanceMonitor.generated.h"

DECLARE_STATS_GROUP(TEXT("Terrain Physics Performance"), STATGROUP_TerrainPhysics, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Terrain Deformation"), STAT_TerrainDeformation, STATGROUP_TerrainPhysics);
DECLARE_CYCLE_STAT(TEXT("Physics Collision Update"), STAT_PhysicsCollisionUpdate, STATGROUP_TerrainPhysics);
DECLARE_CYCLE_STAT(TEXT("Terrain Mesh Rebuild"), STAT_TerrainMeshRebuild, STATGROUP_TerrainPhysics);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainPhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DeformationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CollisionUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MeshRebuildTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveDeformationPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsBodyCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FPerf_TerrainPhysicsMetrics()
        : DeformationTime(0.0f)
        , CollisionUpdateTime(0.0f)
        , MeshRebuildTime(0.0f)
        , ActiveDeformationPoints(0)
        , PhysicsBodyCount(0)
        , MemoryUsageMB(0.0f)
    {
    }
};

UENUM(BlueprintType)
enum class EPerf_TerrainPerformanceLevel : uint8
{
    Ultra UMETA(DisplayName = "Ultra"),
    High UMETA(DisplayName = "High"),
    Medium UMETA(DisplayName = "Medium"),
    Low UMETA(DisplayName = "Low"),
    Minimal UMETA(DisplayName = "Minimal")
};

/**
 * Monitors performance of terrain physics integration and deformation systems
 * Provides real-time metrics and automatic quality scaling for terrain operations
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_TerrainPhysicsPerformanceMonitor : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_TerrainPhysicsPerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartTerrainDeformationProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EndTerrainDeformationProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPhysicsCollisionProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EndPhysicsCollisionProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMeshRebuildProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EndMeshRebuildProfiling();

    // Metrics access
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_TerrainPhysicsMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetAverageFrameTime() const { return AverageFrameTime; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsPerformanceAcceptable() const { return bPerformanceAcceptable; }

    // Quality scaling
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTerrainPerformanceLevel(EPerf_TerrainPerformanceLevel Level);

    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerf_TerrainPerformanceLevel GetCurrentPerformanceLevel() const { return CurrentPerformanceLevel; }

    // Automatic optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoOptimization(bool bEnable) { bAutoOptimizationEnabled = bEnable; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void TriggerPerformanceOptimization();

    // Debug and testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void RunPerformanceBenchmark();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogCurrentPerformanceState();

protected:
    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_TerrainPhysicsMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceAcceptable;

    // Quality settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    EPerf_TerrainPerformanceLevel CurrentPerformanceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TargetFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxAcceptableFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoOptimizationEnabled;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float DeformationTimeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float CollisionUpdateThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MeshRebuildThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxDeformationPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxPhysicsBodies;

private:
    // Timing variables
    double DeformationStartTime;
    double CollisionStartTime;
    double MeshRebuildStartTime;

    // Frame time tracking
    TArray<float> FrameTimeHistory;
    int32 FrameTimeHistoryIndex;
    static const int32 FrameTimeHistorySize = 60;

    // Optimization state
    float LastOptimizationTime;
    float OptimizationCooldown;

    // Internal methods
    void UpdateFrameTimeTracking(float DeltaTime);
    void CheckPerformanceThresholds();
    void ApplyPerformanceLevel(EPerf_TerrainPerformanceLevel Level);
    void OptimizeTerrainPerformance();
    void CollectMemoryMetrics();
    void UpdatePhysicsBodyCount();
};