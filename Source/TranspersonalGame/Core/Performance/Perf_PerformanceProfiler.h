#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"
#include "Perf_PerformanceProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameStats
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

    FPerf_FrameStats()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 0.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedPhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedVirtualMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PeakUsedPhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PeakUsedVirtualMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 TexturePoolSizeMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 StreamingPoolSizeMB;

    FPerf_MemoryStats()
    {
        UsedPhysicalMemoryMB = 0.0f;
        UsedVirtualMemoryMB = 0.0f;
        PeakUsedPhysicalMemoryMB = 0.0f;
        PeakUsedVirtualMemoryMB = 0.0f;
        TexturePoolSizeMB = 0;
        StreamingPoolSizeMB = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RenderStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 StaticMeshes;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 SkeletalMeshes;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 Lights;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 Shadows;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 Particles;

    FPerf_RenderStats()
    {
        DrawCalls = 0;
        Triangles = 0;
        StaticMeshes = 0;
        SkeletalMeshes = 0;
        Lights = 0;
        Shadows = 0;
        Particles = 0;
    }
};

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
    void ResetStats();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    FPerf_FrameStats GetFrameStats() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    FPerf_MemoryStats GetMemoryStats() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    FPerf_RenderStats GetRenderStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsTargetFrameRateAchieved() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float NewTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPerformanceScore() const;

    // Performance optimization suggestions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetOptimizationSuggestions() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void TriggerGarbageCollection();

    // Performance budget management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetRenderBudget(int32 MaxDrawCalls, int32 MaxTriangles);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMemoryBudget(float MaxPhysicalMemoryMB, float MaxVirtualMemoryMB);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinRenderBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinMemoryBudget() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float ProfilingUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bAutoOptimizationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bLogPerformanceWarnings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    int32 StatsSampleSize;

    // Performance budgets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budgets")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budgets")
    int32 MaxTriangles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budgets")
    float MaxPhysicalMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budgets")
    float MaxVirtualMemoryMB;

private:
    bool bIsProfiling;
    float ProfilingTimer;
    
    // Frame stats tracking
    TArray<float> FrameTimeHistory;
    float TotalFrameTime;
    float MinFrameTime;
    float MaxFrameTime;
    int32 FrameCount;

    // Memory tracking
    float PeakPhysicalMemory;
    float PeakVirtualMemory;

    // Internal methods
    void UpdateFrameStats(float DeltaTime);
    void UpdateMemoryStats();
    void UpdateRenderStats();
    void CheckPerformanceThresholds();
    void ApplyAutoOptimizations();
    
    FPerf_FrameStats CachedFrameStats;
    FPerf_MemoryStats CachedMemoryStats;
    FPerf_RenderStats CachedRenderStats;
};