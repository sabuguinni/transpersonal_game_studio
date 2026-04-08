#include "DynamicPerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "RHI.h"
#include "RenderingThread.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "MassEntitySubsystem.h"
#include "Landscape.h"

DEFINE_LOG_CATEGORY_STATIC(LogDynamicPerformance, Log, All);

// Console variables for runtime adjustment
static TAutoConsoleVariable<bool> CVarDynamicPerformanceEnabled(
    TEXT("tp.DynamicPerformance.Enabled"),
    true,
    TEXT("Enable dynamic performance management"),
    ECVF_Default
);

static TAutoConsoleVariable<float> CVarPerformanceAdjustmentSensitivity(
    TEXT("tp.DynamicPerformance.Sensitivity"),
    1.0f,
    TEXT("Sensitivity of dynamic performance adjustments (0.1 - 2.0)"),
    ECVF_Default
);

static TAutoConsoleVariable<bool> CVarPerformanceDebugOutput(
    TEXT("tp.DynamicPerformance.Debug"),
    false,
    TEXT("Enable debug output for performance management"),
    ECVF_Default
);

UDynamicPerformanceManager::UDynamicPerformanceManager()
{
    CurrentTarget = EPerformanceTarget::PC_HighEnd;
    CurrentPerformanceLevel = EPerformanceLevel::High;
    CurrentBottleneck = EPerformanceBottleneck::None;
    CurrentBudget = FPerformanceBudget::GetBudgetForTarget(CurrentTarget);
    CurrentSettings = FPerformanceSettings::GetSettingsForLevel(CurrentPerformanceLevel);
    
    bDynamicAdjustmentEnabled = true;
    AdjustmentSensitivity = 1.0f;
    PerformanceHistoryWindow = 3.0f;
    AdjustmentCooldown = 2.0f;
    LastAdjustmentTime = 0.0f;
    
    // Reserve space for performance history
    FrameTimeHistory.Reserve(180); // 3 seconds at 60fps
    GameThreadHistory.Reserve(180);
    RenderThreadHistory.Reserve(180);
    GPUTimeHistory.Reserve(180);
}

void UDynamicPerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Dynamic Performance Manager initialized"));
    
    // Start performance monitoring
    GetWorld()->GetTimerManager().SetTimer(
        MetricsUpdateTimer,
        this,
        &UDynamicPerformanceManager::UpdatePerformanceMetrics,
        0.1f, // Update every 100ms
        true
    );
    
    // Apply initial settings
    ApplyPerformanceSettings(CurrentSettings);
}

void UDynamicPerformanceManager::Deinitialize()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(MetricsUpdateTimer);
    }
    
    if (bCapturingPerformance)
    {
        StopPerformanceCapture();
    }
    
    Super::Deinitialize();
}

void UDynamicPerformanceManager::SetPerformanceTarget(EPerformanceTarget Target)
{
    if (CurrentTarget != Target)
    {
        CurrentTarget = Target;
        CurrentBudget = FPerformanceBudget::GetBudgetForTarget(Target);
        
        UE_LOG(LogDynamicPerformance, Log, TEXT("Performance target changed to: %d"), (int32)Target);
        
        // Reset performance level to allow re-evaluation
        SetPerformanceLevel(EPerformanceLevel::High);
    }
}

void UDynamicPerformanceManager::SetPerformanceLevel(EPerformanceLevel Level)
{
    if (CurrentPerformanceLevel != Level)
    {
        EPerformanceLevel OldLevel = CurrentPerformanceLevel;
        CurrentPerformanceLevel = Level;
        CurrentSettings = FPerformanceSettings::GetSettingsForLevel(Level);
        
        ApplyPerformanceSettings(CurrentSettings);
        
        UE_LOG(LogDynamicPerformance, Log, TEXT("Performance level changed from %d to %d"), 
               (int32)OldLevel, (int32)Level);
        
        OnPerformanceLevelChanged.Broadcast(Level);
    }
}

