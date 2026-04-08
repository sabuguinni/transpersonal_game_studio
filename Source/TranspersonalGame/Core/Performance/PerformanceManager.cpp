// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformMemory.h"
#include "RHI.h"
#include "RenderingThread.h"
#include "Stats/StatsData.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/ConfigCacheIni.h"

DECLARE_CYCLE_STAT(TEXT("Performance Manager Tick"), STAT_PerformanceManagerTick, STATGROUP_Game);
DECLARE_DWORD_COUNTER_STAT(TEXT("Draw Calls"), STAT_DrawCalls, STATGROUP_Rendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("Triangle Count"), STAT_TriangleCount, STATGROUP_Rendering);

UPerformanceManager::UPerformanceManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // Tick every frame
    PrimaryComponentTick.bTickEvenWhenPaused = false;
    
    // Initialize default quality settings
    CurrentQualitySettings.ScreenPercentage = 100.0f;
    CurrentQualitySettings.ShadowQuality = 3;
    CurrentQualitySettings.PostProcessQuality = 3;
    CurrentQualitySettings.TextureQuality = 3;
    CurrentQualitySettings.EffectsQuality = 3;
    CurrentQualitySettings.ViewDistanceScale = 1.0f;
    CurrentQualitySettings.PhysicsLODBias = 0;
    CurrentQualitySettings.AIUpdateScale = 1.0f;
    
    // Reserve space for frame rate history
    FrameRateHistory.Reserve(120); // 2 seconds at 60fps
}

void UPerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Initializing performance optimization system"));
    
    DetectPlatformProfile();
    InitializePerformanceSystem();
    
    // Register core systems with their budgets
    RegisterCriticalSystem(TEXT("Physics"), PhysicsBudgetMs);
    RegisterCriticalSystem(TEXT("AI"), AIBudgetMs);
    RegisterCriticalSystem(TEXT("Rendering"), RenderThreadBudgetMs);
    RegisterCriticalSystem(TEXT("GameLogic"), GameThreadBudgetMs);
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: System initialized for %s profile, target %d fps"), 
           *UEnum::GetValueAsString(CurrentPlatformProfile), TargetFrameRate);
}

void UPerformanceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    SCOPE_CYCLE_COUNTER(STAT_PerformanceManagerTick);
    
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Update monitoring timer
    MonitoringTimer += DeltaTime;
    
    // Check if we need to perform optimization
    if (MonitoringTimer >= MonitoringFrequency)
    {
        MonitoringTimer = 0.0f;
        
        if (ShouldOptimizePerformance())
        {
            ApplyPerformanceOptimizations();
        }
        
        if (bEnablePerformanceLogging)
        {
            LogPerformanceData();
        }
    }
}

void UPerformanceManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Close performance log file if open
    if (PerformanceLogFile.IsValid())
    {
        PerformanceLogFile->Close();
        PerformanceLogFile.Reset();
    }
    
    Super::EndPlay(EndPlayReason);
}

