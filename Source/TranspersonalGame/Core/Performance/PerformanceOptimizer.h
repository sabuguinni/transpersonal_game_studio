// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "PerformanceOptimizer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPerformanceOptimizer, Log, All);

/**
 * Performance optimization levels
 */
UENUM(BlueprintType)
enum class EPerformanceOptimizationLevel : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Conservative    UMETA(DisplayName = "Conservative"),
    Balanced        UMETA(DisplayName = "Balanced"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Maximum         UMETA(DisplayName = "Maximum")
};

/**
 * Performance target platforms
 */
UENUM(BlueprintType)
enum class EPerformanceTargetPlatform : uint8
{
    HighEndPC       UMETA(DisplayName = "High-End PC"),
    MidRangePC      UMETA(DisplayName = "Mid-Range PC"),
    Console         UMETA(DisplayName = "Console"),
    Mobile          UMETA(DisplayName = "Mobile"),
    VR              UMETA(DisplayName = "VR")
};

/**
 * Performance metrics structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LODBias;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float ShadowQuality;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TextureQuality;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float EffectsQuality;

    FPerformanceMetrics()
        : FrameTime(0.0f)
        , GameThreadTime(0.0f)
        , RenderThreadTime(0.0f)
        , GPUTime(0.0f)
        , PhysicsTime(0.0f)
        , DrawCalls(0)
        , Triangles(0)
        , MemoryUsageMB(0.0f)
        , ActivePhysicsObjects(0)
        , LODBias(0.0f)
        , ShadowQuality(1.0f)
        , TextureQuality(1.0f)
        , EffectsQuality(1.0f)
    {}
};

/**
 * Performance optimization settings
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    EPerformanceOptimizationLevel OptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    EPerformanceTargetPlatform TargetPlatform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bOptimizePhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 MaxPhysicsObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float PhysicsSubstepDeltaTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bOptimizeRendering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    float DynamicLODBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bUseDynamicResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    float MinResolutionScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bOptimizeCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CullDistanceMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bUseOcclusionCulling;

    FPerformanceOptimizationSettings()
        : OptimizationLevel(EPerformanceOptimizationLevel::Balanced)
        , TargetPlatform(EPerformanceTargetPlatform::HighEndPC)
        , TargetFrameRate(60.0f)
        , MaxFrameTime(16.67f)
        , MaxMemoryUsageMB(4096.0f)
        , bOptimizePhysics(true)
        , MaxPhysicsObjects(1000)
        , PhysicsSubstepDeltaTime(0.016f)
        , bOptimizeRendering(true)
        , DynamicLODBias(0.0f)
        , bUseDynamicResolution(true)
        , MinResolutionScale(0.5f)
        , bOptimizeCulling(true)
        , CullDistanceMultiplier(1.0f)
        , bUseOcclusionCulling(true)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceOptimized, const FPerformanceMetrics&, Metrics);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerformanceThresholdExceeded, const FString&, MetricName, float, Value);

/**
 * Advanced Performance Optimizer System
 * Provides real-time performance monitoring and automatic optimization
 * Targets 60fps on PC and 30fps on console platforms
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceOptimizer : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceOptimizer();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // World Subsystem interface
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FPerformanceOptimizationSettings OptimizationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAutomaticOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float OptimizationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableDetailedLogging;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPerformanceOptimized OnPerformanceOptimized;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPerformanceThresholdExceeded OnPerformanceThresholdExceeded;

    // Public Methods
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyOptimizationPreset(EPerformanceOptimizationLevel Level, EPerformanceTargetPlatform Platform);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRenderingPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCullingPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetToDefaultSettings();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StartPerformanceProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StopPerformanceProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void GeneratePerformanceReport();

    // Advanced optimization methods
    UFUNCTION(BlueprintCallable, Category = "Advanced")
    void OptimizeForTargetFramerate(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Advanced")
    void EnableDynamicQualityScaling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Advanced")
    void SetPerformanceBudgets(float CPUBudgetMS, float GPUBudgetMS, float MemoryBudgetMB);

private:
    // Internal state
    bool bOptimizationActive;
    float LastOptimizationTime;
    FPerformanceMetrics CurrentMetrics;
    TArray<FPerformanceMetrics> MetricsHistory;
    
    // Optimization timers
    FTimerHandle OptimizationTimerHandle;
    FTimerHandle ProfilingTimerHandle;
    
    // Performance budgets
    float CPUBudgetMS;
    float GPUBudgetMS;
    float MemoryBudgetMB;
    
    // Dynamic quality scaling
    bool bDynamicQualityEnabled;
    float CurrentQualityScale;
    float QualityScaleTarget;
    
    // Internal methods
    void UpdatePerformanceMetrics();
    void AnalyzePerformance();
    void ApplyOptimizations();
    void AdjustQualitySettings();
    void OptimizePhysicsSettings();
    void OptimizeRenderingSettings();
    void OptimizeCullingSettings();
    void MonitorMemoryUsage();
    
    // Platform-specific optimizations
    void ApplyPCOptimizations();
    void ApplyConsoleOptimizations();
    void ApplyMobileOptimizations();
    void ApplyVROptimizations();
    
    // Metrics collection
    float GetGameThreadTime() const;
    float GetRenderThreadTime() const;
    float GetGPUTime() const;
    float GetPhysicsTime() const;
    int32 GetDrawCallCount() const;
    int32 GetTriangleCount() const;
    float GetMemoryUsage() const;
    int32 GetActivePhysicsObjectCount() const;
    
    // Console variable management
    void SetConsoleVariable(const FString& VariableName, float Value);
    void SetConsoleVariable(const FString& VariableName, int32 Value);
    float GetConsoleVariableFloat(const FString& VariableName) const;
    int32 GetConsoleVariableInt(const FString& VariableName) const;
    
    // Logging and reporting
    void LogPerformanceMetrics(const FPerformanceMetrics& Metrics);
    void LogOptimizationAction(const FString& Action, const FString& Details);
    void SavePerformanceReport(const FString& FilePath);
};