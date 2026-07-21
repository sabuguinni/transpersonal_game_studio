#include "PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Light.h"

APerformanceMonitor::APerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 times per second

    // Default settings
    MonitoringInterval = 0.1f;
    bEnableDetailedLogging = true;
    bEnableAutoOptimization = false;
    TargetFPS = 60.0f;

    // Performance thresholds
    CriticalFPSThreshold = 15.0f;
    PoorFPSThreshold = 30.0f;
    AcceptableFPSThreshold = 45.0f;
    GoodFPSThreshold = 60.0f;

    // Auto optimization settings
    bEnableLODOptimization = true;
    bEnableCullingOptimization = true;
    bEnableShadowOptimization = true;
    OptimizationCooldown = 5.0f;

    // Internal tracking
    LastMonitorTime = 0.0f;
    LastOptimizationTime = 0.0f;
    SampleCount = 0;
    bIsMonitoring = false;
    
    PerformanceLevel = EPerf_PerformanceLevel::Good;
}

void APerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    StartMonitoring();
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Started monitoring with target FPS: %.1f"), TargetFPS);
}

void APerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsMonitoring)
        return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastMonitorTime >= MonitoringInterval)
    {
        UpdatePerformanceMetrics();
        LastMonitorTime = CurrentTime;

        // Check for auto-optimization
        if (bEnableAutoOptimization && CurrentTime - LastOptimizationTime >= OptimizationCooldown)
        {
            CheckForOptimizations();
        }
    }
}

void APerformanceMonitor::StartMonitoring()
{
    bIsMonitoring = true;
    ResetMetrics();
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Monitoring started"));
}

void APerformanceMonitor::StopMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Monitoring stopped"));
}

void APerformanceMonitor::ResetMetrics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    FPSSamples.Empty();
    SampleCount = 0;
    PerformanceLevel = EPerf_PerformanceLevel::Good;
}

void APerformanceMonitor::UpdatePerformanceMetrics()
{
    if (!GetWorld())
        return;

    // Calculate current FPS
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    }

    // Add to samples for averaging
    FPSSamples.Add(CurrentMetrics.CurrentFPS);
    if (FPSSamples.Num() > 100) // Keep last 100 samples
    {
        FPSSamples.RemoveAt(0);
    }

    CalculateAverages();

    // Update performance level
    EPerf_PerformanceLevel NewLevel = DeterminePerformanceLevel(CurrentMetrics.CurrentFPS);
    if (NewLevel != PerformanceLevel)
    {
        PerformanceLevel = NewLevel;
        OnPerformanceLevelChanged(PerformanceLevel);
        
        if (PerformanceLevel == EPerf_PerformanceLevel::Critical)
        {
            OnCriticalPerformance(CurrentMetrics.CurrentFPS);
        }
    }

    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);

    // Count actors
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActorCount = World->GetActorCount();
        
        // Count visible actors (simplified - just non-hidden actors)
        CurrentMetrics.VisibleActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && !Actor->IsHidden())
            {
                CurrentMetrics.VisibleActorCount++;
            }
        }
    }

    // Estimate draw calls (simplified calculation)
    CurrentMetrics.DrawCalls = CurrentMetrics.VisibleActorCount * 2; // Rough estimate

    SampleCount++;

    // Log detailed info if enabled
    if (bEnableDetailedLogging && SampleCount % 50 == 0) // Every 5 seconds at 0.1s intervals
    {
        LogPerformanceReport();
    }
}

void APerformanceMonitor::CalculateAverages()
{
    if (FPSSamples.Num() == 0)
        return;

    float Sum = 0.0f;
    CurrentMetrics.MinFPS = FPSSamples[0];
    CurrentMetrics.MaxFPS = FPSSamples[0];

    for (float FPS : FPSSamples)
    {
        Sum += FPS;
        CurrentMetrics.MinFPS = FMath::Min(CurrentMetrics.MinFPS, FPS);
        CurrentMetrics.MaxFPS = FMath::Max(CurrentMetrics.MaxFPS, FPS);
    }

    CurrentMetrics.AverageFPS = Sum / FPSSamples.Num();
}

