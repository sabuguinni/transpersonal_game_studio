// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "Stats/Stats.h"
#include "HAL/IConsoleManager.h"
#include "Misc/App.h"
#include "PerformanceOptimizer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPerformanceOptimizer, Log, All);

DECLARE_STATS_GROUP(TEXT("TranspersonalGame Performance"), STATGROUP_TranspersonalPerformance, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Physics Tick Time"), STAT_PhysicsTickTime, STATGROUP_TranspersonalPerformance);
DECLARE_CYCLE_STAT(TEXT("AI Tick Time"), STAT_AITickTime, STATGROUP_TranspersonalPerformance);
DECLARE_CYCLE_STAT(TEXT("Rendering Time"), STAT_RenderingTime, STATGROUP_TranspersonalPerformance);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Physics Bodies"), STAT_ActivePhysicsBodies, STATGROUP_TranspersonalPerformance);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active AI Agents"), STAT_ActiveAIAgents, STATGROUP_TranspersonalPerformance);
DECLARE_DWORD_COUNTER_STAT(TEXT("Visible Dinosaurs"), STAT_VisibleDinosaurs, STATGROUP_TranspersonalPerformance);

/**
 * Performance Quality Level
 */
UENUM(BlueprintType)
enum class EPerformanceQualityLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"), 
    High        UMETA(DisplayName = "High"),
    Epic        UMETA(DisplayName = "Epic"),
    Custom      UMETA(DisplayName = "Custom")
};

/**
 * Performance Bottleneck Types
 */
UENUM(BlueprintType)
enum class EPerformanceBottleneck : uint8
{
    None            UMETA(DisplayName = "None"),
    CPU_GameThread  UMETA(DisplayName = "CPU Game Thread"),
    CPU_RenderThread UMETA(DisplayName = "CPU Render Thread"),
    GPU             UMETA(DisplayName = "GPU"),
    Memory          UMETA(DisplayName = "Memory"),
    Physics         UMETA(DisplayName = "Physics"),
    Collision       UMETA(DisplayName = "Collision"),
    AI              UMETA(DisplayName = "AI"),
    Streaming       UMETA(DisplayName = "Streaming")
};

/**
 * Optimization Categories
 */
UENUM(BlueprintType)
enum class EOptimizationCategory : uint8
{
    Rendering       UMETA(DisplayName = "Rendering"),
    Physics         UMETA(DisplayName = "Physics"),
    Collision       UMETA(DisplayName = "Collision"),
    LOD             UMETA(DisplayName = "Level of Detail"),
    Shadows         UMETA(DisplayName = "Shadows"),
    PostProcessing  UMETA(DisplayName = "Post Processing"),
    Particles       UMETA(DisplayName = "Particles"),
    Audio           UMETA(DisplayName = "Audio"),
    Streaming       UMETA(DisplayName = "Streaming"),
    Memory          UMETA(DisplayName = "Memory"),
    Nanite          UMETA(DisplayName = "Nanite"),
    Lumen           UMETA(DisplayName = "Lumen")
};

/**
 * Platform Performance Profile
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPlatformPerformanceProfile
{
    GENERATED_BODY()

    /** Target FPS for this platform */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TargetFPS = 60;

    /** Physics tick time budget in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsTickBudget = 4.17f;

    /** Maximum active physics bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActivePhysicsBodies = 500;

    /** Maximum collision checks per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxCollisionChecksPerFrame = 10000;

    /** Maximum AI agents to simulate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxAIAgents = 100;

    /** Maximum visible dinosaurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleDinosaurs = 50;

    /** LOD distance multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceMultiplier = 1.0f;

    /** Memory budget in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MemoryBudgetMB = 8192;

    /** Nanite enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bNaniteEnabled = true;

    /** Lumen enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bLumenEnabled = true;

    FPlatformPerformanceProfile()
    {
        TargetFPS = 60;
        PhysicsTickBudget = 4.17f;
        MaxActivePhysicsBodies = 500;
        MaxCollisionChecksPerFrame = 10000;
        MaxAIAgents = 100;
        MaxVisibleDinosaurs = 50;
        LODDistanceMultiplier = 1.0f;
        MemoryBudgetMB = 8192;
        bNaniteEnabled = true;
        bLumenEnabled = true;
    }
};

/**
 * Performance Metrics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    /** Current FPS */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CurrentFPS = 0.0f;

    /** Frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float FrameTimeMS = 0.0f;

    /** Game thread time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float GameThreadTimeMS = 0.0f;

    /** Render thread time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float RenderThreadTimeMS = 0.0f;

    /** GPU time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float GPUTimeMS = 0.0f;

    /** Physics time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float PhysicsTimeMS = 0.0f;

    /** AI time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AITimeMS = 0.0f;

    /** Memory usage in MB */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB = 0.0f;

    /** Active physics bodies count */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActivePhysicsBodies = 0;

    /** Active AI agents count */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActiveAIAgents = 0;

    /** Visible dinosaurs count */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 VisibleDinosaurs = 0;

    /** Draw calls count */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DrawCalls = 0;

    /** Triangles rendered */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TrianglesRendered = 0;

    /** Current bottleneck */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    EPerformanceBottleneck Bottleneck = EPerformanceBottleneck::None;

    /** Physics objects count */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 PhysicsObjects = 0;

    /** Collision tests count */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CollisionTests = 0;
};

/**
 * Performance Optimizer
 * Manages performance optimization for the Transpersonal Game
 * Targets: 60fps on PC, 30fps on Console with large-scale dinosaur simulation
 */
