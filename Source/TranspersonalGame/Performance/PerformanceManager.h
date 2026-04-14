#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "../SharedTypes.h"
#include "PerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low (30 FPS)"),
    Medium      UMETA(DisplayName = "Medium (45 FPS)"),
    High        UMETA(DisplayName = "High (60 FPS)"),
    Ultra       UMETA(DisplayName = "Ultra (120+ FPS)")
};

UENUM(BlueprintType)
enum class EPerf_OptimizationState : uint8
{
    Analyzing   UMETA(DisplayName = "Analyzing"),
    Optimizing  UMETA(DisplayName = "Optimizing"),
    Stable      UMETA(DisplayName = "Stable"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AvailableMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 TriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "World")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "World")
    int32 ComponentCount = 0;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        UsedMemoryMB = 0.0f;
        AvailableMemoryMB = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        ActorCount = 0;
        ComponentCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MinAcceptableFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxLODLevel = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    float ViewDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MaxMemoryUsageMB = 8192.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bAutoOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bDynamicLOD = true;

    FPerf_OptimizationSettings()
    {
        TargetFPS = 60.0f;
        MinAcceptableFPS = 30.0f;
        LODDistanceScale = 1.0f;
        MaxLODLevel = 3;
        ViewDistanceScale = 1.0f;
        MaxDrawCalls = 2000;
        MaxMemoryUsageMB = 8192.0f;
        CullingDistance = 5000.0f;
        bAutoOptimization = true;
        bDynamicLOD = true;
    }
};

/**
 * Performance Manager - Monitors and optimizes game performance in real-time
 * Ensures 60fps on PC and 30fps on console through dynamic optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_OptimizationState GetOptimizationState() const;

    // Optimization Controls
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetOptimizationSettings(const FPerf_OptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    FPerf_OptimizationSettings GetOptimizationSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ForceOptimizationPass();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetPerformanceLevel(EPerf_PerformanceLevel Level);

    // Dynamic LOD Management
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateDynamicLOD();

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void SetLODDistanceScale(float Scale);

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void OptimizeMemoryUsage();

    // Performance Profiling
    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StartProfiling(const FString& ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void SavePerformanceReport(const FString& ReportName);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceStateChanged, EPerf_OptimizationState, NewState);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPerformanceStateChanged OnPerformanceStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFPSDropDetected, float, CurrentFPS);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnFPSDropDetected OnFPSDropDetected;

protected:
    // Internal monitoring
    void UpdatePerformanceMetrics();
    void AnalyzePerformanceBottlenecks();
    void ApplyOptimizations();
    void CheckPerformanceThresholds();

    // Optimization methods
    void OptimizeLODLevels();
    void OptimizeRenderingSettings();
    void OptimizeCullingDistance();
    void OptimizeMemorySettings();

private:
    UPROPERTY()
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY()
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY()
    EPerf_OptimizationState CurrentState;

    UPROPERTY()
    EPerf_PerformanceLevel CurrentLevel;

    // Monitoring state
    bool bIsMonitoring;
    bool bIsProfiling;
    FString CurrentProfileName;
    
    // Timing
    FDateTime LastUpdateTime;
    float UpdateInterval;
    
    // Performance history
    TArray<float> FPSHistory;
    TArray<float> FrameTimeHistory;
    int32 MaxHistorySize;
    
    // Optimization timers
    float LastOptimizationTime;
    float OptimizationCooldown;
    
    // Performance thresholds
    float CriticalFPSThreshold;
    float LowFPSThreshold;
    int32 ConsecutiveLowFrames;
    int32 MaxConsecutiveLowFrames;
};