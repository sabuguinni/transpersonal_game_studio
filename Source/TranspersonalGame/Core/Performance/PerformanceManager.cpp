// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformMemory.h"
#include "Misc/App.h"
#include "Stats/StatsData.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogPerformanceManager, Log, All);

UPerformanceManager::UPerformanceManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(FrameHistorySize);
}

void UPerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    CachedWorld = GetWorld();
    InitializePlatformSettings();
    InitializePerformanceTargets();
    
    // Setup performance logging path
    PerformanceLogPath = FPaths::ProjectSavedDir() / TEXT("Logs") / TEXT("Performance") / 
                        FDateTime::Now().ToString(TEXT("%Y-%m-%d_%H-%M-%S")) + TEXT("_performance.csv");
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance Manager initialized for platform: %s, Target: %.2f fps"), 
           *GetCurrentPlatform(), 1000.0f / TargetFrameTimeMS);
}

void UPerformanceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdatePerformanceMetrics();
    
    if (bEnableDynamicScaling)
    {
        AdjustPhysicsQuality();
    }
    
    // Log performance data if enabled
    if (bLogPerformance)
    {
        LogPerformanceMetrics();
    }
}

void UPerformanceManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance Manager shutdown. Final average FPS: %.2f"), 
           CurrentMetrics.CurrentFPS);
}

void UPerformanceManager::InitializePerformanceTargets()
{
    InitializePlatformSettings();
    ApplyQualitySettings();
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance targets initialized: %.2f ms frame time, Quality: %s"), 
           TargetFrameTimeMS, 
           *UEnum::GetValueAsString(CurrentQuality));
}

void UPerformanceManager::InitializePlatformSettings()
{
    FString Platform = GetCurrentPlatform();
    
    if (Platform.Contains(TEXT("Console")) || Platform.Contains(TEXT("PlayStation")) || Platform.Contains(TEXT("Xbox")))
    {
        // Console settings - prioritize stability at 30fps
        PerformanceTarget = EPerformanceTarget::Console_30fps;
        TargetFrameTimeMS = 33.33f; // 30fps
        CurrentQuality = EPerformanceQuality::Medium;
        MemoryThresholdMB = 3000.0f; // Lower memory threshold for consoles
        
        UE_LOG(LogPerformanceManager, Log, TEXT("Console platform detected - targeting 30fps"));
    }
    else
    {
        // PC settings - target 60fps
        PerformanceTarget = EPerformanceTarget::PC_60fps;
        TargetFrameTimeMS = 16.67f; // 60fps
        CurrentQuality = EPerformanceQuality::High;
        MemoryThresholdMB = 4000.0f;
        
        UE_LOG(LogPerformanceManager, Log, TEXT("PC platform detected - targeting 60fps"));
    }
}

