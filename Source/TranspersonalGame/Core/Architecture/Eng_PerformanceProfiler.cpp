#include "Eng_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UEng_PerformanceProfiler::UEng_PerformanceProfiler()
{
    bIsProfiling = false;
    bAutoOptimization = false;
    TargetFPS = 60.0f;
    TargetMemoryMB = 8192;
    CurrentPlatform = TEXT("PC");
}

void UEng_PerformanceProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Register core performance metrics
    RegisterMetric(TEXT("FPS"), 60.0f);
    RegisterMetric(TEXT("FrameTime"), 16.67f); // 60fps = 16.67ms
    RegisterMetric(TEXT("MemoryUsage"), 4096.0f); // 4GB target
    RegisterMetric(TEXT("DrawCalls"), 2000.0f);
    RegisterMetric(TEXT("Triangles"), 2000000.0f); // 2M triangles
    RegisterMetric(TEXT("CPUTime"), 12.0f); // 12ms for 60fps with headroom
    RegisterMetric(TEXT("GPUTime"), 14.0f); // 14ms for 60fps with headroom
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceProfiler initialized with %d metrics"), Metrics.Num());
}

void UEng_PerformanceProfiler::Deinitialize()
{
    StopProfiling();
    Super::Deinitialize();
}

void UEng_PerformanceProfiler::StartProfiling()
{
    if (bIsProfiling)
    {
        UE_LOG(LogTemp, Warning, TEXT("Profiling already active"));
        return;
    }
    
    bIsProfiling = true;
    ProfilingStartTime = FDateTime::Now();
    
    // Clear previous data
    for (FEng_PerformanceMetric& Metric : Metrics)
    {
        Metric.CurrentValue = 0.0f;
        Metric.AverageValue = 0.0f;
        Metric.MinValue = FLT_MAX;
        Metric.MaxValue = -FLT_MAX;
    }
    
    SystemProfiles.Empty();
    MetricHistory.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance profiling started"));
}

void UEng_PerformanceProfiler::StopProfiling()
{
    if (!bIsProfiling)
    {
        return;
    }
    
    bIsProfiling = false;
    FTimespan Duration = FDateTime::Now() - ProfilingStartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Performance profiling stopped after %f seconds"), Duration.GetTotalSeconds());
    
    // Generate final report
    FString Report = GetPerformanceReport();
    UE_LOG(LogTemp, Log, TEXT("Performance Report:\n%s"), *Report);
}

void UEng_PerformanceProfiler::RegisterMetric(const FString& MetricName, float TargetValue)
{
    // Check if metric already exists
    FEng_PerformanceMetric* ExistingMetric = Metrics.FindByPredicate([&MetricName](const FEng_PerformanceMetric& Metric)
    {
        return Metric.MetricName == MetricName;
    });
    
    if (ExistingMetric)
    {
        ExistingMetric->TargetValue = TargetValue;
        UE_LOG(LogTemp, Log, TEXT("Updated metric target: %s = %f"), *MetricName, TargetValue);
        return;
    }
    
    FEng_PerformanceMetric NewMetric;
    NewMetric.MetricName = MetricName;
    NewMetric.TargetValue = TargetValue;
    Metrics.Add(NewMetric);
    
    UE_LOG(LogTemp, Log, TEXT("Registered performance metric: %s (Target: %f)"), *MetricName, TargetValue);
}

void UEng_PerformanceProfiler::UpdateMetric(const FString& MetricName, float Value)
{
    if (!bIsProfiling)
    {
        return;
    }
    
    FEng_PerformanceMetric* Metric = Metrics.FindByPredicate([&MetricName](const FEng_PerformanceMetric& M)
    {
        return M.MetricName == MetricName;
    });
    
    if (Metric)
    {
        UpdateMetricStatistics(*Metric, Value);
        
        // Store in history
        if (!MetricHistory.Contains(MetricName))
        {
            MetricHistory.Add(MetricName, TArray<float>());
        }
        MetricHistory[MetricName].Add(Value);
        
        // Keep only last 100 samples
        if (MetricHistory[MetricName].Num() > 100)
        {
            MetricHistory[MetricName].RemoveAt(0);
        }
    }
}

