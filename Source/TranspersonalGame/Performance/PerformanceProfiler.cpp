#include "PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/ProfilingHelpers.h"
#include "RHI.h"
#include "RenderCore.h"

DEFINE_LOG_CATEGORY_STATIC(LogPerformanceProfiler, Log, All);

UPerformanceProfiler::UPerformanceProfiler()
    : CurrentTarget(EPerformanceTarget::PC_HighEnd)
    , bIsProfilingActive(false)
    , CachedFrameTime(0.0f)
    , CachedGameThreadTime(0.0f)
    , CachedRenderThreadTime(0.0f)
    , CachedGPUTime(0.0f)
    , LastUpdateTime(0.0)
    , FrameCounter(0)
{
    CurrentBudget = FPerformanceBudget::GetBudgetForTarget(CurrentTarget);
    MetricsHistory.Reserve(MAX_HISTORY_SAMPLES);
}

void UPerformanceProfiler::Initialize(EPerformanceTarget Target)
{
    CurrentTarget = Target;
    CurrentBudget = FPerformanceBudget::GetBudgetForTarget(Target);
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Performance Profiler initialized for target: %d"), (int32)Target);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Frame budget: %.2fms"), CurrentBudget.TotalFrameTime);
    
    // Set up automatic profiling
    bIsProfilingActive = true;
    LastUpdateTime = FPlatformTime::Seconds();
}

void UPerformanceProfiler::UpdateMetrics()
{
    if (!bIsProfilingActive)
    {
        return;
    }

    double CurrentTime = FPlatformTime::Seconds();
    double DeltaTime = CurrentTime - LastUpdateTime;
    
    // Update at 10Hz to avoid overhead
    if (DeltaTime < 0.1)
    {
        return;
    }
    
    LastUpdateTime = CurrentTime;
    FrameCounter++;

    // Collect all performance metrics
    CollectFrameTimingMetrics();
    CollectMemoryMetrics();
    CollectRenderingMetrics();
    CollectMassAIMetrics();
    CollectStreamingMetrics();

    // Store in history
    MetricsHistory.Add(CurrentMetrics);
    if (MetricsHistory.Num() > MAX_HISTORY_SAMPLES)
    {
        MetricsHistory.RemoveAt(0);
    }

    // Check performance thresholds
    CheckPerformanceThresholds();
}

void UPerformanceProfiler::CollectFrameTimingMetrics()
{
    // Get frame timing from stats system
    if (FStats::IsThreadingReady())
    {
        // Frame time
        CurrentMetrics.FrameTime = FPlatformTime::ToMilliseconds(GFrameTime);
        
        // Game thread time
        const FStatMessage* GameThreadStat = FStats::GetStatData(GET_STATID(STAT_FrameTime), false);
        if (GameThreadStat)
        {
            CurrentMetrics.GameThreadTime = FPlatformTime::ToMilliseconds64(GameThreadStat->GetValue_Duration());
        }
        
        // Render thread time
        const FStatMessage* RenderThreadStat = FStats::GetStatData(GET_STATID(STAT_RenderingIdleTime), false);
        if (RenderThreadStat)
        {
            CurrentMetrics.RenderThreadTime = FPlatformTime::ToMilliseconds64(RenderThreadStat->GetValue_Duration());
        }
        
        // GPU time (approximation)
        CurrentMetrics.GPUTime = CurrentMetrics.FrameTime * 0.8f; // Rough estimate
    }
    
    // Cache values for quick access
    CachedFrameTime = CurrentMetrics.FrameTime;
    CachedGameThreadTime = CurrentMetrics.GameThreadTime;
    CachedRenderThreadTime = CurrentMetrics.RenderThreadTime;
    CachedGPUTime = CurrentMetrics.GPUTime;
}

void UPerformanceProfiler::CollectMemoryMetrics()
{
    // Get memory stats from platform
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    
    CurrentMetrics.TotalMemoryUsed = MemoryStats.UsedPhysical / (1024 * 1024); // Convert to MB
    
    // Texture memory (approximation)
    CurrentMetrics.TextureMemoryUsed = CurrentMetrics.TotalMemoryUsed * 0.4f; // Rough estimate
    
    // Mesh memory (approximation)
    CurrentMetrics.MeshMemoryUsed = CurrentMetrics.TotalMemoryUsed * 0.2f; // Rough estimate
    
    // Audio memory (approximation)
    CurrentMetrics.AudioMemoryUsed = CurrentMetrics.TotalMemoryUsed * 0.05f; // Rough estimate
}

