#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Perf_PhysicsConsolidationOptimizer.generated.h"

// Performance metrics for physics consolidation
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsConsolidationMetrics
{
    GENERATED_BODY()

    // Consolidation performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation")
    float ConsolidationTime;

    // Subsystem integration performance
    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation")
    float SubsystemIntegrationTime;

    // Validation performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation")
    float ValidationTime;

    // Memory usage during consolidation
    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation")
    float ConsolidationMemoryUsage;

    // Physics system count
    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation")
    int32 ActivePhysicsSystemCount;

    // Optimization effectiveness
    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation")
    float OptimizationEffectiveness;

    FPerf_PhysicsConsolidationMetrics()
    {
        ConsolidationTime = 0.0f;
        SubsystemIntegrationTime = 0.0f;
        ValidationTime = 0.0f;
        ConsolidationMemoryUsage = 0.0f;
        ActivePhysicsSystemCount = 0;
        OptimizationEffectiveness = 0.0f;
    }
};

// Physics consolidation optimization settings
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsConsolidationSettings
{
    GENERATED_BODY()

    // Maximum consolidation time threshold (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxConsolidationTime;

    // Maximum subsystem integration time (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxSubsystemIntegrationTime;

    // Memory usage threshold for consolidation (MB)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float ConsolidationMemoryThreshold;

    // Enable aggressive optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    bool bEnableAggressiveOptimization;

    // Enable consolidation batching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    bool bEnableConsolidationBatching;

    // Consolidation batch size
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    int32 ConsolidationBatchSize;

    FPerf_PhysicsConsolidationSettings()
    {
        MaxConsolidationTime = 16.67f; // 60fps target
        MaxSubsystemIntegrationTime = 8.33f; // Half frame budget
        ConsolidationMemoryThreshold = 512.0f; // 512MB
        bEnableAggressiveOptimization = true;
        bEnableConsolidationBatching = true;
        ConsolidationBatchSize = 4;
    }
};

// Physics consolidation performance state
UENUM(BlueprintType)
enum class EPerf_PhysicsConsolidationState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Consolidating UMETA(DisplayName = "Consolidating"),
    Validating UMETA(DisplayName = "Validating"),
    Optimizing UMETA(DisplayName = "Optimizing"),
    Complete UMETA(DisplayName = "Complete"),
    Error UMETA(DisplayName = "Error")
};

/**
 * Performance optimizer for physics consolidation system
 * Monitors and optimizes the Core_PhysicsConsolidator performance
 * Ensures 60fps PC / 30fps console targets during physics consolidation
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsConsolidationOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsConsolidationOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation Performance")
    void StartConsolidationMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation Performance")
    void StopConsolidationMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation Performance")
    FPerf_PhysicsConsolidationMetrics GetConsolidationMetrics() const;

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation Performance")
    void OptimizeConsolidationPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation Performance")
    void SetConsolidationSettings(const FPerf_PhysicsConsolidationSettings& NewSettings);

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation Performance")
    bool IsConsolidationPerformanceOptimal() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation Performance")
    float GetConsolidationEfficiency() const;

    // State management
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation Performance")
    EPerf_PhysicsConsolidationState GetConsolidationState() const;

protected:
    // Performance metrics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_PhysicsConsolidationMetrics CurrentMetrics;

    // Optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_PhysicsConsolidationSettings ConsolidationSettings;

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance State")
    EPerf_PhysicsConsolidationState CurrentState;

    // Monitoring state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance State")
    bool bIsMonitoring;

    // Performance tracking
    float ConsolidationStartTime;
    float LastOptimizationTime;
    int32 OptimizationCycleCount;

private:
    // Internal performance tracking
    void UpdateConsolidationMetrics();
    void AnalyzeConsolidationPerformance();
    void ApplyPerformanceOptimizations();
    void LogPerformanceMetrics();
};