// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "RHI.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"
#include "GPUPerformanceMonitor.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGPUPerformance, Log, All);

/**
 * @brief GPU Performance Monitor for Transpersonal Game Studio
 * 
 * Monitors GPU performance metrics and enforces rendering budgets:
 * - GPU frame time tracking
 * - VRAM usage monitoring
 * - Draw call optimization
 * - Texture streaming performance
 * - Nanite/Lumen performance tracking
 * 
 * Performance Targets:
 * - PC: 60fps (16.67ms GPU budget)
 * - Console: 30fps (33.33ms GPU budget)
 * 
 * @author Performance Optimizer — Agent #4
 * @version 1.0 — March 2026
 */
UCLASS()
class TRANSPERSONALGAME_API UGPUPerformanceMonitor : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Get the GPU performance monitor instance */
    UFUNCTION(BlueprintPure, Category = "GPU Performance")
    static UGPUPerformanceMonitor* Get(const UObject* WorldContext);

    /** Initialize GPU monitoring */
    UFUNCTION(BlueprintCallable, Category = "GPU Performance")
    void InitializeGPUMonitoring();

    /** Get current GPU frame time in milliseconds */
    UFUNCTION(BlueprintPure, Category = "GPU Performance")
    float GetGPUFrameTimeMS() const;

    /** Get current VRAM usage in MB */
    UFUNCTION(BlueprintPure, Category = "GPU Performance")
    float GetVRAMUsageMB() const;

    /** Get current draw call count */
    UFUNCTION(BlueprintPure, Category = "GPU Performance")
    int32 GetDrawCallCount() const;

    /** Get current triangle count */
    UFUNCTION(BlueprintPure, Category = "GPU Performance")
    int32 GetTriangleCount() const;

    /** Check if GPU is within performance budget */
    UFUNCTION(BlueprintPure, Category = "GPU Performance")
    bool IsGPUWithinBudget() const;

    /** Get GPU performance analysis */
    UFUNCTION(BlueprintCallable, Category = "GPU Performance")
    FGPUPerformanceAnalysis AnalyzeGPUPerformance();

    /** Force GPU optimization (emergency scaling) */
    UFUNCTION(BlueprintCallable, Category = "GPU Performance")
    void ForceGPUOptimization();

    /** Enable/disable GPU monitoring */
    UFUNCTION(BlueprintCallable, Category = "GPU Performance")
    void SetGPUMonitoring(bool bEnabled);

    /** Get Nanite performance metrics */
    UFUNCTION(BlueprintPure, Category = "GPU Performance")
    FNanitePerformanceMetrics GetNaniteMetrics() const;

    /** Get Lumen performance metrics */
    UFUNCTION(BlueprintPure, Category = "GPU Performance")
    FLumenPerformanceMetrics GetLumenMetrics() const;

protected:
    /** GPU performance analysis structure */
    USTRUCT(BlueprintType)
    struct FGPUPerformanceAnalysis
    {
        GENERATED_BODY()

        /** GPU frame time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float GPUFrameTimeMS = 0.0f;

        /** VRAM usage in MB */
        UPROPERTY(BlueprintReadOnly)
        float VRAMUsageMB = 0.0f;

        /** Draw call count */
        UPROPERTY(BlueprintReadOnly)
        int32 DrawCalls = 0;

        /** Triangle count */
        UPROPERTY(BlueprintReadOnly)
        int32 Triangles = 0;

        /** GPU budget usage percentage */
        UPROPERTY(BlueprintReadOnly)
        float BudgetUsagePercent = 0.0f;

        /** Performance bottlenecks */
        UPROPERTY(BlueprintReadOnly)
        TArray<FString> Bottlenecks;

        /** Optimization recommendations */
        UPROPERTY(BlueprintReadOnly)
        TArray<FString> Recommendations;

        /** Emergency optimization required */
        UPROPERTY(BlueprintReadOnly)
        bool bEmergencyOptimizationRequired = false;
    };

    /** Nanite performance metrics */
    USTRUCT(BlueprintType)
    struct FNanitePerformanceMetrics
    {
        GENERATED_BODY()

        /** Nanite triangles rendered */
        UPROPERTY(BlueprintReadOnly)
        int32 NaniteTriangles = 0;

        /** Nanite clusters rendered */
        UPROPERTY(BlueprintReadOnly)
        int32 NaniteClusters = 0;

        /** Nanite GPU time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float NaniteGPUTimeMS = 0.0f;

        /** Nanite memory usage in MB */
        UPROPERTY(BlueprintReadOnly)
        float NaniteMemoryMB = 0.0f;

        /** Nanite culling efficiency */
        UPROPERTY(BlueprintReadOnly)
        float CullingEfficiency = 0.0f;
    };

    /** Lumen performance metrics */
    USTRUCT(BlueprintType)
    struct FLumenPerformanceMetrics
    {
        GENERATED_BODY()

        /** Lumen global illumination time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float LumenGITimeMS = 0.0f;

        /** Lumen reflections time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float LumenReflectionsTimeMS = 0.0f;

        /** Lumen surface cache memory in MB */
        UPROPERTY(BlueprintReadOnly)
        float SurfaceCacheMemoryMB = 0.0f;

        /** Lumen ray count */
        UPROPERTY(BlueprintReadOnly)
        int32 LumenRayCount = 0;

        /** Hardware ray tracing enabled */
        UPROPERTY(BlueprintReadOnly)
        bool bHardwareRayTracingEnabled = false;
    };

    /** GPU monitoring enabled */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GPU Performance")
    bool bGPUMonitoringEnabled = true;

    /** GPU budget in milliseconds (auto-set based on platform) */
    UPROPERTY(BlueprintReadOnly, Category = "GPU Performance")
    float GPUBudgetMS = 16.67f; // 60fps default

    /** Emergency optimization threshold (percentage over budget) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GPU Performance", meta = (ClampMin = "10.0", ClampMax = "50.0"))
    float EmergencyThreshold = 25.0f;

    /** VRAM warning threshold in MB */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GPU Performance")
    float VRAMWarningThresholdMB = 6000.0f; // 6GB

    /** Draw call warning threshold */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GPU Performance")
    int32 DrawCallWarningThreshold = 5000;

    /** Triangle warning threshold */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GPU Performance")
    int32 TriangleWarningThreshold = 10000000; // 10M triangles