void UDynamicPerformanceManager::UpdatePerformanceMetrics()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UDynamicPerformanceManager::UpdatePerformanceMetrics);
    
    // Get current frame timing
    const FGameThreadHitchHeartBeat& HeartBeat = FGameThreadHitchHeartBeat::Get();
    CurrentMetrics.FrameTime = HeartBeat.GetCurrentFrameTime() * 1000.0f; // Convert to ms
    CurrentMetrics.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
    CurrentMetrics.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    CurrentMetrics.GPUTime = FPlatformTime::ToMilliseconds(GGPUFrameTime);
    
    // Get memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.UsedMemory = MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
    
    // Get rendering stats
    CurrentMetrics.DrawCalls = GNumDrawCallsRHI[GRHICommandList.GetGPUMask().ToIndex()];
    CurrentMetrics.Triangles = GNumPrimitivesDrawnRHI[GRHICommandList.GetGPUMask().ToIndex()];
    
    // Get Mass AI stats if available
    if (UMassEntitySubsystem* MassSubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>())
    {
        // These would need to be exposed by Mass AI system
        CurrentMetrics.ActiveAgents = 0; // Placeholder
        CurrentMetrics.ProcessedAgents = 0; // Placeholder
        CurrentMetrics.CulledAgents = 0; // Placeholder
    }
    
    // Update performance history
    UpdatePerformanceHistory();
    
    // Analyze current performance state
    AnalyzeBottlenecks();
    
    // Apply dynamic adjustments if enabled
    if (bDynamicAdjustmentEnabled && CVarDynamicPerformanceEnabled.GetValueOnGameThread())
    {
        ApplyDynamicAdjustments();
    }
    
    // Capture performance data if recording
    if (bCapturingPerformance)
    {
        CapturePerformanceData();
    }
    
    // Debug output
    if (CVarPerformanceDebugOutput.GetValueOnGameThread())
    {
        UE_LOG(LogDynamicPerformance, VeryVerbose, 
               TEXT("Frame: %.2fms, Game: %.2fms, Render: %.2fms, GPU: %.2fms, Bottleneck: %d"),
               CurrentMetrics.FrameTime, CurrentMetrics.GameThreadTime, 
               CurrentMetrics.RenderThreadTime, CurrentMetrics.GPUTime, (int32)CurrentBottleneck);
    }
}

void UDynamicPerformanceManager::UpdatePerformanceHistory()
{
    // Add current metrics to history
    FrameTimeHistory.Add(CurrentMetrics.FrameTime);
    GameThreadHistory.Add(CurrentMetrics.GameThreadTime);
    RenderThreadHistory.Add(CurrentMetrics.RenderThreadTime);
    GPUTimeHistory.Add(CurrentMetrics.GPUTime);
    
    // Maintain history window size
    const int32 MaxHistorySize = FMath::CeilToInt(PerformanceHistoryWindow * 10.0f); // 10 updates per second
    
    if (FrameTimeHistory.Num() > MaxHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
        GameThreadHistory.RemoveAt(0);
        RenderThreadHistory.RemoveAt(0);
        GPUTimeHistory.RemoveAt(0);
    }
}

void UDynamicPerformanceManager::AnalyzeBottlenecks()
{
    EPerformanceBottleneck NewBottleneck = EPerformanceBottleneck::None;
    
    // Calculate average times over recent history
    float AvgFrameTime = CalculateAverage(FrameTimeHistory);
    float AvgGameThreadTime = CalculateAverage(GameThreadHistory);
    float AvgRenderThreadTime = CalculateAverage(RenderThreadHistory);
    float AvgGPUTime = CalculateAverage(GPUTimeHistory);
    
    // Check if we're over budget
    if (AvgFrameTime > CurrentBudget.TotalFrameTime * 1.1f) // 10% tolerance
    {
        // Determine primary bottleneck
        if (AvgGameThreadTime > CurrentBudget.GameThreadBudget)
        {
            NewBottleneck = EPerformanceBottleneck::GameThread;
        }
        else if (AvgRenderThreadTime > CurrentBudget.RenderThreadBudget)
        {
            NewBottleneck = EPerformanceBottleneck::RenderThread;
        }
        else if (AvgGPUTime > CurrentBudget.GPUBudget)
        {
            NewBottleneck = EPerformanceBottleneck::GPU;
        }
        else if (CurrentMetrics.UsedMemory > CurrentBudget.TotalMemoryBudget * 0.9f)
        {
            NewBottleneck = EPerformanceBottleneck::Memory;
        }
    }
    
    // Update bottleneck state
    if (CurrentBottleneck != NewBottleneck)
    {
        CurrentBottleneck = NewBottleneck;
        OnBottleneckDetected.Broadcast(NewBottleneck);
        
        UE_LOG(LogDynamicPerformance, Warning, TEXT("Performance bottleneck detected: %d"), (int32)NewBottleneck);
    }
}

