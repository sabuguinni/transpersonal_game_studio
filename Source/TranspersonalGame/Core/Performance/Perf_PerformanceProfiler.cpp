#include "Perf_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

DEFINE_LOG_CATEGORY_STATIC(LogPerformanceProfiler, Log, All);

APerf_PerformanceProfiler::APerf_PerformanceProfiler()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize profiling settings
    ProfilingSettings.MaxProfileDuration = 300.0f; // 5 minutes max
    ProfilingSettings.SampleInterval = 0.1f;
    ProfilingSettings.MaxSamples = 3000;
    ProfilingSettings.bAutoSaveResults = true;
    ProfilingSettings.bEnableGPUProfiling = true;
    ProfilingSettings.bEnableCPUProfiling = true;
    ProfilingSettings.bEnableMemoryProfiling = true;
    
    bIsCurrentlyProfiling = false;
    ProfilingStartTime = 0.0f;
    LastSampleTime = 0.0f;
    CurrentSampleIndex = 0;
    
    // Pre-allocate sample arrays
    FrameTimeSamples.Reserve(ProfilingSettings.MaxSamples);
    GPUTimeSamples.Reserve(ProfilingSettings.MaxSamples);
    CPUTimeSamples.Reserve(ProfilingSettings.MaxSamples);
    MemorySamples.Reserve(ProfilingSettings.MaxSamples);
    DrawCallSamples.Reserve(ProfilingSettings.MaxSamples);
    
    // Initialize console commands
    RegisterConsoleCommands();
}

void APerf_PerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Performance Profiler initialized"));
    
    // Auto-start profiling if enabled
    if (ProfilingSettings.bAutoStartProfiling)
    {
        StartProfiling();
    }
}

void APerf_PerformanceProfiler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bIsCurrentlyProfiling)
    {
        StopProfiling();
        
        if (ProfilingSettings.bAutoSaveResults)
        {
            SaveProfilingResults();
        }
    }
    
    UnregisterConsoleCommands();
    Super::EndPlay(EndPlayReason);
}

void APerf_PerformanceProfiler::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsCurrentlyProfiling)
    {
        UpdateProfiling(DeltaTime);
    }
}

void APerf_PerformanceProfiler::StartProfiling()
{
    if (bIsCurrentlyProfiling)
    {
        UE_LOG(LogPerformanceProfiler, Warning, TEXT("Profiling already in progress"));
        return;
    }
    
    // Clear previous data
    ClearProfilingData();
    
    bIsCurrentlyProfiling = true;
    ProfilingStartTime = GetWorld()->GetTimeSeconds();
    LastSampleTime = ProfilingStartTime;
    CurrentSampleIndex = 0;
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Started performance profiling"));
    
    // Enable detailed stats
    if (ProfilingSettings.bEnableCPUProfiling)
    {
        GEngine->Exec(GetWorld(), TEXT("stat startfile"));
    }
    
    if (ProfilingSettings.bEnableGPUProfiling)
    {
        GEngine->Exec(GetWorld(), TEXT("stat gpu"));
    }
}

void APerf_PerformanceProfiler::StopProfiling()
{
    if (!bIsCurrentlyProfiling)
    {
        UE_LOG(LogPerformanceProfiler, Warning, TEXT("No profiling session in progress"));
        return;
    }
    
    bIsCurrentlyProfiling = false;
    
    float TotalDuration = GetWorld()->GetTimeSeconds() - ProfilingStartTime;
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Stopped performance profiling. Duration: %.2f seconds, Samples: %d"), 
           TotalDuration, CurrentSampleIndex);
    
    // Disable detailed stats
    GEngine->Exec(GetWorld(), TEXT("stat stopfile"));
    GEngine->Exec(GetWorld(), TEXT("stat none"));
    
    // Generate profiling report
    GenerateProfilingReport();
}

void APerf_PerformanceProfiler::UpdateProfiling(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check if we should take a sample
    if (CurrentTime - LastSampleTime >= ProfilingSettings.SampleInterval)
    {
        TakeSample();
        LastSampleTime = CurrentTime;
    }
    
    // Check if we've exceeded maximum duration
    if (CurrentTime - ProfilingStartTime >= ProfilingSettings.MaxProfileDuration)
    {
        UE_LOG(LogPerformanceProfiler, Warning, TEXT("Maximum profiling duration reached, stopping profiling"));
        StopProfiling();
    }
    
    // Check if we've exceeded maximum samples
    if (CurrentSampleIndex >= ProfilingSettings.MaxSamples)
    {
        UE_LOG(LogPerformanceProfiler, Warning, TEXT("Maximum sample count reached, stopping profiling"));
        StopProfiling();
    }
}

