#include "Perf_PhysicsArchitecturalPerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"

UPerf_PhysicsArchitecturalPerformanceMonitor::UPerf_PhysicsArchitecturalPerformanceMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default performance settings
    TargetFrameTimeMS = 16.67f; // 60 FPS target
    MaxAllowedComplianceOverheadMS = 2.0f; // Max 2ms overhead for compliance checks
    OptimizationLevel = EPerf_PhysicsArchitecturalOptimizationLevel::Medium;
    bAdaptiveOptimizationEnabled = true;
    ComplianceCheckFrequency = 1.0f; // Check every second

    // Initialize monitoring state
    bIsMonitoring = false;
    LastFrameTime = 0.0f;
    ComplianceOverheadAccumulator = 0.0f;
    FrameCounter = 0;

    // Set performance thresholds
    CriticalFrameTimeThreshold = 33.33f; // 30 FPS
    WarningFrameTimeThreshold = 20.0f; // 50 FPS
    MaxPhysicsActorsForOptimalPerformance = 100;

    // Initialize timing
    LastUpdateTime = 0.0;
    ComplianceCheckStartTime = 0.0;
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Architectural Performance Monitor initialized"));
    
    // Auto-start monitoring if enabled
    if (bAdaptiveOptimizationEnabled)
    {
        StartPerformanceMonitoring();
    }
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsMonitoring)
    {
        UpdateMetrics(DeltaTime);
        AnalyzePerformanceTrends();
        
        if (bAdaptiveOptimizationEnabled)
        {
            ApplyOptimizations();
        }
        
        CheckPerformanceThresholds();
    }
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    FrameCounter = 0;
    FrameTimeHistory.Empty();
    ComplianceCheckTimes.Empty();
    ComplianceOverheadAccumulator = 0.0f;
    LastUpdateTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Architectural Performance Monitoring started"));
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("Physics Architectural Performance Monitoring stopped"));
}

