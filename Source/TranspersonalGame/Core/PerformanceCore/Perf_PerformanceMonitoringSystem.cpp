#include "Perf_PerformanceMonitoringSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/StatsHierarchical.h"
#include "RenderingThread.h"
#include "HAL/IConsoleManager.h"

DEFINE_STAT(STAT_PhysicsUpdateTime);
DEFINE_STAT(STAT_AIProcessingTime);
DEFINE_STAT(STAT_RenderingTime);
DEFINE_STAT(STAT_AnimationUpdateTime);
DEFINE_STAT(STAT_ActiveActors);
DEFINE_STAT(STAT_VisiblePrimitives);
DEFINE_STAT(STAT_DrawCalls);
DEFINE_STAT(STAT_PhysicsMemory);
DEFINE_STAT(STAT_AnimationMemory);
DEFINE_STAT(STAT_TextureMemory);

APerf_PerformanceMonitoringSystem* APerf_PerformanceMonitoringSystem::Instance = nullptr;

UPerf_PerformanceMonitoringComponent::UPerf_PerformanceMonitoringComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    OptimizationMode = EPerf_OptimizationMode::Automatic;
    TargetPerformanceLevel = EPerf_PerformanceLevel::High;
    bMonitoringEnabled = true;
    MonitoringInterval = 1.0f;
    LastMonitoringTime = 0.0f;
    
    FrameTimeHistory.Reserve(60); // Keep 60 frames of history
}

void UPerf_PerformanceMonitoringComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (bMonitoringEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance Monitoring Component initialized"));
        
        // Register with global performance system
        if (APerf_PerformanceMonitoringSystem* PerfSystem = APerf_PerformanceMonitoringSystem::GetPerformanceMonitoringSystem(GetWorld()))
        {
            PerfSystem->RegisterPerformanceComponent(this);
        }
    }
}

void UPerf_PerformanceMonitoringComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bMonitoringEnabled)
    {
        return;
    }
    
    LastMonitoringTime += DeltaTime;
    
    if (LastMonitoringTime >= MonitoringInterval)
    {
        UpdatePerformanceMetrics();
        
        if (OptimizationMode == EPerf_OptimizationMode::Automatic)
        {
            ApplyOptimizations();
        }
        
        LastMonitoringTime = 0.0f;
    }
}

FPerf_PerformanceMetrics UPerf_PerformanceMonitoringComponent::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PerformanceMonitoringComponent::SetOptimizationMode(EPerf_OptimizationMode Mode)
{
    OptimizationMode = Mode;
    UE_LOG(LogTemp, Log, TEXT("Performance optimization mode set to: %d"), (int32)Mode);
}

void UPerf_PerformanceMonitoringComponent::ApplyPerformanceLevel(EPerf_PerformanceLevel Level)
{
    TargetPerformanceLevel = Level;
    
    switch (Level)
    {
    case EPerf_PerformanceLevel::Low:
        OptimizationSettings.LODBias = 2.0f;
        OptimizationSettings.ShadowDistanceScale = 0.5f;
        OptimizationSettings.ParticleSystemScale = 0.5f;
        OptimizationSettings.ScreenPercentage = 75.0f;
        break;
        
    case EPerf_PerformanceLevel::Medium:
        OptimizationSettings.LODBias = 1.0f;
        OptimizationSettings.ShadowDistanceScale = 0.75f;
        OptimizationSettings.ParticleSystemScale = 0.75f;
        OptimizationSettings.ScreenPercentage = 85.0f;
        break;
        
    case EPerf_PerformanceLevel::High:
        OptimizationSettings.LODBias = 0.0f;
        OptimizationSettings.ShadowDistanceScale = 1.0f;
        OptimizationSettings.ParticleSystemScale = 1.0f;
        OptimizationSettings.ScreenPercentage = 100.0f;
        break;
        
    case EPerf_PerformanceLevel::Ultra:
        OptimizationSettings.LODBias = -1.0f;
        OptimizationSettings.ShadowDistanceScale = 1.5f;
        OptimizationSettings.ParticleSystemScale = 1.25f;
        OptimizationSettings.ScreenPercentage = 100.0f;
        break;
    }
    
    ApplyOptimizations();
    UE_LOG(LogTemp, Log, TEXT("Applied performance level: %d"), (int32)Level);
}

