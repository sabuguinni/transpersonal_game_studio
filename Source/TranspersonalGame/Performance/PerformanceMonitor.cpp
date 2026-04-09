#include "PerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/PlatformMemory.h"
#include "Stats/StatsData.h"
#include "RenderCore.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

DEFINE_LOG_CATEGORY_STATIC(LogPerformanceMonitor, Log, All);

// Performance stat declarations
DECLARE_CYCLE_STAT(TEXT("Performance Monitor Update"), STAT_PerformanceMonitorUpdate, STATGROUP_Game);
DECLARE_DWORD_COUNTER_STAT(TEXT("Performance Snapshots"), STAT_PerformanceSnapshots, STATGROUP_Game);
DECLARE_FLOAT_COUNTER_STAT(TEXT("Current Frame Rate"), STAT_CurrentFrameRate, STATGROUP_Game);

UPerformanceMonitor::UPerformanceMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    // Set default thresholds for PC high-end
    Thresholds.TargetFrameRate = 60.0f;
    Thresholds.MaxGameThreadTime = 16.0f;
    Thresholds.MaxRenderThreadTime = 16.0f;
    Thresholds.MaxGPUTime = 16.0f;
    Thresholds.MaxPhysicsTime = 5.0f;
    Thresholds.MaxMemoryUsageMB = 4096.0f;
    Thresholds.MaxDrawCalls = 2000;
    Thresholds.MaxTriangles = 2000000;
    Thresholds.MaxCollisionQueries = 500;
    
    // Set default settings
    Settings.PlatformTarget = EPlatformTarget::PC_High;
    Settings.bEnableAutoOptimization = true;
    Settings.bLogPerformanceData = true;
    Settings.MonitoringFrequency = 1.0f;
    Settings.HistorySize = 300;
    Settings.bShowDebugOverlay = false;
    Settings.bEnableHitchDetection = true;
    Settings.HitchThreshold = 33.0f;
    
    // Initialize state
    bIsMonitoring = false;
    LastMonitorTime = 0.0f;
    LastFrameTime = 0.0f;
    FrameCounter = 0;
    CurrentPerformanceLevel = EPerformanceLevel::Good;
    
    RecentFrameTimes.Reserve(120); // 2 seconds at 60fps
}

void UPerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPerformanceMonitor, Log, TEXT("PerformanceMonitor: Initializing for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("World"));
    
    InitializePerformanceMonitor();
    
    if (Settings.bEnableAutoOptimization)
    {
        ApplyPlatformOptimizations();
    }
    
    StartMonitoring();
}

void UPerformanceMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    SCOPE_CYCLE_COUNTER(STAT_PerformanceMonitorUpdate);
    
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsMonitoring)
        return;
    
    FrameCounter++;
    RecentFrameTimes.Add(DeltaTime);
    
    // Trim recent frame times array
    if (RecentFrameTimes.Num() > 120)
    {
        RecentFrameTimes.RemoveAt(0);
    }
    
    // Detect hitches
    if (Settings.bEnableHitchDetection)
    {
        DetectHitches(DeltaTime);
    }
    
    // Update performance snapshot at specified frequency
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastMonitorTime >= Settings.MonitoringFrequency)
    {
        UpdatePerformanceSnapshot();
        AnalyzePerformance();
        LastMonitorTime = CurrentTime;
        
        INC_DWORD_STAT(STAT_PerformanceSnapshots);
    }
}

void UPerformanceMonitor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopMonitoring();
    
    if (Settings.bLogPerformanceData && PerformanceHistory.Num() > 0)
    {
        UE_LOG(LogPerformanceMonitor, Log, TEXT("PerformanceMonitor: Session complete. Average FPS: %.2f"), 
               GetAverageFrameRate(PerformanceHistory.Num()));
        
        // Export final performance data
        FString ExportPath = FPaths::ProjectSavedDir() / TEXT("Performance") / TEXT("SessionData.csv");
        ExportPerformanceData(ExportPath);
    }
    
    Super::EndPlay(EndPlayReason);
}

