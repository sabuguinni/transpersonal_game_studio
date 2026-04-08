#include "DynamicPerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "MassEntitySubsystem.h"
#include "Landscape.h"
#include "GameFramework/GameUserSettings.h"
#include "RHI.h"
#include "Engine/Console.h"

DEFINE_LOG_CATEGORY_STATIC(LogDynamicPerformance, Log, All);

UDynamicPerformanceManager::UDynamicPerformanceManager()
{
    CurrentTarget = EPerformanceTarget::PC_HighEnd;
    CurrentPerformanceLevel = EPerformanceLevel::High;
    CurrentBottleneck = EPerformanceBottleneck::None;
    bDynamicAdjustmentEnabled = true;
    AdjustmentSensitivity = 1.0f;
    PerformanceHistoryWindow = 3.0f;
    LastAdjustmentTime = 0.0f;
    AdjustmentCooldown = 2.0f;
    bCapturingPerformance = false;
    
    // Initialize performance history arrays
    FrameTimeHistory.Reserve(180); // 3 seconds at 60fps
    GameThreadHistory.Reserve(180);
    RenderThreadHistory.Reserve(180);
    GPUTimeHistory.Reserve(180);
}

void UDynamicPerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Dynamic Performance Manager initialized"));
    
    // Set up performance budget based on current target
    switch (CurrentTarget)
    {
        case EPerformanceTarget::PC_HighEnd:
            CurrentBudget.TargetFrameTime = 16.67f; // 60fps
            CurrentBudget.GameThreadBudget = 8.0f;
            CurrentBudget.RenderThreadBudget = 12.0f;
            CurrentBudget.GPUBudget = 14.0f;
            break;
        case EPerformanceTarget::PC_MidRange:
            CurrentBudget.TargetFrameTime = 20.0f; // 50fps
            CurrentBudget.GameThreadBudget = 10.0f;
            CurrentBudget.RenderThreadBudget = 15.0f;
            CurrentBudget.GPUBudget = 18.0f;
            break;
        case EPerformanceTarget::Console_HighEnd:
            CurrentBudget.TargetFrameTime = 33.33f; // 30fps
            CurrentBudget.GameThreadBudget = 16.0f;
            CurrentBudget.RenderThreadBudget = 25.0f;
            CurrentBudget.GPUBudget = 30.0f;
            break;
        case EPerformanceTarget::Console_Standard:
            CurrentBudget.TargetFrameTime = 33.33f; // 30fps
            CurrentBudget.GameThreadBudget = 18.0f;
            CurrentBudget.RenderThreadBudget = 28.0f;
            CurrentBudget.GPUBudget = 32.0f;
            break;
    }
    
    // Start metrics update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MetricsUpdateTimer, 
            FTimerDelegate::CreateUObject(this, &UDynamicPerformanceManager::UpdatePerformanceMetrics),
            0.1f, true); // Update 10 times per second
    }
    
    // Apply initial settings
    CurrentSettings = FPerformanceSettings::GetSettingsForLevel(CurrentPerformanceLevel);
    ApplyPerformanceSettings(CurrentSettings);
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
        
        // Update budget for new target
        Initialize(FSubsystemCollectionBase::GetTypeHash(this));
        
        UE_LOG(LogDynamicPerformance, Log, TEXT("Performance target changed to: %d"), (int32)Target);
    }
}

void UDynamicPerformanceManager::SetPerformanceLevel(EPerformanceLevel Level)
{
    if (CurrentPerformanceLevel != Level)
    {
        EPerformanceLevel OldLevel = CurrentPerformanceLevel;
        CurrentPerformanceLevel = Level;
        
        // Update settings for new level
        CurrentSettings = FPerformanceSettings::GetSettingsForLevel(Level);
        ApplyPerformanceSettings(CurrentSettings);
        
        // Broadcast level change
        OnPerformanceLevelChanged.Broadcast(Level);
        
        UE_LOG(LogDynamicPerformance, Log, TEXT("Performance level changed from %d to %d"), 
            (int32)OldLevel, (int32)Level);
    }
}

