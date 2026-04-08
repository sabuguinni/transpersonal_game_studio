#include "DynamicPerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Engine/GameViewportClient.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

// UE5 specific includes for performance monitoring
#include "Engine/GameInstance.h"
#include "Subsystems/SubsystemCollection.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogDynamicPerformance, Log, All);

// Console variables for runtime configuration
static TAutoConsoleVariable<bool> CVarDynamicPerfEnabled(
    TEXT("r.DynamicPerformance.Enabled"),
    true,
    TEXT("Enable dynamic performance management"),
    ECVF_Scalability
);

static TAutoConsoleVariable<float> CVarDynamicPerfSensitivity(
    TEXT("r.DynamicPerformance.Sensitivity"),
    1.0f,
    TEXT("Sensitivity of dynamic performance adjustments (0.1 - 2.0)"),
    ECVF_Scalability
);

static TAutoConsoleVariable<int32> CVarDynamicPerfMaxAgents(
    TEXT("r.DynamicPerformance.MaxMassAgents"),
    50000,
    TEXT("Maximum number of Mass AI agents"),
    ECVF_Scalability
);

UDynamicPerformanceManager::UDynamicPerformanceManager()
{
    // Initialize default settings
    CurrentTarget = EPerformanceTarget::PC_HighEnd;
    CurrentPerformanceLevel = EPerformanceLevel::High;
    CurrentBottleneck = EPerformanceBottleneck::None;
    
    bDynamicAdjustmentEnabled = true;
    AdjustmentSensitivity = 1.0f;
    PerformanceHistoryWindow = 3.0f;
    AdjustmentCooldown = 2.0f;
    LastAdjustmentTime = 0.0f;
    
    // Initialize performance budget based on target
    CurrentBudget = FPerformanceBudget::GetBudgetForTarget(CurrentTarget);
    
    // Initialize performance settings
    CurrentSettings = FPerformanceSettings::GetSettingsForLevel(CurrentPerformanceLevel);
    
    // Initialize metrics
    CurrentMetrics = FPerformanceMetrics();
    
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
    
    // Detect platform and set appropriate target
    DetectPlatformTarget();
    
    // Start performance monitoring timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MetricsUpdateTimer,
            this,
            &UDynamicPerformanceManager::UpdatePerformanceMetrics,
            0.1f, // Update every 100ms
            true
        );
    }
    
    // Apply initial settings
    ApplyPerformanceSettings(CurrentSettings);
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Performance target set to: %d"), (int32)CurrentTarget);
    UE_LOG(LogDynamicPerformance, Log, TEXT("Initial performance level: %d"), (int32)CurrentPerformanceLevel);
}

void UDynamicPerformanceManager::Deinitialize()
{
    // Clean up timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
    }
    
    // Stop any active performance capture
    if (bCapturingPerformance)
    {
        StopPerformanceCapture();
    }
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Dynamic Performance Manager deinitialized"));
    
    Super::Deinitialize();
}

void UDynamicPerformanceManager::SetPerformanceTarget(EPerformanceTarget Target)
{
    if (CurrentTarget != Target)
    {
        CurrentTarget = Target;
        CurrentBudget = FPerformanceBudget::GetBudgetForTarget(Target);
        
        // Recalculate appropriate performance level for new target
        DetermineOptimalPerformanceLevel();
        
        UE_LOG(LogDynamicPerformance, Log, TEXT("Performance target changed to: %d"), (int32)Target);
    }
}

void UDynamicPerformanceManager::SetPerformanceLevel(EPerformanceLevel Level)
{
    if (CurrentPerformanceLevel != Level)
    {
        EPerformanceLevel OldLevel = CurrentPerformanceLevel;
        CurrentPerformanceLevel = Level;
        
        // Get new settings for this level
        CurrentSettings = FPerformanceSettings::GetSettingsForLevel(Level);
        ValidateSettings(CurrentSettings);
        
        // Apply the new settings
        ApplyPerformanceSettings(CurrentSettings);
        
        // Broadcast change event
        OnPerformanceLevelChanged.Broadcast(Level);
        
        UE_LOG(LogDynamicPerformance, Log, TEXT("Performance level changed from %d to %d"), 
               (int32)OldLevel, (int32)Level);
    }
}

