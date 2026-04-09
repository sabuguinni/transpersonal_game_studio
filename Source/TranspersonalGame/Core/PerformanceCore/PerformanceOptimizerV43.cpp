#include "PerformanceOptimizerV43.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Stats/StatsHierarchical.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

DEFINE_LOG_CATEGORY(LogPerformanceOptimizer);

// Performance target constants
const float UPerformanceOptimizerV43::PC_60FPS_TARGET = 16.67f;      // 60fps = 16.67ms
const float UPerformanceOptimizerV43::CONSOLE_30FPS_TARGET = 33.33f; // 30fps = 33.33ms
const float UPerformanceOptimizerV43::MOBILE_30FPS_TARGET = 33.33f;  // 30fps = 33.33ms
const float UPerformanceOptimizerV43::VR_90FPS_TARGET = 11.11f;      // 90fps = 11.11ms

UPerformanceOptimizerV43::UPerformanceOptimizerV43()
{
    bIsAnalyzing = false;
    AnalysisStartTime = 0.0f;
    MaxHistorySize = 300; // 5 minutes at 60fps
    
    // Set default thresholds
    FrameTimeThreshold = 20.0f; // 20ms threshold
    MemoryThreshold = 6144.0f;  // 6GB threshold
    DrawCallThreshold = 8000;   // 8k draw calls threshold
    
    // Enable dynamic optimizations
    bDynamicLODEnabled = true;
    bDynamicCullingEnabled = true;
    bDynamicQualityEnabled = true;
    
    // Initialize default profile for PC 60fps
    CurrentProfile.TargetPlatform = EPerformanceTarget::PC_60FPS;
    CurrentProfile.QualityLevel = EPerformanceLevel::High;
    CurrentProfile.TargetFrameRate = 60.0f;
    CurrentProfile.MaxFrameTime = PC_60FPS_TARGET;
}

void UPerformanceOptimizerV43::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance Optimizer V43 initialized"));
    
    // Apply initial optimization settings
    ApplyScalabilitySettings();
    OptimizeNaniteSettings();
    OptimizeLumenSettings();
    OptimizeTSRSettings();
    
    // Start performance monitoring
    StartPerformanceAnalysis();
}

void UPerformanceOptimizerV43::Deinitialize()
{
    StopPerformanceAnalysis();
    Super::Deinitialize();
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance Optimizer V43 deinitialized"));
}

void UPerformanceOptimizerV43::StartPerformanceAnalysis()
{
    if (bIsAnalyzing)
    {
        return;
    }
    
    bIsAnalyzing = true;
    AnalysisStartTime = FPlatformTime::Seconds();
    
    // Clear previous metrics
    MetricsHistory.Empty();
    
    // Start monitoring timer (every 100ms)
    GetWorld()->GetTimerManager().SetTimer(
        PerformanceMonitorTimer,
        this,
        &UPerformanceOptimizerV43::MonitorPerformanceMetrics,
        0.1f,
        true
    );
    
    // Enable performance stats
    ExecuteConsoleCommand(TEXT("stat unit"));
    ExecuteConsoleCommand(TEXT("stat fps"));
    ExecuteConsoleCommand(TEXT("stat gpu"));
    ExecuteConsoleCommand(TEXT("stat memory"));
    ExecuteConsoleCommand(TEXT("stat scenerendering"));
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance analysis started"));
}

void UPerformanceOptimizerV43::StopPerformanceAnalysis()
{
    if (!bIsAnalyzing)
    {
        return;
    }
    
    bIsAnalyzing = false;
    
    // Stop monitoring timer
    GetWorld()->GetTimerManager().ClearTimer(PerformanceMonitorTimer);
    
    // Disable performance stats
    ExecuteConsoleCommand(TEXT("stat none"));
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance analysis stopped"));
}

FPerformanceMetrics UPerformanceOptimizerV43::GetCurrentPerformanceMetrics()
{
    SCOPE_CYCLE_COUNTER(STAT_PerformanceAnalysis);
    
    FPerformanceMetrics Metrics;
    
    // Get frame timing
    if (GEngine && GEngine->GetGameViewport())
    {
        const FEngineShowFlags& ShowFlags = GEngine->GetGameViewport()->GetEngineShowFlags();
        
        // Frame time from stats
        Metrics.FrameTime = FPlatformTime::ToMilliseconds(GFrameTime);
        
        // Get render thread time
        Metrics.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
        
        // Get game thread time
        Metrics.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
        
        // Get GPU time (approximate)
        Metrics.GPUTime = FPlatformTime::ToMilliseconds(GGPUFrameTime);
    }
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Metrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Get rendering stats (approximate values)
    Metrics.DrawCalls = GNumDrawCallsRHI;
    Metrics.Triangles = GNumPrimitivesDrawnRHI;
    
    CurrentMetrics = Metrics;
    return Metrics;
}