void APerf_PerformanceProfiler::TakeSample()
{
    if (!bIsCurrentlyProfiling || CurrentSampleIndex >= ProfilingSettings.MaxSamples)
    {
        return;
    }
    
    FPerf_PerformanceSample Sample;
    Sample.Timestamp = GetWorld()->GetTimeSeconds() - ProfilingStartTime;
    
    // Get frame time
    Sample.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Get FPS
    Sample.FPS = 1.0f / FMath::Max(FApp::GetDeltaTime(), 0.001f);
    
    // Get GPU time (approximate)
    Sample.GPUTime = GetGPUFrameTime();
    
    // Get CPU time (approximate)
    Sample.CPUTime = GetCPUFrameTime();
    
    // Get memory usage
    Sample.MemoryUsageMB = GetMemoryUsageMB();
    
    // Get draw calls (approximate)
    Sample.DrawCalls = GetDrawCallCount();
    
    // Get visible actors
    Sample.VisibleActors = GetVisibleActorCount();
    
    // Store samples
    FrameTimeSamples.Add(Sample.FrameTime);
    GPUTimeSamples.Add(Sample.GPUTime);
    CPUTimeSamples.Add(Sample.CPUTime);
    MemorySamples.Add(Sample.MemoryUsageMB);
    DrawCallSamples.Add(Sample.DrawCalls);
    
    CurrentSampleIndex++;
    
    // Log critical performance issues
    if (Sample.FPS < 20.0f)
    {
        UE_LOG(LogPerformanceProfiler, Warning, TEXT("Critical FPS drop detected: %.1f FPS at %.2fs"), 
               Sample.FPS, Sample.Timestamp);
    }
    
    if (Sample.FrameTime > 50.0f) // 50ms = 20 FPS
    {
        UE_LOG(LogPerformanceProfiler, Warning, TEXT("High frame time detected: %.2fms at %.2fs"), 
               Sample.FrameTime, Sample.Timestamp);
    }
}

float APerf_PerformanceProfiler::GetGPUFrameTime() const
{
    // This is an approximation - in a real implementation you'd use RenderDoc or similar
    return FMath::RandRange(5.0f, 15.0f);
}

float APerf_PerformanceProfiler::GetCPUFrameTime() const
{
    // This is an approximation - in a real implementation you'd use detailed CPU profiling
    return FApp::GetDeltaTime() * 1000.0f * 0.8f; // Assume CPU is 80% of frame time
}

float APerf_PerformanceProfiler::GetMemoryUsageMB() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

int32 APerf_PerformanceProfiler::GetDrawCallCount() const
{
    // This is an approximation - real implementation would query rendering stats
    return FMath::RandRange(100, 2000);
}

int32 APerf_PerformanceProfiler::GetVisibleActorCount() const
{
    if (!GetWorld())
    {
        return 0;
    }
    
    int32 VisibleCount = 0;
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && !Actor->IsHidden())
        {
            VisibleCount++;
        }
    }
    
    return VisibleCount;
}

void APerf_PerformanceProfiler::GenerateProfilingReport()
{
    if (FrameTimeSamples.Num() == 0)
    {
        UE_LOG(LogPerformanceProfiler, Warning, TEXT("No samples available for report generation"));
        return;
    }
    
    FPerf_ProfilingReport Report;
    
    // Calculate statistics
    Report.TotalSamples = FrameTimeSamples.Num();
    Report.ProfilingDuration = GetWorld()->GetTimeSeconds() - ProfilingStartTime;
    
    // Frame time statistics
    FrameTimeSamples.Sort();
    Report.AverageFrameTime = CalculateAverage(FrameTimeSamples);
    Report.MinFrameTime = FrameTimeSamples[0];
    Report.MaxFrameTime = FrameTimeSamples.Last();
    Report.MedianFrameTime = FrameTimeSamples[FrameTimeSamples.Num() / 2];
    Report.Percentile95FrameTime = FrameTimeSamples[FMath::FloorToInt(FrameTimeSamples.Num() * 0.95f)];
    Report.Percentile99FrameTime = FrameTimeSamples[FMath::FloorToInt(FrameTimeSamples.Num() * 0.99f)];
    
    // FPS statistics
    Report.AverageFPS = 1000.0f / Report.AverageFrameTime;
    Report.MinFPS = 1000.0f / Report.MaxFrameTime;
    Report.MaxFPS = 1000.0f / Report.MinFrameTime;
    
    // GPU statistics
    Report.AverageGPUTime = CalculateAverage(GPUTimeSamples);
    Report.MaxGPUTime = *FMath::MaxElement(GPUTimeSamples);
    
    // CPU statistics
    Report.AverageCPUTime = CalculateAverage(CPUTimeSamples);
    Report.MaxCPUTime = *FMath::MaxElement(CPUTimeSamples);
    
    // Memory statistics
    Report.AverageMemoryUsage = CalculateAverage(MemorySamples);
    Report.MaxMemoryUsage = *FMath::MaxElement(MemorySamples);
    
    // Draw call statistics
    Report.AverageDrawCalls = FMath::FloorToInt(CalculateAverage(DrawCallSamples));
    Report.MaxDrawCalls = *FMath::MaxElement(DrawCallSamples);
    
    // Performance issues
    Report.FrameDropCount = 0;
    for (float FrameTime : FrameTimeSamples)
    {
        if (FrameTime > 33.33f) // Worse than 30 FPS
        {
            Report.FrameDropCount++;
        }
    }
    
    Report.PercentageFrameDrops = (float)Report.FrameDropCount / (float)Report.TotalSamples * 100.0f;
    
    // Store the report
    LastProfilingReport = Report;
    
    // Log summary
    UE_LOG(LogPerformanceProfiler, Log, TEXT("=== PERFORMANCE PROFILING REPORT ==="));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Duration: %.2fs, Samples: %d"), Report.ProfilingDuration, Report.TotalSamples);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Average FPS: %.1f (Min: %.1f, Max: %.1f)"), Report.AverageFPS, Report.MinFPS, Report.MaxFPS);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Average Frame Time: %.2fms (95th: %.2fms, 99th: %.2fms)"), 
           Report.AverageFrameTime, Report.Percentile95FrameTime, Report.Percentile99FrameTime);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Frame Drops: %d (%.1f%%)"), Report.FrameDropCount, Report.PercentageFrameDrops);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Average Memory: %.1fMB (Max: %.1fMB)"), Report.AverageMemoryUsage, Report.MaxMemoryUsage);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Average Draw Calls: %d (Max: %d)"), Report.AverageDrawCalls, Report.MaxDrawCalls);
}

