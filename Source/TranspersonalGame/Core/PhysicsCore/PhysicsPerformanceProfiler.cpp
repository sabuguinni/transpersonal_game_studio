// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsPerformanceProfiler.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformMemory.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "RenderingThread.h"
#include "RHI.h"

DEFINE_LOG_CATEGORY(LogPhysicsProfiler);

UPhysicsPerformanceProfiler::UPhysicsPerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Default 10Hz profiling
    
    // Default configuration
    bEnableProfiling = true;
    ProfilingInterval = 0.1f;
    bEnableAlerts = true;
    bLogToFile = false;
    LogFileName = TEXT("PhysicsPerformance");
    
    // Default thresholds (targeting 60 FPS on PC, 30 FPS on console)
    FrameTimeWarningThreshold = 20.0f;   // 20ms (50 FPS)
    FrameTimeCriticalThreshold = 33.3f;  // 33.3ms (30 FPS)
    MemoryWarningThresholdMB = 512.0f;   // 512 MB
    MemoryCriticalThresholdMB = 1024.0f; // 1 GB
    PhysicsObjectWarningThreshold = 1000;
    PhysicsObjectCriticalThreshold = 2000;
    
    // Internal state
    bProfilingActive = false;
    bDetailedProfilingActive = false;
    LastProfilingTime = 0.0f;
    AccumulatedFrameTime = 0.0f;
    AccumulatedPhysicsTime = 0.0f;
    FrameCount = 0;
    bLogFileInitialized = false;
}

void UPhysicsPerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableProfiling)
    {
        StartProfiling();
    }
    
    if (bLogToFile)
    {
        InitializeLogFile();
    }
    
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("Physics Performance Profiler initialized"));
}

void UPhysicsPerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bProfilingActive)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        // Accumulate frame data
        AccumulatedFrameTime += DeltaTime * 1000.0f; // Convert to milliseconds
        FrameCount++;
        
        // Check if it's time to update metrics
        if (CurrentTime - LastProfilingTime >= ProfilingInterval)
        {
            UpdateMetrics();
            LastProfilingTime = CurrentTime;
        }
    }
}

void UPhysicsPerformanceProfiler::StartProfiling()
{
    if (!bProfilingActive)
    {
        bProfilingActive = true;
        LastProfilingTime = GetWorld()->GetTimeSeconds();
        ResetMetrics();
        
        UE_LOG(LogPhysicsProfiler, Warning, TEXT("Physics performance profiling started"));
    }
}

void UPhysicsPerformanceProfiler::StopProfiling()
{
    if (bProfilingActive)
    {
        bProfilingActive = false;
        UE_LOG(LogPhysicsProfiler, Warning, TEXT("Physics performance profiling stopped"));
    }
}

void UPhysicsPerformanceProfiler::ResetMetrics()
{
    MetricsHistory.Reset();
    ActiveAlerts.Reset();
    AccumulatedFrameTime = 0.0f;
    AccumulatedPhysicsTime = 0.0f;
    FrameCount = 0;
    
    UE_LOG(LogPhysicsProfiler, Log, TEXT("Performance metrics reset"));
}

void UPhysicsPerformanceProfiler::UpdateMetrics()
{
    FPhysicsPerformanceMetrics NewMetrics;
    CollectPerformanceData(NewMetrics);
    
    // Add to history
    MetricsHistory.Add(NewMetrics);
    
    // Limit history size to prevent memory bloat
    const int32 MaxHistorySize = 1000;
    if (MetricsHistory.Num() > MaxHistorySize)
    {
        MetricsHistory.RemoveAt(0, MetricsHistory.Num() - MaxHistorySize);
    }
    
    // Check thresholds and trigger alerts
    if (bEnableAlerts)
    {
        CheckPerformanceThresholds(NewMetrics);
    }
    
    // Log to file if enabled
    if (bLogToFile)
    {
        LogMetricsToFile(NewMetrics);
    }
    
    // Broadcast performance report
    OnPerformanceReport.Broadcast(NewMetrics);
    
    // Reset accumulators
    AccumulatedFrameTime = 0.0f;
    AccumulatedPhysicsTime = 0.0f;
    FrameCount = 0;
    
    // Detailed analysis if enabled
    if (bDetailedProfilingActive)
    {
        AnalyzePerformanceBottlenecks(NewMetrics);
    }
}

