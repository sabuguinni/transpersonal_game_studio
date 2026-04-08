// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "PerformanceManager.generated.h"

/**
 * @brief Core Performance Management System for Transpersonal Game
 * 
 * Ensures consistent frame rates across platforms:
 * - 60fps on high-end PC
 * - 30fps on next-gen consoles
 * - Adaptive quality scaling based on performance metrics
 * - Real-time optimization of physics, rendering, and AI systems
 * 
 * Performance Philosophy:
 * "Performance is not the enemy of quality visual — it's the condition 
 * for quality visual to be experienced. An effect that no one can see 
 * because the game is stuttering doesn't exist."
 * 
 * @author Performance Optimizer — Agent #4
 * @version 1.0 — March 2026
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerformanceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /**
     * @brief Initialize performance monitoring and optimization systems
     * 
     * Sets up frame rate targets, performance budgets, and adaptive scaling
     * based on detected hardware platform
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Management")
    void InitializePerformanceSystem();

    /**
     * @brief Get current performance metrics
     * 
     * @return Current frame rate, frame time, and performance budget usage
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Management")
    FPerformanceMetrics GetCurrentPerformanceMetrics() const;

    /**
     * @brief Set target frame rate for platform
     * 
     * @param TargetFPS Target frames per second (60 for PC, 30 for console)
     * @param bEnableAdaptiveScaling Whether to enable automatic quality scaling
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Management")
    void SetTargetFrameRate(int32 TargetFPS, bool bEnableAdaptiveScaling = true);

    /**
     * @brief Force performance optimization pass
     * 
     * Immediately applies optimizations to maintain target frame rate
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Management")
    void ForceOptimizationPass();

    /**
     * @brief Register performance-critical system for monitoring
     * 
     * @param SystemName Name of the system to monitor
     * @param BudgetMs Time budget in milliseconds for this system
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Management")
    void RegisterCriticalSystem(const FString& SystemName, float BudgetMs);

    /**
     * @brief Report system performance for frame budget tracking
     * 
     * @param SystemName Name of the reporting system
     * @param ExecutionTimeMs Time spent this frame in milliseconds
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Management")
    void ReportSystemPerformance(const FString& SystemName, float ExecutionTimeMs);

    /**
     * @brief Get recommended quality settings for current performance
     * 
     * @return Quality settings optimized for current hardware performance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Management")
    FQualitySettings GetRecommendedQualitySettings() const;

protected:
    /** Performance metrics structure */
    USTRUCT(BlueprintType)
    struct FPerformanceMetrics
    {
        GENERATED_BODY()

        /** Current frames per second */
        UPROPERTY(BlueprintReadOnly)
        float CurrentFPS = 0.0f;

        /** Current frame time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float FrameTimeMs = 0.0f;

        /** Game thread time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float GameThreadMs = 0.0f;

        /** Render thread time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float RenderThreadMs = 0.0f;

        /** GPU time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float GPUTimeMs = 0.0f;

        /** Physics time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float PhysicsTimeMs = 0.0f;

        /** AI time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float AITimeMs = 0.0f;

        /** Memory usage in MB */
        UPROPERTY(BlueprintReadOnly)
        float MemoryUsageMB = 0.0f;

        /** Draw calls count */
        UPROPERTY(BlueprintReadOnly)
        int32 DrawCalls = 0;

        /** Triangle count */
        UPROPERTY(BlueprintReadOnly)
        int32 TriangleCount = 0;

        /** Performance health (0-100) */
        UPROPERTY(BlueprintReadOnly)
        float PerformanceHealth = 100.0f;
    };

    /** Quality settings structure */
    USTRUCT(BlueprintType)
    struct FQualitySettings
    {
        GENERATED_BODY()

        /** Screen percentage (50-100) */
        UPROPERTY(BlueprintReadOnly)
        float ScreenPercentage = 100.0f;

        /** Shadow quality (0-3) */
        UPROPERTY(BlueprintReadOnly)
        int32 ShadowQuality = 3;

        /** Post-process quality (0-3) */
        UPROPERTY(BlueprintReadOnly)
        int32 PostProcessQuality = 3;

        /** Texture quality (0-3) */
        UPROPERTY(BlueprintReadOnly)
        int32 TextureQuality = 3;

        /** Effects quality (0-3) */
        UPROPERTY(BlueprintReadOnly)
        int32 EffectsQuality = 3;

        /** View distance scale (0.1-1.0) */
        UPROPERTY(BlueprintReadOnly)
        float ViewDistanceScale = 1.0f;

        /** Physics LOD bias */
        UPROPERTY(BlueprintReadOnly)
        int32 PhysicsLODBias = 0;

        /** AI update frequency scale */
        UPROPERTY(BlueprintReadOnly)
        float AIUpdateScale = 1.0f;
    };

    /** Platform performance profiles */
    UENUM(BlueprintType)
    enum class EPlatformProfile : uint8
    {
        HighEndPC       UMETA(DisplayName = "High-End PC"),
        MidRangePC      UMETA(DisplayName = "Mid-Range PC"),
        LowEndPC        UMETA(DisplayName = "Low-End PC"),
        NextGenConsole  UMETA(DisplayName = "Next-Gen Console"),
        Mobile          UMETA(DisplayName = "Mobile Device")
    };

    /** Target frame rate for current platform */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    int32 TargetFrameRate = 60;

    /** Target frame time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float TargetFrameTimeMs = 16.67f; // 60fps = 16.67ms

    /** Frame time budget for game thread */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Budgets")
    float GameThreadBudgetMs = 10.0f;

    /** Frame time budget for render thread */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Budgets")
    float RenderThreadBudgetMs = 12.0f;

    /** Frame time budget for GPU */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Budgets")
    float GPUBudgetMs = 14.0f;

    /** Frame time budget for physics */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Budgets")
    float PhysicsBudgetMs = 3.0f;

    /** Frame time budget for AI */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Budgets")
    float AIBudgetMs = 2.0f;

    /** Enable adaptive quality scaling */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive Scaling")
    bool bEnableAdaptiveScaling = true;

    /** Frames below target before scaling down */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive Scaling")
    int32 FramesToleranceBeforeScaling = 10;

    /** Minimum screen percentage when scaling */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive Scaling")
    float MinScreenPercentage = 50.0f;

    /** Performance monitoring frequency in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monitoring")
    float MonitoringFrequency = 1.0f;

    /** Enable performance logging */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monitoring")
    bool bEnablePerformanceLogging = true;