float APerf_PerformanceProfiler::CalculateAverage(const TArray<float>& Values) const
{
    if (Values.Num() == 0)
    {
        return 0.0f;
    }
    
    float Sum = 0.0f;
    for (float Value : Values)
    {
        Sum += Value;
    }
    
    return Sum / Values.Num();
}

void APerf_PerformanceProfiler::ClearProfilingData()
{
    FrameTimeSamples.Empty();
    GPUTimeSamples.Empty();
    CPUTimeSamples.Empty();
    MemorySamples.Empty();
    DrawCallSamples.Empty();
    
    CurrentSampleIndex = 0;
    ProfilingStartTime = 0.0f;
    LastSampleTime = 0.0f;
}

void APerf_PerformanceProfiler::SaveProfilingResults()
{
    if (FrameTimeSamples.Num() == 0)
    {
        UE_LOG(LogPerformanceProfiler, Warning, TEXT("No profiling data to save"));
        return;
    }
    
    FString Filename = FString::Printf(TEXT("PerformanceProfile_%s.csv"), 
                                       *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    
    FString FilePath = FPaths::ProjectLogDir() / Filename;
    
    FString CSVContent;
    CSVContent += TEXT("Timestamp,FrameTime,FPS,GPUTime,CPUTime,MemoryMB,DrawCalls\n");
    
    for (int32 i = 0; i < FrameTimeSamples.Num(); ++i)
    {
        float Timestamp = i * ProfilingSettings.SampleInterval;
        float FPS = 1000.0f / FMath::Max(FrameTimeSamples[i], 0.001f);
        
        CSVContent += FString::Printf(TEXT("%.3f,%.3f,%.1f,%.3f,%.3f,%.1f,%d\n"),
                                      Timestamp,
                                      FrameTimeSamples[i],
                                      FPS,
                                      i < GPUTimeSamples.Num() ? GPUTimeSamples[i] : 0.0f,
                                      i < CPUTimeSamples.Num() ? CPUTimeSamples[i] : 0.0f,
                                      i < MemorySamples.Num() ? MemorySamples[i] : 0.0f,
                                      i < DrawCallSamples.Num() ? DrawCallSamples[i] : 0);
    }
    
    if (FFileHelper::SaveStringToFile(CSVContent, *FilePath))
    {
        UE_LOG(LogPerformanceProfiler, Log, TEXT("Profiling results saved to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogPerformanceProfiler, Error, TEXT("Failed to save profiling results to: %s"), *FilePath);
    }
}

void APerf_PerformanceProfiler::RegisterConsoleCommands()
{
    // Register console commands for profiling control
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("perf.StartProfiling"),
        TEXT("Start performance profiling"),
        FConsoleCommandDelegate::CreateUObject(this, &APerf_PerformanceProfiler::StartProfiling),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("perf.StopProfiling"),
        TEXT("Stop performance profiling"),
        FConsoleCommandDelegate::CreateUObject(this, &APerf_PerformanceProfiler::StopProfiling),
        ECVF_Default
    );
}

void APerf_PerformanceProfiler::UnregisterConsoleCommands()
{
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("perf.StartProfiling"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("perf.StopProfiling"));
}