#include "Perf_FrameRateManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"

UPerf_FrameRateManager::UPerf_FrameRateManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f;

    PerformanceTarget = EPerf_PerformanceTarget::PC_60FPS;
    bAdaptivePerformanceEnabled = true;
    TargetFrameRate = 60.0f;
    PerformanceCheckInterval = 1.0f;
    MaxPhysicsActors = 500;
    LODDistanceMultiplier = 1.0f;

    bMonitoringActive = false;
    MonitoringTimer = 0.0f;
    FrameCount = 0;
    TotalFrameTime = 0.0f;

    FrameTimeHistory.Reserve(300); // Store 5 seconds at 60fps
}

void UPerf_FrameRateManager::BeginPlay()
{
    Super::BeginPlay();
    
    StartPerformanceMonitoring();
    
    // Set initial performance target
    switch (PerformanceTarget)
    {
        case EPerf_PerformanceTarget::Console_30FPS:
            TargetFrameRate = 30.0f;
            break;
        case EPerf_PerformanceTarget::PC_60FPS:
            TargetFrameRate = 60.0f;
            break;
        case EPerf_PerformanceTarget::PC_120FPS:
            TargetFrameRate = 120.0f;
            break;
        case EPerf_PerformanceTarget::Adaptive:
            TargetFrameRate = 60.0f; // Start with 60fps
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("FrameRateManager initialized - Target: %.1f FPS"), TargetFrameRate);
}

void UPerf_FrameRateManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bMonitoringActive)
    {
        UpdateFrameData(DeltaTime);
        
        MonitoringTimer += DeltaTime;
        if (MonitoringTimer >= PerformanceCheckInterval)
        {
            if (bAdaptivePerformanceEnabled)
            {
                CheckAdaptivePerformance();
            }
            MonitoringTimer = 0.0f;
        }
    }
}

void UPerf_FrameRateManager::StartPerformanceMonitoring()
{
    bMonitoringActive = true;
    ResetPerformanceStats();
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
}

void UPerf_FrameRateManager::StopPerformanceMonitoring()
{
    bMonitoringActive = false;
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
}

FPerf_FrameData UPerf_FrameRateManager::GetCurrentFrameData() const
{
    return CurrentFrameData;
}

void UPerf_FrameRateManager::SetPerformanceTarget(EPerf_PerformanceTarget Target)
{
    PerformanceTarget = Target;
    
    switch (Target)
    {
        case EPerf_PerformanceTarget::Console_30FPS:
            TargetFrameRate = 30.0f;
            MaxPhysicsActors = 200;
            LODDistanceMultiplier = 0.7f;
            break;
        case EPerf_PerformanceTarget::PC_60FPS:
            TargetFrameRate = 60.0f;
            MaxPhysicsActors = 500;
            LODDistanceMultiplier = 1.0f;
            break;
        case EPerf_PerformanceTarget::PC_120FPS:
            TargetFrameRate = 120.0f;
            MaxPhysicsActors = 1000;
            LODDistanceMultiplier = 1.5f;
            break;
        case EPerf_PerformanceTarget::Adaptive:
            // Keep current settings, will be adjusted automatically
            break;
    }

    ApplyPerformanceOptimizations();
    UE_LOG(LogTemp, Log, TEXT("Performance target set to %.1f FPS"), TargetFrameRate);
}

void UPerf_FrameRateManager::EnableAdaptivePerformance(bool bEnable)
{
    bAdaptivePerformanceEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Adaptive performance %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_FrameRateManager::OptimizeForCurrentFrameRate()
{
    if (CurrentFrameData.AverageFPS < TargetFrameRate * 0.8f) // Below 80% of target
    {
        // Reduce quality settings
        MaxPhysicsActors = FMath::Max(100, MaxPhysicsActors - 50);
        LODDistanceMultiplier = FMath::Max(0.5f, LODDistanceMultiplier - 0.1f);
        
        UE_LOG(LogTemp, Warning, TEXT("Performance below target - reducing quality"));
    }
    else if (CurrentFrameData.AverageFPS > TargetFrameRate * 1.2f) // Above 120% of target
    {
        // Increase quality settings
        MaxPhysicsActors = FMath::Min(1000, MaxPhysicsActors + 25);
        LODDistanceMultiplier = FMath::Min(2.0f, LODDistanceMultiplier + 0.05f);
        
        UE_LOG(LogTemp, Log, TEXT("Performance above target - increasing quality"));
    }

    ApplyPerformanceOptimizations();
}

void UPerf_FrameRateManager::SetMaxPhysicsActors(int32 MaxActors)
{
    MaxPhysicsActors = FMath::Clamp(MaxActors, 50, 2000);
    ApplyPerformanceOptimizations();
}

void UPerf_FrameRateManager::SetLODDistanceMultiplier(float Multiplier)
{
    LODDistanceMultiplier = FMath::Clamp(Multiplier, 0.1f, 3.0f);
    ApplyPerformanceOptimizations();
}

void UPerf_FrameRateManager::LogPerformanceStats()
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE STATS ==="));
    UE_LOG(LogTemp, Log, TEXT("Current FPS: %.1f"), CurrentFrameData.CurrentFPS);
    UE_LOG(LogTemp, Log, TEXT("Average FPS: %.1f"), CurrentFrameData.AverageFPS);
    UE_LOG(LogTemp, Log, TEXT("Min FPS: %.1f"), CurrentFrameData.MinFPS);
    UE_LOG(LogTemp, Log, TEXT("Max FPS: %.1f"), CurrentFrameData.MaxFPS);
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentFrameData.FrameTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("Target FPS: %.1f"), TargetFrameRate);
    UE_LOG(LogTemp, Log, TEXT("Max Physics Actors: %d"), MaxPhysicsActors);
    UE_LOG(LogTemp, Log, TEXT("LOD Distance Multiplier: %.2f"), LODDistanceMultiplier);
    UE_LOG(LogTemp, Log, TEXT("========================"));
}

