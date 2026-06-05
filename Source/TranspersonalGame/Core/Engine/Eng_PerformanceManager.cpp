#include "Eng_PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

UEng_PerformanceManager::UEng_PerformanceManager()
{
    bIsMonitoring = false;
    CurrentPerformanceLevel = EEng_PerformanceLevel::High;
    MetricsUpdateInterval = 1.0f;
    bBudgetViolationDetected = false;
    
    // Initialize default budget
    PerformanceBudget = FEng_PerformanceBudget();
    CurrentMetrics = FEng_PerformanceMetrics();
}

void UEng_PerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect Performance Manager Initialized"));
    
    // Set default performance level based on platform
    #if PLATFORM_DESKTOP
        SetPerformanceLevel(EEng_PerformanceLevel::High);
    #elif PLATFORM_CONSOLE
        SetPerformanceLevel(EEng_PerformanceLevel::Medium);
    #else
        SetPerformanceLevel(EEng_PerformanceLevel::Low);
    #endif
    
    // Start monitoring by default
    StartPerformanceMonitoring();
}

void UEng_PerformanceManager::Deinitialize()
{
    StopPerformanceMonitoring();
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect Performance Manager Deinitialized"));
    
    Super::Deinitialize();
}

void UEng_PerformanceManager::StartPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = true;
    
    // Start metrics update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MetricsUpdateTimer,
            this,
            &UEng_PerformanceManager::UpdateMetrics,
            MetricsUpdateInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
}

void UEng_PerformanceManager::StopPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = false;
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
}

FEng_PerformanceMetrics UEng_PerformanceManager::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

bool UEng_PerformanceManager::IsPerformanceWithinBudget() const
{
    const FEng_PerformanceMetrics& Metrics = CurrentMetrics;
    const FEng_PerformanceBudget& Budget = PerformanceBudget;
    
    bool bWithinBudget = true;
    
    if (Metrics.FrameTime > Budget.MaxFrameTime)
    {
        bWithinBudget = false;
    }
    
    if (Metrics.GPUTime > Budget.MaxGPUTime)
    {
        bWithinBudget = false;
    }
    
    if (Metrics.CPUTime > Budget.MaxCPUTime)
    {
        bWithinBudget = false;
    }
    
    if (Metrics.DrawCalls > Budget.MaxDrawCalls)
    {
        bWithinBudget = false;
    }
    
    if (Metrics.Triangles > Budget.MaxTriangles)
    {
        bWithinBudget = false;
    }
    
    if (Metrics.MemoryUsageMB > Budget.MaxMemoryMB)
    {
        bWithinBudget = false;
    }
    
    return bWithinBudget;
}

void UEng_PerformanceManager::SetPerformanceLevel(EEng_PerformanceLevel Level)
{
    CurrentPerformanceLevel = Level;
    ApplyLevelSettings(Level);
    
    UE_LOG(LogTemp, Log, TEXT("Performance level set to: %d"), (int32)Level);
}

EEng_PerformanceLevel UEng_PerformanceManager::GetCurrentPerformanceLevel() const
{
    return CurrentPerformanceLevel;
}

void UEng_PerformanceManager::SetPerformanceBudget(const FEng_PerformanceBudget& Budget)
{
    PerformanceBudget = Budget;
    UE_LOG(LogTemp, Log, TEXT("Performance budget updated"));
}

FEng_PerformanceBudget UEng_PerformanceManager::GetPerformanceBudget() const
{
    return PerformanceBudget;
}

void UEng_PerformanceManager::OptimizeForCurrentHardware()
{
    UE_LOG(LogTemp, Log, TEXT("Optimizing for current hardware..."));
    
    // Detect hardware capabilities and adjust settings
    OptimizeDrawCalls();
    OptimizeMemoryUsage();
    OptimizeLODSettings();
    
    // Apply optimizations
    ApplyPerformanceSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Hardware optimization complete"));
}

void UEng_PerformanceManager::ApplyPerformanceSettings()
{
    ApplyLevelSettings(CurrentPerformanceLevel);
    UE_LOG(LogTemp, Log, TEXT("Performance settings applied"));
}

