#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Perf_PerformanceProfiler.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"),
    High        UMETA(DisplayName = "High Performance"),
    Ultra       UMETA(DisplayName = "Ultra Performance")
};

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
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float ViewDistanceScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 ShadowResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 TextureStreamingPoolSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float StaticMeshLODBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float SkeletalMeshLODBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableOcclusion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableDistanceCulling;

    FPerf_OptimizationSettings()
    {
        ViewDistanceScale = 1.0f;
        ShadowResolution = 2048;
        TextureStreamingPoolSize = 2048;
        StaticMeshLODBias = 0.0f;
        SkeletalMeshLODBias = 0.0f;
        bEnableOcclusion = true;
        bEnableDistanceCulling = true;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PerformanceProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profiler")
    bool bEnableProfiling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profiler")
    float ProfilingInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profiler")
    EPerf_PerformanceLevel TargetPerformanceLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Profiler")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profiler")
    FPerf_OptimizationSettings OptimizationSettings;

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void ApplyOptimizationSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void SetPerformanceLevel(EPerf_PerformanceLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void AutoOptimize();

private:
    float ProfilingTimer;
    TArray<float> FPSSamples;
    int32 SampleIndex;
    static const int32 MaxSamples = 60;

    void UpdateMetrics();
    void CalculateAverages();
    void ApplyLowSettings();
    void ApplyMediumSettings();
    void ApplyHighSettings();
    void ApplyUltraSettings();
};