void UPerformanceMonitor::InitializePerformanceMonitor()
{
    // Clear any existing data
    PerformanceHistory.Empty();
    RecentFrameTimes.Empty();
    
    // Apply platform-specific thresholds
    switch (Settings.PlatformTarget)
    {
        case EPlatformTarget::PC_High:
            Thresholds.TargetFrameRate = 60.0f;
            Thresholds.MaxGameThreadTime = 16.0f;
            Thresholds.MaxRenderThreadTime = 16.0f;
            break;
            
        case EPlatformTarget::PC_Medium:
            Thresholds.TargetFrameRate = 45.0f;
            Thresholds.MaxGameThreadTime = 22.0f;
            Thresholds.MaxRenderThreadTime = 22.0f;
            break;
            
        case EPlatformTarget::Console_Next:
            Thresholds.TargetFrameRate = 30.0f;
            Thresholds.MaxGameThreadTime = 33.0f;
            Thresholds.MaxRenderThreadTime = 33.0f;
            Thresholds.MaxMemoryUsageMB = 8192.0f;
            break;
            
        case EPlatformTarget::Console_Current:
            Thresholds.TargetFrameRate = 30.0f;
            Thresholds.MaxGameThreadTime = 33.0f;
            Thresholds.MaxRenderThreadTime = 33.0f;
            Thresholds.MaxMemoryUsageMB = 4096.0f;
            Thresholds.MaxDrawCalls = 1000;
            break;
            
        case EPlatformTarget::Mobile:
            Thresholds.TargetFrameRate = 30.0f;
            Thresholds.MaxGameThreadTime = 33.0f;
            Thresholds.MaxRenderThreadTime = 33.0f;
            Thresholds.MaxMemoryUsageMB = 2048.0f;
            Thresholds.MaxDrawCalls = 500;
            Thresholds.MaxTriangles = 500000;
            break;
    }
    
    UE_LOG(LogPerformanceMonitor, Log, TEXT("PerformanceMonitor: Initialized for platform target: %d"), 
           (int32)Settings.PlatformTarget);
}

void UPerformanceMonitor::UpdatePerformanceSnapshot()
{
    FPerformanceSnapshot NewSnapshot;
    
    // Measure all performance metrics
    NewSnapshot.FrameRate = MeasureFrameRate();
    NewSnapshot.GameThreadTime = MeasureGameThreadTime();
    NewSnapshot.RenderThreadTime = MeasureRenderThreadTime();
    NewSnapshot.GPUTime = MeasureGPUTime();
    NewSnapshot.PhysicsTime = MeasurePhysicsTime();
    NewSnapshot.MemoryUsageMB = MeasureMemoryUsage();
    NewSnapshot.DrawCalls = MeasureDrawCalls();
    NewSnapshot.TriangleCount = MeasureTriangleCount();
    NewSnapshot.CollisionQueries = MeasureCollisionQueries();
    NewSnapshot.Timestamp = FDateTime::Now();
    NewSnapshot.PerformanceLevel = CalculatePerformanceLevel(NewSnapshot.FrameRate);
    
    // Update current snapshot
    CurrentSnapshot = NewSnapshot;
    
    // Add to history
    PerformanceHistory.Add(NewSnapshot);
    TrimPerformanceHistory();
    
    // Update stats
    SET_FLOAT_STAT(STAT_CurrentFrameRate, NewSnapshot.FrameRate);
    
    if (Settings.bLogPerformanceData)
    {
        UE_LOG(LogPerformanceMonitor, VeryVerbose, 
               TEXT("Performance: FPS=%.1f, Game=%.2fms, Render=%.2fms, GPU=%.2fms, Physics=%.2fms, Mem=%.1fMB"), 
               NewSnapshot.FrameRate, NewSnapshot.GameThreadTime, NewSnapshot.RenderThreadTime, 
               NewSnapshot.GPUTime, NewSnapshot.PhysicsTime, NewSnapshot.MemoryUsageMB);
    }
}

void UPerformanceMonitor::AnalyzePerformance()
{
    CheckThresholds();
    UpdatePerformanceLevel();
    
    if (Settings.bEnableAutoOptimization && !IsPerformanceAcceptable())
    {
        ApplyAutoOptimization();
    }
}

