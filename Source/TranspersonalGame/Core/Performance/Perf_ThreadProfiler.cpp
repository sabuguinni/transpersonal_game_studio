#include "Perf_ThreadProfiler.h"
#include "Engine/Engine.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/ThreadManager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

UPerf_ThreadProfiler::UPerf_ThreadProfiler()
    : ProfilingInterval(1.0f)
    , HighUsageThreshold(70.0f)
    , bIsProfiling(false)
    , LastProfilingTime(0.0)
    , ProfilingStartTime(0.0)
    , SystemCoreCount(0)
    , SystemThreadCount(0)
{
    // Get system information
    SystemCoreCount = FPlatformMisc::NumberOfCores();
    SystemThreadCount = FPlatformMisc::NumberOfWorkerThreadsToSpawn();
    
    UE_LOG(LogTemp, Log, TEXT("ThreadProfiler initialized: %d cores, %d worker threads"), 
           SystemCoreCount, SystemThreadCount);
}

void UPerf_ThreadProfiler::StartProfiling()
{
    if (bIsProfiling)
    {
        UE_LOG(LogTemp, Warning, TEXT("ThreadProfiler already running"));
        return;
    }

    bIsProfiling = true;
    ProfilingStartTime = FPlatformTime::Seconds();
    LastProfilingTime = ProfilingStartTime;
    ThreadDataMap.Empty();
    
    // Capture initial thread state
    CaptureThreadData();
    IdentifyKnownThreads();
    
    UE_LOG(LogTemp, Log, TEXT("ThreadProfiler started - monitoring %d threads"), ThreadDataMap.Num());
}

void UPerf_ThreadProfiler::StopProfiling()
{
    if (!bIsProfiling)
    {
        UE_LOG(LogTemp, Warning, TEXT("ThreadProfiler not running"));
        return;
    }

    bIsProfiling = false;
    double TotalTime = FPlatformTime::Seconds() - ProfilingStartTime;
    
    UE_LOG(LogTemp, Log, TEXT("ThreadProfiler stopped after %.2f seconds"), TotalTime);
    LogThreadReport();
}

void UPerf_ThreadProfiler::UpdateThreadData()
{
    if (!bIsProfiling)
    {
        return;
    }

    double CurrentTime = FPlatformTime::Seconds();
    if (CurrentTime - LastProfilingTime < ProfilingInterval)
    {
        return;
    }

    CaptureThreadData();
    LastProfilingTime = CurrentTime;
}

void UPerf_ThreadProfiler::CaptureThreadData()
{
    // Get current thread list
    TArray<uint32> ThreadIds;
    FPlatformProcess::GetAllThreadIds(ThreadIds);
    
    for (uint32 ThreadId : ThreadIds)
    {
        FString ThreadName = FPlatformProcess::GetThreadName(ThreadId);
        if (ThreadName.IsEmpty())
        {
            ThreadName = FString::Printf(TEXT("Thread_%d"), ThreadId);
        }
        
        FPerf_ThreadData ThreadData = CreateThreadData(ThreadId, ThreadName);
        ThreadDataMap.Add(ThreadId, ThreadData);
    }
    
    // Remove threads that no longer exist
    TArray<uint32> ThreadsToRemove;
    for (auto& Pair : ThreadDataMap)
    {
        if (!ThreadIds.Contains(Pair.Key))
        {
            ThreadsToRemove.Add(Pair.Key);
        }
    }
    
    for (uint32 ThreadId : ThreadsToRemove)
    {
        ThreadDataMap.Remove(ThreadId);
    }
}

FPerf_ThreadData UPerf_ThreadProfiler::CreateThreadData(uint32 ThreadID, const FString& ThreadName) const
{
    FPerf_ThreadData Data;
    Data.ThreadID = ThreadID;
    Data.ThreadName = ThreadName;
    Data.CPUUsagePercent = CalculateThreadCPUUsage(ThreadID);
    Data.Priority = FPlatformProcess::GetThreadPriority(ThreadID);
    Data.bIsActive = Data.CPUUsagePercent > 0.1f;
    Data.LastUpdateTime = FPlatformTime::Seconds();
    Data.ExecutionTimeMS = Data.CPUUsagePercent * 10.0f; // Rough estimate
    
    return Data;
}

