#include "Perf_PerformanceProfiler.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/StatsHierarchical.h"
#include "RenderingThread.h"
#include "RHI.h"

// Console variables
TAutoConsoleVariable<int32> UPerf_PerformanceProfiler::CVarProfilerEnabled(
    TEXT("tp.profiler.enabled"),
    1,
    TEXT("Enable/disable performance profiler (0=disabled, 1=enabled)"),
    ECVF_Default
);

TAutoConsoleVariable<int32> UPerf_PerformanceProfiler::CVarShowStats(
    TEXT("tp.profiler.showstats"),
    0,
    TEXT("Show performance stats on screen (0=hidden, 1=visible)"),
    ECVF_Default
);

TAutoConsoleVariable<float> UPerf_PerformanceProfiler::CVarTargetFrameRate(
    TEXT("tp.profiler.targetfps"),
    60.0f,
    TEXT("Target frame rate for performance monitoring"),
    ECVF_Default
);

UPerf_PerformanceProfiler::UPerf_PerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    // Initialize defaults
    ProfilerMode = EPerf_ProfilerMode::Basic;
    bIsProfilingActive = false;
    bShowOnScreenDisplay = false;
    UpdateInterval = 0.5f;
    LastUpdateTime = 0.0f;
    
    // Performance thresholds
    TargetFrameRate = 60.0f;
    CriticalFrameRate = 20.0f;
    MaxGPUTime = 16.67f; // ~60fps
    MaxMemoryUsage = 4096.0f; // 4GB
    
    // Frame tracking
    MinFrameRate = FLT_MAX;
    MaxFrameRate = 0.0f;
    FrameRateSum = 0.0f;
    FrameCount = 0;
}

void UPerf_PerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-start profiling if enabled
    if (CVarProfilerEnabled.GetValueOnGameThread() > 0)
    {
        StartProfiling();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UPerf_PerformanceProfiler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopProfiling();
    Super::EndPlay(EndPlayReason);
}

void UPerf_PerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsProfilingActive || ProfilerMode == EPerf_ProfilerMode::Disabled)
    {
        return;
    }
    
    // Update console variables
    UpdateConsoleVariables();
    
    // Update metrics based on interval
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateInterval)
    {
        UpdateFrameMetrics();
        UpdateSystemMetrics();
        CheckPerformanceThresholds();
        
        if (bShowOnScreenDisplay || CVarShowStats.GetValueOnGameThread() > 0)
        {
            DisplayOnScreenStats();
        }
        
        LastUpdateTime = CurrentTime;
    }
}

void UPerf_PerformanceProfiler::StartProfiling()
{
    bIsProfilingActive = true;
    ResetMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started - Mode: %d"), 
           static_cast<int32>(ProfilerMode));
}