void UPerformanceMonitor::CheckThresholds()
{
    const FPerformanceSnapshot& Snapshot = CurrentSnapshot;
    
    if (Snapshot.FrameRate < Thresholds.TargetFrameRate * 0.9f) // 10% tolerance
    {
        OnPerformanceThresholdExceeded.Broadcast(EPerformanceMetric::FrameRate);
    }
    
    if (Snapshot.GameThreadTime > Thresholds.MaxGameThreadTime)
    {
        OnPerformanceThresholdExceeded.Broadcast(EPerformanceMetric::GameThreadTime);
    }
    
    if (Snapshot.RenderThreadTime > Thresholds.MaxRenderThreadTime)
    {
        OnPerformanceThresholdExceeded.Broadcast(EPerformanceMetric::RenderThreadTime);
    }
    
    if (Snapshot.GPUTime > Thresholds.MaxGPUTime)
    {
        OnPerformanceThresholdExceeded.Broadcast(EPerformanceMetric::GPUTime);
    }
    
    if (Snapshot.PhysicsTime > Thresholds.MaxPhysicsTime)
    {
        OnPerformanceThresholdExceeded.Broadcast(EPerformanceMetric::PhysicsTime);
    }
    
    if (Snapshot.MemoryUsageMB > Thresholds.MaxMemoryUsageMB)
    {
        OnPerformanceThresholdExceeded.Broadcast(EPerformanceMetric::MemoryUsage);
    }
    
    if (Snapshot.DrawCalls > Thresholds.MaxDrawCalls)
    {
        OnPerformanceThresholdExceeded.Broadcast(EPerformanceMetric::DrawCalls);
    }
    
    if (Snapshot.TriangleCount > Thresholds.MaxTriangles)
    {
        OnPerformanceThresholdExceeded.Broadcast(EPerformanceMetric::Triangles);
    }
    
    if (Snapshot.CollisionQueries > Thresholds.MaxCollisionQueries)
    {
        OnPerformanceThresholdExceeded.Broadcast(EPerformanceMetric::CollisionQueries);
    }
}

void UPerformanceMonitor::DetectHitches(float DeltaTime)
{
    float FrameTimeMs = DeltaTime * 1000.0f;
    
    if (FrameTimeMs > Settings.HitchThreshold)
    {
        UE_LOG(LogPerformanceMonitor, Warning, TEXT("Hitch detected: %.2fms frame time"), FrameTimeMs);
        OnHitchDetected.Broadcast(FrameTimeMs, CurrentSnapshot);
    }
}

void UPerformanceMonitor::UpdatePerformanceLevel()
{
    EPerformanceLevel NewLevel = CalculatePerformanceLevel(CurrentSnapshot.FrameRate);
    
    if (NewLevel != CurrentPerformanceLevel)
    {
        CurrentPerformanceLevel = NewLevel;
        OnFrameRateChanged.Broadcast(NewLevel);
        
        UE_LOG(LogPerformanceMonitor, Log, TEXT("Performance level changed to: %d"), (int32)NewLevel);
    }
}

float UPerformanceMonitor::MeasureFrameRate() const
{
    if (RecentFrameTimes.Num() == 0)
        return 0.0f;
    
    float AverageFrameTime = 0.0f;
    int32 SampleCount = FMath::Min(60, RecentFrameTimes.Num()); // Use last 60 frames
    
    for (int32 i = RecentFrameTimes.Num() - SampleCount; i < RecentFrameTimes.Num(); i++)
    {
        AverageFrameTime += RecentFrameTimes[i];
    }
    
    AverageFrameTime /= SampleCount;
    return AverageFrameTime > 0.0f ? 1.0f / AverageFrameTime : 0.0f;
}

float UPerformanceMonitor::MeasureGameThreadTime() const
{
    // Get game thread time from stats system
    if (FThreadStats::IsCollectingData())
    {
        return FPlatformTime::ToMilliseconds(FThreadStats::GetStatValue(GET_STATID(STAT_FrameTime)));
    }
    return 0.0f;
}

float UPerformanceMonitor::MeasureRenderThreadTime() const
{
    // Get render thread time from stats system
    if (FThreadStats::IsCollectingData())
    {
        return FPlatformTime::ToMilliseconds(FThreadStats::GetStatValue(GET_STATID(STAT_RenderingIdleTime)));
    }
    return 0.0f;
}

float UPerformanceMonitor::MeasureGPUTime() const
{
    // Get GPU time from RHI
    if (GRHISupportsGPUTimestamps)
    {
        return FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
    }
    return 0.0f;
}

float UPerformanceMonitor::MeasurePhysicsTime() const
{
    // Get physics time from stats system
    if (FThreadStats::IsCollectingData())
    {
        return FPlatformTime::ToMilliseconds(FThreadStats::GetStatValue(GET_STATID(STAT_PhysicsTime)));
    }
    return 0.0f;
}

float UPerformanceMonitor::MeasureMemoryUsage() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

int32 UPerformanceMonitor::MeasureDrawCalls() const
{
    // Get draw calls from stats system
    if (FThreadStats::IsCollectingData())
    {
        return FThreadStats::GetStatValue(GET_STATID(STAT_RenderDrawCalls));
    }
    return 0;
}

int32 UPerformanceMonitor::MeasureTriangleCount() const
{
    // Get triangle count from stats system
    if (FThreadStats::IsCollectingData())
    {
        return FThreadStats::GetStatValue(GET_STATID(STAT_RenderTriangles));
    }
    return 0;
}

