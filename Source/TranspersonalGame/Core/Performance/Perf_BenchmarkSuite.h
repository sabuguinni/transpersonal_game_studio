#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Perf_BenchmarkSuite.generated.h"

UENUM(BlueprintType)
enum class EPerf_BenchmarkType : uint8
{
    FrameRate,
    Memory,
    Physics,
    Rendering,
    AI,
    Streaming,
    Audio,
    Input,
    Network,
    All
};

USTRUCT(BlueprintType)
struct FPerf_BenchmarkResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    EPerf_BenchmarkType BenchmarkType;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float AverageScore;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float MinScore;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float MaxScore;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float StandardDeviation;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    int32 SampleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    FString BenchmarkName;

    FPerf_BenchmarkResult()
    {
        BenchmarkType = EPerf_BenchmarkType::FrameRate;
        AverageScore = 0.0f;
        MinScore = 0.0f;
        MaxScore = 0.0f;
        StandardDeviation = 0.0f;
        SampleCount = 0;
        BenchmarkName = TEXT("Unknown");
    }
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_BenchmarkSuite : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_BenchmarkSuite();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Main benchmark functions
    UFUNCTION(BlueprintCallable, Category = "Performance Benchmark")
    void StartBenchmark(EPerf_BenchmarkType BenchmarkType);

    UFUNCTION(BlueprintCallable, Category = "Performance Benchmark")
    void StopBenchmark();

    UFUNCTION(BlueprintCallable, Category = "Performance Benchmark")
    FPerf_BenchmarkResult GetBenchmarkResult(EPerf_BenchmarkType BenchmarkType);

    UFUNCTION(BlueprintCallable, Category = "Performance Benchmark")
    TArray<FPerf_BenchmarkResult> GetAllBenchmarkResults();

    UFUNCTION(BlueprintCallable, Category = "Performance Benchmark")
    void RunFullBenchmarkSuite();

    UFUNCTION(BlueprintCallable, Category = "Performance Benchmark")
    void ClearBenchmarkResults();

    // Specific benchmark tests
    UFUNCTION(BlueprintCallable, Category = "Performance Benchmark")
    float RunFrameRateBenchmark(float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Performance Benchmark")
    float RunMemoryBenchmark();

    UFUNCTION(BlueprintCallable, Category = "Performance Benchmark")
    float RunPhysicsBenchmark();

    UFUNCTION(BlueprintCallable, Category = "Performance Benchmark")
    float RunRenderingBenchmark();

    UFUNCTION(BlueprintCallable, Category = "Performance Benchmark")
    float RunAIBenchmark();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark Settings")
    float BenchmarkDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark Settings")
    int32 MaxSamples;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark Settings")
    bool bAutoRunOnStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark Settings")
    bool bLogResults;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark State")
    bool bIsBenchmarkRunning;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark State")
    EPerf_BenchmarkType CurrentBenchmarkType;

protected:
    UPROPERTY()
    TMap<EPerf_BenchmarkType, FPerf_BenchmarkResult> BenchmarkResults;

    UPROPERTY()
    TArray<float> CurrentSamples;

    UPROPERTY()
    float BenchmarkStartTime;

    UPROPERTY()
    float LastSampleTime;

    void UpdateBenchmarkSample(float SampleValue);
    void FinalizeBenchmarkResult();
    float CalculateStandardDeviation(const TArray<float>& Samples, float Average);
    void LogBenchmarkResult(const FPerf_BenchmarkResult& Result);
};