FPerformanceMetrics UPerformanceManager::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UPerformanceManager::UpdatePerformanceMetrics()
{
    if (!CachedWorld)
    {
        return;
    }
    
    // Get frame time
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    CurrentMetrics.FrameTimeMS = CurrentFrameTime;
    CurrentMetrics.CurrentFPS = CurrentFrameTime > 0.0f ? 1000.0f / CurrentFrameTime : 0.0f;
    
    // Update frame time history
    FrameTimeHistory.Add(CurrentFrameTime);
    if (FrameTimeHistory.Num() > FrameHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Get GPU time (approximate)
    CurrentMetrics.GPUTimeMS = CurrentFrameTime * 0.6f; // Estimate GPU takes 60% of frame time
    CurrentMetrics.GameThreadTimeMS = CurrentFrameTime * 0.3f; // Game thread ~30%
    CurrentMetrics.RenderThreadTimeMS = CurrentFrameTime * 0.4f; // Render thread ~40%
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Get rendering stats (approximate)
    CurrentMetrics.DrawCalls = 1000 + FMath::RandRange(-200, 500); // Simulated for now
    CurrentMetrics.TriangleCount = 500000 + FMath::RandRange(-100000, 200000); // Simulated
    
    // Physics stats
    CurrentMetrics.PhysicsTimeMS = CurrentFrameTime * 0.15f; // Physics typically 15% of frame
    CurrentMetrics.ActivePhysicsBodies = 150 + FMath::RandRange(-50, 100); // Simulated
    
    // Check if meeting performance target
    CurrentMetrics.bMeetingTarget = CurrentFrameTime <= (TargetFrameTimeMS + FrameTimeTolerance);
}

void UPerformanceManager::AdjustPhysicsQuality()
{
    if (!IsPerformanceTargetMet())
    {
        // Performance is below target - scale down
        float CurrentTime = CachedWorld->GetTimeSeconds();
        if (CurrentTime - LastQualityAdjustmentTime >= QualityAdjustmentCooldown)
        {
            ScaleQualityDown();
            LastQualityAdjustmentTime = CurrentTime;
        }
    }
    else if (CalculatePerformanceScore() > 85.0f) // Good performance headroom
    {
        // Performance is good - try scaling up
        float CurrentTime = CachedWorld->GetTimeSeconds();
        if (CurrentTime - LastQualityAdjustmentTime >= QualityAdjustmentCooldown * 2.0f) // Longer cooldown for scaling up
        {
            ScaleQualityUp();
            LastQualityAdjustmentTime = CurrentTime;
        }
    }
}

bool UPerformanceManager::IsPerformanceTargetMet() const
{
    if (FrameTimeHistory.Num() < 30) // Need at least half second of data
    {
        return true; // Assume good until we have data
    }
    
    // Check if 90% of recent frames meet the target
    int32 FramesMeetingTarget = 0;
    for (float FrameTime : FrameTimeHistory)
    {
        if (FrameTime <= (TargetFrameTimeMS + FrameTimeTolerance))
        {
            FramesMeetingTarget++;
        }
    }
    
    float PercentageMeeting = static_cast<float>(FramesMeetingTarget) / FrameTimeHistory.Num();
    return PercentageMeeting >= 0.9f; // 90% of frames must meet target
}

void UPerformanceManager::ScaleQualityDown()
{
    EPerformanceQuality NewQuality = CurrentQuality;
    
    switch (CurrentQuality)
    {
        case EPerformanceQuality::Epic:
            NewQuality = EPerformanceQuality::High;
            break;
        case EPerformanceQuality::High:
            NewQuality = EPerformanceQuality::Medium;
            break;
        case EPerformanceQuality::Medium:
            NewQuality = EPerformanceQuality::Low;
            break;
        case EPerformanceQuality::Low:
            // Already at lowest quality
            return;
    }
    
    SetPerformanceQuality(NewQuality);
    UE_LOG(LogPerformanceManager, Warning, TEXT("Performance below target - scaling quality down to %s"), 
           *UEnum::GetValueAsString(NewQuality));
}

void UPerformanceManager::ScaleQualityUp()
{
    EPerformanceQuality NewQuality = CurrentQuality;
    
    switch (CurrentQuality)
    {
        case EPerformanceQuality::Low:
            NewQuality = EPerformanceQuality::Medium;
            break;
        case EPerformanceQuality::Medium:
            NewQuality = EPerformanceQuality::High;
            break;
        case EPerformanceQuality::High:
            NewQuality = EPerformanceQuality::Epic;
            break;
        case EPerformanceQuality::Epic:
            // Already at highest quality
            return;
    }
    
    SetPerformanceQuality(NewQuality);
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance headroom available - scaling quality up to %s"), 
           *UEnum::GetValueAsString(NewQuality));
}

void UPerformanceManager::SetPerformanceQuality(EPerformanceQuality QualityLevel)
{
    if (CurrentQuality == QualityLevel)
    {
        return;
    }
    
    CurrentQuality = QualityLevel;
    ApplyQualitySettings();
    
    // Broadcast quality change event
    OnPerformanceQualityChanged.Broadcast(CurrentQuality);
}

void UPerformanceManager::ApplyQualitySettings()
{
    // Apply physics settings
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        if (PhysicsSubstepCounts.Contains(CurrentQuality))
        {
            PhysicsSettings->MaxSubsteps = PhysicsSubstepCounts[CurrentQuality];
        }
    }
    
    // Apply console variables for quality
    switch (CurrentQuality)
    {
        case EPerformanceQuality::Low:
            // Low quality settings for stable 30fps
            GEngine->Exec(CachedWorld, TEXT("r.ViewDistanceScale 0.6"));
            GEngine->Exec(CachedWorld, TEXT("r.SkeletalMeshLODBias 2"));
            GEngine->Exec(CachedWorld, TEXT("r.ParticleLODBias 2"));
            GEngine->Exec(CachedWorld, TEXT("r.ShadowQuality 1"));
            GEngine->Exec(CachedWorld, TEXT("r.PostProcessAAQuality 1"));
            break;
            
        case EPerformanceQuality::Medium:
            // Medium quality settings
            GEngine->Exec(CachedWorld, TEXT("r.ViewDistanceScale 0.8"));
            GEngine->Exec(CachedWorld, TEXT("r.SkeletalMeshLODBias 1"));
            GEngine->Exec(CachedWorld, TEXT("r.ParticleLODBias 1"));
            GEngine->Exec(CachedWorld, TEXT("r.ShadowQuality 2"));
            GEngine->Exec(CachedWorld, TEXT("r.PostProcessAAQuality 2"));
            break;
            
        case EPerformanceQuality::High:
            // High quality settings for 60fps
            GEngine->Exec(CachedWorld, TEXT("r.ViewDistanceScale 1.0"));
            GEngine->Exec(CachedWorld, TEXT("r.SkeletalMeshLODBias 0"));
            GEngine->Exec(CachedWorld, TEXT("r.ParticleLODBias 0"));
            GEngine->Exec(CachedWorld, TEXT("r.ShadowQuality 3"));
            GEngine->Exec(CachedWorld, TEXT("r.PostProcessAAQuality 3"));
            break;
            
        case EPerformanceQuality::Epic:
            // Epic quality settings for high-end systems
            GEngine->Exec(CachedWorld, TEXT("r.ViewDistanceScale 1.2"));
            GEngine->Exec(CachedWorld, TEXT("r.SkeletalMeshLODBias 0"));
            GEngine->Exec(CachedWorld, TEXT("r.ParticleLODBias 0"));
            GEngine->Exec(CachedWorld, TEXT("r.ShadowQuality 4"));
            GEngine->Exec(CachedWorld, TEXT("r.PostProcessAAQuality 4"));
            break;
    }
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Applied quality settings: %s"), 
           *UEnum::GetValueAsString(CurrentQuality));
}

