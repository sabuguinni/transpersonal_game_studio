// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Stats/StatsData.h"
#include "RHIStats.h"
#include "Scalability.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformMemory.h"
#include "RenderCore.h"
#include "Rendering/RenderingCommon.h"

DEFINE_LOG_CATEGORY(LogPerformanceManager);

void UPerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance Manager initialized"));
    
    // Initialize performance budgets
    InitializePerformanceBudgets();
    
    // Set default target based on platform
#if PLATFORM_DESKTOP
    SetPerformanceTarget(EPerformanceTarget::PC_60FPS);
#elif PLATFORM_CONSOLE
    SetPerformanceTarget(EPerformanceTarget::Console_30FPS);
#else
    SetPerformanceTarget(EPerformanceTarget::Mobile_30FPS);
#endif

    // Start performance monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimer,
            this,
            &UPerformanceManager::UpdatePerformanceMetrics,
            1.0f / MonitoringFrequency,
            true
        );
        
        // Start quality adjustment timer (less frequent)
        World->GetTimerManager().SetTimer(
            QualityAdjustmentTimer,
            this,
            &UPerformanceManager::CheckBudgetAndAdjustQuality,
            2.0f, // Check every 2 seconds
            true
        );
    }
}

void UPerformanceManager::Deinitialize()
{
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimer);
        World->GetTimerManager().ClearTimer(QualityAdjustmentTimer);
    }
    
    // Stop any active profiling
    if (bProfilingActive)
    {
        StopProfilingSession();
    }
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance Manager deinitialized"));
    
    Super::Deinitialize();
}

UPerformanceManager* UPerformanceManager::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UPerformanceManager>();
        }
    }
    return nullptr;
}

void UPerformanceManager::SetPerformanceTarget(EPerformanceTarget Target)
{
    CurrentTarget = Target;
    CurrentBudget = GetPlatformBudget(Target);
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance target set to: %s"), 
           *UEnum::GetValueAsString(Target));
    
    // Apply initial scalability settings based on target
    int32 QualityLevel = 2; // Medium by default
    switch (Target)
    {
        case EPerformanceTarget::PC_120FPS:
            QualityLevel = 1; // Low for high framerate
            break;
        case EPerformanceTarget::PC_60FPS:
            QualityLevel = 2; // Medium
            break;
        case EPerformanceTarget::Console_60FPS:
            QualityLevel = 2; // Medium
            break;
        case EPerformanceTarget::Console_30FPS:
            QualityLevel = 3; // High
            break;
        case EPerformanceTarget::Mobile_30FPS:
            QualityLevel = 0; // Low
            break;
    }
    
    ApplyScalabilityPreset(QualityLevel);
}

void UPerformanceManager::SetCustomBudget(const FPerformanceBudget& Budget)
{
    CurrentBudget = Budget;
    CurrentTarget = EPerformanceTarget::Custom;
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Custom performance budget applied: %.2f ms target frame time"),
           Budget.TargetFrameTimeMs);
}

FPerformanceMetrics UPerformanceManager::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

FPerformanceMetrics UPerformanceManager::GetAverageMetrics(float TimeWindowSeconds) const
{
    return CalculateAverageMetrics(TimeWindowSeconds);
}

bool UPerformanceManager::IsPerformanceWithinBudget() const
{
    return CurrentMetrics.FrameTimeMs <= CurrentBudget.TargetFrameTimeMs * 1.1f && // 10% tolerance
           CurrentMetrics.GameThreadMs <= CurrentBudget.MaxGameThreadMs &&
           CurrentMetrics.RenderThreadMs <= CurrentBudget.MaxRenderThreadMs &&
           CurrentMetrics.GPUTimeMs <= CurrentBudget.MaxGPUTimeMs &&
           CurrentMetrics.MemoryUsageMB <= CurrentBudget.MaxMemoryMB &&
           CurrentMetrics.VRAMUsageMB <= CurrentBudget.MaxVRAMMB &&
           CurrentMetrics.DrawCalls <= CurrentBudget.MaxDrawCalls &&
           CurrentMetrics.Triangles <= CurrentBudget.MaxTriangles;
}

