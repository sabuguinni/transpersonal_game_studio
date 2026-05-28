#include "Perf_RealTimeProfiler.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "TranspersonalGame.h"

DEFINE_LOG_CATEGORY_STATIC(LogPerfRealTimeProfiler, Log, All);

UPerf_RealTimeProfiler::UPerf_RealTimeProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    // Initialize performance thresholds
    FrameTimeThreshold = 16.67f; // 60 FPS target
    MemoryThreshold = 8192.0f; // 8GB threshold
    DrawCallThreshold = 5000;
    TriangleThreshold = 2000000; // 2M triangles
    
    bIsProfilingActive = false;
    bAutoSaveEnabled = true;
    ProfilingDuration = 60.0f; // Default 1 minute sessions
    
    // Initialize performance data
    CurrentFrameData = FPerf_FrameData();
    PerformanceHistory.Reserve(600); // 1 minute at 10Hz
    
    UE_LOG(LogPerfRealTimeProfiler, Log, TEXT("Real-time Performance Profiler initialized"));
}

void UPerf_RealTimeProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    // Start profiling automatically if enabled
    if (bAutoStartProfiling)
    {
        StartProfiling();
    }
}

void UPerf_RealTimeProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsProfilingActive)
        return;
        
    // Update profiling timer
    CurrentProfilingTime += DeltaTime;
    
    // Collect current frame data
    CollectFrameData();
    
    // Add to history
    PerformanceHistory.Add(CurrentFrameData);
    
    // Limit history size
    if (PerformanceHistory.Num() > 600)
    {
        PerformanceHistory.RemoveAt(0);
    }
    
    // Check for performance alerts
    CheckPerformanceAlerts();
    
    // Auto-stop if duration reached
    if (ProfilingDuration > 0.0f && CurrentProfilingTime >= ProfilingDuration)
    {
        StopProfiling();
    }
}

void UPerf_RealTimeProfiler::StartProfiling()
{
    if (bIsProfilingActive)
    {
        UE_LOG(LogPerfRealTimeProfiler, Warning, TEXT("Profiling already active"));
        return;
    }
    
    bIsProfilingActive = true;
    CurrentProfilingTime = 0.0f;
    PerformanceHistory.Empty();
    
    UE_LOG(LogPerfRealTimeProfiler, Log, TEXT("Real-time profiling started"));
    
    // Broadcast profiling started event
    OnProfilingStarted.Broadcast();
}

void UPerf_RealTimeProfiler::StopProfiling()
{
    if (!bIsProfilingActive)
    {
        UE_LOG(LogPerfRealTimeProfiler, Warning, TEXT("Profiling not active"));
        return;
    }
    
    bIsProfilingActive = false;
    
    UE_LOG(LogPerfRealTimeProfiler, Log, TEXT("Real-time profiling stopped. Collected %d frames"), PerformanceHistory.Num());
    
    // Auto-save if enabled
    if (bAutoSaveEnabled)
    {
        SaveProfilingData();
    }
    
    // Broadcast profiling stopped event
    OnProfilingStopped.Broadcast();
}

void UPerf_RealTimeProfiler::CollectFrameData()
{
    // Reset frame data
    CurrentFrameData = FPerf_FrameData();
    CurrentFrameData.Timestamp = FDateTime::Now();
    
    // Get frame time
    CurrentFrameData.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    CurrentFrameData.FPS = 1.0f / FApp::GetDeltaTime();
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentFrameData.UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    CurrentFrameData.AvailableMemoryMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
    
    // Get rendering stats
    if (GEngine && GEngine->GetWorld())
    {
        UWorld* World = GEngine->GetWorld();
        if (World && World->Scene)
        {
            // Get primitive count (approximate)
            CurrentFrameData.DrawCalls = World->Scene->GetNumPrimitivesInScene();
            
            // Estimate triangle count (rough approximation)
            CurrentFrameData.TriangleCount = CurrentFrameData.DrawCalls * 500; // Rough estimate
        }
    }
    
    // Get CPU usage (simplified)
    CurrentFrameData.CPUUsage = FPlatformTime::GetCPUTime().CPUTimePct;
    
    // GPU usage is harder to get in real-time, set to 0 for now
    CurrentFrameData.GPUUsage = 0.0f;
}

