#include "DynamicPerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "Stats/Stats.h"
#include "MassEntitySubsystem.h"
#include "Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

DEFINE_LOG_CATEGORY_STATIC(LogDynamicPerformance, Log, All);

// Console variables for runtime tuning
static TAutoConsoleVariable<bool> CVarDynamicPerformanceEnabled(
    TEXT("tp.DynamicPerformance.Enabled"),
    true,
    TEXT("Enable dynamic performance management"),
    ECVF_Default
);

static TAutoConsoleVariable<float> CVarPerformanceAdjustmentSensitivity(
    TEXT("tp.DynamicPerformance.Sensitivity"),
    1.0f,
    TEXT("Sensitivity of performance adjustments (0.1 - 2.0)"),
    ECVF_Default
);

static TAutoConsoleVariable<bool> CVarShowPerformanceDebug(
    TEXT("tp.DynamicPerformance.ShowDebug"),
    false,
    TEXT("Show performance debug information on screen"),
    ECVF_Default
);

UDynamicPerformanceManager::UDynamicPerformanceManager()
{
    // Initialize default settings
    CurrentTarget = EPerformanceTarget::PC_HighEnd;
    CurrentPerformanceLevel = EPerformanceLevel::High;
    CurrentBottleneck = EPerformanceBottleneck::None;
    CurrentBudget = FPerformanceBudget::GetBudgetForTarget(CurrentTarget);
    
    bDynamicAdjustmentEnabled = true;
    AdjustmentSensitivity = 1.0f;
    PerformanceHistoryWindow = 3.0f;
    AdjustmentCooldown = 2.0f;
    LastAdjustmentTime = 0.0f;
    
    // Initialize history arrays
    FrameTimeHistory.Reserve(180); // 3 seconds at 60fps
    GameThreadHistory.Reserve(180);
    RenderThreadHistory.Reserve(180);
    GPUTimeHistory.Reserve(180);
}

void UDynamicPerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Dynamic Performance Manager initialized"));
    
    // Set up metrics update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MetricsUpdateTimer,
            FTimerDelegate::CreateUObject(this, &UDynamicPerformanceManager::UpdatePerformanceMetrics),
            0.1f, // Update every 100ms
            true
        );
    }
    
    // Apply initial performance settings
    ApplyPerformanceSettings(FPerformanceSettings::GetSettingsForLevel(CurrentPerformanceLevel));
}

void UDynamicPerformanceManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
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
        
        // Reapply settings for new target
        ApplyPerformanceSettings(FPerformanceSettings::GetSettingsForLevel(CurrentPerformanceLevel));
    }
}

void UDynamicPerformanceManager::SetPerformanceLevel(EPerformanceLevel Level)
{
    if (CurrentPerformanceLevel != Level)
    {
        EPerformanceLevel OldLevel = CurrentPerformanceLevel;
        CurrentPerformanceLevel = Level;
        
        // Apply new settings
        FPerformanceSettings NewSettings = FPerformanceSettings::GetSettingsForLevel(Level);
        ApplyPerformanceSettings(NewSettings);
        
        UE_LOG(LogDynamicPerformance, Log, TEXT("Performance level changed from %d to %d"), (int32)OldLevel, (int32)Level);
        
        // Broadcast event
        OnPerformanceLevelChanged.Broadcast(Level);
    }
}

