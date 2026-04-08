#include "PerformanceBudgetManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/ConfigCacheIni.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "RHI.h"
#include "RenderingThread.h"

DEFINE_LOG_CATEGORY(LogPerformanceBudget);

void UPerformanceBudgetManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogPerformanceBudget, Log, TEXT("Performance Budget Manager initialized"));
    
    InitializePerformanceBudgets();
    
    // Initialize frame time history
    FrameTimeHistory.SetNum(MaxFrameHistory);
    for (int32 i = 0; i < MaxFrameHistory; ++i)
    {
        FrameTimeHistory[i] = 16.67f; // Initialize to 60fps target
    }
}

void UPerformanceBudgetManager::Deinitialize()
{
    UE_LOG(LogPerformanceBudget, Log, TEXT("Performance Budget Manager deinitialized"));
    Super::Deinitialize();
}

bool UPerformanceBudgetManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UPerformanceBudgetManager::InitializePerformanceBudgets()
{
    PerformanceBudgets.Empty();
    
    // Define performance budgets based on platform
    const bool bIsConsole = IsConsole();
    const float TargetFrameTimeMS = bIsConsole ? 33.33f : 16.67f; // 30fps console, 60fps PC
    
    // Game Thread Budgets (40% of frame time)
    const float GameThreadBudget = TargetFrameTimeMS * 0.4f;
    
    FPerformanceBudgetEntry CoreBudget;
    CoreBudget.Category = EPerformanceBudgetCategory::GameThread_Core;
    CoreBudget.TargetTimeMS_PC = GameThreadBudget * 0.3f;      // 2.0ms
    CoreBudget.TargetTimeMS_Console = GameThreadBudget * 0.3f; // 4.0ms
    PerformanceBudgets.Add(CoreBudget);
    
    FPerformanceBudgetEntry AIBudget;
    AIBudget.Category = EPerformanceBudgetCategory::GameThread_AI;
    AIBudget.TargetTimeMS_PC = GameThreadBudget * 0.5f;      // 3.33ms
    AIBudget.TargetTimeMS_Console = GameThreadBudget * 0.5f; // 6.67ms
    PerformanceBudgets.Add(AIBudget);
    
    FPerformanceBudgetEntry PhysicsBudget;
    PhysicsBudget.Category = EPerformanceBudgetCategory::GameThread_Physics;
    PhysicsBudget.TargetTimeMS_PC = GameThreadBudget * 0.2f;      // 1.33ms
    PhysicsBudget.TargetTimeMS_Console = GameThreadBudget * 0.2f; // 2.67ms
    PerformanceBudgets.Add(PhysicsBudget);
    
    // Render Thread Budgets (35% of frame time)
    const float RenderThreadBudget = TargetFrameTimeMS * 0.35f;
    
    FPerformanceBudgetEntry GeometryBudget;
    GeometryBudget.Category = EPerformanceBudgetCategory::RenderThread_Geometry;
    GeometryBudget.TargetTimeMS_PC = RenderThreadBudget * 0.5f;      // 2.92ms
    GeometryBudget.TargetTimeMS_Console = RenderThreadBudget * 0.5f; // 5.83ms
    PerformanceBudgets.Add(GeometryBudget);
    
    FPerformanceBudgetEntry LightingBudget;
    LightingBudget.Category = EPerformanceBudgetCategory::RenderThread_Lighting;
    LightingBudget.TargetTimeMS_PC = RenderThreadBudget * 0.3f;      // 1.75ms
    LightingBudget.TargetTimeMS_Console = RenderThreadBudget * 0.3f; // 3.5ms
    PerformanceBudgets.Add(LightingBudget);
    
    FPerformanceBudgetEntry EffectsBudget;
    EffectsBudget.Category = EPerformanceBudgetCategory::RenderThread_Effects;
    EffectsBudget.TargetTimeMS_PC = RenderThreadBudget * 0.2f;      // 1.17ms
    EffectsBudget.TargetTimeMS_Console = RenderThreadBudget * 0.2f; // 2.33ms
    PerformanceBudgets.Add(EffectsBudget);
    
    // GPU Budgets (varies by platform)
    const float GPUBudget = bIsConsole ? 28.0f : 14.0f; // Console has more GPU budget relative to CPU
    
    FPerformanceBudgetEntry BasePassBudget;
    BasePassBudget.Category = EPerformanceBudgetCategory::GPU_BasePass;
    BasePassBudget.TargetTimeMS_PC = GPUBudget * 0.25f;     // 3.5ms
    BasePassBudget.TargetTimeMS_Console = GPUBudget * 0.25f; // 7.0ms
    PerformanceBudgets.Add(BasePassBudget);
    
    FPerformanceBudgetEntry ShadowsBudget;
    ShadowsBudget.Category = EPerformanceBudgetCategory::GPU_Shadows;
    ShadowsBudget.TargetTimeMS_PC = GPUBudget * 0.2f;     // 2.8ms
    ShadowsBudget.TargetTimeMS_Console = GPUBudget * 0.2f; // 5.6ms
    PerformanceBudgets.Add(ShadowsBudget);
    
    FPerformanceBudgetEntry NaniteBudget;
    NaniteBudget.Category = EPerformanceBudgetCategory::GPU_Nanite;
    NaniteBudget.TargetTimeMS_PC = GPUBudget * 0.2f;     // 2.8ms
    NaniteBudget.TargetTimeMS_Console = GPUBudget * 0.2f; // 5.6ms
    PerformanceBudgets.Add(NaniteBudget);
    
    FPerformanceBudgetEntry LumenBudget;
    LumenBudget.Category = EPerformanceBudgetCategory::GPU_Lumen;
    LumenBudget.TargetTimeMS_PC = GPUBudget * 0.25f;     // 3.5ms
    LumenBudget.TargetTimeMS_Console = GPUBudget * 0.25f; // 7.0ms
    PerformanceBudgets.Add(LumenBudget);
    
    FPerformanceBudgetEntry PostProcessBudget;
    PostProcessBudget.Category = EPerformanceBudgetCategory::GPU_PostProcess;
    PostProcessBudget.TargetTimeMS_PC = GPUBudget * 0.1f;     // 1.4ms
    PostProcessBudget.TargetTimeMS_Console = GPUBudget * 0.1f; // 2.8ms
    PerformanceBudgets.Add(PostProcessBudget);
    
    UE_LOG(LogPerformanceBudget, Log, TEXT("Performance budgets initialized for %s (Target: %.2fms)"), 
           bIsConsole ? TEXT("Console") : TEXT("PC"), TargetFrameTimeMS);
}

