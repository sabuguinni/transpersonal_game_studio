#include "Perf_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"
#include "RenderingThread.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

APerf_PerformanceProfiler::APerf_PerformanceProfiler()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize profiling settings
    ProfilingSettings.bEnableCPUProfiling = true;
    ProfilingSettings.bEnableGPUProfiling = true;
    ProfilingSettings.bEnableMemoryProfiling = true;
    ProfilingSettings.bEnableDrawCallProfiling = true;
    ProfilingSettings.ProfilingInterval = 0.1f;
    ProfilingSettings.MaxSampleHistory = 300; // 30 seconds at 10Hz
    
    // Initialize performance targets
    PerformanceTargets.TargetFPS = 60.0f;
    PerformanceTargets.MinimumFPS = 30.0f;
    PerformanceTargets.MaxFrameTime = 16.67f; // 60 FPS
    PerformanceTargets.MaxGPUTime = 12.0f;
    PerformanceTargets.MaxMemoryUsageMB = 4096.0f;
    PerformanceTargets.MaxDrawCalls = 2000;
    
    // Initialize current metrics
    CurrentProfileData = FPerf_ProfileData();
    
    bIsProfilingActive = false;
    LastProfilingTime = 0.0f;
}

void APerf_PerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoStartProfiling)
    {
        StartProfiling();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler initialized"));
}

void APerf_PerformanceProfiler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bIsProfilingActive)
    {
        StopProfiling();
    }
    
    Super::EndPlay(EndPlayReason);
}

void APerf_PerformanceProfiler::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsProfilingActive)
    {
        UpdateProfilingData(DeltaTime);
    }
}

