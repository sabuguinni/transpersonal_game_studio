#include "Perf_FPSMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"

UPerf_FPSMonitor::UPerf_FPSMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    MonitoringInterval = 0.1f;
    SampleCount = 100;
    TargetFPS = 60.0f;
    bAutoOptimize = true;
    bLogPerformanceWarnings = true;
    bIsMonitoring = false;
    
    FPSSamples.Reserve(SampleCount);
    CurrentSampleIndex = 0;
    TotalFPSSum = 0.0f;
    LastUpdateTime = 0.0f;
}

void UPerf_FPSMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize metrics
    ResetMetrics();
    
    // Start monitoring automatically
    if (bAutoOptimize)
    {
        StartMonitoring();
    }
}

void UPerf_FPSMonitor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopMonitoring();
    Super::EndPlay(EndPlayReason);
}

void UPerf_FPSMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMonitoring)
    {
        UpdateFPSMetrics();
    }
}

void UPerf_FPSMonitor::UpdateFPSMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Calculate current FPS
    float DeltaTime = World->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    }
    
    // Add to samples array
    if (FPSSamples.Num() < SampleCount)
    {
        FPSSamples.Add(CurrentMetrics.CurrentFPS);
        TotalFPSSum += CurrentMetrics.CurrentFPS;
    }
    else
    {
        // Replace oldest sample
        TotalFPSSum -= FPSSamples[CurrentSampleIndex];
        FPSSamples[CurrentSampleIndex] = CurrentMetrics.CurrentFPS;
        TotalFPSSum += CurrentMetrics.CurrentFPS;
        CurrentSampleIndex = (CurrentSampleIndex + 1) % SampleCount;
    }
    
    // Calculate statistics
    if (FPSSamples.Num() > 0)
    {
        CurrentMetrics.AverageFPS = TotalFPSSum / FPSSamples.Num();
        
        // Find min/max
        CurrentMetrics.MinFPS = FPSSamples[0];
        CurrentMetrics.MaxFPS = FPSSamples[0];
        
        for (float Sample : FPSSamples)
        {
            CurrentMetrics.MinFPS = FMath::Min(CurrentMetrics.MinFPS, Sample);
            CurrentMetrics.MaxFPS = FMath::Max(CurrentMetrics.MaxFPS, Sample);
        }
    }
    
    // Update performance level
    CalculatePerformanceLevel();
    
    // Count actors for performance context
    CountActors();
    
    // Log warnings if needed
    if (bLogPerformanceWarnings && CurrentMetrics.PerformanceLevel == EPerf_PerformanceLevel::Poor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warning: FPS dropped to %.1f (Target: %.1f)"), 
               CurrentMetrics.CurrentFPS, TargetFPS);
    }
    
    // Trigger optimization if needed
    if (bAutoOptimize && CurrentMetrics.PerformanceLevel == EPerf_PerformanceLevel::Critical)
    {
        TriggerOptimization();
    }
}

void UPerf_FPSMonitor::CalculatePerformanceLevel()
{
    float FPS = CurrentMetrics.CurrentFPS;
    
    if (FPS >= 60.0f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Excellent;
    }
    else if (FPS >= 45.0f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Good;
    }
    else if (FPS >= 30.0f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Fair;
    }
    else if (FPS >= 15.0f)
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Poor;
    }
    else
    {
        CurrentMetrics.PerformanceLevel = EPerf_PerformanceLevel::Critical;
    }
}

void UPerf_FPSMonitor::CountActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    CurrentMetrics.PhysicsActorCount = 0;
    CurrentMetrics.VisibleActorCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor->IsPendingKill())
        {
            continue;
        }
        
        CurrentMetrics.VisibleActorCount++;
        
        // Check if actor has physics simulation
        UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            CurrentMetrics.PhysicsActorCount++;
        }
    }
}

void UPerf_FPSMonitor::TriggerOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("Performance Critical: Triggering automatic optimization"));
    
    // Basic optimization: Reduce physics simulation quality
    UWorld* World = GetWorld();
    if (World)
    {
        // This could be expanded to actually perform optimizations
        UE_LOG(LogTemp, Log, TEXT("Optimization triggered - Physics actors: %d, Visible actors: %d"), 
               CurrentMetrics.PhysicsActorCount, CurrentMetrics.VisibleActorCount);
    }
}

bool UPerf_FPSMonitor::IsPerformanceAcceptable() const
{
    return CurrentMetrics.PerformanceLevel != EPerf_PerformanceLevel::Critical &&
           CurrentMetrics.PerformanceLevel != EPerf_PerformanceLevel::Poor;
}

void UPerf_FPSMonitor::ResetMetrics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    FPSSamples.Empty();
    TotalFPSSum = 0.0f;
    CurrentSampleIndex = 0;
}

void UPerf_FPSMonitor::StartMonitoring()
{
    if (!bIsMonitoring)
    {
        bIsMonitoring = true;
        ResetMetrics();
        
        UWorld* World = GetWorld();
        if (World)
        {
            World->GetTimerManager().SetTimer(MonitoringTimer, this, &UPerf_FPSMonitor::MonitoringTick, 
                                            MonitoringInterval, true);
        }
        
        UE_LOG(LogTemp, Log, TEXT("FPS Monitoring started"));
    }
}

void UPerf_FPSMonitor::StopMonitoring()
{
    if (bIsMonitoring)
    {
        bIsMonitoring = false;
        
        UWorld* World = GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(MonitoringTimer);
        }
        
        UE_LOG(LogTemp, Log, TEXT("FPS Monitoring stopped"));
    }
}

void UPerf_FPSMonitor::LogCurrentMetrics()
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE METRICS ==="));
    UE_LOG(LogTemp, Log, TEXT("Current FPS: %.1f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Log, TEXT("Average FPS: %.1f"), CurrentMetrics.AverageFPS);
    UE_LOG(LogTemp, Log, TEXT("Min FPS: %.1f"), CurrentMetrics.MinFPS);
    UE_LOG(LogTemp, Log, TEXT("Max FPS: %.1f"), CurrentMetrics.MaxFPS);
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Log, TEXT("Performance Level: %d"), (int32)CurrentMetrics.PerformanceLevel);
    UE_LOG(LogTemp, Log, TEXT("Physics Actors: %d"), CurrentMetrics.PhysicsActorCount);
    UE_LOG(LogTemp, Log, TEXT("Visible Actors: %d"), CurrentMetrics.VisibleActorCount);
    UE_LOG(LogTemp, Log, TEXT("========================"));
}

void UPerf_FPSMonitor::MonitoringTick()
{
    // This is called by the timer for additional monitoring tasks
    // Currently, the main monitoring is done in TickComponent
}