UCLASS()
class TRANSPERSONALGAME_API UPerformanceOptimizer : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceOptimizer();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Get the performance optimizer instance */
    UFUNCTION(BlueprintPure, Category = "Performance")
    static UPerformanceOptimizer* Get(const UObject* WorldContext);

    /** Initialize performance optimization */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void InitializePerformanceOptimization();

    /** Set performance quality level */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceQualityLevel(EPerformanceQualityLevel QualityLevel);

    /** Get current performance quality level */
    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerformanceQualityLevel GetPerformanceQualityLevel() const { return CurrentQualityLevel; }

    /** Set platform performance profile */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPlatformPerformanceProfile(const FPlatformPerformanceProfile& Profile);

    /** Get platform performance profile */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPlatformPerformanceProfile GetPlatformPerformanceProfile() const { return PlatformProfile; }

    /** Get current performance metrics */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceMetrics GetPerformanceMetrics() const { return CurrentMetrics; }

    /** Enable/disable automatic performance scaling */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAutoPerformanceScaling(bool bEnabled);

    /** Check if performance target is being met */
    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    /** Get performance bottleneck information */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FString GetPerformanceBottleneck() const;

    /** Force performance optimization pass */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForcePerformanceOptimization();

    /** Set LOD bias for performance */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODBias(float Bias);

    /** Set physics quality level */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPhysicsQuality(int32 QualityLevel);

    /** Set AI quality level */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAIQuality(int32 QualityLevel);

    /** Set rendering quality level */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetRenderingQuality(int32 QualityLevel);

    /** Enable performance profiling */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceProfiling(bool bEnabled);

    /** Start performance capture */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceCapture(const FString& CaptureName);

    /** Stop performance capture */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceCapture();

    /** Optimize Nanite settings */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeNaniteSettings();

    /** Optimize Lumen settings */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLumenSettings();

    /** Set console performance mode */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetConsolePerformanceMode(bool bConsoleMode);

protected:
    /** Current performance quality level */
    UPROPERTY()
    EPerformanceQualityLevel CurrentQualityLevel = EPerformanceQualityLevel::High;

    /** Platform performance profile */
    UPROPERTY()
    FPlatformPerformanceProfile PlatformProfile;

    /** Current performance metrics */
    UPROPERTY()
    FPerformanceMetrics CurrentMetrics;

    /** Auto performance scaling enabled */
    UPROPERTY()
    bool bAutoPerformanceScaling = true;

    /** Performance profiling enabled */
    UPROPERTY()
    bool bPerformanceProfilingEnabled = false;

    /** Performance capture active */
    UPROPERTY()
    bool bPerformanceCaptureActive = false;

    /** Current capture name */
    UPROPERTY()
    FString CurrentCaptureName;

    /** Performance update frequency */
    UPROPERTY()
    float PerformanceUpdateFrequency = 1.0f;

    /** Last performance update time */
    UPROPERTY()
    float LastPerformanceUpdateTime = 0.0f;

    /** Performance history for averaging */
    UPROPERTY()
    TArray<float> FPSHistory;

    /** Maximum history samples */
    UPROPERTY()
    int32 MaxHistorySamples = 60;

    /** Console mode enabled */
    UPROPERTY()
    bool bConsoleMode = false;

    /** Optimization categories enabled */
    UPROPERTY()
    TMap<EOptimizationCategory, bool> EnabledOptimizations;

    /** Performance monitoring timer */
    FTimerHandle PerformanceMonitoringTimer;

    /** Monitoring update interval */
    float MonitoringUpdateInterval = 1.0f;

    /** Performance monitoring enabled */
    bool bEnablePerformanceMonitoring = true;

    /** Frame time warning threshold */
    float FrameTimeWarningThreshold = 20.0f;

    /** Memory warning threshold in MB */
    float MemoryWarningThreshold = 6144.0f;

    /** Memory critical threshold in MB */
    float MemoryCriticalThreshold = 7168.0f;

    /** Optimizations active */
    bool bOptimizationsActive = false;

    /** Current platform */
    FString CurrentPlatform;

    /** Platform performance targets */
    TMap<FString, FPlatformPerformanceProfile> PlatformTargets;

    /** Metrics history */
    TArray<FPerformanceMetrics> MetricsHistory;

    /** Maximum metrics history size */
    int32 MetricsHistorySize = 300;

    /** Current target FPS */
    float CurrentTargetFPS = 60.0f;

    /** Optimization application count */
    int32 OptimizationApplicationCount = 0;

    /** Last optimization time */
    double LastOptimizationTime = 0.0;

    /** Total optimization time */
    double TotalOptimizationTime = 0.0;

private:
    void UpdatePerformanceMetrics();
    void ApplyPerformanceOptimizations();
    void DetectPerformanceBottlenecks();
    void ApplyQualitySettings();
    void ConfigureConsoleCommands();
    void StartPerformanceMonitoring();
    void StopPerformanceMonitoring();
    void ApplyDynamicOptimizations();
    void AnalyzePerformanceBottlenecks();
    void ApplyFrameRateOptimizations();
    void OptimizePhysicsSimulation();
    void OptimizeCollisionDetection();
    void OptimizeLODSystem();
    void OptimizeRenderingQuality();
    void OptimizeShadowQuality();
    void OptimizePostProcessing();
    void OptimizeMemoryUsage();
    void OptimizeAssetStreaming();
    void OptimizeRagdollPhysics();
    void ApplyPlatformOptimizations();
    void SetTargetFrameRate(float TargetFPS);
    void SetOptimizationCategory(EOptimizationCategory Category, bool bEnabled);
};