void UPerf_PerformanceMonitoringComponent::EnablePerformanceMonitoring(bool bEnable)
{
    bMonitoringEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_PerformanceMonitoringComponent::LogPerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("FPS: %.2f"), CurrentMetrics.FPS);
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Game Thread: %.2f ms"), CurrentMetrics.GameThreadTime);
    UE_LOG(LogTemp, Warning, TEXT("Render Thread: %.2f ms"), CurrentMetrics.RenderThreadTime);
    UE_LOG(LogTemp, Warning, TEXT("GPU Time: %.2f ms"), CurrentMetrics.GPUTime);
    UE_LOG(LogTemp, Warning, TEXT("Draw Calls: %d"), CurrentMetrics.DrawCalls);
    UE_LOG(LogTemp, Warning, TEXT("Active Actors: %d"), CurrentMetrics.ActiveActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void UPerf_PerformanceMonitoringComponent::UpdatePerformanceMetrics()
{
    // Get frame time and FPS
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
        CurrentMetrics.FPS = CurrentMetrics.FrameTime > 0.0f ? 1000.0f / CurrentMetrics.FrameTime : 0.0f;
    }
    
    // Update frame time history
    FrameTimeHistory.Add(CurrentMetrics.FrameTime);
    if (FrameTimeHistory.Num() > 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Get actor count
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActiveActorCount = World->GetActorCount();
    }
    
    // Update stats
    SET_DWORD_STAT(STAT_ActiveActors, CurrentMetrics.ActiveActorCount);
    SET_DWORD_STAT(STAT_DrawCalls, CurrentMetrics.DrawCalls);
    
    // Get memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    SET_MEMORY_STAT(STAT_PhysicsMemory, MemStats.UsedPhysical / 4); // Estimate physics memory as 1/4 of total
}

void UPerf_PerformanceMonitoringComponent::ApplyOptimizations()
{
    if (OptimizationMode == EPerf_OptimizationMode::Manual)
    {
        return;
    }
    
    // Apply LOD optimizations
    AdjustLODSettings();
    
    // Apply shadow optimizations
    AdjustShadowSettings();
    
    // Apply effect optimizations
    AdjustEffectSettings();
}

