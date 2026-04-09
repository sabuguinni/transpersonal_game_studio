#include "QAPerformanceValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Stats/StatsData.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"
#include "TimerManager.h"
#include "GenericPlatform/GenericPlatformMisc.h"

DEFINE_LOG_CATEGORY(LogQAPerformance);

UQAPerformanceValidator::UQAPerformanceValidator()
{
    // Set default performance thresholds
    DefaultThresholds.MinFPS_PC = 60.0f;
    DefaultThresholds.MinFPS_Console = 30.0f;
    DefaultThresholds.MaxMemoryMB_PC = 8192;
    DefaultThresholds.MaxMemoryMB_Console = 4096;
    DefaultThresholds.MaxFrameTimeMS = 16.67f;
    DefaultThresholds.MaxDrawCalls = 2000;
    DefaultThresholds.MaxTriangles = 2000000;
    DefaultThresholds.MaxGPUTimeMS = 16.0f;
    DefaultThresholds.MaxVisibleStaticMeshElements = 5000;
}

void UQAPerformanceValidator::Initialize()
{
    UE_LOG(LogQAPerformance, Log, TEXT("Initializing QA Performance Validator"));
    
    // Clear any existing data
    PerformanceHistory.Empty();
    bIsMonitoring = false;
    ElapsedMonitoringTime = 0.0f;
    
    UE_LOG(LogQAPerformance, Log, TEXT("QA Performance Validator initialized successfully"));
}

void UQAPerformanceValidator::StartMonitoring(float Duration)
{
    if (bIsMonitoring)
    {
        UE_LOG(LogQAPerformance, Warning, TEXT("Performance monitoring already active"));
        return;
    }

    UE_LOG(LogQAPerformance, Log, TEXT("Starting performance monitoring for %.2f seconds"), Duration);
    
    bIsMonitoring = true;
    MonitoringDuration = Duration;
    ElapsedMonitoringTime = 0.0f;
    MonitoringStartTime = FDateTime::Now();
    PerformanceHistory.Empty();

    // Start monitoring timer
    if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UQAPerformanceValidator::UpdateMetrics,
            0.1f, // Update every 100ms
            true
        );
    }
}

void UQAPerformanceValidator::StopMonitoring()
{
    if (!bIsMonitoring)
    {
        UE_LOG(LogQAPerformance, Warning, TEXT("Performance monitoring not active"));
        return;
    }

    UE_LOG(LogQAPerformance, Log, TEXT("Stopping performance monitoring"));
    
    bIsMonitoring = false;
    
    // Clear timer
    if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
    }

    // Log final statistics
    if (PerformanceHistory.Num() > 0)
    {
        FPerformanceMetrics AverageMetrics = CalculateAverageMetrics(MonitoringDuration);
        UE_LOG(LogQAPerformance, Log, TEXT("Monitoring completed. Average FPS: %.2f, Frame Time: %.2fms"), 
               AverageMetrics.CurrentFPS, AverageMetrics.FrameTimeMS);
    }
}

FPerformanceMetrics UQAPerformanceValidator::GetCurrentMetrics()
{
    return CollectMetrics();
}

