#include "Perf_SystemsIntegrationOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogPerfSystemsIntegration, Log, All);

UPerf_SystemsIntegrationOptimizer::UPerf_SystemsIntegrationOptimizer()
    : bPerformanceMonitoringEnabled(false)
    , LastOptimizationTime(0.0f)
    , PerformanceSamplingInterval(1.0f)
    , MaxHistorySize(60)
{
    // Initialize default settings
    CurrentSettings.IntegrationLevel = EPerf_SystemIntegrationLevel::Standard;
    CurrentSettings.TargetFrameRate = 60.0f;
    CurrentSettings.MaxMemoryBudgetMB = 512.0f;
    CurrentSettings.bEnableAdaptiveOptimization = true;
    CurrentSettings.bEnablePerformanceLogging = false;

    // Initialize metrics
    CurrentMetrics.InitializationTime = 0.0f;
    CurrentMetrics.AverageFrameTime = 0.0f;
    CurrentMetrics.MemoryUsageMB = 0.0f;
    CurrentMetrics.ActiveSystemCount = 0;
    CurrentMetrics.bIsOptimized = false;
}

void UPerf_SystemsIntegrationOptimizer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Initializing Systems Integration Optimizer"));
    
    // Initialize performance monitoring
    InitializePerformanceMonitoring();
    
    // Apply default optimization settings
    ApplyOptimizationSettings(CurrentSettings);
    
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Systems Integration Optimizer initialized successfully"));
}

void UPerf_SystemsIntegrationOptimizer::Deinitialize()
{
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Deinitializing Systems Integration Optimizer"));
    
    // Clear performance monitoring timer
    if (UWorld* World = GetWorld())
    {
        if (FTimerManager* TimerManager = &World->GetTimerManager())
        {
            TimerManager->ClearTimer(PerformanceMonitoringTimer);
        }
    }
    
    bPerformanceMonitoringEnabled = false;
    
    Super::Deinitialize();
}

void UPerf_SystemsIntegrationOptimizer::OptimizeSystemsIntegration()
{
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Optimizing systems integration"));
    
    // Update current performance metrics
    UpdatePerformanceMetrics();
    
    // Apply integration level optimizations
    ApplyIntegrationLevelOptimizations(CurrentSettings.IntegrationLevel);
    
    // Check if adaptive optimization should trigger
    if (CurrentSettings.bEnableAdaptiveOptimization && ShouldTriggerAdaptiveOptimization())
    {
        PerformAdaptiveOptimization();
    }
    
    // Update optimization timestamp
    LastOptimizationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Log performance metrics if enabled
    if (CurrentSettings.bEnablePerformanceLogging)
    {
        LogPerformanceMetrics();
    }
    
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Systems integration optimization complete"));
}

FPerf_SystemPerformanceMetrics UPerf_SystemsIntegrationOptimizer::AnalyzeSystemsPerformance()
{
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Analyzing systems performance"));
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Calculate average frame time from history
    if (FrameTimeHistory.Num() > 0)
    {
        float TotalFrameTime = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            TotalFrameTime += FrameTime;
        }
        CurrentMetrics.AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
    }
    
    // Estimate memory usage (simplified calculation)
    CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    
    // Count active systems (simplified - would integrate with actual systems registry)
    CurrentMetrics.ActiveSystemCount = 7; // Based on Core Systems count
    
    // Determine if systems are optimized
    CurrentMetrics.bIsOptimized = (CurrentMetrics.AverageFrameTime <= (1000.0f / CurrentSettings.TargetFrameRate)) &&
                                  (CurrentMetrics.MemoryUsageMB <= CurrentSettings.MaxMemoryBudgetMB);
    
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Performance analysis complete - Optimized: %s"), 
           CurrentMetrics.bIsOptimized ? TEXT("Yes") : TEXT("No"));
    
    return CurrentMetrics;
}

