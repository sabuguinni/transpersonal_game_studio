#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "Perf_TerrainPhysicsIntegrationOptimizer.generated.h"

// Forward declarations
class UCore_TerrainPhysicsSystem;
class UPhysicsComponent;

UENUM(BlueprintType)
enum class EPerf_TerrainPhysicsIntegrationLevel : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Integration"),
    Standard    UMETA(DisplayName = "Standard Integration"),
    Enhanced    UMETA(DisplayName = "Enhanced Integration"),
    Maximum     UMETA(DisplayName = "Maximum Integration")
};

UENUM(BlueprintType)
enum class EPerf_TerrainPhysicsOptimizationMode : uint8
{
    Conservative    UMETA(DisplayName = "Conservative Mode"),
    Balanced       UMETA(DisplayName = "Balanced Mode"),
    Aggressive     UMETA(DisplayName = "Aggressive Mode"),
    Emergency      UMETA(DisplayName = "Emergency Mode")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainPhysicsIntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float TerrainPhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CollisionDetectionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float RigidBodySimulationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float TerrainDeformationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 ActivePhysicsBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 TerrainCollisionQueries;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CPUUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float GPUUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float IntegrationEfficiency;

    FPerf_TerrainPhysicsIntegrationMetrics()
        : TerrainPhysicsFrameTime(0.0f)
        , CollisionDetectionTime(0.0f)
        , RigidBodySimulationTime(0.0f)
        , TerrainDeformationTime(0.0f)
        , ActivePhysicsBodies(0)
        , TerrainCollisionQueries(0)
        , MemoryUsageMB(0.0f)
        , CPUUsagePercent(0.0f)
        , GPUUsagePercent(0.0f)
        , IntegrationEfficiency(100.0f)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainPhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    EPerf_TerrainPhysicsIntegrationLevel IntegrationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    EPerf_TerrainPhysicsOptimizationMode OptimizationMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float MaxFrameTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    int32 MaxActivePhysicsBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float CollisionComplexityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    bool bEnableAdaptiveOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    bool bEnableTerrainLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    bool bEnablePhysicsLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float OptimizationUpdateInterval;

    FPerf_TerrainPhysicsOptimizationSettings()
        : IntegrationLevel(EPerf_TerrainPhysicsIntegrationLevel::Standard)
        , OptimizationMode(EPerf_TerrainPhysicsOptimizationMode::Balanced)
        , MaxFrameTimeMS(16.67f)
        , TargetFPS(60.0f)
        , MaxActivePhysicsBodies(1000)
        , CollisionComplexityThreshold(0.75f)
        , bEnableAdaptiveOptimization(true)
        , bEnableTerrainLOD(true)
        , bEnablePhysicsLOD(true)
        , OptimizationUpdateInterval(0.1f)
    {
    }
};

/**
 * Performance Optimizer for Terrain Physics Integration
 * Monitors and optimizes the integration between terrain systems and physics simulation
 * Ensures optimal performance for large-scale prehistoric environments
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_TerrainPhysicsIntegrationOptimizer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_TerrainPhysicsIntegrationOptimizer();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    FPerf_TerrainPhysicsIntegrationMetrics GetCurrentMetrics() const;

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetOptimizationSettings(const FPerf_TerrainPhysicsOptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    FPerf_TerrainPhysicsOptimizationSettings GetOptimizationSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeTerrainPhysicsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetIntegrationLevel(EPerf_TerrainPhysicsIntegrationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetOptimizationMode(EPerf_TerrainPhysicsOptimizationMode NewMode);

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Analysis")
    float GetTerrainPhysicsFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    float GetIntegrationEfficiency() const;

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    bool IsPerformanceOptimal() const;

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void GeneratePerformanceReport();

    // Event delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceThresholdExceeded, float, FrameTimeMS);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptimizationApplied, EPerf_TerrainPhysicsOptimizationMode, NewMode);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPerformanceThresholdExceeded OnPerformanceThresholdExceeded;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnOptimizationApplied OnOptimizationApplied;

protected:
    // Core optimization methods
    void UpdatePerformanceMetrics();
    void ApplyOptimizationLevel();
    void HandlePerformanceThreshold();
    void OptimizePhysicsSimulation();
    void OptimizeTerrainCollision();
    void OptimizeMemoryUsage();

    // Integration management
    void InitializeTerrainPhysicsIntegration();
    void UpdateIntegrationSettings();
    void ValidateIntegrationPerformance();

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_TerrainPhysicsIntegrationMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    FPerf_TerrainPhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    bool bIsMonitoring;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    bool bOptimizationActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    float LastOptimizationTime;

    // Timer handles
    FTimerHandle PerformanceUpdateTimer;
    FTimerHandle OptimizationTimer;

    // Performance thresholds
    static constexpr float CRITICAL_FRAME_TIME_MS = 33.33f; // 30 FPS threshold
    static constexpr float WARNING_FRAME_TIME_MS = 20.0f;   // 50 FPS threshold
    static constexpr float OPTIMAL_FRAME_TIME_MS = 16.67f;  // 60 FPS threshold
};