#include "Perf_PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"

UPerf_PerformanceManager::UPerf_PerformanceManager()
{
    CurrentPerformanceLevel = EPerf_PerformanceLevel::Medium;
    TargetFPS = 60.0f;
    bPerformanceMonitoringEnabled = true;
    LastOptimizationTime = 0.0f;
    
    // Initialize default LOD settings
    CurrentLODSettings.ViewDistanceScale = 1.0f;
    CurrentLODSettings.FoliageLODScale = 1.0f;
    CurrentLODSettings.SkeletalMeshLODBias = 0;
    CurrentLODSettings.MaxShadowResolution = 4096;
}

void UPerf_PerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Manager initialized - Target FPS: %.1f"), TargetFPS);
    
    // Set initial performance level based on platform
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
    SetPerformanceLevel(EPerf_PerformanceLevel::High);
    TargetFPS = 60.0f;
#else
    SetPerformanceLevel(EPerf_PerformanceLevel::Medium);
    TargetFPS = 30.0f;
#endif

    // Apply initial LOD settings
    OptimizeLODSettings();
}

void UPerf_PerformanceManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Performance Manager shutting down"));
    Super::Deinitialize();
}

FPerf_PerformanceMetrics UPerf_PerformanceManager::GetCurrentMetrics() const
{
    FPerf_PerformanceMetrics Metrics;
    
    // Get current FPS
    if (GEngine && GEngine->GetGameViewport())
    {
        Metrics.CurrentFPS = 1.0f / FApp::GetDeltaTime();
        Metrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    }
    
    // Get memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Metrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Get visible actors count (simplified)
    if (GetWorld())
    {
        Metrics.VisibleActors = GetWorld()->GetActorCount();
    }
    
    // Draw calls would require more complex rendering stats
    Metrics.DrawCalls = 0; // Placeholder
    
    return Metrics;
}

void UPerf_PerformanceManager::SetPerformanceLevel(EPerf_PerformanceLevel Level)
{
    CurrentPerformanceLevel = Level;
    
    switch (Level)
    {
        case EPerf_PerformanceLevel::Low:
            CurrentLODSettings.ViewDistanceScale = 0.5f;
            CurrentLODSettings.FoliageLODScale = 0.3f;
            CurrentLODSettings.SkeletalMeshLODBias = 2;
            CurrentLODSettings.MaxShadowResolution = 1024;
            break;
            
        case EPerf_PerformanceLevel::Medium:
            CurrentLODSettings.ViewDistanceScale = 0.8f;
            CurrentLODSettings.FoliageLODScale = 0.7f;
            CurrentLODSettings.SkeletalMeshLODBias = 1;
            CurrentLODSettings.MaxShadowResolution = 2048;
            break;
            
        case EPerf_PerformanceLevel::High:
            CurrentLODSettings.ViewDistanceScale = 1.0f;
            CurrentLODSettings.FoliageLODScale = 1.0f;
            CurrentLODSettings.SkeletalMeshLODBias = 0;
            CurrentLODSettings.MaxShadowResolution = 4096;
            break;
            
        case EPerf_PerformanceLevel::Ultra:
            CurrentLODSettings.ViewDistanceScale = 1.2f;
            CurrentLODSettings.FoliageLODScale = 1.2f;
            CurrentLODSettings.SkeletalMeshLODBias = -1;
            CurrentLODSettings.MaxShadowResolution = 8192;
            break;
    }
    
    ApplyLODSettings(CurrentLODSettings);
    UE_LOG(LogTemp, Warning, TEXT("Performance level set to: %d"), (int32)Level);
}

EPerf_PerformanceLevel UPerf_PerformanceManager::GetCurrentPerformanceLevel() const
{
    return CurrentPerformanceLevel;
}

void UPerf_PerformanceManager::ApplyLODSettings(const FPerf_LODSettings& Settings)
{
    CurrentLODSettings = Settings;
    
    // Apply console commands for LOD settings
    if (GetWorld())
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(
            GetWorld(),
            FString::Printf(TEXT("r.ViewDistanceScale %.2f"), Settings.ViewDistanceScale)
        );
        
        UKismetSystemLibrary::ExecuteConsoleCommand(
            GetWorld(),
            FString::Printf(TEXT("foliage.LODDistanceScale %.2f"), Settings.FoliageLODScale)
        );
        
        UKismetSystemLibrary::ExecuteConsoleCommand(
            GetWorld(),
            FString::Printf(TEXT("r.SkeletalMeshLODBias %d"), Settings.SkeletalMeshLODBias)
        );
        
        UKismetSystemLibrary::ExecuteConsoleCommand(
            GetWorld(),
            FString::Printf(TEXT("r.Shadow.MaxResolution %d"), Settings.MaxShadowResolution)
        );
    }
}

