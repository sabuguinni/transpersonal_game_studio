// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "PerformanceManager.generated.h"

/**
 * @brief Performance Manager for Jurassic Survival Game
 * 
 * Manages performance targets and optimization for:
 * - 60fps on high-end PC (RTX 3070+, 16GB RAM)
 * - 30fps on console (PS5/Xbox Series X)
 * - Dynamic quality scaling based on performance
 * - Memory management for large open world
 * - Physics optimization for massive dinosaur herds
 * 
 * @author Performance Optimizer — Agent #4
 * @version 1.0 — March 2026
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
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
     * @brief Initialize performance monitoring and targets
     * 
     * Sets up frame time tracking, memory monitoring, and quality scaling
     * based on detected hardware capabilities
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void InitializePerformanceSystem();

    /**
     * @brief Get current performance metrics
     * 
     * @return Struct containing FPS, frame time, memory usage, and quality level
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    struct FPerformanceMetrics GetCurrentMetrics() const;

    /**
     * @brief Force performance level adjustment
     * 
     * @param NewQualityLevel 0=Low, 1=Medium, 2=High, 3=Epic
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityLevel(int32 NewQualityLevel);

    /**
     * @brief Enable/disable dynamic quality scaling
     * 
     * @param bEnable Whether to automatically adjust quality based on performance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetDynamicQualityEnabled(bool bEnable);

    /**
     * @brief Get recommended quality level for current hardware
     * 
     * @return Quality level (0-3) based on GPU benchmark and available memory
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    int32 GetRecommendedQualityLevel() const;

    /**
     * @brief Apply Jurassic-specific performance optimizations
     * 
     * Optimizes settings for:
     * - Large dinosaur herds (Mass AI culling)
     * - Dense vegetation (foliage LOD)
     * - Physics simulation (Chaos optimization)
     * - Memory streaming (texture and mesh LOD)
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyJurassicOptimizations();

protected:
    /** Target frame rates for different platforms */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float TargetFPS_PC = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float TargetFPS_Console = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float TargetFPS_Mobile = 30.0f;

    /** Frame time budgets in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float FrameTimeBudget_PC = 16.67f; // 60fps

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float FrameTimeBudget_Console = 33.33f; // 30fps

    /** Memory usage targets in MB */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float MaxMemoryUsage_PC = 12000.0f; // 12GB

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float MaxMemoryUsage_Console = 8000.0f; // 8GB

    /** Dynamic quality scaling settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quality Scaling")
    bool bDynamicQualityEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quality Scaling")
    float QualityAdjustmentThreshold = 5.0f; // ms over budget

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quality Scaling")
    float QualityCheckInterval = 2.0f; // seconds

    /** Dinosaur-specific performance settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jurassic Optimization")
    int32 MaxVisibleDinosaurs = 50;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jurassic Optimization")
    float DinosaurCullDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jurassic Optimization")
    int32 MaxPhysicsSubsteps = 4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jurassic Optimization")
    float VegetationDensityScale = 1.0f;

private:
    /** Performance monitoring */
    void UpdatePerformanceMetrics();
    void CheckFrameTimeTarget();
    void CheckMemoryUsage();
    void AdjustQualityIfNeeded();

    /** Platform detection */
    void DetectPlatformCapabilities();
    bool IsHighEndPC() const;
    bool IsConsole() const;

    /** Quality level management */
    void ApplyQualityLevel(int32 QualityLevel);
    void SetRenderingQuality(int32 Level);
    void SetPhysicsQuality(int32 Level);
    void SetAudioQuality(int32 Level);

    /** Jurassic-specific optimizations */
    void OptimizeDinosaurRendering();
    void OptimizeVegetationSystem();
    void OptimizePhysicsSimulation();
    void OptimizeMemoryStreaming();

    /** Performance tracking variables */
    float CurrentFPS;
    float CurrentFrameTime;
    float CurrentMemoryUsage;
    int32 CurrentQualityLevel;
    
    /** Timing for quality adjustments */
    float LastQualityCheckTime;
    TArray<float> RecentFrameTimes;
    
    /** Platform capabilities */
    bool bIsHighEndPC;
    bool bIsConsole;
    bool bSupportsDLSS;
    bool bSupportsRayTracing;
    
    /** Console variables for runtime adjustment */
    TAutoConsoleVariable<int32> CVarQualityLevel;
    TAutoConsoleVariable<bool> CVarDynamicQuality;
    TAutoConsoleVariable<float> CVarTargetFPS;
};

/**
 * @brief Performance metrics structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    /** Current frames per second */
    UPROPERTY(BlueprintReadOnly)
    float FPS = 0.0f;

    /** Current frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly)
    float FrameTime = 0.0f;

    /** Current memory usage in MB */
    UPROPERTY(BlueprintReadOnly)
    float MemoryUsage = 0.0f;

    /** Current quality level (0-3) */
    UPROPERTY(BlueprintReadOnly)
    int32 QualityLevel = 2;

    /** Whether target FPS is being met */
    UPROPERTY(BlueprintReadOnly)
    bool bMeetingTargetFPS = true;

    /** Number of visible dinosaurs */
    UPROPERTY(BlueprintReadOnly)
    int32 VisibleDinosaurs = 0;

    /** Current physics simulation load */
    UPROPERTY(BlueprintReadOnly)
    float PhysicsLoad = 0.0f;

    /** GPU utilization percentage */
    UPROPERTY(BlueprintReadOnly)
    float GPUUtilization = 0.0f;

    FPerformanceMetrics()
    {
        FPS = 0.0f;
        FrameTime = 0.0f;
        MemoryUsage = 0.0f;
        QualityLevel = 2;
        bMeetingTargetFPS = true;
        VisibleDinosaurs = 0;
        PhysicsLoad = 0.0f;
        GPUUtilization = 0.0f;
    }
};