void UDynamicPerformanceManager::UpdatePerformanceMetrics()
{
    if (!CVarDynamicPerformanceEnabled.GetValueOnGameThread())
    {
        return;
    }
    
    // Gather performance metrics
    FPerformanceMetrics NewMetrics;
    
    // Frame timing
    NewMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    
    // Get engine stats
    if (GEngine && GEngine->GetEngineStats())
    {
        // Game thread time
        NewMetrics.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
        
        // Render thread time
        NewMetrics.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
        
        // GPU time (approximate)
        NewMetrics.GPUTime = FPlatformTime::ToMilliseconds(GGPUFrameTime);
    }
    
    // Memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    NewMetrics.UsedMemory = MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
    
    // Rendering stats
    if (GEngine && GEngine->GetEngineStats())
    {
        // These would need to be properly gathered from the rendering system
        NewMetrics.DrawCalls = 0; // TODO: Get from RHI stats
        NewMetrics.Triangles = 0; // TODO: Get from rendering stats
        NewMetrics.VisiblePrimitives = 0; // TODO: Get from scene rendering
    }
    
    // Mass AI stats
    if (UWorld* World = GetWorld())
    {
        if (UMassEntitySubsystem* MassSubsystem = World->GetSubsystem<UMassEntitySubsystem>())
        {
            // TODO: Get actual Mass AI statistics
            NewMetrics.ActiveAgents = 0;
            NewMetrics.ProcessedAgents = 0;
            NewMetrics.CulledAgents = 0;
        }
    }
    
    CurrentMetrics = NewMetrics;
    
    // Update performance history
    FrameTimeHistory.Add(NewMetrics.FrameTime);
    GameThreadHistory.Add(NewMetrics.GameThreadTime);
    RenderThreadHistory.Add(NewMetrics.RenderThreadTime);
    GPUTimeHistory.Add(NewMetrics.GPUTime);
    
    // Maintain history window
    const int32 MaxHistorySize = FMath::CeilToInt(PerformanceHistoryWindow * 10.0f); // 10 samples per second
    if (FrameTimeHistory.Num() > MaxHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
        GameThreadHistory.RemoveAt(0);
        RenderThreadHistory.RemoveAt(0);
        GPUTimeHistory.RemoveAt(0);
    }
    
    // Analyze bottlenecks
    AnalyzeBottlenecks();
    
    // Apply dynamic adjustments if enabled
    if (bDynamicAdjustmentEnabled)
    {
        ApplyDynamicAdjustments();
    }
    
    // Capture performance data if recording
    if (bCapturingPerformance)
    {
        CapturePerformanceData();
    }
    
    // Show debug info if enabled
    if (CVarShowPerformanceDebug.GetValueOnGameThread() && GEngine)
    {
        FString DebugText = FString::Printf(
            TEXT("Performance: Frame=%.2fms Game=%.2fms Render=%.2fms GPU=%.2fms Level=%d Bottleneck=%d"),
            NewMetrics.FrameTime,
            NewMetrics.GameThreadTime,
            NewMetrics.RenderThreadTime,
            NewMetrics.GPUTime,
            (int32)CurrentPerformanceLevel,
            (int32)CurrentBottleneck
        );
        GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, DebugText);
    }
}