FPerf_LODSettings UPerf_PerformanceManager::GetCurrentLODSettings() const
{
    return CurrentLODSettings;
}

void UPerf_PerformanceManager::OptimizeForCurrentConditions()
{
    if (!bPerformanceMonitoringEnabled)
    {
        return;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastOptimizationTime < OPTIMIZATION_INTERVAL)
    {
        return;
    }
    
    LastOptimizationTime = CurrentTime;
    
    FPerf_PerformanceMetrics CurrentMetrics = GetCurrentMetrics();
    CachedMetrics = CurrentMetrics;
    
    // Adjust performance level based on current FPS
    if (CurrentMetrics.CurrentFPS < LOW_FPS_THRESHOLD)
    {
        // Performance is poor, lower quality
        if (CurrentPerformanceLevel == EPerf_PerformanceLevel::Ultra)
        {
            SetPerformanceLevel(EPerf_PerformanceLevel::High);
        }
        else if (CurrentPerformanceLevel == EPerf_PerformanceLevel::High)
        {
            SetPerformanceLevel(EPerf_PerformanceLevel::Medium);
        }
        else if (CurrentPerformanceLevel == EPerf_PerformanceLevel::Medium)
        {
            SetPerformanceLevel(EPerf_PerformanceLevel::Low);
        }
    }
    else if (CurrentMetrics.CurrentFPS > HIGH_FPS_THRESHOLD)
    {
        // Performance is good, can increase quality
        if (CurrentPerformanceLevel == EPerf_PerformanceLevel::Low)
        {
            SetPerformanceLevel(EPerf_PerformanceLevel::Medium);
        }
        else if (CurrentPerformanceLevel == EPerf_PerformanceLevel::Medium)
        {
            SetPerformanceLevel(EPerf_PerformanceLevel::High);
        }
        else if (CurrentPerformanceLevel == EPerf_PerformanceLevel::High && TargetFPS >= 60.0f)
        {
            SetPerformanceLevel(EPerf_PerformanceLevel::Ultra);
        }
    }
}

void UPerf_PerformanceManager::EnablePerformanceMonitoring(bool bEnabled)
{
    bPerformanceMonitoringEnabled = bEnabled;
    UE_LOG(LogTemp, Warning, TEXT("Performance monitoring %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_PerformanceManager::SetTargetFPS(float NewTargetFPS)
{
    TargetFPS = FMath::Clamp(NewTargetFPS, 15.0f, 120.0f);
    
    if (GetWorld())
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(
            GetWorld(),
            FString::Printf(TEXT("t.MaxFPS %.1f"), TargetFPS)
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Target FPS set to: %.1f"), TargetFPS);
}

float UPerf_PerformanceManager::GetTargetFPS() const
{
    return TargetFPS;
}

void UPerf_PerformanceManager::UpdatePerformanceMetrics()
{
    CachedMetrics = GetCurrentMetrics();
}

void UPerf_PerformanceManager::CheckPerformanceThresholds()
{
    if (CachedMetrics.CurrentFPS < TargetFPS * 0.8f)
    {
        // Performance below 80% of target, optimize
        OptimizeForCurrentConditions();
    }
}

void UPerf_PerformanceManager::AdjustQualitySettings()
{
    // Implement dynamic quality adjustment based on performance
    if (GetWorld())
    {
        if (CachedMetrics.CurrentFPS < LOW_FPS_THRESHOLD)
        {
            // Reduce quality settings
            UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("r.PostProcessAAQuality 1"));
            UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("r.MotionBlurQuality 0"));
        }
        else if (CachedMetrics.CurrentFPS > HIGH_FPS_THRESHOLD)
        {
            // Increase quality settings
            UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("r.PostProcessAAQuality 3"));
            UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("r.MotionBlurQuality 2"));
        }
    }
}

void UPerf_PerformanceManager::OptimizeLODSettings()
{
    ApplyLODSettings(CurrentLODSettings);
}

void UPerf_PerformanceManager::UpdateViewDistanceScaling()
{
    if (GetWorld())
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(
            GetWorld(),
            FString::Printf(TEXT("r.ViewDistanceScale %.2f"), CurrentLODSettings.ViewDistanceScale)
        );
    }
}

void UPerf_PerformanceManager::OptimizeShadowSettings()
{
    if (GetWorld())
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(
            GetWorld(),
            FString::Printf(TEXT("r.Shadow.MaxResolution %d"), CurrentLODSettings.MaxShadowResolution)
        );
        
        UKismetSystemLibrary::ExecuteConsoleCommand(
            GetWorld(),
            TEXT("r.Shadow.DistanceScale 0.6")
        );
    }
}