int32 UPerformanceMonitor::MeasureCollisionQueries() const
{
    // Get collision queries from stats system
    if (FThreadStats::IsCollectingData())
    {
        return FThreadStats::GetStatValue(GET_STATID(STAT_Collision_QueriesPerFrame));
    }
    return 0;
}

void UPerformanceMonitor::StartMonitoring()
{
    if (bIsMonitoring)
        return;
    
    bIsMonitoring = true;
    LastMonitorTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UE_LOG(LogPerformanceMonitor, Log, TEXT("PerformanceMonitor: Started monitoring"));
}

void UPerformanceMonitor::StopMonitoring()
{
    if (!bIsMonitoring)
        return;
    
    bIsMonitoring = false;
    
    UE_LOG(LogPerformanceMonitor, Log, TEXT("PerformanceMonitor: Stopped monitoring"));
}

void UPerformanceMonitor::ApplyAutoOptimization()
{
    if (!IsPerformanceAcceptable())
    {
        EPerformanceMetric WorstMetric = GetWorstPerformingMetric();
        
        switch (WorstMetric)
        {
            case EPerformanceMetric::FrameRate:
            case EPerformanceMetric::RenderThreadTime:
                OptimizeRenderingSettings();
                break;
                
            case EPerformanceMetric::GameThreadTime:
                OptimizeLODSettings();
                break;
                
            case EPerformanceMetric::GPUTime:
                OptimizeShadowSettings();
                break;
                
            case EPerformanceMetric::PhysicsTime:
                OptimizePhysicsSettings();
                break;
                
            default:
                OptimizeLODSettings(); // Default optimization
                break;
        }
    }
}

void UPerformanceMonitor::OptimizeLODSettings()
{
    float PerformanceRatio = CurrentSnapshot.FrameRate / Thresholds.TargetFrameRate;
    ApplyLODOptimization(PerformanceRatio);
    
    OnPerformanceOptimized.Broadcast(TEXT("LOD settings optimized"));
    UE_LOG(LogPerformanceMonitor, Log, TEXT("Applied LOD optimization"));
}

void UPerformanceMonitor::OptimizeShadowSettings()
{
    float PerformanceRatio = CurrentSnapshot.FrameRate / Thresholds.TargetFrameRate;
    ApplyShadowOptimization(PerformanceRatio);
    
    OnPerformanceOptimized.Broadcast(TEXT("Shadow settings optimized"));
    UE_LOG(LogPerformanceMonitor, Log, TEXT("Applied shadow optimization"));
}

void UPerformanceMonitor::OptimizePhysicsSettings()
{
    float PerformanceRatio = CurrentSnapshot.FrameRate / Thresholds.TargetFrameRate;
    ApplyPhysicsOptimization(PerformanceRatio);
    
    OnPerformanceOptimized.Broadcast(TEXT("Physics settings optimized"));
    UE_LOG(LogPerformanceMonitor, Log, TEXT("Applied physics optimization"));
}

void UPerformanceMonitor::OptimizeRenderingSettings()
{
    float PerformanceRatio = CurrentSnapshot.FrameRate / Thresholds.TargetFrameRate;
    ApplyRenderingOptimization(PerformanceRatio);
    
    OnPerformanceOptimized.Broadcast(TEXT("Rendering settings optimized"));
    UE_LOG(LogPerformanceMonitor, Log, TEXT("Applied rendering optimization"));
}

bool UPerformanceMonitor::IsPerformanceAcceptable() const
{
    return CurrentSnapshot.FrameRate >= (Thresholds.TargetFrameRate * 0.9f) &&
           CurrentSnapshot.GameThreadTime <= Thresholds.MaxGameThreadTime &&
           CurrentSnapshot.RenderThreadTime <= Thresholds.MaxRenderThreadTime &&
           CurrentSnapshot.GPUTime <= Thresholds.MaxGPUTime;
}

