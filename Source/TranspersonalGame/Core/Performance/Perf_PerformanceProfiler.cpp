#include "Perf_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UPerf_PerformanceProfiler::UPerf_PerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // Tick every frame for accurate profiling
    
    SamplingInterval = 0.1f;
    SampleHistorySize = 60;
    bEnableAutomaticProfiling = true;
    bLogPerformanceWarnings = true;
    
    CriticalFPSThreshold = 20.0f;
    LowFPSThreshold = 30.0f;
    MediumFPSThreshold = 45.0f;
    HighFPSThreshold = 60.0f;
    FrameTimeSpikeThreshold = 33.33f;
    
    TimeSinceLastSample = 0.0f;
    bIsProfiling = false;
    LastPerformanceLevel = EPerf_PerformanceLevel::Medium;
    
    FrameTimeHistory.Reserve(SampleHistorySize);
    FPSHistory.Reserve(SampleHistorySize);
}

void UPerf_PerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableAutomaticProfiling)
    {
        StartProfiling();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler initialized - Automatic profiling: %s"), 
           bEnableAutomaticProfiling ? TEXT("Enabled") : TEXT("Disabled"));
}

void UPerf_PerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsProfiling)
    {
        return;
    }
    
    TimeSinceLastSample += DeltaTime;
    
    if (TimeSinceLastSample >= SamplingInterval)
    {
        UpdatePerformanceMetrics();
        UpdatePerformanceLevel();
        CheckForFrameTimeSpikes(DeltaTime * 1000.0f); // Convert to milliseconds
        
        TimeSinceLastSample = 0.0f;
    }
}

void UPerf_PerformanceProfiler::StartProfiling()
{
    bIsProfiling = true;
    ResetMetrics();
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
}

void UPerf_PerformanceProfiler::StopProfiling()
{
    bIsProfiling = false;
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
}

void UPerf_PerformanceProfiler::ResetMetrics()
{
    FrameTimeHistory.Empty();
    FPSHistory.Empty();
    CurrentMetrics = FPerf_PerformanceMetrics();
    TimeSinceLastSample = 0.0f;
    LastPerformanceLevel = EPerf_PerformanceLevel::Medium;
}

FPerf_PerformanceMetrics UPerf_PerformanceProfiler::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

float UPerf_PerformanceProfiler::GetAverageFPS() const
{
    if (FPSHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Sum = 0.0f;
    for (float FPS : FPSHistory)
    {
        Sum += FPS;
    }
    
    return Sum / FPSHistory.Num();
}

float UPerf_PerformanceProfiler::GetAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Sum = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Sum += FrameTime;
    }
    
    return Sum / FrameTimeHistory.Num();
}

EPerf_PerformanceLevel UPerf_PerformanceProfiler::GetCurrentPerformanceLevel() const
{
    return CurrentMetrics.PerformanceLevel;
}

void UPerf_PerformanceProfiler::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Calculate current FPS from world delta time
    float WorldDeltaTime = World->GetDeltaSeconds();
    if (WorldDeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / WorldDeltaTime;
        CurrentMetrics.AverageFrameTime = WorldDeltaTime * 1000.0f; // Convert to milliseconds
    }
    
    // Add to history
    AddFrameTimeSample(CurrentMetrics.AverageFrameTime);
    
    // Estimate other metrics (in a real implementation, these would come from engine stats)
    CurrentMetrics.GameThreadTime = CurrentMetrics.AverageFrameTime * 0.6f; // Rough estimate
    CurrentMetrics.RenderThreadTime = CurrentMetrics.AverageFrameTime * 0.8f; // Rough estimate
    
    // Count actors for draw call estimation
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    CurrentMetrics.DrawCalls = AllActors.Num(); // Simplified estimation
    CurrentMetrics.TriangleCount = AllActors.Num() * 500; // Very rough estimate
    CurrentMetrics.MemoryUsageMB = AllActors.Num() * 0.5f; // Rough estimate
}

void UPerf_PerformanceProfiler::UpdatePerformanceLevel()
{
    EPerf_PerformanceLevel NewLevel = CalculatePerformanceLevel(CurrentMetrics.CurrentFPS);
    CurrentMetrics.PerformanceLevel = NewLevel;
    
    if (NewLevel != LastPerformanceLevel)
    {
        TriggerPerformanceLevelChange(NewLevel);
        LastPerformanceLevel = NewLevel;
    }
}

