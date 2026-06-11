#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "SharedTypes.h"
#include "Perf_PerformanceManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTimeMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTimeMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ComponentCount = 0;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        FrameTimeMS = 0.0f;
        GameThreadTimeMS = 0.0f;
        RenderThreadTimeMS = 0.0f;
        GPUTimeMS = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        ActorCount = 0;
        ComponentCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Budget")
    float TargetFPS = 60.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Budget")
    float MaxFrameTimeMS = 16.67f;

    UPROPERTY(BlueprintReadWrite, Category = "Budget")
    float MaxGameThreadTimeMS = 4.17f;

    UPROPERTY(BlueprintReadWrite, Category = "Budget")
    float MaxRenderThreadTimeMS = 10.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Budget")
    float MaxGPUTimeMS = 12.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Budget")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(BlueprintReadWrite, Category = "Budget")
    int32 MaxTriangles = 500000;

    UPROPERTY(BlueprintReadWrite, Category = "Budget")
    float MaxMemoryUsageMB = 4096.0f;

    FPerf_PerformanceBudget()
    {
        TargetFPS = 60.0f;
        MaxFrameTimeMS = 16.67f;
        MaxGameThreadTimeMS = 4.17f;
        MaxRenderThreadTimeMS = 10.0f;
        MaxGPUTimeMS = 12.5f;
        MaxDrawCalls = 2000;
        MaxTriangles = 500000;
        MaxMemoryUsageMB = 4096.0f;
    }
};

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Excellent   UMETA(DisplayName = "Excellent"),
    Good        UMETA(DisplayName = "Good"),
    Fair        UMETA(DisplayName = "Fair"),
    Poor        UMETA(DisplayName = "Poor"),
    Critical    UMETA(DisplayName = "Critical")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnPerformanceAlert, EPerf_PerformanceLevel, PerformanceLevel);

/**
 * Performance Manager - Monitors and optimizes game performance
 * Tracks FPS, frame times, memory usage, and provides optimization recommendations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PerformanceManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_PerformanceBudget GetPerformanceBudget() const { return PerformanceBudget; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceBudget(const FPerf_PerformanceBudget& NewBudget);

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel AnalyzeCurrentPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetOptimizationRecommendations();

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODDistanceScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetShadowQuality(int32 Quality);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetViewDistanceScale(float Scale);

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsageMB();

    // Performance alerts
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FPerf_OnPerformanceAlert OnPerformanceAlert;

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void SavePerformanceReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    FPerf_PerformanceBudget PerformanceBudget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MonitoringInterval = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<FPerf_PerformanceMetrics> MetricsHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaxHistorySize = 60;

private:
    FTimerHandle MonitoringTimerHandle;
    float LastUpdateTime = 0.0f;

    void CollectPerformanceData();
    void CheckPerformanceThresholds();
    void TriggerPerformanceAlert(EPerf_PerformanceLevel Level);
    
    // Internal optimization functions
    void OptimizeRendering();
    void OptimizePhysics();
    void OptimizeMemory();
    void OptimizeLOD();
};