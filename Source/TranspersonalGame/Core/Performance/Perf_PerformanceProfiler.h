#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Perf_PerformanceProfiler.generated.h"

UENUM(BlueprintType)
enum class EPerf_ProfilerCategory : uint8
{
    Rendering = 0,
    Physics = 1,
    AI = 2,
    Audio = 3,
    Networking = 4,
    Gameplay = 5,
    Memory = 6,
    IO = 7
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    FPerf_PerformanceMetrics()
    {
        FrameTime = 0.0f;
        FPS = 0.0f;
        CPUTime = 0.0f;
        GPUTime = 0.0f;
        MemoryUsage = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
    }
};

USTRUCT(BlueprintType)
struct FPerf_ProfilerSample
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Profiler")
    FString SampleName;

    UPROPERTY(BlueprintReadOnly, Category = "Profiler")
    EPerf_ProfilerCategory Category;

    UPROPERTY(BlueprintReadOnly, Category = "Profiler")
    float StartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Profiler")
    float EndTime;

    UPROPERTY(BlueprintReadOnly, Category = "Profiler")
    float Duration;

    FPerf_ProfilerSample()
    {
        SampleName = TEXT("");
        Category = EPerf_ProfilerCategory::Gameplay;
        StartTime = 0.0f;
        EndTime = 0.0f;
        Duration = 0.0f;
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
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void BeginSample(const FString& SampleName, EPerf_ProfilerCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EndSample(const FString& SampleName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_ProfilerSample> GetProfilerSamples() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ClearSamples();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsProfilingEnabled() const { return bIsProfilingEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float NewTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFPS() const { return TargetFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

private:
    UPROPERTY(EditAnywhere, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsProfilingEnabled;

    UPROPERTY(EditAnywhere, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFPS;

    UPROPERTY(EditAnywhere, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float ProfilingUpdateInterval;

    UPROPERTY(EditAnywhere, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxSamples;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    TArray<FPerf_ProfilerSample> ProfilerSamples;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    TMap<FString, float> ActiveSamples;

    float LastUpdateTime;
    float AccumulatedFrameTime;
    int32 FrameCount;

    void UpdateMetrics();
    void CollectRenderingStats();
    void CollectMemoryStats();
    float GetCurrentTime() const;
};