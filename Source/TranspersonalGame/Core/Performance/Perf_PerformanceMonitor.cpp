#include "Perf_PerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "RHI.h"
#include "Stats/Stats.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"

APerf_PerformanceMonitor::APerf_PerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default settings
    OptimizationSettings = FPerf_OptimizationSettings();
    MonitoringInterval = 1.0f;
    bLogPerformanceData = true;
    bSavePerformanceReports = true;
    
    // Reserve space for FPS samples
    FPSSamples.Reserve(MaxSamples);
}

void APerf_PerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor started - Target FPS: %.1f"), OptimizationSettings.TargetFPS);
    
    // Initialize metrics
    ResetMetrics();
    
    // Set initial console commands for performance monitoring
    if (UWorld* World = GetWorld())
    {
        // Enable stat collection
        GEngine->Exec(World, TEXT("stat fps"));
        GEngine->Exec(World, TEXT("stat unit"));
        
        // Set target frame rate
        FString TargetFPSCommand = FString::Printf(TEXT("t.MaxFPS %.1f"), OptimizationSettings.TargetFPS);
        GEngine->Exec(World, *TargetFPSCommand);
    }
}

void APerf_PerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    MonitorTimer += DeltaTime;
    
    // Collect performance data every frame
    CollectFPSData(DeltaTime);
    
    // Update full metrics at specified intervals
    if (MonitorTimer >= MonitoringInterval)
    {
        UpdatePerformanceMetrics();
        
        if (OptimizationSettings.bAutoOptimize)
        {
            ApplyAutoOptimizations();
        }
        
        if (bLogPerformanceData)
        {
            LogPerformanceData();
        }
        
        MonitorTimer = 0.0f;
    }
}

void APerf_PerformanceMonitor::UpdatePerformanceMetrics()
{
    // Collect all performance data
    CollectMemoryData();
    CollectRenderingData();
    
    // Calculate average FPS
    if (FPSSamples.Num() > 0)
    {
        float Sum = 0.0f;
        CurrentMetrics.MinFPS = FPSSamples[0];
        CurrentMetrics.MaxFPS = FPSSamples[0];
        
        for (float Sample : FPSSamples)
        {
            Sum += Sample;
            CurrentMetrics.MinFPS = FMath::Min(CurrentMetrics.MinFPS, Sample);
            CurrentMetrics.MaxFPS = FMath::Max(CurrentMetrics.MaxFPS, Sample);
        }
        
        CurrentMetrics.AverageFPS = Sum / FPSSamples.Num();
    }
}

void APerf_PerformanceMonitor::CollectFPSData(float DeltaTime)
{
    if (DeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        
        // Add to samples array
        FPSSamples.Add(CurrentMetrics.CurrentFPS);
        
        // Keep only the most recent samples
        if (FPSSamples.Num() > MaxSamples)
        {
            FPSSamples.RemoveAt(0);
        }
    }
}

void APerf_PerformanceMonitor::CollectMemoryData()
{
    // Get platform memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Get GPU memory usage if available
    if (GDynamicRHI)
    {
        uint64 GPUMemory = GDynamicRHI->RHIGetAvailableGPUMemory();
        if (GPUMemory > 0)
        {
            CurrentMetrics.GPUMemoryUsageMB = GPUMemory / (1024.0f * 1024.0f);
        }
    }
}

void APerf_PerformanceMonitor::CollectRenderingData()
{
    // Get rendering stats from engine
    if (GEngine && GEngine->GetGameViewport())
    {
        // These would typically come from render thread stats
        // For now, we'll use estimated values based on scene complexity
        if (UWorld* World = GetWorld())
        {
            TArray<AActor*> AllActors;
            UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
            
            // Estimate draw calls based on visible actors
            CurrentMetrics.DrawCalls = 0;
            CurrentMetrics.Triangles = 0;
            
            for (AActor* Actor : AllActors)
            {
                if (Actor && Actor->IsActorBeingDestroyed() == false)
                {
                    // Count static mesh components
                    TArray<UStaticMeshComponent*> MeshComponents;
                    Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
                    
                    for (UStaticMeshComponent* MeshComp : MeshComponents)
                    {
                        if (MeshComp && MeshComp->GetStaticMesh())
                        {
                            CurrentMetrics.DrawCalls++;
                            
                            // Estimate triangle count
                            if (MeshComp->GetStaticMesh()->GetRenderData() && 
                                MeshComp->GetStaticMesh()->GetRenderData()->LODResources.Num() > 0)
                            {
                                CurrentMetrics.Triangles += MeshComp->GetStaticMesh()->GetRenderData()->LODResources[0].GetNumTriangles();
                            }
                        }
                    }
                }
            }
        }
    }
}

void APerf_PerformanceMonitor::ApplyAutoOptimizations()
{
    if (!IsPerformanceAcceptable())
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance below acceptable levels - applying optimizations"));
        
        if (OptimizationSettings.bEnableLODOptimization)
        {
            OptimizeLODSettings();
        }
        
        if (OptimizationSettings.bEnableCullingOptimization)
        {
            OptimizeCullingDistances();
        }
        
        if (CurrentMetrics.MemoryUsageMB > OptimizationSettings.MaxMemoryUsageMB)
        {
            OptimizeMemoryUsage();
        }
    }
}

