// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/ConfigCacheIni.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"
#include "Scalability.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogPerformanceManager, Log, All);

UPerformanceManager::UPerformanceManager()
{
    // Initialize default performance budgets (in milliseconds)
    PerformanceBudgets.Add(EPerformanceBudget::GameThread, 10.0f);     // 10ms for gameplay logic
    PerformanceBudgets.Add(EPerformanceBudget::RenderThread, 8.0f);    // 8ms for render commands
    PerformanceBudgets.Add(EPerformanceBudget::GPU, 14.0f);           // 14ms for GPU rendering
    PerformanceBudgets.Add(EPerformanceBudget::Memory, 12000.0f);     // 12GB RAM budget
    PerformanceBudgets.Add(EPerformanceBudget::VRAM, 6000.0f);        // 6GB VRAM budget
    PerformanceBudgets.Add(EPerformanceBudget::Streaming, 100.0f);    // 100MB/s streaming
}

void UPerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CachedEngine = GEngine;
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance Manager initialized"));
    
    // Detect hardware and set appropriate target
    DetectHardwareCapabilities();
    
    // Initialize performance monitoring
    if (bEnablePerformanceMonitoring)
    {
        InitializePerformanceTargets();
        RegisterConsoleCommands();
        
        // Start monitoring timer
        GetWorld()->GetTimerManager().SetTimer(
            PerformanceMonitoringTimer,
            FTimerDelegate::CreateUObject(this, &UPerformanceManager::UpdateFrameTimeTracking),
            0.1f, // Update every 100ms
            true
        );
    }
}

void UPerformanceManager::Deinitialize()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(PerformanceMonitoringTimer);
    }
    
    Super::Deinitialize();
}

void UPerformanceManager::InitializePerformanceTargets()
{
    InitializeBudgets();
    ApplyPerformanceOptimizations();
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance targets initialized for: %s"), 
           *UEnum::GetValueAsString(CurrentTarget));
}

void UPerformanceManager::SetPerformanceTarget(EPerformanceTarget Target)
{
    CurrentTarget = Target;
    
    switch (Target)
    {
        case EPerformanceTarget::HighEndPC_60fps:
            TargetFrameTime = 16.67f; // 60fps
            break;
        case EPerformanceTarget::MidRangePC_60fps:
            TargetFrameTime = 16.67f; // 60fps with reduced quality
            break;
        case EPerformanceTarget::Console_30fps:
            TargetFrameTime = 33.33f; // 30fps
            break;
        case EPerformanceTarget::Console_60fps:
            TargetFrameTime = 16.67f; // 60fps performance mode
            break;
        case EPerformanceTarget::Potato_30fps:
            TargetFrameTime = 33.33f; // 30fps minimum spec
            break;
    }
    
    InitializeBudgets();
    ApplyRecommendedScalabilitySettings();
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance target set to: %s (%.2fms target)"), 
           *UEnum::GetValueAsString(Target), TargetFrameTime);
}

float UPerformanceManager::GetCurrentFrameTime() const
{
    if (CachedEngine && CachedEngine->GetGameViewport())
    {
        // Get frame time from engine
        const float CurrentTime = FPlatformTime::Seconds();
        if (CurrentTime != LastUpdateTime)
        {
            CachedFrameTime = (CurrentTime - LastUpdateTime) * 1000.0f; // Convert to ms
            LastUpdateTime = CurrentTime;
        }
    }
    
    return CachedFrameTime;
}

float UPerformanceManager::GetCurrentFPS() const
{
    const float FrameTime = GetCurrentFrameTime();
    CachedFPS = (FrameTime > 0.0f) ? (1000.0f / FrameTime) : 0.0f;
    return CachedFPS;
}

bool UPerformanceManager::IsMeetingPerformanceTargets() const
{
    const float CurrentFrameTime = GetCurrentFrameTime();
    return CurrentFrameTime <= TargetFrameTime * 1.1f; // 10% tolerance
}

float UPerformanceManager::GetBudgetUsage(EPerformanceBudget Budget) const
{
    const float* BudgetPtr = PerformanceBudgets.Find(Budget);
    if (!BudgetPtr)
    {
        return 0.0f;
    }
    
    float CurrentUsage = 0.0f;
    
    switch (Budget)
    {
        case EPerformanceBudget::GameThread:
            // Get game thread time from stats
            CurrentUsage = FPlatformTime::ToMilliseconds(GGameThreadTime);
            break;
            
        case EPerformanceBudget::RenderThread:
            // Get render thread time from stats
            CurrentUsage = FPlatformTime::ToMilliseconds(GRenderThreadTime);
            break;
            
        case EPerformanceBudget::GPU:
            // Get GPU time from stats
            CurrentUsage = FPlatformTime::ToMilliseconds(GGPUFrameTime);
            break;
            
        case EPerformanceBudget::Memory:
            // Get memory usage
            CurrentUsage = FPlatformMemory::GetStats().UsedPhysical / (1024 * 1024); // MB
            break;
            
        case EPerformanceBudget::VRAM:
            // Get VRAM usage (approximation)
            CurrentUsage = GEngine->GetTextureMemoryUsed() / (1024 * 1024); // MB
            break;
            
        case EPerformanceBudget::Streaming:
            // Streaming bandwidth usage
            CurrentUsage = 50.0f; // Placeholder - would need streaming system integration
            break;
    }
    
    return FMath::Clamp(CurrentUsage / *BudgetPtr, 0.0f, 2.0f); // Return as percentage (can exceed 100%)
}