void UPerf_PerformanceProfiler::StopProfiling()
{
    bIsProfilingActive = false;
    
    if (FrameCount > 0)
    {
        LogPerformanceReport();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
}

void UPerf_PerformanceProfiler::ResetMetrics()
{
    CurrentFrameMetrics = FPerf_FrameMetrics();
    CurrentSystemMetrics = FPerf_SystemMetrics();
    
    MinFrameRate = FLT_MAX;
    MaxFrameRate = 0.0f;
    FrameRateSum = 0.0f;
    FrameCount = 0;
    
    PerformanceWarnings.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Performance metrics reset"));
}

void UPerf_PerformanceProfiler::SetProfilerMode(EPerf_ProfilerMode NewMode)
{
    ProfilerMode = NewMode;
    
    if (NewMode == EPerf_ProfilerMode::Disabled)
    {
        StopProfiling();
    }
    else if (!bIsProfilingActive)
    {
        StartProfiling();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Profiler mode changed to: %d"), static_cast<int32>(NewMode));
}

float UPerf_PerformanceProfiler::GetAverageFrameRate() const
{
    return FrameCount > 0 ? FrameRateSum / FrameCount : 0.0f;
}

bool UPerf_PerformanceProfiler::IsPerformanceCritical() const
{
    if (!bIsProfilingActive)
    {
        return false;
    }
    
    float CurrentFPS = 1.0f / CurrentFrameMetrics.FrameTime;
    return CurrentFPS < CriticalFrameRate || 
           CurrentFrameMetrics.GPUTime > MaxGPUTime ||
           CurrentFrameMetrics.MemoryUsage > MaxMemoryUsage;
}

TArray<FString> UPerf_PerformanceProfiler::GetPerformanceWarnings() const
{
    return PerformanceWarnings;
}

void UPerf_PerformanceProfiler::EnableOnScreenDisplay(bool bEnable)
{
    bShowOnScreenDisplay = bEnable;
    UE_LOG(LogTemp, Log, TEXT("On-screen display %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_PerformanceProfiler::LogPerformanceReport()
{
    if (FrameCount == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No performance data to report"));
        return;
    }
    
    float AvgFPS = GetAverageFrameRate();
    
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Frames Analyzed: %d"), FrameCount);
    UE_LOG(LogTemp, Log, TEXT("Average FPS: %.2f"), AvgFPS);
    UE_LOG(LogTemp, Log, TEXT("Min FPS: %.2f"), MinFrameRate);
    UE_LOG(LogTemp, Log, TEXT("Max FPS: %.2f"), MaxFrameRate);
    UE_LOG(LogTemp, Log, TEXT("Current Frame Time: %.2f ms"), CurrentFrameMetrics.FrameTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("Current GPU Time: %.2f ms"), CurrentFrameMetrics.GPUTime);
    UE_LOG(LogTemp, Log, TEXT("Current Memory: %.2f MB"), CurrentFrameMetrics.MemoryUsage);
    UE_LOG(LogTemp, Log, TEXT("Active Dinosaurs: %d"), CurrentSystemMetrics.ActiveDinosaurs);
    UE_LOG(LogTemp, Log, TEXT("Visible Actors: %d"), CurrentSystemMetrics.VisibleActors);
    UE_LOG(LogTemp, Log, TEXT("Physics Bodies: %d"), CurrentSystemMetrics.PhysicsBodies);
    
    if (PerformanceWarnings.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warnings:"));
        for (const FString& Warning : PerformanceWarnings)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Warning);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
}

void UPerf_PerformanceProfiler::UpdateFrameMetrics()
{
    SCOPE_CYCLE_COUNTER(STAT_SurvivalPhysicsUpdate);
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Frame time
    float DeltaTime = World->GetDeltaSeconds();
    CurrentFrameMetrics.FrameTime = DeltaTime;
    
    // Frame rate tracking
    if (DeltaTime > 0.0f)
    {
        float CurrentFPS = 1.0f / DeltaTime;
        FrameRateSum += CurrentFPS;
        FrameCount++;
        
        MinFrameRate = FMath::Min(MinFrameRate, CurrentFPS);
        MaxFrameRate = FMath::Max(MaxFrameRate, CurrentFPS);
    }
    
    // GPU time (approximation using render thread time)
    CurrentFrameMetrics.GPUTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    
    // CPU time
    CurrentFrameMetrics.CPUTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
    
    // Memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentFrameMetrics.MemoryUsage = MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
    
    // Rendering stats (basic approximation)
    CurrentFrameMetrics.DrawCalls = GNumDrawCallsRHI[GRHIThreadId];
    CurrentFrameMetrics.Triangles = GNumPrimitivesDrawnRHI[GRHIThreadId];
    
    // Update stats
    SET_FLOAT_STAT(STAT_FrameTime, CurrentFrameMetrics.FrameTime * 1000.0f);
    SET_FLOAT_STAT(STAT_GPUTime, CurrentFrameMetrics.GPUTime);
    SET_FLOAT_STAT(STAT_MemoryUsage, CurrentFrameMetrics.MemoryUsage);
}

void UPerf_PerformanceProfiler::UpdateSystemMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count actors by type
    int32 DinosaurCount = 0;
    int32 VisibleCount = 0;
    int32 PhysicsCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        // Count dinosaurs (actors with "dino" in their name)
        FString ActorName = Actor->GetName().ToLower();
        if (ActorName.Contains(TEXT("dino")) || ActorName.Contains(TEXT("trex")) || 
            ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachi")))
        {
            DinosaurCount++;
        }
        
        // Count visible actors
        if (Actor->GetRootComponent() && Actor->GetRootComponent()->IsVisible())
        {
            VisibleCount++;
        }
        
        // Count physics bodies
        if (Actor->GetRootComponent() && Actor->GetRootComponent()->IsSimulatingPhysics())
        {
            PhysicsCount++;
        }
    }
    
    CurrentSystemMetrics.ActiveDinosaurs = DinosaurCount;
    CurrentSystemMetrics.VisibleActors = VisibleCount;
    CurrentSystemMetrics.PhysicsBodies = PhysicsCount;
    
    // System timing (using cycle stats)
    CurrentSystemMetrics.SurvivalPhysicsTime = FStatsUtils::CyclesToMS(GET_STATFNAME(STAT_SurvivalPhysicsUpdate));
    CurrentSystemMetrics.DinosaurAITime = FStatsUtils::CyclesToMS(GET_STATFNAME(STAT_DinosaurAITick));
    CurrentSystemMetrics.WorldGenTime = FStatsUtils::CyclesToMS(GET_STATFNAME(STAT_WorldGeneration));
    CurrentSystemMetrics.FoliageRenderTime = FStatsUtils::CyclesToMS(GET_STATFNAME(STAT_FoliageRendering));
    CurrentSystemMetrics.CrowdSimTime = FStatsUtils::CyclesToMS(GET_STATFNAME(STAT_CrowdSimulation));
    
    // Update stats
    SET_DWORD_STAT(STAT_ActiveDinosaurs, DinosaurCount);
    SET_DWORD_STAT(STAT_VisibleActors, VisibleCount);
    SET_DWORD_STAT(STAT_PhysicsBodies, PhysicsCount);
}

void UPerf_PerformanceProfiler::CheckPerformanceThresholds()
{
    PerformanceWarnings.Empty();
    
    float CurrentFPS = CurrentFrameMetrics.FrameTime > 0.0f ? 1.0f / CurrentFrameMetrics.FrameTime : 0.0f;
    
    // Frame rate warnings
    if (CurrentFPS < CriticalFrameRate)
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("Critical FPS: %.1f (target: %.1f)"), 
                                               CurrentFPS, TargetFrameRate));
    }
    else if (CurrentFPS < TargetFrameRate)
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("Low FPS: %.1f (target: %.1f)"), 
                                               CurrentFPS, TargetFrameRate));
    }
    
    // GPU time warnings
    if (CurrentFrameMetrics.GPUTime > MaxGPUTime)
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("High GPU time: %.2f ms (max: %.2f ms)"), 
                                               CurrentFrameMetrics.GPUTime, MaxGPUTime));
    }
    
    // Memory warnings
    if (CurrentFrameMetrics.MemoryUsage > MaxMemoryUsage)
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("High memory usage: %.1f MB (max: %.1f MB)"), 
                                               CurrentFrameMetrics.MemoryUsage, MaxMemoryUsage));
    }
    
    // System-specific warnings
    if (CurrentSystemMetrics.ActiveDinosaurs > 50)
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("High dinosaur count: %d"), 
                                               CurrentSystemMetrics.ActiveDinosaurs));
    }
    
    if (CurrentSystemMetrics.VisibleActors > 1000)
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("High visible actor count: %d"), 
                                               CurrentSystemMetrics.VisibleActors));
    }
}

