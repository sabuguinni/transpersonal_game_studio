// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "PerformanceOptimizer.generated.h"

/**
 * @brief Performance Optimizer for Transpersonal Game Studio
 * 
 * Ensures 60fps on high-end PC and 30fps on console for the prehistoric survival game.
 * Implements dynamic performance scaling, LOD management, and frame budget optimization.
 * 
 * Core Responsibilities:
 * - Monitor frame time and GPU/CPU performance
 * - Dynamic quality scaling based on performance targets
 * - Physics optimization and collision culling
 * - Memory management and streaming optimization
 * - Platform-specific performance tuning
 * 
 * Performance Targets:
 * - PC High-End: 60fps (16.67ms frame budget)
 * - Console (PS5/Xbox Series X): 30fps (33.33ms frame budget)
 * - Console (Xbox Series S): 30fps with reduced quality
 * 
 * @author Performance Optimizer — Agent #4
 * @version 1.0 — March 2026
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceOptimizer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceOptimizer();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Performance monitoring and optimization */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    /** Get current performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceMetrics GetCurrentPerformanceMetrics() const;

    /** Apply performance optimizations based on current metrics */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyDynamicOptimizations();

    /** Set target frame rate for the platform */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float TargetFPS);

    /** Enable/disable specific optimization categories */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationCategory(EOptimizationCategory Category, bool bEnabled);

protected:
    /** Performance metrics structure */
    USTRUCT(BlueprintType)
    struct FPerformanceMetrics
    {
        GENERATED_BODY()

        /** Current frame rate */
        UPROPERTY(BlueprintReadOnly)
        float CurrentFPS = 0.0f;

        /** Frame time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float FrameTimeMS = 0.0f;

        /** Game thread time */
        UPROPERTY(BlueprintReadOnly)
        float GameThreadTimeMS = 0.0f;

        /** Render thread time */
        UPROPERTY(BlueprintReadOnly)
        float RenderThreadTimeMS = 0.0f;

        /** GPU time */
        UPROPERTY(BlueprintReadOnly)
        float GPUTimeMS = 0.0f;

        /** Memory usage in MB */
        UPROPERTY(BlueprintReadOnly)
        float MemoryUsageMB = 0.0f;

        /** Draw calls per frame */
        UPROPERTY(BlueprintReadOnly)
        int32 DrawCalls = 0;

        /** Triangles rendered per frame */
        UPROPERTY(BlueprintReadOnly)
        int32 TrianglesRendered = 0;

        /** Physics objects simulated */
        UPROPERTY(BlueprintReadOnly)
        int32 PhysicsObjects = 0;

        /** Collision tests per frame */
        UPROPERTY(BlueprintReadOnly)
        int32 CollisionTests = 0;

        /** Performance bottleneck type */
        UPROPERTY(BlueprintReadOnly)
        EPerformanceBottleneck Bottleneck = EPerformanceBottleneck::None;
    };

    /** Performance bottleneck types */
    UENUM(BlueprintType)
    enum class EPerformanceBottleneck : uint8
    {
        None            UMETA(DisplayName = "No Bottleneck"),
        CPU_GameThread  UMETA(DisplayName = "CPU Game Thread"),
        CPU_RenderThread UMETA(DisplayName = "CPU Render Thread"),
        GPU             UMETA(DisplayName = "GPU Bound"),
        Memory          UMETA(DisplayName = "Memory Bound"),
        Physics         UMETA(DisplayName = "Physics Bound"),
        Collision       UMETA(DisplayName = "Collision Bound")
    };

    /** Optimization categories */
    UENUM(BlueprintType)
    enum class EOptimizationCategory : uint8
    {
        Rendering       UMETA(DisplayName = "Rendering Quality"),
        Physics         UMETA(DisplayName = "Physics Simulation"),
        Collision       UMETA(DisplayName = "Collision Detection"),
        LOD             UMETA(DisplayName = "Level of Detail"),
        Shadows         UMETA(DisplayName = "Shadow Quality"),
        PostProcessing  UMETA(DisplayName = "Post Processing"),
        Particles       UMETA(DisplayName = "Particle Systems"),
        Audio           UMETA(DisplayName = "Audio Processing"),
        Streaming       UMETA(DisplayName = "Asset Streaming"),
        Memory          UMETA(DisplayName = "Memory Management")
    };

    /** Performance quality levels */
    UENUM(BlueprintType)
    enum class EPerformanceQuality : uint8
    {
        Low         UMETA(DisplayName = "Low Quality"),
        Medium      UMETA(DisplayName = "Medium Quality"),
        High        UMETA(DisplayName = "High Quality"),
        Epic        UMETA(DisplayName = "Epic Quality"),
        Cinematic   UMETA(DisplayName = "Cinematic Quality")
    };

    /** Platform-specific performance targets */
    USTRUCT(BlueprintType)
    struct FPlatformPerformanceTarget
    {
        GENERATED_BODY()

        /** Target frame rate */
        UPROPERTY(EditAnywhere, BlueprintReadOnly)
        float TargetFPS = 60.0f;

        /** Maximum frame time budget in milliseconds */
        UPROPERTY(EditAnywhere, BlueprintReadOnly)
        float MaxFrameTimeMS = 16.67f;

        /** Target quality level */
        UPROPERTY(EditAnywhere, BlueprintReadOnly)
        EPerformanceQuality TargetQuality = EPerformanceQuality::High;

        /** Memory budget in MB */
        UPROPERTY(EditAnywhere, BlueprintReadOnly)
        float MemoryBudgetMB = 8192.0f;

        /** Maximum draw calls per frame */
        UPROPERTY(EditAnywhere, BlueprintReadOnly)
        int32 MaxDrawCalls = 5000;

        /** Maximum physics objects */
        UPROPERTY(EditAnywhere, BlueprintReadOnly)
        int32 MaxPhysicsObjects = 1000;
    };

