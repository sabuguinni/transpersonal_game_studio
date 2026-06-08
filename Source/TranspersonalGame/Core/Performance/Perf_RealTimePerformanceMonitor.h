#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "SharedTypes.h"
#include "Perf_RealTimePerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f;

    FPerf_FrameMetrics()
    {
        CurrentFPS = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float UsedPhysicalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float UsedVirtualMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PeakUsedPhysicalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PeakUsedVirtualMemoryMB = 0.0f;

    FPerf_MemoryMetrics()
    {
        UsedPhysicalMemoryMB = 0.0f;
        UsedVirtualMemoryMB = 0.0f;
        PeakUsedPhysicalMemoryMB = 0.0f;
        PeakUsedVirtualMemoryMB = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUUsagePercent = 0.0f;

    FPerf_SystemMetrics()
    {
        DrawCalls = 0;
        Triangles = 0;
        ActiveActors = 0;
        VisibleActors = 0;
        CPUUsagePercent = 0.0f;
    }
};

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Excellent   UMETA(DisplayName = "Excellent (60+ FPS)"),
    Good        UMETA(DisplayName = "Good (45-60 FPS)"),
    Average     UMETA(DisplayName = "Average (30-45 FPS)"),
    Poor        UMETA(DisplayName = "Poor (15-30 FPS)"),
    Critical    UMETA(DisplayName = "Critical (<15 FPS)")
};

/**
 * Real-time performance monitoring subsystem
 * Tracks FPS, memory, CPU, GPU metrics and provides optimization suggestions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_RealTimePerformanceMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_RealTimePerformanceMonitor();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsMonitoring() const { return bIsMonitoring; }

    // Metrics access
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_FrameMetrics GetCurrentFrameMetrics() const { return CurrentFrameMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_MemoryMetrics GetCurrentMemoryMetrics() const { return CurrentMemoryMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_SystemMetrics GetCurrentSystemMetrics() const { return CurrentSystemMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const;

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetOptimizationSuggestions() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyAutomaticOptimizations();

    // Performance targets
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float InTargetFPS) { TargetFPS = InTargetFPS; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetTargetFPS() const { return TargetFPS; }

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceChanged, EPerf_PerformanceLevel, NewLevel);
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceChanged OnPerformanceChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceCritical, float, CurrentFPS);
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceCritical OnPerformanceCritical;

protected:
    // Core monitoring data
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsMonitoring = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFPS = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MonitoringInterval = 0.1f;

    // Current metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_FrameMetrics CurrentFrameMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_MemoryMetrics CurrentMemoryMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_SystemMetrics CurrentSystemMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_PerformanceLevel LastPerformanceLevel = EPerf_PerformanceLevel::Excellent;

    // Historical data for analysis
    UPROPERTY()
    TArray<float> FPSHistory;

    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    TArray<float> MemoryHistory;

    // Timer handles
    FTimerHandle MonitoringTimerHandle;

private:
    // Internal monitoring functions
    void UpdateMetrics();
    void UpdateFrameMetrics();
    void UpdateMemoryMetrics();
    void UpdateSystemMetrics();
    void CheckPerformanceLevel();
    void TriggerOptimizationIfNeeded();

    // Optimization functions
    void OptimizeLODSettings();
    void OptimizeCullingSettings();
    void OptimizeMemorySettings();
    void OptimizeRenderingSettings();

    // Analysis helpers
    float CalculateAverageFPS(int32 SampleCount = 30) const;
    float CalculateFrameTimeVariance() const;
    bool IsPerformanceStable() const;
};