#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SharedTypes.h"
#include "Perf_TerrainPhysicsPerformanceIntegrator.generated.h"

UENUM(BlueprintType)
enum class EPerf_TerrainPhysicsOptimizationLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Minimal     UMETA(DisplayName = "Minimal Quality")
};

USTRUCT(BlueprintType)
struct FPerf_TerrainPhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CollisionCheckTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TerrainCollisionQueries;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    FPerf_TerrainPhysicsMetrics()
        : PhysicsUpdateTime(0.0f)
        , CollisionCheckTime(0.0f)
        , ActivePhysicsActors(0)
        , TerrainCollisionQueries(0)
        , MemoryUsageMB(0.0f)
        , FrameTime(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct FPerf_TerrainPhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxPhysicsUpdateDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxActivePhysicsActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsLODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsLODDistance2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsLODDistance3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAdaptivePhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float TargetFrameTime;

    FPerf_TerrainPhysicsOptimizationSettings()
        : MaxPhysicsUpdateDistance(5000.0f)
        , MaxActivePhysicsActors(500)
        , PhysicsLODDistance1(1000.0f)
        , PhysicsLODDistance2(2500.0f)
        , PhysicsLODDistance3(5000.0f)
        , bEnableAdaptivePhysics(true)
        , TargetFrameTime(16.67f)
    {}
};

/**
 * Terrain Physics Performance Integrator
 * Optimizes terrain physics performance by monitoring and adapting physics complexity
 * Integrates with Core_TerrainPhysicsEnhancer for comprehensive performance management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_TerrainPhysicsPerformanceIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_TerrainPhysicsPerformanceIntegrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_TerrainPhysicsMetrics GetCurrentMetrics() const;

    // Optimization control
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetOptimizationLevel(EPerf_TerrainPhysicsOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    EPerf_TerrainPhysicsOptimizationLevel GetCurrentOptimizationLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ApplyOptimizationSettings(const FPerf_TerrainPhysicsOptimizationSettings& Settings);

    // Adaptive optimization
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void EnableAdaptiveOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void UpdateAdaptiveOptimization();

    // Integration with terrain physics
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateWithTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void OptimizeTerrainPhysicsActors();

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void AnalyzePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void GeneratePerformanceReport();

protected:
    // Performance monitoring
    void UpdatePerformanceMetrics();
    void MonitorPhysicsPerformance();
    void CheckFrameTimeThresholds();

    // Optimization implementation
    void ApplyUltraOptimization();
    void ApplyHighOptimization();
    void ApplyMediumOptimization();
    void ApplyLowOptimization();
    void ApplyMinimalOptimization();

    // Physics integration
    void FindTerrainPhysicsActors();
    void OptimizePhysicsActorsByDistance();
    void UpdatePhysicsLOD();

private:
    UPROPERTY()
    FPerf_TerrainPhysicsMetrics CurrentMetrics;

    UPROPERTY()
    EPerf_TerrainPhysicsOptimizationLevel CurrentOptimizationLevel;

    UPROPERTY()
    FPerf_TerrainPhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY()
    bool bIsMonitoring;

    UPROPERTY()
    bool bAdaptiveOptimizationEnabled;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> TerrainPhysicsActors;

    UPROPERTY()
    float LastFrameTime;

    UPROPERTY()
    float AverageFrameTime;

    UPROPERTY()
    int32 FrameCounter;

    // Performance thresholds
    static constexpr float PERFORMANCE_THRESHOLD_CRITICAL = 33.33f; // 30fps
    static constexpr float PERFORMANCE_THRESHOLD_WARNING = 20.0f;   // 50fps
    static constexpr float PERFORMANCE_THRESHOLD_OPTIMAL = 16.67f;  // 60fps

    // Timer handles
    FTimerHandle PerformanceMonitorTimer;
    FTimerHandle AdaptiveOptimizationTimer;
};