void UPerformanceManager::SetDynamicScaling(bool bEnabled)
{
    bEnableDynamicScaling = bEnabled;
    UE_LOG(LogPerformanceManager, Log, TEXT("Dynamic scaling %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerformanceManager::ForceGarbageCollectionIfNeeded()
{
    if (CurrentMetrics.MemoryUsageMB > MemoryThresholdMB)
    {
        UE_LOG(LogPerformanceManager, Warning, TEXT("Memory usage high (%.2f MB) - forcing garbage collection"), 
               CurrentMetrics.MemoryUsageMB);
        
        GEngine->ForceGarbageCollection(true);
    }
}

FPerformanceBottleneckAnalysis UPerformanceManager::AnalyzePerformanceBottlenecks()
{
    FPerformanceBottleneckAnalysis Analysis;
    
    // Determine primary bottleneck
    if (CurrentMetrics.GPUTimeMS > CurrentMetrics.GameThreadTimeMS * 1.2f)
    {
        Analysis.PrimaryBottleneck = TEXT("GPU Bound");
        Analysis.GPUBoundPercentage = 70.0f;
        Analysis.CPUBoundPercentage = 30.0f;
        
        Analysis.RecommendedActions.Add(TEXT("Reduce shadow quality"));
        Analysis.RecommendedActions.Add(TEXT("Lower view distance"));
        Analysis.RecommendedActions.Add(TEXT("Reduce particle effects"));
    }
    else if (CurrentMetrics.GameThreadTimeMS > TargetFrameTimeMS * 0.7f)
    {
        Analysis.PrimaryBottleneck = TEXT("CPU Bound");
        Analysis.CPUBoundPercentage = 70.0f;
        Analysis.GPUBoundPercentage = 30.0f;
        
        Analysis.RecommendedActions.Add(TEXT("Reduce physics complexity"));
        Analysis.RecommendedActions.Add(TEXT("Optimize AI tick frequency"));
        Analysis.RecommendedActions.Add(TEXT("Reduce collision checks"));
    }
    else
    {
        Analysis.PrimaryBottleneck = TEXT("Balanced");
        Analysis.CPUBoundPercentage = 50.0f;
        Analysis.GPUBoundPercentage = 50.0f;
    }
    
    // Memory pressure analysis
    Analysis.MemoryPressure = FMath::Clamp((CurrentMetrics.MemoryUsageMB / MemoryThresholdMB) * 100.0f, 0.0f, 100.0f);
    if (Analysis.MemoryPressure > 80.0f)
    {
        Analysis.RecommendedActions.Add(TEXT("Force garbage collection"));
        Analysis.RecommendedActions.Add(TEXT("Reduce texture quality"));
    }
    
    // Physics overhead analysis
    Analysis.PhysicsOverhead = (CurrentMetrics.PhysicsTimeMS / CurrentMetrics.FrameTimeMS) * 100.0f;
    if (Analysis.PhysicsOverhead > 20.0f)
    {
        Analysis.RecommendedActions.Add(TEXT("Reduce physics substeps"));
        Analysis.RecommendedActions.Add(TEXT("Simplify collision meshes"));
    }
    
    return Analysis;
}

void UPerformanceManager::LogPerformanceMetrics()
{
    // Simple CSV logging for performance analysis
    FString LogLine = FString::Printf(TEXT("%.3f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%.2f,%d,%s\n"),
        CachedWorld->GetTimeSeconds(),
        CurrentMetrics.FrameTimeMS,
        CurrentMetrics.CurrentFPS,
        CurrentMetrics.GPUTimeMS,
        CurrentMetrics.GameThreadTimeMS,
        CurrentMetrics.MemoryUsageMB,
        CurrentMetrics.DrawCalls,
        CurrentMetrics.TriangleCount,
        CurrentMetrics.PhysicsTimeMS,
        CurrentMetrics.ActivePhysicsBodies,
        *UEnum::GetValueAsString(CurrentQuality)
    );
    
    // Write to file (simplified for this implementation)
    UE_LOG(LogPerformanceManager, VeryVerbose, TEXT("Performance: %s"), *LogLine);
}

FString UPerformanceManager::GetCurrentPlatform() const
{
    return FPlatformProperties::PlatformName();
}

float UPerformanceManager::CalculatePerformanceScore() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 100.0f;
    }
    
    // Calculate average frame time
    float AverageFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        AverageFrameTime += FrameTime;
    }
    AverageFrameTime /= FrameTimeHistory.Num();
    
    // Score based on how well we meet the target (100 = perfect, 0 = terrible)
    float Score = FMath::Clamp(100.0f - ((AverageFrameTime - TargetFrameTimeMS) / TargetFrameTimeMS) * 100.0f, 0.0f, 100.0f);
    
    return Score;
}