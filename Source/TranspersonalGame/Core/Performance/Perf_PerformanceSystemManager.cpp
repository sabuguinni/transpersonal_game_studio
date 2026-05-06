#include "Perf_PerformanceSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "RHI.h"

APerf_PerformanceSystemManager::APerf_PerformanceSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    OptimizationLevel = EPerf_OptimizationLevel::High;
    TargetFrameRate = 60.0f;
    bAutoOptimization = true;
    OptimizationCheckInterval = 2.0f;
    bEnableLODSystem = true;
    MemoryThresholdMB = 4096.0f;
    bAutoGarbageCollection = true;
    bEnableGPUCulling = true;
    bOptimizeLumen = true;
    bOptimizeNanite = true;
    
    // Initialize timers
    OptimizationTimer = 0.0f;
    MetricsUpdateTimer = 0.0f;
    MaxFrameTimesSamples = 60; // Track last 60 frames
    
    // Initialize LOD settings
    LODSettings.LODDistanceMultiplier = 1.0f;
    LODSettings.MaxLODLevel = 3;
    LODSettings.bEnableDistanceCulling = true;
    LODSettings.CullingDistance = 10000.0f;
}

void APerf_PerformanceSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance System Manager initialized"));
    
    // Apply initial optimization settings
    ApplyOptimizationLevel();
    
    // Enable performance stats
    EnablePerformanceStats(true);
    
    // Initial metrics update
    UpdatePerformanceMetrics();
}

void APerf_PerformanceSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update frame time history
    UpdateFrameTimeHistory(DeltaTime);
    
    // Update timers
    OptimizationTimer += DeltaTime;
    MetricsUpdateTimer += DeltaTime;
    
    // Update metrics every 0.5 seconds
    if (MetricsUpdateTimer >= 0.5f)
    {
        UpdatePerformanceMetrics();
        MetricsUpdateTimer = 0.0f;
    }
    
    // Check for auto-optimization
    if (bAutoOptimization && OptimizationTimer >= OptimizationCheckInterval)
    {
        PerformAutoOptimization();
        OptimizationTimer = 0.0f;
    }
    
    // Check memory usage
    if (bAutoGarbageCollection)
    {
        CheckMemoryUsage();
    }
}

void APerf_PerformanceSystemManager::UpdatePerformanceMetrics()
{
    // Calculate current FPS
    float AverageFrameTime = CalculateAverageFrameTime();
    CurrentMetrics.CurrentFPS = (AverageFrameTime > 0.0f) ? (1.0f / AverageFrameTime) : 0.0f;
    CurrentMetrics.TargetFPS = TargetFrameRate;
    
    // Get GPU time (approximation)
    CurrentMetrics.GPUTime = AverageFrameTime * 0.6f; // Assume 60% GPU bound
    CurrentMetrics.CPUTime = AverageFrameTime * 0.4f; // Assume 40% CPU bound
    
    // Get memory usage
    CurrentMetrics.MemoryUsageMB = GetMemoryUsageMB();
    
    // Count visible actors
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        CurrentMetrics.VisibleActors = AllActors.Num();
        
        // Estimate draw calls (rough approximation)
        int32 StaticMeshCount = 0;
        int32 SkeletalMeshCount = 0;
        
        for (AActor* Actor : AllActors)
        {
            if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor))
            {
                StaticMeshCount++;
            }
            else if (ASkeletalMeshActor* SkeletalMeshActor = Cast<ASkeletalMeshActor>(Actor))
            {
                SkeletalMeshCount++;
            }
            else if (ACharacter* Character = Cast<ACharacter>(Actor))
            {
                SkeletalMeshCount++;
            }
        }
        
        CurrentMetrics.DrawCalls = StaticMeshCount + (SkeletalMeshCount * 2); // Skeletal meshes typically use more draw calls
    }
}