void UPerformanceOptimizerV43::SetPerformanceProfile(const FPerformanceProfile& Profile)
{
    CurrentProfile = Profile;
    
    // Apply the new profile settings
    ApplyScalabilitySettings();
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Applied performance profile: Target=%.1f fps, Quality=%d"), 
           Profile.TargetFrameRate, (int32)Profile.QualityLevel);
}

void UPerformanceOptimizerV43::OptimizeForTarget(EPerformanceTarget Target)
{
    FPerformanceProfile NewProfile = CurrentProfile;
    NewProfile.TargetPlatform = Target;
    
    switch (Target)
    {
        case EPerformanceTarget::PC_60FPS:
            NewProfile.TargetFrameRate = 60.0f;
            NewProfile.MaxFrameTime = PC_60FPS_TARGET;
            NewProfile.QualityLevel = EPerformanceLevel::High;
            NewProfile.MaxDrawCalls = 8000;
            NewProfile.MaxMemoryUsageMB = 8192.0f;
            NewProfile.ViewDistanceScale = 1.0f;
            NewProfile.bEnableNanite = true;
            NewProfile.bEnableLumen = true;
            NewProfile.bEnableTSR = true;
            break;
            
        case EPerformanceTarget::Console_30FPS:
            NewProfile.TargetFrameRate = 30.0f;
            NewProfile.MaxFrameTime = CONSOLE_30FPS_TARGET;
            NewProfile.QualityLevel = EPerformanceLevel::Medium;
            NewProfile.MaxDrawCalls = 5000;
            NewProfile.MaxMemoryUsageMB = 6144.0f;
            NewProfile.ViewDistanceScale = 0.8f;
            NewProfile.bEnableNanite = true;
            NewProfile.bEnableLumen = true;
            NewProfile.bEnableTSR = true;
            break;
            
        case EPerformanceTarget::Mobile_30FPS:
            NewProfile.TargetFrameRate = 30.0f;
            NewProfile.MaxFrameTime = MOBILE_30FPS_TARGET;
            NewProfile.QualityLevel = EPerformanceLevel::Low;
            NewProfile.MaxDrawCalls = 2000;
            NewProfile.MaxMemoryUsageMB = 2048.0f;
            NewProfile.ViewDistanceScale = 0.6f;
            NewProfile.bEnableNanite = false;
            NewProfile.bEnableLumen = false;
            NewProfile.bEnableTSR = false;
            break;
            
        case EPerformanceTarget::VR_90FPS:
            NewProfile.TargetFrameRate = 90.0f;
            NewProfile.MaxFrameTime = VR_90FPS_TARGET;
            NewProfile.QualityLevel = EPerformanceLevel::Medium;
            NewProfile.MaxDrawCalls = 3000;
            NewProfile.MaxMemoryUsageMB = 4096.0f;
            NewProfile.ViewDistanceScale = 0.7f;
            NewProfile.bEnableNanite = true;
            NewProfile.bEnableLumen = false;
            NewProfile.bEnableTSR = true;
            break;
    }
    
    SetPerformanceProfile(NewProfile);
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Optimized for target: %s"), 
           *UEnum::GetValueAsString(Target));
}

void UPerformanceOptimizerV43::ApplyDynamicLODOptimization()
{
    SCOPE_CYCLE_COUNTER(STAT_LODOptimization);
    
    if (!bDynamicLODEnabled)
    {
        return;
    }
    
    FPerformanceMetrics Metrics = GetCurrentPerformanceMetrics();
    
    // Adjust LOD bias based on performance
    float LODBias = 0.0f;
    if (Metrics.FrameTime > CurrentProfile.MaxFrameTime * 1.2f)
    {
        // Performance is poor, increase LOD bias (lower quality, better performance)
        LODBias = 1.0f;
    }
    else if (Metrics.FrameTime < CurrentProfile.MaxFrameTime * 0.8f)
    {
        // Performance is good, decrease LOD bias (higher quality)
        LODBias = -0.5f;
    }
    
    SetCVar(TEXT("r.StaticMeshLODBias"), LODBias);
    SetCVar(TEXT("r.SkeletalMeshLODBias"), LODBias);
    
    UE_LOG(LogPerformanceOptimizer, VeryVerbose, TEXT("Applied LOD bias: %.2f"), LODBias);
}