private:
    /** Performance monitoring */
    void UpdatePerformanceMetrics();
    void AnalyzePerformanceBottlenecks();
    void ApplyFrameRateOptimizations();

    /** Rendering optimizations */
    void OptimizeRenderingQuality();
    void OptimizeLODSystem();
    void OptimizeShadowQuality();
    void OptimizePostProcessing();

    /** Physics optimizations */
    void OptimizePhysicsSimulation();
    void OptimizeCollisionDetection();
    void OptimizeRagdollPhysics();

    /** Memory optimizations */
    void OptimizeMemoryUsage();
    void OptimizeAssetStreaming();
    void OptimizeTextureStreaming();

    /** Platform-specific optimizations */
    void ApplyPlatformOptimizations();
    void ConfigureConsoleOptimizations();
    void ConfigurePCOptimizations();

    /** Console variable management */
    void SetConsoleVariable(const FString& VariableName, float Value);
    void SetConsoleVariable(const FString& VariableName, int32 Value);
    float GetConsoleVariableFloat(const FString& VariableName);
    int32 GetConsoleVariableInt(const FString& VariableName);

    /** Performance targets for different platforms */
    UPROPERTY(EditAnywhere, Category = "Performance Targets")
    TMap<FString, FPlatformPerformanceTarget> PlatformTargets;

    /** Current performance metrics */
    UPROPERTY()
    FPerformanceMetrics CurrentMetrics;

    /** Enabled optimization categories */
    UPROPERTY(EditAnywhere, Category = "Optimization")
    TMap<EOptimizationCategory, bool> EnabledOptimizations;

    /** Performance monitoring settings */
    UPROPERTY(EditAnywhere, Category = "Monitoring")
    bool bEnablePerformanceMonitoring = true;

    UPROPERTY(EditAnywhere, Category = "Monitoring")
    float MonitoringUpdateInterval = 0.1f; // Update every 100ms

    UPROPERTY(EditAnywhere, Category = "Monitoring")
    int32 MetricsHistorySize = 300; // 30 seconds at 10fps monitoring

    /** Performance history for trend analysis */
    UPROPERTY()
    TArray<FPerformanceMetrics> MetricsHistory;

    /** Optimization thresholds */
    UPROPERTY(EditAnywhere, Category = "Thresholds")
    float FrameTimeWarningThreshold = 20.0f; // ms

    UPROPERTY(EditAnywhere, Category = "Thresholds")
    float FrameTimeCriticalThreshold = 40.0f; // ms

    UPROPERTY(EditAnywhere, Category = "Thresholds")
    float MemoryWarningThreshold = 7000.0f; // MB

    UPROPERTY(EditAnywhere, Category = "Thresholds")
    float MemoryCriticalThreshold = 8000.0f; // MB

    /** Timer handle for performance monitoring */
    FTimerHandle PerformanceMonitoringTimer;

    /** Console variables for optimization */
    TMap<FString, IConsoleVariable*> OptimizationCVars;

    /** Current platform name */
    FString CurrentPlatform;

    /** Performance optimization state */
    bool bOptimizationsActive = false;
    EPerformanceQuality CurrentQualityLevel = EPerformanceQuality::High;
    float CurrentTargetFPS = 60.0f;

    /** Statistics tracking */
    int32 OptimizationApplicationCount = 0;
    float TotalOptimizationTime = 0.0f;
    float LastOptimizationTime = 0.0f;

public:
    /** Delegate for performance events */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerformanceEvent, EPerformanceBottleneck, Bottleneck, const FPerformanceMetrics&, Metrics);

    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FOnPerformanceEvent OnPerformanceWarning;

    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FOnPerformanceEvent OnPerformanceCritical;

    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FOnPerformanceEvent OnPerformanceImproved;
};