bool UQAPerformanceValidator::ValidatePerformance(const FPerformanceThresholds& Thresholds, FString& OutFailureReason)
{
    FPerformanceMetrics CurrentMetrics = GetCurrentMetrics();
    
    // Determine platform-specific thresholds
    float MinFPS = GetCurrentPlatform().Contains(TEXT("Console")) ? Thresholds.MinFPS_Console : Thresholds.MinFPS_PC;
    int32 MaxMemory = GetCurrentPlatform().Contains(TEXT("Console")) ? Thresholds.MaxMemoryMB_Console : Thresholds.MaxMemoryMB_PC;
    
    // Validate FPS
    if (CurrentMetrics.CurrentFPS < MinFPS)
    {
        OutFailureReason = FString::Printf(TEXT("FPS below threshold: %.2f < %.2f"), CurrentMetrics.CurrentFPS, MinFPS);
        return false;
    }
    
    // Validate frame time
    if (CurrentMetrics.FrameTimeMS > Thresholds.MaxFrameTimeMS)
    {
        OutFailureReason = FString::Printf(TEXT("Frame time above threshold: %.2fms > %.2fms"), 
                                         CurrentMetrics.FrameTimeMS, Thresholds.MaxFrameTimeMS);
        return false;
    }
    
    // Validate memory usage
    if (CurrentMetrics.MemoryUsageMB > MaxMemory)
    {
        OutFailureReason = FString::Printf(TEXT("Memory usage above threshold: %dMB > %dMB"), 
                                         CurrentMetrics.MemoryUsageMB, MaxMemory);
        return false;
    }
    
    // Validate GPU time
    if (CurrentMetrics.GPUTimeMS > Thresholds.MaxGPUTimeMS)
    {
        OutFailureReason = FString::Printf(TEXT("GPU time above threshold: %.2fms > %.2fms"), 
                                         CurrentMetrics.GPUTimeMS, Thresholds.MaxGPUTimeMS);
        return false;
    }
    
    // Validate draw calls
    if (CurrentMetrics.DrawCalls > Thresholds.MaxDrawCalls)
    {
        OutFailureReason = FString::Printf(TEXT("Draw calls above threshold: %d > %d"), 
                                         CurrentMetrics.DrawCalls, Thresholds.MaxDrawCalls);
        return false;
    }
    
    OutFailureReason = TEXT("Performance validation passed");
    return true;
}

FString UQAPerformanceValidator::GeneratePerformanceReport()
{
    FString Report;
    Report += TEXT("=== QA Performance Report ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Platform: %s\n"), *GetCurrentPlatform());
    Report += TEXT("\n");
    
    if (PerformanceHistory.Num() > 0)
    {
        FPerformanceMetrics AverageMetrics = CalculateAverageMetrics(MonitoringDuration);
        
        Report += TEXT("=== Average Performance Metrics ===\n");
        Report += FString::Printf(TEXT("FPS: %.2f\n"), AverageMetrics.CurrentFPS);
        Report += FString::Printf(TEXT("Frame Time: %.2fms\n"), AverageMetrics.FrameTimeMS);
        Report += FString::Printf(TEXT("Game Thread: %.2fms\n"), AverageMetrics.GameThreadTimeMS);
        Report += FString::Printf(TEXT("Render Thread: %.2fms\n"), AverageMetrics.RenderThreadTimeMS);
        Report += FString::Printf(TEXT("GPU Time: %.2fms\n"), AverageMetrics.GPUTimeMS);
        Report += FString::Printf(TEXT("Memory Usage: %dMB\n"), AverageMetrics.MemoryUsageMB);
        Report += FString::Printf(TEXT("Draw Calls: %d\n"), AverageMetrics.DrawCalls);
        Report += FString::Printf(TEXT("Triangles: %d\n"), AverageMetrics.Triangles);
        Report += TEXT("\n");
        
        // Performance validation
        FPerformanceThresholds Thresholds = GetPlatformThresholds();
        FString FailureReason;
        bool bPassed = ValidatePerformance(Thresholds, FailureReason);
        
        Report += TEXT("=== Performance Validation ===\n");
        Report += FString::Printf(TEXT("Status: %s\n"), bPassed ? TEXT("PASSED") : TEXT("FAILED"));
        Report += FString::Printf(TEXT("Details: %s\n"), *FailureReason);
        Report += TEXT("\n");
        
        // Hitch detection
        bool bHitchesDetected = DetectPerformanceHitches();
        Report += FString::Printf(TEXT("Performance Hitches Detected: %s\n"), bHitchesDetected ? TEXT("YES") : TEXT("NO"));
    }
    else
    {
        FPerformanceMetrics CurrentMetrics = GetCurrentMetrics();
        Report += TEXT("=== Current Performance Metrics ===\n");
        Report += FString::Printf(TEXT("FPS: %.2f\n"), CurrentMetrics.CurrentFPS);
        Report += FString::Printf(TEXT("Frame Time: %.2fms\n"), CurrentMetrics.FrameTimeMS);
        Report += FString::Printf(TEXT("Memory Usage: %dMB\n"), CurrentMetrics.MemoryUsageMB);
    }
    
    return Report;
}