EPerf_PerformanceLevel APerformanceMonitor::DeterminePerformanceLevel(float FPS) const
{
    if (FPS < CriticalFPSThreshold)
        return EPerf_PerformanceLevel::Critical;
    else if (FPS < PoorFPSThreshold)
        return EPerf_PerformanceLevel::Poor;
    else if (FPS < AcceptableFPSThreshold)
        return EPerf_PerformanceLevel::Acceptable;
    else if (FPS < GoodFPSThreshold)
        return EPerf_PerformanceLevel::Good;
    else
        return EPerf_PerformanceLevel::Excellent;
}

void APerformanceMonitor::CheckForOptimizations()
{
    if (CurrentMetrics.CurrentFPS < TargetFPS * 0.8f) // 80% of target
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceMonitor: Low FPS detected (%.1f), applying optimizations"), CurrentMetrics.CurrentFPS);
        ApplyOptimizations();
        LastOptimizationTime = GetWorld()->GetTimeSeconds();
    }
}

void APerformanceMonitor::ApplyOptimizations()
{
    if (bEnableLODOptimization)
        OptimizeLOD();
    
    if (bEnableCullingOptimization)
        OptimizeCulling();
    
    if (bEnableShadowOptimization)
        OptimizeShadows();

    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitor: Optimizations applied"));
}

void APerformanceMonitor::OptimizeLOD()
{
    if (!GetWorld())
        return;

    // Reduce LOD bias for static meshes when performance is poor
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
            continue;

        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp)
        {
            // Increase LOD bias to use lower detail models sooner
            MeshComp->SetForcedLodModel(2); // Force LOD 1 (lower detail)
        }
    }
}

void APerformanceMonitor::OptimizeCulling()
{
    if (!GetWorld())
        return;

    // Reduce cull distances for non-essential objects
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
            continue;

        // Target foliage and decoration objects
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Rock")) || ActorName.Contains(TEXT("Grass")))
        {
            UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp)
            {
                // Reduce cull distance for performance
                MeshComp->SetCullDistance(3000.0f);
            }
        }
    }
}

void APerformanceMonitor::OptimizeShadows()
{
    if (!GetWorld())
        return;

    // Reduce shadow quality for non-key lights
    int32 LightCount = 0;
    for (TActorIterator<ALight> LightItr(GetWorld()); LightItr; ++LightItr)
    {
        ALight* Light = *LightItr;
        if (!Light)
            continue;

        LightCount++;
        
        // Keep first 2 lights high quality, reduce others
        if (LightCount > 2)
        {
            ULightComponent* LightComp = Light->GetLightComponent();
            if (LightComp)
            {
                // Reduce shadow resolution
                LightComp->SetShadowResolutionScale(0.5f);
                // Increase shadow bias to reduce artifacts
                LightComp->SetShadowBias(0.1f);
            }
        }
    }
}

FPerf_PerformanceMetrics APerformanceMonitor::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

EPerf_PerformanceLevel APerformanceMonitor::GetPerformanceLevel() const
{
    return PerformanceLevel;
}

void APerformanceMonitor::LogPerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Current FPS: %.1f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Log, TEXT("Average FPS: %.1f"), CurrentMetrics.AverageFPS);
    UE_LOG(LogTemp, Log, TEXT("Min/Max FPS: %.1f / %.1f"), CurrentMetrics.MinFPS, CurrentMetrics.MaxFPS);
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Actor Count: %d (%d visible)"), CurrentMetrics.ActorCount, CurrentMetrics.VisibleActorCount);
    UE_LOG(LogTemp, Log, TEXT("Performance Level: %s"), *UEnum::GetValueAsString(PerformanceLevel));
    
    WritePerformanceLog();
}

void APerformanceMonitor::WritePerformanceLog()
{
    // Write performance data to file for external analysis
    FString LogPath = FPaths::ProjectLogDir() / TEXT("PerformanceLog.csv");
    FString LogEntry = FString::Printf(TEXT("%.2f,%.1f,%.1f,%.1f,%.1f,%.2f,%.1f,%d,%d\n"),
        GetWorld()->GetTimeSeconds(),
        CurrentMetrics.CurrentFPS,
        CurrentMetrics.AverageFPS,
        CurrentMetrics.MinFPS,
        CurrentMetrics.MaxFPS,
        CurrentMetrics.FrameTime,
        CurrentMetrics.MemoryUsageMB,
        CurrentMetrics.ActorCount,
        CurrentMetrics.VisibleActorCount
    );
    
    FFileHelper::SaveStringToFile(LogEntry, *LogPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}