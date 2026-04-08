#include "DynamicPerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "RenderingThread.h"
#include "Rendering/SlateRenderer.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameUserSettings.h"
#include "AudioDevice.h"
#include "Sound/SoundClass.h"

DEFINE_LOG_CATEGORY_STATIC(LogDynamicPerformanceOptimizer, Log, All);

// Console variables for runtime tuning
static TAutoConsoleVariable<bool> CVarAutoOptimizationEnabled(
    TEXT("perf.AutoOptimizationEnabled"),
    true,
    TEXT("Enable/disable automatic performance optimization"),
    ECVF_Default
);

static TAutoConsoleVariable<float> CVarOptimizationSensitivity(
    TEXT("perf.OptimizationSensitivity"),
    1.0f,
    TEXT("Sensitivity multiplier for optimization triggers (0.5 = less aggressive, 2.0 = more aggressive)"),
    ECVF_Default
);

static TAutoConsoleVariable<bool> CVarShowOptimizationDebug(
    TEXT("perf.ShowOptimizationDebug"),
    false,
    TEXT("Show debug information for performance optimization"),
    ECVF_Default
);

UDynamicPerformanceOptimizer::UDynamicPerformanceOptimizer()
{
    PerformanceProfiler = nullptr;
    TargetPlatform = EPerformanceTarget::PC_High;
    bAutoOptimizationEnabled = true;
    bShowDebugStats = false;
    
    CurrentState = FPerformanceState();
    CurrentSettings = FOptimizationSettings();
    
    FrameTimeHistory.Reserve(MAX_FRAME_HISTORY);
}

void UDynamicPerformanceOptimizer::Initialize(UPerformanceProfiler* InProfiler, EPerformanceTarget InTarget)
{
    PerformanceProfiler = InProfiler;
    TargetPlatform = InTarget;
    
    // Initialize settings based on target platform
    CurrentSettings = FOptimizationSettings::GetSettingsForLevel(EOptimizationLevel::Ultra);
    
    // Apply initial settings
    ApplyOptimizationSettings();
    
    UE_LOG(LogDynamicPerformanceOptimizer, Log, TEXT("Dynamic Performance Optimizer initialized for target: %s"), 
           *UEnum::GetValueAsString(InTarget));
}

void UDynamicPerformanceOptimizer::UpdateOptimization(float DeltaTime)
{
    if (!PerformanceProfiler || !bAutoOptimizationEnabled)
    {
        return;
    }

    // Update console variable states
    bAutoOptimizationEnabled = CVarAutoOptimizationEnabled.GetValueOnGameThread();
    bShowDebugStats = CVarShowOptimizationDebug.GetValueOnGameThread();

    // Get current performance metrics
    FPerformanceMetrics Metrics = PerformanceProfiler->GetCurrentMetrics();
    
    // Update frame time history
    UpdateFrameTimeHistory(Metrics.FrameTime);
    
    // Analyze performance and determine if optimization is needed
    AnalyzePerformance(Metrics);
    
    // Update state timers
    CurrentState.TimeInCurrentLevel += DeltaTime;
    PerformanceStabilityTimer += DeltaTime;
    
    // Reset optimization changes counter
    CurrentState.OptimizationChangesThisFrame = 0;
    
    // Show debug stats if enabled
    if (bShowDebugStats)
    {
        ShowDebugStats(Metrics);
    }
}