void UPerformanceManager::InitializePerformanceSystem()
{
    // Set target frame rate based on platform
    switch (CurrentPlatformProfile)
    {
        case EPlatformProfile::HighEndPC:
        case EPlatformProfile::MidRangePC:
            SetTargetFrameRate(60, true);
            break;
            
        case EPlatformProfile::NextGenConsole:
            SetTargetFrameRate(30, true);
            break;
            
        case EPlatformProfile::LowEndPC:
        case EPlatformProfile::Mobile:
            SetTargetFrameRate(30, true);
            // Start with reduced quality settings
            CurrentQualitySettings.ScreenPercentage = 75.0f;
            CurrentQualitySettings.ShadowQuality = 1;
            CurrentQualitySettings.PostProcessQuality = 1;
            CurrentQualitySettings.ViewDistanceScale = 0.7f;
            break;
    }
    
    // Apply initial quality settings
    ApplyQualitySettings();
    
    // Initialize performance logging if enabled
    if (bEnablePerformanceLogging)
    {
        FString LogFileName = FString::Printf(TEXT("PerformanceLog_%s.csv"), 
                                            *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
        FString LogFilePath = FPaths::ProjectLogDir() / LogFileName;
        
        PerformanceLogFile = TSharedPtr<FArchive>(IFileManager::Get().CreateFileWriter(*LogFilePath));
        if (PerformanceLogFile.IsValid())
        {
            FString Header = TEXT("Timestamp,FPS,FrameTime,GameThread,RenderThread,GPU,Physics,AI,Memory,DrawCalls,Triangles,PerformanceHealth\\n");
            PerformanceLogFile->Serialize(TCHAR_TO_UTF8(*Header), Header.Len());
        }
    }
}

FPerformanceMetrics UPerformanceManager::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UPerformanceManager::SetTargetFrameRate(int32 TargetFPS, bool bEnableAdaptiveScaling)
{
    TargetFrameRate = TargetFPS;
    TargetFrameTimeMs = 1000.0f / TargetFPS;
    this->bEnableAdaptiveScaling = bEnableAdaptiveScaling;
    
    // Update engine frame rate settings
    if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
    {
        UserSettings->SetFrameRateLimit(TargetFPS);
        UserSettings->ApplySettings(false);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Target frame rate set to %d fps (%.2f ms)"), 
           TargetFPS, TargetFrameTimeMs);
}

void UPerformanceManager::ForceOptimizationPass()
{
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Forcing optimization pass"));
    ApplyPerformanceOptimizations();
}

void UPerformanceManager::RegisterCriticalSystem(const FString& SystemName, float BudgetMs)
{
    SystemBudgets.Add(SystemName, BudgetMs);
    SystemPerformance.Add(SystemName, 0.0f);
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Registered system '%s' with budget %.2f ms"), 
           *SystemName, BudgetMs);
}

void UPerformanceManager::ReportSystemPerformance(const FString& SystemName, float ExecutionTimeMs)
{
    if (SystemPerformance.Contains(SystemName))
    {
        SystemPerformance[SystemName] = ExecutionTimeMs;
        
        // Check if system is over budget
        if (SystemBudgets.Contains(SystemName))
        {
            float Budget = SystemBudgets[SystemName];
            if (ExecutionTimeMs > Budget * 1.2f) // 20% tolerance
            {
                UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: System '%s' over budget: %.2f ms (budget: %.2f ms)"), 
                       *SystemName, ExecutionTimeMs, Budget);
            }
        }
    }
}

FQualitySettings UPerformanceManager::GetRecommendedQualitySettings() const
{
    return CurrentQualitySettings;
}

void UPerformanceManager::DetectPlatformProfile()
{
    // Detect platform based on hardware capabilities
    FString PlatformName = UGameplayStatics::GetPlatformName();
    
    if (PlatformName.Contains(TEXT("Windows")) || PlatformName.Contains(TEXT("Mac")) || PlatformName.Contains(TEXT("Linux")))
    {
        // PC platform - detect performance tier
        uint32 TotalPhysicalMemory = FPlatformMemory::GetConstants().TotalPhysicalGB;
        
        if (TotalPhysicalMemory >= 16)
        {
            CurrentPlatformProfile = EPlatformProfile::HighEndPC;
        }
        else if (TotalPhysicalMemory >= 8)
        {
            CurrentPlatformProfile = EPlatformProfile::MidRangePC;
        }
        else
        {
            CurrentPlatformProfile = EPlatformProfile::LowEndPC;
        }
    }
    else if (PlatformName.Contains(TEXT("XSX")) || PlatformName.Contains(TEXT("PS5")))
    {
        CurrentPlatformProfile = EPlatformProfile::NextGenConsole;
    }
    else
    {
        CurrentPlatformProfile = EPlatformProfile::Mobile;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Detected platform profile: %s"), 
           *UEnum::GetValueAsString(CurrentPlatformProfile));
}

