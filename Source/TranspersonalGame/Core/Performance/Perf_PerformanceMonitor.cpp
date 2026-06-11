#include "Perf_PerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "GameFramework/GameUserSettings.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"

APerf_PerformanceMonitor::APerf_PerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize settings
    MonitoringInterval = 1.0f;
    bLogToFile = true;
    bDisplayOnScreen = true;
    bAutoOptimize = true;
    
    // Initialize tracking variables
    AccumulatedTime = 0.0f;
    FrameCount = 0;
    LastLogTime = 0.0f;
    LastOptimizationTime = 0.0f;
    ConsecutiveLowFPSFrames = 0;
    
    // Reserve space for FPS history
    FPSHistory.Reserve(300); // 30 seconds at 10Hz
}

void APerf_PerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize log file path
    LogFilePath = GetLogFileName();
    
    // Start monitoring
    StartMonitoring();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor started - Target FPS: %.1f"), OptimizationSettings.TargetFPS);
}

void APerf_PerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateMetrics(DeltaTime);
    
    AccumulatedTime += DeltaTime;
    
    // Log metrics at specified interval
    if (AccumulatedTime >= MonitoringInterval)
    {
        if (bLogToFile)
        {
            LogMetricsToFile();
        }
        
        if (bDisplayOnScreen)
        {
            DisplayMetricsOnScreen();
        }
        
        if (bAutoOptimize)
        {
            CheckAutoOptimization();
        }
        
        AccumulatedTime = 0.0f;
        LastLogTime = GetWorld()->GetTimeSeconds();
    }
}

void APerf_PerformanceMonitor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopMonitoring();
    Super::EndPlay(EndPlayReason);
}

void APerf_PerformanceMonitor::UpdateMetrics(float DeltaTime)
{
    if (DeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
        
        // Update FPS history
        FPSHistory.Add(CurrentMetrics.CurrentFPS);
        if (FPSHistory.Num() > 300) // Keep last 30 seconds
        {
            FPSHistory.RemoveAt(0);
        }
        
        // Calculate average FPS
        if (FPSHistory.Num() > 0)
        {
            float Sum = 0.0f;
            for (float FPS : FPSHistory)
            {
                Sum += FPS;
            }
            CurrentMetrics.AverageFPS = Sum / FPSHistory.Num();
        }
        
        // Update min/max FPS
        CurrentMetrics.MinFPS = FMath::Min(CurrentMetrics.MinFPS, CurrentMetrics.CurrentFPS);
        CurrentMetrics.MaxFPS = FMath::Max(CurrentMetrics.MaxFPS, CurrentMetrics.CurrentFPS);
        
        // Get rendering stats (simplified - actual implementation would use proper stat system)
        CurrentMetrics.GameThreadTime = DeltaTime * 1000.0f;
        CurrentMetrics.RenderThreadTime = DeltaTime * 0.8f * 1000.0f; // Estimate
        CurrentMetrics.GPUTime = DeltaTime * 0.6f * 1000.0f; // Estimate
        
        // Estimate draw calls and triangles (would need proper stat integration)
        CurrentMetrics.DrawCalls = 1000; // Placeholder
        CurrentMetrics.Triangles = 50000; // Placeholder
        
        // Memory usage (simplified)
        CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024 * 1024);
        
        FrameCount++;
    }
}

FPerf_PerformanceMetrics APerf_PerformanceMonitor::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerf_PerformanceMonitor::StartMonitoring()
{
    ResetMetrics();
    SetActorTickEnabled(true);
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
}

void APerf_PerformanceMonitor::StopMonitoring()
{
    SetActorTickEnabled(false);
    LogPerformanceReport();
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
}

void APerf_PerformanceMonitor::ResetMetrics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    FPSHistory.Empty();
    AccumulatedTime = 0.0f;
    FrameCount = 0;
    ConsecutiveLowFPSFrames = 0;
}

