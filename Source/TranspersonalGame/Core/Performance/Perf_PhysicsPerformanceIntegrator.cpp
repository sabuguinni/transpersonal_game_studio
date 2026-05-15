#include "Perf_PhysicsPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

UPerf_PhysicsPerformanceIntegrator::UPerf_PhysicsPerformanceIntegrator()
{
    CurrentQualityLevel = EPerf_PhysicsQualityLevel::High;
    bIsMonitoring = false;
    LastFrameTime = 0.0f;
    FrameCounter = 0;
    LastPerformanceCheck = 0.0;
    AverageFrameTime = 16.67f; // 60 FPS target
    QualityAdjustmentCount = 0;
    
    // Initialize default settings
    OptimizationSettings = FPerf_PhysicsOptimizationSettings();
    CurrentMetrics = FPerf_PhysicsPerformanceMetrics();
    
    FrameTimeHistory.Reserve(120); // 2 seconds at 60fps
}

void UPerf_PhysicsPerformanceIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsPerformanceIntegrator: Initializing physics performance optimization system"));
    
    // Start monitoring after a brief delay
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceMonitoringTimer,
            this,
            &UPerf_PhysicsPerformanceIntegrator::StartPerformanceMonitoring,
            2.0f,
            false
        );
    }
}

void UPerf_PhysicsPerformanceIntegrator::Deinitialize()
{
    StopPerformanceMonitoring();
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceMonitoringTimer);
        World->GetTimerManager().ClearTimer(QualityAdjustmentTimer);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsPerformanceIntegrator: Deinitialized"));
    
    Super::Deinitialize();
}

void UPerf_PhysicsPerformanceIntegrator::StartPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = true;
    FrameCounter = 0;
    FrameTimeHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsPerformanceIntegrator: Started performance monitoring"));
    
    if (UWorld* World = GetWorld())
    {
        // Monitor performance every frame
        World->GetTimerManager().SetTimer(
            PerformanceMonitoringTimer,
            this,
            &UPerf_PhysicsPerformanceIntegrator::UpdatePerformanceMetrics,
            0.016f, // ~60fps
            true
        );
        
        // Check for quality adjustments every 2 seconds
        World->GetTimerManager().SetTimer(
            QualityAdjustmentTimer,
            this,
            &UPerf_PhysicsPerformanceIntegrator::CheckPerformanceThresholds,
            2.0f,
            true
        );
    }
}

void UPerf_PhysicsPerformanceIntegrator::StopPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceMonitoringTimer);
        World->GetTimerManager().ClearTimer(QualityAdjustmentTimer);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsPerformanceIntegrator: Stopped performance monitoring"));
}

FPerf_PhysicsPerformanceMetrics UPerf_PhysicsPerformanceIntegrator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PhysicsPerformanceIntegrator::SetPhysicsQualityLevel(EPerf_PhysicsQualityLevel QualityLevel)
{
    if (CurrentQualityLevel == QualityLevel)
    {
        return;
    }
    
    CurrentQualityLevel = QualityLevel;
    
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsPerformanceIntegrator: Set quality level to %d"), (int32)QualityLevel);
    
    // Apply quality settings
    switch (QualityLevel)
    {
        case EPerf_PhysicsQualityLevel::Low:
            OptimizationSettings.MaxSimulatingBodies = 200;
            OptimizationSettings.CullingDistance = 3000.0f;
            break;
            
        case EPerf_PhysicsQualityLevel::Medium:
            OptimizationSettings.MaxSimulatingBodies = 350;
            OptimizationSettings.CullingDistance = 4000.0f;
            break;
            
        case EPerf_PhysicsQualityLevel::High:
            OptimizationSettings.MaxSimulatingBodies = 500;
            OptimizationSettings.CullingDistance = 5000.0f;
            break;
            
        case EPerf_PhysicsQualityLevel::Ultra:
            OptimizationSettings.MaxSimulatingBodies = 750;
            OptimizationSettings.CullingDistance = 6000.0f;
            break;
    }
    
    // Apply optimizations immediately
    OptimizePhysicsActors();
}

EPerf_PhysicsQualityLevel UPerf_PhysicsPerformanceIntegrator::GetCurrentQualityLevel() const
{
    return CurrentQualityLevel;
}

void UPerf_PhysicsPerformanceIntegrator::OptimizePhysicsActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 OptimizedActors = 0;
    
    // Get all actors in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        // Find primitive components with physics
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                OptimizeRigidBody(PrimComp);
                OptimizedActors++;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsPerformanceIntegrator: Optimized %d physics actors"), OptimizedActors);
}