float UPerf_ThreadProfiler::CalculateThreadCPUUsage(uint32 ThreadID) const
{
    // Simplified CPU usage calculation
    // In a real implementation, you'd track actual CPU time
    static TMap<uint32, double> LastCPUTimes;
    
    double CurrentTime = FPlatformTime::Seconds();
    double* LastTime = LastCPUTimes.Find(ThreadID);
    
    if (!LastTime)
    {
        LastCPUTimes.Add(ThreadID, CurrentTime);
        return 0.0f;
    }
    
    double TimeDelta = CurrentTime - *LastTime;
    *LastTime = CurrentTime;
    
    // Simulate CPU usage based on thread activity
    if (IsKnownGameThread(ThreadID))
    {
        return FMath::RandRange(60.0f, 85.0f); // Game thread typically high usage
    }
    else if (IsKnownRenderThread(ThreadID))
    {
        return FMath::RandRange(40.0f, 75.0f); // Render thread varies
    }
    else
    {
        return FMath::RandRange(5.0f, 30.0f); // Worker threads lower usage
    }
}

void UPerf_ThreadProfiler::IdentifyKnownThreads()
{
    // Mark known important threads for special handling
    for (auto& Pair : ThreadDataMap)
    {
        FPerf_ThreadData& Data = Pair.Value;
        
        if (Data.ThreadName.Contains(TEXT("GameThread")) || 
            Data.ThreadName.Contains(TEXT("Main")))
        {
            Data.ThreadName += TEXT(" [GAME]");
        }
        else if (Data.ThreadName.Contains(TEXT("RenderThread")) || 
                 Data.ThreadName.Contains(TEXT("Render")))
        {
            Data.ThreadName += TEXT(" [RENDER]");
        }
        else if (Data.ThreadName.Contains(TEXT("Worker")) || 
                 Data.ThreadName.Contains(TEXT("TaskGraph")))
        {
            Data.ThreadName += TEXT(" [WORKER]");
        }
    }
}

bool UPerf_ThreadProfiler::IsKnownGameThread(uint32 ThreadID) const
{
    const FPerf_ThreadData* Data = ThreadDataMap.Find(ThreadID);
    return Data && Data->ThreadName.Contains(TEXT("[GAME]"));
}

bool UPerf_ThreadProfiler::IsKnownRenderThread(uint32 ThreadID) const
{
    const FPerf_ThreadData* Data = ThreadDataMap.Find(ThreadID);
    return Data && Data->ThreadName.Contains(TEXT("[RENDER]"));
}

FPerf_ThreadStats UPerf_ThreadProfiler::GetThreadStats() const
{
    FPerf_ThreadStats Stats;
    
    Stats.TotalThreadCount = ThreadDataMap.Num();
    
    float TotalUsage = 0.0f;
    int32 ActiveCount = 0;
    int32 HighUsageCount = 0;
    
    for (const auto& Pair : ThreadDataMap)
    {
        const FPerf_ThreadData& Data = Pair.Value;
        
        TotalUsage += Data.CPUUsagePercent;
        
        if (Data.bIsActive)
        {
            ActiveCount++;
        }
        
        if (Data.CPUUsagePercent > HighUsageThreshold)
        {
            HighUsageCount++;
        }
        
        // Categorize thread usage
        if (IsKnownGameThread(Data.ThreadID))
        {
            Stats.GameThreadUsage = Data.CPUUsagePercent;
        }
        else if (IsKnownRenderThread(Data.ThreadID))
        {
            Stats.RenderThreadUsage = Data.CPUUsagePercent;
        }
        else
        {
            Stats.WorkerThreadsUsage += Data.CPUUsagePercent;
        }
    }
    
    Stats.ActiveThreadCount = ActiveCount;
    Stats.TotalCPUUsage = TotalUsage;
    Stats.AverageThreadUsage = Stats.TotalThreadCount > 0 ? TotalUsage / Stats.TotalThreadCount : 0.0f;
    Stats.HighUsageThreadCount = HighUsageCount;
    
    return Stats;
}

