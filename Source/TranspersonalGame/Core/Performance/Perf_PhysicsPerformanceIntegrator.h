#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Perf_PhysicsPerformanceIntegrator.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsQualityLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra")
};

USTRUCT(BlueprintType)
struct FPerf_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SimulatingBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CollisionDetectionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionPairs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsMemoryUsage = 0.0f;

    FPerf_PhysicsPerformanceMetrics()
    {
        PhysicsFrameTime = 0.0f;
        ActiveRigidBodies = 0;
        SimulatingBodies = 0;
        CollisionDetectionTime = 0.0f;
        CollisionPairs = 0;
        PhysicsMemoryUsage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxPhysicsFrameTime = 16.67f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxSimulatingBodies = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAdaptiveQuality = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnablePhysicsLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float QualityAdjustmentThreshold = 20.0f; // ms

    FPerf_PhysicsOptimizationSettings()
    {
        MaxPhysicsFrameTime = 16.67f;
        MaxSimulatingBodies = 500;
        CullingDistance = 5000.0f;
        bEnableAdaptiveQuality = true;
        bEnablePhysicsLOD = true;
        QualityAdjustmentThreshold = 20.0f;
    }
};

/**
 * Performance optimizer specifically for physics systems integration
 * Monitors and optimizes physics performance in real-time
 * Integrates with Core_RealTimePhysicsManager from Agent #3
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PhysicsPerformanceIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PhysicsPerformanceIntegrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PhysicsPerformanceMetrics GetCurrentMetrics() const;

    // Quality management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPhysicsQualityLevel(EPerf_PhysicsQualityLevel QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PhysicsQualityLevel GetCurrentQualityLevel() const;

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsActors();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantPhysicsObjects();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustPhysicsLOD();

    // Integration with Core_RealTimePhysicsManager
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void IntegrateWithPhysicsManager();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ValidatePhysicsIntegration();

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationSettings(const FPerf_PhysicsOptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PhysicsOptimizationSettings GetOptimizationSettings() const;

    // Debug and testing
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void RunPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void LogCurrentPerformanceState();

protected:
    // Internal monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void AdjustQualityBasedOnPerformance();

    // Physics optimization
    void OptimizeRigidBodies();
    void OptimizeCollisionSettings();
    void OptimizePhysicsSubstepping();

    // Timer handles
    FTimerHandle PerformanceMonitoringTimer;
    FTimerHandle QualityAdjustmentTimer;

private:
    UPROPERTY()
    FPerf_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY()
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY()
    EPerf_PhysicsQualityLevel CurrentQualityLevel;

    UPROPERTY()
    bool bIsMonitoring;

    UPROPERTY()
    float LastFrameTime;

    UPROPERTY()
    int32 FrameCounter;

    UPROPERTY()
    TArray<float> FrameTimeHistory;

    // Performance tracking
    double LastPerformanceCheck;
    float AverageFrameTime;
    int32 QualityAdjustmentCount;
};