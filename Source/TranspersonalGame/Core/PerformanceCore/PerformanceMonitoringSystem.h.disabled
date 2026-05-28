#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"
#include "Stats/Stats.h"
#include "RHI.h"
#include "RenderingThread.h"
#include "PerformanceMonitoringSystem.generated.h"

// Performance monitoring enums
UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"),
    High        UMETA(DisplayName = "High Performance"),
    Ultra       UMETA(DisplayName = "Ultra Performance")
};

UENUM(BlueprintType)
enum class EPerf_BottleneckType : uint8
{
    CPU         UMETA(DisplayName = "CPU Bottleneck"),
    GPU         UMETA(DisplayName = "GPU Bottleneck"),
    Memory      UMETA(DisplayName = "Memory Bottleneck"),
    IO          UMETA(DisplayName = "I/O Bottleneck"),
    Network     UMETA(DisplayName = "Network Bottleneck"),
    None        UMETA(DisplayName = "No Bottleneck")
};

// Performance metrics structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_BottleneckType CurrentBottleneck;

    FPerf_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        FrameTime = 0.0f;
        CPUTime = 0.0f;
        GPUTime = 0.0f;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        CurrentBottleneck = EPerf_BottleneckType::None;
    }
};

// LOD management structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistanceScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxLODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableAutomaticLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullingDistance;

    FPerf_LODSettings()
    {
        LODDistanceScale = 1.0f;
        MaxLODLevel = 3;
        bEnableAutomaticLOD = true;
        CullingDistance = 10000.0f;
    }
};

/**
 * Performance Monitoring System
 * Tracks frame rate, memory usage, and automatically adjusts quality settings
 * to maintain target performance levels for the prehistoric survival game.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceMonitoringSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceMonitoringSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const;

    // Quality adjustment
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustQualitySettings(EPerf_PerformanceLevel TargetLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAdaptiveQuality(bool bEnable);

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_LODSettings GetLODSettings() const;

    // Culling optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCullingDistance(float Distance);

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsageMB() const;

    // Debug and profiling
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void EnablePerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void DisablePerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

protected:
    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MinAcceptableFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxMemoryUsageMB;

    // Current metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel CurrentLevel;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAdaptiveQualityEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bMonitoringEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MonitoringInterval;

private:
    // Internal monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void AdjustQualityAutomatically();
    EPerf_BottleneckType DetectBottleneck() const;

    // Timer handle for monitoring
    FTimerHandle MonitoringTimerHandle;

    // Performance history for trend analysis
    TArray<float> FrameRateHistory;
    TArray<float> MemoryHistory;
    static const int32 MaxHistorySize = 60; // 1 minute at 1Hz sampling
};