TArray<FPerf_ThreadData> UPerf_ThreadProfiler::GetAllThreadData() const
{
    TArray<FPerf_ThreadData> Result;
    ThreadDataMap.GenerateValueArray(Result);
    
    // Sort by CPU usage (highest first)
    Result.Sort([](const FPerf_ThreadData& A, const FPerf_ThreadData& B) {
        return A.CPUUsagePercent > B.CPUUsagePercent;
    });
    
    return Result;
}

TArray<FPerf_ThreadData> UPerf_ThreadProfiler::GetHighUsageThreads(float ThresholdPercent) const
{
    TArray<FPerf_ThreadData> Result;
    
    for (const auto& Pair : ThreadDataMap)
    {
        if (Pair.Value.CPUUsagePercent > ThresholdPercent)
        {
            Result.Add(Pair.Value);
        }
    }
    
    // Sort by CPU usage (highest first)
    Result.Sort([](const FPerf_ThreadData& A, const FPerf_ThreadData& B) {
        return A.CPUUsagePercent > B.CPUUsagePercent;
    });
    
    return Result;
}

bool UPerf_ThreadProfiler::IsGameThreadOverloaded(float ThresholdPercent) const
{
    FPerf_ThreadStats Stats = GetThreadStats();
    return Stats.GameThreadUsage > ThresholdPercent;
}

bool UPerf_ThreadProfiler::IsRenderThreadOverloaded(float ThresholdPercent) const
{
    FPerf_ThreadStats Stats = GetThreadStats();
    return Stats.RenderThreadUsage > ThresholdPercent;
}

bool UPerf_ThreadProfiler::AreWorkerThreadsOverloaded(float ThresholdPercent) const
{
    FPerf_ThreadStats Stats = GetThreadStats();
    float AverageWorkerUsage = Stats.ActiveThreadCount > 2 ? 
        Stats.WorkerThreadsUsage / (Stats.ActiveThreadCount - 2) : 0.0f;
    return AverageWorkerUsage > ThresholdPercent;
}

float UPerf_ThreadProfiler::GetCPUCoreUtilization() const
{
    FPerf_ThreadStats Stats = GetThreadStats();
    return SystemCoreCount > 0 ? Stats.TotalCPUUsage / (SystemCoreCount * 100.0f) : 0.0f;
}

int32 UPerf_ThreadProfiler::GetOptimalWorkerThreadCount() const
{
    float CoreUtilization = GetCPUCoreUtilization();
    
    if (CoreUtilization < 0.6f)
    {
        return FMath::Min(SystemCoreCount - 1, SystemThreadCount + 2);
    }
    else if (CoreUtilization > 0.9f)
    {
        return FMath::Max(1, SystemThreadCount - 1);
    }
    
    return SystemThreadCount;
}

TArray<FString> UPerf_ThreadProfiler::GetPerformanceRecommendations() const
{
    TArray<FString> Recommendations;
    FPerf_ThreadStats Stats = GetThreadStats();
    
    if (IsGameThreadOverloaded())
    {
        Recommendations.Add(TEXT("Game thread overloaded - consider moving work to background threads"));
    }
    
    if (IsRenderThreadOverloaded())
    {
        Recommendations.Add(TEXT("Render thread overloaded - reduce draw calls or visual complexity"));
    }
    
    if (AreWorkerThreadsOverloaded())
    {
        Recommendations.Add(TEXT("Worker threads overloaded - consider reducing parallel tasks"));
    }
    
    if (Stats.HighUsageThreadCount > SystemCoreCount)
    {
        Recommendations.Add(TEXT("More high-usage threads than CPU cores - thread contention likely"));
    }
    
    float CoreUtilization = GetCPUCoreUtilization();
    if (CoreUtilization > 0.95f)
    {
        Recommendations.Add(TEXT("CPU utilization critical - reduce overall workload"));
    }
    else if (CoreUtilization < 0.4f)
    {
        Recommendations.Add(TEXT("CPU underutilized - opportunity for performance improvements"));
    }
    
    if (Recommendations.Num() == 0)
    {
        Recommendations.Add(TEXT("Thread performance appears optimal"));
    }
    
    return Recommendations;
}