void UDynamicPerformanceOptimizer::AnalyzePerformance(const FPerformanceMetrics& Metrics)
{
    float TargetFrameTime = PerformanceProfiler->GetTargetFrameTime();
    float CurrentFrameTime = AverageFrameTime;
    float PerformanceRatio = CurrentFrameTime / TargetFrameTime;
    
    // Update performance health (0.0 = terrible, 1.0 = perfect)
    CurrentState.PerformanceHealth = FMath::Clamp(2.0f - PerformanceRatio, 0.0f, 1.0f);
    
    // Apply sensitivity multiplier
    float Sensitivity = CVarOptimizationSensitivity.GetValueOnGameThread();
    PerformanceRatio *= Sensitivity;
    
    // Determine if we need to change optimization level
    bool bNeedsOptimization = false;
    EOptimizationLevel TargetLevel = CurrentState.CurrentLevel;
    
    // Check for critical performance issues
    if (PerformanceRatio >= PERFORMANCE_CRITICAL_THRESHOLD)
    {
        // Immediate emergency optimization
        TargetLevel = EOptimizationLevel::Emergency;
        bNeedsOptimization = true;
        
        OnPerformanceCritical.Broadcast(CurrentFrameTime, TargetFrameTime, TEXT("Critical Performance"));
        UE_LOG(LogDynamicPerformanceOptimizer, Warning, TEXT("Critical performance detected! Frame time: %.2fms, Target: %.2fms"), 
               CurrentFrameTime, TargetFrameTime);
    }
    else if (PerformanceRatio >= PERFORMANCE_BAD_THRESHOLD)
    {
        ConsecutiveBadFrames++;
        ConsecutiveGoodFrames = 0;
        
        if (ConsecutiveBadFrames >= CONSECUTIVE_FRAMES_THRESHOLD && 
            PerformanceStabilityTimer >= STABILITY_REQUIRED_TIME)
        {
            // Downgrade optimization level
            TargetLevel = GetLowerOptimizationLevel(CurrentState.CurrentLevel);
            bNeedsOptimization = true;
        }
    }
    else if (PerformanceRatio <= PERFORMANCE_GOOD_THRESHOLD)
    {
        ConsecutiveGoodFrames++;
        ConsecutiveBadFrames = 0;
        
        if (ConsecutiveGoodFrames >= CONSECUTIVE_FRAMES_THRESHOLD * 2 && // Be more conservative about upgrading
            PerformanceStabilityTimer >= STABILITY_REQUIRED_TIME * 2.0f)
        {
            // Upgrade optimization level
            TargetLevel = GetHigherOptimizationLevel(CurrentState.CurrentLevel);
            bNeedsOptimization = true;
        }
    }
    else
    {
        // Performance is acceptable, reset counters
        ConsecutiveBadFrames = 0;
        ConsecutiveGoodFrames = 0;
    }
    
    // Apply optimization level change if needed
    if (bNeedsOptimization && TargetLevel != CurrentState.CurrentLevel)
    {
        SetOptimizationLevel(TargetLevel, true);
    }
    
    // Update stability flag
    CurrentState.bIsStable = (ConsecutiveBadFrames == 0 && ConsecutiveGoodFrames < CONSECUTIVE_FRAMES_THRESHOLD);
}

void UDynamicPerformanceOptimizer::SetOptimizationLevel(EOptimizationLevel NewLevel, bool bForceApply)
{
    if (!bForceApply && NewLevel == CurrentState.CurrentLevel)
    {
        return;
    }
    
    EOptimizationLevel OldLevel = CurrentState.CurrentLevel;
    CurrentState.CurrentLevel = NewLevel;
    CurrentState.TimeInCurrentLevel = 0.0f;
    CurrentState.OptimizationChangesThisFrame++;
    
    // Get new settings for this level
    CurrentSettings = FOptimizationSettings::GetSettingsForLevel(NewLevel);
    
    // Apply the new settings
    ApplyOptimizationSettings();
    
    // Reset stability timer
    PerformanceStabilityTimer = 0.0f;
    ConsecutiveBadFrames = 0;
    ConsecutiveGoodFrames = 0;
    
    // Broadcast event
    OnOptimizationLevelChanged.Broadcast(OldLevel, NewLevel);
    
    UE_LOG(LogDynamicPerformanceOptimizer, Log, TEXT("Optimization level changed from %s to %s"), 
           *UEnum::GetValueAsString(OldLevel), *UEnum::GetValueAsString(NewLevel));
}

void UDynamicPerformanceOptimizer::ApplyOptimizationSettings()
{
    // Apply rendering optimizations
    OptimizeRendering(CurrentState.CurrentLevel);
    
    // Apply Mass AI optimizations
    OptimizeMassAI(CurrentState.CurrentLevel);
    
    // Apply physics optimizations
    OptimizePhysics(CurrentState.CurrentLevel);
    
    // Apply streaming optimizations
    OptimizeStreaming(CurrentState.CurrentLevel);
    
    // Apply audio optimizations
    OptimizeAudio(CurrentState.CurrentLevel);
}

