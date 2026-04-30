#include "Perf_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "RHI.h"

UPerf_PerformanceProfiler::UPerf_PerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    bIsProfilingActive = true;
    ProfilingInterval = 0.1f;
    MetricsHistorySize = 300; // 30 seconds at 10Hz
    LastProfilingTime = 0.0f;
    FrameCount = 0;
    
    // Initialize performance budget with default values
    PerformanceBudget = FPerf_PerformanceBudget();
}

void UPerf_PerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Started monitoring"));
    
    // Register with subsystem
    if (UPerf_PerformanceProfilerSubsystem* Subsystem = UPerf_PerformanceProfilerSubsystem::Get(this))
    {
        Subsystem->RegisterProfiler(this);
    }
    
    StartProfiling();
}

void UPerf_PerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsProfilingActive)
    {
        return;
    }
    
    LastProfilingTime += DeltaTime;
    
    if (LastProfilingTime >= ProfilingInterval)
    {
        UpdateMetrics();
        AnalyzePerformance();
        LastProfilingTime = 0.0f;
    }
}

void UPerf_PerformanceProfiler::StartProfiling()
{
    bIsProfilingActive = true;
    ResetMetrics();
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Profiling started"));
}

void UPerf_PerformanceProfiler::StopProfiling()
{
    bIsProfilingActive = false;
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Profiling stopped"));
}

void UPerf_PerformanceProfiler::ResetMetrics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    FPSHistory.Empty();
    FrameTimeHistory.Empty();
    FrameCount = 0;
    CachedWarnings.Empty();
    CachedRecommendations.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Metrics reset"));
}

FPerf_PerformanceMetrics UPerf_PerformanceProfiler::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

EPerf_PerformanceLevel UPerf_PerformanceProfiler::GetPerformanceLevel() const
{
    return DeterminePerformanceLevel();
}

TArray<FString> UPerf_PerformanceProfiler::GetPerformanceWarnings() const
{
    GenerateWarnings();
    return CachedWarnings;
}

void UPerf_PerformanceProfiler::SetPerformanceBudget(const FPerf_PerformanceBudget& NewBudget)
{
    PerformanceBudget = NewBudget;
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Budget updated - Target FPS: %.1f"), NewBudget.TargetFPS);
}

TArray<FString> UPerf_PerformanceProfiler::GetOptimizationRecommendations() const
{
    CachedRecommendations.Empty();
    
    // Analyze current metrics and provide recommendations
    if (CurrentMetrics.CurrentFPS < PerformanceBudget.TargetFPS * 0.8f)
    {
        CachedRecommendations.Add(TEXT("Consider reducing draw calls - Current: ") + FString::FromInt(CurrentMetrics.DrawCalls));
        CachedRecommendations.Add(TEXT("Enable LOD system for distant objects"));
        CachedRecommendations.Add(TEXT("Consider using instanced static meshes for repeated objects"));
    }
    
    if (CurrentMetrics.MemoryUsageMB > PerformanceBudget.MaxMemoryUsageMB * 0.8f)
    {
        CachedRecommendations.Add(TEXT("Memory usage high - Consider texture streaming"));
        CachedRecommendations.Add(TEXT("Reduce texture resolution for distant objects"));
    }
    
    if (CurrentMetrics.Triangles > PerformanceBudget.MaxTriangles * 0.8f)
    {
        CachedRecommendations.Add(TEXT("Triangle count high - Enable automatic LOD generation"));
        CachedRecommendations.Add(TEXT("Use simplified collision meshes"));
    }
    
    if (CurrentMetrics.GameThreadTime > PerformanceBudget.MaxGameThreadTime * 0.8f)
    {
        CachedRecommendations.Add(TEXT("Game thread bottleneck - Optimize Blueprint execution"));
        CachedRecommendations.Add(TEXT("Consider moving heavy calculations to worker threads"));
    }
    
    if (CurrentMetrics.RenderThreadTime > PerformanceBudget.MaxRenderThreadTime * 0.8f)
    {
        CachedRecommendations.Add(TEXT("Render thread bottleneck - Reduce draw calls"));
        CachedRecommendations.Add(TEXT("Enable GPU instancing for similar objects"));
    }
    
    return CachedRecommendations;
}

void UPerf_PerformanceProfiler::ApplyAutoOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Applying automatic optimizations"));
    
    // Apply basic optimizations based on current performance
    if (CurrentMetrics.CurrentFPS < PerformanceBudget.TargetFPS * 0.7f)
    {
        // Emergency optimizations
        if (GEngine)
        {
            GEngine->Exec(GetWorld(), TEXT("r.ScreenPercentage 75"));
            GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.8"));
            UE_LOG(LogTemp, Warning, TEXT("Emergency optimizations applied: Reduced screen percentage and view distance"));
        }
    }
    else if (CurrentMetrics.CurrentFPS < PerformanceBudget.TargetFPS * 0.9f)
    {
        // Moderate optimizations
        if (GEngine)
        {
            GEngine->Exec(GetWorld(), TEXT("r.ScreenPercentage 90"));
            GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 2"));
            UE_LOG(LogTemp, Log, TEXT("Moderate optimizations applied: Reduced screen percentage and shadow quality"));
        }
    }
}