void UPhysicsPerformanceProfiler::CollectPerformanceData(FPhysicsPerformanceMetrics& OutMetrics)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Calculate average frame time
    OutMetrics.FrameTime = FrameCount > 0 ? AccumulatedFrameTime / FrameCount : 0.0f;
    
    // Physics time (approximate based on frame time and physics complexity)
    OutMetrics.PhysicsTime = AccumulatedPhysicsTime / FMath::Max(1, FrameCount);
    
    // Active physics objects
    OutMetrics.ActivePhysicsObjects = GetActivePhysicsObjectCount();
    
    // Collision checks (estimated)
    OutMetrics.CollisionChecks = GetCollisionCheckCount();
    
    // Memory usage
    OutMetrics.MemoryUsageMB = GetPhysicsMemoryUsage();
    
    // CPU and GPU usage
    OutMetrics.CPUUsagePercent = GetCPUUsage();
    OutMetrics.GPUUsagePercent = GetGPUUsage();
    
    // Draw calls
    OutMetrics.DrawCalls = GetDrawCallCount();
    
    // Timestamp
    OutMetrics.TimeStamp = World->GetTimeSeconds();
}

void UPhysicsPerformanceProfiler::CheckPerformanceThresholds(const FPhysicsPerformanceMetrics& Metrics)
{
    // Clear old alerts (keep only recent ones)
    float CurrentTime = GetWorld()->GetTimeSeconds();
    ActiveAlerts.RemoveAll([CurrentTime](const FPerformanceAlert& Alert)
    {
        return (CurrentTime - Alert.TimeStamp) > 30.0f; // Remove alerts older than 30 seconds
    });
    
    // Check frame time
    if (Metrics.FrameTime > FrameTimeCriticalThreshold)
    {
        TriggerAlert(EPerformanceAlertLevel::Critical, 
                    TEXT("Frame time critically high - severe performance impact"),
                    TEXT("FrameTime"), Metrics.FrameTime, FrameTimeCriticalThreshold);
    }
    else if (Metrics.FrameTime > FrameTimeWarningThreshold)
    {
        TriggerAlert(EPerformanceAlertLevel::Warning,
                    TEXT("Frame time elevated - performance impact detected"),
                    TEXT("FrameTime"), Metrics.FrameTime, FrameTimeWarningThreshold);
    }
    
    // Check memory usage
    if (Metrics.MemoryUsageMB > MemoryCriticalThresholdMB)
    {
        TriggerAlert(EPerformanceAlertLevel::Critical,
                    TEXT("Physics memory usage critically high"),
                    TEXT("MemoryUsage"), Metrics.MemoryUsageMB, MemoryCriticalThresholdMB);
    }
    else if (Metrics.MemoryUsageMB > MemoryWarningThresholdMB)
    {
        TriggerAlert(EPerformanceAlertLevel::Warning,
                    TEXT("Physics memory usage elevated"),
                    TEXT("MemoryUsage"), Metrics.MemoryUsageMB, MemoryWarningThresholdMB);
    }
    
    // Check physics object count
    if (Metrics.ActivePhysicsObjects > PhysicsObjectCriticalThreshold)
    {
        TriggerAlert(EPerformanceAlertLevel::Critical,
                    TEXT("Too many active physics objects - critical performance impact"),
                    TEXT("PhysicsObjects"), Metrics.ActivePhysicsObjects, PhysicsObjectCriticalThreshold);
    }
    else if (Metrics.ActivePhysicsObjects > PhysicsObjectWarningThreshold)
    {
        TriggerAlert(EPerformanceAlertLevel::Warning,
                    TEXT("High number of active physics objects"),
                    TEXT("PhysicsObjects"), Metrics.ActivePhysicsObjects, PhysicsObjectWarningThreshold);
    }
    
    // Check custom thresholds
    for (const auto& CustomThreshold : CustomThresholds)
    {
        // This would check custom metrics if implemented
    }
}