void UDynamicPerformanceOptimizer::OptimizeRendering(EOptimizationLevel Level)
{
    UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
    if (!Settings)
    {
        return;
    }
    
    // Apply screen percentage
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetEngineShowFlags()->ScreenPercentage = CurrentSettings.ScreenPercentage;
    }
    
    // Apply quality settings
    Settings->SetShadowQuality(CurrentSettings.ShadowQuality);
    Settings->SetPostProcessingQuality(CurrentSettings.PostProcessQuality);
    Settings->SetVisualEffectQuality(CurrentSettings.EffectsQuality);
    Settings->SetTextureQuality(CurrentSettings.TextureQuality);
    Settings->SetViewDistanceQuality(FMath::RoundToInt(CurrentSettings.ViewDistanceScale * 3));
    
    // Apply Nanite settings
    static IConsoleVariable* NaniteTriangleDensityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite.MaxPixelsPerTriangle"));
    if (NaniteTriangleDensityCVar)
    {
        NaniteTriangleDensityCVar->Set(CurrentSettings.NaniteMaxPixelsPerTriangle);
    }
    
    // Apply Lumen settings
    static IConsoleVariable* LumenSceneViewDistanceCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.LumenScene.DirectLighting.ViewDistance"));
    if (LumenSceneViewDistanceCVar)
    {
        LumenSceneViewDistanceCVar->Set(CurrentSettings.LumenSceneViewDistance);
    }
    
    static IConsoleVariable* LumenReflectionQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.Reflections.Quality"));
    if (LumenReflectionQualityCVar)
    {
        LumenReflectionQualityCVar->Set(CurrentSettings.LumenReflectionQuality);
    }
    
    static IConsoleVariable* LumenGIQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.GlobalIllumination.Quality"));
    if (LumenGIQualityCVar)
    {
        LumenGIQualityCVar->Set(CurrentSettings.LumenGlobalIlluminationQuality);
    }
    
    UE_LOG(LogDynamicPerformanceOptimizer, Verbose, TEXT("Applied rendering optimizations for level: %s"), 
           *UEnum::GetValueAsString(Level));
}

void UDynamicPerformanceOptimizer::OptimizeMassAI(EOptimizationLevel Level)
{
    // Apply Mass AI optimizations (critical for dinosaur ecosystem)
    static IConsoleVariable* MaxMassAgentsCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("mass.MaxAgents"));
    if (MaxMassAgentsCVar)
    {
        MaxMassAgentsCVar->Set(CurrentSettings.MaxActiveMassAgents);
    }
    
    static IConsoleVariable* MassUpdateFrequencyCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("mass.UpdateFrequency"));
    if (MassUpdateFrequencyCVar)
    {
        MassUpdateFrequencyCVar->Set(CurrentSettings.MassAIUpdateFrequency);
    }
    
    // Dinosaur-specific behavior complexity scaling
    static IConsoleVariable* DinosaurBehaviorComplexityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("dino.BehaviorComplexity"));
    if (DinosaurBehaviorComplexityCVar)
    {
        DinosaurBehaviorComplexityCVar->Set(CurrentSettings.DinosaurBehaviorComplexity);
    }
    
    static IConsoleVariable* PerceptionRangeCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("ai.PerceptionRange"));
    if (PerceptionRangeCVar)
    {
        PerceptionRangeCVar->Set(CurrentSettings.PerceptionRange);
    }
    
    UE_LOG(LogDynamicPerformanceOptimizer, Verbose, TEXT("Applied Mass AI optimizations: MaxAgents=%d, UpdateFreq=%.1f"), 
           CurrentSettings.MaxActiveMassAgents, CurrentSettings.MassAIUpdateFrequency);
}

