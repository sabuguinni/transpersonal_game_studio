#include "Perf_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"

UPerf_PerformanceProfiler::UPerf_PerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    bEnableProfiling = true;
    ProfilingInterval = 1.0f;
    bLogToFile = false;
    bDisplayOnScreen = true;
    LastProfilingTime = 0.0f;
    CurrentPerformanceLevel = EPerf_PerformanceLevel::High;
    
    LogFilePath = FPaths::ProjectLogDir() / TEXT("PerformanceProfiler.log");
}

void UPerf_PerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableProfiling)
    {
        StartProfiling();
    }
}

void UPerf_PerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableProfiling)
    {
        return;
    }
    
    LastProfilingTime += DeltaTime;
    
    if (LastProfilingTime >= ProfilingInterval)
    {
        UpdateMetrics();
        CalculatePerformanceLevel();
        CheckPerformanceThresholds();
        
        if (bLogToFile)
        {
            LogPerformanceData();
        }
        
        if (bDisplayOnScreen)
        {
            DisplayPerformanceOnScreen();
        }
        
        LastProfilingTime = 0.0f;
    }
}

void UPerf_PerformanceProfiler::StartProfiling()
{
    bEnableProfiling = true;
    ResetMetrics();
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
}

void UPerf_PerformanceProfiler::StopProfiling()
{
    bEnableProfiling = false;
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
}

void UPerf_PerformanceProfiler::ResetMetrics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    MetricsHistory.Empty();
    LastProfilingTime = 0.0f;
}

FPerf_PerformanceMetrics UPerf_PerformanceProfiler::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

EPerf_PerformanceLevel UPerf_PerformanceProfiler::GetPerformanceLevel() const
{
    return CurrentPerformanceLevel;
}

bool UPerf_PerformanceProfiler::IsPerformanceCritical() const
{
    return CurrentMetrics.FrameTime > PerformanceThresholds.CriticalFrameTime ||
           CurrentMetrics.ActiveActors > PerformanceThresholds.MaxActiveActors ||
           CurrentMetrics.GPUMemoryUsed > PerformanceThresholds.MaxGPUMemory;
}

void UPerf_PerformanceProfiler::SetPerformanceThresholds(const FPerf_PerformanceThresholds& NewThresholds)
{
    PerformanceThresholds = NewThresholds;
}

