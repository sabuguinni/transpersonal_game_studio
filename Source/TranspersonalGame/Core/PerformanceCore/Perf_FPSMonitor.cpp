#include "Perf_FPSMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

APerf_FPSMonitor::APerf_FPSMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.TickInterval = 0.0f; // Tick every frame for accurate FPS measurement

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Initialize settings
    bEnableMonitoring = true;
    UpdateFrequency = 0.5f; // Update twice per second
    SampleCount = 60; // Average over 60 samples
    bLogPerformanceWarnings = true;
    WarningFPSThreshold = 30.0f;

    // Initialize performance tracking
    CurrentSampleIndex = 0;
    TimeSinceLastUpdate = 0.0f;
    TotalSamples = 0;
    PerformanceLevel = EPerf_PerformanceLevel::Good;

    // Reserve space for FPS samples
    FPSSamples.Reserve(SampleCount);
    for (int32 i = 0; i < SampleCount; i++)
    {
        FPSSamples.Add(60.0f); // Initialize with 60 FPS
    }
}

void APerf_FPSMonitor::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("Performance Monitor started - Target: 60 FPS PC / 30 FPS Console"));
    
    // Reset statistics on begin play
    ResetStatistics();
}

void APerf_FPSMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bEnableMonitoring)
    {
        return;
    }

    // Update metrics every frame but only process at specified frequency
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= UpdateFrequency)
    {
        UpdateMetrics(DeltaTime);
        TimeSinceLastUpdate = 0.0f;
    }
}

void APerf_FPSMonitor::UpdateMetrics(float DeltaTime)
{
    // Calculate current FPS
    float CurrentFPS = (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 0.0f;
    
    // Store FPS sample
    FPSSamples[CurrentSampleIndex] = CurrentFPS;
    CurrentSampleIndex = (CurrentSampleIndex + 1) % SampleCount;
    TotalSamples = FMath::Min(TotalSamples + 1, SampleCount);

    // Update current metrics
    CurrentMetrics.CurrentFPS = CurrentFPS;

    // Calculate average FPS
    float TotalFPS = 0.0f;
    for (int32 i = 0; i < TotalSamples; i++)
    {
        TotalFPS += FPSSamples[i];
    }
    CurrentMetrics.AverageFPS = (TotalSamples > 0) ? (TotalFPS / TotalSamples) : 0.0f;

    // Update min/max FPS
    CurrentMetrics.MinFPS = FMath::Min(CurrentMetrics.MinFPS, CurrentFPS);
    CurrentMetrics.MaxFPS = FMath::Max(CurrentMetrics.MaxFPS, CurrentFPS);

    // Get engine stats (approximate values since detailed stats require engine access)
    CurrentMetrics.GameThreadTime = DeltaTime * 1000.0f; // Convert to milliseconds
    CurrentMetrics.RenderThreadTime = DeltaTime * 1000.0f * 0.8f; // Estimate
    CurrentMetrics.GPUTime = DeltaTime * 1000.0f * 0.7f; // Estimate

    // Estimate draw calls and triangles based on world complexity
    if (UWorld* World = GetWorld())
    {
        // Simple estimation based on actor count
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            ActorCount++;
        }
        CurrentMetrics.DrawCalls = ActorCount * 2; // Rough estimate
        CurrentMetrics.Triangles = ActorCount * 1000; // Rough estimate
    }

    // Update performance level
    PerformanceLevel = CalculatePerformanceLevel(CurrentMetrics.CurrentFPS);

    // Check for performance warnings
    CheckPerformanceWarnings();
}

EPerf_PerformanceLevel APerf_FPSMonitor::CalculatePerformanceLevel(float FPS) const
{
    if (FPS >= 60.0f)
    {
        return EPerf_PerformanceLevel::Excellent;
    }
    else if (FPS >= 45.0f)
    {
        return EPerf_PerformanceLevel::Good;
    }
    else if (FPS >= 30.0f)
    {
        return EPerf_PerformanceLevel::Acceptable;
    }
    else if (FPS >= 15.0f)
    {
        return EPerf_PerformanceLevel::Poor;
    }
    else
    {
        return EPerf_PerformanceLevel::Critical;
    }
}

void APerf_FPSMonitor::CheckPerformanceWarnings() const
{
    if (bLogPerformanceWarnings && CurrentMetrics.CurrentFPS < WarningFPSThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warning: FPS dropped to %.1f (threshold: %.1f)"), 
               CurrentMetrics.CurrentFPS, WarningFPSThreshold);
        
        // Log additional context
        UE_LOG(LogTemp, Warning, TEXT("Performance Details - Game Thread: %.2fms, Render Thread: %.2fms, GPU: %.2fms"), 
               CurrentMetrics.GameThreadTime, CurrentMetrics.RenderThreadTime, CurrentMetrics.GPUTime);
    }
}

float APerf_FPSMonitor::GetCurrentFPS() const
{
    return CurrentMetrics.CurrentFPS;
}

float APerf_FPSMonitor::GetAverageFPS() const
{
    return CurrentMetrics.AverageFPS;
}

EPerf_PerformanceLevel APerf_FPSMonitor::GetPerformanceLevel() const
{
    return PerformanceLevel;
}

void APerf_FPSMonitor::ResetStatistics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    CurrentSampleIndex = 0;
    TotalSamples = 0;
    
    // Reset FPS samples to 60 FPS
    for (int32 i = 0; i < SampleCount; i++)
    {
        FPSSamples[i] = 60.0f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance statistics reset"));
}

void APerf_FPSMonitor::SetMonitoringEnabled(bool bEnabled)
{
    bEnableMonitoring = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

FString APerf_FPSMonitor::GetPerformanceReport() const
{
    FString Report = FString::Printf(TEXT("=== PERFORMANCE REPORT ===\n"));
    Report += FString::Printf(TEXT("Current FPS: %.1f\n"), CurrentMetrics.CurrentFPS);
    Report += FString::Printf(TEXT("Average FPS: %.1f\n"), CurrentMetrics.AverageFPS);
    Report += FString::Printf(TEXT("Min FPS: %.1f\n"), CurrentMetrics.MinFPS);
    Report += FString::Printf(TEXT("Max FPS: %.1f\n"), CurrentMetrics.MaxFPS);
    Report += FString::Printf(TEXT("Performance Level: %s\n"), 
        PerformanceLevel == EPerf_PerformanceLevel::Excellent ? TEXT("Excellent") :
        PerformanceLevel == EPerf_PerformanceLevel::Good ? TEXT("Good") :
        PerformanceLevel == EPerf_PerformanceLevel::Acceptable ? TEXT("Acceptable") :
        PerformanceLevel == EPerf_PerformanceLevel::Poor ? TEXT("Poor") : TEXT("Critical"));
    Report += FString::Printf(TEXT("Game Thread: %.2fms\n"), CurrentMetrics.GameThreadTime);
    Report += FString::Printf(TEXT("Render Thread: %.2fms\n"), CurrentMetrics.RenderThreadTime);
    Report += FString::Printf(TEXT("GPU Time: %.2fms\n"), CurrentMetrics.GPUTime);
    Report += FString::Printf(TEXT("Draw Calls: %d\n"), CurrentMetrics.DrawCalls);
    Report += FString::Printf(TEXT("Triangles: %d\n"), CurrentMetrics.Triangles);
    
    return Report;
}

bool APerf_FPSMonitor::IsPerformanceCritical() const
{
    return PerformanceLevel == EPerf_PerformanceLevel::Critical || 
           PerformanceLevel == EPerf_PerformanceLevel::Poor;
}