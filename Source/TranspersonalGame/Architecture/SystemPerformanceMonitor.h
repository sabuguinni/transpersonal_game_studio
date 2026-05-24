#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "SystemPerformanceMonitor.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSystemPerformance, Log, All);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f;

    FEng_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        FrameTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        ActiveActors = 0;
        VisibleActors = 0;
        CPUTime = 0.0f;
        GPUTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MinFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxFrameTime = 33.33f; // 30 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxTriangles = 2000000; // 2M triangles

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxMemoryUsageMB = 8192.0f; // 8GB

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxActiveActors = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxCPUTime = 16.67f; // Half frame time at 30fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxGPUTime = 16.67f; // Half frame time at 30fps

    FEng_PerformanceThresholds()
    {
        MinFrameRate = 30.0f;
        MaxFrameTime = 33.33f;
        MaxDrawCalls = 2000;
        MaxTriangles = 2000000;
        MaxMemoryUsageMB = 8192.0f;
        MaxActiveActors = 10000;
        MaxCPUTime = 16.67f;
        MaxGPUTime = 16.67f;
    }
};

/**
 * System performance monitoring and optimization manager
 * Tracks real-time performance metrics and enforces performance budgets
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API USystemPerformanceMonitor : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    USystemPerformanceMonitor();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    bool IsPerformanceWithinThresholds() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    TArray<FString> GetPerformanceWarnings() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void SetPerformanceThresholds(const FEng_PerformanceThresholds& NewThresholds);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    FEng_PerformanceThresholds GetPerformanceThresholds() const { return PerformanceThresholds; }

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void ReduceActorCount();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeLODSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeCullingSettings();

    // Profiling
    UFUNCTION(BlueprintCallable, Category = "Profiling", CallInEditor)
    void StartPerformanceProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling", CallInEditor)
    void StopPerformanceProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling", CallInEditor)
    void GeneratePerformanceReport();

    // Budget management
    UFUNCTION(BlueprintCallable, Category = "Budget Management")
    bool IsWithinMemoryBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Budget Management")
    bool IsWithinDrawCallBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Budget Management")
    bool IsWithinTriangleBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Budget Management")
    float GetMemoryBudgetUsagePercent() const;

protected:
    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    FEng_PerformanceThresholds PerformanceThresholds;

    // Monitoring settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monitoring Settings")
    float MonitoringInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monitoring Settings")
    bool bEnableAutoOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monitoring Settings")
    bool bLogPerformanceWarnings = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monitoring Settings")
    bool bEnableProfilingMode = false;

    // Current metrics
    UPROPERTY(BlueprintReadOnly, Category = "Current Metrics")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Current Metrics")
    TArray<FString> CurrentWarnings;

    UPROPERTY(BlueprintReadOnly, Category = "Current Metrics")
    bool bPerformanceHealthy = true;

private:
    // Internal monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void LogPerformanceData();
    
    // Metric collection
    float CollectFrameRate() const;
    float CollectFrameTime() const;
    int32 CollectDrawCalls() const;
    int32 CollectTriangles() const;
    float CollectMemoryUsage() const;
    int32 CollectActiveActors() const;
    int32 CollectVisibleActors() const;
    float CollectCPUTime() const;
    float CollectGPUTime() const;
    
    // Optimization helpers
    void AutoOptimizeIfNeeded();
    void OptimizeActorCulling();
    void OptimizeTextureLODs();
    void OptimizeMeshLODs();
    void OptimizeParticleSystems();
    
    // Profiling data
    TArray<FEng_PerformanceMetrics> ProfilingHistory;
    float ProfilingStartTime = 0.0f;
    bool bIsProfiling = false;
    
    // Timer handles
    FTimerHandle MonitoringTimer;
    FTimerHandle OptimizationTimer;
};