TArray<FString> UPerformanceManager::GetBudgetViolations() const
{
    TArray<FString> Violations;
    
    if (CurrentMetrics.FrameTimeMs > CurrentBudget.TargetFrameTimeMs * 1.1f)
    {
        Violations.Add(FString::Printf(TEXT("Frame time: %.2f ms (target: %.2f ms)"), 
                                     CurrentMetrics.FrameTimeMs, CurrentBudget.TargetFrameTimeMs));
    }
    
    if (CurrentMetrics.GameThreadMs > CurrentBudget.MaxGameThreadMs)
    {
        Violations.Add(FString::Printf(TEXT("Game thread: %.2f ms (max: %.2f ms)"), 
                                     CurrentMetrics.GameThreadMs, CurrentBudget.MaxGameThreadMs));
    }
    
    if (CurrentMetrics.RenderThreadMs > CurrentBudget.MaxRenderThreadMs)
    {
        Violations.Add(FString::Printf(TEXT("Render thread: %.2f ms (max: %.2f ms)"), 
                                     CurrentMetrics.RenderThreadMs, CurrentBudget.MaxRenderThreadMs));
    }
    
    if (CurrentMetrics.GPUTimeMs > CurrentBudget.MaxGPUTimeMs)
    {
        Violations.Add(FString::Printf(TEXT("GPU time: %.2f ms (max: %.2f ms)"), 
                                     CurrentMetrics.GPUTimeMs, CurrentBudget.MaxGPUTimeMs));
    }
    
    if (CurrentMetrics.MemoryUsageMB > CurrentBudget.MaxMemoryMB)
    {
        Violations.Add(FString::Printf(TEXT("Memory: %.2f MB (max: %.2f MB)"), 
                                     CurrentMetrics.MemoryUsageMB, CurrentBudget.MaxMemoryMB));
    }
    
    if (CurrentMetrics.VRAMUsageMB > CurrentBudget.MaxVRAMMB)
    {
        Violations.Add(FString::Printf(TEXT("VRAM: %.2f MB (max: %.2f MB)"), 
                                     CurrentMetrics.VRAMUsageMB, CurrentBudget.MaxVRAMMB));
    }
    
    if (CurrentMetrics.DrawCalls > CurrentBudget.MaxDrawCalls)
    {
        Violations.Add(FString::Printf(TEXT("Draw calls: %d (max: %d)"), 
                                     CurrentMetrics.DrawCalls, CurrentBudget.MaxDrawCalls));
    }
    
    if (CurrentMetrics.Triangles > CurrentBudget.MaxTriangles)
    {
        Violations.Add(FString::Printf(TEXT("Triangles: %d (max: %d)"), 
                                     CurrentMetrics.Triangles, CurrentBudget.MaxTriangles));
    }
    
    return Violations;
}

