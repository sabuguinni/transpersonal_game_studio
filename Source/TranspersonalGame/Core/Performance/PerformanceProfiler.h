#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "Stats/Stats.h"
#include "PerformanceProfiler.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPerformanceProfiler, Log, All);

/** Performance metrics structure */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    /** Frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    /** Game thread time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    /** Render thread time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    /** GPU time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f;

    /** Current FPS */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FPS = 0.0f;

    /** Memory usage in MB */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    /** Number of draw calls */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    /** Number of triangles rendered */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles = 0;

    /** Number of active physics bodies */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsBodies = 0;

    /** Audio processing time */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AudioTime = 0.0f;
};

/** Performance target settings */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceTargets
{
    GENERATED_BODY()

    /** Target frame rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFPS = 60.0f;

    /** Maximum acceptable frame time in ms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxFrameTime = 16.67f; // 60fps = 16.67ms

    /** Maximum GPU time in ms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxGPUTime = 12.0f;

    /** Maximum memory usage in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxMemoryUsageMB = 4096.0f;

    /** Maximum draw calls per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    int32 MaxDrawCalls = 2000;

    /** Maximum physics bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    int32 MaxPhysicsBodies = 1000;
};

/** Performance optimization settings */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceOptimizationSettings
{
    GENERATED_BODY()

    /** Enable automatic LOD adjustment */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bAutoLODAdjustment = true;

    /** Enable dynamic resolution scaling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bDynamicResolutionScaling = true;

    /** Enable physics LOD */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bPhysicsLOD = true;

    /** Enable culling optimizations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bCullingOptimizations = true;

    /** Enable texture streaming optimizations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bTextureStreamingOptimizations = true;

    /** Minimum resolution scale (0.5 = 50%) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (ClampMin = "0.3", ClampMax = "1.0"))
    float MinResolutionScale = 0.5f;

    /** Maximum resolution scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (ClampMin = "1.0", ClampMax = "2.0"))
    float MaxResolutionScale = 1.0f;
};

/**
 * Performance Profiler System
 * Monitors real-time performance and applies automatic optimizations
 * Ensures 60fps on PC and 30fps on console
 */
UCLASS()
class TRANSPERSONALGAME_API UPerformanceProfiler : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Get the performance profiler instance */
    UFUNCTION(BlueprintPure, Category = "Performance")
    static UPerformanceProfiler* Get(const UObject* WorldContext);

    /** Get current performance metrics */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    /** Get performance targets */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceTargets GetPerformanceTargets() const { return PerformanceTargets; }

    /** Set performance targets */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTargets(const FPerformanceTargets& NewTargets);

    /** Get optimization settings */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceOptimizationSettings GetOptimizationSettings() const { return OptimizationSettings; }

    /** Set optimization settings */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationSettings(const FPerformanceOptimizationSettings& NewSettings);

    /** Enable/disable performance monitoring */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceMonitoringEnabled(bool bEnabled);

    /** Enable/disable automatic optimizations */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAutoOptimizationEnabled(bool bEnabled);

    /** Force performance optimization update */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceOptimizationUpdate();

    /** Get average FPS over last N seconds */
    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetAverageFPS(float TimeWindow = 5.0f) const;

    /** Check if performance is within targets */
    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsPerformanceWithinTargets() const;

    /** Get performance bottleneck type */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FString GetPerformanceBottleneck() const;

    /** Log performance metrics to output */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceMetrics();

    /** Start performance capture session */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceCapture(const FString& SessionName);

    /** Stop performance capture session */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceCapture();

    /** Export performance data to CSV */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ExportPerformanceData(const FString& FilePath);

protected:
    /** Current performance metrics */
    UPROPERTY()
    FPerformanceMetrics CurrentMetrics;

    /** Performance targets */
    UPROPERTY(EditAnywhere, Category = "Performance")
    FPerformanceTargets PerformanceTargets;

    /** Optimization settings */
    UPROPERTY(EditAnywhere, Category = "Performance")
    FPerformanceOptimizationSettings OptimizationSettings;

    /** Performance monitoring enabled */
    UPROPERTY()
    bool bPerformanceMonitoringEnabled = true;

    /** Auto optimization enabled */
    UPROPERTY()
    bool bAutoOptimizationEnabled = true;

    /** Performance capture enabled */
    UPROPERTY()
    bool bPerformanceCaptureEnabled = false;

    /** Current capture session name */
    UPROPERTY()
    FString CurrentCaptureSession;

    /** Performance history for averaging */
    UPROPERTY()
    TArray<float> FPSHistory;

    /** Frame time history */
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    /** GPU time history */
    UPROPERTY()
    TArray<float> GPUTimeHistory;

    /** Maximum history entries */
    UPROPERTY()
    int32 MaxHistoryEntries = 300; // 5 seconds at 60fps

    /** Current resolution scale */
    UPROPERTY()
    float CurrentResolutionScale = 1.0f;

    /** Last optimization time */
    UPROPERTY()
    float LastOptimizationTime = 0.0f;

    /** Optimization interval in seconds */
    UPROPERTY()
    float OptimizationInterval = 1.0f;

    /** Performance capture data */
    UPROPERTY()
    TArray<FPerformanceMetrics> CaptureData;

private:
    void UpdatePerformanceMetrics();
    void UpdatePerformanceHistory();
    void ApplyAutomaticOptimizations();
    void AdjustResolutionScale();
    void AdjustLODSettings();
    void AdjustPhysicsSettings();
    void AdjustCullingSettings();
    void AdjustTextureStreamingSettings();
    
    FTimerHandle PerformanceUpdateTimer;
    FTimerHandle OptimizationTimer;
    
    // Platform-specific performance targets
    void SetupPlatformTargets();
    bool IsConsole() const;
    bool IsHighEndPC() const;
    
    // Stat collection helpers
    float GetStatValue(const FName& StatName) const;
    int32 GetStatValueInt(const FName& StatName) const;
};