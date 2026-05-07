#include "Perf_RenderingOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "Stats/Stats.h"
#include "HAL/IConsoleManager.h"
#include "Engine/GameViewportClient.h"

DEFINE_LOG_CATEGORY_STATIC(LogRenderingOptimizer, Log, All);

UPerf_RenderingOptimizer::UPerf_RenderingOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    // Initialize settings
    TargetFrameRate = 60.0f;
    bAutoOptimization = true;
    OptimizationInterval = 5.0f;
    CurrentQuality = EPerf_RenderQuality::High;
    LastOptimizationTime = 0.0f;
    
    // Store default settings
    DefaultSettings = FPerf_RenderingSettings();
    CurrentSettings = DefaultSettings;
}

void UPerf_RenderingOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Rendering Optimizer initialized"));
    
    // Store current settings as defaults
    DefaultSettings = CurrentSettings;
    
    // Start auto-optimization timer if enabled
    if (bAutoOptimization && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            OptimizationTimerHandle,
            this,
            &UPerf_RenderingOptimizer::CheckAutoOptimization,
            OptimizationInterval,
            true
        );
    }
}

void UPerf_RenderingOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateRenderingStats();
}

void UPerf_RenderingOptimizer::UpdateRenderingStats()
{
    // Update rendering statistics
    if (GEngine && GEngine->GetGameViewport())
    {
        // Get frame rate
        float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        
        // Update stats on render thread
        ENQUEUE_RENDER_COMMAND(UpdateRenderStats)(
            [this](FRHICommandListImmediate& RHICmdList)
            {
                // Get GPU memory usage
                if (GRHISupportsMemoryInfo)
                {
                    uint64 TotalGPUMemory = 0;
                    uint64 UsedGPUMemory = 0;
                    RHIGetGPUMemoryInfo(TotalGPUMemory, UsedGPUMemory);
                    CurrentStats.GPUMemoryUsageMB = UsedGPUMemory / (1024.0f * 1024.0f);
                }
                
                // Get render thread timing
                CurrentStats.RenderThreadTime = FPlatformTime::Seconds();
            });
    }
}

void UPerf_RenderingOptimizer::OptimizeForTargetFPS(float TargetFPS)
{
    TargetFrameRate = TargetFPS;
    
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    float PerformanceRatio = CurrentFPS / TargetFPS;
    
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Optimizing for target FPS: %.1f (Current: %.1f, Ratio: %.2f)"), 
           TargetFPS, CurrentFPS, PerformanceRatio);
    
    // Choose quality level based on performance
    if (PerformanceRatio < 0.5f)
    {
        SetRenderQuality(EPerf_RenderQuality::Low);
    }
    else if (PerformanceRatio < 0.7f)
    {
        SetRenderQuality(EPerf_RenderQuality::Medium);
    }
    else if (PerformanceRatio < 0.9f)
    {
        SetRenderQuality(EPerf_RenderQuality::High);
    }
    else
    {
        SetRenderQuality(EPerf_RenderQuality::Ultra);
    }
}

void UPerf_RenderingOptimizer::SetRenderQuality(EPerf_RenderQuality Quality)
{
    CurrentQuality = Quality;
    ApplyQualityPreset(Quality);
    
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Render quality set to: %s"), 
           *UEnum::GetValueAsString(Quality));
    
    OnRenderingOptimized.Broadcast(Quality);
}

void UPerf_RenderingOptimizer::ApplyQualityPreset(EPerf_RenderQuality Quality)
{
    FPerf_RenderingSettings NewSettings;
    
    switch (Quality)
    {
        case EPerf_RenderQuality::Low:
            NewSettings = GetLowQualitySettings();
            break;
        case EPerf_RenderQuality::Medium:
            NewSettings = GetMediumQualitySettings();
            break;
        case EPerf_RenderQuality::High:
            NewSettings = GetHighQualitySettings();
            break;
        case EPerf_RenderQuality::Ultra:
            NewSettings = GetUltraQualitySettings();
            break;
        case EPerf_RenderQuality::Custom:
            // Keep current custom settings
            return;
    }
    
    ApplyCustomSettings(NewSettings);
}

void UPerf_RenderingOptimizer::ApplyCustomSettings(const FPerf_RenderingSettings& Settings)
{
    CurrentSettings = Settings;
    
    // Apply shadow settings
    ExecuteConsoleCommand(FString::Printf(TEXT("r.ShadowQuality %d"), Settings.ShadowQuality));
    
    // Apply texture settings
    ExecuteConsoleCommand(FString::Printf(TEXT("r.TextureQuality %d"), Settings.TextureQuality));
    
    // Apply effects settings
    ExecuteConsoleCommand(FString::Printf(TEXT("r.EffectsQuality %d"), Settings.EffectsQuality));
    
    // Apply post-process settings
    ExecuteConsoleCommand(FString::Printf(TEXT("r.PostProcessQuality %d"), Settings.PostProcessQuality));
    
    // Apply view distance
    ExecuteConsoleCommand(FString::Printf(TEXT("r.ViewDistanceScale %f"), Settings.ViewDistanceScale));
    
    // Apply foliage density
    ExecuteConsoleCommand(FString::Printf(TEXT("foliage.DensityScale %f"), Settings.FoliageDensityScale));
    
    // Apply Lumen settings
    ExecuteConsoleCommand(FString::Printf(TEXT("r.Lumen.DiffuseIndirect.Allow %d"), Settings.bEnableLumen ? 1 : 0));
    ExecuteConsoleCommand(FString::Printf(TEXT("r.Lumen.Reflections.Allow %d"), Settings.bEnableLumen ? 1 : 0));
    
    // Apply Nanite settings
    ExecuteConsoleCommand(FString::Printf(TEXT("r.Nanite.MaxPixelsPerEdge %d"), Settings.bEnableNanite ? 1 : 0));
    
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Applied custom rendering settings"));
}