FEng_PerformanceMetric UEng_PerformanceProfiler::GetMetric(const FString& MetricName)
{
    const FEng_PerformanceMetric* Metric = Metrics.FindByPredicate([&MetricName](const FEng_PerformanceMetric& M)
    {
        return M.MetricName == MetricName;
    });
    
    return Metric ? *Metric : FEng_PerformanceMetric();
}

TArray<FEng_PerformanceMetric> UEng_PerformanceProfiler::GetAllMetrics()
{
    return Metrics;
}

void UEng_PerformanceProfiler::ProfileSystem(const FString& SystemName)
{
    if (!bIsProfiling)
    {
        return;
    }
    
    FEng_SystemProfile* ExistingProfile = SystemProfiles.FindByPredicate([&SystemName](const FEng_SystemProfile& Profile)
    {
        return Profile.SystemName == SystemName;
    });
    
    if (!ExistingProfile)
    {
        FEng_SystemProfile NewProfile;
        NewProfile.SystemName = SystemName;
        NewProfile.bIsActive = true;
        SystemProfiles.Add(NewProfile);
        ExistingProfile = &SystemProfiles.Last();
    }
    
    // Simulate profiling data (in real implementation, this would gather actual stats)
    ExistingProfile->CPUTime = FMath::RandRange(0.5f, 5.0f);
    ExistingProfile->GPUTime = FMath::RandRange(0.2f, 3.0f);
    ExistingProfile->MemoryUsage = FMath::RandRange(50, 500);
    ExistingProfile->DrawCalls = FMath::RandRange(10, 200);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Profiled system %s: CPU=%.2fms, GPU=%.2fms"), 
           *SystemName, ExistingProfile->CPUTime, ExistingProfile->GPUTime);
}

FEng_SystemProfile UEng_PerformanceProfiler::GetSystemProfile(const FString& SystemName)
{
    const FEng_SystemProfile* Profile = SystemProfiles.FindByPredicate([&SystemName](const FEng_SystemProfile& P)
    {
        return P.SystemName == SystemName;
    });
    
    return Profile ? *Profile : FEng_SystemProfile();
}

TArray<FEng_SystemProfile> UEng_PerformanceProfiler::GetAllSystemProfiles()
{
    return SystemProfiles;
}

void UEng_PerformanceProfiler::SetPerformanceTarget(const FString& Platform, float TargetFPS, int32 TargetMemoryMB)
{
    CurrentPlatform = Platform;
    this->TargetFPS = TargetFPS;
    this->TargetMemoryMB = TargetMemoryMB;
    
    // Update FPS-related metrics
    RegisterMetric(TEXT("FPS"), TargetFPS);
    RegisterMetric(TEXT("FrameTime"), 1000.0f / TargetFPS);
    RegisterMetric(TEXT("MemoryUsage"), static_cast<float>(TargetMemoryMB));
    
    UE_LOG(LogTemp, Warning, TEXT("Performance targets set for %s: %f FPS, %d MB"), 
           *Platform, TargetFPS, TargetMemoryMB);
}

bool UEng_PerformanceProfiler::IsPerformanceWithinTargets()
{
    for (const FEng_PerformanceMetric& Metric : Metrics)
    {
        if (!Metric.bIsWithinTarget)
        {
            return false;
        }
    }
    return true;
}

