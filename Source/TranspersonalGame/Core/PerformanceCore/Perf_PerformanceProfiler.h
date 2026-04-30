#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../SharedTypes.h"
#include "Perf_PerformanceProfiler.generated.h"

// Performance metrics structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 0.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        ActiveActors = 0;
        VisibleActors = 0;
    }
};

// Performance warning levels
UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Optimal     UMETA(DisplayName = "Optimal"),
    Good        UMETA(DisplayName = "Good"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical"),
    Unplayable  UMETA(DisplayName = "Unplayable")
};

// Performance budget structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    float MaxGameThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    float MaxRenderThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    float MaxGPUTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    int32 MaxTriangles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    float MaxMemoryUsageMB;

    FPerf_PerformanceBudget()
    {
        TargetFPS = 60.0f;
        MaxFrameTime = 16.67f; // 60 FPS = 16.67ms per frame
        MaxGameThreadTime = 10.0f;
        MaxRenderThreadTime = 12.0f;
        MaxGPUTime = 14.0f;
        MaxDrawCalls = 2000;
        MaxTriangles = 1000000;
        MaxMemoryUsageMB = 4096.0f; // 4GB
    }
};

/**
 * Performance Profiler Component
 * Monitors real-time performance metrics and provides optimization recommendations
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PerformanceProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetPerformanceWarnings() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceBudget(const FPerf_PerformanceBudget& NewBudget);

    // Optimization recommendations
    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetOptimizationRecommendations() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyAutoOptimizations();

protected:
    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_PerformanceBudget PerformanceBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bIsProfilingActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ProfilingInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MetricsHistorySize;

    // Internal tracking
    TArray<float> FPSHistory;
    TArray<float> FrameTimeHistory;
    float LastProfilingTime;
    int32 FrameCount;

private:
    void UpdateMetrics();
    void UpdateFPSHistory(float NewFPS);
    void AnalyzePerformance();
    float CalculateAverageFPS() const;
    EPerf_PerformanceLevel DeterminePerformanceLevel() const;
    void GenerateWarnings() const;
    void LogPerformanceData() const;

    mutable TArray<FString> CachedWarnings;
    mutable TArray<FString> CachedRecommendations;
};

/**
 * Performance Profiler Subsystem
 * Global performance monitoring and optimization system
 */
UCLASS()
class TRANSPERSONALGAME_API UPerf_PerformanceProfilerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    static UPerf_PerformanceProfilerSubsystem* Get(const UObject* WorldContext);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterProfiler(UPerf_PerformanceProfiler* Profiler);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterProfiler(UPerf_PerformanceProfiler* Profiler);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetGlobalMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGlobalPerformanceBudget(const FPerf_PerformanceBudget& Budget);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableGlobalProfiling(bool bEnable);

protected:
    UPROPERTY()
    TArray<UPerf_PerformanceProfiler*> RegisteredProfilers;

    UPROPERTY()
    FPerf_PerformanceBudget GlobalBudget;

    UPROPERTY()
    bool bGlobalProfilingEnabled;
};