void UPerformanceOptimizerV43::OptimizeCullingSettings()
{
    SCOPE_CYCLE_COUNTER(STAT_CullingOptimization);
    
    // Optimize view distance based on performance
    FPerformanceMetrics Metrics = GetCurrentPerformanceMetrics();
    
    float ViewDistanceScale = CurrentProfile.ViewDistanceScale;
    if (Metrics.FrameTime > CurrentProfile.MaxFrameTime * 1.1f)
    {
        // Reduce view distance for better performance
        ViewDistanceScale *= 0.9f;
    }
    else if (Metrics.FrameTime < CurrentProfile.MaxFrameTime * 0.9f)
    {
        // Increase view distance for better quality
        ViewDistanceScale *= 1.05f;
    }
    
    ViewDistanceScale = FMath::Clamp(ViewDistanceScale, 0.3f, 1.5f);
    SetCVar(TEXT("r.ViewDistanceScale"), ViewDistanceScale);
    
    // Optimize occlusion culling
    SetCVar(TEXT("r.AllowOcclusionQueries"), true);
    SetCVar(TEXT("r.HZBOcclusion"), 1);
    
    // Optimize frustum culling
    SetCVar(TEXT("r.EarlyZPass"), 3); // Full early Z-pass
    
    UE_LOG(LogPerformanceOptimizer, VeryVerbose, TEXT("Applied culling optimizations"));
}

void UPerformanceOptimizerV43::OptimizeMemoryUsage()
{
    SCOPE_CYCLE_COUNTER(STAT_MemoryOptimization);
    
    FPerformanceMetrics Metrics = GetCurrentPerformanceMetrics();
    
    if (Metrics.MemoryUsageMB > CurrentProfile.MaxMemoryUsageMB * 0.9f)
    {
        // High memory usage, optimize
        
        // Reduce texture streaming pool
        SetCVar(TEXT("r.Streaming.PoolSize"), 1024);
        
        // Reduce texture quality
        SetCVar(TEXT("r.Streaming.MipBias"), 1);
        
        // Force garbage collection
        if (GEngine)
        {
            GEngine->ForceGarbageCollection(true);
        }
        
        UE_LOG(LogPerformanceOptimizer, Log, TEXT("Applied memory optimizations due to high usage: %.1f MB"), 
               Metrics.MemoryUsageMB);
    }
}