void UPerformanceManager::AdjustLODForPerformance(float PerformanceScale)
{
    // Adjust LOD bias based on performance
    const float LODBias = FMath::Lerp(0.0f, 2.0f, 1.0f - PerformanceScale);
    
    // Apply to static mesh LOD
    static IConsoleVariable* StaticMeshLODBias = IConsoleManager::Get().FindConsoleVariable(TEXT("r.StaticMeshLODBias"));
    if (StaticMeshLODBias)
    {
        StaticMeshLODBias->Set(LODBias);
    }
    
    // Apply to skeletal mesh LOD
    static IConsoleVariable* SkeletalMeshLODBias = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SkeletalMeshLODBias"));
    if (SkeletalMeshLODBias)
    {
        SkeletalMeshLODBias->Set(LODBias);
    }
    
    UE_LOG(LogPerformanceManager, Log, TEXT("LOD adjusted for performance: LODBias=%.2f"), LODBias);
}

void UPerformanceManager::ScaleVisualQuality(float QualityScale)
{
    // Clamp quality scale
    QualityScale = FMath::Clamp(QualityScale, 0.1f, 1.0f);
    
    // Adjust screen percentage
    const float ScreenPercentage = FMath::Lerp(50.0f, 100.0f, QualityScale);
    static IConsoleVariable* ScreenPercentageVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
    if (ScreenPercentageVar)
    {
        ScreenPercentageVar->Set(ScreenPercentage);
    }
    
    // Adjust shadow quality
    const int32 ShadowQuality = FMath::RoundToInt(FMath::Lerp(0.0f, 3.0f, QualityScale));
    static IConsoleVariable* ShadowQualityVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality"));
    if (ShadowQualityVar)
    {
        ShadowQualityVar->Set(ShadowQuality);
    }
    
    // Adjust post-process quality
    const int32 PostProcessQuality = FMath::RoundToInt(FMath::Lerp(0.0f, 3.0f, QualityScale));
    static IConsoleVariable* PostProcessVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.PostProcessQuality"));
    if (PostProcessVar)
    {
        PostProcessVar->Set(PostProcessQuality);
    }
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Visual quality scaled: %.2f (Screen: %.1f%%, Shadows: %d, PostProcess: %d)"), 
           QualityScale, ScreenPercentage, ShadowQuality, PostProcessQuality);
}

void UPerformanceManager::ApplyRecommendedScalabilitySettings()
{
    Scalability::FQualityLevels QualityLevels;
    
    switch (CurrentTarget)
    {
        case EPerformanceTarget::HighEndPC_60fps:
            QualityLevels = Scalability::FQualityLevels(3, 3, 3, 3, 3, 3); // Epic quality
            break;
        case EPerformanceTarget::MidRangePC_60fps:
            QualityLevels = Scalability::FQualityLevels(2, 2, 2, 2, 2, 2); // High quality
            break;
        case EPerformanceTarget::Console_30fps:
            QualityLevels = Scalability::FQualityLevels(3, 3, 2, 3, 2, 3); // Mixed for console
            break;
        case EPerformanceTarget::Console_60fps:
            QualityLevels = Scalability::FQualityLevels(2, 2, 1, 2, 1, 2); // Performance mode
            break;
        case EPerformanceTarget::Potato_30fps:
            QualityLevels = Scalability::FQualityLevels(0, 0, 0, 1, 0, 1); // Low quality
            break;
    }
    
    Scalability::SetQualityLevels(QualityLevels);
    Scalability::SaveState();
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Applied scalability settings for target: %s"), 
           *UEnum::GetValueAsString(CurrentTarget));
}

void UPerformanceManager::InitializeBudgets()
{
    // Adjust budgets based on target
    switch (CurrentTarget)
    {
        case EPerformanceTarget::HighEndPC_60fps:
            PerformanceBudgets[EPerformanceBudget::GameThread] = 8.0f;
            PerformanceBudgets[EPerformanceBudget::RenderThread] = 6.0f;
            PerformanceBudgets[EPerformanceBudget::GPU] = 12.0f;
            break;
            
        case EPerformanceTarget::Console_30fps:
            PerformanceBudgets[EPerformanceBudget::GameThread] = 20.0f;
            PerformanceBudgets[EPerformanceBudget::RenderThread] = 15.0f;
            PerformanceBudgets[EPerformanceBudget::GPU] = 28.0f;
            break;
            
        case EPerformanceTarget::Potato_30fps:
            PerformanceBudgets[EPerformanceBudget::GameThread] = 25.0f;
            PerformanceBudgets[EPerformanceBudget::RenderThread] = 20.0f;
            PerformanceBudgets[EPerformanceBudget::GPU] = 30.0f;
            PerformanceBudgets[EPerformanceBudget::Memory] = 6000.0f; // 6GB
            PerformanceBudgets[EPerformanceBudget::VRAM] = 3000.0f;   // 3GB
            break;
    }
}