FPerf_PerformanceMetrics APerf_PerformanceSystemManager::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerf_PerformanceSystemManager::SetOptimizationLevel(EPerf_OptimizationLevel NewLevel)
{
    OptimizationLevel = NewLevel;
    ApplyOptimizationLevel();
    UE_LOG(LogTemp, Warning, TEXT("Optimization level changed to: %d"), (int32)OptimizationLevel);
}

void APerf_PerformanceSystemManager::SetTargetFrameRate(float NewTargetFPS)
{
    TargetFrameRate = FMath::Clamp(NewTargetFPS, 30.0f, 120.0f);
    CurrentMetrics.TargetFPS = TargetFrameRate;
    UE_LOG(LogTemp, Warning, TEXT("Target frame rate set to: %.1f"), TargetFrameRate);
}

void APerf_PerformanceSystemManager::UpdateLODSettings()
{
    if (!bEnableLODSystem)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Apply LOD settings to all static mesh actors
    TArray<AActor*> StaticMeshActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshActors);
    
    for (AActor* Actor : StaticMeshActors)
    {
        if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor))
        {
            UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                // Apply LOD distance multiplier
                MeshComp->SetLODDistanceScale(LODSettings.LODDistanceMultiplier);
                
                // Apply culling distance
                if (LODSettings.bEnableDistanceCulling)
                {
                    MeshComp->SetCullDistance(LODSettings.CullingDistance);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("LOD settings updated for %d static mesh actors"), StaticMeshActors.Num());
}

void APerf_PerformanceSystemManager::ApplyLODToActors()
{
    UpdateLODSettings();
}

void APerf_PerformanceSystemManager::SetLODDistanceMultiplier(float Multiplier)
{
    LODSettings.LODDistanceMultiplier = FMath::Clamp(Multiplier, 0.1f, 5.0f);
    UpdateLODSettings();
}

void APerf_PerformanceSystemManager::CheckMemoryUsage()
{
    float CurrentMemoryMB = GetMemoryUsageMB();
    
    if (CurrentMemoryMB > MemoryThresholdMB)
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory usage high: %.1f MB (threshold: %.1f MB)"), CurrentMemoryMB, MemoryThresholdMB);
        
        if (bAutoGarbageCollection)
        {
            ForceGarbageCollection();
        }
    }
}

void APerf_PerformanceSystemManager::ForceGarbageCollection()
{
    GEngine->ForceGarbageCollection(true);
    UE_LOG(LogTemp, Warning, TEXT("Forced garbage collection"));
}

