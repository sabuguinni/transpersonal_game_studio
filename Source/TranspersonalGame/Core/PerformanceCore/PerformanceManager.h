// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RHI.h"
#include "PerformanceManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPerformanceManager, Log, All);

/** Performance target levels for different platforms */
UENUM(BlueprintType)
enum class EPerformanceTarget : uint8
{
    PC_60FPS        UMETA(DisplayName = "PC 60 FPS"),
    PC_120FPS       UMETA(DisplayName = "PC 120 FPS"),
    Console_30FPS   UMETA(DisplayName = "Console 30 FPS"),
    Console_60FPS   UMETA(DisplayName = "Console 60 FPS"),
    Mobile_30FPS    UMETA(DisplayName = "Mobile 30 FPS"),
    Custom          UMETA(DisplayName = "Custom Target")
};

/** Performance monitoring categories */
UENUM(BlueprintType)
enum class EPerformanceCategory : uint8
{
    CPU_Game        UMETA(DisplayName = "CPU Game Thread"),
    CPU_Render      UMETA(DisplayName = "CPU Render Thread"),
    GPU_Total       UMETA(DisplayName = "GPU Total"),
    GPU_Basepass    UMETA(DisplayName = "GPU Base Pass"),
    GPU_Lighting    UMETA(DisplayName = "GPU Lighting"),
    GPU_Shadows     UMETA(DisplayName = "GPU Shadows"),
    GPU_PostProcess UMETA(DisplayName = "GPU Post Process"),
    Memory_Total    UMETA(DisplayName = "Memory Total"),
    Memory_Textures UMETA(DisplayName = "Memory Textures"),
    Memory_Meshes   UMETA(DisplayName = "Memory Meshes"),
    Physics         UMETA(DisplayName = "Physics Simulation"),
    AI              UMETA(DisplayName = "AI Systems"),
    Audio           UMETA(DisplayName = "Audio Processing"),
    Streaming       UMETA(DisplayName = "Asset Streaming")
};

/** Real-time performance metrics */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    /** Frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMs = 0.0f;

    /** Frames per second */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FPS = 0.0f;

    /** Game thread time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadMs = 0.0f;

    /** Render thread time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadMs = 0.0f;

    /** GPU frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMs = 0.0f;

    /** Total memory usage in MB */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    /** VRAM usage in MB */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float VRAMUsageMB = 0.0f;

    /** Draw calls count */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    /** Triangle count */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles = 0;

    /** Active physics bodies */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsBodies = 0;

    /** AI agents count */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 AIAgents = 0;

    /** Timestamp when metrics were captured */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FDateTime Timestamp;
};