void APerf_PerformanceMonitor::LogPerformanceData()
{
    UE_LOG(LogTemp, Log, TEXT("Performance: FPS=%.1f (Avg: %.1f, Min: %.1f, Max: %.1f), Memory=%.1fMB, DrawCalls=%d, Triangles=%d"),
        CurrentMetrics.CurrentFPS,
        CurrentMetrics.AverageFPS,
        CurrentMetrics.MinFPS,
        CurrentMetrics.MaxFPS,
        CurrentMetrics.MemoryUsageMB,
        CurrentMetrics.DrawCalls,
        CurrentMetrics.Triangles
    );
}

void APerf_PerformanceMonitor::SetOptimizationSettings(const FPerf_OptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    
    // Apply target FPS immediately
    if (UWorld* World = GetWorld())
    {
        FString TargetFPSCommand = FString::Printf(TEXT("t.MaxFPS %.1f"), OptimizationSettings.TargetFPS);
        GEngine->Exec(World, *TargetFPSCommand);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Optimization settings updated - Target FPS: %.1f"), OptimizationSettings.TargetFPS);
}

void APerf_PerformanceMonitor::TriggerOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("Manual optimization triggered"));
    
    OptimizeLODSettings();
    OptimizeCullingDistances();
    OptimizeMemoryUsage();
}

void APerf_PerformanceMonitor::SavePerformanceReport()
{
    if (!bSavePerformanceReports)
    {
        return;
    }
    
    FString ReportContent;
    ReportContent += TEXT("PERFORMANCE REPORT\n");
    ReportContent += TEXT("==================\n\n");
    ReportContent += FString::Printf(TEXT("Timestamp: %s\n"), *FDateTime::Now().ToString());
    ReportContent += FString::Printf(TEXT("Current FPS: %.2f\n"), CurrentMetrics.CurrentFPS);
    ReportContent += FString::Printf(TEXT("Average FPS: %.2f\n"), CurrentMetrics.AverageFPS);
    ReportContent += FString::Printf(TEXT("Min FPS: %.2f\n"), CurrentMetrics.MinFPS);
    ReportContent += FString::Printf(TEXT("Max FPS: %.2f\n"), CurrentMetrics.MaxFPS);
    ReportContent += FString::Printf(TEXT("Memory Usage: %.2f MB\n"), CurrentMetrics.MemoryUsageMB);
    ReportContent += FString::Printf(TEXT("GPU Memory: %.2f MB\n"), CurrentMetrics.GPUMemoryUsageMB);
    ReportContent += FString::Printf(TEXT("Draw Calls: %d\n"), CurrentMetrics.DrawCalls);
    ReportContent += FString::Printf(TEXT("Triangles: %d\n"), CurrentMetrics.Triangles);
    ReportContent += FString::Printf(TEXT("Performance Acceptable: %s\n"), IsPerformanceAcceptable() ? TEXT("Yes") : TEXT("No"));
    
    FString FileName = FString::Printf(TEXT("PerformanceReport_%s.txt"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    FString FilePath = ReportSavePath + FileName;
    
    if (FFileHelper::SaveStringToFile(ReportContent, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("Performance report saved to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save performance report to: %s"), *FilePath);
    }
}

void APerf_PerformanceMonitor::ResetMetrics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    FPSSamples.Empty();
    MonitorTimer = 0.0f;
    LastReportTime = 0.0f;
}

bool APerf_PerformanceMonitor::IsPerformanceAcceptable() const
{
    return CurrentMetrics.AverageFPS >= OptimizationSettings.MinAcceptableFPS &&
           CurrentMetrics.DrawCalls <= OptimizationSettings.MaxDrawCalls &&
           CurrentMetrics.MemoryUsageMB <= OptimizationSettings.MaxMemoryUsageMB;
}

void APerf_PerformanceMonitor::OptimizeLODSettings()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> StaticMeshActors;
        UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshActors);
        
        for (AActor* Actor : StaticMeshActors)
        {
            if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
            {
                if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
                {
                    // Increase LOD bias to reduce detail at distance
                    MeshComp->SetForcedLodModel(1);
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("LOD optimization applied to %d static mesh actors"), StaticMeshActors.Num());
    }
}

void APerf_PerformanceMonitor::OptimizeCullingDistances()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetRootComponent())
            {
                // Set culling distance based on actor size and importance
                FVector ActorBounds = Actor->GetActorBounds(false).BoxExtent;
                float MaxExtent = ActorBounds.GetMax();
                
                float CullingDistance = MaxExtent * 100.0f * OptimizationSettings.CullingDistanceMultiplier;
                CullingDistance = FMath::Clamp(CullingDistance, 1000.0f, 20000.0f);
                
                Actor->SetCullDistance(CullingDistance);
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Culling distance optimization applied to %d actors"), AllActors.Num());
    }
}

void APerf_PerformanceMonitor::OptimizeMemoryUsage()
{
    if (UWorld* World = GetWorld())
    {
        // Force garbage collection
        GEngine->Exec(World, TEXT("gc"));
        
        // Reduce texture streaming pool if memory usage is high
        if (CurrentMetrics.MemoryUsageMB > OptimizationSettings.MaxMemoryUsageMB * 0.8f)
        {
            GEngine->Exec(World, TEXT("r.Streaming.PoolSize 1024"));
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Memory optimization applied - forced GC and reduced streaming pool"));
    }
}