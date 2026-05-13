#include "Perf_TerrainPhysicsPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Landscape/Landscape.h"

UPerf_TerrainPhysicsPerformanceIntegrator::UPerf_TerrainPhysicsPerformanceIntegrator()
    : CurrentOptimizationLevel(EPerf_TerrainPhysicsOptimizationLevel::High)
    , bIsMonitoring(false)
    , bAdaptiveOptimizationEnabled(true)
    , LastFrameTime(0.0f)
    , AverageFrameTime(0.0f)
    , FrameCounter(0)
{
    // Initialize optimization settings with balanced defaults
    OptimizationSettings.MaxPhysicsUpdateDistance = 5000.0f;
    OptimizationSettings.MaxActivePhysicsActors = 500;
    OptimizationSettings.PhysicsLODDistance1 = 1000.0f;
    OptimizationSettings.PhysicsLODDistance2 = 2500.0f;
    OptimizationSettings.PhysicsLODDistance3 = 5000.0f;
    OptimizationSettings.bEnableAdaptivePhysics = true;
    OptimizationSettings.TargetFrameTime = 16.67f;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Terrain Physics Performance Integrator initialized"));
    
    // Start performance monitoring automatically
    StartPerformanceMonitoring();
    
    // Enable adaptive optimization by default
    EnableAdaptiveOptimization(true);
    
    // Integrate with terrain physics system
    IntegrateWithTerrainPhysics();
}

void UPerf_TerrainPhysicsPerformanceIntegrator::Deinitialize()
{
    StopPerformanceMonitoring();
    Super::Deinitialize();
}

void UPerf_TerrainPhysicsPerformanceIntegrator::StartPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = true;
    FrameCounter = 0;
    AverageFrameTime = 0.0f;

    // Start performance monitoring timer (update every 0.1 seconds)
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceMonitorTimer,
            this,
            &UPerf_TerrainPhysicsPerformanceIntegrator::UpdatePerformanceMetrics,
            0.1f,
            true
        );
        
        UE_LOG(LogTemp, Log, TEXT("Terrain Physics Performance monitoring started"));
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::StopPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }

    bIsMonitoring = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceMonitorTimer);
        World->GetTimerManager().ClearTimer(AdaptiveOptimizationTimer);
    }

    UE_LOG(LogTemp, Log, TEXT("Terrain Physics Performance monitoring stopped"));
}

FPerf_TerrainPhysicsMetrics UPerf_TerrainPhysicsPerformanceIntegrator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::SetOptimizationLevel(EPerf_TerrainPhysicsOptimizationLevel Level)
{
    CurrentOptimizationLevel = Level;
    
    switch (Level)
    {
        case EPerf_TerrainPhysicsOptimizationLevel::Ultra:
            ApplyUltraOptimization();
            break;
        case EPerf_TerrainPhysicsOptimizationLevel::High:
            ApplyHighOptimization();
            break;
        case EPerf_TerrainPhysicsOptimizationLevel::Medium:
            ApplyMediumOptimization();
            break;
        case EPerf_TerrainPhysicsOptimizationLevel::Low:
            ApplyLowOptimization();
            break;
        case EPerf_TerrainPhysicsOptimizationLevel::Minimal:
            ApplyMinimalOptimization();
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Terrain Physics optimization level set to: %d"), (int32)Level);
}

EPerf_TerrainPhysicsOptimizationLevel UPerf_TerrainPhysicsPerformanceIntegrator::GetCurrentOptimizationLevel() const
{
    return CurrentOptimizationLevel;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::ApplyOptimizationSettings(const FPerf_TerrainPhysicsOptimizationSettings& Settings)
{
    OptimizationSettings = Settings;
    
    // Apply the new settings immediately
    OptimizeTerrainPhysicsActors();
    
    UE_LOG(LogTemp, Log, TEXT("Terrain Physics optimization settings applied"));
}

void UPerf_TerrainPhysicsPerformanceIntegrator::EnableAdaptiveOptimization(bool bEnable)
{
    bAdaptiveOptimizationEnabled = bEnable;
    
    if (bEnable && bIsMonitoring)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                AdaptiveOptimizationTimer,
                this,
                &UPerf_TerrainPhysicsPerformanceIntegrator::UpdateAdaptiveOptimization,
                1.0f,
                true
            );
        }
        UE_LOG(LogTemp, Log, TEXT("Adaptive terrain physics optimization enabled"));
    }
    else
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(AdaptiveOptimizationTimer);
        }
        UE_LOG(LogTemp, Log, TEXT("Adaptive terrain physics optimization disabled"));
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::UpdateAdaptiveOptimization()
{
    if (!bAdaptiveOptimizationEnabled || !bIsMonitoring)
    {
        return;
    }

    CheckFrameTimeThresholds();
}

void UPerf_TerrainPhysicsPerformanceIntegrator::IntegrateWithTerrainPhysics()
{
    FindTerrainPhysicsActors();
    OptimizeTerrainPhysicsActors();
    
    UE_LOG(LogTemp, Log, TEXT("Integrated with terrain physics system"));
}