void UPerformanceManager::UpdatePerformanceMetrics()
{
    // Get current frame rate
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        float CurrentFPS = 1.0f / DeltaTime;
        CurrentMetrics.CurrentFPS = CurrentFPS;
        CurrentMetrics.FrameTimeMs = DeltaTime * 1000.0f;
        
        // Add to history for averaging
        FrameRateHistory.Add(CurrentFPS);
        if (FrameRateHistory.Num() > 120) // Keep last 2 seconds
        {
            FrameRateHistory.RemoveAt(0);
        }
    }
    
    // Gather engine stats
    GatherEngineStats();
    
    // Calculate performance health (0-100)
    float TargetFPS = static_cast<float>(TargetFrameRate);
    float HealthRatio = FMath::Clamp(CurrentMetrics.CurrentFPS / TargetFPS, 0.0f, 1.0f);
    CurrentMetrics.PerformanceHealth = HealthRatio * 100.0f;
    
    // Track consecutive frames above/below target
    if (CurrentMetrics.CurrentFPS < TargetFPS * 0.9f) // 10% tolerance
    {
        FramesBelowTarget++;
        FramesAboveTarget = 0;
    }
    else if (CurrentMetrics.CurrentFPS > TargetFPS * 1.1f)
    {
        FramesAboveTarget++;
        FramesBelowTarget = 0;
    }
    else
    {
        FramesBelowTarget = 0;
        FramesAboveTarget = 0;
    }
}

bool UPerformanceManager::ShouldOptimizePerformance() const
{
    if (!bEnableAdaptiveScaling)
    {
        return false;
    }
    
    // Optimize if we've had too many frames below target
    if (FramesBelowTarget >= FramesToleranceBeforeScaling)
    {
        return true;
    }
    
    // Also check if any critical system is over budget
    for (const auto& SystemPair : SystemPerformance)
    {
        const FString& SystemName = SystemPair.Key;
        float Performance = SystemPair.Value;
        
        if (SystemBudgets.Contains(SystemName))
        {
            float Budget = SystemBudgets[SystemName];
            if (Performance > Budget * 1.5f) // 50% over budget
            {
                return true;
            }
        }
    }
    
    return false;
}

void UPerformanceManager::ApplyPerformanceOptimizations()
{
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Applying performance optimizations"));
    
    float CurrentFPS = CurrentMetrics.CurrentFPS;
    float TargetFPS = static_cast<float>(TargetFrameRate);
    
    // Calculate scaling factor based on performance deficit
    float PerformanceRatio = CurrentFPS / TargetFPS;
    float ScaleFactor = FMath::Clamp(PerformanceRatio, 0.5f, 1.0f);
    
    // Apply quality scaling
    ScaleQualitySettings(ScaleFactor);
    
    // Apply engine-level optimizations
    ApplyEngineOptimizations();
    
    LastOptimizationTime = GetWorld()->GetTimeSeconds();
    FramesBelowTarget = 0; // Reset counter after optimization
}

void UPerformanceManager::ScaleQualitySettings(float ScaleFactor)
{
    // Scale screen percentage
    float NewScreenPercentage = FMath::Clamp(
        CurrentQualitySettings.ScreenPercentage * ScaleFactor, 
        MinScreenPercentage, 
        100.0f
    );
    CurrentQualitySettings.ScreenPercentage = NewScreenPercentage;
    
    // Scale view distance
    CurrentQualitySettings.ViewDistanceScale = FMath::Clamp(ScaleFactor, 0.3f, 1.0f);
    
    // Adjust quality levels based on performance
    if (ScaleFactor < 0.7f)
    {
        CurrentQualitySettings.ShadowQuality = FMath::Max(0, CurrentQualitySettings.ShadowQuality - 1);
        CurrentQualitySettings.PostProcessQuality = FMath::Max(0, CurrentQualitySettings.PostProcessQuality - 1);
        CurrentQualitySettings.EffectsQuality = FMath::Max(0, CurrentQualitySettings.EffectsQuality - 1);
    }
    else if (ScaleFactor > 1.1f && FramesAboveTarget > 60) // 1 second of good performance
    {
        // Gradually increase quality if performance allows
        CurrentQualitySettings.ShadowQuality = FMath::Min(3, CurrentQualitySettings.ShadowQuality + 1);
        CurrentQualitySettings.PostProcessQuality = FMath::Min(3, CurrentQualitySettings.PostProcessQuality + 1);
    }
    
    ApplyQualitySettings();
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Quality scaled - Screen: %.1f%%, ViewDist: %.2f, Shadow: %d"), 
           NewScreenPercentage, CurrentQualitySettings.ViewDistanceScale, CurrentQualitySettings.ShadowQuality);
}