EPerformanceMetric UPerformanceMonitor::GetWorstPerformingMetric() const
{
    float FrameRateRatio = CurrentSnapshot.FrameRate / Thresholds.TargetFrameRate;
    float GameThreadRatio = CurrentSnapshot.GameThreadTime / Thresholds.MaxGameThreadTime;
    float RenderThreadRatio = CurrentSnapshot.RenderThreadTime / Thresholds.MaxRenderThreadTime;
    float GPUTimeRatio = CurrentSnapshot.GPUTime / Thresholds.MaxGPUTime;
    float PhysicsTimeRatio = CurrentSnapshot.PhysicsTime / Thresholds.MaxPhysicsTime;
    
    // Find the metric with the worst ratio (highest for times, lowest for frame rate)
    if (FrameRateRatio < 0.9f && FrameRateRatio < GameThreadRatio && FrameRateRatio < RenderThreadRatio)
    {
        return EPerformanceMetric::FrameRate;
    }
    else if (GameThreadRatio > 1.0f && GameThreadRatio >= RenderThreadRatio && GameThreadRatio >= GPUTimeRatio)
    {
        return EPerformanceMetric::GameThreadTime;
    }
    else if (RenderThreadRatio > 1.0f && RenderThreadRatio >= GPUTimeRatio)
    {
        return EPerformanceMetric::RenderThreadTime;
    }
    else if (GPUTimeRatio > 1.0f)
    {
        return EPerformanceMetric::GPUTime;
    }
    else if (PhysicsTimeRatio > 1.0f)
    {
        return EPerformanceMetric::PhysicsTime;
    }
    
    return EPerformanceMetric::FrameRate; // Default
}

EPerformanceLevel UPerformanceMonitor::CalculatePerformanceLevel(float FrameRate) const
{
    if (FrameRate >= 60.0f)
        return EPerformanceLevel::Excellent;
    else if (FrameRate >= 45.0f)
        return EPerformanceLevel::Good;
    else if (FrameRate >= 30.0f)
        return EPerformanceLevel::Fair;
    else if (FrameRate >= 15.0f)
        return EPerformanceLevel::Poor;
    else
        return EPerformanceLevel::Critical;
}

void UPerformanceMonitor::ApplyPlatformOptimizations()
{
    switch (Settings.PlatformTarget)
    {
        case EPlatformTarget::PC_High:
            OptimizeForPC();
            break;
        case EPlatformTarget::Console_Next:
        case EPlatformTarget::Console_Current:
            OptimizeForConsole();
            break;
        case EPlatformTarget::Mobile:
            OptimizeForMobile();
            break;
    }
}

void UPerformanceMonitor::OptimizeForPC()
{
    // PC optimizations - high quality settings
    if (GEngine && GEngine->GameUserSettings)
    {
        UGameUserSettings* Settings = GEngine->GameUserSettings;
        Settings->SetShadowQuality(3); // Epic
        Settings->SetTextureQuality(3); // Epic
        Settings->SetEffectsQuality(3); // Epic
        Settings->SetPostProcessingQuality(3); // Epic
        Settings->ApplySettings(false);
    }
}

void UPerformanceMonitor::OptimizeForConsole()
{
    // Console optimizations - balanced settings
    if (GEngine && GEngine->GameUserSettings)
    {
        UGameUserSettings* Settings = GEngine->GameUserSettings;
        Settings->SetShadowQuality(2); // High
        Settings->SetTextureQuality(2); // High
        Settings->SetEffectsQuality(2); // High
        Settings->SetPostProcessingQuality(2); // High
        Settings->ApplySettings(false);
    }
}

void UPerformanceMonitor::OptimizeForMobile()
{
    // Mobile optimizations - performance focused
    if (GEngine && GEngine->GameUserSettings)
    {
        UGameUserSettings* Settings = GEngine->GameUserSettings;
        Settings->SetShadowQuality(0); // Low
        Settings->SetTextureQuality(1); // Medium
        Settings->SetEffectsQuality(0); // Low
        Settings->SetPostProcessingQuality(0); // Low
        Settings->ApplySettings(false);
    }
}

void UPerformanceMonitor::ExportPerformanceData(const FString& FilePath) const
{
    if (PerformanceHistory.Num() == 0)
        return;
    
    FString CSVContent = TEXT("Timestamp,FrameRate,GameThreadTime,RenderThreadTime,GPUTime,PhysicsTime,MemoryUsageMB,DrawCalls,TriangleCount,CollisionQueries,PerformanceLevel\n");
    
    for (const FPerformanceSnapshot& Snapshot : PerformanceHistory)
    {
        CSVContent += FString::Printf(TEXT("%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%d\n"),
            *Snapshot.Timestamp.ToString(),
            Snapshot.FrameRate,
            Snapshot.GameThreadTime,
            Snapshot.RenderThreadTime,
            Snapshot.GPUTime,
            Snapshot.PhysicsTime,
            Snapshot.MemoryUsageMB,
            Snapshot.DrawCalls,
            Snapshot.TriangleCount,
            Snapshot.CollisionQueries,
            (int32)Snapshot.PerformanceLevel
        );
    }
    
    FFileHelper::SaveStringToFile(CSVContent, *FilePath);
    UE_LOG(LogPerformanceMonitor, Log, TEXT("Performance data exported to: %s"), *FilePath);
}