void UPerf_TerrainPhysicsPerformanceIntegrator::OptimizeTerrainPhysicsActors()
{
    OptimizePhysicsActorsByDistance();
    UpdatePhysicsLOD();
}

void UPerf_TerrainPhysicsPerformanceIntegrator::AnalyzePhysicsPerformance()
{
    MonitorPhysicsPerformance();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Analysis:"));
    UE_LOG(LogTemp, Log, TEXT("- Physics Update Time: %.2fms"), CurrentMetrics.PhysicsUpdateTime);
    UE_LOG(LogTemp, Log, TEXT("- Collision Check Time: %.2fms"), CurrentMetrics.CollisionCheckTime);
    UE_LOG(LogTemp, Log, TEXT("- Active Physics Actors: %d"), CurrentMetrics.ActivePhysicsActors);
    UE_LOG(LogTemp, Log, TEXT("- Terrain Collision Queries: %d"), CurrentMetrics.TerrainCollisionQueries);
    UE_LOG(LogTemp, Log, TEXT("- Memory Usage: %.2fMB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("- Frame Time: %.2fms"), CurrentMetrics.FrameTime);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::GeneratePerformanceReport()
{
    AnalyzePhysicsPerformance();
    
    FString ReportContent = FString::Printf(TEXT(
        "Terrain Physics Performance Report - %s\n"
        "========================================\n"
        "Optimization Level: %d\n"
        "Physics Update Time: %.2fms\n"
        "Collision Check Time: %.2fms\n"
        "Active Physics Actors: %d\n"
        "Terrain Collision Queries: %d\n"
        "Memory Usage: %.2fMB\n"
        "Average Frame Time: %.2fms\n"
        "Target Frame Time: %.2fms\n"
        "Performance Status: %s\n"
    ),
    *FDateTime::Now().ToString(),
    (int32)CurrentOptimizationLevel,
    CurrentMetrics.PhysicsUpdateTime,
    CurrentMetrics.CollisionCheckTime,
    CurrentMetrics.ActivePhysicsActors,
    CurrentMetrics.TerrainCollisionQueries,
    CurrentMetrics.MemoryUsageMB,
    AverageFrameTime,
    OptimizationSettings.TargetFrameTime,
    AverageFrameTime <= OptimizationSettings.TargetFrameTime ? TEXT("OPTIMAL") : TEXT("NEEDS OPTIMIZATION")
    );
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *ReportContent);
}

void UPerf_TerrainPhysicsPerformanceIntegrator::UpdatePerformanceMetrics()
{
    if (!bIsMonitoring)
    {
        return;
    }

    MonitorPhysicsPerformance();
    
    // Update frame time tracking
    FrameCounter++;
    float CurrentFrameTime = CurrentMetrics.FrameTime;
    
    if (FrameCounter == 1)
    {
        AverageFrameTime = CurrentFrameTime;
    }
    else
    {
        // Rolling average
        AverageFrameTime = (AverageFrameTime * 0.9f) + (CurrentFrameTime * 0.1f);
    }
    
    LastFrameTime = CurrentFrameTime;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::MonitorPhysicsPerformance()
{
    // Get current frame time
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.FrameTime = World->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    }
    
    // Count active physics actors
    CurrentMetrics.ActivePhysicsActors = 0;
    CurrentMetrics.TerrainCollisionQueries = 0;
    
    for (const TWeakObjectPtr<AActor>& ActorPtr : TerrainPhysicsActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                if (MeshComp->IsSimulatingPhysics())
                {
                    CurrentMetrics.ActivePhysicsActors++;
                }
            }
        }
    }
    
    // Estimate physics update time (simplified calculation)
    CurrentMetrics.PhysicsUpdateTime = CurrentMetrics.ActivePhysicsActors * 0.02f; // 0.02ms per actor
    CurrentMetrics.CollisionCheckTime = CurrentMetrics.TerrainCollisionQueries * 0.01f; // 0.01ms per query
    
    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = CurrentMetrics.ActivePhysicsActors * 0.1f; // 0.1MB per actor
}