void UPerf_FrameRateManager::ResetPerformanceStats()
{
    CurrentFrameData = FPerf_FrameData();
    FrameTimeHistory.Empty();
    FrameCount = 0;
    TotalFrameTime = 0.0f;
    MonitoringTimer = 0.0f;
}

void UPerf_FrameRateManager::UpdateFrameData(float DeltaTime)
{
    FrameCount++;
    TotalFrameTime += DeltaTime;

    // Current FPS
    CurrentFrameData.CurrentFPS = (DeltaTime > 0.0f) ? 1.0f / DeltaTime : 0.0f;
    CurrentFrameData.FrameTime = DeltaTime;

    // Store frame time history
    FrameTimeHistory.Add(DeltaTime);
    if (FrameTimeHistory.Num() > 300) // Keep last 5 seconds at 60fps
    {
        FrameTimeHistory.RemoveAt(0);
    }

    // Calculate average FPS from history
    if (FrameTimeHistory.Num() > 0)
    {
        float AverageFrameTime = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            AverageFrameTime += FrameTime;
        }
        AverageFrameTime /= FrameTimeHistory.Num();
        CurrentFrameData.AverageFPS = (AverageFrameTime > 0.0f) ? 1.0f / AverageFrameTime : 0.0f;
    }

    // Update min/max FPS
    if (CurrentFrameData.CurrentFPS < CurrentFrameData.MinFPS && CurrentFrameData.CurrentFPS > 0.0f)
    {
        CurrentFrameData.MinFPS = CurrentFrameData.CurrentFPS;
    }
    if (CurrentFrameData.CurrentFPS > CurrentFrameData.MaxFPS)
    {
        CurrentFrameData.MaxFPS = CurrentFrameData.CurrentFPS;
    }

    // Estimate render and game time (simplified)
    CurrentFrameData.RenderTime = DeltaTime * 0.6f; // Assume 60% render
    CurrentFrameData.GameTime = DeltaTime * 0.4f;   // Assume 40% game logic
}

void UPerf_FrameRateManager::ApplyPerformanceOptimizations()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Apply physics actor limits
    TArray<AActor*> PhysicsActors;
    int32 ActivePhysicsCount = 0;
    
    for (AActor* Actor : World->GetCurrentLevel()->Actors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp && MeshComp->GetBodyInstance())
            {
                ActivePhysicsCount++;
                if (ActivePhysicsCount > MaxPhysicsActors)
                {
                    // Disable physics on distant actors
                    float Distance = FVector::Dist(Actor->GetActorLocation(), 
                        World->GetFirstPlayerController()->GetPawn()->GetActorLocation());
                    
                    if (Distance > 5000.0f * LODDistanceMultiplier)
                    {
                        MeshComp->SetSimulatePhysics(false);
                    }
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Applied performance optimizations - Active physics actors: %d"), 
           FMath::Min(ActivePhysicsCount, MaxPhysicsActors));
}

void UPerf_FrameRateManager::CheckAdaptivePerformance()
{
    if (PerformanceTarget != EPerf_PerformanceTarget::Adaptive) return;

    float PerformanceRatio = CurrentFrameData.AverageFPS / TargetFrameRate;

    if (PerformanceRatio < 0.85f) // Below 85% of target
    {
        // Reduce target frame rate
        if (TargetFrameRate > 30.0f)
        {
            TargetFrameRate = FMath::Max(30.0f, TargetFrameRate - 5.0f);
            UE_LOG(LogTemp, Warning, TEXT("Adaptive: Reducing target to %.1f FPS"), TargetFrameRate);
        }
        OptimizeForCurrentFrameRate();
    }
    else if (PerformanceRatio > 1.15f) // Above 115% of target
    {
        // Increase target frame rate
        if (TargetFrameRate < 120.0f)
        {
            TargetFrameRate = FMath::Min(120.0f, TargetFrameRate + 5.0f);
            UE_LOG(LogTemp, Log, TEXT("Adaptive: Increasing target to %.1f FPS"), TargetFrameRate);
        }
        OptimizeForCurrentFrameRate();
    }
}