void UDynamicPerformanceManager::UpdatePerformanceMetrics()
{
    // Get current frame stats
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    
    // Get thread times from stats system
    CurrentMetrics.FrameTime = CurrentFrameTime;
    CurrentMetrics.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
    CurrentMetrics.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    CurrentMetrics.GPUTime = FPlatformTime::ToMilliseconds(GGPUFrameTime);
    
    // Get memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.UsedMemory = MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
    
    // Get rendering stats
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentMetrics.DrawCalls = GET_DWORD_STAT(STAT_RHIDrawPrimitiveCalls);
        CurrentMetrics.Triangles = GET_DWORD_STAT(STAT_RHITriangles);
        CurrentMetrics.VisiblePrimitives = GET_DWORD_STAT(STAT_StaticMeshTriangles);
    }
    
    // Get Mass AI stats if available
    if (UWorld* World = GetWorld())
    {
        if (UMassEntitySubsystem* MassSubsystem = World->GetSubsystem<UMassEntitySubsystem>())
        {
            // Get Mass AI metrics (simplified for now)
            CurrentMetrics.ActiveAgents = 0; // Would need actual Mass AI integration
            CurrentMetrics.ProcessedAgents = 0;
            CurrentMetrics.CulledAgents = 0;
        }
    }
    
    // Update performance history
    FrameTimeHistory.Add(CurrentFrameTime);
    GameThreadHistory.Add(CurrentMetrics.GameThreadTime);
    RenderThreadHistory.Add(CurrentMetrics.RenderThreadTime);
    GPUTimeHistory.Add(CurrentMetrics.GPUTime);
    
    // Maintain history window size
    int32 MaxHistorySize = FMath::CeilToInt(PerformanceHistoryWindow * 10.0f); // 10 updates per second
    if (FrameTimeHistory.Num() > MaxHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
        GameThreadHistory.RemoveAt(0);
        RenderThreadHistory.RemoveAt(0);
        GPUTimeHistory.RemoveAt(0);
    }
    
    // Analyze performance and apply adjustments
    AnalyzeBottlenecks();
    
    if (bDynamicAdjustmentEnabled)
    {
        ApplyDynamicAdjustments();
    }
    
    // Capture performance data if recording
    if (bCapturingPerformance)
    {
        CapturePerformanceData();
    }
}

void UDynamicPerformanceManager::AnalyzeBottlenecks()
{
    EPerformanceBottleneck NewBottleneck = EPerformanceBottleneck::None;
    
    // Calculate average times over recent history
    float AvgFrameTime = 0.0f;
    float AvgGameTime = 0.0f;
    float AvgRenderTime = 0.0f;
    float AvgGPUTime = 0.0f;
    
    if (FrameTimeHistory.Num() > 0)
    {
        for (int32 i = 0; i < FrameTimeHistory.Num(); ++i)
        {
            AvgFrameTime += FrameTimeHistory[i];
            AvgGameTime += GameThreadHistory[i];
            AvgRenderTime += RenderThreadHistory[i];
            AvgGPUTime += GPUTimeHistory[i];
        }
        
        int32 NumSamples = FrameTimeHistory.Num();
        AvgFrameTime /= NumSamples;
        AvgGameTime /= NumSamples;
        AvgRenderTime /= NumSamples;
        AvgGPUTime /= NumSamples;
    }
    
    // Check if we're over budget
    if (AvgFrameTime > CurrentBudget.TargetFrameTime * 1.1f) // 10% tolerance
    {
        // Determine primary bottleneck
        if (AvgGameTime > CurrentBudget.GameThreadBudget)
        {
            NewBottleneck = EPerformanceBottleneck::GameThread;
        }
        else if (AvgRenderTime > CurrentBudget.RenderThreadBudget)
        {
            NewBottleneck = EPerformanceBottleneck::RenderThread;
        }
        else if (AvgGPUTime > CurrentBudget.GPUBudget)
        {
            NewBottleneck = EPerformanceBottleneck::GPU;
        }
        else if (CurrentMetrics.UsedMemory > 6000.0f) // 6GB threshold
        {
            NewBottleneck = EPerformanceBottleneck::Memory;
        }
    }
    
    // Update bottleneck if changed
    if (NewBottleneck != CurrentBottleneck)
    {
        CurrentBottleneck = NewBottleneck;
        OnBottleneckDetected.Broadcast(NewBottleneck);
        
        UE_LOG(LogDynamicPerformance, Warning, TEXT("Performance bottleneck detected: %d"), (int32)NewBottleneck);
    }
}