void UPerf_PerformanceProfiler::UpdateMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get current FPS
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    }
    
    UpdateFPSHistory(CurrentMetrics.CurrentFPS);
    CurrentMetrics.AverageFPS = CalculateAverageFPS();
    
    // Update min/max FPS
    if (FrameCount == 0 || CurrentMetrics.CurrentFPS < CurrentMetrics.MinFPS)
    {
        CurrentMetrics.MinFPS = CurrentMetrics.CurrentFPS;
    }
    if (FrameCount == 0 || CurrentMetrics.CurrentFPS > CurrentMetrics.MaxFPS)
    {
        CurrentMetrics.MaxFPS = CurrentMetrics.CurrentFPS;
    }
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Get actor counts
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    CurrentMetrics.ActiveActors = AllActors.Num();
    
    // Estimate visible actors (simplified)
    CurrentMetrics.VisibleActors = FMath::Max(1, CurrentMetrics.ActiveActors / 2);
    
    // Estimate draw calls and triangles (simplified for now)
    CurrentMetrics.DrawCalls = FMath::Max(100, CurrentMetrics.VisibleActors * 2);
    CurrentMetrics.Triangles = FMath::Max(10000, CurrentMetrics.VisibleActors * 500);
    
    // Thread times (simplified estimates)
    CurrentMetrics.GameThreadTime = CurrentMetrics.FrameTime * 0.6f;
    CurrentMetrics.RenderThreadTime = CurrentMetrics.FrameTime * 0.7f;
    CurrentMetrics.GPUTime = CurrentMetrics.FrameTime * 0.8f;
    
    FrameCount++;
    
    // Log performance data periodically
    if (FrameCount % 100 == 0) // Every 10 seconds at 10Hz
    {
        LogPerformanceData();
    }
}

void UPerf_PerformanceProfiler::UpdateFPSHistory(float NewFPS)
{
    FPSHistory.Add(NewFPS);
    
    if (FPSHistory.Num() > MetricsHistorySize)
    {
        FPSHistory.RemoveAt(0);
    }
}

void UPerf_PerformanceProfiler::AnalyzePerformance()
{
    EPerf_PerformanceLevel Level = DeterminePerformanceLevel();
    
    // Log warnings for critical performance issues
    if (Level == EPerf_PerformanceLevel::Critical || Level == EPerf_PerformanceLevel::Unplayable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Profiler: Critical performance detected - FPS: %.1f, Target: %.1f"), 
               CurrentMetrics.CurrentFPS, PerformanceBudget.TargetFPS);
    }
}

float UPerf_PerformanceProfiler::CalculateAverageFPS() const
{
    if (FPSHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Sum = 0.0f;
    for (float FPS : FPSHistory)
    {
        Sum += FPS;
    }
    
    return Sum / FPSHistory.Num();
}

EPerf_PerformanceLevel UPerf_PerformanceProfiler::DeterminePerformanceLevel() const
{
    float TargetFPS = PerformanceBudget.TargetFPS;
    float CurrentFPS = CurrentMetrics.CurrentFPS;
    
    if (CurrentFPS >= TargetFPS * 0.95f)
    {
        return EPerf_PerformanceLevel::Optimal;
    }
    else if (CurrentFPS >= TargetFPS * 0.8f)
    {
        return EPerf_PerformanceLevel::Good;
    }
    else if (CurrentFPS >= TargetFPS * 0.6f)
    {
        return EPerf_PerformanceLevel::Warning;
    }
    else if (CurrentFPS >= TargetFPS * 0.3f)
    {
        return EPerf_PerformanceLevel::Critical;
    }
    else
    {
        return EPerf_PerformanceLevel::Unplayable;
    }
}

void UPerf_PerformanceProfiler::GenerateWarnings() const
{
    CachedWarnings.Empty();
    
    if (CurrentMetrics.CurrentFPS < PerformanceBudget.TargetFPS * 0.8f)
    {
        CachedWarnings.Add(FString::Printf(TEXT("Low FPS: %.1f (Target: %.1f)"), 
                                          CurrentMetrics.CurrentFPS, PerformanceBudget.TargetFPS));
    }
    
    if (CurrentMetrics.FrameTime > PerformanceBudget.MaxFrameTime)
    {
        CachedWarnings.Add(FString::Printf(TEXT("High frame time: %.2fms (Max: %.2fms)"), 
                                          CurrentMetrics.FrameTime, PerformanceBudget.MaxFrameTime));
    }
    
    if (CurrentMetrics.MemoryUsageMB > PerformanceBudget.MaxMemoryUsageMB * 0.9f)
    {
        CachedWarnings.Add(FString::Printf(TEXT("High memory usage: %.1fMB (Max: %.1fMB)"), 
                                          CurrentMetrics.MemoryUsageMB, PerformanceBudget.MaxMemoryUsageMB));
    }
    
    if (CurrentMetrics.DrawCalls > PerformanceBudget.MaxDrawCalls)
    {
        CachedWarnings.Add(FString::Printf(TEXT("High draw calls: %d (Max: %d)"), 
                                          CurrentMetrics.DrawCalls, PerformanceBudget.MaxDrawCalls));
    }
    
    if (CurrentMetrics.Triangles > PerformanceBudget.MaxTriangles)
    {
        CachedWarnings.Add(FString::Printf(TEXT("High triangle count: %d (Max: %d)"), 
                                          CurrentMetrics.Triangles, PerformanceBudget.MaxTriangles));
    }
}

void UPerf_PerformanceProfiler::LogPerformanceData() const
{
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler Stats - FPS: %.1f (Avg: %.1f), Frame: %.2fms, Memory: %.1fMB, Actors: %d"), 
           CurrentMetrics.CurrentFPS, 
           CurrentMetrics.AverageFPS,
           CurrentMetrics.FrameTime,
           CurrentMetrics.MemoryUsageMB,
           CurrentMetrics.ActiveActors);
}