void UPerformanceOptimizerV43::OptimizeForDinosaurAI(int32 DinosaurCount)
{
    // Optimize AI systems based on dinosaur count
    if (DinosaurCount > 50)
    {
        // High dinosaur count - reduce AI quality
        SetCVar(TEXT("ai.PerceptionSystemEnabled"), true);
        SetCVar(TEXT("ai.DebugDraw"), false);
        
        // Reduce AI tick frequency for distant dinosaurs
        SetCVar(TEXT("ai.UseSimplifiedBehaviorTrees"), true);
    }
    else if (DinosaurCount > 20)
    {
        // Medium dinosaur count - balanced settings
        SetCVar(TEXT("ai.PerceptionSystemEnabled"), true);
        SetCVar(TEXT("ai.UseSimplifiedBehaviorTrees"), false);
    }
    else
    {
        // Low dinosaur count - full AI quality
        SetCVar(TEXT("ai.PerceptionSystemEnabled"), true);
        SetCVar(TEXT("ai.UseSimplifiedBehaviorTrees"), false);
        SetCVar(TEXT("ai.DebugDraw"), false);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Optimized AI for %d dinosaurs"), DinosaurCount);
}

void UPerformanceOptimizerV43::OptimizeRenderingPipeline()
{
    // Optimize rendering based on current performance profile
    switch (CurrentProfile.QualityLevel)
    {
        case EPerformanceLevel::Low:
            SetCVar(TEXT("sg.ViewDistanceQuality"), 0);
            SetCVar(TEXT("sg.ShadowQuality"), 0);
            SetCVar(TEXT("sg.TextureQuality"), 0);
            SetCVar(TEXT("sg.EffectsQuality"), 0);
            SetCVar(TEXT("sg.PostProcessQuality"), 0);
            break;
            
        case EPerformanceLevel::Medium:
            SetCVar(TEXT("sg.ViewDistanceQuality"), 1);
            SetCVar(TEXT("sg.ShadowQuality"), 1);
            SetCVar(TEXT("sg.TextureQuality"), 1);
            SetCVar(TEXT("sg.EffectsQuality"), 1);
            SetCVar(TEXT("sg.PostProcessQuality"), 1);
            break;
            
        case EPerformanceLevel::High:
            SetCVar(TEXT("sg.ViewDistanceQuality"), 2);
            SetCVar(TEXT("sg.ShadowQuality"), 2);
            SetCVar(TEXT("sg.TextureQuality"), 2);
            SetCVar(TEXT("sg.EffectsQuality"), 2);
            SetCVar(TEXT("sg.PostProcessQuality"), 2);
            break;
            
        case EPerformanceLevel::Epic:
            SetCVar(TEXT("sg.ViewDistanceQuality"), 3);
            SetCVar(TEXT("sg.ShadowQuality"), 3);
            SetCVar(TEXT("sg.TextureQuality"), 3);
            SetCVar(TEXT("sg.EffectsQuality"), 3);
            SetCVar(TEXT("sg.PostProcessQuality"), 3);
            break;
            
        case EPerformanceLevel::Cinematic:
            SetCVar(TEXT("sg.ViewDistanceQuality"), 4);
            SetCVar(TEXT("sg.ShadowQuality"), 4);
            SetCVar(TEXT("sg.TextureQuality"), 4);
            SetCVar(TEXT("sg.EffectsQuality"), 4);
            SetCVar(TEXT("sg.PostProcessQuality"), 4);
            break;
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Optimized rendering pipeline for quality level: %d"), 
           (int32)CurrentProfile.QualityLevel);
}

void UPerformanceOptimizerV43::OptimizeNaniteSettings()
{
    if (CurrentProfile.bEnableNanite)
    {
        SetCVar(TEXT("r.Nanite"), 1);
        
        // Optimize Nanite based on target platform
        switch (CurrentProfile.TargetPlatform)
        {
            case EPerformanceTarget::PC_60FPS:
                SetCVar(TEXT("r.Nanite.MaxPixelsPerEdge"), 1);
                SetCVar(TEXT("r.Nanite.MaxCandidateClusters"), 16384);
                break;
                
            case EPerformanceTarget::Console_30FPS:
                SetCVar(TEXT("r.Nanite.MaxPixelsPerEdge"), 2);
                SetCVar(TEXT("r.Nanite.MaxCandidateClusters"), 8192);
                break;
                
            default:
                SetCVar(TEXT("r.Nanite.MaxPixelsPerEdge"), 2);
                SetCVar(TEXT("r.Nanite.MaxCandidateClusters"), 4096);
                break;
        }
        
        UE_LOG(LogPerformanceOptimizer, Log, TEXT("Nanite optimization applied"));
    }
    else
    {
        SetCVar(TEXT("r.Nanite"), 0);
        UE_LOG(LogPerformanceOptimizer, Log, TEXT("Nanite disabled for current profile"));
    }
}

void UPerformanceOptimizerV43::OptimizeLumenSettings()
{
    if (CurrentProfile.bEnableLumen)
    {
        SetCVar(TEXT("r.DynamicGlobalIlluminationMethod"), 1); // Lumen
        SetCVar(TEXT("r.ReflectionMethod"), 1); // Lumen
        
        // Optimize Lumen based on target platform
        switch (CurrentProfile.TargetPlatform)
        {
            case EPerformanceTarget::PC_60FPS:
                SetCVar(TEXT("r.Lumen.DiffuseIndirect.Allow"), 1);
                SetCVar(TEXT("r.Lumen.Reflections.Allow"), 1);
                SetCVar(TEXT("r.Lumen.ScreenProbeGather.DownsampleFactor"), 1);
                break;
                
            case EPerformanceTarget::Console_30FPS:
                SetCVar(TEXT("r.Lumen.DiffuseIndirect.Allow"), 1);
                SetCVar(TEXT("r.Lumen.Reflections.Allow"), 1);
                SetCVar(TEXT("r.Lumen.ScreenProbeGather.DownsampleFactor"), 2);
                break;
                
            default:
                SetCVar(TEXT("r.Lumen.DiffuseIndirect.Allow"), 0);
                SetCVar(TEXT("r.Lumen.Reflections.Allow"), 0);
                break;
        }
        
        UE_LOG(LogPerformanceOptimizer, Log, TEXT("Lumen optimization applied"));
    }
    else
    {
        SetCVar(TEXT("r.DynamicGlobalIlluminationMethod"), 0); // None
        SetCVar(TEXT("r.ReflectionMethod"), 0); // None
        UE_LOG(LogPerformanceOptimizer, Log, TEXT("Lumen disabled for current profile"));
    }
}

void UPerformanceOptimizerV43::OptimizeTSRSettings()
{
    if (CurrentProfile.bEnableTSR)
    {
        SetCVar(TEXT("r.AntiAliasingMethod"), 3); // TSR
        
        // Optimize TSR based on target platform
        switch (CurrentProfile.TargetPlatform)
        {
            case EPerformanceTarget::PC_60FPS:
                SetCVar(TEXT("r.TSR.ShadingRejection.SampleCount"), 8);
                SetCVar(TEXT("r.ScreenPercentage"), 100);
                break;
                
            case EPerformanceTarget::Console_30FPS:
                SetCVar(TEXT("r.TSR.ShadingRejection.SampleCount"), 4);
                SetCVar(TEXT("r.ScreenPercentage"), 75);
                break;
                
            default:
                SetCVar(TEXT("r.TSR.ShadingRejection.SampleCount"), 2);
                SetCVar(TEXT("r.ScreenPercentage"), 50);
                break;
        }
        
        UE_LOG(LogPerformanceOptimizer, Log, TEXT("TSR optimization applied"));
    }
    else
    {
        SetCVar(TEXT("r.AntiAliasingMethod"), 0); // None
        UE_LOG(LogPerformanceOptimizer, Log, TEXT("TSR disabled for current profile"));
    }
}

bool UPerformanceOptimizerV43::IsPerformanceTargetMet() const
{
    if (CurrentMetrics.FrameTime <= CurrentProfile.MaxFrameTime * 1.1f &&
        CurrentMetrics.MemoryUsageMB <= CurrentProfile.MaxMemoryUsageMB &&
        CurrentMetrics.DrawCalls <= CurrentProfile.MaxDrawCalls)
    {
        return true;
    }
    
    return false;
}

void UPerformanceOptimizerV43::MonitorPerformanceMetrics()
{
    if (!bIsAnalyzing)
    {
        return;
    }
    
    FPerformanceMetrics Metrics = GetCurrentPerformanceMetrics();
    
    // Add to history
    MetricsHistory.Add(Metrics);
    if (MetricsHistory.Num() > MaxHistorySize)
    {
        MetricsHistory.RemoveAt(0);
    }
    
    // Check for performance issues and apply dynamic adjustments
    if (bDynamicLODEnabled || bDynamicCullingEnabled || bDynamicQualityEnabled)
    {
        AdjustDynamicSettings();
    }
}

void UPerformanceOptimizerV43::AdjustDynamicSettings()
{
    FPerformanceMetrics Metrics = GetCurrentPerformanceMetrics();
    
    // Check if we need to handle performance drops
    if (Metrics.FrameTime > CurrentProfile.MaxFrameTime * 1.2f)
    {
        HandlePerformanceDrop();
    }
    else if (Metrics.FrameTime < CurrentProfile.MaxFrameTime * 0.7f)
    {
        HandlePerformanceSpike();
    }
}

void UPerformanceOptimizerV43::HandlePerformanceDrop()
{
    UE_LOG(LogPerformanceOptimizer, Warning, TEXT("Performance drop detected: %.2f ms (target: %.2f ms)"), 
           CurrentMetrics.FrameTime, CurrentProfile.MaxFrameTime);
    
    // Apply emergency optimizations
    if (bDynamicLODEnabled)
    {
        ApplyDynamicLODOptimization();
    }
    
    if (bDynamicCullingEnabled)
    {
        OptimizeCullingSettings();
    }
    
    if (bDynamicQualityEnabled)
    {
        // Temporarily reduce quality
        SetCVar(TEXT("r.ScreenPercentage"), 75);
        SetCVar(TEXT("r.ViewDistanceScale"), CurrentProfile.ViewDistanceScale * 0.8f);
    }
}

void UPerformanceOptimizerV43::HandlePerformanceSpike()
{
    // Performance is better than expected, we can increase quality
    if (bDynamicQualityEnabled)
    {
        SetCVar(TEXT("r.ScreenPercentage"), 100);
        SetCVar(TEXT("r.ViewDistanceScale"), CurrentProfile.ViewDistanceScale);
    }
}

void UPerformanceOptimizerV43::ApplyScalabilitySettings()
{
    OptimizeRenderingPipeline();
    OptimizeNaniteSettings();
    OptimizeLumenSettings();
    OptimizeTSRSettings();
}

void UPerformanceOptimizerV43::ExecuteConsoleCommand(const FString& Command)
{
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->ConsoleCommand(Command);
    }
}

void UPerformanceOptimizerV43::SetCVar(const FString& CVarName, float Value)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
    if (CVar)
    {
        CVar->Set(Value);
    }
}

void UPerformanceOptimizerV43::SetCVar(const FString& CVarName, int32 Value)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
    if (CVar)
    {
        CVar->Set(Value);
    }
}

void UPerformanceOptimizerV43::SetCVar(const FString& CVarName, bool bValue)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
    if (CVar)
    {
        CVar->Set(bValue ? 1 : 0);
    }
}