void UDynamicPerformanceOptimizer::OptimizePhysics(EOptimizationLevel Level)
{
    // Apply physics optimizations
    static IConsoleVariable* PhysicsTickRateCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.FixedFrameRate"));
    if (PhysicsTickRateCVar)
    {
        PhysicsTickRateCVar->Set(CurrentSettings.PhysicsTickRate);
    }
    
    static IConsoleVariable* MaxPhysicsBodiesCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxPhysicsBodies"));
    if (MaxPhysicsBodiesCVar)
    {
        MaxPhysicsBodiesCVar->Set(CurrentSettings.MaxPhysicsBodies);
    }
    
    static IConsoleVariable* RagdollCullDistanceCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.RagdollCullDistance"));
    if (RagdollCullDistanceCVar)
    {
        RagdollCullDistanceCVar->Set(CurrentSettings.RagdollCullDistance);
    }
    
    UE_LOG(LogDynamicPerformanceOptimizer, Verbose, TEXT("Applied physics optimizations for level: %s"), 
           *UEnum::GetValueAsString(Level));
}

void UDynamicPerformanceOptimizer::OptimizeStreaming(EOptimizationLevel Level)
{
    // Apply streaming optimizations
    static IConsoleVariable* MaxStreamingCellsCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("wp.Runtime.MaxLoadingCells"));
    if (MaxStreamingCellsCVar)
    {
        MaxStreamingCellsCVar->Set(CurrentSettings.MaxStreamingCells);
    }
    
    static IConsoleVariable* TextureStreamingPoolCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize"));
    if (TextureStreamingPoolCVar)
    {
        TextureStreamingPoolCVar->Set(CurrentSettings.TextureStreamingPoolSize);
    }
    
    static IConsoleVariable* StreamingDistanceCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale"));
    if (StreamingDistanceCVar)
    {
        StreamingDistanceCVar->Set(CurrentSettings.StreamingDistanceMultiplier);
    }
    
    UE_LOG(LogDynamicPerformanceOptimizer, Verbose, TEXT("Applied streaming optimizations for level: %s"), 
           *UEnum::GetValueAsString(Level));
}

void UDynamicPerformanceOptimizer::OptimizeAudio(EOptimizationLevel Level)
{
    // Apply audio optimizations
    if (GEngine && GEngine->GetMainAudioDevice())
    {
        FAudioDevice* AudioDevice = GEngine->GetMainAudioDevice();
        
        // Set max audio sources
        static IConsoleVariable* MaxAudioSourcesCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("au.MaxChannels"));
        if (MaxAudioSourcesCVar)
        {
            MaxAudioSourcesCVar->Set(CurrentSettings.MaxAudioSources);
        }
        
        static IConsoleVariable* Max3DAudioSourcesCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("au.Max3DChannels"));
        if (Max3DAudioSourcesCVar)
        {
            Max3DAudioSourcesCVar->Set(CurrentSettings.Max3DAudioSources);
        }
        
        static IConsoleVariable* AudioCullDistanceCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("au.CullDistance"));
        if (AudioCullDistanceCVar)
        {
            AudioCullDistanceCVar->Set(CurrentSettings.AudioCullDistance);
        }
    }
    
    UE_LOG(LogDynamicPerformanceOptimizer, Verbose, TEXT("Applied audio optimizations for level: %s"), 
           *UEnum::GetValueAsString(Level));
}

void UDynamicPerformanceOptimizer::UpdateFrameTimeHistory(float FrameTime)
{
    FrameTimeHistory.Add(FrameTime);
    
    if (FrameTimeHistory.Num() > MAX_FRAME_HISTORY)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate rolling average
    if (FrameTimeHistory.Num() > 0)
    {
        float Sum = 0.0f;
        for (float Time : FrameTimeHistory)
        {
            Sum += Time;
        }
        AverageFrameTime = Sum / FrameTimeHistory.Num();
    }
}

EOptimizationLevel UDynamicPerformanceOptimizer::GetLowerOptimizationLevel(EOptimizationLevel CurrentLevel)
{
    switch (CurrentLevel)
    {
        case EOptimizationLevel::Ultra:
            return EOptimizationLevel::High;
        case EOptimizationLevel::High:
            return EOptimizationLevel::Medium;
        case EOptimizationLevel::Medium:
            return EOptimizationLevel::Low;
        case EOptimizationLevel::Low:
            return EOptimizationLevel::Emergency;
        case EOptimizationLevel::Emergency:
        default:
            return EOptimizationLevel::Emergency;
    }
}

