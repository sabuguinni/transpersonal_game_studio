#include "Perf_BenchmarkSuite.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"
#include "RenderingThread.h"
#include "HAL/PlatformMemory.h"

UPerf_BenchmarkSuite::UPerf_BenchmarkSuite()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Sample every 100ms
    
    BenchmarkDuration = 10.0f;
    MaxSamples = 100;
    bAutoRunOnStart = false;
    bLogResults = true;
    bIsBenchmarkRunning = false;
    CurrentBenchmarkType = EPerf_BenchmarkType::FrameRate;
    BenchmarkStartTime = 0.0f;
    LastSampleTime = 0.0f;
}

void UPerf_BenchmarkSuite::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunOnStart)
    {
        // Delay auto-run to allow world to fully initialize
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this]() { RunFullBenchmarkSuite(); },
            2.0f,
            false
        );
    }
}

void UPerf_BenchmarkSuite::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsBenchmarkRunning)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check if benchmark duration has elapsed
    if (CurrentTime - BenchmarkStartTime >= BenchmarkDuration)
    {
        StopBenchmark();
        return;
    }
    
    // Sample based on benchmark type
    float SampleValue = 0.0f;
    
    switch (CurrentBenchmarkType)
    {
        case EPerf_BenchmarkType::FrameRate:
            SampleValue = 1.0f / DeltaTime; // FPS
            break;
            
        case EPerf_BenchmarkType::Memory:
            SampleValue = static_cast<float>(FPlatformMemory::GetStats().UsedPhysical) / (1024.0f * 1024.0f); // MB
            break;
            
        case EPerf_BenchmarkType::Physics:
            // Measure physics thread time (simplified)
            SampleValue = DeltaTime * 1000.0f; // Convert to milliseconds
            break;
            
        case EPerf_BenchmarkType::Rendering:
            // Measure render thread time (simplified)
            SampleValue = DeltaTime * 1000.0f; // Convert to milliseconds
            break;
            
        default:
            SampleValue = DeltaTime * 1000.0f;
            break;
    }
    
    UpdateBenchmarkSample(SampleValue);
}

void UPerf_BenchmarkSuite::StartBenchmark(EPerf_BenchmarkType BenchmarkType)
{
    if (bIsBenchmarkRunning)
    {
        UE_LOG(LogTemp, Warning, TEXT("Benchmark already running. Stop current benchmark first."));
        return;
    }
    
    CurrentBenchmarkType = BenchmarkType;
    bIsBenchmarkRunning = true;
    BenchmarkStartTime = GetWorld()->GetTimeSeconds();
    LastSampleTime = BenchmarkStartTime;
    CurrentSamples.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Starting %s benchmark for %.1f seconds"), 
           *UEnum::GetValueAsString(BenchmarkType), BenchmarkDuration);
}

void UPerf_BenchmarkSuite::StopBenchmark()
{
    if (!bIsBenchmarkRunning)
    {
        return;
    }
    
    bIsBenchmarkRunning = false;
    FinalizeBenchmarkResult();
    
    UE_LOG(LogTemp, Log, TEXT("Benchmark completed with %d samples"), CurrentSamples.Num());
}

void UPerf_BenchmarkSuite::UpdateBenchmarkSample(float SampleValue)
{
    if (CurrentSamples.Num() >= MaxSamples)
    {
        return;
    }
    
    CurrentSamples.Add(SampleValue);
}

void UPerf_BenchmarkSuite::FinalizeBenchmarkResult()
{
    if (CurrentSamples.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No samples collected for benchmark"));
        return;
    }
    
    FPerf_BenchmarkResult Result;
    Result.BenchmarkType = CurrentBenchmarkType;
    Result.SampleCount = CurrentSamples.Num();
    Result.BenchmarkName = UEnum::GetValueAsString(CurrentBenchmarkType);
    
    // Calculate statistics
    float Sum = 0.0f;
    Result.MinScore = CurrentSamples[0];
    Result.MaxScore = CurrentSamples[0];
    
    for (float Sample : CurrentSamples)
    {
        Sum += Sample;
        Result.MinScore = FMath::Min(Result.MinScore, Sample);
        Result.MaxScore = FMath::Max(Result.MaxScore, Sample);
    }
    
    Result.AverageScore = Sum / CurrentSamples.Num();
    Result.StandardDeviation = CalculateStandardDeviation(CurrentSamples, Result.AverageScore);
    
    // Store result
    BenchmarkResults.Add(CurrentBenchmarkType, Result);
    
    if (bLogResults)
    {
        LogBenchmarkResult(Result);
    }
}

float UPerf_BenchmarkSuite::CalculateStandardDeviation(const TArray<float>& Samples, float Average)
{
    if (Samples.Num() <= 1)
    {
        return 0.0f;
    }
    
    float VarianceSum = 0.0f;
    for (float Sample : Samples)
    {
        float Deviation = Sample - Average;
        VarianceSum += Deviation * Deviation;
    }
    
    float Variance = VarianceSum / (Samples.Num() - 1);
    return FMath::Sqrt(Variance);
}

