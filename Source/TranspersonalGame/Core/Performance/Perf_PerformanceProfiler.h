#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Perf_PerformanceProfiler.generated.h"

USTRUCT(BlueprintType)
struct FPerf_ProfileData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float UsedPhysicalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float UsedVirtualMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AvailablePhysicalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TextureMemoryMB = 0.0f;

    FPerf_ProfileData()
    {
        Timestamp = 0.0f;
        CurrentFPS = 60.0f;
        FrameTime = 16.67f;
        AverageFrameTime = 16.67f;
        GameThreadTime = 8.0f;
        RenderThreadTime = 6.0f;
        GPUFrameTime = 10.0f;
        DrawCalls = 0;
        Triangles = 0;
        UsedPhysicalMemoryMB = 0.0f;
        UsedVirtualMemoryMB = 0.0f;
        AvailablePhysicalMemoryMB = 0.0f;
        TextureMemoryMB = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerf_ProfilingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bEnableCPUProfiling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bEnableGPUProfiling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bEnableMemoryProfiling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bEnableDrawCallProfiling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    float ProfilingInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    int32 MaxSampleHistory = 300;

    FPerf_ProfilingSettings()
    {
        bEnableCPUProfiling = true;
        bEnableGPUProfiling = true;
        bEnableMemoryProfiling = true;
        bEnableDrawCallProfiling = true;
        ProfilingInterval = 0.1f;
        MaxSampleHistory = 300;
    }
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceTargets
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float MinimumFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float MaxFrameTime = 16.67f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float MaxGPUTime = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float MaxMemoryUsageMB = 4096.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    int32 MaxDrawCalls = 2000;

    FPerf_PerformanceTargets()
    {
        TargetFPS = 60.0f;
        MinimumFPS = 30.0f;
        MaxFrameTime = 16.67f;
        MaxGPUTime = 12.0f;
        MaxMemoryUsageMB = 4096.0f;
        MaxDrawCalls = 2000;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PerformanceProfiler : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Profiling control
    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    bool IsProfilingActive() const { return bIsProfilingActive; }

    // Data access
    UFUNCTION(BlueprintCallable, Category = "Profiling")
    FPerf_ProfileData GetCurrentProfileData() const;

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    TArray<FPerf_ProfileData> GetProfileHistory() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void SetProfilingSettings(const FPerf_ProfilingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void SetPerformanceTargets(const FPerf_PerformanceTargets& NewTargets);

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void SaveReportToFile();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void ClearProfileHistory();

    // Performance optimization trigger
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void TriggerPerformanceOptimization();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    FPerf_ProfilingSettings ProfilingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    FPerf_PerformanceTargets PerformanceTargets;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    FPerf_ProfileData CurrentProfileData;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    TArray<FPerf_ProfileData> ProfileHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bAutoStartProfiling = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bAutoOptimize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bSaveReportToFile = false;

private:
    bool bIsProfilingActive = false;
    float LastProfilingTime = 0.0f;
    TArray<float> FrameTimeHistory;

    void UpdateProfilingData(float DeltaTime);
    void UpdateCPUMetrics(float DeltaTime);
    void UpdateGPUMetrics();
    void UpdateMemoryMetrics();
    void UpdateRenderingMetrics();
    void CheckPerformanceThresholds();
};