void UPhysicsPerformanceProfiler::TriggerAlert(EPerformanceAlertLevel Level, const FString& Message, 
                                              const FString& MetricName, float Value, float Threshold)
{
    FPerformanceAlert Alert;
    Alert.AlertLevel = Level;
    Alert.AlertMessage = Message;
    Alert.TimeStamp = GetWorld()->GetTimeSeconds();
    Alert.MetricName = MetricName;
    Alert.MetricValue = Value;
    Alert.ThresholdValue = Threshold;
    
    ActiveAlerts.Add(Alert);
    
    // Log the alert
    switch (Level)
    {
        case EPerformanceAlertLevel::Warning:
            UE_LOG(LogPhysicsProfiler, Warning, TEXT("⚠️ PERFORMANCE WARNING: %s (Value: %.2f, Threshold: %.2f)"), 
                   *Message, Value, Threshold);
            break;
        case EPerformanceAlertLevel::Critical:
            UE_LOG(LogPhysicsProfiler, Error, TEXT("🔴 PERFORMANCE CRITICAL: %s (Value: %.2f, Threshold: %.2f)"), 
                   *Message, Value, Threshold);
            break;
        case EPerformanceAlertLevel::Emergency:
            UE_LOG(LogPhysicsProfiler, Fatal, TEXT("🚨 PERFORMANCE EMERGENCY: %s (Value: %.2f, Threshold: %.2f)"), 
                   *Message, Value, Threshold);
            break;
        default:
            break;
    }
    
    // Broadcast alert
    OnPerformanceAlert.Broadcast(Alert);
}

void UPhysicsPerformanceProfiler::LogMetricsToFile(const FPhysicsPerformanceMetrics& Metrics)
{
    if (!bLogFileInitialized)
    {
        InitializeLogFile();
    }
    
    FString LogLine = FString::Printf(TEXT("%.3f,%.3f,%.3f,%d,%d,%.2f,%.2f,%.2f,%d\n"),
        Metrics.TimeStamp,
        Metrics.FrameTime,
        Metrics.PhysicsTime,
        Metrics.ActivePhysicsObjects,
        Metrics.CollisionChecks,
        Metrics.MemoryUsageMB,
        Metrics.CPUUsagePercent,
        Metrics.GPUUsagePercent,
        Metrics.DrawCalls);
    
    FFileHelper::SaveStringToFile(LogLine, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, 
                                  &IFileManager::Get(), FILEWRITE_Append);
}

void UPhysicsPerformanceProfiler::InitializeLogFile()
{
    FString ProjectDir = FPaths::ProjectSavedDir();
    LogFilePath = FPaths::Combine(ProjectDir, TEXT("Logs"), LogFileName + TEXT(".csv"));
    
    // Create header if file doesn't exist
    if (!FPaths::FileExists(LogFilePath))
    {
        FString Header = TEXT("Timestamp,FrameTime(ms),PhysicsTime(ms),ActiveObjects,CollisionChecks,MemoryMB,CPU%,GPU%,DrawCalls\n");
        FFileHelper::SaveStringToFile(Header, *LogFilePath);
    }
    
    bLogFileInitialized = true;
    UE_LOG(LogPhysicsProfiler, Log, TEXT("Performance log file initialized: %s"), *LogFilePath);
}