void UDynamicPerformanceManager::AnalyzeBottlenecks()
{
    if (FrameTimeHistory.Num() < 10)
    {
        return; // Not enough data
    }
    
    // Calculate averages
    float AvgFrameTime = 0.0f;
    float AvgGameThread = 0.0f;
    float AvgRenderThread = 0.0f;
    float AvgGPU = 0.0f;
    
    const int32 SampleCount = FMath::Min(30, FrameTimeHistory.Num()); // Last 3 seconds
    for (int32 i = FrameTimeHistory.Num() - SampleCount; i < FrameTimeHistory.Num(); ++i)
    {
        AvgFrameTime += FrameTimeHistory[i];
        AvgGameThread += GameThreadHistory[i];
        AvgRenderThread += RenderThreadHistory[i];
        AvgGPU += GPUTimeHistory[i];
    }
    
    AvgFrameTime /= SampleCount;
    AvgGameThread /= SampleCount;
    AvgRenderThread /= SampleCount;
    AvgGPU /= SampleCount;
    
    // Determine bottleneck
    EPerformanceBottleneck NewBottleneck = EPerformanceBottleneck::None;
    
    if (AvgFrameTime > CurrentBudget.TotalFrameTime * 1.1f) // 10% over budget
    {
        // Find the worst offender
        float GameThreadRatio = AvgGameThread / CurrentBudget.GameThreadBudget;
        float RenderThreadRatio = AvgRenderThread / CurrentBudget.RenderThreadBudget;
        float GPURatio = AvgGPU / CurrentBudget.GPUBudget;
        
        if (GameThreadRatio > RenderThreadRatio && GameThreadRatio > GPURatio)
        {
            if (AvgGameThread > CurrentBudget.MassAIBudget * 2.0f)
            {
                NewBottleneck = EPerformanceBottleneck::MassAI;
            }
            else if (AvgGameThread > CurrentBudget.PhysicsBudget * 2.0f)
            {
                NewBottleneck = EPerformanceBottleneck::Physics;
            }
            else
            {
                NewBottleneck = EPerformanceBottleneck::GameThread;
            }
        }
        else if (RenderThreadRatio > GPURatio)
        {
            NewBottleneck = EPerformanceBottleneck::RenderThread;
        }
        else
        {
            NewBottleneck = EPerformanceBottleneck::GPU;
        }
        
        // Check for memory pressure
        if (CurrentMetrics.UsedMemory > CurrentBudget.TotalMemoryBudget * 0.9f)
        {
            NewBottleneck = EPerformanceBottleneck::Memory;
        }
    }
    
    if (CurrentBottleneck != NewBottleneck)
    {
        CurrentBottleneck = NewBottleneck;
        OnBottleneckDetected.Broadcast(NewBottleneck);
        
        UE_LOG(LogDynamicPerformance, Warning, TEXT("Performance bottleneck detected: %d"), (int32)NewBottleneck);
    }
}

void UDynamicPerformanceManager::ApplyDynamicAdjustments()
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Don't adjust too frequently
    if (CurrentTime - LastAdjustmentTime < AdjustmentCooldown)
    {
        return;
    }
    
    if (FrameTimeHistory.Num() < 30)
    {
        return; // Need enough data
    }
    
    // Calculate performance health
    const float AvgFrameTime = FrameTimeHistory.Last(30).GetAverage();
    const float TargetFrameTime = CurrentBudget.TotalFrameTime;
    const float PerformanceRatio = AvgFrameTime / TargetFrameTime;
    
    EPerformanceLevel TargetLevel = CurrentPerformanceLevel;
    
    // Determine if we need to change performance level
    if (PerformanceRatio > 1.2f) // 20% over budget
    {
        // Performance is bad, reduce quality
        if (CurrentPerformanceLevel > EPerformanceLevel::Potato)
        {
            TargetLevel = (EPerformanceLevel)((int32)CurrentPerformanceLevel + 1);
        }
    }
    else if (PerformanceRatio < 0.8f) // 20% under budget
    {
        // Performance is good, increase quality
        if (CurrentPerformanceLevel > EPerformanceLevel::Ultra)
        {
            TargetLevel = (EPerformanceLevel)((int32)CurrentPerformanceLevel - 1);
        }
    }
    
    // Apply adjustment if needed
    if (TargetLevel != CurrentPerformanceLevel)
    {
        SetPerformanceLevel(TargetLevel);
        LastAdjustmentTime = CurrentTime;
        
        UE_LOG(LogDynamicPerformance, Log, TEXT("Dynamic adjustment: Performance ratio %.2f, changed to level %d"), 
               PerformanceRatio, (int32)TargetLevel);
    }
}