void UPerf_RenderingOptimizer::ResetToDefaultSettings()
{
    ApplyCustomSettings(DefaultSettings);
    CurrentQuality = EPerf_RenderQuality::High;
    
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Reset to default rendering settings"));
}

FPerf_RenderingStats UPerf_RenderingOptimizer::GetCurrentRenderingStats() const
{
    return CurrentStats;
}

float UPerf_RenderingOptimizer::GetCurrentGPUUsage() const
{
    return CurrentStats.GPUMemoryUsageMB;
}

int32 UPerf_RenderingOptimizer::GetCurrentDrawCalls() const
{
    return static_cast<int32>(CurrentStats.DrawCalls);
}

void UPerf_RenderingOptimizer::OptimizeLODSettings()
{
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Optimizing LOD settings"));
    
    // Adjust LOD bias based on performance
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    float PerformanceRatio = CurrentFPS / TargetFrameRate;
    
    if (PerformanceRatio < 0.7f)
    {
        SetGlobalLODBias(1.0f); // Use lower LODs
    }
    else if (PerformanceRatio > 1.2f)
    {
        SetGlobalLODBias(-0.5f); // Use higher LODs
    }
    else
    {
        SetGlobalLODBias(0.0f); // Default LODs
    }
}

void UPerf_RenderingOptimizer::SetGlobalLODBias(float LODBias)
{
    ExecuteConsoleCommand(FString::Printf(TEXT("r.StaticMeshLODBias %f"), LODBias));
    ExecuteConsoleCommand(FString::Printf(TEXT("r.SkeletalMeshLODBias %f"), LODBias));
    
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Set global LOD bias to: %f"), LODBias);
}

void UPerf_RenderingOptimizer::OptimizeShadowSettings()
{
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Optimizing shadow settings"));
    
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    float PerformanceRatio = CurrentFPS / TargetFrameRate;
    
    if (PerformanceRatio < 0.6f)
    {
        // Low performance - reduce shadows significantly
        ExecuteConsoleCommand(TEXT("r.ShadowQuality 1"));
        SetShadowDistance(2000.0f);
        ExecuteConsoleCommand(TEXT("r.Shadow.MaxResolution 512"));
    }
    else if (PerformanceRatio < 0.8f)
    {
        // Medium performance - moderate shadow quality
        ExecuteConsoleCommand(TEXT("r.ShadowQuality 2"));
        SetShadowDistance(4000.0f);
        ExecuteConsoleCommand(TEXT("r.Shadow.MaxResolution 1024"));
    }
    else
    {
        // Good performance - high shadow quality
        ExecuteConsoleCommand(TEXT("r.ShadowQuality 3"));
        SetShadowDistance(8000.0f);
        ExecuteConsoleCommand(TEXT("r.Shadow.MaxResolution 2048"));
    }
}

void UPerf_RenderingOptimizer::SetShadowDistance(float Distance)
{
    ExecuteConsoleCommand(FString::Printf(TEXT("r.Shadow.DistanceScale %f"), Distance / 10000.0f));
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Set shadow distance to: %f"), Distance);
}

void UPerf_RenderingOptimizer::OptimizeTextureSettings()
{
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Optimizing texture settings"));
    
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    float PerformanceRatio = CurrentFPS / TargetFrameRate;
    
    if (PerformanceRatio < 0.6f)
    {
        // Low performance - reduce texture quality
        ExecuteConsoleCommand(TEXT("r.TextureQuality 1"));
        SetTextureStreamingPoolSize(512);
    }
    else if (PerformanceRatio < 0.8f)
    {
        // Medium performance
        ExecuteConsoleCommand(TEXT("r.TextureQuality 2"));
        SetTextureStreamingPoolSize(1024);
    }
    else
    {
        // Good performance
        ExecuteConsoleCommand(TEXT("r.TextureQuality 3"));
        SetTextureStreamingPoolSize(2048);
    }
}

void UPerf_RenderingOptimizer::SetTextureStreamingPoolSize(int32 SizeMB)
{
    ExecuteConsoleCommand(FString::Printf(TEXT("r.Streaming.PoolSize %d"), SizeMB));
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Set texture streaming pool size to: %d MB"), SizeMB);
}

