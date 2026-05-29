#include "Perf_FrameRateManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/Stats.h"

UPerf_FrameRateManager::UPerf_FrameRateManager()
    : bIsMonitoring(false)
    , TargetFrameRate(60.0f)
    , bDynamicQualityAdjustment(true)
    , MonitoringInterval(0.1f)
    , UltraThreshold(60.0f)
    , HighThreshold(45.0f)
    , MediumThreshold(30.0f)
    , LowThreshold(20.0f)
{
    FrameRateHistory.Reserve(100); // Reserve space for 10 seconds of data at 0.1s intervals
}

void UPerf_FrameRateManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Perf_FrameRateManager initialized"));
    
    // Start monitoring automatically
    StartFrameRateMonitoring();
}

void UPerf_FrameRateManager::Deinitialize()
{
    StopFrameRateMonitoring();
    Super::Deinitialize();
}

void UPerf_FrameRateManager::StartFrameRateMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = true;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerf_FrameRateManager::UpdateFrameRateData,
            MonitoringInterval,
            true
        );
        
        UE_LOG(LogTemp, Log, TEXT("Frame rate monitoring started"));
    }
}

void UPerf_FrameRateManager::StopFrameRateMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
        UE_LOG(LogTemp, Log, TEXT("Frame rate monitoring stopped"));
    }
}

FPerf_FrameRateData UPerf_FrameRateManager::GetCurrentFrameRateData() const
{
    return CurrentFrameData;
}

EPerf_PerformanceLevel UPerf_FrameRateManager::GetCurrentPerformanceLevel() const
{
    return CurrentFrameData.PerformanceLevel;
}

void UPerf_FrameRateManager::SetTargetFrameRate(float TargetFPS)
{
    TargetFrameRate = FMath::Clamp(TargetFPS, 15.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("Target frame rate set to: %.1f FPS"), TargetFrameRate);
}

void UPerf_FrameRateManager::EnableDynamicQualityAdjustment(bool bEnable)
{
    bDynamicQualityAdjustment = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Dynamic quality adjustment: %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled"));
}

void UPerf_FrameRateManager::ForceQualityLevel(EPerf_PerformanceLevel Level)
{
    // Disable dynamic adjustment when forcing a specific level
    bDynamicQualityAdjustment = false;
    CurrentFrameData.PerformanceLevel = Level;
    
    UE_LOG(LogTemp, Log, TEXT("Quality level forced to: %d"), (int32)Level);
}

void UPerf_FrameRateManager::SetPerformanceThresholds(float UltraFPS, float HighFPS, float MediumFPS, float LowFPS)
{
    UltraThreshold = UltraFPS;
    HighThreshold = HighFPS;
    MediumThreshold = MediumFPS;
    LowThreshold = LowFPS;
    
    UE_LOG(LogTemp, Log, TEXT("Performance thresholds updated: Ultra=%.1f, High=%.1f, Medium=%.1f, Low=%.1f"), 
           UltraFPS, HighFPS, MediumFPS, LowFPS);
}

void UPerf_FrameRateManager::ShowPerformanceStats()
{
    if (GEngine)
    {
        FString StatsMessage = FString::Printf(
            TEXT("=== PERFORMANCE STATS ===\nCurrent FPS: %.1f\nAverage FPS: %.1f\nMin FPS: %.1f\nMax FPS: %.1f\nFrame Time: %.2fms\nPerformance Level: %d"),
            CurrentFrameData.CurrentFPS,
            CurrentFrameData.AverageFPS,
            CurrentFrameData.MinFPS,
            CurrentFrameData.MaxFPS,
            CurrentFrameData.FrameTime,
            (int32)CurrentFrameData.PerformanceLevel
        );
        
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, StatsMessage);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *StatsMessage);
    }
}

void UPerf_FrameRateManager::ResetFrameRateStats()
{
    FrameRateHistory.Empty();
    CurrentFrameData = FPerf_FrameRateData();
    UE_LOG(LogTemp, Log, TEXT("Frame rate statistics reset"));
}