FString UEng_PerformanceProfiler::GetPerformanceReport()
{
    FString Report = TEXT("=== PERFORMANCE REPORT ===\n");
    Report += FString::Printf(TEXT("Platform: %s | Target: %.1f FPS, %d MB\n\n"), 
                              *CurrentPlatform, TargetFPS, TargetMemoryMB);
    
    Report += TEXT("METRICS:\n");
    for (const FEng_PerformanceMetric& Metric : Metrics)
    {
        FString Status = Metric.bIsWithinTarget ? TEXT("OK") : TEXT("WARN");
        Report += FString::Printf(TEXT("  %s: %.2f (Avg: %.2f, Target: %.2f) [%s]\n"), 
                                  *Metric.MetricName, Metric.CurrentValue, Metric.AverageValue, 
                                  Metric.TargetValue, *Status);
    }
    
    Report += TEXT("\nSYSTEM PROFILES:\n");
    for (const FEng_SystemProfile& Profile : SystemProfiles)
    {
        if (Profile.bIsActive)
        {
            Report += FString::Printf(TEXT("  %s: CPU=%.2fms, GPU=%.2fms, Mem=%dMB, Draws=%d\n"), 
                                      *Profile.SystemName, Profile.CPUTime, Profile.GPUTime, 
                                      Profile.MemoryUsage, Profile.DrawCalls);
        }
    }
    
    bool bWithinTargets = IsPerformanceWithinTargets();
    Report += FString::Printf(TEXT("\nOVERALL STATUS: %s\n"), 
                              bWithinTargets ? TEXT("OPTIMAL") : TEXT("NEEDS OPTIMIZATION"));
    
    return Report;
}

void UEng_PerformanceProfiler::EnableAutoOptimization(bool bEnable)
{
    bAutoOptimization = bEnable;
    UE_LOG(LogTemp, Warning, TEXT("Auto-optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UEng_PerformanceProfiler::TriggerGarbageCollection()
{
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
        UE_LOG(LogTemp, Log, TEXT("Forced garbage collection"));
    }
}

void UEng_PerformanceProfiler::UpdateMetricStatistics(FEng_PerformanceMetric& Metric, float NewValue)
{
    Metric.CurrentValue = NewValue;
    
    // Update min/max
    if (NewValue < Metric.MinValue)
    {
        Metric.MinValue = NewValue;
    }
    if (NewValue > Metric.MaxValue)
    {
        Metric.MaxValue = NewValue;
    }
    
    // Update average (simple moving average)
    if (MetricHistory.Contains(Metric.MetricName))
    {
        const TArray<float>& History = MetricHistory[Metric.MetricName];
        if (History.Num() > 0)
        {
            float Sum = 0.0f;
            for (float Value : History)
            {
                Sum += Value;
            }
            Metric.AverageValue = Sum / History.Num();
        }
    }
    
    // Check if within target (with 10% tolerance)
    float Tolerance = Metric.TargetValue * 0.1f;
    Metric.bIsWithinTarget = FMath::Abs(NewValue - Metric.TargetValue) <= Tolerance;
    
    // Auto-optimization check
    if (bAutoOptimization && !Metric.bIsWithinTarget)
    {
        CheckPerformanceThresholds();
    }
}

void UEng_PerformanceProfiler::CheckPerformanceThresholds()
{
    // Check critical metrics
    const FEng_PerformanceMetric* FPSMetric = Metrics.FindByPredicate([](const FEng_PerformanceMetric& M)
    {
        return M.MetricName == TEXT("FPS");
    });
    
    if (FPSMetric && FPSMetric->CurrentValue < (TargetFPS * 0.8f))
    {
        UE_LOG(LogTemp, Warning, TEXT("FPS below threshold: %.1f < %.1f"), 
               FPSMetric->CurrentValue, TargetFPS * 0.8f);
        OptimizeIfNeeded();
    }
}

void UEng_PerformanceProfiler::OptimizeIfNeeded()
{
    if (!bAutoOptimization)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Auto-optimization triggered"));
    
    // Simple optimization strategies
    // 1. Trigger garbage collection
    TriggerGarbageCollection();
    
    // 2. Log optimization suggestion
    UE_LOG(LogTemp, Warning, TEXT("Consider reducing LOD levels, culling distance, or particle counts"));
}