void UEng_PerformanceManager::ValidatePerformanceCompliance()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE COMPLIANCE VALIDATION ==="));
    
    const FEng_PerformanceMetrics& Metrics = CurrentMetrics;
    const FEng_PerformanceBudget& Budget = PerformanceBudget;
    
    bool bCompliant = true;
    
    // Frame time validation
    if (Metrics.FrameTime > Budget.MaxFrameTime)
    {
        UE_LOG(LogTemp, Error, TEXT("FRAME TIME VIOLATION: %.2fms (Budget: %.2fms)"), 
               Metrics.FrameTime, Budget.MaxFrameTime);
        bCompliant = false;
    }
    
    // GPU time validation
    if (Metrics.GPUTime > Budget.MaxGPUTime)
    {
        UE_LOG(LogTemp, Error, TEXT("GPU TIME VIOLATION: %.2fms (Budget: %.2fms)"), 
               Metrics.GPUTime, Budget.MaxGPUTime);
        bCompliant = false;
    }
    
    // CPU time validation
    if (Metrics.CPUTime > Budget.MaxCPUTime)
    {
        UE_LOG(LogTemp, Error, TEXT("CPU TIME VIOLATION: %.2fms (Budget: %.2fms)"), 
               Metrics.CPUTime, Budget.MaxCPUTime);
        bCompliant = false;
    }
    
    // Draw calls validation
    if (Metrics.DrawCalls > Budget.MaxDrawCalls)
    {
        UE_LOG(LogTemp, Error, TEXT("DRAW CALLS VIOLATION: %d (Budget: %d)"), 
               Metrics.DrawCalls, Budget.MaxDrawCalls);
        bCompliant = false;
    }
    
    // Triangle count validation
    if (Metrics.Triangles > Budget.MaxTriangles)
    {
        UE_LOG(LogTemp, Error, TEXT("TRIANGLE COUNT VIOLATION: %d (Budget: %d)"), 
               Metrics.Triangles, Budget.MaxTriangles);
        bCompliant = false;
    }
    
    // Memory usage validation
    if (Metrics.MemoryUsageMB > Budget.MaxMemoryMB)
    {
        UE_LOG(LogTemp, Error, TEXT("MEMORY USAGE VIOLATION: %.2fMB (Budget: %.2fMB)"), 
               Metrics.MemoryUsageMB, Budget.MaxMemoryMB);
        bCompliant = false;
    }
    
    if (bCompliant)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ ALL PERFORMANCE METRICS WITHIN BUDGET"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ PERFORMANCE BUDGET VIOLATIONS DETECTED"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATION COMPLETE ==="));
}

void UEng_PerformanceManager::GeneratePerformanceReport()
{
    FString ReportContent;
    FDateTime Now = FDateTime::Now();
    
    ReportContent += FString::Printf(TEXT("=== ENGINE ARCHITECT PERFORMANCE REPORT ===\n"));
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *Now.ToString());
    ReportContent += FString::Printf(TEXT("Performance Level: %d\n"), (int32)CurrentPerformanceLevel);
    ReportContent += FString::Printf(TEXT("\n"));
    
    // Current metrics
    ReportContent += FString::Printf(TEXT("CURRENT METRICS:\n"));
    ReportContent += FString::Printf(TEXT("Frame Rate: %.2f FPS\n"), CurrentMetrics.FrameRate);
    ReportContent += FString::Printf(TEXT("Frame Time: %.2f ms\n"), CurrentMetrics.FrameTime);
    ReportContent += FString::Printf(TEXT("GPU Time: %.2f ms\n"), CurrentMetrics.GPUTime);
    ReportContent += FString::Printf(TEXT("CPU Time: %.2f ms\n"), CurrentMetrics.CPUTime);
    ReportContent += FString::Printf(TEXT("Draw Calls: %d\n"), CurrentMetrics.DrawCalls);
    ReportContent += FString::Printf(TEXT("Triangles: %d\n"), CurrentMetrics.Triangles);
    ReportContent += FString::Printf(TEXT("Memory Usage: %.2f MB\n"), CurrentMetrics.MemoryUsageMB);
    ReportContent += FString::Printf(TEXT("\n"));
    
    // Budget comparison
    ReportContent += FString::Printf(TEXT("BUDGET COMPARISON:\n"));
    ReportContent += FString::Printf(TEXT("Frame Time: %.2f / %.2f ms %s\n"), 
                                   CurrentMetrics.FrameTime, PerformanceBudget.MaxFrameTime,
                                   CurrentMetrics.FrameTime <= PerformanceBudget.MaxFrameTime ? TEXT("✓") : TEXT("✗"));
    ReportContent += FString::Printf(TEXT("GPU Time: %.2f / %.2f ms %s\n"), 
                                   CurrentMetrics.GPUTime, PerformanceBudget.MaxGPUTime,
                                   CurrentMetrics.GPUTime <= PerformanceBudget.MaxGPUTime ? TEXT("✓") : TEXT("✗"));
    ReportContent += FString::Printf(TEXT("CPU Time: %.2f / %.2f ms %s\n"), 
                                   CurrentMetrics.CPUTime, PerformanceBudget.MaxCPUTime,
                                   CurrentMetrics.CPUTime <= PerformanceBudget.MaxCPUTime ? TEXT("✓") : TEXT("✗"));
    
    // Save report
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("PerformanceReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
    
    UE_LOG(LogTemp, Log, TEXT("Performance report generated: %s"), *ReportPath);
}

