#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Perf_PerformanceProfiler.generated.h"

UENUM(BlueprintType)
enum class EPerf_ProfilerMetric : uint8
{
    FrameTime,
    RenderTime,
    GameThreadTime,
    RHIThreadTime,
    GPUTime,
    DrawCalls,
    Triangles,
    MemoryUsage
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
    float RenderTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        MinFPS = 60.0f;
        MaxFPS = 60.0f;
        FrameTime = 16.67f;
        RenderTime = 8.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        MemoryUsageMB = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PerformanceProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float NewTargetFPS);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinimumAcceptableFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bIsProfilingActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ProfilingInterval = 1.0f;

private:
    TArray<float> FPSSamples;
    float LastProfilingTime = 0.0f;
    int32 FrameCount = 0;
    float TotalFrameTime = 0.0f;

    void UpdateMetrics(float DeltaTime);
    void CollectRenderStats();
    void CollectMemoryStats();
};