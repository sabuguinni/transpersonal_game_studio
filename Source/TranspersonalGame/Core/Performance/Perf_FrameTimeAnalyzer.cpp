#include "Perf_FrameTimeAnalyzer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"

UPerf_FrameTimeAnalyzer::UPerf_FrameTimeAnalyzer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    bEnableFrameAnalysis = true;
    SampleInterval = 0.1f;
    MaxSamples = 300;
    bLogPerformanceWarnings = true;
    
    CriticalFPSThreshold = 20.0f;
    LowFPSThreshold = 30.0f;
    HighFPSThreshold = 45.0f;
    UltraFPSThreshold = 60.0f;
    
    SampleTimer = 0.0f;
    bAnalysisActive = false;
    LastPerformanceLevel = EPerf_PerformanceLevel::High;
}

void UPerf_FrameTimeAnalyzer::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableFrameAnalysis)
    {
        StartFrameAnalysis();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Frame Time Analyzer initialized"));
}

void UPerf_FrameTimeAnalyzer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bAnalysisActive || !bEnableFrameAnalysis)
    {
        return;
    }
    
    SampleTimer += DeltaTime;
    
    if (SampleTimer >= SampleInterval)
    {
        UpdateFrameMetrics(DeltaTime);
        SampleTimer = 0.0f;
    }
}

void UPerf_FrameTimeAnalyzer::StartFrameAnalysis()
{
    bAnalysisActive = true;
    ResetMetrics();
    SampleTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Frame time analysis started"));
}

void UPerf_FrameTimeAnalyzer::StopFrameAnalysis()
{
    bAnalysisActive = false;
    UE_LOG(LogTemp, Log, TEXT("Frame time analysis stopped"));
}

void UPerf_FrameTimeAnalyzer::ResetMetrics()
{
    FPSSamples.Empty();
    FrameTimeSamples.Empty();
    
    CurrentMetrics = FPerf_FrameMetrics();
    LastPerformanceLevel = EPerf_PerformanceLevel::High;
}

