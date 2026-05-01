#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Perf_FPSMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
    }
};

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Excellent   UMETA(DisplayName = "Excellent (60+ FPS)"),
    Good        UMETA(DisplayName = "Good (45-60 FPS)"),
    Acceptable  UMETA(DisplayName = "Acceptable (30-45 FPS)"),
    Poor        UMETA(DisplayName = "Poor (15-30 FPS)"),
    Critical    UMETA(DisplayName = "Critical (<15 FPS)")
};

/**
 * Performance monitoring actor that tracks FPS, frame times, and rendering metrics
 * Provides real-time performance data for optimization decisions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_FPSMonitor : public AActor
{
    GENERATED_BODY()

public:
    APerf_FPSMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    /** Root component for the monitor */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    /** Current performance metrics */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    /** Performance level based on current FPS */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel PerformanceLevel;

    /** Enable/disable performance monitoring */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableMonitoring;

    /** Update frequency in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float UpdateFrequency;

    /** Number of samples to average for FPS calculation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "10", ClampMax = "1000"))
    int32 SampleCount;

    /** Log performance warnings when FPS drops below threshold */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bLogPerformanceWarnings;

    /** FPS threshold for performance warnings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "15", ClampMax = "60"))
    float WarningFPSThreshold;

    /** Get current FPS */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

    /** Get average FPS over sample period */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const;

    /** Get performance level enum */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const;

    /** Reset performance statistics */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetStatistics();

    /** Enable/disable monitoring at runtime */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMonitoringEnabled(bool bEnabled);

    /** Get detailed performance report as string */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetPerformanceReport() const;

    /** Check if performance is below acceptable threshold */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceCritical() const;

private:
    /** Array of FPS samples for averaging */
    TArray<float> FPSSamples;

    /** Current sample index */
    int32 CurrentSampleIndex;

    /** Time since last update */
    float TimeSinceLastUpdate;

    /** Total samples collected */
    int32 TotalSamples;

    /** Update performance metrics */
    void UpdateMetrics(float DeltaTime);

    /** Calculate performance level based on FPS */
    EPerf_PerformanceLevel CalculatePerformanceLevel(float FPS) const;

    /** Log performance warning if needed */
    void CheckPerformanceWarnings() const;
};