void UPerformanceBudgetManager::UpdatePerformanceMetrics(float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UPerformanceBudgetManager::UpdatePerformanceMetrics);
    
    // Update current frame time
    CurrentFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Update frame time history
    UpdateFrameTimeHistory(CurrentFrameTime);
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    for (const float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    AverageFrameTime = TotalFrameTime / MaxFrameHistory;
    
    // Update dinosaur-specific metrics
    UpdateDinosaurMetrics();
    
    // Check performance thresholds
    CheckPerformanceThresholds();
}

bool UPerformanceBudgetManager::IsPerformanceBudgetExceeded(EPerformanceBudgetCategory Category) const
{
    for (const FPerformanceBudgetEntry& Budget : PerformanceBudgets)
    {
        if (Budget.Category == Category)
        {
            const float TargetTime = IsConsole() ? Budget.TargetTimeMS_Console : Budget.TargetTimeMS_PC;
            const float ThresholdTime = TargetTime * (Budget.WarningThresholdPercent / 100.0f);
            return Budget.CurrentTimeMS > ThresholdTime;
        }
    }
    return false;
}

float UPerformanceBudgetManager::GetTargetFrameTime() const
{
    return IsConsole() ? 33.33f : 16.67f;
}

void UPerformanceBudgetManager::ScalePerformanceForDinosaurCount(int32 DinosaurCount)
{
    // Dynamic scaling based on dinosaur count
    if (DinosaurCount > 1000)
    {
        PerformanceScaleFactor = 0.7f;
        UE_LOG(LogPerformanceBudget, Warning, TEXT("High dinosaur count (%d), applying performance scaling (%.2f)"), 
               DinosaurCount, PerformanceScaleFactor);
    }
    else if (DinosaurCount > 500)
    {
        PerformanceScaleFactor = 0.85f;
    }
    else
    {
        PerformanceScaleFactor = 1.0f;
    }
    
    // Apply scaling to console variables
    if (PerformanceScaleFactor < 1.0f)
    {
        // Reduce AI tick frequency
        if (auto* World = GetWorld())
        {
            if (auto* Console = World->GetGameViewport()->ViewportConsole)
            {
                FString Command = FString::Printf(TEXT("ai.TickFrequencyScale %.2f"), PerformanceScaleFactor);
                Console->ConsoleCommand(Command);
            }
        }
    }
}