void UDynamicPerformanceManager::UpdatePerformanceMetrics()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UDynamicPerformanceManager::UpdatePerformanceMetrics);
    
    // Get current frame timing
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    
    // Get engine stats
    if (GEngine && GEngine->GetGameViewport())
    {
        // Frame timing
        CurrentMetrics.FrameTime = CurrentFrameTime;
        CurrentMetrics.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
        CurrentMetrics.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
        CurrentMetrics.GPUTime = FPlatformTime::ToMilliseconds(GGPUFrameTime);
        
        // Memory usage
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        CurrentMetrics.UsedMemory = MemStats.UsedPhysical / (1024 * 1024); // Convert to MB
        
        // Rendering stats
        if (GEngine->GetGameViewport()->Viewport)
        {
            // Get render stats from RHI
            CurrentMetrics.DrawCalls = GNumDrawCallsRHI[GRHIThreadId];
            CurrentMetrics.Triangles = GNumPrimitivesDrawnRHI[GRHIThreadId];
        }
    }
    
    // Update performance history
    UpdatePerformanceHistory();
    
    // Analyze current performance state
    AnalyzeBottlenecks();
    
    // Apply dynamic adjustments if enabled
    if (bDynamicAdjustmentEnabled && CVarDynamicPerfEnabled.GetValueOnGameThread())
    {
        ApplyDynamicAdjustments();
    }
    
    // Capture performance data if recording
    if (bCapturingPerformance)
    {
        CapturePerformanceData();
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
    int32 MaxHistorySize = FMath::CeilToInt(PerformanceHistoryWindow * 10.0f); // 10 samples per second
    
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
    
    // Determine primary bottleneck based on budget usage
    float FrameBudget = CurrentBudget.TotalFrameTime;
    float GameBudget = CurrentBudget.GameThreadBudget;
    float RenderBudget = CurrentBudget.RenderThreadBudget;
    float GPUBudget = CurrentBudget.GPUBudget;
    
    float FrameUsage = CurrentMetrics.FrameTime / FrameBudget;
    float GameUsage = CurrentMetrics.GameThreadTime / GameBudget;
    float RenderUsage = CurrentMetrics.RenderThreadTime / RenderBudget;
    float GPUUsage = CurrentMetrics.GPUTime / GPUBudget;
    
    // Find the highest budget usage
    float MaxUsage = FMath::Max({FrameUsage, GameUsage, RenderUsage, GPUUsage});
    
    if (MaxUsage > 1.1f) // 10% over budget
    {
        if (GameUsage == MaxUsage)
        {
            NewBottleneck = EPerformanceBottleneck::GameThread;
        }
        else if (RenderUsage == MaxUsage)
        {
            NewBottleneck = EPerformanceBottleneck::RenderThread;
        }
        else if (GPUUsage == MaxUsage)
        {
            NewBottleneck = EPerformanceBottleneck::GPU;
        }
        
        // Check for specific system bottlenecks
        if (CurrentMetrics.MassAITime > CurrentBudget.MassAIBudget * 1.2f)
        {
            NewBottleneck = EPerformanceBottleneck::MassAI;
        }
        else if (CurrentMetrics.PhysicsTime > CurrentBudget.PhysicsBudget * 1.2f)
        {
            NewBottleneck = EPerformanceBottleneck::Physics;
        }
    }
    
    // Update bottleneck if changed
    if (CurrentBottleneck != NewBottleneck)
    {
        CurrentBottleneck = NewBottleneck;
        OnBottleneckDetected.Broadcast(NewBottleneck);
        
        UE_LOG(LogDynamicPerformance, Warning, TEXT("Performance bottleneck detected: %d"), (int32)NewBottleneck);
    }
}

void UDynamicPerformanceManager::ApplyDynamicAdjustments()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check cooldown period
    if (CurrentTime - LastAdjustmentTime < AdjustmentCooldown)
    {
        return;
    }
    
    // Calculate average performance over history window
    float AvgFrameTime = CalculateAverageFromHistory(FrameTimeHistory);
    float TargetFrameTime = CurrentBudget.TotalFrameTime;
    
    // Determine if adjustment is needed
    bool bNeedsUpgrade = AvgFrameTime < TargetFrameTime * 0.8f; // Running well under budget
    bool bNeedsDowngrade = AvgFrameTime > TargetFrameTime * 1.1f; // Over budget
    
    if (bNeedsDowngrade && CurrentPerformanceLevel > EPerformanceLevel::Potato)
    {
        // Reduce performance level
        EPerformanceLevel NewLevel = (EPerformanceLevel)((int32)CurrentPerformanceLevel + 1);
        SetPerformanceLevel(NewLevel);
        LastAdjustmentTime = CurrentTime;
    }
    else if (bNeedsUpgrade && CurrentPerformanceLevel < EPerformanceLevel::Ultra)
    {
        // Increase performance level
        EPerformanceLevel NewLevel = (EPerformanceLevel)((int32)CurrentPerformanceLevel - 1);
        SetPerformanceLevel(NewLevel);
        LastAdjustmentTime = CurrentTime;
    }
}

