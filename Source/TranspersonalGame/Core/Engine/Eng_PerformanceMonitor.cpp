#include "Eng_PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"

UEng_PerformanceMonitor::UEng_PerformanceMonitor()
{
    bIsMonitoring = false;
    TargetFrameRate = 60.0f;
    MonitoringInterval = 0.1f; // Update every 100ms
    MaxHistorySize = 300; // 30 seconds at 10Hz
    LastMonitorTime = 0.0f;
    TotalFrameTime = 0.0f;
    FrameCount = 0;
}

void UEng_PerformanceMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Performance Monitor initialized"));
    
    // Start monitoring by default
    StartPerformanceMonitoring();
}

void UEng_PerformanceMonitor::Deinitialize()
{
    StopPerformanceMonitoring();
    Super::Deinitialize();
}

void UEng_PerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsMonitoring)
    {
        UpdateMetrics(DeltaTime);
    }
}

FEng_PerformanceMetrics UEng_PerformanceMonitor::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

EEng_PerformanceLevel UEng_PerformanceMonitor::GetPerformanceLevel() const
{
    return CurrentMetrics.PerformanceLevel;
}

void UEng_PerformanceMonitor::StartPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        bIsMonitoring = true;
        ResetMetrics();
        UE_LOG(LogTemp, Log, TEXT("Engine Architect: Performance monitoring started"));
    }
}

void UEng_PerformanceMonitor::StopPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        bIsMonitoring = false;
        UE_LOG(LogTemp, Log, TEXT("Engine Architect: Performance monitoring stopped"));
    }
}

void UEng_PerformanceMonitor::ResetMetrics()
{
    CurrentMetrics = FEng_PerformanceMetrics();
    FPSHistory.Empty();
    LastMonitorTime = 0.0f;
    TotalFrameTime = 0.0f;
    FrameCount = 0;
}

bool UEng_PerformanceMonitor::IsMonitoringActive() const
{
    return bIsMonitoring;
}

void UEng_PerformanceMonitor::SetTargetFPS(float TargetFPS)
{
    TargetFrameRate = FMath::Clamp(TargetFPS, 15.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Target FPS set to %.1f"), TargetFrameRate);
}

float UEng_PerformanceMonitor::GetTargetFPS() const
{
    return TargetFrameRate;
}

TArray<float> UEng_PerformanceMonitor::GetFPSHistory() const
{
    return FPSHistory;
}

void UEng_PerformanceMonitor::UpdateMetrics(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Calculate current FPS
    CurrentMetrics.FrameTime = DeltaTime;
    CurrentMetrics.CurrentFPS = DeltaTime > 0.0f ? 1.0f / DeltaTime : 0.0f;
    
    // Update running totals
    TotalFrameTime += DeltaTime;
    FrameCount++;
    
    // Update metrics every monitoring interval
    if (CurrentTime - LastMonitorTime >= MonitoringInterval)
    {
        // Add to FPS history
        FPSHistory.Add(CurrentMetrics.CurrentFPS);
        if (FPSHistory.Num() > MaxHistorySize)
        {
            FPSHistory.RemoveAt(0);
        }
        
        // Calculate averages and extremes
        CalculateAverages();
        
        // Determine performance level
        CurrentMetrics.PerformanceLevel = DeterminePerformanceLevel(CurrentMetrics.CurrentFPS);
        
        // Get memory usage (simplified)
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
        
        // Estimate draw calls and triangles (placeholder values)
        CurrentMetrics.DrawCalls = FMath::RandRange(500, 2000);
        CurrentMetrics.TriangleCount = FMath::RandRange(50000, 200000);
        
        // Check for performance warnings
        if (CurrentMetrics.PerformanceLevel >= EEng_PerformanceLevel::Poor)
        {
            LogPerformanceWarning(FString::Printf(TEXT("Low performance detected: %.1f FPS"), 
                                                 CurrentMetrics.CurrentFPS));
        }
        
        LastMonitorTime = CurrentTime;
    }
}

void UEng_PerformanceMonitor::CalculateAverages()
{
    if (FPSHistory.Num() > 0)
    {
        float Sum = 0.0f;
        CurrentMetrics.MinFPS = FPSHistory[0];
        CurrentMetrics.MaxFPS = FPSHistory[0];
        
        for (float FPS : FPSHistory)
        {
            Sum += FPS;
            CurrentMetrics.MinFPS = FMath::Min(CurrentMetrics.MinFPS, FPS);
            CurrentMetrics.MaxFPS = FMath::Max(CurrentMetrics.MaxFPS, FPS);
        }
        
        CurrentMetrics.AverageFPS = Sum / FPSHistory.Num();
    }
}

EEng_PerformanceLevel UEng_PerformanceMonitor::DeterminePerformanceLevel(float FPS) const
{
    if (FPS >= 60.0f)
    {
        return EEng_PerformanceLevel::Excellent;
    }
    else if (FPS >= 45.0f)
    {
        return EEng_PerformanceLevel::Good;
    }
    else if (FPS >= 30.0f)
    {
        return EEng_PerformanceLevel::Fair;
    }
    else if (FPS >= 15.0f)
    {
        return EEng_PerformanceLevel::Poor;
    }
    else
    {
        return EEng_PerformanceLevel::Critical;
    }
}

void UEng_PerformanceMonitor::LogPerformanceWarning(const FString& Warning)
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Performance Warning: %s"), *Warning);
}

// Performance Component Implementation

UEng_PerformanceComponent::UEng_PerformanceComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    bTrackPerformance = true;
    PerformanceBudget = 1.0f; // 1ms budget
    CurrentPerformanceCost = 0.0f;
    LastTickTime = 0.0f;
}

void UEng_PerformanceComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (bTrackPerformance)
    {
        UE_LOG(LogTemp, Log, TEXT("Engine Architect: Performance tracking enabled for %s"), 
               *GetOwner()->GetName());
    }
}

void UEng_PerformanceComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
                                             FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bTrackPerformance)
    {
        CalculatePerformanceCost(DeltaTime);
    }
}

void UEng_PerformanceComponent::EnablePerformanceTracking(bool bEnable)
{
    bTrackPerformance = bEnable;
    
    if (bEnable)
    {
        UE_LOG(LogTemp, Log, TEXT("Engine Architect: Performance tracking enabled for %s"), 
               *GetOwner()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Engine Architect: Performance tracking disabled for %s"), 
               *GetOwner()->GetName());
    }
}

float UEng_PerformanceComponent::GetActorPerformanceCost() const
{
    return CurrentPerformanceCost;
}

void UEng_PerformanceComponent::SetPerformanceBudget(float Budget)
{
    PerformanceBudget = FMath::Max(0.1f, Budget); // Minimum 0.1ms budget
}

void UEng_PerformanceComponent::CalculatePerformanceCost(float DeltaTime)
{
    float CurrentTime = FPlatformTime::Seconds();
    
    if (LastTickTime > 0.0f)
    {
        // Simple performance cost calculation based on tick frequency
        float TickCost = (CurrentTime - LastTickTime) * 1000.0f; // Convert to milliseconds
        CurrentPerformanceCost = FMath::Lerp(CurrentPerformanceCost, TickCost, 0.1f); // Smooth average
        
        // Check if over budget
        if (CurrentPerformanceCost > PerformanceBudget)
        {
            UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Actor %s over performance budget: %.2fms (Budget: %.2fms)"), 
                   *GetOwner()->GetName(), CurrentPerformanceCost, PerformanceBudget);
        }
    }
    
    LastTickTime = CurrentTime;
}