void UPerformanceManager::SetAutoQualityAdjustment(bool bEnabled)
{
    bAutoQualityAdjustment = bEnabled;
    UE_LOG(LogPerformanceManager, Log, TEXT("Auto quality adjustment %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerformanceManager::ForceQualityAdjustment()
{
    CheckBudgetAndAdjustQuality();
}

void UPerformanceManager::StartProfilingSession(const FString& SessionName)
{
    CurrentSessionName = SessionName;
    bProfilingActive = true;
    SessionData.Empty();
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Started profiling session: %s"), *SessionName);
}

void UPerformanceManager::StopProfilingSession()
{
    if (bProfilingActive)
    {
        bProfilingActive = false;
        UE_LOG(LogPerformanceManager, Log, TEXT("Stopped profiling session: %s (%d samples)"), 
               *CurrentSessionName, SessionData.Num());
    }
}

bool UPerformanceManager::ExportPerformanceData(const FString& FilePath) const
{
    if (SessionData.Num() == 0)
    {
        UE_LOG(LogPerformanceManager, Warning, TEXT("No performance data to export"));
        return false;
    }
    
    FString CSVContent = TEXT("Timestamp,FrameTime,FPS,GameThread,RenderThread,GPU,Memory,VRAM,DrawCalls,Triangles,PhysicsBodies,AIAgents\n");
    
    for (const FPerformanceMetrics& Metrics : SessionData)
    {
        CSVContent += FString::Printf(TEXT("%s,%.3f,%.1f,%.3f,%.3f,%.3f,%.1f,%.1f,%d,%d,%d,%d\n"),
                                    *Metrics.Timestamp.ToString(),
                                    Metrics.FrameTimeMs,
                                    Metrics.FPS,
                                    Metrics.GameThreadMs,
                                    Metrics.RenderThreadMs,
                                    Metrics.GPUTimeMs,
                                    Metrics.MemoryUsageMB,
                                    Metrics.VRAMUsageMB,
                                    Metrics.DrawCalls,
                                    Metrics.Triangles,
                                    Metrics.PhysicsBodies,
                                    Metrics.AIAgents);
    }
    
    return FFileHelper::SaveStringToFile(CSVContent, *FilePath);
}

float UPerformanceManager::GetCategoryTiming(EPerformanceCategory Category) const
{
    switch (Category)
    {
        case EPerformanceCategory::CPU_Game:
            return CurrentMetrics.GameThreadMs;
        case EPerformanceCategory::CPU_Render:
            return CurrentMetrics.RenderThreadMs;
        case EPerformanceCategory::GPU_Total:
            return CurrentMetrics.GPUTimeMs;
        case EPerformanceCategory::Memory_Total:
            return CurrentMetrics.MemoryUsageMB;
        case EPerformanceCategory::Physics:
            return CurrentMetrics.PhysicsBodies * 0.01f; // Rough estimate
        case EPerformanceCategory::AI:
            return CurrentMetrics.AIAgents * 0.005f; // Rough estimate
        default:
            return 0.0f;
    }
}

void UPerformanceManager::SetMonitoringFrequency(float FrequencyHz)
{
    MonitoringFrequency = FMath::Clamp(FrequencyHz, 0.1f, 60.0f);
    
    // Restart timer with new frequency
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimer);
        World->GetTimerManager().SetTimer(
            MonitoringTimer,
            this,
            &UPerformanceManager::UpdatePerformanceMetrics,
            1.0f / MonitoringFrequency,
            true
        );
    }
}

void UPerformanceManager::SetPerformanceAlertsEnabled(bool bEnabled)
{
    bPerformanceAlertsEnabled = bEnabled;
}

void UPerformanceManager::GetScalabilitySettings(int32& ViewDistance, int32& AntiAliasing, int32& PostProcess, 
                                                int32& Shadows, int32& GlobalIllumination, int32& Reflections, 
                                                int32& Textures, int32& Effects) const
{
    const Scalability::FQualityLevels& QualityLevels = Scalability::GetQualityLevels();
    ViewDistance = QualityLevels.ViewDistanceQuality;
    AntiAliasing = QualityLevels.AntiAliasingQuality;
    PostProcess = QualityLevels.PostProcessQuality;
    Shadows = QualityLevels.ShadowQuality;
    GlobalIllumination = QualityLevels.GlobalIlluminationQuality;
    Reflections = QualityLevels.ReflectionQuality;
    Textures = QualityLevels.TextureQuality;
    Effects = QualityLevels.EffectsQuality;
}

void UPerformanceManager::ApplyScalabilityPreset(int32 QualityLevel)
{
    QualityLevel = FMath::Clamp(QualityLevel, 0, 4); // Epic has 5 levels (0-4)
    
    Scalability::FQualityLevels QualityLevels;
    QualityLevels.SetFromSingleQualityLevel(QualityLevel);
    Scalability::SetQualityLevels(QualityLevels);
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Applied scalability preset: %d"), QualityLevel);
}

void UPerformanceManager::InitializePerformanceBudgets()
{
    // Initialize default budgets for different targets
    // These can be overridden by project settings or configuration files
}

void UPerformanceManager::UpdatePerformanceMetrics()
{
    CollectSystemMetrics();
    
    // Add to history
    PerformanceHistory.Add(CurrentMetrics);
    if (PerformanceHistory.Num() > MaxHistorySamples)
    {
        PerformanceHistory.RemoveAt(0);
    }
    
    // Add to profiling session if active
    if (bProfilingActive)
    {
        SessionData.Add(CurrentMetrics);
    }
    
    // Check for performance alerts
    if (bPerformanceAlertsEnabled && !IsPerformanceWithinBudget())
    {
        TArray<FString> Violations = GetBudgetViolations();
        for (const FString& Violation : Violations)
        {
            LogPerformanceAlert(Violation);
        }
    }
}

void UPerformanceManager::CollectSystemMetrics()
{
    CurrentMetrics.Timestamp = FDateTime::Now();
    
    // Frame timing
    CurrentMetrics.FrameTimeMs = FPlatformTime::ToMilliseconds(GFrameTime) * 1000.0f;
    CurrentMetrics.FPS = CurrentMetrics.FrameTimeMs > 0.0f ? 1000.0f / CurrentMetrics.FrameTimeMs : 0.0f;
    
    // Thread timings (using stats system)
    CurrentMetrics.GameThreadMs = FPlatformTime::ToMilliseconds(GGameThreadTime) * 1000.0f;
    CurrentMetrics.RenderThreadMs = FPlatformTime::ToMilliseconds(GRenderThreadTime) * 1000.0f;
    CurrentMetrics.GPUTimeMs = FPlatformTime::ToMilliseconds(GGPUFrameTime) * 1000.0f;
    
    // Memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // VRAM usage (if available)
    if (GRHI)
    {
        CurrentMetrics.VRAMUsageMB = GRHI->GetGPUMemoryStats().DedicatedVideoMemory / (1024.0f * 1024.0f);
    }
    
    // Rendering stats (approximate values - would need more detailed RHI integration)
    CurrentMetrics.DrawCalls = 0; // Would need RHI stats
    CurrentMetrics.Triangles = 0; // Would need RHI stats
    
    // Physics and AI counts (would need integration with respective systems)
    CurrentMetrics.PhysicsBodies = 0; // Would need physics system integration
    CurrentMetrics.AIAgents = 0; // Would need AI system integration
}

void UPerformanceManager::CheckBudgetAndAdjustQuality()
{
    if (!bAutoQualityAdjustment)
    {
        return;
    }
    
    // Get average performance over last few seconds
    FPerformanceMetrics AvgMetrics = CalculateAverageMetrics(5.0f);
    
    // Calculate performance ratio (actual vs target)
    float PerformanceRatio = AvgMetrics.FrameTimeMs / CurrentBudget.TargetFrameTimeMs;
    
    if (PerformanceRatio > QualityDowngradeThreshold)
    {
        // Performance is poor, reduce quality
        AdjustScalabilitySettings(false);
        UE_LOG(LogPerformanceManager, Log, TEXT("Performance below target (%.2f ratio), reducing quality"), 
               PerformanceRatio);
    }
    else if (PerformanceRatio < QualityUpgradeThreshold)
    {
        // Performance is good, can increase quality
        AdjustScalabilitySettings(true);
        UE_LOG(LogPerformanceManager, Log, TEXT("Performance above target (%.2f ratio), increasing quality"), 
               PerformanceRatio);
    }
}

void UPerformanceManager::AdjustScalabilitySettings(bool bIncrease)
{
    Scalability::FQualityLevels CurrentLevels = Scalability::GetQualityLevels();
    
    int32 Adjustment = bIncrease ? 1 : -1;
    
    // Adjust settings with priority (most impactful first)
    CurrentLevels.EffectsQuality = FMath::Clamp(CurrentLevels.EffectsQuality + Adjustment, 0, 4);
    CurrentLevels.PostProcessQuality = FMath::Clamp(CurrentLevels.PostProcessQuality + Adjustment, 0, 4);
    CurrentLevels.ShadowQuality = FMath::Clamp(CurrentLevels.ShadowQuality + Adjustment, 0, 4);
    CurrentLevels.TextureQuality = FMath::Clamp(CurrentLevels.TextureQuality + Adjustment, 0, 4);
    CurrentLevels.ViewDistanceQuality = FMath::Clamp(CurrentLevels.ViewDistanceQuality + Adjustment, 0, 4);
    
    Scalability::SetQualityLevels(CurrentLevels);
}

FPerformanceBudget UPerformanceManager::GetPlatformBudget(EPerformanceTarget Target) const
{
    FPerformanceBudget Budget;
    
    switch (Target)
    {
        case EPerformanceTarget::PC_60FPS:
            Budget.TargetFrameTimeMs = 16.67f;
            Budget.MaxGameThreadMs = 12.0f;
            Budget.MaxRenderThreadMs = 12.0f;
            Budget.MaxGPUTimeMs = 14.0f;
            Budget.MaxMemoryMB = 8192.0f;
            Budget.MaxVRAMMB = 6144.0f;
            Budget.MaxDrawCalls = 5000;
            Budget.MaxTriangles = 10000000;
            break;
            
        case EPerformanceTarget::PC_120FPS:
            Budget.TargetFrameTimeMs = 8.33f;
            Budget.MaxGameThreadMs = 6.0f;
            Budget.MaxRenderThreadMs = 6.0f;
            Budget.MaxGPUTimeMs = 7.0f;
            Budget.MaxMemoryMB = 6144.0f;
            Budget.MaxVRAMMB = 4096.0f;
            Budget.MaxDrawCalls = 3000;
            Budget.MaxTriangles = 5000000;
            break;
            
        case EPerformanceTarget::Console_30FPS:
            Budget.TargetFrameTimeMs = 33.33f;
            Budget.MaxGameThreadMs = 25.0f;
            Budget.MaxRenderThreadMs = 25.0f;
            Budget.MaxGPUTimeMs = 30.0f;
            Budget.MaxMemoryMB = 12288.0f;
            Budget.MaxVRAMMB = 8192.0f;
            Budget.MaxDrawCalls = 8000;
            Budget.MaxTriangles = 20000000;
            break;
            
        case EPerformanceTarget::Console_60FPS:
            Budget.TargetFrameTimeMs = 16.67f;
            Budget.MaxGameThreadMs = 12.0f;
            Budget.MaxRenderThreadMs = 12.0f;
            Budget.MaxGPUTimeMs = 14.0f;
            Budget.MaxMemoryMB = 10240.0f;
            Budget.MaxVRAMMB = 6144.0f;
            Budget.MaxDrawCalls = 5000;
            Budget.MaxTriangles = 12000000;
            break;
            
        case EPerformanceTarget::Mobile_30FPS:
            Budget.TargetFrameTimeMs = 33.33f;
            Budget.MaxGameThreadMs = 20.0f;
            Budget.MaxRenderThreadMs = 20.0f;
            Budget.MaxGPUTimeMs = 25.0f;
            Budget.MaxMemoryMB = 2048.0f;
            Budget.MaxVRAMMB = 1024.0f;
            Budget.MaxDrawCalls = 1000;
            Budget.MaxTriangles = 1000000;
            break;
    }
    
    return Budget;
}

void UPerformanceManager::LogPerformanceAlert(const FString& Message)
{
    UE_LOG(LogPerformanceManager, Warning, TEXT("PERFORMANCE ALERT: %s"), *Message);
}

FPerformanceMetrics UPerformanceManager::CalculateAverageMetrics(float TimeWindow) const
{
    if (PerformanceHistory.Num() == 0)
    {
        return CurrentMetrics;
    }
    
    FDateTime CutoffTime = FDateTime::Now() - FTimespan::FromSeconds(TimeWindow);
    
    FPerformanceMetrics AverageMetrics;
    int32 SampleCount = 0;
    
    for (const FPerformanceMetrics& Metrics : PerformanceHistory)
    {
        if (Metrics.Timestamp >= CutoffTime)
        {
            AverageMetrics.FrameTimeMs += Metrics.FrameTimeMs;
            AverageMetrics.FPS += Metrics.FPS;
            AverageMetrics.GameThreadMs += Metrics.GameThreadMs;
            AverageMetrics.RenderThreadMs += Metrics.RenderThreadMs;
            AverageMetrics.GPUTimeMs += Metrics.GPUTimeMs;
            AverageMetrics.MemoryUsageMB += Metrics.MemoryUsageMB;
            AverageMetrics.VRAMUsageMB += Metrics.VRAMUsageMB;
            AverageMetrics.DrawCalls += Metrics.DrawCalls;
            AverageMetrics.Triangles += Metrics.Triangles;
            AverageMetrics.PhysicsBodies += Metrics.PhysicsBodies;
            AverageMetrics.AIAgents += Metrics.AIAgents;
            SampleCount++;
        }
    }
    
    if (SampleCount > 0)
    {
        float InvSampleCount = 1.0f / SampleCount;
        AverageMetrics.FrameTimeMs *= InvSampleCount;
        AverageMetrics.FPS *= InvSampleCount;
        AverageMetrics.GameThreadMs *= InvSampleCount;
        AverageMetrics.RenderThreadMs *= InvSampleCount;
        AverageMetrics.GPUTimeMs *= InvSampleCount;
        AverageMetrics.MemoryUsageMB *= InvSampleCount;
        AverageMetrics.VRAMUsageMB *= InvSampleCount;
        AverageMetrics.DrawCalls = FMath::RoundToInt(AverageMetrics.DrawCalls * InvSampleCount);
        AverageMetrics.Triangles = FMath::RoundToInt(AverageMetrics.Triangles * InvSampleCount);
        AverageMetrics.PhysicsBodies = FMath::RoundToInt(AverageMetrics.PhysicsBodies * InvSampleCount);
        AverageMetrics.AIAgents = FMath::RoundToInt(AverageMetrics.AIAgents * InvSampleCount);
    }
    
    AverageMetrics.Timestamp = FDateTime::Now();
    return AverageMetrics;
}

void UPerformanceManager::CleanupPerformanceHistory()
{
    FDateTime CutoffTime = FDateTime::Now() - FTimespan::FromSeconds(PerformanceHistoryWindow);
    
    PerformanceHistory.RemoveAll([CutoffTime](const FPerformanceMetrics& Metrics)
    {
        return Metrics.Timestamp < CutoffTime;
    });
}