void UPerf_RealTimeProfiler::CheckPerformanceAlerts()
{
    bool bHasAlert = false;
    FString AlertMessage;
    
    // Check frame time
    if (CurrentFrameData.FrameTime > FrameTimeThreshold)
    {
        bHasAlert = true;
        AlertMessage += FString::Printf(TEXT("High frame time: %.2fms (threshold: %.2fms); "), 
            CurrentFrameData.FrameTime, FrameTimeThreshold);
    }
    
    // Check memory usage
    if (CurrentFrameData.UsedMemoryMB > MemoryThreshold)
    {
        bHasAlert = true;
        AlertMessage += FString::Printf(TEXT("High memory usage: %.2fMB (threshold: %.2fMB); "), 
            CurrentFrameData.UsedMemoryMB, MemoryThreshold);
    }
    
    // Check draw calls
    if (CurrentFrameData.DrawCalls > DrawCallThreshold)
    {
        bHasAlert = true;
        AlertMessage += FString::Printf(TEXT("High draw calls: %d (threshold: %d); "), 
            CurrentFrameData.DrawCalls, DrawCallThreshold);
    }
    
    // Check triangle count
    if (CurrentFrameData.TriangleCount > TriangleThreshold)
    {
        bHasAlert = true;
        AlertMessage += FString::Printf(TEXT("High triangle count: %d (threshold: %d); "), 
            CurrentFrameData.TriangleCount, TriangleThreshold);
    }
    
    if (bHasAlert)
    {
        UE_LOG(LogPerfRealTimeProfiler, Warning, TEXT("Performance Alert: %s"), *AlertMessage);
        OnPerformanceAlert.Broadcast(AlertMessage);
    }
}

FPerf_PerformanceSnapshot UPerf_RealTimeProfiler::GetCurrentSnapshot() const
{
    FPerf_PerformanceSnapshot Snapshot;
    Snapshot.CurrentFrameData = CurrentFrameData;
    Snapshot.IsProfilingActive = bIsProfilingActive;
    Snapshot.ProfilingDuration = CurrentProfilingTime;
    
    // Calculate averages from recent history
    if (PerformanceHistory.Num() > 0)
    {
        float TotalFrameTime = 0.0f;
        float TotalMemory = 0.0f;
        int32 TotalDrawCalls = 0;
        
        int32 SampleCount = FMath::Min(60, PerformanceHistory.Num()); // Last 6 seconds
        for (int32 i = PerformanceHistory.Num() - SampleCount; i < PerformanceHistory.Num(); i++)
        {
            TotalFrameTime += PerformanceHistory[i].FrameTime;
            TotalMemory += PerformanceHistory[i].UsedMemoryMB;
            TotalDrawCalls += PerformanceHistory[i].DrawCalls;
        }
        
        Snapshot.AverageFrameTime = TotalFrameTime / SampleCount;
        Snapshot.AverageMemoryUsage = TotalMemory / SampleCount;
        Snapshot.AverageDrawCalls = TotalDrawCalls / SampleCount;
        Snapshot.AverageFPS = 1000.0f / Snapshot.AverageFrameTime;
    }
    
    return Snapshot;
}

TArray<FPerf_FrameData> UPerf_RealTimeProfiler::GetPerformanceHistory() const
{
    return PerformanceHistory;
}

void UPerf_RealTimeProfiler::SaveProfilingData()
{
    if (PerformanceHistory.Num() == 0)
    {
        UE_LOG(LogPerfRealTimeProfiler, Warning, TEXT("No profiling data to save"));
        return;
    }
    
    FString SavePath = FPaths::ProjectSavedDir() / TEXT("Profiling");
    FString FileName = FString::Printf(TEXT("PerformanceProfile_%s.csv"), 
        *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    FString FullPath = SavePath / FileName;
    
    // Create directory if it doesn't exist
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    PlatformFile.CreateDirectoryTree(*SavePath);
    
    // Build CSV content
    FString CSVContent;
    CSVContent += TEXT("Timestamp,FrameTime(ms),FPS,UsedMemory(MB),AvailableMemory(MB),DrawCalls,TriangleCount,CPUUsage,GPUUsage\n");
    
    for (const FPerf_FrameData& FrameData : PerformanceHistory)
    {
        CSVContent += FString::Printf(TEXT("%s,%.3f,%.2f,%.2f,%.2f,%d,%d,%.2f,%.2f\n"),
            *FrameData.Timestamp.ToString(),
            FrameData.FrameTime,
            FrameData.FPS,
            FrameData.UsedMemoryMB,
            FrameData.AvailableMemoryMB,
            FrameData.DrawCalls,
            FrameData.TriangleCount,
            FrameData.CPUUsage,
            FrameData.GPUUsage);
    }
    
    // Save to file
    if (FFileHelper::SaveStringToFile(CSVContent, *FullPath))
    {
        UE_LOG(LogPerfRealTimeProfiler, Log, TEXT("Profiling data saved to: %s"), *FullPath);
    }
    else
    {
        UE_LOG(LogPerfRealTimeProfiler, Error, TEXT("Failed to save profiling data to: %s"), *FullPath);
    }
}

void UPerf_RealTimeProfiler::SetProfilingThresholds(float InFrameTimeThreshold, float InMemoryThreshold, 
    int32 InDrawCallThreshold, int32 InTriangleThreshold)
{
    FrameTimeThreshold = InFrameTimeThreshold;
    MemoryThreshold = InMemoryThreshold;
    DrawCallThreshold = InDrawCallThreshold;
    TriangleThreshold = InTriangleThreshold;
    
    UE_LOG(LogPerfRealTimeProfiler, Log, TEXT("Performance thresholds updated"));
}

bool UPerf_RealTimeProfiler::IsProfilingActive() const
{
    return bIsProfilingActive;
}

float UPerf_RealTimeProfiler::GetCurrentProfilingTime() const
{
    return CurrentProfilingTime;
}