float APerf_PerformanceSystemManager::GetMemoryUsageMB() const
{
    // Get memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

void APerf_PerformanceSystemManager::ApplyGPUOptimizations()
{
    if (bOptimizeLumen)
    {
        OptimizeLumenSettings();
    }
    
    if (bOptimizeNanite)
    {
        OptimizeNaniteSettings();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GPU optimizations applied"));
}

void APerf_PerformanceSystemManager::OptimizeLumenSettings()
{
    // Apply Lumen optimization based on performance level
    switch (OptimizationLevel)
    {
        case EPerf_OptimizationLevel::Ultra:
            // High quality Lumen settings
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.GlobalIllumination.Quality"))->Set(4);
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.Reflections.Quality"))->Set(4);
            break;
        case EPerf_OptimizationLevel::High:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.GlobalIllumination.Quality"))->Set(3);
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.Reflections.Quality"))->Set(3);
            break;
        case EPerf_OptimizationLevel::Medium:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.GlobalIllumination.Quality"))->Set(2);
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.Reflections.Quality"))->Set(2);
            break;
        case EPerf_OptimizationLevel::Low:
        case EPerf_OptimizationLevel::Performance:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.GlobalIllumination.Quality"))->Set(1);
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.Reflections.Quality"))->Set(1);
            break;
    }
}

void APerf_PerformanceSystemManager::OptimizeNaniteSettings()
{
    // Apply Nanite optimization based on performance level
    switch (OptimizationLevel)
    {
        case EPerf_OptimizationLevel::Ultra:
        case EPerf_OptimizationLevel::High:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite.MaxPixelsPerEdge"))->Set(1.0f);
            break;
        case EPerf_OptimizationLevel::Medium:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite.MaxPixelsPerEdge"))->Set(2.0f);
            break;
        case EPerf_OptimizationLevel::Low:
        case EPerf_OptimizationLevel::Performance:
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite.MaxPixelsPerEdge"))->Set(4.0f);
            break;
    }
}

void APerf_PerformanceSystemManager::EnableAutoOptimization(bool bEnable)
{
    bAutoOptimization = bEnable;
    UE_LOG(LogTemp, Warning, TEXT("Auto-optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void APerf_PerformanceSystemManager::PerformAutoOptimization()
{
    if (!IsPerformanceBelowTarget())
    {
        return; // Performance is acceptable
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance below target (%.1f fps), applying optimizations"), CurrentMetrics.CurrentFPS);
    
    // Step down optimization level if needed
    switch (OptimizationLevel)
    {
        case EPerf_OptimizationLevel::Ultra:
            SetOptimizationLevel(EPerf_OptimizationLevel::High);
            break;
        case EPerf_OptimizationLevel::High:
            SetOptimizationLevel(EPerf_OptimizationLevel::Medium);
            break;
        case EPerf_OptimizationLevel::Medium:
            SetOptimizationLevel(EPerf_OptimizationLevel::Low);
            break;
        case EPerf_OptimizationLevel::Low:
            SetOptimizationLevel(EPerf_OptimizationLevel::Performance);
            break;
        case EPerf_OptimizationLevel::Performance:
            // Already at lowest level, apply additional optimizations
            AdjustQualitySettings();
            break;
    }
}

void APerf_PerformanceSystemManager::EnablePerformanceStats(bool bEnable)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    if (bEnable)
    {
        GEngine->Exec(World, TEXT("stat fps"));
        GEngine->Exec(World, TEXT("stat unit"));
    }
    else
    {
        GEngine->Exec(World, TEXT("stat none"));
    }
}

void APerf_PerformanceSystemManager::LogPerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f (Target: %.1f)"), CurrentMetrics.CurrentFPS, CurrentMetrics.TargetFPS);
    UE_LOG(LogTemp, Warning, TEXT("GPU Time: %.2f ms"), CurrentMetrics.GPUTime * 1000.0f);
    UE_LOG(LogTemp, Warning, TEXT("CPU Time: %.2f ms"), CurrentMetrics.CPUTime * 1000.0f);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Draw Calls: %d"), CurrentMetrics.DrawCalls);
    UE_LOG(LogTemp, Warning, TEXT("Visible Actors: %d"), CurrentMetrics.VisibleActors);
    UE_LOG(LogTemp, Warning, TEXT("Optimization Level: %d"), (int32)OptimizationLevel);
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void APerf_PerformanceSystemManager::TestPerformanceOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("Testing performance optimization..."));
    UpdatePerformanceMetrics();
    LogPerformanceReport();
    PerformAutoOptimization();
}

void APerf_PerformanceSystemManager::ApplyOptimizationLevel()
{
    OptimizeRenderingSettings();
    OptimizePhysicsSettings();
    OptimizeAudioSettings();
    ApplyGPUOptimizations();
    UpdateLODSettings();
}

void APerf_PerformanceSystemManager::UpdateFrameTimeHistory(float DeltaTime)
{
    RecentFrameTimes.Add(DeltaTime);
    
    // Keep only the most recent samples
    if (RecentFrameTimes.Num() > MaxFrameTimesSamples)
    {
        RecentFrameTimes.RemoveAt(0);
    }
}

float APerf_PerformanceSystemManager::CalculateAverageFrameTime() const
{
    if (RecentFrameTimes.Num() == 0)
    {
        return 0.0f;
    }
    
    float Total = 0.0f;
    for (float FrameTime : RecentFrameTimes)
    {
        Total += FrameTime;
    }
    
    return Total / RecentFrameTimes.Num();
}

bool APerf_PerformanceSystemManager::IsPerformanceBelowTarget() const
{
    return CurrentMetrics.CurrentFPS < (TargetFrameRate * 0.9f); // 10% tolerance
}

void APerf_PerformanceSystemManager::AdjustQualitySettings()
{
    // Apply aggressive quality reductions
    IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ViewDistanceQuality"))->Set(1);
    IConsoleManager::Get().FindConsoleVariable(TEXT("sg.AntiAliasingQuality"))->Set(1);
    IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality"))->Set(1);
    IConsoleManager::Get().FindConsoleVariable(TEXT("sg.PostProcessQuality"))->Set(1);
    IConsoleManager::Get().FindConsoleVariable(TEXT("sg.TextureQuality"))->Set(1);
    IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality"))->Set(1);
    
    UE_LOG(LogTemp, Warning, TEXT("Applied aggressive quality settings for performance"));
}

void APerf_PerformanceSystemManager::OptimizeRenderingSettings()
{
    // Apply rendering optimizations based on level
    switch (OptimizationLevel)
    {
        case EPerf_OptimizationLevel::Ultra:
            IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ViewDistanceQuality"))->Set(4);
            IConsoleManager::Get().FindConsoleVariable(TEXT("sg.AntiAliasingQuality"))->Set(4);
            IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality"))->Set(4);
            break;
        case EPerf_OptimizationLevel::High:
            IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ViewDistanceQuality"))->Set(3);
            IConsoleManager::Get().FindConsoleVariable(TEXT("sg.AntiAliasingQuality"))->Set(3);
            IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality"))->Set(3);
            break;
        case EPerf_OptimizationLevel::Medium:
            IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ViewDistanceQuality"))->Set(2);
            IConsoleManager::Get().FindConsoleVariable(TEXT("sg.AntiAliasingQuality"))->Set(2);
            IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality"))->Set(2);
            break;
        case EPerf_OptimizationLevel::Low:
        case EPerf_OptimizationLevel::Performance:
            IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ViewDistanceQuality"))->Set(1);
            IConsoleManager::Get().FindConsoleVariable(TEXT("sg.AntiAliasingQuality"))->Set(1);
            IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality"))->Set(1);
            break;
    }
}

void APerf_PerformanceSystemManager::OptimizePhysicsSettings()
{
    // Apply physics optimizations
    switch (OptimizationLevel)
    {
        case EPerf_OptimizationLevel::Ultra:
        case EPerf_OptimizationLevel::High:
            IConsoleManager::Get().FindConsoleVariable(TEXT("p.DefaultMaxSubstepDeltaTime"))->Set(0.016667f);
            IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxSubsteps"))->Set(8);
            break;
        case EPerf_OptimizationLevel::Medium:
            IConsoleManager::Get().FindConsoleVariable(TEXT("p.DefaultMaxSubstepDeltaTime"))->Set(0.02f);
            IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxSubsteps"))->Set(6);
            break;
        case EPerf_OptimizationLevel::Low:
        case EPerf_OptimizationLevel::Performance:
            IConsoleManager::Get().FindConsoleVariable(TEXT("p.DefaultMaxSubstepDeltaTime"))->Set(0.033333f);
            IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxSubsteps"))->Set(4);
            break;
    }
}

void APerf_PerformanceSystemManager::OptimizeAudioSettings()
{
    // Apply audio optimizations
    switch (OptimizationLevel)
    {
        case EPerf_OptimizationLevel::Ultra:
        case EPerf_OptimizationLevel::High:
            IConsoleManager::Get().FindConsoleVariable(TEXT("au.MaxChannels"))->Set(64);
            break;
        case EPerf_OptimizationLevel::Medium:
            IConsoleManager::Get().FindConsoleVariable(TEXT("au.MaxChannels"))->Set(32);
            break;
        case EPerf_OptimizationLevel::Low:
        case EPerf_OptimizationLevel::Performance:
            IConsoleManager::Get().FindConsoleVariable(TEXT("au.MaxChannels"))->Set(16);
            break;
    }
}