void UEng_PerformanceManager::UpdateMetrics()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    // Update current metrics (simplified for now)
    // In a real implementation, these would query actual engine stats
    CurrentMetrics.FrameRate = 1000.0f / CurrentMetrics.FrameTime;
    
    // Simulate some variance in metrics
    float Variance = FMath::RandRange(0.8f, 1.2f);
    CurrentMetrics.FrameTime = 16.67f * Variance;
    CurrentMetrics.GPUTime = 8.0f * Variance;
    CurrentMetrics.CPUTime = 8.0f * Variance;
    CurrentMetrics.DrawCalls = FMath::RandRange(800, 1200);
    CurrentMetrics.Triangles = FMath::RandRange(80000, 120000);
    CurrentMetrics.MemoryUsageMB = 512.0f * Variance;
    
    // Check budget compliance
    CheckBudgetCompliance();
}

void UEng_PerformanceManager::CheckBudgetCompliance()
{
    bool bCurrentlyWithinBudget = IsPerformanceWithinBudget();
    
    if (!bCurrentlyWithinBudget && !bBudgetViolationDetected)
    {
        bBudgetViolationDetected = true;
        UE_LOG(LogTemp, Warning, TEXT("Performance budget violation detected!"));
    }
    else if (bCurrentlyWithinBudget && bBudgetViolationDetected)
    {
        bBudgetViolationDetected = false;
        UE_LOG(LogTemp, Log, TEXT("Performance back within budget"));
    }
}

void UEng_PerformanceManager::ApplyLevelSettings(EEng_PerformanceLevel Level)
{
    switch (Level)
    {
        case EEng_PerformanceLevel::Ultra:
            PerformanceBudget.MaxFrameTime = 16.67f;  // 60 FPS
            PerformanceBudget.MaxDrawCalls = 3000;
            PerformanceBudget.MaxTriangles = 1000000;
            break;
            
        case EEng_PerformanceLevel::High:
            PerformanceBudget.MaxFrameTime = 16.67f;  // 60 FPS
            PerformanceBudget.MaxDrawCalls = 2000;
            PerformanceBudget.MaxTriangles = 500000;
            break;
            
        case EEng_PerformanceLevel::Medium:
            PerformanceBudget.MaxFrameTime = 33.33f;  // 30 FPS
            PerformanceBudget.MaxDrawCalls = 1500;
            PerformanceBudget.MaxTriangles = 300000;
            break;
            
        case EEng_PerformanceLevel::Low:
            PerformanceBudget.MaxFrameTime = 33.33f;  // 30 FPS
            PerformanceBudget.MaxDrawCalls = 1000;
            PerformanceBudget.MaxTriangles = 200000;
            break;
            
        case EEng_PerformanceLevel::Potato:
            PerformanceBudget.MaxFrameTime = 50.0f;   // 20 FPS
            PerformanceBudget.MaxDrawCalls = 500;
            PerformanceBudget.MaxTriangles = 100000;
            break;
    }
}

void UEng_PerformanceManager::OptimizeDrawCalls()
{
    UE_LOG(LogTemp, Log, TEXT("Optimizing draw calls..."));
    // Implementation would optimize batching, instancing, etc.
}

void UEng_PerformanceManager::OptimizeMemoryUsage()
{
    UE_LOG(LogTemp, Log, TEXT("Optimizing memory usage..."));
    // Implementation would optimize texture streaming, garbage collection, etc.
}

void UEng_PerformanceManager::OptimizeLODSettings()
{
    UE_LOG(LogTemp, Log, TEXT("Optimizing LOD settings..."));
    // Implementation would adjust LOD distances, quality settings, etc.
}