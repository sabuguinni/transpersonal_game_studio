#include "PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/GameUserSettings.h"
#include "Scalability.h"

UPerformanceManager::UPerformanceManager()
{
    CurrentMetrics = FPerformanceMetrics();
    MetricsHistory.Reserve(MaxHistorySize);
}

void UPerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Initializing performance monitoring system"));
    
    // Start monitoring by default
    StartPerformanceMonitoring();
    
    // Set up memory optimization timer
    GetWorld()->GetTimerManager().SetTimer(
        MemoryOptimizationTimer,
        this,
        &UPerformanceManager::OptimizeMemoryUsage,
        30.0f, // Every 30 seconds
        true
    );
}

void UPerformanceManager::Deinitialize()
{
    StopPerformanceMonitoring();
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
        GetWorld()->GetTimerManager().ClearTimer(MemoryOptimizationTimer);
    }
    
    Super::Deinitialize();
}

bool UPerformanceManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UPerformanceManager::StartPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = true;
    
    // Update metrics every frame
    GetWorld()->GetTimerManager().SetTimer(
        PerformanceUpdateTimer,
        this,
        &UPerformanceManager::UpdatePerformanceMetrics,
        0.016f, // ~60 FPS
        true
    );
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Performance monitoring started"));
}

void UPerformanceManager::StopPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = false;
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Performance monitoring stopped"));
}

FPerformanceMetrics UPerformanceManager::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerformanceManager::SetTargetFPS(float InTargetFPS)
{
    TargetFPS = FMath::Clamp(InTargetFPS, 30.0f, 120.0f);
    MaxFrameTimeThreshold = 1000.0f / TargetFPS; // Convert to milliseconds
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Target FPS set to %.1f"), TargetFPS);
}

void UPerformanceManager::SetPerformanceThresholds(float MaxFrameTime, float MaxGameThreadTime, float MaxRenderThreadTime)
{
    MaxFrameTimeThreshold = MaxFrameTime;
    MaxGameThreadTimeThreshold = MaxGameThreadTime;
    MaxRenderThreadTimeThreshold = MaxRenderThreadTime;
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Performance thresholds updated"));
}

void UPerformanceManager::SetGlobalLODBias(int32 LODBias)
{
    static IConsoleVariable* LODBiasVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForceLOD"));
    if (LODBiasVar)
    {
        LODBiasVar->Set(LODBias);
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: LOD bias set to %d"), LODBias);
    }
}

void UPerformanceManager::SetViewDistanceScale(float Scale)
{
    static IConsoleVariable* ViewDistanceVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale"));
    if (ViewDistanceVar)
    {
        ViewDistanceVar->Set(Scale);
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: View distance scale set to %.2f"), Scale);
    }
}

void UPerformanceManager::SetRenderingQuality(int32 QualityLevel)
{
    QualityLevel = FMath::Clamp(QualityLevel, 0, 4);
    
    // Apply scalability settings
    Scalability::SetQualityLevels(Scalability::CreateQualityLevelSet(
        QualityLevel, // ResolutionQuality
        QualityLevel, // ViewDistanceQuality
        QualityLevel, // AntiAliasingQuality
        QualityLevel, // ShadowQuality
        QualityLevel, // GlobalIlluminationQuality
        QualityLevel, // ReflectionQuality
        QualityLevel, // PostProcessQuality
        QualityLevel, // TextureQuality
        QualityLevel, // EffectsQuality
        QualityLevel  // FoliageQuality
    ));
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Rendering quality set to %d"), QualityLevel);
}

void UPerformanceManager::EnablePerformanceMode(bool bEnable)
{
    bPerformanceModeEnabled = bEnable;
    
    if (bEnable)
    {
        // Store original settings
        UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
        if (Settings)
        {
            OriginalQualityLevel = Settings->GetOverallScalabilityLevel();
        }
        
        // Apply performance optimizations
        SetRenderingQuality(1); // Low quality
        SetViewDistanceScale(0.7f);
        SetGlobalLODBias(1);
        
        // Disable expensive features
        static IConsoleVariable* MotionBlurVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality"));
        if (MotionBlurVar) MotionBlurVar->Set(0);
        
        static IConsoleVariable* BloomVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BloomQuality"));
        if (BloomVar) BloomVar->Set(1);
        
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Performance mode ENABLED"));
    }
    else
    {
        // Restore original settings
        SetRenderingQuality(OriginalQualityLevel);
        SetViewDistanceScale(OriginalViewDistanceScale);
        SetGlobalLODBias(OriginalLODBias);
        
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Performance mode DISABLED"));
    }
}

void UPerformanceManager::ForceGarbageCollection()
{
    GEngine->ForceGarbageCollection(true);
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Forced garbage collection"));
}

void UPerformanceManager::OptimizeMemoryUsage()
{
    // Force garbage collection
    ForceGarbageCollection();
    
    // Flush rendering commands
    FlushRenderingCommands();
    
    // Trim memory pools
    FPlatformMisc::TrimMemory();
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Memory optimization completed"));
}

void UPerformanceManager::UpdatePerformanceMetrics()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    // Get FPS and frame time
    CurrentMetrics.CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    CurrentMetrics.FrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to ms
    
    // Get thread times (approximation)
    CurrentMetrics.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
    CurrentMetrics.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    CurrentMetrics.GPUTime = FPlatformTime::ToMilliseconds(GGPUFrameTime);
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Get scene complexity
    CurrentMetrics.ActiveActors = GetWorld()->GetActorCount();
    
    // Store in history
    MetricsHistory.Add(CurrentMetrics);
    if (MetricsHistory.Num() > MaxHistorySize)
    {
        MetricsHistory.RemoveAt(0);
    }
    
    // Check thresholds
    CheckPerformanceThresholds();
}

void UPerformanceManager::CheckPerformanceThresholds()
{
    bool bThresholdExceeded = false;
    
    if (CurrentMetrics.FrameTime > MaxFrameTimeThreshold)
    {
        bThresholdExceeded = true;
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Frame time threshold exceeded: %.2fms"), CurrentMetrics.FrameTime);
    }
    
    if (CurrentMetrics.GameThreadTime > MaxGameThreadTimeThreshold)
    {
        bThresholdExceeded = true;
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Game thread time threshold exceeded: %.2fms"), CurrentMetrics.GameThreadTime);
    }
    
    if (CurrentMetrics.RenderThreadTime > MaxRenderThreadTimeThreshold)
    {
        bThresholdExceeded = true;
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Render thread time threshold exceeded: %.2fms"), CurrentMetrics.RenderThreadTime);
    }
    
    if (bThresholdExceeded)
    {
        OnPerformanceThresholdExceeded.Broadcast(CurrentMetrics);
        
        // Auto-apply optimizations if not in performance mode
        if (!bPerformanceModeEnabled)
        {
            ApplyPerformanceOptimizations();
        }
    }
}

void UPerformanceManager::ApplyPerformanceOptimizations()
{
    // Reduce LOD bias temporarily
    SetGlobalLODBias(1);
    
    // Reduce view distance
    SetViewDistanceScale(0.8f);
    
    // Schedule restoration after a few seconds
    FTimerHandle RestoreTimer;
    GetWorld()->GetTimerManager().SetTimer(
        RestoreTimer,
        [this]()
        {
            SetGlobalLODBias(0);
            SetViewDistanceScale(1.0f);
        },
        5.0f,
        false
    );
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Applied temporary performance optimizations"));
}