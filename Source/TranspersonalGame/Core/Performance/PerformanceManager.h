// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "PerformanceManager.generated.h"

/**
 * @brief Core Performance Management System for Jurassic Survival Game
 * 
 * Manages all performance-related systems to maintain stable framerates:
 * - Target: 60fps on High-end PC (RTX 3070+, 16GB RAM)
 * - Target: 30fps on Console (PS5/Xbox Series X)
 * - Dynamic LOD scaling based on performance budget
 * - Memory management for streaming world
 * - GPU profiling and bottleneck detection
 * 
 * @author Performance Optimizer — Agent #4
 * @version 1.0 — March 2026
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Performance targets for different platforms */
    UENUM(BlueprintType)
    enum class EPerformanceTarget : uint8
    {
        HighEndPC_60fps,    // RTX 3070+, 16GB RAM, SSD
        MidRangePC_60fps,   // GTX 1660, 8GB RAM
        Console_30fps,      // PS5/Xbox Series X
        Console_60fps,      // PS5/Xbox Series X (Performance Mode)
        Potato_30fps        // Minimum spec fallback
    };

    /** Performance budget categories */
    UENUM(BlueprintType)
    enum class EPerformanceBudget : uint8
    {
        GameThread,         // CPU gameplay logic
        RenderThread,       // CPU rendering commands
        GPU,               // GPU rendering
        Memory,            // RAM usage
        VRAM,             // Video memory
        Streaming         // Asset streaming bandwidth
    };

public:
    /**
     * @brief Initialize performance monitoring and targets
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void InitializePerformanceTargets();

    /**
     * @brief Set performance target based on detected hardware
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(EPerformanceTarget Target);

    /**
     * @brief Get current frame time in milliseconds
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime() const;

    /**
     * @brief Get current FPS
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

    /**
     * @brief Check if we're meeting performance targets
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsMeetingPerformanceTargets() const;

    /**
     * @brief Get performance budget usage for specific category
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetBudgetUsage(EPerformanceBudget Budget) const;

    /**
     * @brief Force LOD adjustment based on performance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustLODForPerformance(float PerformanceScale);

    /**
     * @brief Enable/disable expensive visual features based on performance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ScaleVisualQuality(float QualityScale);

    /**
     * @brief Get recommended scalability settings for current hardware
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyRecommendedScalabilitySettings();

protected:
    /** Current performance target */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerformanceTarget CurrentTarget = EPerformanceTarget::HighEndPC_60fps;

    /** Target frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60fps

    /** Performance budgets in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Budgets")
    TMap<EPerformanceBudget, float> PerformanceBudgets;

    /** Moving average frame times for smoothing */
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    /** Maximum history samples for averaging */
    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxFrameHistorySamples = 60;

    /** Performance monitoring enabled */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bEnablePerformanceMonitoring = true;

    /** Auto-adjust quality based on performance */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bAutoAdjustQuality = true;

    /** Minimum acceptable frame rate before quality reduction */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MinAcceptableFPS = 50.0f;

private:
    /** Initialize performance budgets based on target */
    void InitializeBudgets();

    /** Update frame time tracking */
    void UpdateFrameTimeTracking();

    /** Detect hardware capabilities */
    void DetectHardwareCapabilities();

    /** Apply performance optimizations */
    void ApplyPerformanceOptimizations();

    /** Console commands for performance debugging */
    void RegisterConsoleCommands();

    /** Timer handle for performance monitoring */
    FTimerHandle PerformanceMonitoringTimer;

    /** Cached engine reference */
    UPROPERTY()
    UEngine* CachedEngine;

    /** Performance statistics */
    mutable float CachedFrameTime = 0.0f;
    mutable float CachedFPS = 0.0f;
    mutable float LastUpdateTime = 0.0f;
};