void UPerformanceBudgetManager::ApplyEmergencyPerformanceScaling()
{
    if (bEmergencyScalingActive)
    {
        return;
    }
    
    bEmergencyScalingActive = true;
    UE_LOG(LogPerformanceBudget, Warning, TEXT("Applying emergency performance scaling"));
    
    ReduceAITickFrequency();
    ReduceRenderingQuality();
}

void UPerformanceBudgetManager::RestoreNormalPerformanceScaling()
{
    if (!bEmergencyScalingActive)
    {
        return;
    }
    
    bEmergencyScalingActive = false;
    UE_LOG(LogPerformanceBudget, Log, TEXT("Restoring normal performance scaling"));
    
    RestoreAITickFrequency();
    RestoreRenderingQuality();
}

void UPerformanceBudgetManager::EnablePerformanceHUD(bool bEnabled)
{
    if (auto* World = GetWorld())
    {
        if (auto* Console = World->GetGameViewport()->ViewportConsole)
        {
            if (bEnabled)
            {
                Console->ConsoleCommand(TEXT("stat unit"));
                Console->ConsoleCommand(TEXT("stat fps"));
                Console->ConsoleCommand(TEXT("stat ai"));
            }
            else
            {
                Console->ConsoleCommand(TEXT("stat none"));
            }
        }
    }
}

void UPerformanceBudgetManager::LogPerformanceReport()
{
    UE_LOG(LogPerformanceBudget, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogPerformanceBudget, Log, TEXT("Current Frame Time: %.2fms"), CurrentFrameTime);
    UE_LOG(LogPerformanceBudget, Log, TEXT("Average Frame Time: %.2fms"), AverageFrameTime);
    UE_LOG(LogPerformanceBudget, Log, TEXT("Target Frame Time: %.2fms"), GetTargetFrameTime());
    UE_LOG(LogPerformanceBudget, Log, TEXT("Performance Scale Factor: %.2f"), PerformanceScaleFactor);
    UE_LOG(LogPerformanceBudget, Log, TEXT("Emergency Scaling Active: %s"), bEmergencyScalingActive ? TEXT("Yes") : TEXT("No"));
    
    UE_LOG(LogPerformanceBudget, Log, TEXT("Dinosaur Metrics:"));
    UE_LOG(LogPerformanceBudget, Log, TEXT("  Total Dinosaurs: %d"), DinosaurMetrics.TotalDinosaursInScene);
    UE_LOG(LogPerformanceBudget, Log, TEXT("  Active AI: %d"), DinosaurMetrics.ActiveAIDinosaurs);
    UE_LOG(LogPerformanceBudget, Log, TEXT("  Visible: %d"), DinosaurMetrics.VisibleDinosaurs);
    UE_LOG(LogPerformanceBudget, Log, TEXT("  AI Tick Time: %.2fms"), DinosaurMetrics.AverageAITickTime);
    UE_LOG(LogPerformanceBudget, Log, TEXT("  AI Memory: %.2fMB"), DinosaurMetrics.TotalAIMemoryMB);
    
    for (const FPerformanceBudgetEntry& Budget : PerformanceBudgets)
    {
        const float TargetTime = IsConsole() ? Budget.TargetTimeMS_Console : Budget.TargetTimeMS_PC;
        const float UsagePercent = (Budget.CurrentTimeMS / TargetTime) * 100.0f;
        
        UE_LOG(LogPerformanceBudget, Log, TEXT("Budget %d: %.2fms / %.2fms (%.1f%%)"), 
               (int32)Budget.Category, Budget.CurrentTimeMS, TargetTime, UsagePercent);
    }
}