void UPerf_RenderingOptimizer::OptimizePostProcessSettings()
{
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Optimizing post-process settings"));
    
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    float PerformanceRatio = CurrentFPS / TargetFrameRate;
    
    if (PerformanceRatio < 0.6f)
    {
        ExecuteConsoleCommand(TEXT("r.PostProcessQuality 1"));
        ExecuteConsoleCommand(TEXT("r.MotionBlurQuality 0"));
        ExecuteConsoleCommand(TEXT("r.AmbientOcclusionLevels 0"));
    }
    else if (PerformanceRatio < 0.8f)
    {
        ExecuteConsoleCommand(TEXT("r.PostProcessQuality 2"));
        ExecuteConsoleCommand(TEXT("r.MotionBlurQuality 2"));
        ExecuteConsoleCommand(TEXT("r.AmbientOcclusionLevels 1"));
    }
    else
    {
        ExecuteConsoleCommand(TEXT("r.PostProcessQuality 3"));
        ExecuteConsoleCommand(TEXT("r.MotionBlurQuality 3"));
        ExecuteConsoleCommand(TEXT("r.AmbientOcclusionLevels 3"));
    }
}

void UPerf_RenderingOptimizer::OptimizeCullingSettings()
{
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Optimizing culling settings"));
    
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    float PerformanceRatio = CurrentFPS / TargetFrameRate;
    
    if (PerformanceRatio < 0.7f)
    {
        SetViewDistanceScale(0.7f);
        ExecuteConsoleCommand(TEXT("r.Shadow.ForcedScreenRadius 0.01"));
    }
    else if (PerformanceRatio > 1.1f)
    {
        SetViewDistanceScale(1.2f);
        ExecuteConsoleCommand(TEXT("r.Shadow.ForcedScreenRadius 0.005"));
    }
    else
    {
        SetViewDistanceScale(1.0f);
        ExecuteConsoleCommand(TEXT("r.Shadow.ForcedScreenRadius 0.007"));
    }
}

void UPerf_RenderingOptimizer::SetViewDistanceScale(float Scale)
{
    CurrentSettings.ViewDistanceScale = Scale;
    ExecuteConsoleCommand(FString::Printf(TEXT("r.ViewDistanceScale %f"), Scale));
    UE_LOG(LogRenderingOptimizer, Log, TEXT("Set view distance scale to: %f"), Scale);
}

void UPerf_RenderingOptimizer::ExecuteConsoleCommand(const FString& Command)
{
    if (UWorld* World = GetWorld())
    {
        if (GEngine)
        {
            GEngine->Exec(World, *Command);
        }
    }
}

void UPerf_RenderingOptimizer::CheckAutoOptimization()
{
    if (!bAutoOptimization)
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastOptimizationTime < OptimizationInterval)
        return;
    
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    float PerformanceRatio = CurrentFPS / TargetFrameRate;
    
    // Only optimize if performance is significantly off target
    if (PerformanceRatio < 0.8f || PerformanceRatio > 1.3f)
    {
        OptimizeForTargetFPS(TargetFrameRate);
        LastOptimizationTime = CurrentTime;
    }
}

FPerf_RenderingSettings UPerf_RenderingOptimizer::GetLowQualitySettings() const
{
    FPerf_RenderingSettings Settings;
    Settings.ShadowQuality = 1;
    Settings.TextureQuality = 1;
    Settings.EffectsQuality = 1;
    Settings.PostProcessQuality = 1;
    Settings.ViewDistanceScale = 0.6f;
    Settings.FoliageDensityScale = 0.5f;
    Settings.bEnableLumen = false;
    Settings.bEnableNanite = false;
    return Settings;
}

FPerf_RenderingSettings UPerf_RenderingOptimizer::GetMediumQualitySettings() const
{
    FPerf_RenderingSettings Settings;
    Settings.ShadowQuality = 2;
    Settings.TextureQuality = 2;
    Settings.EffectsQuality = 2;
    Settings.PostProcessQuality = 2;
    Settings.ViewDistanceScale = 0.8f;
    Settings.FoliageDensityScale = 0.7f;
    Settings.bEnableLumen = false;
    Settings.bEnableNanite = true;
    return Settings;
}

FPerf_RenderingSettings UPerf_RenderingOptimizer::GetHighQualitySettings() const
{
    FPerf_RenderingSettings Settings;
    Settings.ShadowQuality = 3;
    Settings.TextureQuality = 3;
    Settings.EffectsQuality = 3;
    Settings.PostProcessQuality = 3;
    Settings.ViewDistanceScale = 1.0f;
    Settings.FoliageDensityScale = 1.0f;
    Settings.bEnableLumen = true;
    Settings.bEnableNanite = true;
    return Settings;
}

FPerf_RenderingSettings UPerf_RenderingOptimizer::GetUltraQualitySettings() const
{
    FPerf_RenderingSettings Settings;
    Settings.ShadowQuality = 4;
    Settings.TextureQuality = 4;
    Settings.EffectsQuality = 4;
    Settings.PostProcessQuality = 4;
    Settings.ViewDistanceScale = 1.2f;
    Settings.FoliageDensityScale = 1.2f;
    Settings.bEnableLumen = true;
    Settings.bEnableNanite = true;
    return Settings;
}