void UPerf_PerformanceMonitoringComponent::AdjustLODSettings()
{
    if (IConsoleVariable* LODBiasVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.StaticMeshLODBias")))
    {
        LODBiasVar->Set(OptimizationSettings.LODBias);
    }
    
    if (IConsoleVariable* LODDistanceVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.LODDistanceScale")))
    {
        LODDistanceVar->Set(OptimizationSettings.LODDistanceScale);
    }
}

void UPerf_PerformanceMonitoringComponent::AdjustShadowSettings()
{
    if (IConsoleVariable* ShadowDistanceVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.DistanceScale")))
    {
        ShadowDistanceVar->Set(OptimizationSettings.ShadowDistanceScale);
    }
    
    if (IConsoleVariable* DynamicShadowsVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicShadows")))
    {
        DynamicShadowsVar->Set(OptimizationSettings.bEnableDynamicShadows ? 1 : 0);
    }
}

void UPerf_PerformanceMonitoringComponent::AdjustEffectSettings()
{
    if (IConsoleVariable* ScreenPercentageVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage")))
    {
        ScreenPercentageVar->Set(OptimizationSettings.ScreenPercentage);
    }
    
    if (IConsoleVariable* PostProcessVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.PostProcessing")))
    {
        PostProcessVar->Set(OptimizationSettings.bEnablePostProcessing ? 1 : 0);
    }
}

// APerf_PerformanceMonitoringSystem Implementation

APerf_PerformanceMonitoringSystem::APerf_PerformanceMonitoringSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update once per second
    
    MonitoringComponent = CreateDefaultSubobject<UPerf_PerformanceMonitoringComponent>(TEXT("MonitoringComponent"));
    
    TargetFPS = 60.0f;
    bAutoOptimization = true;
    OptimizationThreshold = 0.8f; // Optimize when below 80% of target FPS
    
    Instance = this;
}

void APerf_PerformanceMonitoringSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitoring System initialized"));
    UE_LOG(LogTemp, Warning, TEXT("Target FPS: %.1f"), TargetFPS);
    UE_LOG(LogTemp, Warning, TEXT("Auto-optimization: %s"), bAutoOptimization ? TEXT("Enabled") : TEXT("Disabled"));
}

void APerf_PerformanceMonitoringSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateGlobalMetrics();
    
    if (bAutoOptimization && ShouldOptimize())
    {
        PerformGlobalOptimization();
    }
}

APerf_PerformanceMonitoringSystem* APerf_PerformanceMonitoringSystem::GetPerformanceMonitoringSystem(UWorld* World)
{
    if (Instance && IsValid(Instance))
    {
        return Instance;
    }
    
    if (World)
    {
        for (TActorIterator<APerf_PerformanceMonitoringSystem> ActorItr(World); ActorItr; ++ActorItr)
        {
            Instance = *ActorItr;
            return Instance;
        }
    }
    
    return nullptr;
}

void APerf_PerformanceMonitoringSystem::RegisterPerformanceComponent(UPerf_PerformanceMonitoringComponent* Component)
{
    if (Component && !RegisteredComponents.Contains(Component))
    {
        RegisteredComponents.Add(Component);
        UE_LOG(LogTemp, Log, TEXT("Registered performance component: %s"), *Component->GetName());
    }
}

void APerf_PerformanceMonitoringSystem::UnregisterPerformanceComponent(UPerf_PerformanceMonitoringComponent* Component)
{
    if (Component)
    {
        RegisteredComponents.Remove(Component);
        UE_LOG(LogTemp, Log, TEXT("Unregistered performance component: %s"), *Component->GetName());
    }
}

FPerf_PerformanceMetrics APerf_PerformanceMonitoringSystem::GetGlobalPerformanceMetrics() const
{
    if (MonitoringComponent)
    {
        return MonitoringComponent->GetCurrentMetrics();
    }
    
    return FPerf_PerformanceMetrics();
}

void APerf_PerformanceMonitoringSystem::SetGlobalOptimizationMode(EPerf_OptimizationMode Mode)
{
    if (MonitoringComponent)
    {
        MonitoringComponent->SetOptimizationMode(Mode);
    }
    
    for (UPerf_PerformanceMonitoringComponent* Component : RegisteredComponents)
    {
        if (Component)
        {
            Component->SetOptimizationMode(Mode);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Global optimization mode set to: %d"), (int32)Mode);
}

void APerf_PerformanceMonitoringSystem::OptimizeForTargetFPS(float NewTargetFPS)
{
    TargetFPS = NewTargetFPS;
    
    FPerf_PerformanceMetrics CurrentMetrics = GetGlobalPerformanceMetrics();
    
    if (CurrentMetrics.FPS < TargetFPS * OptimizationThreshold)
    {
        // Performance is below threshold, apply optimizations
        EPerf_PerformanceLevel OptimalLevel = EPerf_PerformanceLevel::High;
        
        if (CurrentMetrics.FPS < TargetFPS * 0.5f)
        {
            OptimalLevel = EPerf_PerformanceLevel::Low;
        }
        else if (CurrentMetrics.FPS < TargetFPS * 0.7f)
        {
            OptimalLevel = EPerf_PerformanceLevel::Medium;
        }
        
        if (MonitoringComponent)
        {
            MonitoringComponent->ApplyPerformanceLevel(OptimalLevel);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Optimized for target FPS %.1f - Applied level: %d"), TargetFPS, (int32)OptimalLevel);
    }
}

void APerf_PerformanceMonitoringSystem::RunPerformanceBenchmark()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE BENCHMARK ==="));
    
    FPerf_PerformanceMetrics Metrics = GetGlobalPerformanceMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.2f"), Metrics.FPS);
    UE_LOG(LogTemp, Warning, TEXT("Target FPS: %.2f"), TargetFPS);
    UE_LOG(LogTemp, Warning, TEXT("Performance Ratio: %.2f%%"), (Metrics.FPS / TargetFPS) * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Active Actors: %d"), Metrics.ActiveActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB"), Metrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Registered Components: %d"), RegisteredComponents.Num());
    
    // Test different performance levels
    if (MonitoringComponent)
    {
        MonitoringComponent->LogPerformanceReport();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("============================="));
}

void APerf_PerformanceMonitoringSystem::PerformGlobalOptimization()
{
    FPerf_PerformanceMetrics CurrentMetrics = GetGlobalPerformanceMetrics();
    
    // Determine optimal performance level based on current FPS
    EPerf_PerformanceLevel OptimalLevel = EPerf_PerformanceLevel::High;
    
    if (CurrentMetrics.FPS < TargetFPS * 0.5f)
    {
        OptimalLevel = EPerf_PerformanceLevel::Low;
    }
    else if (CurrentMetrics.FPS < TargetFPS * 0.7f)
    {
        OptimalLevel = EPerf_PerformanceLevel::Medium;
    }
    else if (CurrentMetrics.FPS > TargetFPS * 1.2f)
    {
        OptimalLevel = EPerf_PerformanceLevel::Ultra;
    }
    
    // Apply to all registered components
    for (UPerf_PerformanceMonitoringComponent* Component : RegisteredComponents)
    {
        if (Component)
        {
            Component->ApplyPerformanceLevel(OptimalLevel);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Global optimization applied - Level: %d"), (int32)OptimalLevel);
}

void APerf_PerformanceMonitoringSystem::UpdateGlobalMetrics()
{
    // Update global performance statistics
    if (MonitoringComponent)
    {
        FPerf_PerformanceMetrics Metrics = MonitoringComponent->GetCurrentMetrics();
        
        // Update UE5 stats system
        SET_DWORD_STAT(STAT_ActiveActors, Metrics.ActiveActorCount);
        SET_DWORD_STAT(STAT_DrawCalls, Metrics.DrawCalls);
    }
}

bool APerf_PerformanceMonitoringSystem::ShouldOptimize() const
{
    FPerf_PerformanceMetrics CurrentMetrics = GetGlobalPerformanceMetrics();
    return CurrentMetrics.FPS < TargetFPS * OptimizationThreshold;
}