void UDynamicPerformanceManager::ApplyPerformanceSettings(const FPerformanceSettings& Settings)
{
    CurrentSettings = Settings;
    
    // Apply rendering settings
    if (IConsoleVariable* ScreenPercentageCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage")))
    {
        ScreenPercentageCVar->Set(Settings.ScreenPercentage);
    }
    
    if (IConsoleVariable* ViewDistanceCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale")))
    {
        ViewDistanceCVar->Set(Settings.ViewDistanceScale / 100.0f);
    }
    
    if (IConsoleVariable* ShadowQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality")))
    {
        ShadowQualityCVar->Set(Settings.ShadowQuality);
    }
    
    if (IConsoleVariable* TextureQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.TextureQuality")))
    {
        TextureQualityCVar->Set(Settings.TextureQuality);
    }
    
    if (IConsoleVariable* EffectsQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality")))
    {
        EffectsQualityCVar->Set(Settings.EffectsQuality);
    }
    
    if (IConsoleVariable* PostProcessQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.PostProcessQuality")))
    {
        PostProcessQualityCVar->Set(Settings.PostProcessQuality);
    }
    
    // Apply Nanite settings
    if (IConsoleVariable* NaniteCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite")))
    {
        NaniteCVar->Set(Settings.bNaniteEnabled ? 1 : 0);
    }
    
    // Apply Lumen settings
    if (IConsoleVariable* LumenCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicGlobalIlluminationMethod")))
    {
        LumenCVar->Set(Settings.bLumenEnabled ? 1 : 0);
    }
    
    // Apply Virtual Shadow Maps settings
    if (IConsoleVariable* VSMCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.Virtual.Enable")))
    {
        VSMCVar->Set(Settings.bVirtualShadowMapsEnabled ? 1 : 0);
    }
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Applied performance settings: ScreenPercentage=%.1f, ShadowQuality=%d"), 
           Settings.ScreenPercentage, Settings.ShadowQuality);
}

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
        Settings.bNaniteEnabled = true;
        Settings.bLumenEnabled = true;
        Settings.bVirtualShadowMapsEnabled = true;
        Settings.MaxActiveAgents = 50000;
        Settings.AITickRate = 60.0f;
        Settings.DinosaurCullDistance = 30000.0f;
        break;
        
    case EPerformanceLevel::High:
        Settings.ScreenPercentage = 90.0f;
        Settings.ViewDistanceScale = 90;
        Settings.ShadowQuality = 3;
        Settings.TextureQuality = 3;
        Settings.EffectsQuality = 2;
        Settings.PostProcessQuality = 2;
        Settings.bNaniteEnabled = true;
        Settings.bLumenEnabled = true;
        Settings.bVirtualShadowMapsEnabled = true;
        Settings.MaxActiveAgents = 40000;
        Settings.AITickRate = 45.0f;
        Settings.DinosaurCullDistance = 25000.0f;
        break;
        
    case EPerformanceLevel::Medium:
        Settings.ScreenPercentage = 80.0f;
        Settings.ViewDistanceScale = 75;
        Settings.ShadowQuality = 2;
        Settings.TextureQuality = 2;
        Settings.EffectsQuality = 2;
        Settings.PostProcessQuality = 1;
        Settings.bNaniteEnabled = true;
        Settings.bLumenEnabled = true;
        Settings.bVirtualShadowMapsEnabled = false;
        Settings.MaxActiveAgents = 30000;
        Settings.AITickRate = 30.0f;
        Settings.DinosaurCullDistance = 20000.0f;
        break;
        
    case EPerformanceLevel::Low:
        Settings.ScreenPercentage = 70.0f;
        Settings.ViewDistanceScale = 60;
        Settings.ShadowQuality = 1;
        Settings.TextureQuality = 1;
        Settings.EffectsQuality = 1;
        Settings.PostProcessQuality = 0;
        Settings.bNaniteEnabled = true;
        Settings.bLumenEnabled = false;
        Settings.bVirtualShadowMapsEnabled = false;
        Settings.MaxActiveAgents = 20000;
        Settings.AITickRate = 20.0f;
        Settings.DinosaurCullDistance = 15000.0f;
        break;
        
    case EPerformanceLevel::Potato:
        Settings.ScreenPercentage = 50.0f;
        Settings.ViewDistanceScale = 40;
        Settings.ShadowQuality = 0;
        Settings.TextureQuality = 0;
        Settings.EffectsQuality = 0;
        Settings.PostProcessQuality = 0;
        Settings.bNaniteEnabled = false;
        Settings.bLumenEnabled = false;
        Settings.bVirtualShadowMapsEnabled = false;
        Settings.MaxActiveAgents = 10000;
        Settings.AITickRate = 15.0f;
        Settings.DinosaurCullDistance = 10000.0f;
        break;
    }
    
    return Settings;
}

void UDynamicPerformanceManager::StartPerformanceCapture(const FString& SessionName)
{
    if (bCapturingPerformance)
    {
        StopPerformanceCapture();
    }
    
    bCapturingPerformance = true;
    CurrentSessionName = SessionName;
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Started performance capture session: %s"), *SessionName);
}

void UDynamicPerformanceManager::StopPerformanceCapture()
{
    if (bCapturingPerformance)
    {
        bCapturingPerformance = false;
        WritePerformanceLog();
        
        UE_LOG(LogDynamicPerformance, Log, TEXT("Stopped performance capture session: %s"), *CurrentSessionName);
        CurrentSessionName.Empty();
    }
}

void UDynamicPerformanceManager::CapturePerformanceData()
{
    // TODO: Implement detailed performance data capture
    // This would write performance metrics to a file for later analysis
}

void UDynamicPerformanceManager::WritePerformanceLog()
{
    // TODO: Implement performance log writing
    // This would save the captured data to a CSV or JSON file
}

void UDynamicPerformanceManager::DumpPerformanceReport()
{
    UE_LOG(LogDynamicPerformance, Log, TEXT("=== Performance Report ==="));
    UE_LOG(LogDynamicPerformance, Log, TEXT("Target: %d, Level: %d, Bottleneck: %d"), 
           (int32)CurrentTarget, (int32)CurrentPerformanceLevel, (int32)CurrentBottleneck);
    UE_LOG(LogDynamicPerformance, Log, TEXT("Frame: %.2fms, Game: %.2fms, Render: %.2fms, GPU: %.2fms"),
           CurrentMetrics.FrameTime, CurrentMetrics.GameThreadTime, CurrentMetrics.RenderThreadTime, CurrentMetrics.GPUTime);
    UE_LOG(LogDynamicPerformance, Log, TEXT("Memory: %.1fMB, Draw Calls: %d, Triangles: %d"),
           CurrentMetrics.UsedMemory, CurrentMetrics.DrawCalls, CurrentMetrics.Triangles);
    UE_LOG(LogDynamicPerformance, Log, TEXT("Mass AI: %d active, %d processed, %d culled"),
           CurrentMetrics.ActiveAgents, CurrentMetrics.ProcessedAgents, CurrentMetrics.CulledAgents);
    UE_LOG(LogDynamicPerformance, Log, TEXT("========================"));
}

void UDynamicPerformanceManager::OverrideMassAISettings(int32 MaxAgents, float TickRate, float CullDistance)
{
    CurrentSettings.MaxActiveAgents = MaxAgents;
    CurrentSettings.AITickRate = TickRate;
    CurrentSettings.DinosaurCullDistance = CullDistance;
    
    // TODO: Apply these settings to the Mass AI system
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Mass AI settings overridden: MaxAgents=%d, TickRate=%.1f, CullDistance=%.1f"),
           MaxAgents, TickRate, CullDistance);
}

void UDynamicPerformanceManager::OverrideRenderingSettings(float ScreenPercentage, int32 ViewDistance, int32 ShadowQuality)
{
    CurrentSettings.ScreenPercentage = ScreenPercentage;
    CurrentSettings.ViewDistanceScale = ViewDistance;
    CurrentSettings.ShadowQuality = ShadowQuality;
    
    // Apply immediately
    ApplyPerformanceSettings(CurrentSettings);
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Rendering settings overridden: ScreenPercentage=%.1f, ViewDistance=%d, ShadowQuality=%d"),
           ScreenPercentage, ViewDistance, ShadowQuality);
}