private:
    /** Update GPU performance metrics */
    void UpdateGPUMetrics();
    
    /** Calculate GPU budget usage */
    float CalculateGPUBudgetUsage() const;
    
    /** Apply GPU optimizations */
    void ApplyGPUOptimizations();
    
    /** Get current GPU statistics */
    void GatherGPUStatistics();
    
    /** Detect platform GPU budget */
    void DetectPlatformGPUBudget();
    
    /** Log GPU performance warnings */
    void LogGPUPerformanceWarnings(const FGPUPerformanceAnalysis& Analysis);

    /** GPU monitoring timer */
    FTimerHandle GPUMonitoringTimer;
    
    /** Current GPU metrics */
    mutable FGPUPerformanceAnalysis CurrentGPUMetrics;
    
    /** Nanite metrics cache */
    mutable FNanitePerformanceMetrics NaniteMetrics;
    
    /** Lumen metrics cache */
    mutable FLumenPerformanceMetrics LumenMetrics;
    
    /** GPU performance history for trend analysis */
    TArray<float> GPUFrameTimeHistory;
    
    /** VRAM usage history */
    TArray<float> VRAMUsageHistory;
    
    /** Draw call history */
    TArray<int32> DrawCallHistory;
};

/**
 * @brief GPU Budget Enforcer Component
 * 
 * Attaches to rendering-heavy actors to enforce GPU budgets
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UGPUBudgetEnforcerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGPUBudgetEnforcerComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /** Check if GPU is within budget */
    UFUNCTION(BlueprintPure, Category = "GPU Budget")
    bool IsGPUWithinBudget() const;

    /** Get current GPU budget usage percentage */
    UFUNCTION(BlueprintPure, Category = "GPU Budget")
    float GetGPUBudgetUsagePercent() const;

    /** Force GPU budget compliance (reduce rendering quality) */
    UFUNCTION(BlueprintCallable, Category = "GPU Budget")
    void ForceGPUBudgetCompliance();

    /** Set LOD bias for this actor */
    UFUNCTION(BlueprintCallable, Category = "GPU Budget")
    void SetLODBias(float Bias);

    /** Set texture quality scale for this actor */
    UFUNCTION(BlueprintCallable, Category = "GPU Budget")
    void SetTextureQualityScale(float Scale);

protected:
    /** Auto-enforce GPU budget when exceeded */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GPU Budget")
    bool bAutoEnforceGPUBudget = true;

    /** GPU budget violation threshold before enforcement */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GPU Budget", meta = (ClampMin = "5.0", ClampMax = "50.0"))
    float GPUViolationThreshold = 20.0f;

    /** LOD bias steps when over budget */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GPU Budget")
    TArray<float> LODBiasSteps = {0.0f, 1.0f, 2.0f, 3.0f};

    /** Texture quality steps when over budget */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GPU Budget")
    TArray<float> TextureQualitySteps = {1.0f, 0.75f, 0.5f, 0.25f};

    /** Shadow quality steps when over budget */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GPU Budget")
    TArray<float> ShadowQualitySteps = {1.0f, 0.75f, 0.5f, 0.0f};

private:
    /** Current quality level index */
    int32 CurrentQualityIndex = 0;
    
    /** GPU performance monitor reference */
    UPROPERTY()
    TObjectPtr<UGPUPerformanceMonitor> GPUPerformanceMonitor;
    
    /** Original LOD bias */
    float OriginalLODBias = 0.0f;
    
    /** Original texture quality */
    float OriginalTextureQuality = 1.0f;
    
    /** Original shadow quality */
    float OriginalShadowQuality = 1.0f;
};