void UPerf_PerformanceProfiler::AddFrameTimeSample(float FrameTime)
{
    FrameTimeHistory.Add(FrameTime);
    FPSHistory.Add(CurrentMetrics.CurrentFPS);
    
    // Maintain history size
    if (FrameTimeHistory.Num() > SampleHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    if (FPSHistory.Num() > SampleHistorySize)
    {
        FPSHistory.RemoveAt(0);
    }
}

void UPerf_PerformanceProfiler::CheckForFrameTimeSpikes(float FrameTime)
{
    if (FrameTime > FrameTimeSpikeThreshold)
    {
        OnFrameTimeSpike.Broadcast(FrameTime);
        
        if (bLogPerformanceWarnings)
        {
            UE_LOG(LogTemp, Warning, TEXT("Frame time spike detected: %.2f ms"), FrameTime);
        }
    }
}

EPerf_PerformanceLevel UPerf_PerformanceProfiler::CalculatePerformanceLevel(float FPS) const
{
    if (FPS < CriticalFPSThreshold)
    {
        return EPerf_PerformanceLevel::Critical;
    }
    else if (FPS < LowFPSThreshold)
    {
        return EPerf_PerformanceLevel::Low;
    }
    else if (FPS < MediumFPSThreshold)
    {
        return EPerf_PerformanceLevel::Medium;
    }
    else if (FPS < HighFPSThreshold)
    {
        return EPerf_PerformanceLevel::High;
    }
    else
    {
        return EPerf_PerformanceLevel::Ultra;
    }
}

void UPerf_PerformanceProfiler::TriggerPerformanceLevelChange(EPerf_PerformanceLevel NewLevel)
{
    OnPerformanceLevelChanged.Broadcast(NewLevel);
    
    if (bLogPerformanceWarnings)
    {
        FString LevelName;
        switch (NewLevel)
        {
            case EPerf_PerformanceLevel::Critical: LevelName = TEXT("Critical"); break;
            case EPerf_PerformanceLevel::Low: LevelName = TEXT("Low"); break;
            case EPerf_PerformanceLevel::Medium: LevelName = TEXT("Medium"); break;
            case EPerf_PerformanceLevel::High: LevelName = TEXT("High"); break;
            case EPerf_PerformanceLevel::Ultra: LevelName = TEXT("Ultra"); break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Performance level changed to: %s (FPS: %.1f)"), 
               *LevelName, CurrentMetrics.CurrentFPS);
    }
}

void UPerf_PerformanceProfiler::LogPerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Current FPS: %.1f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Log, TEXT("Average FPS: %.1f"), GetAverageFPS());
    UE_LOG(LogTemp, Log, TEXT("Average Frame Time: %.2f ms"), GetAverageFrameTime());
    UE_LOG(LogTemp, Log, TEXT("Performance Level: %d"), (int32)CurrentMetrics.PerformanceLevel);
    UE_LOG(LogTemp, Log, TEXT("Draw Calls: %d"), CurrentMetrics.DrawCalls);
    UE_LOG(LogTemp, Log, TEXT("Triangle Count: %d"), CurrentMetrics.TriangleCount);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("========================="));
}

void UPerf_PerformanceProfiler::SavePerformanceReport(const FString& FilePath)
{
    FString ReportContent;
    ReportContent += FString::Printf(TEXT("Performance Report - %s\n"), *FDateTime::Now().ToString());
    ReportContent += FString::Printf(TEXT("Current FPS: %.1f\n"), CurrentMetrics.CurrentFPS);
    ReportContent += FString::Printf(TEXT("Average FPS: %.1f\n"), GetAverageFPS());
    ReportContent += FString::Printf(TEXT("Average Frame Time: %.2f ms\n"), GetAverageFrameTime());
    ReportContent += FString::Printf(TEXT("Performance Level: %d\n"), (int32)CurrentMetrics.PerformanceLevel);
    ReportContent += FString::Printf(TEXT("Draw Calls: %d\n"), CurrentMetrics.DrawCalls);
    ReportContent += FString::Printf(TEXT("Triangle Count: %d\n"), CurrentMetrics.TriangleCount);
    ReportContent += FString::Printf(TEXT("Memory Usage: %.1f MB\n"), CurrentMetrics.MemoryUsageMB);
    
    if (!FFileHelper::SaveStringToFile(ReportContent, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save performance report to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Performance report saved to: %s"), *FilePath);
    }
}