void UPerformanceManager::UpdateFrameTimeTracking()
{
    if (!bEnablePerformanceMonitoring)
    {
        return;
    }
    
    const float CurrentFrameTime = GetCurrentFrameTime();
    
    // Add to history
    FrameTimeHistory.Add(CurrentFrameTime);
    
    // Maintain history size
    if (FrameTimeHistory.Num() > MaxFrameHistorySamples)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Auto-adjust quality if enabled
    if (bAutoAdjustQuality)
    {
        const float CurrentFPS = GetCurrentFPS();
        if (CurrentFPS < MinAcceptableFPS)
        {
            const float PerformanceRatio = CurrentFPS / (1000.0f / TargetFrameTime);
            ScaleVisualQuality(PerformanceRatio * 0.9f); // Reduce quality by 10% more than needed
        }
    }
}

void UPerformanceManager::DetectHardwareCapabilities()
{
    // Simple hardware detection - in real implementation would be more sophisticated
    const FString GPUBrand = FPlatformApplicationMisc::GetGPUBrand();
    const uint32 MemoryMB = FPlatformMemory::GetConstants().TotalPhysicalGB * 1024;
    
    // Set default target based on detected hardware
    if (MemoryMB >= 16384 && (GPUBrand.Contains(TEXT("RTX")) || GPUBrand.Contains(TEXT("RX 6"))))
    {
        SetPerformanceTarget(EPerformanceTarget::HighEndPC_60fps);
    }
    else if (MemoryMB >= 8192)
    {
        SetPerformanceTarget(EPerformanceTarget::MidRangePC_60fps);
    }
    else
    {
        SetPerformanceTarget(EPerformanceTarget::Potato_30fps);
    }
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Hardware detected: GPU=%s, RAM=%dGB, Target=%s"), 
           *GPUBrand, MemoryMB/1024, *UEnum::GetValueAsString(CurrentTarget));
}

void UPerformanceManager::ApplyPerformanceOptimizations()
{
    // Apply Jurassic-specific optimizations
    
    // Enable Nanite for high-poly dinosaur meshes
    static IConsoleVariable* NaniteVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite"));
    if (NaniteVar && CurrentTarget != EPerformanceTarget::Potato_30fps)
    {
        NaniteVar->Set(1);
    }
    
    // Configure Lumen for dynamic lighting
    static IConsoleVariable* LumenVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicGlobalIllumination"));
    if (LumenVar && CurrentTarget == EPerformanceTarget::HighEndPC_60fps)
    {
        LumenVar->Set(1);
    }
    
    // Set appropriate view distance for open world
    const float ViewDistance = (CurrentTarget == EPerformanceTarget::HighEndPC_60fps) ? 1.0f : 0.7f;
    static IConsoleVariable* ViewDistanceVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale"));
    if (ViewDistanceVar)
    {
        ViewDistanceVar->Set(ViewDistance);
    }
    
    UE_LOG(LogPerformanceManager, Log, TEXT("Performance optimizations applied for Jurassic world"));
}

void UPerformanceManager::RegisterConsoleCommands()
{
    // Register debug console commands
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("perf.ShowTargets"),
        TEXT("Show current performance targets and budget usage"),
        FConsoleCommandDelegate::CreateUObject(this, [this]()
        {
            UE_LOG(LogPerformanceManager, Warning, TEXT("=== PERFORMANCE TARGETS ==="));
            UE_LOG(LogPerformanceManager, Warning, TEXT("Target: %s (%.2fms)"), 
                   *UEnum::GetValueAsString(CurrentTarget), TargetFrameTime);
            UE_LOG(LogPerformanceManager, Warning, TEXT("Current FPS: %.1f"), GetCurrentFPS());
            UE_LOG(LogPerformanceManager, Warning, TEXT("Meeting Targets: %s"), 
                   IsMeetingPerformanceTargets() ? TEXT("YES") : TEXT("NO"));
            
            for (const auto& Budget : PerformanceBudgets)
            {
                const float Usage = GetBudgetUsage(Budget.Key);
                UE_LOG(LogPerformanceManager, Warning, TEXT("%s: %.1f%% (%.2f/%.2f)"), 
                       *UEnum::GetValueAsString(Budget.Key), Usage * 100.0f, 
                       Usage * Budget.Value, Budget.Value);
            }
        })
    );
}