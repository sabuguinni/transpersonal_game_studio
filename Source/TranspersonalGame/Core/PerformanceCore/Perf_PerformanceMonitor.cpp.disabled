#include "Perf_PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

APerf_PerformanceMonitor::APerf_PerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms
    
    MonitoringInterval = 1.0f;
    bEnableAutoLOD = true;
    TargetFPS = 60.0f;
    CriticalFPSThreshold = 20.0f;
    TimeSinceLastUpdate = 0.0f;
    MaxFPSHistorySize = 60; // Keep 60 samples for averaging
    
    CurrentMetrics = FPerf_PerformanceMetrics();
}

void APerf_PerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor started - Target FPS: %.1f"), TargetFPS);
    
    // Initialize FPS history
    FPSHistory.Reserve(MaxFPSHistorySize);
    
    // Enable basic performance stats
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), TEXT("stat fps"));
        GEngine->Exec(GetWorld(), TEXT("stat unit"));
    }
}

void APerf_PerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= MonitoringInterval)
    {
        UpdatePerformanceMetrics();
        
        if (bEnableAutoLOD)
        {
            UpdateLODBasedOnPerformance();
        }
        
        LogPerformanceWarnings();
        TimeSinceLastUpdate = 0.0f;
    }
}

void APerf_PerformanceMonitor::UpdatePerformanceMetrics()
{
    // Get current FPS
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentMetrics.CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        
        // Clamp to reasonable values
        CurrentMetrics.CurrentFPS = FMath::Clamp(CurrentMetrics.CurrentFPS, 1.0f, 300.0f);
        
        // Add to history for averaging
        FPSHistory.Add(CurrentMetrics.CurrentFPS);
        if (FPSHistory.Num() > MaxFPSHistorySize)
        {
            FPSHistory.RemoveAt(0);
        }
        
        CalculateAverageFPS();
    }
    
    // Get memory usage (basic estimation)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Count visible actors (simplified)
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentMetrics.VisibleActors = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && !Actor->IsHidden())
            {
                CurrentMetrics.VisibleActors++;
            }
        }
    }
    
    // Determine performance level
    if (CurrentMetrics.AverageFPS >= TargetFPS * 0.9f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::High;
    }
    else if (CurrentMetrics.AverageFPS >= TargetFPS * 0.7f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Medium;
    }
    else if (CurrentMetrics.AverageFPS >= CriticalFPSThreshold)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Low;
    }
    else
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Critical;
    }
}

void APerf_PerformanceMonitor::CalculateAverageFPS()
{
    if (FPSHistory.Num() > 0)
    {
        float Sum = 0.0f;
        for (float FPS : FPSHistory)
        {
            Sum += FPS;
        }
        CurrentMetrics.AverageFPS = Sum / FPSHistory.Num();
    }
}

void APerf_PerformanceMonitor::UpdateLODBasedOnPerformance()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 TargetLOD = 0;
    
    // Determine target LOD based on performance
    switch (CurrentMetrics.PerformanceLevel)
    {
        case EPerf_PerformanceLevel::High:
            TargetLOD = 0; // Highest quality
            break;
        case EPerf_PerformanceLevel::Medium:
            TargetLOD = 1; // Medium quality
            break;
        case EPerf_PerformanceLevel::Low:
            TargetLOD = 2; // Lower quality
            break;
        case EPerf_PerformanceLevel::Critical:
            TargetLOD = 3; // Lowest quality
            break;
    }
    
    SetLODLevel(TargetLOD);
}

void APerf_PerformanceMonitor::SetLODLevel(int32 LODLevel)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Apply LOD to static mesh actors
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AStaticMeshActor* MeshActor = *ActorItr;
        if (MeshActor && MeshActor->GetStaticMeshComponent())
        {
            UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
            MeshComp->SetForcedLodModel(LODLevel + 1); // LOD model is 1-based
        }
    }
}

void APerf_PerformanceMonitor::OptimizeScene()
{
    UE_LOG(LogTemp, Warning, TEXT("Optimizing scene for performance..."));
    
    // Force garbage collection
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
    }
    
    // Apply aggressive LOD
    if (CurrentMetrics.PerformanceLevel == EPerf_PerformanceLevel::Critical)
    {
        SetLODLevel(3);
        UE_LOG(LogTemp, Warning, TEXT("Applied critical performance optimizations"));
    }
}

EPerf_PerformanceLevel APerf_PerformanceMonitor::GetCurrentPerformanceLevel() const
{
    return CurrentMetrics.PerformanceLevel;
}

void APerf_PerformanceMonitor::EnablePerformanceStats(bool bEnable)
{
    if (GEngine)
    {
        if (bEnable)
        {
            GEngine->Exec(GetWorld(), TEXT("stat fps"));
            GEngine->Exec(GetWorld(), TEXT("stat unit"));
            GEngine->Exec(GetWorld(), TEXT("stat memory"));
        }
        else
        {
            GEngine->Exec(GetWorld(), TEXT("stat none"));
        }
    }
}

void APerf_PerformanceMonitor::LogPerformanceWarnings()
{
    if (CurrentMetrics.PerformanceLevel == EPerf_PerformanceLevel::Critical)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL PERFORMANCE: FPS %.1f (Target: %.1f) - Memory: %.1fMB - Actors: %d"), 
               CurrentMetrics.CurrentFPS, TargetFPS, CurrentMetrics.MemoryUsageMB, CurrentMetrics.VisibleActors);
    }
    else if (CurrentMetrics.PerformanceLevel == EPerf_PerformanceLevel::Low)
    {
        UE_LOG(LogTemp, Warning, TEXT("LOW PERFORMANCE: FPS %.1f (Target: %.1f) - Memory: %.1fMB - Actors: %d"), 
               CurrentMetrics.CurrentFPS, TargetFPS, CurrentMetrics.MemoryUsageMB, CurrentMetrics.VisibleActors);
    }
}