EOptimizationLevel UDynamicPerformanceOptimizer::GetHigherOptimizationLevel(EOptimizationLevel CurrentLevel)
{
    switch (CurrentLevel)
    {
        case EOptimizationLevel::Emergency:
            return EOptimizationLevel::Low;
        case EOptimizationLevel::Low:
            return EOptimizationLevel::Medium;
        case EOptimizationLevel::Medium:
            return EOptimizationLevel::High;
        case EOptimizationLevel::High:
            return EOptimizationLevel::Ultra;
        case EOptimizationLevel::Ultra:
        default:
            return EOptimizationLevel::Ultra;
    }
}

void UDynamicPerformanceOptimizer::ShowDebugStats(const FPerformanceMetrics& Metrics)
{
    if (GEngine)
    {
        FString DebugString = FString::Printf(
            TEXT("Performance Optimizer Debug:\n")
            TEXT("Level: %s | Health: %.2f | Stable: %s\n")
            TEXT("Frame Time: %.2fms (Avg: %.2fms) | Target: %.2fms\n")
            TEXT("Bad Frames: %d | Good Frames: %d\n")
            TEXT("Mass Agents: %d | Physics Bodies: %d\n")
            TEXT("Screen %%: %.1f | Shadow Quality: %d"),
            *UEnum::GetValueAsString(CurrentState.CurrentLevel),
            CurrentState.PerformanceHealth,
            CurrentState.bIsStable ? TEXT("Yes") : TEXT("No"),
            Metrics.FrameTime,
            AverageFrameTime,
            PerformanceProfiler ? PerformanceProfiler->GetTargetFrameTime() : 16.67f,
            ConsecutiveBadFrames,
            ConsecutiveGoodFrames,
            CurrentSettings.MaxActiveMassAgents,
            CurrentSettings.MaxPhysicsBodies,
            CurrentSettings.ScreenPercentage,
            CurrentSettings.ShadowQuality
        );
        
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, DebugString);
    }
}