void UPerformanceProfiler::CollectRenderingMetrics()
{
    // Get rendering stats
    if (FStats::IsThreadingReady())
    {
        // Draw calls
        const FStatMessage* DrawCallStat = FStats::GetStatData(GET_STATID(STAT_SceneDrawCalls), false);
        if (DrawCallStat)
        {
            CurrentMetrics.DrawCalls = DrawCallStat->GetValue_int64();
        }
        
        // Triangles
        const FStatMessage* TriangleStat = FStats::GetStatData(GET_STATID(STAT_SceneTriangles), false);
        if (TriangleStat)
        {
            CurrentMetrics.Triangles = TriangleStat->GetValue_int64();
        }
        
        // Visible primitives
        const FStatMessage* VisiblePrimStat = FStats::GetStatData(GET_STATID(STAT_VisibleStaticMeshElements), false);
        if (VisiblePrimStat)
        {
            CurrentMetrics.VisiblePrimitives = VisiblePrimStat->GetValue_int64();
        }
        
        // Culled primitives
        const FStatMessage* CulledPrimStat = FStats::GetStatData(GET_STATID(STAT_FrustumCulledPrimitives), false);
        if (CulledPrimStat)
        {
            CurrentMetrics.CulledPrimitives = CulledPrimStat->GetValue_int64();
        }
    }
}

void UPerformanceProfiler::CollectMassAIMetrics()
{
    // Mass AI metrics - these would be populated by the Mass AI system
    // For now, using placeholder values that would be set by the actual Mass AI implementation
    
    CurrentMetrics.ActiveMassAgents = 0; // To be set by Mass AI system
    CurrentMetrics.ActiveBehaviorTrees = 0; // To be set by Behavior Tree system
    CurrentMetrics.PerceptionQueries = 0; // To be set by AI Perception system
    
    // These would be actual stats from the Mass AI system:
    // CurrentMetrics.MassAITime = MassEntitySubsystem->GetLastTickTime();
}

void UPerformanceProfiler::CollectStreamingMetrics()
{
    // World Partition streaming metrics
    CurrentMetrics.LoadedWorldPartitionCells = 0; // To be set by World Partition system
    
    // Texture streaming
    CurrentMetrics.TextureStreamingPoolUsage = 0.0f; // To be set by texture streaming system
}

void UPerformanceProfiler::CheckPerformanceThresholds()
{
    // Check frame time
    if (CurrentMetrics.FrameTime > CurrentBudget.TotalFrameTime)
    {
        LogPerformanceWarning(TEXT("Frame Time"), CurrentMetrics.FrameTime, CurrentBudget.TotalFrameTime);
        OnPerformanceThresholdExceeded.Broadcast(TEXT("Frame Time"), CurrentMetrics.FrameTime);
    }
    
    // Check game thread
    if (CurrentMetrics.GameThreadTime > CurrentBudget.GameThreadBudget)
    {
        LogPerformanceWarning(TEXT("Game Thread"), CurrentMetrics.GameThreadTime, CurrentBudget.GameThreadBudget);
        OnPerformanceThresholdExceeded.Broadcast(TEXT("Game Thread"), CurrentMetrics.GameThreadTime);
    }
    
    // Check render thread
    if (CurrentMetrics.RenderThreadTime > CurrentBudget.RenderThreadBudget)
    {
        LogPerformanceWarning(TEXT("Render Thread"), CurrentMetrics.RenderThreadTime, CurrentBudget.RenderThreadBudget);
        OnPerformanceThresholdExceeded.Broadcast(TEXT("Render Thread"), CurrentMetrics.RenderThreadTime);
    }
    
    // Check GPU
    if (CurrentMetrics.GPUTime > CurrentBudget.GPUBudget)
    {
        LogPerformanceWarning(TEXT("GPU"), CurrentMetrics.GPUTime, CurrentBudget.GPUBudget);
        OnPerformanceThresholdExceeded.Broadcast(TEXT("GPU"), CurrentMetrics.GPUTime);
    }
    
    // Check Mass AI
    if (CurrentMetrics.MassAITime > CurrentBudget.MassAIBudget)
    {
        LogPerformanceWarning(TEXT("Mass AI"), CurrentMetrics.MassAITime, CurrentBudget.MassAIBudget);
        OnPerformanceThresholdExceeded.Broadcast(TEXT("Mass AI"), CurrentMetrics.MassAITime);
    }
    
    // Check memory
    if (CurrentMetrics.TotalMemoryUsed > CurrentBudget.TotalMemoryBudget)
    {
        UE_LOG(LogPerformanceProfiler, Warning, TEXT("Memory usage exceeded budget: %dMB / %dMB"), 
               CurrentMetrics.TotalMemoryUsed, CurrentBudget.TotalMemoryBudget);
        OnPerformanceThresholdExceeded.Broadcast(TEXT("Memory"), CurrentMetrics.TotalMemoryUsed);
    }
}