/** Performance budget settings */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceBudget
{
    GENERATED_BODY()

    /** Target frame time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float TargetFrameTimeMs = 16.67f; // 60 FPS

    /** Maximum allowed game thread time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float MaxGameThreadMs = 12.0f;

    /** Maximum allowed render thread time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float MaxRenderThreadMs = 12.0f;

    /** Maximum allowed GPU time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float MaxGPUTimeMs = 14.0f;

    /** Maximum memory usage in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float MaxMemoryMB = 8192.0f;

    /** Maximum VRAM usage in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float MaxVRAMMB = 6144.0f;

    /** Maximum draw calls per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    int32 MaxDrawCalls = 5000;

    /** Maximum triangles per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    int32 MaxTriangles = 10000000;
};

/**
 * @brief Performance Manager - Central performance monitoring and optimization system
 * 
 * Monitors real-time performance across all game systems and automatically
 * adjusts quality settings to maintain target framerates. Provides detailed
 * profiling data and performance budgets for different platforms.
 * 
 * Key Features:
 * - Real-time performance monitoring (CPU, GPU, Memory)
 * - Automatic quality adjustment based on performance targets
 * - Performance budgets and alerts
 * - Detailed profiling and metrics collection
 * - Platform-specific optimization profiles
 * - Performance regression detection
 * 
 * @author Performance Optimizer — Agent #4
 * @version 1.0 — March 2026
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Get the performance manager instance */
    UFUNCTION(BlueprintPure, Category = "Performance", CallInEditor = true)
    static UPerformanceManager* Get(const UObject* WorldContext);

    /** Set performance target for current platform */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(EPerformanceTarget Target);

    /** Set custom performance budget */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCustomBudget(const FPerformanceBudget& Budget);

    /** Get current performance metrics */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceMetrics GetCurrentMetrics() const;

    /** Get average performance over time window */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceMetrics GetAverageMetrics(float TimeWindowSeconds = 5.0f) const;

    /** Check if performance is within budget */
    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsPerformanceWithinBudget() const;

    /** Get performance budget violations */
    UFUNCTION(BlueprintPure, Category = "Performance")
    TArray<FString> GetBudgetViolations() const;

    /** Enable/disable automatic quality adjustment */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAutoQualityAdjustment(bool bEnabled);

    /** Force immediate quality adjustment */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceQualityAdjustment();

    /** Start performance profiling session */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfilingSession(const FString& SessionName);

    /** Stop current profiling session */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfilingSession();

    /** Export performance data to file */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ExportPerformanceData(const FString& FilePath) const;

    /** Get performance category timing */
    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetCategoryTiming(EPerformanceCategory Category) const;

    /** Set performance monitoring frequency */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMonitoringFrequency(float FrequencyHz);

    /** Enable/disable performance alerts */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceAlertsEnabled(bool bEnabled);

    /** Get current scalability settings */
    UFUNCTION(BlueprintPure, Category = "Performance")
    void GetScalabilitySettings(int32& ViewDistance, int32& AntiAliasing, int32& PostProcess, 
                               int32& Shadows, int32& GlobalIllumination, int32& Reflections, 
                               int32& Textures, int32& Effects) const;

    /** Apply scalability preset */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyScalabilityPreset(int32 QualityLevel);

protected:
    /** Current performance target */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerformanceTarget CurrentTarget = EPerformanceTarget::PC_60FPS;

    /** Current performance budget */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerformanceBudget CurrentBudget;

    /** Enable automatic quality adjustment */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoQualityAdjustment = true;

    /** Performance monitoring frequency in Hz */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "60.0"))
    float MonitoringFrequency = 10.0f;

    /** Enable performance alerts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceAlertsEnabled = true;

    /** Performance history window in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float PerformanceHistoryWindow = 10.0f;

    /** Quality adjustment sensitivity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float QualityAdjustmentSensitivity = 1.0f;

private:
    /** Current performance metrics */
    FPerformanceMetrics CurrentMetrics;

    /** Performance history for averaging */
    TArray<FPerformanceMetrics> PerformanceHistory;

    /** Maximum history samples to keep */
    int32 MaxHistorySamples = 600; // 60 seconds at 10Hz

    /** Profiling session data */
    FString CurrentSessionName;
    bool bProfilingActive = false;
    TArray<FPerformanceMetrics> SessionData;

    /** Timer handles */
    FTimerHandle MonitoringTimer;
    FTimerHandle QualityAdjustmentTimer;

    /** Performance thresholds for quality adjustment */
    float QualityUpgradeThreshold = 0.8f;   // Upgrade quality if under 80% budget
    float QualityDowngradeThreshold = 1.1f; // Downgrade quality if over 110% budget

    /** Initialize performance budgets for different targets */
    void InitializePerformanceBudgets();

    /** Update performance metrics */
    void UpdatePerformanceMetrics();

    /** Collect system performance data */
    void CollectSystemMetrics();

    /** Check for budget violations and adjust quality */
    void CheckBudgetAndAdjustQuality();

    /** Adjust scalability settings based on performance */
    void AdjustScalabilitySettings(bool bIncrease);

    /** Get platform-specific budget */
    FPerformanceBudget GetPlatformBudget(EPerformanceTarget Target) const;

    /** Log performance alert */
    void LogPerformanceAlert(const FString& Message);

    /** Calculate average metrics over time window */
    FPerformanceMetrics CalculateAverageMetrics(float TimeWindow) const;

    /** Cleanup old performance history */
    void CleanupPerformanceHistory();
};