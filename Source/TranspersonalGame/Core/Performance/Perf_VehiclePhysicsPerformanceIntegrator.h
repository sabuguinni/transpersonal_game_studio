#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/Platform.h"
#include "Stats/Stats.h"
#include "Engine/EngineTypes.h"
#include "Perf_VehiclePhysicsPerformanceIntegrator.generated.h"

// Forward declarations
class UCore_VehiclePhysicsSystem;
class AActor;
class UWorld;

UENUM(BlueprintType)
enum class EPerf_VehiclePerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"), 
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Minimal     UMETA(DisplayName = "Minimal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_VehiclePhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float SurfaceDetectionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DamageCalculationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float SuspensionUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TirePhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveVehicleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceTargetMet;

    FPerf_VehiclePhysicsMetrics()
        : PhysicsUpdateTime(0.0f)
        , SurfaceDetectionTime(0.0f)
        , DamageCalculationTime(0.0f)
        , SuspensionUpdateTime(0.0f)
        , TirePhysicsTime(0.0f)
        , ActiveVehicleCount(0)
        , MemoryUsageMB(0.0f)
        , AverageFrameTime(0.0f)
        , bPerformanceTargetMet(true)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_VehicleOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxPhysicsUpdateDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float SurfaceDetectionLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxConcurrentVehicles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAsyncPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableDamageSimulation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float TargetFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    EPerf_VehiclePerformanceLevel PerformanceLevel;

    FPerf_VehicleOptimizationSettings()
        : MaxPhysicsUpdateDistance(5000.0f)
        , SurfaceDetectionLODDistance(2000.0f)
        , MaxConcurrentVehicles(10)
        , bEnableAsyncPhysics(true)
        , bEnableDamageSimulation(true)
        , TargetFrameTime(16.67f)
        , PerformanceLevel(EPerf_VehiclePerformanceLevel::High)
    {
    }
};

/**
 * Advanced Vehicle Physics Performance Integration System
 * Monitors and optimizes vehicle physics performance for prehistoric transportation
 * Integrates with Core_VehiclePhysicsSystem for comprehensive performance management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_VehiclePhysicsPerformanceIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_VehiclePhysicsPerformanceIntegrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Initialize vehicle physics performance monitoring
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void InitializeVehiclePerformanceMonitoring();

    /**
     * Update performance metrics for all active vehicles
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateVehiclePerformanceMetrics();

    /**
     * Get current vehicle physics performance metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_VehiclePhysicsMetrics GetVehiclePhysicsMetrics() const;

    /**
     * Set vehicle optimization settings
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetVehicleOptimizationSettings(const FPerf_VehicleOptimizationSettings& Settings);

    /**
     * Get current optimization settings
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_VehicleOptimizationSettings GetOptimizationSettings() const;

    /**
     * Optimize vehicle physics performance based on current metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeVehiclePhysicsPerformance();

    /**
     * Set performance level for adaptive optimization
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_VehiclePerformanceLevel Level);

    /**
     * Check if performance targets are being met
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ArePerformanceTargetsMet() const;

    /**
     * Get performance optimization recommendations
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetPerformanceRecommendations() const;

    /**
     * Enable/disable vehicle physics debugging
     */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void SetVehiclePhysicsDebugging(bool bEnabled);

    /**
     * Get debug information for vehicle physics performance
     */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetVehiclePhysicsDebugInfo() const;

protected:
    /**
     * Update vehicle physics LOD based on distance and performance
     */
    void UpdateVehiclePhysicsLOD();

    /**
     * Monitor vehicle physics memory usage
     */
    void MonitorVehicleMemoryUsage();

    /**
     * Apply performance optimizations based on current metrics
     */
    void ApplyPerformanceOptimizations();

    /**
     * Calculate vehicle physics performance score
     */
    float CalculateVehiclePerformanceScore() const;

    /**
     * Update async physics settings for vehicles
     */
    void UpdateAsyncPhysicsSettings();

private:
    UPROPERTY()
    FPerf_VehiclePhysicsMetrics CurrentMetrics;

    UPROPERTY()
    FPerf_VehicleOptimizationSettings OptimizationSettings;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> TrackedVehicles;

    UPROPERTY()
    bool bIsMonitoringActive;

    UPROPERTY()
    bool bDebugEnabled;

    UPROPERTY()
    float LastOptimizationTime;

    UPROPERTY()
    float OptimizationInterval;

    // Performance tracking
    TArray<float> FrameTimeHistory;
    TArray<float> PhysicsTimeHistory;
    float AccumulatedPhysicsTime;
    float AccumulatedFrameTime;
    int32 SampleCount;

    // Constants
    static constexpr float PERFORMANCE_UPDATE_INTERVAL = 1.0f;
    static constexpr int32 MAX_FRAME_HISTORY = 60;
    static constexpr float TARGET_60FPS_FRAME_TIME = 16.67f;
    static constexpr float TARGET_30FPS_FRAME_TIME = 33.33f;
};