void UPerf_BenchmarkSuite::LogBenchmarkResult(const FPerf_BenchmarkResult& Result)
{
    FString Unit = TEXT("");
    
    switch (Result.BenchmarkType)
    {
        case EPerf_BenchmarkType::FrameRate:
            Unit = TEXT("FPS");
            break;
        case EPerf_BenchmarkType::Memory:
            Unit = TEXT("MB");
            break;
        case EPerf_BenchmarkType::Physics:
        case EPerf_BenchmarkType::Rendering:
            Unit = TEXT("ms");
            break;
        default:
            Unit = TEXT("units");
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== BENCHMARK RESULT: %s ==="), *Result.BenchmarkName);
    UE_LOG(LogTemp, Log, TEXT("Average: %.2f %s"), Result.AverageScore, *Unit);
    UE_LOG(LogTemp, Log, TEXT("Min: %.2f %s"), Result.MinScore, *Unit);
    UE_LOG(LogTemp, Log, TEXT("Max: %.2f %s"), Result.MaxScore, *Unit);
    UE_LOG(LogTemp, Log, TEXT("Std Dev: %.2f"), Result.StandardDeviation);
    UE_LOG(LogTemp, Log, TEXT("Samples: %d"), Result.SampleCount);
}

FPerf_BenchmarkResult UPerf_BenchmarkSuite::GetBenchmarkResult(EPerf_BenchmarkType BenchmarkType)
{
    if (BenchmarkResults.Contains(BenchmarkType))
    {
        return BenchmarkResults[BenchmarkType];
    }
    
    return FPerf_BenchmarkResult(); // Return default result if not found
}

TArray<FPerf_BenchmarkResult> UPerf_BenchmarkSuite::GetAllBenchmarkResults()
{
    TArray<FPerf_BenchmarkResult> Results;
    for (auto& Pair : BenchmarkResults)
    {
        Results.Add(Pair.Value);
    }
    return Results;
}

void UPerf_BenchmarkSuite::RunFullBenchmarkSuite()
{
    UE_LOG(LogTemp, Log, TEXT("Starting full benchmark suite"));
    
    // Clear previous results
    ClearBenchmarkResults();
    
    // Run each benchmark sequentially with delays
    TArray<EPerf_BenchmarkType> BenchmarkTypes = {
        EPerf_BenchmarkType::FrameRate,
        EPerf_BenchmarkType::Memory,
        EPerf_BenchmarkType::Physics,
        EPerf_BenchmarkType::Rendering
    };
    
    int32 CurrentIndex = 0;
    
    // Lambda function to run next benchmark
    TFunction<void()> RunNextBenchmark = [this, BenchmarkTypes, &CurrentIndex, &RunNextBenchmark]() mutable
    {
        if (CurrentIndex >= BenchmarkTypes.Num())
        {
            UE_LOG(LogTemp, Log, TEXT("Full benchmark suite completed"));
            return;
        }
        
        StartBenchmark(BenchmarkTypes[CurrentIndex]);
        CurrentIndex++;
        
        // Schedule next benchmark after current one completes + 1 second delay
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            RunNextBenchmark,
            BenchmarkDuration + 1.0f,
            false
        );
    };
    
    // Start the first benchmark
    RunNextBenchmark();
}

void UPerf_BenchmarkSuite::ClearBenchmarkResults()
{
    BenchmarkResults.Empty();
    CurrentSamples.Empty();
    UE_LOG(LogTemp, Log, TEXT("Benchmark results cleared"));
}

float UPerf_BenchmarkSuite::RunFrameRateBenchmark(float Duration)
{
    BenchmarkDuration = Duration;
    StartBenchmark(EPerf_BenchmarkType::FrameRate);
    
    // Return current FPS as immediate result
    return GetWorld()->GetDeltaSeconds() > 0.0f ? 1.0f / GetWorld()->GetDeltaSeconds() : 0.0f;
}

float UPerf_BenchmarkSuite::RunMemoryBenchmark()
{
    StartBenchmark(EPerf_BenchmarkType::Memory);
    
    // Return current memory usage in MB
    return static_cast<float>(FPlatformMemory::GetStats().UsedPhysical) / (1024.0f * 1024.0f);
}

float UPerf_BenchmarkSuite::RunPhysicsBenchmark()
{
    StartBenchmark(EPerf_BenchmarkType::Physics);
    
    // Return current frame time in ms as immediate result
    return GetWorld()->GetDeltaSeconds() * 1000.0f;
}

float UPerf_BenchmarkSuite::RunRenderingBenchmark()
{
    StartBenchmark(EPerf_BenchmarkType::Rendering);
    
    // Return current frame time in ms as immediate result
    return GetWorld()->GetDeltaSeconds() * 1000.0f;
}

float UPerf_BenchmarkSuite::RunAIBenchmark()
{
    StartBenchmark(EPerf_BenchmarkType::AI);
    
    // Return current frame time in ms as immediate result
    return GetWorld()->GetDeltaSeconds() * 1000.0f;
}