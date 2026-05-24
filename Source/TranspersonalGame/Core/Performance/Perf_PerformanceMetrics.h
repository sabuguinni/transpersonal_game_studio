#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Perf_PerformanceMetrics.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameMetrics
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

    FPerf_FrameMetrics()
        : CurrentFPS(0.0f)
        , AverageFPS(0.0f)
        , MinFPS(0.0f)
        , MaxFPS(0.0f)
        , FrameTime(0.0f)
        , GameThreadTime(0.0f)
        , RenderThreadTime(0.0f)
        , GPUTime(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float UsedPhysicalMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float UsedVirtualMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PeakUsedPhysicalMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PeakUsedVirtualMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AvailablePhysicalMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AvailableVirtualMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TexturePoolSizeMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 UsedTexturePoolMB;

    FPerf_MemoryMetrics()
        : UsedPhysicalMB(0.0f)
        , UsedVirtualMB(0.0f)
        , PeakUsedPhysicalMB(0.0f)
        , PeakUsedVirtualMB(0.0f)
        , AvailablePhysicalMB(0.0f)
        , AvailableVirtualMB(0.0f)
        , TexturePoolSizeMB(0)
        , UsedTexturePoolMB(0)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RenderMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Vertices;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TextureMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MeshMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LightCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ShadowCastingLights;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float ViewDistance;

    FPerf_RenderMetrics()
        : DrawCalls(0)
        , Triangles(0)
        , Vertices(0)
        , TextureMemoryMB(0)
        , MeshMemoryMB(0)
        , LightCount(0)
        , ShadowCastingLights(0)
        , ViewDistance(0.0f)
    {}
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_PerformanceMetrics : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PerformanceMetrics();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Metrics collection
    UFUNCTION(BlueprintCallable, Category = "Performance Metrics")
    void StartMetricsCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance Metrics")
    void StopMetricsCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance Metrics")
    void UpdateMetrics();

    // Metrics access
    UFUNCTION(BlueprintPure, Category = "Performance Metrics")
    FPerf_FrameMetrics GetFrameMetrics() const { return FrameMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance Metrics")
    FPerf_MemoryMetrics GetMemoryMetrics() const { return MemoryMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance Metrics")
    FPerf_RenderMetrics GetRenderMetrics() const { return RenderMetrics; }

    // Performance warnings
    UFUNCTION(BlueprintPure, Category = "Performance Metrics")
    bool IsPerformanceWarning() const;

    UFUNCTION(BlueprintPure, Category = "Performance Metrics")
    bool IsPerformanceCritical() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Metrics")
    void LogCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Metrics")
    void ResetPeakValues();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    float WarningFPSThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    float CriticalFPSThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    float WarningMemoryThresholdMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Metrics")
    float CriticalMemoryThresholdMB;

protected:
    void UpdateFrameMetrics();
    void UpdateMemoryMetrics();
    void UpdateRenderMetrics();

private:
    UPROPERTY()
    FPerf_FrameMetrics FrameMetrics;

    UPROPERTY()
    FPerf_MemoryMetrics MemoryMetrics;

    UPROPERTY()
    FPerf_RenderMetrics RenderMetrics;

    FTimerHandle MetricsTimerHandle;
    bool bCollectingMetrics;

    // Frame tracking
    TArray<float> FrameTimeHistory;
    float FrameTimeSum;
    int32 FrameCount;
    static const int32 MaxFrameHistory = 120; // 2 seconds at 60fps
};