void APerf_PerformanceProfiler::StartProfiling()
{
    if (bIsProfilingActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Profiling already active"));
        return;
    }
    
    bIsProfilingActive = true;
    LastProfilingTime = GetWorld()->GetTimeSeconds();
    
    // Clear previous data
    ProfileHistory.Empty();
    CurrentProfileData = FPerf_ProfileData();
    
    // Enable engine stats
    if (ProfilingSettings.bEnableCPUProfiling)
    {
        GEngine->Exec(GetWorld(), TEXT("stat fps"));
        GEngine->Exec(GetWorld(), TEXT("stat unit"));
    }
    
    if (ProfilingSettings.bEnableGPUProfiling)
    {
        GEngine->Exec(GetWorld(), TEXT("stat gpu"));
        GEngine->Exec(GetWorld(), TEXT("stat rhi"));
    }
    
    if (ProfilingSettings.bEnableMemoryProfiling)
    {
        GEngine->Exec(GetWorld(), TEXT("stat memory"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
}

void APerf_PerformanceProfiler::StopProfiling()
{
    if (!bIsProfilingActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Profiling not active"));
        return;
    }
    
    bIsProfilingActive = false;
    
    // Disable engine stats
    GEngine->Exec(GetWorld(), TEXT("stat none"));
    
    // Generate final report
    GeneratePerformanceReport();
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
}

void APerf_PerformanceProfiler::UpdateProfilingData(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastProfilingTime < ProfilingSettings.ProfilingInterval)
    {
        return;
    }
    
    LastProfilingTime = CurrentTime;
    
    // Update current profile data
    UpdateCPUMetrics(DeltaTime);
    UpdateGPUMetrics();
    UpdateMemoryMetrics();
    UpdateRenderingMetrics();
    
    // Add to history
    ProfileHistory.Add(CurrentProfileData);
    
    // Limit history size
    if (ProfileHistory.Num() > ProfilingSettings.MaxSampleHistory)
    {
        ProfileHistory.RemoveAt(0);
    }
    
    // Check performance thresholds
    CheckPerformanceThresholds();
}

void APerf_PerformanceProfiler::UpdateCPUMetrics(float DeltaTime)
{
    if (!ProfilingSettings.bEnableCPUProfiling)
    {
        return;
    }
    
    // Calculate FPS
    CurrentProfileData.CurrentFPS = 1.0f / DeltaTime;
    CurrentProfileData.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Update frame time history for smoothing
    FrameTimeHistory.Add(DeltaTime);
    if (FrameTimeHistory.Num() > 60) // Keep 1 second of history at 60fps
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    CurrentProfileData.AverageFrameTime = (TotalFrameTime / FrameTimeHistory.Num()) * 1000.0f;
    
    // Game thread time (approximation)
    CurrentProfileData.GameThreadTime = DeltaTime * 1000.0f * 0.7f; // Rough estimate
    
    // Update timestamp
    CurrentProfileData.Timestamp = CurrentTime;
}

void APerf_PerformanceProfiler::UpdateGPUMetrics()
{
    if (!ProfilingSettings.bEnableGPUProfiling)
    {
        return;
    }
    
    // GPU timing (these are approximations - real GPU timing requires more complex setup)
    CurrentProfileData.GPUFrameTime = CurrentProfileData.FrameTime * 0.6f; // Rough estimate
    CurrentProfileData.RenderThreadTime = CurrentProfileData.FrameTime * 0.5f;
}

void APerf_PerformanceProfiler::UpdateMemoryMetrics()
{
    if (!ProfilingSettings.bEnableMemoryProfiling)
    {
        return;
    }
    
    // Get memory stats from engine
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    CurrentProfileData.UsedPhysicalMemoryMB = MemStats.UsedPhysical / (1024 * 1024);
    CurrentProfileData.UsedVirtualMemoryMB = MemStats.UsedVirtual / (1024 * 1024);
    CurrentProfileData.AvailablePhysicalMemoryMB = MemStats.AvailablePhysical / (1024 * 1024);
}

void APerf_PerformanceProfiler::UpdateRenderingMetrics()
{
    if (!ProfilingSettings.bEnableDrawCallProfiling)
    {
        return;
    }
    
    // These would require access to rendering stats - using estimates for now
    CurrentProfileData.DrawCalls = 1500; // Placeholder
    CurrentProfileData.Triangles = 500000; // Placeholder
    CurrentProfileData.TextureMemoryMB = 512.0f; // Placeholder
}

void APerf_PerformanceProfiler::CheckPerformanceThresholds()
{
    bool bPerformanceIssue = false;
    
    // Check FPS threshold
    if (CurrentProfileData.CurrentFPS < PerformanceTargets.MinimumFPS)
    {
        bPerformanceIssue = true;
        UE_LOG(LogTemp, Warning, TEXT("FPS below threshold: %.1f < %.1f"), 
               CurrentProfileData.CurrentFPS, PerformanceTargets.MinimumFPS);
    }
    
    // Check frame time threshold
    if (CurrentProfileData.FrameTime > PerformanceTargets.MaxFrameTime)
    {
        bPerformanceIssue = true;
        UE_LOG(LogTemp, Warning, TEXT("Frame time above threshold: %.1f > %.1f ms"), 
               CurrentProfileData.FrameTime, PerformanceTargets.MaxFrameTime);
    }
    
    // Check memory threshold
    if (CurrentProfileData.UsedPhysicalMemoryMB > PerformanceTargets.MaxMemoryUsageMB)
    {
        bPerformanceIssue = true;
        UE_LOG(LogTemp, Warning, TEXT("Memory usage above threshold: %.1f > %.1f MB"), 
               CurrentProfileData.UsedPhysicalMemoryMB, PerformanceTargets.MaxMemoryUsageMB);
    }
    
    // Trigger optimization if needed
    if (bPerformanceIssue && bAutoOptimize)
    {
        TriggerPerformanceOptimization();
    }
}

void APerf_PerformanceProfiler::TriggerPerformanceOptimization()
{
    UE_LOG(LogTemp, Log, TEXT("Triggering performance optimization"));
    
    // Find performance manager in the world
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (TActorIterator<APerf_DynamicPerformanceManager> ActorItr(World); ActorItr; ++ActorItr)
    {
        APerf_DynamicPerformanceManager* PerfManager = *ActorItr;
        if (PerfManager)
        {
            PerfManager->OptimizeScene();
            break;
        }
    }
}

FPerf_ProfileData APerf_PerformanceProfiler::GetCurrentProfileData() const
{
    return CurrentProfileData;
}

TArray<FPerf_ProfileData> APerf_PerformanceProfiler::GetProfileHistory() const
{
    return ProfileHistory;
}

void APerf_PerformanceProfiler::GeneratePerformanceReport()
{
    if (ProfileHistory.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No profile data available for report"));
        return;
    }
    
    // Calculate statistics
    float MinFPS = 999.0f;
    float MaxFPS = 0.0f;
    float AvgFPS = 0.0f;
    float MinFrameTime = 0.0f;
    float MaxFrameTime = 0.0f;
    float AvgFrameTime = 0.0f;
    
    for (const FPerf_ProfileData& Data : ProfileHistory)
    {
        MinFPS = FMath::Min(MinFPS, Data.CurrentFPS);
        MaxFPS = FMath::Max(MaxFPS, Data.CurrentFPS);
        AvgFPS += Data.CurrentFPS;
        
        MinFrameTime = FMath::Min(MinFrameTime, Data.FrameTime);
        MaxFrameTime = FMath::Max(MaxFrameTime, Data.FrameTime);
        AvgFrameTime += Data.FrameTime;
    }
    
    AvgFPS /= ProfileHistory.Num();
    AvgFrameTime /= ProfileHistory.Num();
    
    // Log performance report
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Sample Count: %d"), ProfileHistory.Num());
    UE_LOG(LogTemp, Log, TEXT("FPS - Min: %.1f, Max: %.1f, Avg: %.1f"), MinFPS, MaxFPS, AvgFPS);
    UE_LOG(LogTemp, Log, TEXT("Frame Time - Min: %.1f ms, Max: %.1f ms, Avg: %.1f ms"), MinFrameTime, MaxFrameTime, AvgFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Current Memory: %.1f MB"), CurrentProfileData.UsedPhysicalMemoryMB);
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
    
    // Save to file if requested
    if (bSaveReportToFile)
    {
        SaveReportToFile();
    }
}

void APerf_PerformanceProfiler::SaveReportToFile()
{
    FString ReportContent = TEXT("Performance Profiling Report\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    ReportContent += FString::Printf(TEXT("Profiling Settings:\n"));
    ReportContent += FString::Printf(TEXT("- CPU Profiling: %s\n"), ProfilingSettings.bEnableCPUProfiling ? TEXT("Enabled") : TEXT("Disabled"));
    ReportContent += FString::Printf(TEXT("- GPU Profiling: %s\n"), ProfilingSettings.bEnableGPUProfiling ? TEXT("Enabled") : TEXT("Disabled"));
    ReportContent += FString::Printf(TEXT("- Memory Profiling: %s\n"), ProfilingSettings.bEnableMemoryProfiling ? TEXT("Enabled") : TEXT("Disabled"));
    ReportContent += FString::Printf(TEXT("- Sample Interval: %.2f seconds\n\n"), ProfilingSettings.ProfilingInterval);
    
    if (ProfileHistory.Num() > 0)
    {
        ReportContent += TEXT("Performance Data:\n");
        ReportContent += TEXT("Timestamp,FPS,FrameTime(ms),GameThread(ms),GPU(ms),Memory(MB)\n");
        
        for (const FPerf_ProfileData& Data : ProfileHistory)
        {
            ReportContent += FString::Printf(TEXT("%.2f,%.1f,%.2f,%.2f,%.2f,%.1f\n"),
                Data.Timestamp, Data.CurrentFPS, Data.FrameTime, Data.GameThreadTime,
                Data.GPUFrameTime, Data.UsedPhysicalMemoryMB);
        }
    }
    
    // Save to project saved directory
    FString FilePath = FPaths::ProjectSavedDir() / TEXT("PerformanceReports") / 
                      FString::Printf(TEXT("PerfReport_%s.csv"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    UE_LOG(LogTemp, Log, TEXT("Performance report saved to: %s"), *FilePath);
}

void APerf_PerformanceProfiler::SetProfilingSettings(const FPerf_ProfilingSettings& NewSettings)
{
    ProfilingSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Profiling settings updated"));
}

void APerf_PerformanceProfiler::SetPerformanceTargets(const FPerf_PerformanceTargets& NewTargets)
{
    PerformanceTargets = NewTargets;
    UE_LOG(LogTemp, Log, TEXT("Performance targets updated"));
}

void APerf_PerformanceProfiler::ClearProfileHistory()
{
    ProfileHistory.Empty();
    UE_LOG(LogTemp, Log, TEXT("Profile history cleared"));
}