void UPerformanceProfiler::LogPerformanceWarning(const FString& SystemName, float ActualTime, float BudgetTime)
{
    float OveragePercent = ((ActualTime - BudgetTime) / BudgetTime) * 100.0f;
    UE_LOG(LogPerformanceProfiler, Warning, TEXT("%s over budget: %.2fms / %.2fms (%.1f%% overage)"), 
           *SystemName, ActualTime, BudgetTime, OveragePercent);
}

bool UPerformanceProfiler::IsPerformanceWithinBudget() const
{
    return CurrentMetrics.FrameTime <= CurrentBudget.TotalFrameTime &&
           CurrentMetrics.GameThreadTime <= CurrentBudget.GameThreadBudget &&
           CurrentMetrics.RenderThreadTime <= CurrentBudget.RenderThreadBudget &&
           CurrentMetrics.GPUTime <= CurrentBudget.GPUBudget &&
           CurrentMetrics.TotalMemoryUsed <= CurrentBudget.TotalMemoryBudget;
}

float UPerformanceProfiler::GetPerformanceHealth() const
{
    // Calculate worst-case performance ratio
    float FrameRatio = CurrentMetrics.FrameTime / CurrentBudget.TotalFrameTime;
    float GameThreadRatio = CurrentMetrics.GameThreadTime / CurrentBudget.GameThreadBudget;
    float RenderThreadRatio = CurrentMetrics.RenderThreadTime / CurrentBudget.RenderThreadBudget;
    float GPURatio = CurrentMetrics.GPUTime / CurrentBudget.GPUBudget;
    float MemoryRatio = (float)CurrentMetrics.TotalMemoryUsed / (float)CurrentBudget.TotalMemoryBudget;
    
    float WorstRatio = FMath::Max({FrameRatio, GameThreadRatio, RenderThreadRatio, GPURatio, MemoryRatio});
    
    // Return inverted ratio (1.0 = perfect, 0.0 = over budget)
    return FMath::Clamp(1.0f / WorstRatio, 0.0f, 1.0f);
}

void UPerformanceProfiler::StartProfilingSession(const FString& SessionName)
{
    CurrentSessionName = SessionName;
    bIsProfilingActive = true;
    MetricsHistory.Empty();
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Started profiling session: %s"), *SessionName);
    
    // Start Unreal Insights trace if available
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), TEXT("trace.start default,memory,cpu"));
    }
}

void UPerformanceProfiler::StopProfilingSession()
{
    if (!bIsProfilingActive)
    {
        return;
    }
    
    bIsProfilingActive = false;
    
    // Stop Unreal Insights trace
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), TEXT("trace.stop"));
    }
    
    // Generate performance report
    DumpPerformanceReport();
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Stopped profiling session: %s"), *CurrentSessionName);
    CurrentSessionName.Empty();
}

void UPerformanceProfiler::SetPerformanceTarget(int32 TargetIndex)
{
    if (TargetIndex >= 0 && TargetIndex < (int32)EPerformanceTarget::Mobile_High + 1)
    {
        Initialize((EPerformanceTarget)TargetIndex);
        UE_LOG(LogPerformanceProfiler, Log, TEXT("Performance target changed to: %d"), TargetIndex);
    }
}

