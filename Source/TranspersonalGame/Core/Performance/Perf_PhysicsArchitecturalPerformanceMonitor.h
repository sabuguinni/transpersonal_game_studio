#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Perf_PhysicsArchitecturalPerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsArchitecturalMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Architectural Performance")
    float FrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Architectural Performance")
    int32 PhysicsActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Architectural Performance")
    float PhysicsUpdateTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Architectural Performance")
    float ArchitecturalComplianceOverheadMS;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Architectural Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Architectural Performance")
    bool bIsPerformanceWithinBudget;

    FPerf_PhysicsArchitecturalMetrics()
    {
        FrameTimeMS = 0.0f;
        PhysicsActorCount = 0;
        PhysicsUpdateTimeMS = 0.0f;
        ArchitecturalComplianceOverheadMS = 0.0f;
        MemoryUsageMB = 0.0f;
        bIsPerformanceWithinBudget = true;
    }
};

UENUM(BlueprintType)
enum class EPerf_PhysicsArchitecturalOptimizationLevel : uint8
{
    Low = 0,
    Medium = 1,
    High = 2,
    Ultra = 3
};

/**
 * Performance monitor specifically for Physics Architectural Compliance system integration.
 * Tracks performance impact of architectural compliance checks on physics simulation.
 * Ensures 60fps target is maintained even with compliance validation overhead.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance))
class TRANSPERSONALGAME_API UPerf_PhysicsArchitecturalPerformanceMonitor : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsArchitecturalPerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Physics Architectural Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Architectural Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Architectural Performance")
    FPerf_PhysicsArchitecturalMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Architectural Performance")
    void SetOptimizationLevel(EPerf_PhysicsArchitecturalOptimizationLevel NewLevel);

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Physics Architectural Performance")
    bool IsPerformanceWithinBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Architectural Performance")
    float GetArchitecturalComplianceOverhead() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Architectural Performance")
    void OptimizePhysicsArchitecturalIntegration();

    // Profiling and metrics
    UFUNCTION(BlueprintCallable, Category = "Physics Architectural Performance")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Physics Architectural Performance")
    void ExportMetricsToFile(const FString& FilePath);

    // Real-time optimization
    UFUNCTION(BlueprintCallable, Category = "Physics Architectural Performance")
    void AdjustComplianceCheckFrequency(float NewFrequency);

    UFUNCTION(BlueprintCallable, Category = "Physics Architectural Performance")
    void EnableAdaptiveOptimization(bool bEnable);

protected:
    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Monitoring")
    FPerf_PhysicsArchitecturalMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float TargetFrameTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MaxAllowedComplianceOverheadMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    EPerf_PhysicsArchitecturalOptimizationLevel OptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bAdaptiveOptimizationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float ComplianceCheckFrequency;

    // Internal monitoring state
    UPROPERTY()
    bool bIsMonitoring;

    UPROPERTY()
    float LastFrameTime;

    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    float ComplianceOverheadAccumulator;

    UPROPERTY()
    int32 FrameCounter;

    // Performance thresholds
    UPROPERTY(EditAnywhere, Category = "Performance Thresholds")
    float CriticalFrameTimeThreshold;

    UPROPERTY(EditAnywhere, Category = "Performance Thresholds")
    float WarningFrameTimeThreshold;

    UPROPERTY(EditAnywhere, Category = "Performance Thresholds")
    int32 MaxPhysicsActorsForOptimalPerformance;

private:
    // Internal performance tracking
    void UpdateMetrics(float DeltaTime);
    void AnalyzePerformanceTrends();
    void ApplyOptimizations();
    float CalculateComplianceOverhead();
    void CollectPhysicsActorMetrics();
    void CheckPerformanceThresholds();
    void LogPerformanceWarning(const FString& Warning);
    void AdjustOptimizationBasedOnPerformance();

    // Timing utilities
    double LastUpdateTime;
    double ComplianceCheckStartTime;
    TArray<double> ComplianceCheckTimes;
};