void UPerf_TerrainPhysicsPerformanceIntegrator::CheckFrameTimeThresholds()
{
    if (AverageFrameTime > PERFORMANCE_THRESHOLD_CRITICAL)
    {
        // Critical performance - switch to minimal optimization
        if (CurrentOptimizationLevel != EPerf_TerrainPhysicsOptimizationLevel::Minimal)
        {
            SetOptimizationLevel(EPerf_TerrainPhysicsOptimizationLevel::Minimal);
            UE_LOG(LogTemp, Warning, TEXT("Critical performance detected - switching to minimal optimization"));
        }
    }
    else if (AverageFrameTime > PERFORMANCE_THRESHOLD_WARNING)
    {
        // Warning performance - switch to low optimization
        if (CurrentOptimizationLevel != EPerf_TerrainPhysicsOptimizationLevel::Low)
        {
            SetOptimizationLevel(EPerf_TerrainPhysicsOptimizationLevel::Low);
            UE_LOG(LogTemp, Warning, TEXT("Performance warning - switching to low optimization"));
        }
    }
    else if (AverageFrameTime <= PERFORMANCE_THRESHOLD_OPTIMAL)
    {
        // Good performance - can use higher quality
        if (CurrentOptimizationLevel == EPerf_TerrainPhysicsOptimizationLevel::Minimal ||
            CurrentOptimizationLevel == EPerf_TerrainPhysicsOptimizationLevel::Low)
        {
            SetOptimizationLevel(EPerf_TerrainPhysicsOptimizationLevel::Medium);
            UE_LOG(LogTemp, Log, TEXT("Good performance - upgrading to medium optimization"));
        }
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::ApplyUltraOptimization()
{
    OptimizationSettings.MaxPhysicsUpdateDistance = 8000.0f;
    OptimizationSettings.MaxActivePhysicsActors = 1000;
    OptimizationSettings.PhysicsLODDistance1 = 1500.0f;
    OptimizationSettings.PhysicsLODDistance2 = 4000.0f;
    OptimizationSettings.PhysicsLODDistance3 = 8000.0f;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::ApplyHighOptimization()
{
    OptimizationSettings.MaxPhysicsUpdateDistance = 5000.0f;
    OptimizationSettings.MaxActivePhysicsActors = 500;
    OptimizationSettings.PhysicsLODDistance1 = 1000.0f;
    OptimizationSettings.PhysicsLODDistance2 = 2500.0f;
    OptimizationSettings.PhysicsLODDistance3 = 5000.0f;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::ApplyMediumOptimization()
{
    OptimizationSettings.MaxPhysicsUpdateDistance = 3000.0f;
    OptimizationSettings.MaxActivePhysicsActors = 300;
    OptimizationSettings.PhysicsLODDistance1 = 800.0f;
    OptimizationSettings.PhysicsLODDistance2 = 1500.0f;
    OptimizationSettings.PhysicsLODDistance3 = 3000.0f;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::ApplyLowOptimization()
{
    OptimizationSettings.MaxPhysicsUpdateDistance = 2000.0f;
    OptimizationSettings.MaxActivePhysicsActors = 150;
    OptimizationSettings.PhysicsLODDistance1 = 500.0f;
    OptimizationSettings.PhysicsLODDistance2 = 1000.0f;
    OptimizationSettings.PhysicsLODDistance3 = 2000.0f;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::ApplyMinimalOptimization()
{
    OptimizationSettings.MaxPhysicsUpdateDistance = 1000.0f;
    OptimizationSettings.MaxActivePhysicsActors = 50;
    OptimizationSettings.PhysicsLODDistance1 = 300.0f;
    OptimizationSettings.PhysicsLODDistance2 = 600.0f;
    OptimizationSettings.PhysicsLODDistance3 = 1000.0f;
}

void UPerf_TerrainPhysicsPerformanceIntegrator::FindTerrainPhysicsActors()
{
    TerrainPhysicsActors.Empty();
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                // Check if actor has physics simulation enabled
                if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
                {
                    if (MeshComp->IsSimulatingPhysics() || MeshComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                    {
                        TerrainPhysicsActors.Add(Actor);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Found %d terrain physics actors"), TerrainPhysicsActors.Num());
}

void UPerf_TerrainPhysicsPerformanceIntegrator::OptimizePhysicsActorsByDistance()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get player location for distance calculations
    FVector PlayerLocation = FVector::ZeroVector;
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    int32 ActiveCount = 0;
    for (const TWeakObjectPtr<AActor>& ActorPtr : TerrainPhysicsActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            
            if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                // Disable physics for distant actors
                if (Distance > OptimizationSettings.MaxPhysicsUpdateDistance)
                {
                    MeshComp->SetSimulatePhysics(false);
                }
                else if (ActiveCount < OptimizationSettings.MaxActivePhysicsActors)
                {
                    MeshComp->SetSimulatePhysics(true);
                    ActiveCount++;
                }
            }
        }
    }
}

void UPerf_TerrainPhysicsPerformanceIntegrator::UpdatePhysicsLOD()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get player location for LOD calculations
    FVector PlayerLocation = FVector::ZeroVector;
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    for (const TWeakObjectPtr<AActor>& ActorPtr : TerrainPhysicsActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            
            if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                // Apply LOD based on distance
                if (Distance <= OptimizationSettings.PhysicsLODDistance1)
                {
                    // High detail physics
                    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                }
                else if (Distance <= OptimizationSettings.PhysicsLODDistance2)
                {
                    // Medium detail physics
                    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                }
                else if (Distance <= OptimizationSettings.PhysicsLODDistance3)
                {
                    // Low detail physics
                    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                }
                else
                {
                    // No physics
                    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                }
            }
        }
    }
}