#include "PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "RenderingThread.h"
#include "RHI.h"

DEFINE_LOG_CATEGORY(LogTranspersonalProfiler);

UTranspersonalPerformanceProfiler::UTranspersonalPerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    // Initialize performance thresholds
    TargetFPS = 60.0f;
    MinimumFPS = 30.0f;
    MaxMemoryUsageMB = 8192.0f;
    MaxGPUMemoryUsageMB = 4096.0f;
    
    // Initialize tracking arrays
    FrameTimeHistory.Reserve(600); // 60 seconds at 10Hz
    MemoryUsageHistory.Reserve(600);
    GPUTimeHistory.Reserve(600);
    
    bIsProfilingActive = false;
    bLogToFile = true;
    
    ProfileStartTime = 0.0;
    TotalFramesCounted = 0;
    
    // Performance counters
    AverageFrameTime = 0.0f;
    MinFrameTime = FLT_MAX;
    MaxFrameTime = 0.0f;
    
    CurrentMemoryUsageMB = 0.0f;
    PeakMemoryUsageMB = 0.0f;
    
    CurrentGPUTime = 0.0f;
    AverageGPUTime = 0.0f;
}

void UTranspersonalPerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTranspersonalProfiler, Warning, TEXT("Performance Profiler initialized"));
    
    // Start profiling automatically if enabled
    if (bAutoStartProfiling)
    {
        StartProfiling();
    }
}

void UTranspersonalPerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsProfilingActive)
    {
        UpdatePerformanceMetrics(DeltaTime);
    }
}

void UTranspersonalPerformanceProfiler::StartProfiling()
{
    if (bIsProfilingActive)
    {
        UE_LOG(LogTranspersonalProfiler, Warning, TEXT("Profiling already active"));
        return;
    }
    
    bIsProfilingActive = true;
    ProfileStartTime = FPlatformTime::Seconds();
    TotalFramesCounted = 0;
    
    // Clear previous data
    FrameTimeHistory.Empty();
    MemoryUsageHistory.Empty();
    GPUTimeHistory.Empty();
    
    // Reset counters
    AverageFrameTime = 0.0f;
    MinFrameTime = FLT_MAX;
    MaxFrameTime = 0.0f;
    CurrentMemoryUsageMB = 0.0f;
    PeakMemoryUsageMB = 0.0f;
    CurrentGPUTime = 0.0f;
    AverageGPUTime = 0.0f;
    
    UE_LOG(LogTranspersonalProfiler, Warning, TEXT("Performance profiling started"));
    
    // Broadcast profiling started event
    OnProfilingStarted.Broadcast();
}

void UTranspersonalPerformanceProfiler::StopProfiling()
{
    if (!bIsProfilingActive)
    {
        UE_LOG(LogTranspersonalProfiler, Warning, TEXT("Profiling not active"));
        return;
    }
    
    bIsProfilingActive = false;
    double ProfileEndTime = FPlatformTime::Seconds();
    double TotalProfileTime = ProfileEndTime - ProfileStartTime;
    
    // Generate final report
    GeneratePerformanceReport(TotalProfileTime);
    
    UE_LOG(LogTranspersonalProfiler, Warning, TEXT("Performance profiling stopped after %.2f seconds"), TotalProfileTime);
    
    // Broadcast profiling stopped event
    OnProfilingStopped.Broadcast();
}