FPerformanceThresholds UQAPerformanceValidator::GetPlatformThresholds() const
{
    FPerformanceThresholds PlatformThresholds = DefaultThresholds;
    
    // Adjust thresholds based on current platform
    FString Platform = GetCurrentPlatform();
    if (Platform.Contains(TEXT("Console")))
    {
        // Console-specific adjustments
        PlatformThresholds.MinFPS_PC = PlatformThresholds.MinFPS_Console;
        PlatformThresholds.MaxMemoryMB_PC = PlatformThresholds.MaxMemoryMB_Console;
        PlatformThresholds.MaxFrameTimeMS = 33.33f; // 30fps
    }
    
    return PlatformThresholds;
}

bool UQAPerformanceValidator::ValidateFrameRateStability(float MinFPS, float TestDuration, float AllowedVariance)
{
    if (!bIsMonitoring || PerformanceHistory.Num() < 10)
    {
        UE_LOG(LogQAPerformance, Warning, TEXT("Insufficient data for frame rate stability test"));
        return false;
    }
    
    // Calculate FPS variance over the test period
    TArray<float> FPSValues;
    for (const FPerformanceMetrics& Metrics : PerformanceHistory)
    {
        FPSValues.Add(Metrics.CurrentFPS);
    }
    
    // Calculate average and variance
    float Sum = 0.0f;
    for (float FPS : FPSValues)
    {
        Sum += FPS;
    }
    float Average = Sum / FPSValues.Num();
    
    float VarianceSum = 0.0f;
    for (float FPS : FPSValues)
    {
        VarianceSum += FMath::Pow(FPS - Average, 2);
    }
    float Variance = FMath::Sqrt(VarianceSum / FPSValues.Num());
    
    bool bStable = (Average >= MinFPS) && (Variance <= AllowedVariance);
    
    UE_LOG(LogQAPerformance, Log, TEXT("Frame rate stability test: Average=%.2f, Variance=%.2f, Stable=%s"), 
           Average, Variance, bStable ? TEXT("YES") : TEXT("NO"));
    
    return bStable;
}

bool UQAPerformanceValidator::ValidateMemoryUsage(int32 MaxMemoryMB)
{
    FPerformanceMetrics CurrentMetrics = GetCurrentMetrics();
    bool bValid = CurrentMetrics.MemoryUsageMB <= MaxMemoryMB;
    
    UE_LOG(LogQAPerformance, Log, TEXT("Memory validation: %dMB <= %dMB = %s"), 
           CurrentMetrics.MemoryUsageMB, MaxMemoryMB, bValid ? TEXT("PASS") : TEXT("FAIL"));
    
    return bValid;
}

bool UQAPerformanceValidator::ValidateRenderingPerformance()
{
    FPerformanceMetrics CurrentMetrics = GetCurrentMetrics();
    FPerformanceThresholds Thresholds = GetPlatformThresholds();
    
    bool bValid = (CurrentMetrics.DrawCalls <= Thresholds.MaxDrawCalls) &&
                  (CurrentMetrics.Triangles <= Thresholds.MaxTriangles) &&
                  (CurrentMetrics.GPUTimeMS <= Thresholds.MaxGPUTimeMS);
    
    UE_LOG(LogQAPerformance, Log, TEXT("Rendering validation: DrawCalls=%d, Triangles=%d, GPUTime=%.2fms, Valid=%s"), 
           CurrentMetrics.DrawCalls, CurrentMetrics.Triangles, CurrentMetrics.GPUTimeMS, 
           bValid ? TEXT("PASS") : TEXT("FAIL"));
    
    return bValid;
}

bool UQAPerformanceValidator::ExportPerformanceData(const FString& FilePath)
{
    if (PerformanceHistory.Num() == 0)
    {
        UE_LOG(LogQAPerformance, Warning, TEXT("No performance data to export"));
        return false;
    }
    
    FString CSVContent;
    CSVContent += TEXT("Timestamp,FPS,FrameTimeMS,GameThreadMS,RenderThreadMS,GPUTimeMS,MemoryMB,DrawCalls,Triangles\n");
    
    for (const FPerformanceMetrics& Metrics : PerformanceHistory)
    {
        CSVContent += FString::Printf(TEXT("%s,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d\n"),
                                    *Metrics.Timestamp.ToString(),
                                    Metrics.CurrentFPS,
                                    Metrics.FrameTimeMS,
                                    Metrics.GameThreadTimeMS,
                                    Metrics.RenderThreadTimeMS,
                                    Metrics.GPUTimeMS,
                                    Metrics.MemoryUsageMB,
                                    Metrics.DrawCalls,
                                    Metrics.Triangles);
    }
    
    bool bSuccess = FFileHelper::SaveStringToFile(CSVContent, *FilePath);
    
    UE_LOG(LogQAPerformance, Log, TEXT("Performance data export %s: %s"), 
           bSuccess ? TEXT("successful") : TEXT("failed"), *FilePath);
    
    return bSuccess;
}