void UPerf_PhysicsPerformanceIntegrator::OptimizeRigidBody(UPrimitiveComponent* PrimComp)
{
    if (!PrimComp)
    {
        return;
    }
    
    // Adjust physics settings based on quality level
    switch (CurrentQualityLevel)
    {
        case EPerf_PhysicsQualityLevel::Low:
            PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
            PrimComp->SetNotifyRigidBodyCollision(false);
            break;
            
        case EPerf_PhysicsQualityLevel::Medium:
            PrimComp->SetNotifyRigidBodyCollision(false);
            break;
            
        case EPerf_PhysicsQualityLevel::High:
        case EPerf_PhysicsQualityLevel::Ultra:
            // Keep full physics simulation
            break;
    }
}

void UPerf_PhysicsPerformanceIntegrator::CullDistantPhysicsObjects()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get player location for distance calculations
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    int32 CulledObjects = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor == PlayerPawn)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        if (Distance > OptimizationSettings.CullingDistance)
        {
            // Disable physics simulation for distant objects
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    PrimComp->SetSimulatePhysics(false);
                    CulledObjects++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsPerformanceIntegrator: Culled %d distant physics objects"), CulledObjects);
}

void UPerf_PhysicsPerformanceIntegrator::AdjustPhysicsLOD()
{
    if (!OptimizationSettings.bEnablePhysicsLOD)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Implement distance-based LOD for physics simulation
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor == PlayerPawn)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (!PrimComp)
            {
                continue;
            }
            
            // Adjust physics quality based on distance
            if (Distance < 1000.0f)
            {
                // High detail physics
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
            else if (Distance < 3000.0f)
            {
                // Medium detail physics
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            }
            else
            {
                // Low detail physics
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
    }
}

void UPerf_PhysicsPerformanceIntegrator::IntegrateWithPhysicsManager()
{
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsPerformanceIntegrator: Integrating with Core_RealTimePhysicsManager"));
    
    // Try to find and integrate with the physics manager from Agent #3
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Look for physics manager actors in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Core_RealTimePhysicsManager")))
        {
            UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsPerformanceIntegrator: Found Core_RealTimePhysicsManager: %s"), *Actor->GetName());
            // Integration logic would go here
            break;
        }
    }
}

void UPerf_PhysicsPerformanceIntegrator::ValidatePhysicsIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_PhysicsPerformanceIntegrator: No world found for validation"));
        return;
    }
    
    int32 PhysicsActors = 0;
    int32 SimulatingActors = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->GetBodyInstance())
            {
                PhysicsActors++;
                if (PrimComp->IsSimulatingPhysics())
                {
                    SimulatingActors++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsPerformanceIntegrator: Validation - %d physics actors, %d simulating"), PhysicsActors, SimulatingActors);
    
    CurrentMetrics.ActiveRigidBodies = PhysicsActors;
    CurrentMetrics.SimulatingBodies = SimulatingActors;
}

void UPerf_PhysicsPerformanceIntegrator::SetOptimizationSettings(const FPerf_PhysicsOptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsPerformanceIntegrator: Updated optimization settings"));
}

FPerf_PhysicsOptimizationSettings UPerf_PhysicsPerformanceIntegrator::GetOptimizationSettings() const
{
    return OptimizationSettings;
}

void UPerf_PhysicsPerformanceIntegrator::RunPerformanceTest()
{
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsPerformanceIntegrator: Running performance test"));
    
    ValidatePhysicsIntegration();
    UpdatePerformanceMetrics();
    
    // Log current state
    UE_LOG(LogTemp, Log, TEXT("Performance Test Results:"));
    UE_LOG(LogTemp, Log, TEXT("- Physics Frame Time: %.2f ms"), CurrentMetrics.PhysicsFrameTime);
    UE_LOG(LogTemp, Log, TEXT("- Active Rigid Bodies: %d"), CurrentMetrics.ActiveRigidBodies);
    UE_LOG(LogTemp, Log, TEXT("- Simulating Bodies: %d"), CurrentMetrics.SimulatingBodies);
    UE_LOG(LogTemp, Log, TEXT("- Current Quality Level: %d"), (int32)CurrentQualityLevel);
}

void UPerf_PhysicsPerformanceIntegrator::LogCurrentPerformanceState()
{
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("=== Physics Performance State ==="));
    UE_LOG(LogTemp, Log, TEXT("Quality Level: %d"), (int32)CurrentQualityLevel);
    UE_LOG(LogTemp, Log, TEXT("Monitoring Active: %s"), bIsMonitoring ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Physics Frame Time: %.2f ms"), CurrentMetrics.PhysicsFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Active Bodies: %d"), CurrentMetrics.ActiveRigidBodies);
    UE_LOG(LogTemp, Log, TEXT("Simulating Bodies: %d"), CurrentMetrics.SimulatingBodies);
    UE_LOG(LogTemp, Log, TEXT("Average Frame Time: %.2f ms"), AverageFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Quality Adjustments: %d"), QualityAdjustmentCount);
    UE_LOG(LogTemp, Log, TEXT("================================"));
}