void UPerf_FrameTimeAnalyzer::UpdateFrameMetrics(float DeltaTime)
{
    // Calculate current FPS
    float CurrentFPS = (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 0.0f;
    CurrentFPS = FMath::Clamp(CurrentFPS, 0.0f, 1000.0f);
    
    // Add to samples
    FPSSamples.Add(CurrentFPS);
    FrameTimeSamples.Add(DeltaTime * 1000.0f); // Convert to milliseconds
    
    // Maintain sample limit
    if (FPSSamples.Num() > MaxSamples)
    {
        FPSSamples.RemoveAt(0);
        FrameTimeSamples.RemoveAt(0);
    }
    
    // Update current metrics
    CurrentMetrics.CurrentFPS = CurrentFPS;
    CurrentMetrics.FrameTime = DeltaTime * 1000.0f;
    
    // Get engine timing data
    CurrentMetrics.GameThreadTime = GetGameThreadTime();
    CurrentMetrics.RenderThreadTime = GetRenderThreadTime();
    CurrentMetrics.GPUTime = GetGPUTime();
    
    // Calculate averages and extremes
    CalculateAverages();
    
    // Update performance level
    EPerf_PerformanceLevel NewLevel = CalculatePerformanceLevel(CurrentMetrics.CurrentFPS);
    if (NewLevel != LastPerformanceLevel)
    {
        OnPerformanceLevelChanged(NewLevel, LastPerformanceLevel);
        LastPerformanceLevel = NewLevel;
    }
    
    CurrentMetrics.PerformanceLevel = NewLevel;
    
    // Check for performance issues
    CheckPerformanceThresholds();
}

void UPerf_FrameTimeAnalyzer::CalculateAverages()
{
    if (FPSSamples.Num() == 0)
    {
        return;
    }
    
    // Calculate FPS statistics
    float TotalFPS = 0.0f;
    float MinFPS = FPSSamples[0];
    float MaxFPS = FPSSamples[0];
    
    for (float FPS : FPSSamples)
    {
        TotalFPS += FPS;
        MinFPS = FMath::Min(MinFPS, FPS);
        MaxFPS = FMath::Max(MaxFPS, FPS);
    }
    
    CurrentMetrics.AverageFPS = TotalFPS / FPSSamples.Num();
    CurrentMetrics.MinFPS = MinFPS;
    CurrentMetrics.MaxFPS = MaxFPS;
}

EPerf_PerformanceLevel UPerf_FrameTimeAnalyzer::CalculatePerformanceLevel(float FPS) const
{
    if (FPS >= UltraFPSThreshold)
    {
        return EPerf_PerformanceLevel::Ultra;
    }
    else if (FPS >= HighFPSThreshold)
    {
        return EPerf_PerformanceLevel::High;
    }
    else if (FPS >= LowFPSThreshold)
    {
        return EPerf_PerformanceLevel::Medium;
    }
    else if (FPS >= CriticalFPSThreshold)
    {
        return EPerf_PerformanceLevel::Low;
    }
    else
    {
        return EPerf_PerformanceLevel::Critical;
    }
}

void UPerf_FrameTimeAnalyzer::CheckPerformanceThresholds()
{
    if (CurrentMetrics.CurrentFPS < CriticalFPSThreshold)
    {
        if (bLogPerformanceWarnings)
        {
            UE_LOG(LogTemp, Warning, TEXT("CRITICAL PERFORMANCE: FPS dropped to %.1f"), CurrentMetrics.CurrentFPS);
        }
        OnCriticalPerformance(CurrentMetrics.CurrentFPS);
    }
    else if (CurrentMetrics.CurrentFPS > UltraFPSThreshold && LastPerformanceLevel != EPerf_PerformanceLevel::Ultra)
    {
        OnPerformanceImproved(CurrentMetrics.CurrentFPS);
    }
}

FPerf_FrameMetrics UPerf_FrameTimeAnalyzer::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

EPerf_PerformanceLevel UPerf_FrameTimeAnalyzer::GetCurrentPerformanceLevel() const
{
    return CurrentMetrics.PerformanceLevel;
}

bool UPerf_FrameTimeAnalyzer::IsPerformanceCritical() const
{
    return CurrentMetrics.PerformanceLevel == EPerf_PerformanceLevel::Critical ||
           CurrentMetrics.PerformanceLevel == EPerf_PerformanceLevel::Low;
}

void UPerf_FrameTimeAnalyzer::LogPerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Current FPS: %.1f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Log, TEXT("Average FPS: %.1f"), CurrentMetrics.AverageFPS);
    UE_LOG(LogTemp, Log, TEXT("Min FPS: %.1f"), CurrentMetrics.MinFPS);
    UE_LOG(LogTemp, Log, TEXT("Max FPS: %.1f"), CurrentMetrics.MaxFPS);
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Log, TEXT("Game Thread: %.2f ms"), CurrentMetrics.GameThreadTime);
    UE_LOG(LogTemp, Log, TEXT("Render Thread: %.2f ms"), CurrentMetrics.RenderThreadTime);
    UE_LOG(LogTemp, Log, TEXT("GPU Time: %.2f ms"), CurrentMetrics.GPUTime);
    
    FString LevelString;
    switch (CurrentMetrics.PerformanceLevel)
    {
        case EPerf_PerformanceLevel::Ultra: LevelString = TEXT("Ultra"); break;
        case EPerf_PerformanceLevel::High: LevelString = TEXT("High"); break;
        case EPerf_PerformanceLevel::Medium: LevelString = TEXT("Medium"); break;
        case EPerf_PerformanceLevel::Low: LevelString = TEXT("Low"); break;
        case EPerf_PerformanceLevel::Critical: LevelString = TEXT("Critical"); break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Level: %s"), *LevelString);
    UE_LOG(LogTemp, Log, TEXT("Sample Count: %d"), FPSSamples.Num());
    UE_LOG(LogTemp, Log, TEXT("========================="));
}

float UPerf_FrameTimeAnalyzer::GetEngineFrameTime() const
{
    // Get frame time from engine stats
    return FApp::GetDeltaTime() * 1000.0f;
}

float UPerf_FrameTimeAnalyzer::GetGameThreadTime() const
{
    // Approximate game thread time
    return FApp::GetDeltaTime() * 1000.0f * 0.6f;
}

float UPerf_FrameTimeAnalyzer::GetRenderThreadTime() const
{
    // Approximate render thread time
    return FApp::GetDeltaTime() * 1000.0f * 0.4f;
}

float UPerf_FrameTimeAnalyzer::GetGPUTime() const
{
    // Approximate GPU time
    return FApp::GetDeltaTime() * 1000.0f * 0.7f;
}