float UDynamicPerformanceManager::CalculateAverage(const TArray<float>& Values)
{
    if (Values.Num() == 0) return 0.0f;
    
    float Sum = 0.0f;
    for (float Value : Values)
    {
        Sum += Value;
    }
    return Sum / Values.Num();
}

void UDynamicPerformanceManager::ApplyDynamicAdjustments()
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check cooldown
    if (CurrentTime - LastAdjustmentTime < AdjustmentCooldown)
    {
        return;
    }
    
    // Only adjust if we have a consistent bottleneck
    if (CurrentBottleneck == EPerformanceBottleneck::None)
    {
        return;
    }
    
    float AdjustmentFactor = CVarPerformanceAdjustmentSensitivity.GetValueOnGameThread() * AdjustmentSensitivity;
    
    switch (CurrentBottleneck)
    {
    case EPerformanceBottleneck::GameThread:
        AdjustMassAIPerformance(CurrentBudget.MassAIBudget, CurrentMetrics.MassAITime);
        break;
        
    case EPerformanceBottleneck::RenderThread:
    case EPerformanceBottleneck::GPU:
        AdjustRenderingPerformance(CurrentBudget.RenderingBudget, CurrentMetrics.RenderingTime);
        break;
        
    case EPerformanceBottleneck::Memory:
        AdjustStreamingPerformance();
        break;
        
    case EPerformanceBottleneck::Physics:
        AdjustPhysicsPerformance(CurrentBudget.PhysicsBudget, CurrentMetrics.PhysicsTime);
        break;
    }
    
    LastAdjustmentTime = CurrentTime;
}

void UDynamicPerformanceManager::AdjustMassAIPerformance(float TargetTime, float CurrentTime)
{
    if (CurrentTime <= TargetTime) return;
    
    // Reduce Mass AI complexity
    FPerformanceSettings NewSettings = CurrentSettings;
    
    // Reduce active agents
    NewSettings.MaxActiveAgents = FMath::Max(1000, FMath::FloorToInt(NewSettings.MaxActiveAgents * 0.9f));
    
    // Reduce tick rate
    NewSettings.AITickRate = FMath::Max(10.0f, NewSettings.AITickRate * 0.95f);
    
    // Increase cull distance
    NewSettings.DinosaurCullDistance = FMath::Min(50000.0f, NewSettings.DinosaurCullDistance * 1.1f);
    
    ApplyPerformanceSettings(NewSettings);
    CurrentSettings = NewSettings;
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Adjusted Mass AI: MaxAgents=%d, TickRate=%.1f, CullDist=%.0f"),
           NewSettings.MaxActiveAgents, NewSettings.AITickRate, NewSettings.DinosaurCullDistance);
}

void UDynamicPerformanceManager::AdjustRenderingPerformance(float TargetTime, float CurrentTime)
{
    if (CurrentTime <= TargetTime) return;
    
    FPerformanceSettings NewSettings = CurrentSettings;
    
    // Reduce screen percentage first
    if (NewSettings.ScreenPercentage > 50.0f)
    {
        NewSettings.ScreenPercentage = FMath::Max(50.0f, NewSettings.ScreenPercentage - 5.0f);
    }
    // Then reduce view distance
    else if (NewSettings.ViewDistanceScale > 25)
    {
        NewSettings.ViewDistanceScale = FMath::Max(25, NewSettings.ViewDistanceScale - 10);
    }
    // Then reduce quality settings
    else
    {
        NewSettings.ShadowQuality = FMath::Max(0, NewSettings.ShadowQuality - 1);
        NewSettings.EffectsQuality = FMath::Max(0, NewSettings.EffectsQuality - 1);
        NewSettings.PostProcessQuality = FMath::Max(0, NewSettings.PostProcessQuality - 1);
    }
    
    ApplyPerformanceSettings(NewSettings);
    CurrentSettings = NewSettings;
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Adjusted Rendering: ScreenPct=%.1f, ViewDist=%d, ShadowQ=%d"),
           NewSettings.ScreenPercentage, NewSettings.ViewDistanceScale, NewSettings.ShadowQuality);
}