void UDynamicPerformanceManager::ApplyDynamicAdjustments()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check cooldown
    if (CurrentTime - LastAdjustmentTime < AdjustmentCooldown)
    {
        return;
    }
    
    // Calculate performance ratio
    float PerformanceRatio = CurrentMetrics.FrameTime / CurrentBudget.TargetFrameTime;
    
    // Determine if adjustment is needed
    bool bNeedAdjustment = false;
    EPerformanceLevel TargetLevel = CurrentPerformanceLevel;
    
    if (PerformanceRatio > 1.2f) // 20% over budget
    {
        // Need to reduce quality
        if (CurrentPerformanceLevel > EPerformanceLevel::Potato)
        {
            TargetLevel = (EPerformanceLevel)((int32)CurrentPerformanceLevel + 1);
            bNeedAdjustment = true;
        }
    }
    else if (PerformanceRatio < 0.8f) // 20% under budget
    {
        // Can increase quality
        if (CurrentPerformanceLevel < EPerformanceLevel::Ultra)
        {
            TargetLevel = (EPerformanceLevel)((int32)CurrentPerformanceLevel - 1);
            bNeedAdjustment = true;
        }
    }
    
    if (bNeedAdjustment)
    {
        SetPerformanceLevel(TargetLevel);
        LastAdjustmentTime = CurrentTime;
    }
}

void UDynamicPerformanceManager::ApplyPerformanceSettings(const FPerformanceSettings& Settings)
{
    // Apply rendering settings via console variables
    if (GEngine && GEngine->GameViewport && GEngine->GameViewport->GetGameViewport())
    {
        // Screen percentage
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.ScreenPercentage %f"), Settings.ScreenPercentage));
        
        // View distance
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.ViewDistanceScale %f"), Settings.ViewDistanceScale / 100.0f));
        
        // Shadow quality
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("sg.ShadowQuality %d"), Settings.ShadowQuality));
        
        // Texture quality
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("sg.TextureQuality %d"), Settings.TextureQuality));
        
        // Effects quality
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("sg.EffectsQuality %d"), Settings.EffectsQuality));
        
        // Post process quality
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("sg.PostProcessQuality %d"), Settings.PostProcessQuality));
        
        // Nanite
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.Nanite %d"), Settings.bNaniteEnabled ? 1 : 0));
        
        // Lumen
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.DynamicGlobalIlluminationMethod %d"), Settings.bLumenEnabled ? 1 : 0));
        
        // Virtual Shadow Maps
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.Shadow.Virtual.Enable %d"), Settings.bVirtualShadowMapsEnabled ? 1 : 0));
    }
    
    UE_LOG(LogDynamicPerformance, Log, TEXT("Applied performance settings for level %d"), (int32)CurrentPerformanceLevel);
}

void UDynamicPerformanceManager::StartPerformanceCapture(const FString& SessionName)
{
    if (!bCapturingPerformance)
    {
        bCapturingPerformance = true;
        CurrentSessionName = SessionName;
        
        UE_LOG(LogDynamicPerformance, Log, TEXT("Started performance capture: %s"), *SessionName);
    }
}

void UDynamicPerformanceManager::StopPerformanceCapture()
{
    if (bCapturingPerformance)
    {
        bCapturingPerformance = false;
        WritePerformanceLog();
        
        UE_LOG(LogDynamicPerformance, Log, TEXT("Stopped performance capture: %s"), *CurrentSessionName);
        CurrentSessionName.Empty();
    }
}

void UDynamicPerformanceManager::CapturePerformanceData()
{
    // Implementation for capturing detailed performance data
    // This would write to a structured log file for analysis
}

