#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_PerformanceMonitor.generated.h"

/**
 * Performance monitoring system for engine architecture
 * Tracks FPS, memory usage, system load, and performance bottlenecks
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount;

    FEng_PerformanceData()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        MemoryUsageMB = 0.0f;
        CPUUsagePercent = 0.0f;
        GPUUsagePercent = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_PerformanceMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_PerformanceMonitor();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    FEng_PerformanceData GetCurrentPerformanceData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    float GetAverageFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void LogPerformanceReport() const;

    // Performance thresholds
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void SetPerformanceThresholds(float MinFPS, float MaxMemoryMB);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    bool IsPerformanceBelowThreshold() const;

    // System monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void StartPerformanceCapture();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void StopPerformanceCapture();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void ResetPerformanceData();

protected:
    // Performance data
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitor")
    FEng_PerformanceData CurrentData;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitor")
    TArray<float> FPSHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitor")
    TArray<float> MemoryHistory;

    // Monitoring settings
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitor")
    float MinimumFPSThreshold;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitor")
    float MaximumMemoryThreshold;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitor")
    bool bIsMonitoringActive;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitor")
    float MonitoringInterval;

private:
    // Internal monitoring
    void UpdatePerformanceData();
    void CalculateAverages();
    void CheckPerformanceThresholds();

    // Timing
    float LastUpdateTime;
    float AccumulatedTime;
    int32 FrameCount;
    
    static constexpr float UPDATE_INTERVAL = 0.5f;
    static constexpr int32 MAX_HISTORY_SIZE = 100;
};