void UPerf_PerformanceProfiler::DisplayOnScreenStats()
{
    if (!GEngine)
    {
        return;
    }
    
    float CurrentFPS = CurrentFrameMetrics.FrameTime > 0.0f ? 1.0f / CurrentFrameMetrics.FrameTime : 0.0f;
    
    // Basic stats
    GEngine->AddOnScreenDebugMessage(-1, UpdateInterval + 0.1f, FColor::White,
        FString::Printf(TEXT("FPS: %.1f | Frame: %.2f ms | GPU: %.2f ms"), 
                       CurrentFPS, CurrentFrameMetrics.FrameTime * 1000.0f, CurrentFrameMetrics.GPUTime));
    
    GEngine->AddOnScreenDebugMessage(-1, UpdateInterval + 0.1f, FColor::White,
        FString::Printf(TEXT("Memory: %.1f MB | Actors: %d | Dinosaurs: %d"), 
                       CurrentFrameMetrics.MemoryUsage, CurrentSystemMetrics.VisibleActors, 
                       CurrentSystemMetrics.ActiveDinosaurs));
    
    // Performance warnings
    if (PerformanceWarnings.Num() > 0)
    {
        for (int32 i = 0; i < FMath::Min(3, PerformanceWarnings.Num()); i++)
        {
            GEngine->AddOnScreenDebugMessage(-1, UpdateInterval + 0.1f, FColor::Red,
                FString::Printf(TEXT("WARNING: %s"), *PerformanceWarnings[i]));
        }
    }
}

void UPerf_PerformanceProfiler::UpdateConsoleVariables()
{
    // Update target frame rate from console variable
    float CVarTargetFPS = CVarTargetFrameRate.GetValueOnGameThread();
    if (CVarTargetFPS != TargetFrameRate && CVarTargetFPS > 0.0f)
    {
        TargetFrameRate = CVarTargetFPS;
    }
    
    // Update profiler state from console variable
    bool bCVarEnabled = CVarProfilerEnabled.GetValueOnGameThread() > 0;
    if (bCVarEnabled != bIsProfilingActive)
    {
        if (bCVarEnabled)
        {
            StartProfiling();
        }
        else
        {
            StopProfiling();
        }
    }
}