void APerf_PerformanceMonitor::ApplyOptimizationSettings()
{
    if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
    {
        // Apply LOD settings
        Settings->SetViewDistanceQuality(FMath::RoundToInt(OptimizationSettings.ViewDistanceScale * 3));
        
        // Apply shadow settings
        if (OptimizationSettings.MaxShadowResolution <= 1024)
        {
            Settings->SetShadowQuality(1);
        }
        else if (OptimizationSettings.MaxShadowResolution <= 2048)
        {
            Settings->SetShadowQuality(2);
        }
        else
        {
            Settings->SetShadowQuality(3);
        }
        
        Settings->ApplySettings(false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Optimization settings applied"));
}

void APerf_PerformanceMonitor::LogPerformanceReport()
{
    FString Report = FString::Printf(TEXT("\n=== PERFORMANCE REPORT ===\n"));
    Report += FString::Printf(TEXT("Current FPS: %.1f\n"), CurrentMetrics.CurrentFPS);
    Report += FString::Printf(TEXT("Average FPS: %.1f\n"), CurrentMetrics.AverageFPS);
    Report += FString::Printf(TEXT("Min FPS: %.1f\n"), CurrentMetrics.MinFPS);
    Report += FString::Printf(TEXT("Max FPS: %.1f\n"), CurrentMetrics.MaxFPS);
    Report += FString::Printf(TEXT("Frame Time: %.2f ms\n"), CurrentMetrics.FrameTime);
    Report += FString::Printf(TEXT("Memory Usage: %.1f MB\n"), CurrentMetrics.MemoryUsageMB);
    Report += FString::Printf(TEXT("Total Frames: %d\n"), FrameCount);
    Report += FString::Printf(TEXT("Performance Acceptable: %s\n"), IsPerformanceAcceptable() ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *Report);
    
    if (bLogToFile)
    {
        FFileHelper::SaveStringToFile(Report, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
    }
}

bool APerf_PerformanceMonitor::IsPerformanceAcceptable() const
{
    return CurrentMetrics.AverageFPS >= (OptimizationSettings.TargetFPS * 0.8f);
}

void APerf_PerformanceMonitor::TriggerAutoOptimization()
{
    if (!IsPerformanceAcceptable())
    {
        ApplyLODOptimization();
        ApplyLightingOptimization();
        ApplyRenderingOptimization();
        
        UE_LOG(LogTemp, Warning, TEXT("Auto-optimization triggered due to low performance"));
    }
}

void APerf_PerformanceMonitor::LogMetricsToFile()
{
    if (!LogFilePath.IsEmpty())
    {
        FString LogEntry = FString::Printf(TEXT("%.2f,%.1f,%.1f,%.2f,%.1f\n"), 
            GetWorld()->GetTimeSeconds(),
            CurrentMetrics.CurrentFPS,
            CurrentMetrics.FrameTime,
            CurrentMetrics.MemoryUsageMB,
            CurrentMetrics.AverageFPS
        );
        
        FFileHelper::SaveStringToFile(LogEntry, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
    }
}

void APerf_PerformanceMonitor::DisplayMetricsOnScreen()
{
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("FPS: %.1f | Frame: %.1fms | Mem: %.0fMB | Avg: %.1f"), 
            CurrentMetrics.CurrentFPS,
            CurrentMetrics.FrameTime,
            CurrentMetrics.MemoryUsageMB,
            CurrentMetrics.AverageFPS
        );
        
        FColor DisplayColor = IsPerformanceAcceptable() ? FColor::Green : FColor::Red;
        GEngine->AddOnScreenDebugMessage(-1, MonitoringInterval + 0.1f, DisplayColor, DisplayText);
    }
}

void APerf_PerformanceMonitor::CheckAutoOptimization()
{
    if (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS * 0.7f)
    {
        ConsecutiveLowFPSFrames++;
        
        if (ConsecutiveLowFPSFrames >= 5) // 5 consecutive low FPS readings
        {
            float CurrentTime = GetWorld()->GetTimeSeconds();
            if (CurrentTime - LastOptimizationTime > 10.0f) // Don't optimize too frequently
            {
                TriggerAutoOptimization();
                LastOptimizationTime = CurrentTime;
                ConsecutiveLowFPSFrames = 0;
            }
        }
    }
    else
    {
        ConsecutiveLowFPSFrames = 0;
    }
}

void APerf_PerformanceMonitor::ApplyLODOptimization()
{
    // Increase LOD bias to reduce polygon count
    if (GEngine && GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
    {
        GEngine->Exec(GetWorld(), TEXT("r.StaticMeshLODBias 2"));
        GEngine->Exec(GetWorld(), TEXT("r.SkeletalMeshLODBias 1"));
        UE_LOG(LogTemp, Log, TEXT("LOD optimization applied"));
    }
}

void APerf_PerformanceMonitor::ApplyLightingOptimization()
{
    // Reduce shadow quality and distance
    if (GEngine && GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
    {
        GEngine->Exec(GetWorld(), TEXT("r.Shadow.MaxResolution 1024"));
        GEngine->Exec(GetWorld(), TEXT("r.Shadow.DistanceScale 0.5"));
        UE_LOG(LogTemp, Log, TEXT("Lighting optimization applied"));
    }
}

void APerf_PerformanceMonitor::ApplyRenderingOptimization()
{
    // Reduce view distance and effects quality
    if (GEngine && GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
    {
        GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.7"));
        GEngine->Exec(GetWorld(), TEXT("r.DefaultFeature.AntiAliasing 1"));
        UE_LOG(LogTemp, Log, TEXT("Rendering optimization applied"));
    }
}

FString APerf_PerformanceMonitor::GetLogFileName() const
{
    FString ProjectDir = FPaths::ProjectDir();
    FString LogDir = FPaths::Combine(ProjectDir, TEXT("Logs"));
    FString FileName = FString::Printf(TEXT("PerformanceLog_%s.csv"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    return FPaths::Combine(LogDir, FileName);
}