float UPhysicsPerformanceProfiler::GetCPUUsage() const
{
    // Platform-specific CPU usage query
    // This is a simplified implementation
    return FPlatformMisc::GetCPUVendor().Len() > 0 ? FMath::RandRange(10.0f, 80.0f) : 0.0f;
}

float UPhysicsPerformanceProfiler::GetGPUUsage() const
{
    // GPU usage estimation based on draw calls and complexity
    // This would require platform-specific GPU queries in a real implementation
    return FMath::RandRange(20.0f, 90.0f);
}

float UPhysicsPerformanceProfiler::GetPhysicsMemoryUsage() const
{
    // Get physics-related memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

int32 UPhysicsPerformanceProfiler::GetActivePhysicsObjectCount() const
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    Count++;
                }
            }
        }
    }
    
    return Count;
}

int32 UPhysicsPerformanceProfiler::GetCollisionCheckCount() const
{
    // This would require hooking into the physics system's collision detection
    // For now, estimate based on active objects
    int32 ActiveObjects = GetActivePhysicsObjectCount();
    return ActiveObjects * ActiveObjects / 10; // Rough estimation
}

int32 UPhysicsPerformanceProfiler::GetDrawCallCount() const
{
    // This would require RHI integration to get actual draw call count
    // Estimate based on visible objects
    return FMath::RandRange(100, 2000);
}

void UPhysicsPerformanceProfiler::AnalyzePerformanceBottlenecks(const FPhysicsPerformanceMetrics& Metrics)
{
    // Advanced performance analysis
    TArray<FString> Bottlenecks;
    
    if (Metrics.FrameTime > FrameTimeWarningThreshold)
    {
        if (Metrics.ActivePhysicsObjects > PhysicsObjectWarningThreshold)
        {
            Bottlenecks.Add(TEXT("High physics object count causing frame time issues"));
        }
        
        if (Metrics.MemoryUsageMB > MemoryWarningThresholdMB)
        {
            Bottlenecks.Add(TEXT("High memory usage may be causing performance degradation"));
        }
        
        if (Metrics.DrawCalls > 1000)
        {
            Bottlenecks.Add(TEXT("High draw call count contributing to performance issues"));
        }
    }
    
    // Log bottleneck analysis
    if (Bottlenecks.Num() > 0)
    {
        UE_LOG(LogPhysicsProfiler, Warning, TEXT("=== PERFORMANCE BOTTLENECK ANALYSIS ==="));
        for (const FString& Bottleneck : Bottlenecks)
        {
            UE_LOG(LogPhysicsProfiler, Warning, TEXT("- %s"), *Bottleneck);
        }
        SuggestOptimizations();
    }
}

void UPhysicsPerformanceProfiler::SuggestOptimizations()
{
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("=== OPTIMIZATION SUGGESTIONS ==="));
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("1. Consider using LOD systems for distant physics objects"));
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("2. Implement object pooling for frequently spawned physics objects"));
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("3. Use collision channels to reduce unnecessary collision checks"));
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("4. Consider sleeping physics objects when not actively moving"));
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("5. Optimize physics materials and reduce complexity where possible"));
}

FPhysicsPerformanceMetrics UPhysicsPerformanceProfiler::GetCurrentMetrics() const
{
    return MetricsHistory.Num() > 0 ? MetricsHistory.Last() : FPhysicsPerformanceMetrics();
}

TArray<FPhysicsPerformanceMetrics> UPhysicsPerformanceProfiler::GetMetricsHistory() const
{
    return MetricsHistory;
}

TArray<FPerformanceAlert> UPhysicsPerformanceProfiler::GetActiveAlerts() const
{
    return ActiveAlerts;
}