private:
    /** Detect current platform and set appropriate profile */
    void DetectPlatformProfile();
    
    /** Update performance metrics */
    void UpdatePerformanceMetrics();
    
    /** Check if performance optimization is needed */
    bool ShouldOptimizePerformance() const;
    
    /** Apply performance optimizations */
    void ApplyPerformanceOptimizations();
    
    /** Scale quality settings based on performance */
    void ScaleQualitySettings(float ScaleFactor);
    
    /** Log performance data to file */
    void LogPerformanceData();
    
    /** Get engine performance stats */
    void GatherEngineStats();

    /** Current platform profile */
    EPlatformProfile CurrentPlatformProfile = EPlatformProfile::HighEndPC;

    /** Current performance metrics */
    FPerformanceMetrics CurrentMetrics;

    /** Current quality settings */
    FQualitySettings CurrentQualitySettings;

    /** Registered systems and their budgets */
    TMap<FString, float> SystemBudgets;

    /** System performance tracking */
    TMap<FString, float> SystemPerformance;

    /** Frame rate history for averaging */
    TArray<float> FrameRateHistory;

    /** Performance monitoring timer */
    float MonitoringTimer = 0.0f;

    /** Consecutive frames below target */
    int32 FramesBelowTarget = 0;

    /** Consecutive frames above target */
    int32 FramesAboveTarget = 0;

    /** Last optimization time */
    float LastOptimizationTime = 0.0f;

    /** Performance log file handle */
    TSharedPtr<class FArchive> PerformanceLogFile;
};