void UPerformanceProfiler::ShowPerformanceStats(bool bShow)
{
    if (GEngine)
    {
        if (bShow)
        {
            GEngine->Exec(GetWorld(), TEXT("stat unit"));
            GEngine->Exec(GetWorld(), TEXT("stat fps"));
        }
        else
        {
            GEngine->Exec(GetWorld(), TEXT("stat none"));
        }
    }
}

void UPerformanceProfiler::DumpPerformanceReport()
{
    if (MetricsHistory.Num() == 0)
    {
        return;
    }
    
    // Calculate averages
    FPerformanceMetrics AverageMetrics;
    for (const FPerformanceMetrics& Metrics : MetricsHistory)
    {
        AverageMetrics.FrameTime += Metrics.FrameTime;
        AverageMetrics.GameThreadTime += Metrics.GameThreadTime;
        AverageMetrics.RenderThreadTime += Metrics.RenderThreadTime;
        AverageMetrics.GPUTime += Metrics.GPUTime;
        AverageMetrics.TotalMemoryUsed += Metrics.TotalMemoryUsed;
        AverageMetrics.DrawCalls += Metrics.DrawCalls;
        AverageMetrics.Triangles += Metrics.Triangles;
    }
    
    int32 SampleCount = MetricsHistory.Num();
    AverageMetrics.FrameTime /= SampleCount;
    AverageMetrics.GameThreadTime /= SampleCount;
    AverageMetrics.RenderThreadTime /= SampleCount;
    AverageMetrics.GPUTime /= SampleCount;
    AverageMetrics.TotalMemoryUsed /= SampleCount;
    AverageMetrics.DrawCalls /= SampleCount;
    AverageMetrics.Triangles /= SampleCount;
    
    // Log performance report
    UE_LOG(LogPerformanceProfiler, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Session: %s"), *CurrentSessionName);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Samples: %d"), SampleCount);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Target: %d"), (int32)CurrentTarget);
    UE_LOG(LogPerformanceProfiler, Log, TEXT(""));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("FRAME TIMING (Average):"));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Frame Time: %.2fms (Budget: %.2fms)"), AverageMetrics.FrameTime, CurrentBudget.TotalFrameTime);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Game Thread: %.2fms (Budget: %.2fms)"), AverageMetrics.GameThreadTime, CurrentBudget.GameThreadBudget);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Render Thread: %.2fms (Budget: %.2fms)"), AverageMetrics.RenderThreadTime, CurrentBudget.RenderThreadBudget);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  GPU: %.2fms (Budget: %.2fms)"), AverageMetrics.GPUTime, CurrentBudget.GPUBudget);
    UE_LOG(LogPerformanceProfiler, Log, TEXT(""));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("MEMORY (Average):"));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Total: %dMB (Budget: %dMB)"), AverageMetrics.TotalMemoryUsed, CurrentBudget.TotalMemoryBudget);
    UE_LOG(LogPerformanceProfiler, Log, TEXT(""));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("RENDERING (Average):"));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Draw Calls: %d"), AverageMetrics.DrawCalls);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Triangles: %d"), AverageMetrics.Triangles);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("========================="));
}

// Subsystem implementation
void UPerformanceProfilerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    ProfilerInstance = NewObject<UPerformanceProfiler>(this);
    ProfilerInstance->Initialize(EPerformanceTarget::PC_HighEnd);
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Performance Profiler Subsystem initialized"));
}

void UPerformanceProfilerSubsystem::Deinitialize()
{
    if (ProfilerInstance)
    {
        ProfilerInstance->StopProfilingSession();
        ProfilerInstance = nullptr;
    }
    
    Super::Deinitialize();
}

UPerformanceProfiler* UPerformanceProfilerSubsystem::GetPerformanceProfiler()
{
    if (UGameInstance* GameInstance = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull)->GetGameInstance())
    {
        if (UPerformanceProfilerSubsystem* Subsystem = GameInstance->GetSubsystem<UPerformanceProfilerSubsystem>())
        {
            return Subsystem->ProfilerInstance;
        }
    }
    
    return nullptr;
}