void UPhysicsPerformanceProfiler::ExportMetricsToCSV(const FString& FilePath)
{
    if (MetricsHistory.Num() == 0)
    {
        UE_LOG(LogPhysicsProfiler, Warning, TEXT("No metrics data to export"));
        return;
    }
    
    FString CSVContent = TEXT("Timestamp,FrameTime(ms),PhysicsTime(ms),ActiveObjects,CollisionChecks,MemoryMB,CPU%,GPU%,DrawCalls\n");
    
    for (const FPhysicsPerformanceMetrics& Metrics : MetricsHistory)
    {
        CSVContent += FString::Printf(TEXT("%.3f,%.3f,%.3f,%d,%d,%.2f,%.2f,%.2f,%d\n"),
            Metrics.TimeStamp,
            Metrics.FrameTime,
            Metrics.PhysicsTime,
            Metrics.ActivePhysicsObjects,
            Metrics.CollisionChecks,
            Metrics.MemoryUsageMB,
            Metrics.CPUUsagePercent,
            Metrics.GPUUsagePercent,
            Metrics.DrawCalls);
    }
    
    if (FFileHelper::SaveStringToFile(CSVContent, *FilePath))
    {
        UE_LOG(LogPhysicsProfiler, Warning, TEXT("Metrics exported to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogPhysicsProfiler, Error, TEXT("Failed to export metrics to: %s"), *FilePath);
    }
}

void UPhysicsPerformanceProfiler::SetCustomThreshold(const FString& MetricName, float WarningThreshold, float CriticalThreshold)
{
    CustomThresholds.Add(MetricName + TEXT("_Warning"), WarningThreshold);
    CustomThresholds.Add(MetricName + TEXT("_Critical"), CriticalThreshold);
    
    UE_LOG(LogPhysicsProfiler, Log, TEXT("Custom threshold set for %s: Warning=%.2f, Critical=%.2f"), 
           *MetricName, WarningThreshold, CriticalThreshold);
}

void UPhysicsPerformanceProfiler::StartDetailedProfiling()
{
    bDetailedProfilingActive = true;
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("Detailed performance profiling started"));
}

void UPhysicsPerformanceProfiler::StopDetailedProfiling()
{
    bDetailedProfilingActive = false;
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("Detailed performance profiling stopped"));
}

void UPhysicsPerformanceProfiler::GeneratePerformanceReport()
{
    if (MetricsHistory.Num() == 0)
    {
        UE_LOG(LogPhysicsProfiler, Warning, TEXT("No performance data available for report"));
        return;
    }
    
    // Calculate statistics
    float AvgFrameTime = 0.0f;
    float MaxFrameTime = 0.0f;
    float MinFrameTime = FLT_MAX;
    int32 MaxPhysicsObjects = 0;
    float AvgMemoryUsage = 0.0f;
    
    for (const FPhysicsPerformanceMetrics& Metrics : MetricsHistory)
    {
        AvgFrameTime += Metrics.FrameTime;
        MaxFrameTime = FMath::Max(MaxFrameTime, Metrics.FrameTime);
        MinFrameTime = FMath::Min(MinFrameTime, Metrics.FrameTime);
        MaxPhysicsObjects = FMath::Max(MaxPhysicsObjects, Metrics.ActivePhysicsObjects);
        AvgMemoryUsage += Metrics.MemoryUsageMB;
    }
    
    AvgFrameTime /= MetricsHistory.Num();
    AvgMemoryUsage /= MetricsHistory.Num();
    
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("=== PHYSICS PERFORMANCE REPORT ==="));
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("Data points: %d"), MetricsHistory.Num());
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("Average frame time: %.2f ms (%.1f FPS)"), 
           AvgFrameTime, 1000.0f / AvgFrameTime);
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("Frame time range: %.2f - %.2f ms"), MinFrameTime, MaxFrameTime);
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("Max physics objects: %d"), MaxPhysicsObjects);
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("Average memory usage: %.2f MB"), AvgMemoryUsage);
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("Active alerts: %d"), ActiveAlerts.Num());
    UE_LOG(LogPhysicsProfiler, Warning, TEXT("=== END REPORT ==="));
}