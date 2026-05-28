#include "Eng_PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"

UEng_PerformanceMonitor::UEng_PerformanceMonitor()
{
    MinimumFPSThreshold = 30.0f;
    MaximumMemoryThreshold = 4096.0f; // 4GB
    bIsMonitoringActive = false;
    MonitoringInterval = UPDATE_INTERVAL;
    LastUpdateTime = 0.0f;
    AccumulatedTime = 0.0f;
    FrameCount = 0;
}

void UEng_PerformanceMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor: Initializing"));
    
    // Initialize performance data
    CurrentData = FEng_PerformanceData();
    
    // Start monitoring
    StartPerformanceCapture();
}

void UEng_PerformanceMonitor::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor: Shutting down"));
    
    StopPerformanceCapture();
    
    FPSHistory.Empty();
    MemoryHistory.Empty();
    
    Super::Deinitialize();
}

void UEng_PerformanceMonitor::Tick(float DeltaTime)
{
    if (!bIsMonitoringActive)
    {
        return;
    }
    
    AccumulatedTime += DeltaTime;
    FrameCount++;
    
    // Update performance data at regular intervals
    if (AccumulatedTime >= MonitoringInterval)
    {
        UpdatePerformanceData();
        AccumulatedTime = 0.0f;
        FrameCount = 0;
    }
}

FEng_PerformanceData UEng_PerformanceMonitor::GetCurrentPerformanceData() const
{
    return CurrentData;
}

float UEng_PerformanceMonitor::GetCurrentFPS() const
{
    return CurrentData.CurrentFPS;
}

float UEng_PerformanceMonitor::GetAverageFPS() const
{
    return CurrentData.AverageFPS;
}

bool UEng_PerformanceMonitor::IsPerformanceAcceptable() const
{
    return CurrentData.CurrentFPS >= MinimumFPSThreshold && 
           CurrentData.MemoryUsageMB <= MaximumMemoryThreshold;
}

void UEng_PerformanceMonitor::LogPerformanceReport() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.2f"), CurrentData.CurrentFPS);
    UE_LOG(LogTemp, Warning, TEXT("Average FPS: %.2f"), CurrentData.AverageFPS);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB"), CurrentData.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("CPU Usage: %.2f%%"), CurrentData.CPUUsagePercent);
    UE_LOG(LogTemp, Warning, TEXT("GPU Usage: %.2f%%"), CurrentData.GPUUsagePercent);
    UE_LOG(LogTemp, Warning, TEXT("Draw Calls: %d"), CurrentData.DrawCalls);
    UE_LOG(LogTemp, Warning, TEXT("Triangle Count: %d"), CurrentData.TriangleCount);
    UE_LOG(LogTemp, Warning, TEXT("Performance Status: %s"), 
        IsPerformanceAcceptable() ? TEXT("ACCEPTABLE") : TEXT("BELOW THRESHOLD"));
}

void UEng_PerformanceMonitor::SetPerformanceThresholds(float MinFPS, float MaxMemoryMB)
{
    MinimumFPSThreshold = MinFPS;
    MaximumMemoryThreshold = MaxMemoryMB;
    
    UE_LOG(LogTemp, Log, TEXT("Performance thresholds updated: Min FPS %.2f, Max Memory %.2f MB"), 
        MinFPS, MaxMemoryMB);
}

bool UEng_PerformanceMonitor::IsPerformanceBelowThreshold() const
{
    return !IsPerformanceAcceptable();
}

void UEng_PerformanceMonitor::StartPerformanceCapture()
{
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor: Starting capture"));
    
    bIsMonitoringActive = true;
    AccumulatedTime = 0.0f;
    FrameCount = 0;
    
    ResetPerformanceData();
}

void UEng_PerformanceMonitor::StopPerformanceCapture()
{
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor: Stopping capture"));
    
    bIsMonitoringActive = false;
}

void UEng_PerformanceMonitor::ResetPerformanceData()
{
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Resetting data"));
    
    CurrentData = FEng_PerformanceData();
    FPSHistory.Empty();
    MemoryHistory.Empty();
}

void UEng_PerformanceMonitor::UpdatePerformanceData()
{
    // Calculate current FPS
    if (AccumulatedTime > 0.0f && FrameCount > 0)
    {
        CurrentData.CurrentFPS = static_cast<float>(FrameCount) / AccumulatedTime;
    }
    else
    {
        CurrentData.CurrentFPS = 0.0f;
    }
    
    // Add to FPS history
    FPSHistory.Add(CurrentData.CurrentFPS);
    if (FPSHistory.Num() > MAX_HISTORY_SIZE)
    {
        FPSHistory.RemoveAt(0);
    }
    
    // Calculate memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentData.MemoryUsageMB = static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
    
    // Add to memory history
    MemoryHistory.Add(CurrentData.MemoryUsageMB);
    if (MemoryHistory.Num() > MAX_HISTORY_SIZE)
    {
        MemoryHistory.RemoveAt(0);
    }
    
    // Simulate other metrics (would be real data in production)
    CurrentData.CPUUsagePercent = FMath::RandRange(20.0f, 80.0f);
    CurrentData.GPUUsagePercent = FMath::RandRange(30.0f, 90.0f);
    CurrentData.DrawCalls = FMath::RandRange(100, 2000);
    CurrentData.TriangleCount = FMath::RandRange(10000, 500000);
    
    // Calculate averages
    CalculateAverages();
    
    // Check thresholds
    CheckPerformanceThresholds();
}

void UEng_PerformanceMonitor::CalculateAverages()
{
    // Calculate average FPS
    if (FPSHistory.Num() > 0)
    {
        float Sum = 0.0f;
        for (float FPS : FPSHistory)
        {
            Sum += FPS;
        }
        CurrentData.AverageFPS = Sum / static_cast<float>(FPSHistory.Num());
    }
    else
    {
        CurrentData.AverageFPS = CurrentData.CurrentFPS;
    }
}

void UEng_PerformanceMonitor::CheckPerformanceThresholds()
{
    if (IsPerformanceBelowThreshold())
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Monitor: Performance below threshold!"));
        UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.2f (Min: %.2f)"), 
            CurrentData.CurrentFPS, MinimumFPSThreshold);
        UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB (Max: %.2f MB)"), 
            CurrentData.MemoryUsageMB, MaximumMemoryThreshold);
    }
}