void UPerformanceManager::ApplyQualitySettings()
{
    // Apply screen percentage
    static IConsoleVariable* ScreenPercentageCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
    if (ScreenPercentageCVar)
    {
        ScreenPercentageCVar->Set(CurrentQualitySettings.ScreenPercentage);
    }
    
    // Apply shadow quality
    static IConsoleVariable* ShadowQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality"));
    if (ShadowQualityCVar)
    {
        ShadowQualityCVar->Set(CurrentQualitySettings.ShadowQuality);
    }
    
    // Apply post-process quality
    static IConsoleVariable* PostProcessCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.PostProcessQuality"));
    if (PostProcessCVar)
    {
        PostProcessCVar->Set(CurrentQualitySettings.PostProcessQuality);
    }
    
    // Apply effects quality
    static IConsoleVariable* EffectsCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality"));
    if (EffectsCVar)
    {
        EffectsCVar->Set(CurrentQualitySettings.EffectsQuality);
    }
    
    // Apply view distance scale
    static IConsoleVariable* ViewDistanceCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale"));
    if (ViewDistanceCVar)
    {
        ViewDistanceCVar->Set(CurrentQualitySettings.ViewDistanceScale);
    }
}

void UPerformanceManager::ApplyEngineOptimizations()
{
    // Apply physics optimizations if physics is over budget
    if (SystemPerformance.Contains(TEXT("Physics")))
    {
        float PhysicsTime = SystemPerformance[TEXT("Physics")];
        if (PhysicsTime > PhysicsBudgetMs * 1.2f)
        {
            // Reduce physics simulation frequency
            static IConsoleVariable* PhysicsSubstepsCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxSubsteps"));
            if (PhysicsSubstepsCVar)
            {
                int32 CurrentSubsteps = PhysicsSubstepsCVar->GetInt();
                PhysicsSubstepsCVar->Set(FMath::Max(1, CurrentSubsteps - 1));
            }
        }
    }
    
    // Apply AI optimizations if AI is over budget
    if (SystemPerformance.Contains(TEXT("AI")))
    {
        float AITime = SystemPerformance[TEXT("AI")];
        if (AITime > AIBudgetMs * 1.2f)
        {
            // Reduce AI update frequency
            CurrentQualitySettings.AIUpdateScale = FMath::Max(0.5f, CurrentQualitySettings.AIUpdateScale * 0.9f);
        }
    }
}

void UPerformanceManager::LogPerformanceData()
{
    if (PerformanceLogFile.IsValid())
    {
        FString LogEntry = FString::Printf(
            TEXT("%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%.2f\\n"),
            *FDateTime::Now().ToString(),
            CurrentMetrics.CurrentFPS,
            CurrentMetrics.FrameTimeMs,
            CurrentMetrics.GameThreadMs,
            CurrentMetrics.RenderThreadMs,
            CurrentMetrics.GPUTimeMs,
            CurrentMetrics.PhysicsTimeMs,
            CurrentMetrics.AITimeMs,
            CurrentMetrics.MemoryUsageMB,
            CurrentMetrics.DrawCalls,
            CurrentMetrics.TriangleCount,
            CurrentMetrics.PerformanceHealth
        );
        
        PerformanceLogFile->Serialize(TCHAR_TO_UTF8(*LogEntry), LogEntry.Len());
        PerformanceLogFile->Flush();
    }
}

void UPerformanceManager::GatherEngineStats()
{
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
    
    // Get rendering stats (simplified - in real implementation would use proper stat gathering)
    CurrentMetrics.DrawCalls = 1000; // Placeholder - would get from actual rendering stats
    CurrentMetrics.TriangleCount = 500000; // Placeholder
    
    // Thread times (simplified - would use actual profiling data)
    CurrentMetrics.GameThreadMs = 8.0f; // Placeholder
    CurrentMetrics.RenderThreadMs = 10.0f; // Placeholder
    CurrentMetrics.GPUTimeMs = 12.0f; // Placeholder
    
    // Get system performance if available
    if (SystemPerformance.Contains(TEXT("Physics")))
    {
        CurrentMetrics.PhysicsTimeMs = SystemPerformance[TEXT("Physics")];
    }
    
    if (SystemPerformance.Contains(TEXT("AI")))
    {
        CurrentMetrics.AITimeMs = SystemPerformance[TEXT("AI")];
    }
}