void UPerf_FrameRateManager::UpdateFrameRateData()
{
    if (!bIsMonitoring)
    {
        return;
    }

    // Get current frame rate
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        CurrentFrameData.CurrentFPS = 1.0f / DeltaTime;
        CurrentFrameData.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    }
    
    // Add to history
    FrameRateHistory.Add(CurrentFrameData.CurrentFPS);
    
    // Keep only last 100 samples (10 seconds at 0.1s intervals)
    if (FrameRateHistory.Num() > 100)
    {
        FrameRateHistory.RemoveAt(0);
    }
    
    // Calculate statistics
    CalculateAverageFrameRate();
    
    // Update performance level
    CurrentFrameData.PerformanceLevel = DeterminePerformanceLevel(CurrentFrameData.CurrentFPS);
    
    // Adjust quality if enabled
    if (bDynamicQualityAdjustment)
    {
        AdjustQualityBasedOnPerformance();
    }
    
    // Log performance data periodically
    static int32 LogCounter = 0;
    if (++LogCounter >= 50) // Log every 5 seconds
    {
        LogPerformanceData();
        LogCounter = 0;
    }
}

void UPerf_FrameRateManager::CalculateAverageFrameRate()
{
    if (FrameRateHistory.Num() == 0)
    {
        return;
    }
    
    float Sum = 0.0f;
    float Min = FrameRateHistory[0];
    float Max = FrameRateHistory[0];
    
    for (float FPS : FrameRateHistory)
    {
        Sum += FPS;
        Min = FMath::Min(Min, FPS);
        Max = FMath::Max(Max, FPS);
    }
    
    CurrentFrameData.AverageFPS = Sum / FrameRateHistory.Num();
    CurrentFrameData.MinFPS = Min;
    CurrentFrameData.MaxFPS = Max;
}

EPerf_PerformanceLevel UPerf_FrameRateManager::DeterminePerformanceLevel(float FPS) const
{
    if (FPS >= UltraThreshold)
    {
        return EPerf_PerformanceLevel::Ultra;
    }
    else if (FPS >= HighThreshold)
    {
        return EPerf_PerformanceLevel::High;
    }
    else if (FPS >= MediumThreshold)
    {
        return EPerf_PerformanceLevel::Medium;
    }
    else if (FPS >= LowThreshold)
    {
        return EPerf_PerformanceLevel::Low;
    }
    else
    {
        return EPerf_PerformanceLevel::Critical;
    }
}

void UPerf_FrameRateManager::AdjustQualityBasedOnPerformance()
{
    static EPerf_PerformanceLevel LastLevel = EPerf_PerformanceLevel::Ultra;
    EPerf_PerformanceLevel CurrentLevel = CurrentFrameData.PerformanceLevel;
    
    // Only adjust if performance level has changed
    if (CurrentLevel != LastLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance level changed from %d to %d - adjusting quality"), 
               (int32)LastLevel, (int32)CurrentLevel);
        
        // Apply quality adjustments based on performance level
        if (UWorld* World = GetWorld())
        {
            switch (CurrentLevel)
            {
                case EPerf_PerformanceLevel::Critical:
                    // Aggressive optimizations
                    World->Exec(World, TEXT("r.ViewDistanceScale 0.5"));
                    World->Exec(World, TEXT("r.SkeletalMeshLODBias 2"));
                    World->Exec(World, TEXT("r.StaticMeshLODBias 2"));
                    break;
                    
                case EPerf_PerformanceLevel::Low:
                    World->Exec(World, TEXT("r.ViewDistanceScale 0.7"));
                    World->Exec(World, TEXT("r.SkeletalMeshLODBias 1"));
                    World->Exec(World, TEXT("r.StaticMeshLODBias 1"));
                    break;
                    
                case EPerf_PerformanceLevel::Medium:
                    World->Exec(World, TEXT("r.ViewDistanceScale 0.85"));
                    World->Exec(World, TEXT("r.SkeletalMeshLODBias 0"));
                    World->Exec(World, TEXT("r.StaticMeshLODBias 0"));
                    break;
                    
                case EPerf_PerformanceLevel::High:
                case EPerf_PerformanceLevel::Ultra:
                    World->Exec(World, TEXT("r.ViewDistanceScale 1.0"));
                    World->Exec(World, TEXT("r.SkeletalMeshLODBias -1"));
                    World->Exec(World, TEXT("r.StaticMeshLODBias -1"));
                    break;
            }
        }
        
        LastLevel = CurrentLevel;
    }
}

void UPerf_FrameRateManager::LogPerformanceData() const
{
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor - FPS: %.1f (Avg: %.1f, Min: %.1f, Max: %.1f) | Level: %d | Frame Time: %.2fms"),
           CurrentFrameData.CurrentFPS,
           CurrentFrameData.AverageFPS,
           CurrentFrameData.MinFPS,
           CurrentFrameData.MaxFPS,
           (int32)CurrentFrameData.PerformanceLevel,
           CurrentFrameData.FrameTime);
}