// Subsystem Implementation
void UPerf_PerformanceProfilerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bGlobalProfilingEnabled = true;
    GlobalBudget = FPerf_PerformanceBudget();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler Subsystem: Initialized"));
}

void UPerf_PerformanceProfilerSubsystem::Deinitialize()
{
    RegisteredProfilers.Empty();
    Super::Deinitialize();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler Subsystem: Deinitialized"));
}

UPerf_PerformanceProfilerSubsystem* UPerf_PerformanceProfilerSubsystem::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UPerf_PerformanceProfilerSubsystem>();
        }
    }
    return nullptr;
}

void UPerf_PerformanceProfilerSubsystem::RegisterProfiler(UPerf_PerformanceProfiler* Profiler)
{
    if (Profiler && !RegisteredProfilers.Contains(Profiler))
    {
        RegisteredProfilers.Add(Profiler);
        UE_LOG(LogTemp, Log, TEXT("Performance Profiler Subsystem: Registered profiler"));
    }
}

void UPerf_PerformanceProfilerSubsystem::UnregisterProfiler(UPerf_PerformanceProfiler* Profiler)
{
    if (Profiler)
    {
        RegisteredProfilers.Remove(Profiler);
        UE_LOG(LogTemp, Log, TEXT("Performance Profiler Subsystem: Unregistered profiler"));
    }
}

FPerf_PerformanceMetrics UPerf_PerformanceProfilerSubsystem::GetGlobalMetrics() const
{
    FPerf_PerformanceMetrics GlobalMetrics;
    
    if (RegisteredProfilers.Num() > 0)
    {
        // Average metrics from all registered profilers
        float TotalFPS = 0.0f;
        float TotalFrameTime = 0.0f;
        float TotalMemory = 0.0f;
        int32 TotalActors = 0;
        
        for (const UPerf_PerformanceProfiler* Profiler : RegisteredProfilers)
        {
            if (Profiler)
            {
                FPerf_PerformanceMetrics Metrics = Profiler->GetCurrentMetrics();
                TotalFPS += Metrics.CurrentFPS;
                TotalFrameTime += Metrics.FrameTime;
                TotalMemory += Metrics.MemoryUsageMB;
                TotalActors += Metrics.ActiveActors;
            }
        }
        
        int32 ValidProfilers = RegisteredProfilers.Num();
        GlobalMetrics.CurrentFPS = TotalFPS / ValidProfilers;
        GlobalMetrics.FrameTime = TotalFrameTime / ValidProfilers;
        GlobalMetrics.MemoryUsageMB = TotalMemory / ValidProfilers;
        GlobalMetrics.ActiveActors = TotalActors;
    }
    
    return GlobalMetrics;
}

void UPerf_PerformanceProfilerSubsystem::SetGlobalPerformanceBudget(const FPerf_PerformanceBudget& Budget)
{
    GlobalBudget = Budget;
    
    // Apply to all registered profilers
    for (UPerf_PerformanceProfiler* Profiler : RegisteredProfilers)
    {
        if (Profiler)
        {
            Profiler->SetPerformanceBudget(Budget);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler Subsystem: Global budget updated"));
}

void UPerf_PerformanceProfilerSubsystem::EnableGlobalProfiling(bool bEnable)
{
    bGlobalProfilingEnabled = bEnable;
    
    // Apply to all registered profilers
    for (UPerf_PerformanceProfiler* Profiler : RegisteredProfilers)
    {
        if (Profiler)
        {
            if (bEnable)
            {
                Profiler->StartProfiling();
            }
            else
            {
                Profiler->StopProfiling();
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler Subsystem: Global profiling %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}