void UQAPerformanceValidator::UpdateMetrics()
{
    if (!bIsMonitoring)
        return;
    
    ElapsedMonitoringTime += 0.1f;
    
    // Collect current metrics
    FPerformanceMetrics CurrentMetrics = CollectMetrics();
    PerformanceHistory.Add(CurrentMetrics);
    
    // Check if monitoring duration exceeded
    if (ElapsedMonitoringTime >= MonitoringDuration)
    {
        StopMonitoring();
    }
}

FPerformanceMetrics UQAPerformanceValidator::CollectMetrics()
{
    FPerformanceMetrics Metrics;
    Metrics.Timestamp = FDateTime::Now();
    
    // Get FPS and frame time
    if (GEngine && GEngine->GetGameViewport())
    {
        Metrics.CurrentFPS = 1.0f / FApp::GetDeltaTime();
        Metrics.FrameTimeMS = FApp::GetDeltaTime() * 1000.0f;
    }
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Metrics.MemoryUsageMB = MemStats.UsedPhysical / (1024 * 1024);
    
    // Get rendering stats (simplified - would need proper stat system integration)
    Metrics.DrawCalls = 1000; // Placeholder - would get from RHI stats
    Metrics.Triangles = 500000; // Placeholder - would get from rendering stats
    Metrics.GPUTimeMS = 12.0f; // Placeholder - would get from GPU profiler
    Metrics.GameThreadTimeMS = 8.0f; // Placeholder - would get from thread profiler
    Metrics.RenderThreadTimeMS = 10.0f; // Placeholder - would get from thread profiler
    
    return Metrics;
}

FString UQAPerformanceValidator::GetCurrentPlatform() const
{
    return FPlatformMisc::GetPlatformName();
}

FPerformanceMetrics UQAPerformanceValidator::CalculateAverageMetrics(float TimePeriod) const
{
    if (PerformanceHistory.Num() == 0)
    {
        return FPerformanceMetrics();
    }
    
    FPerformanceMetrics AverageMetrics;
    int32 SampleCount = PerformanceHistory.Num();
    
    for (const FPerformanceMetrics& Metrics : PerformanceHistory)
    {
        AverageMetrics.CurrentFPS += Metrics.CurrentFPS;
        AverageMetrics.FrameTimeMS += Metrics.FrameTimeMS;
        AverageMetrics.GameThreadTimeMS += Metrics.GameThreadTimeMS;
        AverageMetrics.RenderThreadTimeMS += Metrics.RenderThreadTimeMS;
        AverageMetrics.GPUTimeMS += Metrics.GPUTimeMS;
        AverageMetrics.MemoryUsageMB += Metrics.MemoryUsageMB;
        AverageMetrics.DrawCalls += Metrics.DrawCalls;
        AverageMetrics.Triangles += Metrics.Triangles;
    }
    
    AverageMetrics.CurrentFPS /= SampleCount;
    AverageMetrics.FrameTimeMS /= SampleCount;
    AverageMetrics.GameThreadTimeMS /= SampleCount;
    AverageMetrics.RenderThreadTimeMS /= SampleCount;
    AverageMetrics.GPUTimeMS /= SampleCount;
    AverageMetrics.MemoryUsageMB /= SampleCount;
    AverageMetrics.DrawCalls /= SampleCount;
    AverageMetrics.Triangles /= SampleCount;
    
    return AverageMetrics;
}

bool UQAPerformanceValidator::DetectPerformanceHitches(float HitchThresholdMS) const
{
    if (PerformanceHistory.Num() < 2)
    {
        return false;
    }
    
    for (const FPerformanceMetrics& Metrics : PerformanceHistory)
    {
        if (Metrics.FrameTimeMS > HitchThresholdMS)
        {
            return true;
        }
    }
    
    return false;
}