void UDynamicPerformanceManager::AdjustPhysicsPerformance(float TargetTime, float CurrentTime)
{
    if (CurrentTime <= TargetTime) return;
    
    FPerformanceSettings NewSettings = CurrentSettings;
    
    // Reduce physics complexity
    NewSettings.MaxPhysicsBodies = FMath::Max(1000, FMath::FloorToInt(NewSettings.MaxPhysicsBodies * 0.9f));
    
    // Reduce physics tick rate slightly
    NewSettings.PhysicsTickRate = FMath::Max(30.0f, NewSettings.PhysicsTickRate * 0.95f);
    
    // Disable destruction if necessary
    if (NewSettings.MaxPhysicsBodies < 2000)
    {
        NewSettings.bDestructionEnabled = false;
    }
    
    ApplyPerformanceSettings(NewSettings);
    CurrentSettings = NewSettings;
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Adjusted Physics: MaxBodies=%d, TickRate=%.1f, Destruction=%d"),
           NewSettings.MaxPhysicsBodies, NewSettings.PhysicsTickRate, NewSettings.bDestructionEnabled);
}

void UDynamicPerformanceManager::AdjustStreamingPerformance()
{
    FPerformanceSettings NewSettings = CurrentSettings;
    
    // Reduce streaming pool size
    NewSettings.TextureStreamingPoolSize = FMath::Max(1024.0f, NewSettings.TextureStreamingPoolSize * 0.9f);
    
    // Reduce max streaming cells
    NewSettings.MaxStreamingCells = FMath::Max(9, NewSettings.MaxStreamingCells - 2);
    
    ApplyPerformanceSettings(NewSettings);
    CurrentSettings = NewSettings;
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Adjusted Streaming: PoolSize=%.0fMB, MaxCells=%d"),
           NewSettings.TextureStreamingPoolSize, NewSettings.MaxStreamingCells);
}

void UDynamicPerformanceManager::ApplyPerformanceSettings(const FPerformanceSettings& Settings)
{
    // Apply rendering settings via console variables
    IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"))->Set(Settings.ScreenPercentage);
    IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale"))->Set(Settings.ViewDistanceScale / 100.0f);
    IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality"))->Set(Settings.ShadowQuality);
    IConsoleManager::Get().FindConsoleVariable(TEXT("sg.TextureQuality"))->Set(Settings.TextureQuality);
    IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality"))->Set(Settings.EffectsQuality);
    IConsoleManager::Get().FindConsoleVariable(TEXT("sg.PostProcessQuality"))->Set(Settings.PostProcessQuality);
    
    // Apply Nanite and Lumen settings
    IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite"))->Set(Settings.bNaniteEnabled ? 1 : 0);
    IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.GlobalIllumination"))->Set(Settings.bLumenEnabled ? 1 : 0);
    IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.Virtual.Enable"))->Set(Settings.bVirtualShadowMapsEnabled ? 1 : 0);
    
    // Apply streaming settings
    IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize"))->Set(Settings.TextureStreamingPoolSize);
    
    // Mass AI settings would be applied through the Mass AI subsystem
    // Physics settings would be applied through the Physics subsystem
    
    UE_LOG(LogDynamicPerformance, Verbose, TEXT("Applied performance settings"));
}

void UDynamicPerformanceManager::ValidateSettings(FPerformanceSettings& Settings)
{
    // Clamp values to valid ranges
    Settings.ScreenPercentage = FMath::Clamp(Settings.ScreenPercentage, 25.0f, 100.0f);
    Settings.ViewDistanceScale = FMath::Clamp(Settings.ViewDistanceScale, 10, 100);
    Settings.ShadowQuality = FMath::Clamp(Settings.ShadowQuality, 0, 3);
    Settings.TextureQuality = FMath::Clamp(Settings.TextureQuality, 0, 3);
    Settings.EffectsQuality = FMath::Clamp(Settings.EffectsQuality, 0, 3);
    Settings.PostProcessQuality = FMath::Clamp(Settings.PostProcessQuality, 0, 3);
    
    Settings.MaxActiveAgents = FMath::Clamp(Settings.MaxActiveAgents, 100, 50000);
    Settings.AITickRate = FMath::Clamp(Settings.AITickRate, 5.0f, 60.0f);
    Settings.DinosaurCullDistance = FMath::Clamp(Settings.DinosaurCullDistance, 5000.0f, 50000.0f);
    
    Settings.PhysicsTickRate = FMath::Clamp(Settings.PhysicsTickRate, 20.0f, 120.0f);
    Settings.MaxPhysicsBodies = FMath::Clamp(Settings.MaxPhysicsBodies, 100, 20000);
    
    Settings.MaxStreamingCells = FMath::Clamp(Settings.MaxStreamingCells, 4, 50);
    Settings.TextureStreamingPoolSize = FMath::Clamp(Settings.TextureStreamingPoolSize, 512.0f, 8192.0f);
}