void UPerf_PhysicsPerformanceIntegrator::UpdatePerformanceMetrics()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    // Get current frame time
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Update frame time history
    FrameTimeHistory.Add(CurrentFrameTime);
    if (FrameTimeHistory.Num() > 120) // Keep last 2 seconds
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    AverageFrameTime = FrameTimeHistory.Num() > 0 ? TotalFrameTime / FrameTimeHistory.Num() : 16.67f;
    
    // Update metrics
    CurrentMetrics.PhysicsFrameTime = CurrentFrameTime;
    LastFrameTime = CurrentFrameTime;
    FrameCounter++;
    
    // Update physics-specific metrics
    ValidatePhysicsIntegration();
}

void UPerf_PhysicsPerformanceIntegrator::CheckPerformanceThresholds()
{
    if (!OptimizationSettings.bEnableAdaptiveQuality)
    {
        return;
    }
    
    // Check if we need to adjust quality
    if (AverageFrameTime > OptimizationSettings.QualityAdjustmentThreshold)
    {
        // Performance is poor, reduce quality
        if (CurrentQualityLevel != EPerf_PhysicsQualityLevel::Low)
        {
            EPerf_PhysicsQualityLevel NewLevel = static_cast<EPerf_PhysicsQualityLevel>(
                static_cast<int32>(CurrentQualityLevel) - 1
            );
            SetPhysicsQualityLevel(NewLevel);
            QualityAdjustmentCount++;
            
            UE_LOG(LogTemp, Warning, TEXT("Perf_PhysicsPerformanceIntegrator: Reduced quality due to poor performance (%.2f ms)"), AverageFrameTime);
        }
    }
    else if (AverageFrameTime < OptimizationSettings.QualityAdjustmentThreshold * 0.7f)
    {
        // Performance is good, try to increase quality
        if (CurrentQualityLevel != EPerf_PhysicsQualityLevel::Ultra)
        {
            EPerf_PhysicsQualityLevel NewLevel = static_cast<EPerf_PhysicsQualityLevel>(
                static_cast<int32>(CurrentQualityLevel) + 1
            );
            SetPhysicsQualityLevel(NewLevel);
            QualityAdjustmentCount++;
            
            UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsPerformanceIntegrator: Increased quality due to good performance (%.2f ms)"), AverageFrameTime);
        }
    }
}

void UPerf_PhysicsPerformanceIntegrator::AdjustQualityBasedOnPerformance()
{
    CheckPerformanceThresholds();
    
    // Apply optimizations based on current performance
    if (AverageFrameTime > OptimizationSettings.MaxPhysicsFrameTime)
    {
        CullDistantPhysicsObjects();
        AdjustPhysicsLOD();
    }
}

void UPerf_PhysicsPerformanceIntegrator::OptimizeRigidBodies()
{
    OptimizePhysicsActors();
}

void UPerf_PhysicsPerformanceIntegrator::OptimizeCollisionSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Optimize collision settings based on quality level
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp)
            {
                OptimizeRigidBody(PrimComp);
            }
        }
    }
}

void UPerf_PhysicsPerformanceIntegrator::OptimizePhysicsSubstepping()
{
    // Adjust physics substepping based on performance
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        switch (CurrentQualityLevel)
        {
            case EPerf_PhysicsQualityLevel::Low:
                PhysicsSettings->bSubstepping = false;
                break;
                
            case EPerf_PhysicsQualityLevel::Medium:
                PhysicsSettings->bSubstepping = true;
                PhysicsSettings->MaxSubstepDeltaTime = 0.02f;
                break;
                
            case EPerf_PhysicsQualityLevel::High:
            case EPerf_PhysicsQualityLevel::Ultra:
                PhysicsSettings->bSubstepping = true;
                PhysicsSettings->MaxSubstepDeltaTime = 0.016f;
                break;
        }
    }
}