void UPerf_PerformanceProfiler::LogPerformanceData()
{
    FString LogEntry = FString::Printf(TEXT("[%s] FPS: %.1f, Frame: %.2fms, Actors: %d, GPU: %.1fMB, CPU: %.1fMB, DrawCalls: %d"),
        *FDateTime::Now().ToString(),
        1000.0f / CurrentMetrics.FrameTime,
        CurrentMetrics.FrameTime,
        CurrentMetrics.ActiveActors,
        CurrentMetrics.GPUMemoryUsed,
        CurrentMetrics.CPUMemoryUsed,
        CurrentMetrics.DrawCalls);
    
    FFileHelper::SaveStringToFile(LogEntry + LINE_TERMINATOR, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}

void UPerf_PerformanceProfiler::DisplayPerformanceOnScreen()
{
    if (GEngine)
    {
        FString PerformanceText = FString::Printf(TEXT("Performance Monitor:\nFPS: %.1f | Frame: %.2fms | Level: %s\nActors: %d | GPU: %.1fMB | CPU: %.1fMB"),
            1000.0f / CurrentMetrics.FrameTime,
            CurrentMetrics.FrameTime,
            *UEnum::GetValueAsString(CurrentPerformanceLevel),
            CurrentMetrics.ActiveActors,
            CurrentMetrics.GPUMemoryUsed,
            CurrentMetrics.CPUMemoryUsed);
        
        FColor DisplayColor = FColor::Green;
        if (IsPerformanceCritical())
        {
            DisplayColor = FColor::Red;
        }
        else if (CurrentMetrics.FrameTime > PerformanceThresholds.WarningFrameTime)
        {
            DisplayColor = FColor::Yellow;
        }
        
        GEngine->AddOnScreenDebugMessage(-1, ProfilingInterval + 0.1f, DisplayColor, PerformanceText);
    }
}

void UPerf_PerformanceProfiler::UpdateMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Frame time and delta time
    CurrentMetrics.DeltaTime = World->GetDeltaSeconds();
    CurrentMetrics.FrameTime = CurrentMetrics.DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Actor counts
    CurrentMetrics.ActiveActors = World->GetActorCount();
    CurrentMetrics.TickingActors = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetActorTickEnabled())
        {
            CurrentMetrics.TickingActors++;
        }
    }
    
    // Memory usage (simplified estimates)
    CurrentMetrics.CPUMemoryUsed = GetMemoryUsage();
    CurrentMetrics.GPUMemoryUsed = CurrentMetrics.CPUMemoryUsed * 0.6f; // Rough estimate
    
    // Rendering stats (simplified)
    CurrentMetrics.DrawCalls = FMath::Clamp(CurrentMetrics.ActiveActors / 4, 100, 5000);
    CurrentMetrics.Triangles = CurrentMetrics.DrawCalls * 250; // Rough estimate
    
    // Thread times (simplified)
    CurrentMetrics.GameThreadTime = CurrentMetrics.FrameTime * 0.7f;
    CurrentMetrics.RenderThreadTime = CurrentMetrics.FrameTime * 0.5f;
    CurrentMetrics.PhysicsTime = CurrentMetrics.FrameTime * 0.2f;
    CurrentMetrics.RenderTime = CurrentMetrics.FrameTime * 0.4f;
    
    // Store in history
    MetricsHistory.Add(CurrentMetrics);
    if (MetricsHistory.Num() > 60) // Keep last 60 samples
    {
        MetricsHistory.RemoveAt(0);
    }
}

void UPerf_PerformanceProfiler::CalculatePerformanceLevel()
{
    float AvgFrameTime = GetAverageFrameTime();
    
    if (AvgFrameTime <= 16.67f) // 60+ FPS
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Ultra;
    }
    else if (AvgFrameTime <= 20.0f) // 50+ FPS
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::High;
    }
    else if (AvgFrameTime <= 25.0f) // 40+ FPS
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Medium;
    }
    else if (AvgFrameTime <= 33.33f) // 30+ FPS
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Low;
    }
    else
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Minimal;
    }
}

void UPerf_PerformanceProfiler::WriteToLogFile(const FPerf_PerformanceMetrics& Metrics)
{
    FString LogData = FString::Printf(TEXT("Timestamp: %s, FrameTime: %.2f, FPS: %.1f, Actors: %d, Memory: %.1f MB"),
        *FDateTime::Now().ToString(),
        Metrics.FrameTime,
        1000.0f / Metrics.FrameTime,
        Metrics.ActiveActors,
        Metrics.CPUMemoryUsed);
    
    FFileHelper::SaveStringToFile(LogData + LINE_TERMINATOR, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}

void UPerf_PerformanceProfiler::CheckPerformanceThresholds()
{
    if (IsPerformanceCritical())
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Critical: Frame time %.2fms exceeds threshold %.2fms"), 
               CurrentMetrics.FrameTime, PerformanceThresholds.CriticalFrameTime);
    }
}

float UPerf_PerformanceProfiler::GetAverageFrameTime() const
{
    if (MetricsHistory.Num() == 0)
    {
        return CurrentMetrics.FrameTime;
    }
    
    float Total = 0.0f;
    for (const FPerf_PerformanceMetrics& Metrics : MetricsHistory)
    {
        Total += Metrics.FrameTime;
    }
    
    return Total / MetricsHistory.Num();
}

int32 UPerf_PerformanceProfiler::GetMemoryUsage() const
{
    // Simplified memory calculation based on actor count
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    int32 ActorCount = World->GetActorCount();
    return FMath::Clamp(ActorCount * 2, 512, 16384); // 2MB per actor, clamped between 512MB and 16GB
}