#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Perf_PerformanceSystemManager.generated.h"

// Performance monitoring data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        TargetFPS = 60.0f;
        GPUTime = 0.0f;
        CPUTime = 0.0f;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
        VisibleActors = 0;
    }
};

// Performance optimization levels
UENUM(BlueprintType)
enum class EPerf_OptimizationLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Performance UMETA(DisplayName = "Performance Mode")
};

// LOD management data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistanceMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxLODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableDistanceCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullingDistance;

    FPerf_LODSettings()
    {
        LODDistanceMultiplier = 1.0f;
        MaxLODLevel = 3;
        bEnableDistanceCulling = true;
        CullingDistance = 10000.0f;
    }
};

/**
 * Performance System Manager
 * Manages frame rate optimization, LOD systems, and performance monitoring
 * Ensures 60fps on PC and 30fps on console
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PerformanceSystemManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Performance monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_OptimizationLevel OptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationCheckInterval;

    // LOD management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableLODSystem;

    // Memory management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryThresholdMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bAutoGarbageCollection;

    // GPU optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU")
    bool bEnableGPUCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU")
    bool bOptimizeLumen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPU")
    bool bOptimizeNanite;

private:
    // Internal timers
    float OptimizationTimer;
    float MetricsUpdateTimer;
    
    // Performance tracking
    TArray<float> RecentFrameTimes;
    int32 MaxFrameTimesSamples;

public:
    // Performance monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationLevel(EPerf_OptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float NewTargetFPS);

    // LOD management functions
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateLODSettings();

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void ApplyLODToActors();

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void SetLODDistanceMultiplier(float Multiplier);

    // Memory management functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void CheckMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    float GetMemoryUsageMB() const;

    // GPU optimization functions
    UFUNCTION(BlueprintCallable, Category = "GPU")
    void ApplyGPUOptimizations();

    UFUNCTION(BlueprintCallable, Category = "GPU")
    void OptimizeLumenSettings();

    UFUNCTION(BlueprintCallable, Category = "GPU")
    void OptimizeNaniteSettings();

    // Auto-optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void PerformAutoOptimization();

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void EnablePerformanceStats(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPerformanceReport();

    UFUNCTION(CallInEditor, Category = "Debug")
    void TestPerformanceOptimization();

protected:
    // Internal optimization functions
    void ApplyOptimizationLevel();
    void UpdateFrameTimeHistory(float DeltaTime);
    float CalculateAverageFrameTime() const;
    bool IsPerformanceBelowTarget() const;
    void AdjustQualitySettings();
    void OptimizeRenderingSettings();
    void OptimizePhysicsSettings();
    void OptimizeAudioSettings();
};