void UDynamicPerformanceManager::OverrideMassAISettings(int32 MaxAgents, float TickRate, float CullDistance)
{
    FPerformanceSettings NewSettings = CurrentSettings;
    NewSettings.MaxActiveAgents = MaxAgents;
    NewSettings.AITickRate = TickRate;
    NewSettings.DinosaurCullDistance = CullDistance;
    
    ValidateSettings(NewSettings);
    ApplyPerformanceSettings(NewSettings);
    CurrentSettings = NewSettings;
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Mass AI settings overridden: MaxAgents=%d, TickRate=%.1f, CullDist=%.0f"),
           MaxAgents, TickRate, CullDistance);
}

void UDynamicPerformanceManager::OverrideRenderingSettings(float ScreenPercentage, int32 ViewDistance, int32 ShadowQuality)
{
    FPerformanceSettings NewSettings = CurrentSettings;
    NewSettings.ScreenPercentage = ScreenPercentage;
    NewSettings.ViewDistanceScale = ViewDistance;
    NewSettings.ShadowQuality = ShadowQuality;
    
    ValidateSettings(NewSettings);
    ApplyPerformanceSettings(NewSettings);
    CurrentSettings = NewSettings;
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Rendering settings overridden: ScreenPct=%.1f, ViewDist=%d, ShadowQ=%d"),
           ScreenPercentage, ViewDistance, ShadowQuality);
}

void UDynamicPerformanceManager::StartPerformanceCapture(const FString& SessionName)
{
    if (bCapturingPerformance)
    {
        StopPerformanceCapture();
    }
    
    CurrentSessionName = SessionName.IsEmpty() ? 
        FString::Printf(TEXT("PerfCapture_%s"), *FDateTime::Now().ToString()) : SessionName;
    
    bCapturingPerformance = true;
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Started performance capture: %s"), *CurrentSessionName);
}

void UDynamicPerformanceManager::StopPerformanceCapture()
{
    if (!bCapturingPerformance) return;
    
    bCapturingPerformance = false;
    WritePerformanceLog();
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Stopped performance capture: %s"), *CurrentSessionName);
    CurrentSessionName.Empty();
}

void UDynamicPerformanceManager::CapturePerformanceData()
{
    // This would capture detailed performance data to arrays for later analysis
    // Implementation would depend on specific telemetry requirements
}

void UDynamicPerformanceManager::WritePerformanceLog()
{
    // Write performance data to file
    FString LogPath = FPaths::ProjectLogDir() / TEXT("Performance") / (CurrentSessionName + TEXT(".csv"));
    
    // Create directory if it doesn't exist
    FString LogDir = FPaths::GetPath(LogPath);
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*LogDir))
    {
        PlatformFile.CreateDirectoryTree(*LogDir);
    }
    
    // Write CSV header and data
    FString LogContent = TEXT("Timestamp,FrameTime,GameThread,RenderThread,GPU,Memory,DrawCalls,Triangles,ActiveAgents\n");
    
    // This would contain the actual captured data
    // For now, just write current state
    LogContent += FString::Printf(TEXT("%s,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d\n"),
        *FDateTime::Now().ToString(),
        CurrentMetrics.FrameTime,
        CurrentMetrics.GameThreadTime,
        CurrentMetrics.RenderThreadTime,
        CurrentMetrics.GPUTime,
        CurrentMetrics.UsedMemory,
        CurrentMetrics.DrawCalls,
        CurrentMetrics.Triangles,
        CurrentMetrics.ActiveAgents
    );
    
    FFileHelper::SaveStringToFile(LogContent, *LogPath);
}