void UPerf_SystemsIntegrationOptimizer::ApplyOptimizationSettings(const FPerf_IntegrationOptimizationSettings& Settings)
{
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Applying optimization settings"));
    
    CurrentSettings = Settings;
    
    // Apply integration level optimizations
    ApplyIntegrationLevelOptimizations(Settings.IntegrationLevel);
    
    // Update performance monitoring based on settings
    if (Settings.bEnablePerformanceLogging != bPerformanceMonitoringEnabled)
    {
        bPerformanceMonitoringEnabled = Settings.bEnablePerformanceLogging;
        if (bPerformanceMonitoringEnabled)
        {
            InitializePerformanceMonitoring();
        }
    }
    
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Optimization settings applied successfully"));
}

FPerf_IntegrationOptimizationSettings UPerf_SystemsIntegrationOptimizer::GetOptimizationSettings() const
{
    return CurrentSettings;
}

void UPerf_SystemsIntegrationOptimizer::SetAdaptiveOptimization(bool bEnabled)
{
    CurrentSettings.bEnableAdaptiveOptimization = bEnabled;
    
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Adaptive optimization %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

float UPerf_SystemsIntegrationOptimizer::GetSystemPerformanceMetric(const FString& SystemName)
{
    // This would integrate with actual systems registry to get specific metrics
    // For now, return a simulated metric based on current frame time
    UpdatePerformanceMetrics();
    
    if (SystemName.Contains(TEXT("Physics")))
    {
        return CurrentMetrics.AverageFrameTime * 0.3f; // Physics typically uses 30% of frame time
    }
    else if (SystemName.Contains(TEXT("Rendering")))
    {
        return CurrentMetrics.AverageFrameTime * 0.4f; // Rendering typically uses 40% of frame time
    }
    else if (SystemName.Contains(TEXT("AI")))
    {
        return CurrentMetrics.AverageFrameTime * 0.2f; // AI typically uses 20% of frame time
    }
    
    return CurrentMetrics.AverageFrameTime * 0.1f; // Other systems use remaining time
}

void UPerf_SystemsIntegrationOptimizer::ForceOptimizationUpdate()
{
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Forcing optimization update"));
    OptimizeSystemsIntegration();
}

bool UPerf_SystemsIntegrationOptimizer::AreSystemsOptimized() const
{
    return CurrentMetrics.bIsOptimized;
}

EPerf_SystemIntegrationLevel UPerf_SystemsIntegrationOptimizer::GetRecommendedIntegrationLevel() const
{
    // Analyze current performance to recommend integration level
    float CurrentFrameRate = CurrentMetrics.AverageFrameTime > 0.0f ? (1000.0f / CurrentMetrics.AverageFrameTime) : 60.0f;
    
    if (CurrentFrameRate >= CurrentSettings.TargetFrameRate * 1.2f)
    {
        return EPerf_SystemIntegrationLevel::Maximum;
    }
    else if (CurrentFrameRate >= CurrentSettings.TargetFrameRate)
    {
        return EPerf_SystemIntegrationLevel::Enhanced;
    }
    else if (CurrentFrameRate >= CurrentSettings.TargetFrameRate * 0.8f)
    {
        return EPerf_SystemIntegrationLevel::Standard;
    }
    else
    {
        return EPerf_SystemIntegrationLevel::Minimal;
    }
}

void UPerf_SystemsIntegrationOptimizer::ResetOptimization()
{
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Resetting optimization to defaults"));
    
    // Reset to default settings
    FPerf_IntegrationOptimizationSettings DefaultSettings;
    ApplyOptimizationSettings(DefaultSettings);
    
    // Clear performance history
    FrameTimeHistory.Empty();
    
    // Reset metrics
    CurrentMetrics = FPerf_SystemPerformanceMetrics();
}

void UPerf_SystemsIntegrationOptimizer::InitializePerformanceMonitoring()
{
    if (!bPerformanceMonitoringEnabled)
    {
        bPerformanceMonitoringEnabled = true;
        
        // Set up performance monitoring timer
        if (UWorld* World = GetWorld())
        {
            FTimerManager& TimerManager = World->GetTimerManager();
            TimerManager.SetTimer(
                PerformanceMonitoringTimer,
                this,
                &UPerf_SystemsIntegrationOptimizer::UpdatePerformanceMetrics,
                PerformanceSamplingInterval,
                true
            );
        }
        
        UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Performance monitoring initialized"));
    }
}

void UPerf_SystemsIntegrationOptimizer::UpdatePerformanceMetrics()
{
    if (UWorld* World = GetWorld())
    {
        // Get current frame time
        float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
        
        // Add to history
        FrameTimeHistory.Add(CurrentFrameTime);
        
        // Maintain history size
        if (FrameTimeHistory.Num() > MaxHistorySize)
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Update current metrics
        CurrentMetrics.AverageFrameTime = CurrentFrameTime;
        CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    }
}

void UPerf_SystemsIntegrationOptimizer::ApplyIntegrationLevelOptimizations(EPerf_SystemIntegrationLevel Level)
{
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Applying integration level optimizations: %d"), (int32)Level);
    
    switch (Level)
    {
        case EPerf_SystemIntegrationLevel::Minimal:
            // Minimal integration - basic systems only
            PerformanceSamplingInterval = 2.0f;
            break;
            
        case EPerf_SystemIntegrationLevel::Standard:
            // Standard integration - balanced performance
            PerformanceSamplingInterval = 1.0f;
            break;
            
        case EPerf_SystemIntegrationLevel::Enhanced:
            // Enhanced integration - more frequent monitoring
            PerformanceSamplingInterval = 0.5f;
            break;
            
        case EPerf_SystemIntegrationLevel::Maximum:
            // Maximum integration - real-time monitoring
            PerformanceSamplingInterval = 0.1f;
            break;
    }
    
    // Restart performance monitoring with new interval
    if (bPerformanceMonitoringEnabled && GetWorld())
    {
        FTimerManager& TimerManager = GetWorld()->GetTimerManager();
        TimerManager.ClearTimer(PerformanceMonitoringTimer);
        TimerManager.SetTimer(
            PerformanceMonitoringTimer,
            this,
            &UPerf_SystemsIntegrationOptimizer::UpdatePerformanceMetrics,
            PerformanceSamplingInterval,
            true
        );
    }
}

bool UPerf_SystemsIntegrationOptimizer::ShouldTriggerAdaptiveOptimization() const
{
    // Check if performance has degraded below target
    float CurrentFrameRate = CurrentMetrics.AverageFrameTime > 0.0f ? (1000.0f / CurrentMetrics.AverageFrameTime) : 60.0f;
    
    return (CurrentFrameRate < CurrentSettings.TargetFrameRate * 0.9f) || 
           (CurrentMetrics.MemoryUsageMB > CurrentSettings.MaxMemoryBudgetMB * 0.9f);
}

void UPerf_SystemsIntegrationOptimizer::PerformAdaptiveOptimization()
{
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Performing adaptive optimization"));
    
    // Get recommended integration level based on current performance
    EPerf_SystemIntegrationLevel RecommendedLevel = GetRecommendedIntegrationLevel();
    
    // Apply recommended level if different from current
    if (RecommendedLevel != CurrentSettings.IntegrationLevel)
    {
        CurrentSettings.IntegrationLevel = RecommendedLevel;
        ApplyIntegrationLevelOptimizations(RecommendedLevel);
        
        UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Adaptive optimization changed integration level to: %d"), (int32)RecommendedLevel);
    }
}

void UPerf_SystemsIntegrationOptimizer::LogPerformanceMetrics() const
{
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("=== PERFORMANCE METRICS ==="));
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Average Frame Time: %.2f ms"), CurrentMetrics.AverageFrameTime);
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Active Systems: %d"), CurrentMetrics.ActiveSystemCount);
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Optimized: %s"), CurrentMetrics.bIsOptimized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogPerfSystemsIntegration, Log, TEXT("Integration Level: %d"), (int32)CurrentSettings.IntegrationLevel);
}