#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "HAL/IConsoleManager.h"
#include "Perf_SystemsIntegrationOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_SystemIntegrationLevel : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Integration"),
    Standard    UMETA(DisplayName = "Standard Integration"),
    Enhanced    UMETA(DisplayName = "Enhanced Integration"),
    Maximum     UMETA(DisplayName = "Maximum Integration")
};

USTRUCT(BlueprintType)
struct FPerf_SystemPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float InitializationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveSystemCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsOptimized;

    FPerf_SystemPerformanceMetrics()
        : InitializationTime(0.0f)
        , AverageFrameTime(0.0f)
        , MemoryUsageMB(0.0f)
        , ActiveSystemCount(0)
        , bIsOptimized(false)
    {
    }
};

USTRUCT(BlueprintType)
struct FPerf_IntegrationOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    EPerf_SystemIntegrationLevel IntegrationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float MaxMemoryBudgetMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bEnableAdaptiveOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bEnablePerformanceLogging;

    FPerf_IntegrationOptimizationSettings()
        : IntegrationLevel(EPerf_SystemIntegrationLevel::Standard)
        , TargetFrameRate(60.0f)
        , MaxMemoryBudgetMB(512.0f)
        , bEnableAdaptiveOptimization(true)
        , bEnablePerformanceLogging(false)
    {
    }
};

/**
 * Performance Optimizer for Systems Integration
 * Optimizes the integration between Core Systems and Engine Architect's registry
 * Ensures 60fps PC / 30fps console performance targets are maintained
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_SystemsIntegrationOptimizer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_SystemsIntegrationOptimizer();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Optimize systems integration for target performance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    void OptimizeSystemsIntegration();

    /**
     * Analyze current systems performance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    FPerf_SystemPerformanceMetrics AnalyzeSystemsPerformance();

    /**
     * Apply optimization settings to systems integration
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    void ApplyOptimizationSettings(const FPerf_IntegrationOptimizationSettings& Settings);

    /**
     * Get current optimization settings
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    FPerf_IntegrationOptimizationSettings GetOptimizationSettings() const;

    /**
     * Enable or disable adaptive optimization
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    void SetAdaptiveOptimization(bool bEnabled);

    /**
     * Get performance metrics for specific system
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    float GetSystemPerformanceMetric(const FString& SystemName);

    /**
     * Force optimization update
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    void ForceOptimizationUpdate();

    /**
     * Check if systems are performing within target parameters
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    bool AreSystemsOptimized() const;

    /**
     * Get integration level recommendation based on current performance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    EPerf_SystemIntegrationLevel GetRecommendedIntegrationLevel() const;

    /**
     * Reset optimization to default settings
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    void ResetOptimization();

protected:
    /**
     * Current optimization settings
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_IntegrationOptimizationSettings CurrentSettings;

    /**
     * Current performance metrics
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_SystemPerformanceMetrics CurrentMetrics;

    /**
     * Performance monitoring enabled
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceMonitoringEnabled;

    /**
     * Last optimization update time
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastOptimizationTime;

private:
    /**
     * Initialize performance monitoring
     */
    void InitializePerformanceMonitoring();

    /**
     * Update performance metrics
     */
    void UpdatePerformanceMetrics();

    /**
     * Apply integration level optimizations
     */
    void ApplyIntegrationLevelOptimizations(EPerf_SystemIntegrationLevel Level);

    /**
     * Check if adaptive optimization should trigger
     */
    bool ShouldTriggerAdaptiveOptimization() const;

    /**
     * Perform adaptive optimization
     */
    void PerformAdaptiveOptimization();

    /**
     * Log performance metrics
     */
    void LogPerformanceMetrics() const;

    /**
     * Timer handle for performance monitoring
     */
    FTimerHandle PerformanceMonitoringTimer;

    /**
     * Performance sampling interval
     */
    float PerformanceSamplingInterval;

    /**
     * Performance history for trend analysis
     */
    TArray<float> FrameTimeHistory;

    /**
     * Maximum history size
     */
    int32 MaxHistorySize;
};