FPerf_PhysicsArchitecturalMetrics UPerf_PhysicsArchitecturalPerformanceMonitor::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::SetOptimizationLevel(EPerf_PhysicsArchitecturalOptimizationLevel NewLevel)
{
    OptimizationLevel = NewLevel;
    
    // Adjust settings based on optimization level
    switch (OptimizationLevel)
    {
        case EPerf_PhysicsArchitecturalOptimizationLevel::Low:
            ComplianceCheckFrequency = 2.0f;
            MaxAllowedComplianceOverheadMS = 5.0f;
            break;
        case EPerf_PhysicsArchitecturalOptimizationLevel::Medium:
            ComplianceCheckFrequency = 1.0f;
            MaxAllowedComplianceOverheadMS = 2.0f;
            break;
        case EPerf_PhysicsArchitecturalOptimizationLevel::High:
            ComplianceCheckFrequency = 0.5f;
            MaxAllowedComplianceOverheadMS = 1.0f;
            break;
        case EPerf_PhysicsArchitecturalOptimizationLevel::Ultra:
            ComplianceCheckFrequency = 0.1f;
            MaxAllowedComplianceOverheadMS = 0.5f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Optimization level set to: %d"), (int32)OptimizationLevel);
}

bool UPerf_PhysicsArchitecturalPerformanceMonitor::IsPerformanceWithinBudget() const
{
    return CurrentMetrics.bIsPerformanceWithinBudget;
}

float UPerf_PhysicsArchitecturalPerformanceMonitor::GetArchitecturalComplianceOverhead() const
{
    return CurrentMetrics.ArchitecturalComplianceOverheadMS;
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::OptimizePhysicsArchitecturalIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Optimizing Physics Architectural Integration"));
    
    // Reduce compliance check frequency if overhead is too high
    if (CurrentMetrics.ArchitecturalComplianceOverheadMS > MaxAllowedComplianceOverheadMS)
    {
        ComplianceCheckFrequency = FMath::Max(0.1f, ComplianceCheckFrequency * 0.8f);
        UE_LOG(LogTemp, Warning, TEXT("Reduced compliance check frequency to: %f"), ComplianceCheckFrequency);
    }
    
    // Adjust optimization level based on performance
    if (CurrentMetrics.FrameTimeMS > CriticalFrameTimeThreshold)
    {
        if (OptimizationLevel != EPerf_PhysicsArchitecturalOptimizationLevel::Low)
        {
            SetOptimizationLevel(EPerf_PhysicsArchitecturalOptimizationLevel::Low);
            LogPerformanceWarning(TEXT("Critical performance detected - switching to Low optimization"));
        }
    }
    else if (CurrentMetrics.FrameTimeMS > WarningFrameTimeThreshold)
    {
        if (OptimizationLevel == EPerf_PhysicsArchitecturalOptimizationLevel::Ultra || 
            OptimizationLevel == EPerf_PhysicsArchitecturalOptimizationLevel::High)
        {
            SetOptimizationLevel(EPerf_PhysicsArchitecturalOptimizationLevel::Medium);
            LogPerformanceWarning(TEXT("Performance warning - switching to Medium optimization"));
        }
    }
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::LogPerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== Physics Architectural Performance Report ==="));
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTimeMS);
    UE_LOG(LogTemp, Log, TEXT("Physics Actors: %d"), CurrentMetrics.PhysicsActorCount);
    UE_LOG(LogTemp, Log, TEXT("Physics Update Time: %.2f ms"), CurrentMetrics.PhysicsUpdateTimeMS);
    UE_LOG(LogTemp, Log, TEXT("Compliance Overhead: %.2f ms"), CurrentMetrics.ArchitecturalComplianceOverheadMS);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Performance Within Budget: %s"), CurrentMetrics.bIsPerformanceWithinBudget ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Optimization Level: %d"), (int32)OptimizationLevel);
    UE_LOG(LogTemp, Log, TEXT("Compliance Check Frequency: %.2f"), ComplianceCheckFrequency);
    UE_LOG(LogTemp, Log, TEXT("==============================================="));
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::ExportMetricsToFile(const FString& FilePath)
{
    FString MetricsData = FString::Printf(TEXT(
        "Physics Architectural Performance Metrics\n"
        "Timestamp: %s\n"
        "Frame Time: %.2f ms\n"
        "Physics Actors: %d\n"
        "Physics Update Time: %.2f ms\n"
        "Compliance Overhead: %.2f ms\n"
        "Memory Usage: %.2f MB\n"
        "Performance Within Budget: %s\n"
        "Optimization Level: %d\n"
        "Compliance Check Frequency: %.2f\n"
    ),
    *FDateTime::Now().ToString(),
    CurrentMetrics.FrameTimeMS,
    CurrentMetrics.PhysicsActorCount,
    CurrentMetrics.PhysicsUpdateTimeMS,
    CurrentMetrics.ArchitecturalComplianceOverheadMS,
    CurrentMetrics.MemoryUsageMB,
    CurrentMetrics.bIsPerformanceWithinBudget ? TEXT("Yes") : TEXT("No"),
    (int32)OptimizationLevel,
    ComplianceCheckFrequency
    );

    FFileHelper::SaveStringToFile(MetricsData, *FilePath);
    UE_LOG(LogTemp, Log, TEXT("Performance metrics exported to: %s"), *FilePath);
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::AdjustComplianceCheckFrequency(float NewFrequency)
{
    ComplianceCheckFrequency = FMath::Clamp(NewFrequency, 0.1f, 10.0f);
    UE_LOG(LogTemp, Log, TEXT("Compliance check frequency adjusted to: %f"), ComplianceCheckFrequency);
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::EnableAdaptiveOptimization(bool bEnable)
{
    bAdaptiveOptimizationEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Adaptive optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::UpdateMetrics(float DeltaTime)
{
    FrameCounter++;
    
    // Update frame time
    CurrentMetrics.FrameTimeMS = DeltaTime * 1000.0f;
    LastFrameTime = CurrentMetrics.FrameTimeMS;
    
    // Add to history for trend analysis
    FrameTimeHistory.Add(CurrentMetrics.FrameTimeMS);
    if (FrameTimeHistory.Num() > 100) // Keep last 100 frames
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Collect physics metrics
    CollectPhysicsActorMetrics();
    
    // Calculate compliance overhead
    CurrentMetrics.ArchitecturalComplianceOverheadMS = CalculateComplianceOverhead();
    
    // Update memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    
    // Determine if performance is within budget
    CurrentMetrics.bIsPerformanceWithinBudget = 
        (CurrentMetrics.FrameTimeMS <= TargetFrameTimeMS) &&
        (CurrentMetrics.ArchitecturalComplianceOverheadMS <= MaxAllowedComplianceOverheadMS);
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::AnalyzePerformanceTrends()
{
    if (FrameTimeHistory.Num() < 10) return;
    
    // Calculate average frame time over last 10 frames
    float RecentAverage = 0.0f;
    for (int32 i = FrameTimeHistory.Num() - 10; i < FrameTimeHistory.Num(); i++)
    {
        RecentAverage += FrameTimeHistory[i];
    }
    RecentAverage /= 10.0f;
    
    // Detect performance degradation trend
    if (RecentAverage > TargetFrameTimeMS * 1.2f) // 20% over target
    {
        if (bAdaptiveOptimizationEnabled)
        {
            AdjustOptimizationBasedOnPerformance();
        }
    }
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::ApplyOptimizations()
{
    // Apply optimizations based on current performance
    if (!CurrentMetrics.bIsPerformanceWithinBudget)
    {
        OptimizePhysicsArchitecturalIntegration();
    }
}

float UPerf_PhysicsArchitecturalPerformanceMonitor::CalculateComplianceOverhead()
{
    if (ComplianceCheckTimes.Num() == 0) return 0.0f;
    
    float TotalOverhead = 0.0f;
    for (double CheckTime : ComplianceCheckTimes)
    {
        TotalOverhead += CheckTime;
    }
    
    return TotalOverhead / ComplianceCheckTimes.Num() * 1000.0f; // Convert to milliseconds
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::CollectPhysicsActorMetrics()
{
    if (UWorld* World = GetWorld())
    {
        int32 PhysicsActorCount = 0;
        float PhysicsUpdateTime = 0.0f;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        PhysicsActorCount++;
                    }
                }
            }
        }
        
        CurrentMetrics.PhysicsActorCount = PhysicsActorCount;
        CurrentMetrics.PhysicsUpdateTimeMS = PhysicsUpdateTime;
    }
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::CheckPerformanceThresholds()
{
    if (CurrentMetrics.FrameTimeMS > CriticalFrameTimeThreshold)
    {
        LogPerformanceWarning(FString::Printf(TEXT("CRITICAL: Frame time %.2f ms exceeds threshold %.2f ms"), 
            CurrentMetrics.FrameTimeMS, CriticalFrameTimeThreshold));
    }
    else if (CurrentMetrics.FrameTimeMS > WarningFrameTimeThreshold)
    {
        LogPerformanceWarning(FString::Printf(TEXT("WARNING: Frame time %.2f ms exceeds warning threshold %.2f ms"), 
            CurrentMetrics.FrameTimeMS, WarningFrameTimeThreshold));
    }
    
    if (CurrentMetrics.ArchitecturalComplianceOverheadMS > MaxAllowedComplianceOverheadMS)
    {
        LogPerformanceWarning(FString::Printf(TEXT("WARNING: Compliance overhead %.2f ms exceeds limit %.2f ms"), 
            CurrentMetrics.ArchitecturalComplianceOverheadMS, MaxAllowedComplianceOverheadMS));
    }
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::LogPerformanceWarning(const FString& Warning)
{
    UE_LOG(LogTemp, Warning, TEXT("Physics Architectural Performance: %s"), *Warning);
}

void UPerf_PhysicsArchitecturalPerformanceMonitor::AdjustOptimizationBasedOnPerformance()
{
    // Automatically adjust optimization settings based on performance
    if (CurrentMetrics.FrameTimeMS > CriticalFrameTimeThreshold)
    {
        // Emergency optimization
        ComplianceCheckFrequency = FMath::Max(0.1f, ComplianceCheckFrequency * 0.5f);
        SetOptimizationLevel(EPerf_PhysicsArchitecturalOptimizationLevel::Low);
    }
    else if (CurrentMetrics.FrameTimeMS > WarningFrameTimeThreshold)
    {
        // Moderate optimization
        ComplianceCheckFrequency = FMath::Max(0.5f, ComplianceCheckFrequency * 0.8f);
    }
}