void UDynamicPerformanceOptimizer::SetAutoOptimizationEnabled(bool bEnabled)
{
    bAutoOptimizationEnabled = bEnabled;
    CVarAutoOptimizationEnabled->Set(bEnabled);
    
    UE_LOG(LogDynamicPerformanceOptimizer, Log, TEXT("Auto optimization %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

// Console command implementations
void UDynamicPerformanceOptimizer::SetOptimizationLevelExec(int32 Level)
{
    if (Level >= 0 && Level <= 4)
    {
        SetOptimizationLevel(static_cast<EOptimizationLevel>(Level), true);
    }
    else
    {
        UE_LOG(LogDynamicPerformanceOptimizer, Warning, TEXT("Invalid optimization level: %d (valid range: 0-4)"), Level);
    }
}

void UDynamicPerformanceOptimizer::ToggleAutoOptimization()
{
    SetAutoOptimizationEnabled(!bAutoOptimizationEnabled);
}

void UDynamicPerformanceOptimizer::ShowOptimizationStats(bool bShow)
{
    bShowDebugStats = bShow;
    CVarShowOptimizationDebug->Set(bShow);
}

void UDynamicPerformanceOptimizer::ForceOptimizationUpdate()
{
    if (PerformanceProfiler)
    {
        FPerformanceMetrics Metrics = PerformanceProfiler->GetCurrentMetrics();
        AnalyzePerformance(Metrics);
        UE_LOG(LogDynamicPerformanceOptimizer, Log, TEXT("Forced optimization update completed"));
    }
}

// Static method implementations
FOptimizationSettings FOptimizationSettings::GetSettingsForLevel(EOptimizationLevel Level)
{
    FOptimizationSettings Settings;
    
    switch (Level)
    {
        case EOptimizationLevel::Ultra:
            // Maximum quality settings
            Settings.ScreenPercentage = 100.0f;
            Settings.ShadowQuality = 4;
            Settings.PostProcessQuality = 4;
            Settings.EffectsQuality = 4;
            Settings.TextureQuality = 4;
            Settings.ViewDistanceScale = 1.0f;
            Settings.MaxActiveMassAgents = 50000;
            Settings.MassAIUpdateFrequency = 60.0f;
            Settings.DinosaurBehaviorComplexity = 1.0f;
            Settings.PhysicsTickRate = 60.0f;
            Settings.MaxPhysicsBodies = 10000;
            Settings.NaniteMaxPixelsPerTriangle = 1;
            Settings.LumenReflectionQuality = 3;
            Settings.LumenGlobalIlluminationQuality = 3;
            break;
            
        case EOptimizationLevel::High:
            // High quality with minor optimizations
            Settings.ScreenPercentage = 95.0f;
            Settings.ShadowQuality = 3;
            Settings.PostProcessQuality = 3;
            Settings.EffectsQuality = 3;
            Settings.TextureQuality = 3;
            Settings.ViewDistanceScale = 0.9f;
            Settings.MaxActiveMassAgents = 40000;
            Settings.MassAIUpdateFrequency = 45.0f;
            Settings.DinosaurBehaviorComplexity = 0.9f;
            Settings.PhysicsTickRate = 60.0f;
            Settings.MaxPhysicsBodies = 8000;
            Settings.NaniteMaxPixelsPerTriangle = 2;
            Settings.LumenReflectionQuality = 2;
            Settings.LumenGlobalIlluminationQuality = 2;
            break;
            
        case EOptimizationLevel::Medium:
            // Balanced quality/performance
            Settings.ScreenPercentage = 85.0f;
            Settings.ShadowQuality = 2;
            Settings.PostProcessQuality = 2;
            Settings.EffectsQuality = 2;
            Settings.TextureQuality = 2;
            Settings.ViewDistanceScale = 0.8f;
            Settings.MaxActiveMassAgents = 25000;
            Settings.MassAIUpdateFrequency = 30.0f;
            Settings.DinosaurBehaviorComplexity = 0.7f;
            Settings.PhysicsTickRate = 45.0f;
            Settings.MaxPhysicsBodies = 6000;
            Settings.NaniteMaxPixelsPerTriangle = 4;
            Settings.LumenReflectionQuality = 1;
            Settings.LumenGlobalIlluminationQuality = 1;
            break;
            
        case EOptimizationLevel::Low:
            // Performance priority
            Settings.ScreenPercentage = 75.0f;
            Settings.ShadowQuality = 1;
            Settings.PostProcessQuality = 1;
            Settings.EffectsQuality = 1;
            Settings.TextureQuality = 1;
            Settings.ViewDistanceScale = 0.6f;
            Settings.MaxActiveMassAgents = 15000;
            Settings.MassAIUpdateFrequency = 20.0f;
            Settings.DinosaurBehaviorComplexity = 0.5f;
            Settings.PhysicsTickRate = 30.0f;
            Settings.MaxPhysicsBodies = 4000;
            Settings.NaniteMaxPixelsPerTriangle = 8;
            Settings.LumenReflectionQuality = 0;
            Settings.LumenGlobalIlluminationQuality = 0;
            break;
            
        case EOptimizationLevel::Emergency:
            // Minimum quality for playability
            Settings.ScreenPercentage = 60.0f;
            Settings.ShadowQuality = 0;
            Settings.PostProcessQuality = 0;
            Settings.EffectsQuality = 0;
            Settings.TextureQuality = 0;
            Settings.ViewDistanceScale = 0.4f;
            Settings.MaxActiveMassAgents = 5000;
            Settings.MassAIUpdateFrequency = 15.0f;
            Settings.DinosaurBehaviorComplexity = 0.3f;
            Settings.PhysicsTickRate = 20.0f;
            Settings.MaxPhysicsBodies = 2000;
            Settings.NaniteMaxPixelsPerTriangle = 16;
            Settings.LumenReflectionQuality = 0;
            Settings.LumenGlobalIlluminationQuality = 0;
            break;
    }
    
    return Settings;
}