void UTranspersonalPerformanceProfiler::UpdatePerformanceMetrics(float DeltaTime)
{
    TotalFramesCounted++;
    
    // Update frame time metrics
    float CurrentFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    FrameTimeHistory.Add(CurrentFrameTime);
    
    // Update frame time statistics
    if (CurrentFrameTime < MinFrameTime)
    {
        MinFrameTime = CurrentFrameTime;
    }
    if (CurrentFrameTime > MaxFrameTime)
    {
        MaxFrameTime = CurrentFrameTime;
    }
    
    // Calculate running average
    float TotalFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
    
    // Update memory metrics
    UpdateMemoryMetrics();
    
    // Update GPU metrics
    UpdateGPUMetrics();
    
    // Check for performance warnings
    CheckPerformanceThresholds();
    
    // Limit history size to prevent memory bloat
    const int32 MaxHistorySize = 600; // 60 seconds at 10Hz
    if (FrameTimeHistory.Num() > MaxHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    if (MemoryUsageHistory.Num() > MaxHistorySize)
    {
        MemoryUsageHistory.RemoveAt(0);
    }
    if (GPUTimeHistory.Num() > MaxHistorySize)
    {
        GPUTimeHistory.RemoveAt(0);
    }
}

void UTranspersonalPerformanceProfiler::UpdateMemoryMetrics()
{
    // Get platform memory statistics
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    CurrentMemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    MemoryUsageHistory.Add(CurrentMemoryUsageMB);
    
    if (CurrentMemoryUsageMB > PeakMemoryUsageMB)
    {
        PeakMemoryUsageMB = CurrentMemoryUsageMB;
    }
}

void UTranspersonalPerformanceProfiler::UpdateGPUMetrics()
{
    // Get GPU timing information
    if (GRHISupportsGPUTimestamps)
    {
        // This would require access to render thread timing
        // For now, we'll use a placeholder implementation
        CurrentGPUTime = 16.67f; // Placeholder: 60fps target
        GPUTimeHistory.Add(CurrentGPUTime);
        
        // Calculate average GPU time
        float TotalGPUTime = 0.0f;
        for (float GPUTime : GPUTimeHistory)
        {
            TotalGPUTime += GPUTime;
        }
        AverageGPUTime = TotalGPUTime / GPUTimeHistory.Num();
    }
}

void UTranspersonalPerformanceProfiler::CheckPerformanceThresholds()
{
    // Check frame rate threshold
    float CurrentFPS = 1000.0f / AverageFrameTime;
    if (CurrentFPS < MinimumFPS)
    {
        FPerformanceWarning Warning;
        Warning.WarningType = EPerformanceWarningType::LowFrameRate;
        Warning.Message = FString::Printf(TEXT("Low frame rate detected: %.2f FPS (Target: %.2f)"), CurrentFPS, TargetFPS);
        Warning.Severity = EPerformanceWarningSeverity::High;
        Warning.Timestamp = FDateTime::Now();
        
        PerformanceWarnings.Add(Warning);
        
        UE_LOG(LogTranspersonalProfiler, Warning, TEXT("%s"), *Warning.Message);
        
        // Broadcast warning event
        OnPerformanceWarning.Broadcast(Warning);
    }
    
    // Check memory threshold
    if (CurrentMemoryUsageMB > MaxMemoryUsageMB)
    {
        FPerformanceWarning Warning;
        Warning.WarningType = EPerformanceWarningType::HighMemoryUsage;
        Warning.Message = FString::Printf(TEXT("High memory usage: %.2f MB (Limit: %.2f MB)"), CurrentMemoryUsageMB, MaxMemoryUsageMB);
        Warning.Severity = EPerformanceWarningSeverity::Critical;
        Warning.Timestamp = FDateTime::Now();
        
        PerformanceWarnings.Add(Warning);
        
        UE_LOG(LogTranspersonalProfiler, Error, TEXT("%s"), *Warning.Message);
        
        // Broadcast warning event
        OnPerformanceWarning.Broadcast(Warning);
    }
    
    // Check GPU time threshold
    float GPUTargetTime = 1000.0f / TargetFPS; // Target GPU time in ms
    if (CurrentGPUTime > GPUTargetTime * 1.5f) // 50% over target
    {
        FPerformanceWarning Warning;
        Warning.WarningType = EPerformanceWarningType::HighGPUUsage;
        Warning.Message = FString::Printf(TEXT("High GPU time: %.2f ms (Target: %.2f ms)"), CurrentGPUTime, GPUTargetTime);
        Warning.Severity = EPerformanceWarningSeverity::Medium;
        Warning.Timestamp = FDateTime::Now();
        
        PerformanceWarnings.Add(Warning);
        
        UE_LOG(LogTranspersonalProfiler, Warning, TEXT("%s"), *Warning.Message);
        
        // Broadcast warning event
        OnPerformanceWarning.Broadcast(Warning);
    }
}

void UTranspersonalPerformanceProfiler::GeneratePerformanceReport(double ProfileDuration)
{
    FString ReportContent;
    
    // Header
    ReportContent += TEXT("=== TRANSPERSONAL GAME PERFORMANCE REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    ReportContent += FString::Printf(TEXT("Profile Duration: %.2f seconds\n"), ProfileDuration);
    ReportContent += FString::Printf(TEXT("Total Frames: %d\n"), TotalFramesCounted);
    ReportContent += TEXT("\n");
    
    // Frame Rate Statistics
    ReportContent += TEXT("=== FRAME RATE STATISTICS ===\n");
    float AverageFPS = 1000.0f / AverageFrameTime;
    float MinFPS = 1000.0f / MaxFrameTime;
    float MaxFPS = 1000.0f / MinFrameTime;
    
    ReportContent += FString::Printf(TEXT("Average FPS: %.2f\n"), AverageFPS);
    ReportContent += FString::Printf(TEXT("Minimum FPS: %.2f\n"), MinFPS);
    ReportContent += FString::Printf(TEXT("Maximum FPS: %.2f\n"), MaxFPS);
    ReportContent += FString::Printf(TEXT("Average Frame Time: %.2f ms\n"), AverageFrameTime);
    ReportContent += FString::Printf(TEXT("Min Frame Time: %.2f ms\n"), MinFrameTime);
    ReportContent += FString::Printf(TEXT("Max Frame Time: %.2f ms\n"), MaxFrameTime);
    ReportContent += TEXT("\n");
    
    // Memory Statistics
    ReportContent += TEXT("=== MEMORY STATISTICS ===\n");
    ReportContent += FString::Printf(TEXT("Current Memory Usage: %.2f MB\n"), CurrentMemoryUsageMB);
    ReportContent += FString::Printf(TEXT("Peak Memory Usage: %.2f MB\n"), PeakMemoryUsageMB);
    ReportContent += TEXT("\n");
    
    // GPU Statistics
    ReportContent += TEXT("=== GPU STATISTICS ===\n");
    ReportContent += FString::Printf(TEXT("Current GPU Time: %.2f ms\n"), CurrentGPUTime);
    ReportContent += FString::Printf(TEXT("Average GPU Time: %.2f ms\n"), AverageGPUTime);
    ReportContent += TEXT("\n");
    
    // Performance Warnings
    if (PerformanceWarnings.Num() > 0)
    {
        ReportContent += TEXT("=== PERFORMANCE WARNINGS ===\n");
        for (const FPerformanceWarning& Warning : PerformanceWarnings)
        {
            FString SeverityStr;
            switch (Warning.Severity)
            {
                case EPerformanceWarningSeverity::Low: SeverityStr = TEXT("LOW"); break;
                case EPerformanceWarningSeverity::Medium: SeverityStr = TEXT("MEDIUM"); break;
                case EPerformanceWarningSeverity::High: SeverityStr = TEXT("HIGH"); break;
                case EPerformanceWarningSeverity::Critical: SeverityStr = TEXT("CRITICAL"); break;
            }
            
            ReportContent += FString::Printf(TEXT("[%s] %s - %s\n"), 
                *SeverityStr, *Warning.Timestamp.ToString(), *Warning.Message);
        }
        ReportContent += TEXT("\n");
    }
    
    // System Information
    ReportContent += TEXT("=== SYSTEM INFORMATION ===\n");
    ReportContent += FString::Printf(TEXT("Platform: %s\n"), *FPlatformProperties::PlatformName());
    ReportContent += FString::Printf(TEXT("CPU Cores: %d\n"), FPlatformMisc::NumberOfCores());
    ReportContent += FString::Printf(TEXT("CPU Cores (Logical): %d\n"), FPlatformMisc::NumberOfCoresIncludingHyperthreads());
    
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    ReportContent += FString::Printf(TEXT("Total Physical Memory: %.2f GB\n"), MemStats.TotalPhysical / (1024.0f * 1024.0f * 1024.0f));
    ReportContent += FString::Printf(TEXT("Available Physical Memory: %.2f GB\n"), MemStats.AvailablePhysical / (1024.0f * 1024.0f * 1024.0f));
    
    // Log the report
    UE_LOG(LogTranspersonalProfiler, Warning, TEXT("Performance Report Generated:\n%s"), *ReportContent);
    
    // Save to file if enabled
    if (bLogToFile)
    {
        SaveReportToFile(ReportContent);
    }
    
    // Store the report for Blueprint access
    LastGeneratedReport = ReportContent;
}

void UTranspersonalPerformanceProfiler::SaveReportToFile(const FString& ReportContent)
{
    FString FileName = FString::Printf(TEXT("PerformanceReport_%s.txt"), 
        *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    
    FString FilePath = FPaths::ProjectSavedDir() / TEXT("Profiling") / FileName;
    
    // Ensure directory exists
    FString Directory = FPaths::GetPath(FilePath);
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*Directory))
    {
        PlatformFile.CreateDirectoryTree(*Directory);
    }
    
    // Write file
    if (FFileHelper::SaveStringToFile(ReportContent, *FilePath))
    {
        UE_LOG(LogTranspersonalProfiler, Warning, TEXT("Performance report saved to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTranspersonalProfiler, Error, TEXT("Failed to save performance report to: %s"), *FilePath);
    }
}

float UTranspersonalPerformanceProfiler::GetCurrentFPS() const
{
    if (AverageFrameTime > 0.0f)
    {
        return 1000.0f / AverageFrameTime;
    }
    return 0.0f;
}

float UTranspersonalPerformanceProfiler::GetAverageFrameTime() const
{
    return AverageFrameTime;
}

float UTranspersonalPerformanceProfiler::GetCurrentMemoryUsage() const
{
    return CurrentMemoryUsageMB;
}

float UTranspersonalPerformanceProfiler::GetPeakMemoryUsage() const
{
    return PeakMemoryUsageMB;
}

TArray<float> UTranspersonalPerformanceProfiler::GetFrameTimeHistory() const
{
    return FrameTimeHistory;
}

TArray<float> UTranspersonalPerformanceProfiler::GetMemoryUsageHistory() const
{
    return MemoryUsageHistory;
}

bool UTranspersonalPerformanceProfiler::IsProfilingActive() const
{
    return bIsProfilingActive;
}

FString UTranspersonalPerformanceProfiler::GetLastReport() const
{
    return LastGeneratedReport;
}