bool UPerf_ThreadProfiler::ShouldReduceThreadLoad() const
{
    return GetCPUCoreUtilization() > 0.85f || IsGameThreadOverloaded() || IsRenderThreadOverloaded();
}

bool UPerf_ThreadProfiler::ShouldIncreaseThreadCount() const
{
    return GetCPUCoreUtilization() < 0.5f && !AreWorkerThreadsOverloaded();
}

void UPerf_ThreadProfiler::SetProfilingInterval(float IntervalSeconds)
{
    ProfilingInterval = FMath::Max(0.1f, IntervalSeconds);
    UE_LOG(LogTemp, Log, TEXT("ThreadProfiler interval set to %.2f seconds"), ProfilingInterval);
}

void UPerf_ThreadProfiler::SetHighUsageThreshold(float ThresholdPercent)
{
    HighUsageThreshold = FMath::Clamp(ThresholdPercent, 10.0f, 95.0f);
    UE_LOG(LogTemp, Log, TEXT("ThreadProfiler high usage threshold set to %.1f%%"), HighUsageThreshold);
}

void UPerf_ThreadProfiler::LogThreadReport() const
{
    FString Report = GetThreadReportString();
    UE_LOG(LogTemp, Log, TEXT("Thread Performance Report:\n%s"), *Report);
}

FString UPerf_ThreadProfiler::GetThreadReportString() const
{
    FString Report;
    FPerf_ThreadStats Stats = GetThreadStats();
    
    Report += FString::Printf(TEXT("=== THREAD PERFORMANCE REPORT ===\n"));
    Report += FString::Printf(TEXT("Total Threads: %d (Active: %d)\n"), Stats.TotalThreadCount, Stats.ActiveThreadCount);
    Report += FString::Printf(TEXT("System: %d cores, %d worker threads\n"), SystemCoreCount, SystemThreadCount);
    Report += FString::Printf(TEXT("Total CPU Usage: %.1f%% (%.1f%% utilization)\n"), 
                             Stats.TotalCPUUsage, GetCPUCoreUtilization() * 100.0f);
    Report += FString::Printf(TEXT("Game Thread: %.1f%%, Render Thread: %.1f%%\n"), 
                             Stats.GameThreadUsage, Stats.RenderThreadUsage);
    Report += FString::Printf(TEXT("Worker Threads: %.1f%%, Average: %.1f%%\n"), 
                             Stats.WorkerThreadsUsage, Stats.AverageThreadUsage);
    Report += FString::Printf(TEXT("High Usage Threads: %d (>%.1f%%)\n"), 
                             Stats.HighUsageThreadCount, HighUsageThreshold);
    
    Report += TEXT("\nTop 5 CPU Usage Threads:\n");
    TArray<FPerf_ThreadData> TopThreads = GetHighUsageThreads(0.0f);
    for (int32 i = 0; i < FMath::Min(5, TopThreads.Num()); i++)
    {
        const FPerf_ThreadData& Data = TopThreads[i];
        Report += FString::Printf(TEXT("  %d. %s (ID:%d) - %.1f%% CPU\n"), 
                                 i + 1, *Data.ThreadName, Data.ThreadID, Data.CPUUsagePercent);
    }
    
    Report += TEXT("\nRecommendations:\n");
    TArray<FString> Recommendations = GetPerformanceRecommendations();
    for (const FString& Recommendation : Recommendations)
    {
        Report += FString::Printf(TEXT("  - %s\n"), *Recommendation);
    }
    
    return Report;
}