void UDynamicPerformanceManager::WritePerformanceLog()
{
    // Implementation for writing performance log to file
    FString LogPath = FPaths::ProjectLogDir() / TEXT("Performance") / (CurrentSessionName + TEXT(".json"));
    
    // Create performance report JSON
    FString JsonContent = TEXT("{\n");
    JsonContent += FString::Printf(TEXT("  \"session\": \"%s\",\n"), *CurrentSessionName);
    JsonContent += FString::Printf(TEXT("  \"timestamp\": \"%s\",\n"), *FDateTime::Now().ToString());
    JsonContent += FString::Printf(TEXT("  \"target\": %d,\n"), (int32)CurrentTarget);
    JsonContent += FString::Printf(TEXT("  \"final_level\": %d,\n"), (int32)CurrentPerformanceLevel);
    JsonContent += FString::Printf(TEXT("  \"avg_frame_time\": %.2f,\n"), CurrentMetrics.FrameTime);
    JsonContent += FString::Printf(TEXT("  \"avg_game_thread\": %.2f,\n"), CurrentMetrics.GameThreadTime);
    JsonContent += FString::Printf(TEXT("  \"avg_render_thread\": %.2f,\n"), CurrentMetrics.RenderThreadTime);
    JsonContent += FString::Printf(TEXT("  \"avg_gpu_time\": %.2f\n"), CurrentMetrics.GPUTime);
    JsonContent += TEXT("}");
    
    FFileHelper::SaveStringToFile(JsonContent, *LogPath);
}

void UDynamicPerformanceManager::DumpPerformanceReport()
{
    UE_LOG(LogDynamicPerformance, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Target: %d, Level: %d, Bottleneck: %d"), 
        (int32)CurrentTarget, (int32)CurrentPerformanceLevel, (int32)CurrentBottleneck);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Frame: %.2fms, Game: %.2fms, Render: %.2fms, GPU: %.2fms"),
        CurrentMetrics.FrameTime, CurrentMetrics.GameThreadTime, CurrentMetrics.RenderThreadTime, CurrentMetrics.GPUTime);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("Memory: %.1fMB, Draw Calls: %d, Triangles: %d"),
        CurrentMetrics.UsedMemory, CurrentMetrics.DrawCalls, CurrentMetrics.Triangles);
    UE_LOG(LogDynamicPerformance, Warning, TEXT("========================="));
}

// Static method implementation
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
            Settings.TextureQuality = 2;
            Settings.EffectsQuality = 2;
            Settings.PostProcessQuality = 2;
            Settings.bNaniteEnabled = true;
            Settings.bLumenEnabled = true;
            Settings.bVirtualShadowMapsEnabled = true;
            Settings.MaxActiveAgents = 40000;
            Settings.AITickRate = 45.0f;
            break;
            
        case EPerformanceLevel::Medium:
            Settings.ScreenPercentage = 80.0f;
            Settings.ViewDistanceScale = 75;
            Settings.ShadowQuality = 2;
            Settings.TextureQuality = 2;
            Settings.EffectsQuality = 1;
            Settings.PostProcessQuality = 1;
            Settings.bNaniteEnabled = true;
            Settings.bLumenEnabled = false;
            Settings.bVirtualShadowMapsEnabled = false;
            Settings.MaxActiveAgents = 25000;
            Settings.AITickRate = 30.0f;
            break;
            
        case EPerformanceLevel::Low:
            Settings.ScreenPercentage = 70.0f;
            Settings.ViewDistanceScale = 60;
            Settings.ShadowQuality = 1;
            Settings.TextureQuality = 1;
            Settings.EffectsQuality = 0;
            Settings.PostProcessQuality = 0;
            Settings.bNaniteEnabled = false;
            Settings.bLumenEnabled = false;
            Settings.bVirtualShadowMapsEnabled = false;
            Settings.MaxActiveAgents = 15000;
            Settings.AITickRate = 20.0f;
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
            Settings.MaxActiveAgents = 5000;
            Settings.AITickRate = 15.0f;
            break;
    }
    
    return Settings;
}