void UPerformanceBudgetManager::UpdateFrameTimeHistory(float FrameTime)
{
    FrameTimeHistory[FrameHistoryIndex] = FrameTime;
    FrameHistoryIndex = (FrameHistoryIndex + 1) % MaxFrameHistory;
}

void UPerformanceBudgetManager::UpdateDinosaurMetrics()
{
    // This would be populated by the AI system
    // For now, we'll set up the structure
    
    // Reset metrics
    DinosaurMetrics = FDinosaurPerformanceMetrics();
    
    // These would be updated by the actual AI and rendering systems
    // DinosaurMetrics.TotalDinosaursInScene = DinosaurManager->GetTotalDinosaurCount();
    // DinosaurMetrics.ActiveAIDinosaurs = DinosaurManager->GetActiveAICount();
    // etc.
}

void UPerformanceBudgetManager::CheckPerformanceThresholds()
{
    const float TargetFrameTime = GetTargetFrameTime();
    const float CriticalThreshold = TargetFrameTime * 1.2f; // 20% over target
    
    if (AverageFrameTime > CriticalThreshold && !bEmergencyScalingActive)
    {
        ApplyEmergencyPerformanceScaling();
    }
    else if (AverageFrameTime < TargetFrameTime * 1.05f && bEmergencyScalingActive)
    {
        RestoreNormalPerformanceScaling();
    }
}

bool UPerformanceBudgetManager::IsConsole() const
{
    return PLATFORM_CONSOLE;
}

bool UPerformanceBudgetManager::IsPC() const
{
    return PLATFORM_DESKTOP;
}

void UPerformanceBudgetManager::ReduceAITickFrequency()
{
    if (auto* World = GetWorld())
    {
        if (auto* Console = World->GetGameViewport()->ViewportConsole)
        {
            Console->ConsoleCommand(TEXT("ai.TickFrequencyScale 0.5"));
            Console->ConsoleCommand(TEXT("ai.MaxBehaviorTreesPerFrame 50"));
        }
    }
}

void UPerformanceBudgetManager::ReduceRenderingQuality()
{
    if (auto* World = GetWorld())
    {
        if (auto* Console = World->GetGameViewport()->ViewportConsole)
        {
            Console->ConsoleCommand(TEXT("r.ViewDistanceScale 0.8"));
            Console->ConsoleCommand(TEXT("r.Nanite.MaxPixelsPerEdge 2"));
            Console->ConsoleCommand(TEXT("foliage.CullDistanceScale 0.7"));
        }
    }
}

void UPerformanceBudgetManager::RestoreAITickFrequency()
{
    if (auto* World = GetWorld())
    {
        if (auto* Console = World->GetGameViewport()->ViewportConsole)
        {
            Console->ConsoleCommand(TEXT("ai.TickFrequencyScale 1.0"));
            Console->ConsoleCommand(TEXT("ai.MaxBehaviorTreesPerFrame 100"));
        }
    }
}

void UPerformanceBudgetManager::RestoreRenderingQuality()
{
    if (auto* World = GetWorld())
    {
        if (auto* Console = World->GetGameViewport()->ViewportConsole)
        {
            Console->ConsoleCommand(TEXT("r.ViewDistanceScale 1.0"));
            Console->ConsoleCommand(TEXT("r.Nanite.MaxPixelsPerEdge 1"));
            Console->ConsoleCommand(TEXT("foliage.CullDistanceScale 1.0"));
        }
    }
}