void UDynamicPerformanceManager::DumpPerformanceReport()
{
    UE_LOG(LogDynamicPerformance, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Target: %d, Level: %d, Bottleneck: %d"), 
           (int32)CurrentTarget, (int32)CurrentPerformanceLevel, (int32)CurrentBottleneck);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Frame: %.2fms (Budget: %.2fms)"), 
           CurrentMetrics.FrameTime, CurrentBudget.TotalFrameTime);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Game Thread: %.2fms (Budget: %.2fms)"), 
           CurrentMetrics.GameThreadTime, CurrentBudget.GameThreadBudget);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Render Thread: %.2fms (Budget: %.2fms)"), 
           CurrentMetrics.RenderThreadTime, CurrentBudget.RenderThreadBudget);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("GPU: %.2fms (Budget: %.2fms)"), 
           CurrentMetrics.GPUTime, CurrentBudget.GPUBudget);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Memory: %.0fMB (Budget: %dMB)"), 
           CurrentMetrics.UsedMemory, CurrentBudget.TotalMemoryBudget);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Draw Calls: %d, Triangles: %d"), 
           CurrentMetrics.DrawCalls, CurrentMetrics.Triangles);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Mass AI Agents: %d (Max: %d)"), 
           CurrentMetrics.ActiveAgents, CurrentSettings.MaxActiveAgents);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("========================"));
}

// Static function implementations
FPerformanceSettings FPerformanceSettings::GetSettingsForLevel(EPerformanceLevel Level)
{
    FPerformanceSettings Settings;
    
    switch (Level)
    {
    case EPerformanceLevel::Ultra:
        Settings.ScreenPercentage = 100.0f;
        Settings.ViewDistanceScale = 100;
        Settings.ShadowQuality = 3;
        Settings.TextureQuality = 3;
        Settings.EffectsQuality = 3;
        Settings.PostProcessQuality = 3;
        Settings.MaxActiveAgents = 50000;
        Settings.AITickRate = 30.0f;
        Settings.DinosaurCullDistance = 30000.0f;
        Settings.bNaniteEnabled = true;
        Settings.bLumenEnabled = true;
        Settings.bVirtualShadowMapsEnabled = true;
        break;
        
    case EPerformanceLevel::High:
        Settings.ScreenPercentage = 90.0f;
        Settings.ViewDistanceScale = 90;
        Settings.ShadowQuality = 3;
        Settings.TextureQuality = 3;
        Settings.EffectsQuality = 2;
        Settings.PostProcessQuality = 2;
        Settings.MaxActiveAgents = 35000;
        Settings.AITickRate = 25.0f;
        Settings.DinosaurCullDistance = 25000.0f;
        break;
        
    case EPerformanceLevel::Medium:
        Settings.ScreenPercentage = 80.0f;
        Settings.ViewDistanceScale = 75;
        Settings.ShadowQuality = 2;
        Settings.TextureQuality = 2;
        Settings.EffectsQuality = 2;
        Settings.PostProcessQuality = 2;
        Settings.MaxActiveAgents = 20000;
        Settings.AITickRate = 20.0f;
        Settings.DinosaurCullDistance = 20000.0f;
        break;
        
    case EPerformanceLevel::Low:
        Settings.ScreenPercentage = 70.0f;
        Settings.ViewDistanceScale = 60;
        Settings.ShadowQuality = 1;
        Settings.TextureQuality = 1;
        Settings.EffectsQuality = 1;
        Settings.PostProcessQuality = 1;
        Settings.MaxActiveAgents = 10000;
        Settings.AITickRate = 15.0f;
        Settings.DinosaurCullDistance = 15000.0f;
        Settings.bLumenEnabled = false;
        break;
        
    case EPerformanceLevel::Potato:
        Settings.ScreenPercentage = 50.0f;
        Settings.ViewDistanceScale = 40;
        Settings.ShadowQuality = 0;
        Settings.TextureQuality = 0;
        Settings.EffectsQuality = 0;
        Settings.PostProcessQuality = 0;
        Settings.MaxActiveAgents = 5000;
        Settings.AITickRate = 10.0f;
        Settings.DinosaurCullDistance = 10000.0f;
        Settings.bNaniteEnabled = false;
        Settings.bLumenEnabled = false;
        Settings.bVirtualShadowMapsEnabled = false;
        break;
    }
    
    return Settings;
}