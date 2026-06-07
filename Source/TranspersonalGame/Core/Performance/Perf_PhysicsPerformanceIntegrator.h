#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Perf_PhysicsPerformanceIntegrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SimulatingBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsStepTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CollisionDetectionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionPairs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FPerf_PhysicsPerformanceMetrics()
        : CurrentFPS(60.0f)
        , AverageFrameTime(16.67f)
        , ActivePhysicsBodies(0)
        , SimulatingBodies(0)
        , PhysicsStepTime(0.0f)
        , CollisionDetectionTime(0.0f)
        , CollisionPairs(0)
        , MemoryUsageMB(0.0f)
    {}
};

UENUM(BlueprintType)
enum class EPerf_PhysicsOptimizationLevel : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Conservative    UMETA(DisplayName = "Conservative"),
    Balanced        UMETA(DisplayName = "Balanced"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Maximum         UMETA(DisplayName = "Maximum")
};

UENUM(BlueprintType)
enum class EPerf_PhysicsQualityMode : uint8
{
    Ultra           UMETA(DisplayName = "Ultra Quality"),
    High            UMETA(DisplayName = "High Quality"),
    Medium          UMETA(DisplayName = "Medium Quality"),
    Low             UMETA(DisplayName = "Low Quality"),
    Performance     UMETA(DisplayName = "Performance Mode")
};

/**
 * Physics Performance Integrator - Monitors and optimizes physics performance
 * Integrates with Core_PhysicsIntegration to provide performance monitoring and optimization
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsPerformanceIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring", meta = (AllowPrivateAccess = "true"))
    FPerf_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MinimumFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    EPerf_PhysicsOptimizationLevel OptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    EPerf_PhysicsQualityMode QualityMode;

    // Physics Performance Budgets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budgets")
    int32 MaxPhysicsBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budgets")
    int32 MaxSimulatingBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budgets")
    float MaxPhysicsStepTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budgets")
    int32 MaxCollisionPairs;

    // Optimization Controls
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAutomaticOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableLODOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableCollisionOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableMemoryOptimization;

    // Distance-based optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Optimization")
    float NearDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Optimization")
    float MediumDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Optimization")
    float FarDistance;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float CriticalFPSThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float WarningFPSThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float OptimalFPSThreshold;

    // Monitoring intervals
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float MetricsUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float OptimizationCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    bool bEnableDetailedLogging;

    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PhysicsPerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationLevel(EPerf_PhysicsOptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityMode(EPerf_PhysicsQualityMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeNearbyActors(float Radius);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeActorPhysics(AActor* Actor, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void AnalyzePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    bool IsPerformanceCritical() const;

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    float GetPerformanceScore() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ResetPerformanceCounters();

private:
    // Internal monitoring
    float LastMetricsUpdate;
    float LastOptimizationCheck;
    bool bIsMonitoring;
    
    // Performance history
    TArray<float> FPSHistory;
    TArray<float> FrameTimeHistory;
    int32 MaxHistorySize;

    // Internal methods
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void ApplyOptimizationSettings();
    void OptimizePhysicsBodies();
    void OptimizeCollisionSettings();
    void UpdatePhysicsQuality();
    float CalculateDistance(const AActor* Actor) const;
    void ApplyDistanceBasedOptimization(AActor* Actor, float Distance);
    void CollectPhysicsStatistics();
    void UpdateFPSHistory(float CurrentFPS);
    float GetAverageFPS() const;
    void LogOptimizationAction(const FString& Action, const FString& Details);
};