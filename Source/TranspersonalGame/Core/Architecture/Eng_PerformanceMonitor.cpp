#include "Eng_PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"

void UEng_PerformanceMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsProfilingActive = true;
    ProfilingInterval = 1.0f; // Update every second
    TimeSinceLastUpdate = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor initialized - Profiling: %s"), 
        bIsProfilingActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
}

void UEng_PerformanceMonitor::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor deinitialized"));
    Super::Deinitialize();
}

void UEng_PerformanceMonitor::Tick(float DeltaTime)
{
    if (!bIsProfilingActive)
        return;

    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= ProfilingInterval)
    {
        UpdateMetrics();
        CheckPerformanceThresholds();
        TimeSinceLastUpdate = 0.0f;
    }
}

FEng_PerformanceMetrics UEng_PerformanceMonitor::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UEng_PerformanceMonitor::StartProfiling()
{
    bIsProfilingActive = true;
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
}

void UEng_PerformanceMonitor::StopProfiling()
{
    bIsProfilingActive = false;
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
}

void UEng_PerformanceMonitor::UpdateMetrics()
{
    // Get frame rate
    CurrentMetrics.FrameRate = 1.0f / FApp::GetDeltaTime();
    CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Get actor count
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActorCount = World->GetActorCount();
    }
    
    // Memory usage (rough estimate)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // CPU and GPU times (simplified)
    CurrentMetrics.CPUTime = FApp::GetDeltaTime() * 1000.0f;
    CurrentMetrics.GPUTime = FApp::GetDeltaTime() * 1000.0f * 0.8f; // Estimate
}

void UEng_PerformanceMonitor::CheckPerformanceThresholds()
{
    // Check for performance issues
    if (CurrentMetrics.FrameRate < 30.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("LOW FRAMERATE: %.1f FPS"), CurrentMetrics.FrameRate);
    }
    
    if (CurrentMetrics.ActorCount > 8000)
    {
        UE_LOG(LogTemp, Warning, TEXT("HIGH ACTOR COUNT: %d actors"), CurrentMetrics.ActorCount);
    }
    
    if (CurrentMetrics.MemoryUsageMB > 2048.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("HIGH MEMORY USAGE: %.1f MB"), CurrentMetrics.MemoryUsageMB);
    }
}

void UEng_PerformanceMonitor::LogPerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Frame Rate: %.1f FPS"), CurrentMetrics.FrameRate);
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Log, TEXT("Actor Count: %d"), CurrentMetrics.ActorCount);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("CPU Time: %.2f ms"), CurrentMetrics.CPUTime);
    UE_LOG(LogTemp, Log, TEXT("GPU Time: %.2f ms"), CurrentMetrics.GPUTime);
}