float UDynamicPerformanceManager::CalculateAverageFromHistory(const TArray<float>& History) const
{
    if (History.Num() == 0)
    {
        return 0.0f;
    }
    
    float Sum = 0.0f;
    for (float Value : History)
    {
        Sum += Value;
    }
    
    return Sum / History.Num();
}

void UDynamicPerformanceManager::DetectPlatformTarget()
{
    // Detect platform and set appropriate target
    FString PlatformName = FPlatformProperties::PlatformName();
    
    if (PlatformName == TEXT("Windows") || PlatformName == TEXT("Mac") || PlatformName == TEXT("Linux"))
    {
        CurrentTarget = EPerformanceTarget::PC_HighEnd;
    }
    else if (PlatformName == TEXT("PS5") || PlatformName == TEXT("XSX"))
    {
        CurrentTarget = EPerformanceTarget::Console_NextGen;
    }
    else
    {
        CurrentTarget = EPerformanceTarget::Console_Current;
    }
}

void UDynamicPerformanceManager::DetermineOptimalPerformanceLevel()
{
    // Start with high performance and adjust based on initial frame timing
    CurrentPerformanceLevel = EPerformanceLevel::High;
    
    // This will be refined by the dynamic adjustment system
}

void UDynamicPerformanceManager::ApplyPerformanceSettings(const FPerformanceSettings& Settings)
{
    // Apply rendering settings
    if (auto* ScreenPercentageCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage")))
    {
        ScreenPercentageCVar->Set(Settings.ScreenPercentage);
    }
    
    if (auto* ViewDistanceCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale")))
    {
        ViewDistanceCVar->Set(Settings.ViewDistanceScale / 100.0f);
    }
    
    if (auto* ShadowQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality")))
    {
        ShadowQualityCVar->Set(Settings.ShadowQuality);
    }
    
    if (auto* TextureQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.TextureQuality")))
    {
        TextureQualityCVar->Set(Settings.TextureQuality);
    }
    
    if (auto* EffectsQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality")))
    {
        EffectsQualityCVar->Set(Settings.EffectsQuality);
    }
    
    if (auto* PostProcessQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.PostProcessQuality")))
    {
        PostProcessQualityCVar->Set(Settings.PostProcessQuality);
    }
    
    // Apply Nanite settings
    if (auto* NaniteCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite")))
    {
        NaniteCVar->Set(Settings.bNaniteEnabled ? 1 : 0);
    }
    
    // Apply Lumen settings
    if (auto* LumenCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicGlobalIlluminationMethod")))
    {
        LumenCVar->Set(Settings.bLumenEnabled ? 1 : 0);
    }
    
    // Apply Virtual Shadow Maps
    if (auto* VSMCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.Virtual.Enable")))
    {
        VSMCVar->Set(Settings.bVirtualShadowMapsEnabled ? 1 : 0);
    }
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Applied performance settings - Screen%%: %.1f, Shadows: %d, Effects: %d"), 
           Settings.ScreenPercentage, Settings.ShadowQuality, Settings.EffectsQuality);
}

void UDynamicPerformanceManager::ValidateSettings(FPerformanceSettings& Settings)
{
    // Clamp values to valid ranges
    Settings.ScreenPercentage = FMath::Clamp(Settings.ScreenPercentage, 50.0f, 200.0f);
    Settings.ViewDistanceScale = FMath::Clamp(Settings.ViewDistanceScale, 25, 200);
    Settings.ShadowQuality = FMath::Clamp(Settings.ShadowQuality, 0, 3);
    Settings.TextureQuality = FMath::Clamp(Settings.TextureQuality, 0, 3);
    Settings.EffectsQuality = FMath::Clamp(Settings.EffectsQuality, 0, 3);
    Settings.PostProcessQuality = FMath::Clamp(Settings.PostProcessQuality, 0, 3);
    
    Settings.MaxActiveAgents = FMath::Clamp(Settings.MaxActiveAgents, 1000, 100000);
    Settings.AITickRate = FMath::Clamp(Settings.AITickRate, 10.0f, 120.0f);
    Settings.DinosaurCullDistance = FMath::Clamp(Settings.DinosaurCullDistance, 5000.0f, 100000.0f);
}

void UDynamicPerformanceManager::StartPerformanceCapture(const FString& SessionName)
{
    if (!bCapturingPerformance)
    {
        bCapturingPerformance = true;
        CurrentSessionName = SessionName;
        
        UE_LOG(LogDynamicPerformance, Log, TEXT("Started performance capture session: %s"), *SessionName);
    }
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
    // This would capture detailed performance data for analysis
    // Implementation would depend on specific telemetry requirements
}

void UDynamicPerformanceManager::WritePerformanceLog()
{
    // Write performance data to log file
    FString LogData = FString::Printf(
        TEXT("Performance Session: %s\n")
        TEXT("Target: %d, Level: %d\n")
        TEXT("Avg Frame Time: %.2fms\n")
        TEXT("Avg Game Thread: %.2fms\n")
        TEXT("Avg Render Thread: %.2fms\n")
        TEXT("Avg GPU Time: %.2fms\n"),
        *CurrentSessionName,
        (int32)CurrentTarget,
        (int32)CurrentPerformanceLevel,
        CalculateAverageFromHistory(FrameTimeHistory),
        CalculateAverageFromHistory(GameThreadHistory),
        CalculateAverageFromHistory(RenderThreadHistory),
        CalculateAverageFromHistory(GPUTimeHistory)
    );
    
    FString LogPath = FPaths::ProjectLogDir() / FString::Printf(TEXT("Performance_%s.log"), *CurrentSessionName);
    FFileHelper::SaveStringToFile(LogData, *LogPath);
}

void UDynamicPerformanceManager::DumpPerformanceReport()
{
    UE_LOG(LogDynamicPerformance, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Target: %d, Level: %d"), (int32)CurrentTarget, (int32)CurrentPerformanceLevel);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Frame Time: %.2fms (Budget: %.2fms)"), CurrentMetrics.FrameTime, CurrentBudget.TotalFrameTime);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Game Thread: %.2fms (Budget: %.2fms)"), CurrentMetrics.GameThreadTime, CurrentBudget.GameThreadBudget);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Render Thread: %.2fms (Budget: %.2fms)"), CurrentMetrics.RenderThreadTime, CurrentBudget.RenderThreadBudget);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("GPU Time: %.2fms (Budget: %.2fms)"), CurrentMetrics.GPUTime, CurrentBudget.GPUBudget);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Bottleneck: %d"), (int32)CurrentBottleneck);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Draw Calls: %d, Triangles: %d"), CurrentMetrics.DrawCalls, CurrentMetrics.Triangles);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Memory Usage: %.1fMB"), CurrentMetrics.UsedMemory);
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
        Settings.bNaniteEnabled = true;
        Settings.bLumenEnabled = true;
        Settings.bVirtualShadowMapsEnabled = true;
        Settings.MaxActiveAgents = 50000;
        Settings.AITickRate = 60.0f;
        break;
        
    case EPerformanceLevel::High:
        Settings.ScreenPercentage = 90.0f;
        Settings.ViewDistanceScale = 90;
        Settings.ShadowQuality = 3;
        Settings.TextureQuality = 3;
        Settings.EffectsQuality = 2;
        Settings.PostProcessQuality = 3;
        Settings.MaxActiveAgents = 40000;
        Settings.AITickRate = 45.0f;
        break;
        
    case EPerformanceLevel::Medium:
        Settings.ScreenPercentage = 80.0f;
        Settings.ViewDistanceScale = 80;
        Settings.ShadowQuality = 2;
        Settings.TextureQuality = 2;
        Settings.EffectsQuality = 2;
        Settings.PostProcessQuality = 2;
        Settings.MaxActiveAgents = 30000;
        Settings.AITickRate = 30.0f;
        break;
        
    case EPerformanceLevel::Low:
        Settings.ScreenPercentage = 70.0f;
        Settings.ViewDistanceScale = 70;
        Settings.ShadowQuality = 1;
        Settings.TextureQuality = 1;
        Settings.EffectsQuality = 1;
        Settings.PostProcessQuality = 1;
        Settings.bLumenEnabled = false;
        Settings.MaxActiveAgents = 20000;
        Settings.AITickRate = 20.0f;
        break;
        
    case EPerformanceLevel::Potato:
        Settings.ScreenPercentage = 60.0f;
        Settings.ViewDistanceScale = 50;
        Settings.ShadowQuality = 0;
        Settings.TextureQuality = 0;
        Settings.EffectsQuality = 0;
        Settings.PostProcessQuality = 0;
        Settings.bNaniteEnabled = false;
        Settings.bLumenEnabled = false;
        Settings.bVirtualShadowMapsEnabled = false;
        Settings.MaxActiveAgents = 10000;
        Settings.AITickRate = 15.0f;
        break;
    }
    
    return Settings;
}