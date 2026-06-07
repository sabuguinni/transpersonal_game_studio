#include "Perf_PhysicsPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UPerf_PhysicsPerformanceIntegrator::UPerf_PhysicsPerformanceIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms

    // Default performance settings
    TargetFPS = 60.0f;
    MinimumFPS = 30.0f;
    OptimizationLevel = EPerf_PhysicsOptimizationLevel::Balanced;
    QualityMode = EPerf_PhysicsQualityMode::High;

    // Performance budgets
    MaxPhysicsBodies = 1000;
    MaxSimulatingBodies = 200;
    MaxPhysicsStepTime = 5.0f; // 5ms
    MaxCollisionPairs = 500;

    // Optimization controls
    bEnableAutomaticOptimization = true;
    bEnableLODOptimization = true;
    bEnableCollisionOptimization = true;
    bEnableMemoryOptimization = true;

    // Distance thresholds
    NearDistance = 1000.0f;   // 10 meters
    MediumDistance = 5000.0f; // 50 meters
    FarDistance = 10000.0f;   // 100 meters

    // Performance thresholds
    CriticalFPSThreshold = 20.0f;
    WarningFPSThreshold = 40.0f;
    OptimalFPSThreshold = 55.0f;

    // Monitoring settings
    MetricsUpdateInterval = 0.5f;
    OptimizationCheckInterval = 1.0f;
    bEnableDetailedLogging = false;

    // Internal state
    LastMetricsUpdate = 0.0f;
    LastOptimizationCheck = 0.0f;
    bIsMonitoring = false;
    MaxHistorySize = 60; // 60 samples for history

    // Initialize metrics
    CurrentMetrics = FPerf_PhysicsPerformanceMetrics();
}

void UPerf_PhysicsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Starting performance monitoring"));
    
    // Start monitoring automatically
    StartPerformanceMonitoring();
    
    // Apply initial optimization settings
    ApplyOptimizationSettings();
    
    // Initialize history arrays
    FPSHistory.Reserve(MaxHistorySize);
    FrameTimeHistory.Reserve(MaxHistorySize);
}

void UPerf_PhysicsPerformanceIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsMonitoring)
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Update performance metrics
    if (CurrentTime - LastMetricsUpdate >= MetricsUpdateInterval)
    {
        UpdatePerformanceMetrics();
        LastMetricsUpdate = CurrentTime;
    }

    // Check for optimization needs
    if (bEnableAutomaticOptimization && CurrentTime - LastOptimizationCheck >= OptimizationCheckInterval)
    {
        CheckPerformanceThresholds();
        LastOptimizationCheck = CurrentTime;
    }
}

void UPerf_PhysicsPerformanceIntegrator::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    LastMetricsUpdate = GetWorld()->GetTimeSeconds();
    LastOptimizationCheck = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Performance monitoring started"));
    
    if (bEnableDetailedLogging)
    {
        LogPerformanceMetrics();
    }
}

void UPerf_PhysicsPerformanceIntegrator::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Performance monitoring stopped"));
}

FPerf_PhysicsPerformanceMetrics UPerf_PhysicsPerformanceIntegrator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PhysicsPerformanceIntegrator::SetOptimizationLevel(EPerf_PhysicsOptimizationLevel NewLevel)
{
    OptimizationLevel = NewLevel;
    ApplyOptimizationSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Optimization level changed to %d"), (int32)NewLevel);
}

void UPerf_PhysicsPerformanceIntegrator::SetQualityMode(EPerf_PhysicsQualityMode NewMode)
{
    QualityMode = NewMode;
    UpdatePhysicsQuality();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Quality mode changed to %d"), (int32)NewMode);
}

void UPerf_PhysicsPerformanceIntegrator::OptimizePhysicsPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Starting physics performance optimization"));
    
    // Collect current statistics
    CollectPhysicsStatistics();
    
    // Optimize physics bodies
    OptimizePhysicsBodies();
    
    // Optimize collision settings
    OptimizeCollisionSettings();
    
    // Update physics quality based on performance
    UpdatePhysicsQuality();
    
    LogOptimizationAction(TEXT("Full Optimization"), FString::Printf(TEXT("FPS: %.1f, Bodies: %d"), CurrentMetrics.CurrentFPS, CurrentMetrics.ActivePhysicsBodies));
}

void UPerf_PhysicsPerformanceIntegrator::OptimizeNearbyActors(float Radius)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    TArray<AActor*> FoundActors;
    
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), FoundActors);
    
    int32 OptimizedCount = 0;
    for (AActor* Actor : FoundActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        if (Distance <= Radius)
        {
            OptimizeActorPhysics(Actor, Distance);
            OptimizedCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Optimized %d actors within %.1f units"), OptimizedCount, Radius);
}

void UPerf_PhysicsPerformanceIntegrator::OptimizeActorPhysics(AActor* Actor, float Distance)
{
    if (!Actor)
    {
        return;
    }

    // Apply distance-based optimization
    ApplyDistanceBasedOptimization(Actor, Distance);
    
    // Get static mesh component for optimization
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        return;
    }

    // Optimize based on current performance
    if (CurrentMetrics.CurrentFPS < CriticalFPSThreshold)
    {
        // Critical performance - aggressive optimization
        if (Distance > MediumDistance)
        {
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            MeshComp->SetSimulatePhysics(false);
        }
        else if (Distance > NearDistance)
        {
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            MeshComp->SetSimulatePhysics(false);
        }
    }
    else if (CurrentMetrics.CurrentFPS < WarningFPSThreshold)
    {
        // Warning performance - moderate optimization
        if (Distance > FarDistance)
        {
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
    }
}

void UPerf_PhysicsPerformanceIntegrator::AnalyzePhysicsPerformance()
{
    CollectPhysicsStatistics();
    
    float PerformanceScore = GetPerformanceScore();
    bool bIsCritical = IsPerformanceCritical();
    
    UE_LOG(LogTemp, Warning, TEXT("=== Physics Performance Analysis ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.2f (Target: %.2f)"), CurrentMetrics.CurrentFPS, TargetFPS);
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2f ms"), CurrentMetrics.AverageFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Physics Bodies: %d/%d"), CurrentMetrics.ActivePhysicsBodies, MaxPhysicsBodies);
    UE_LOG(LogTemp, Warning, TEXT("Simulating Bodies: %d/%d"), CurrentMetrics.SimulatingBodies, MaxSimulatingBodies);
    UE_LOG(LogTemp, Warning, TEXT("Physics Step Time: %.2f ms"), CurrentMetrics.PhysicsStepTime);
    UE_LOG(LogTemp, Warning, TEXT("Collision Pairs: %d/%d"), CurrentMetrics.CollisionPairs, MaxCollisionPairs);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Performance Score: %.2f"), PerformanceScore);
    UE_LOG(LogTemp, Warning, TEXT("Critical Performance: %s"), bIsCritical ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("==================================="));
}

bool UPerf_PhysicsPerformanceIntegrator::IsPerformanceCritical() const
{
    return CurrentMetrics.CurrentFPS < CriticalFPSThreshold ||
           CurrentMetrics.ActivePhysicsBodies > MaxPhysicsBodies ||
           CurrentMetrics.PhysicsStepTime > MaxPhysicsStepTime;
}

float UPerf_PhysicsPerformanceIntegrator::GetPerformanceScore() const
{
    float FPSScore = FMath::Clamp(CurrentMetrics.CurrentFPS / TargetFPS, 0.0f, 1.0f);
    float BodiesScore = 1.0f - FMath::Clamp((float)CurrentMetrics.ActivePhysicsBodies / MaxPhysicsBodies, 0.0f, 1.0f);
    float StepTimeScore = 1.0f - FMath::Clamp(CurrentMetrics.PhysicsStepTime / MaxPhysicsStepTime, 0.0f, 1.0f);
    
    return (FPSScore + BodiesScore + StepTimeScore) / 3.0f * 100.0f;
}

void UPerf_PhysicsPerformanceIntegrator::LogPerformanceMetrics()
{
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Metrics:"));
    UE_LOG(LogTemp, Log, TEXT("  FPS: %.2f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Log, TEXT("  Frame Time: %.2f ms"), CurrentMetrics.AverageFrameTime);
    UE_LOG(LogTemp, Log, TEXT("  Physics Bodies: %d"), CurrentMetrics.ActivePhysicsBodies);
    UE_LOG(LogTemp, Log, TEXT("  Simulating Bodies: %d"), CurrentMetrics.SimulatingBodies);
    UE_LOG(LogTemp, Log, TEXT("  Physics Step Time: %.2f ms"), CurrentMetrics.PhysicsStepTime);
    UE_LOG(LogTemp, Log, TEXT("  Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
}

void UPerf_PhysicsPerformanceIntegrator::ResetPerformanceCounters()
{
    CurrentMetrics = FPerf_PhysicsPerformanceMetrics();
    FPSHistory.Empty();
    FrameTimeHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Performance counters reset"));
}

void UPerf_PhysicsPerformanceIntegrator::UpdatePerformanceMetrics()
{
    // Calculate current FPS
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    CurrentMetrics.CurrentFPS = DeltaTime > 0.0f ? 1.0f / DeltaTime : 0.0f;
    CurrentMetrics.AverageFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Update FPS history
    UpdateFPSHistory(CurrentMetrics.CurrentFPS);
    
    // Collect physics statistics
    CollectPhysicsStatistics();
    
    // Update average FPS from history
    if (FPSHistory.Num() > 0)
    {
        CurrentMetrics.CurrentFPS = GetAverageFPS();
    }
}

void UPerf_PhysicsPerformanceIntegrator::CheckPerformanceThresholds()
{
    if (CurrentMetrics.CurrentFPS < CriticalFPSThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator: CRITICAL FPS detected (%.2f)"), CurrentMetrics.CurrentFPS);
        
        if (bEnableAutomaticOptimization)
        {
            OptimizePhysicsPerformance();
        }
    }
    else if (CurrentMetrics.CurrentFPS < WarningFPSThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator: Low FPS warning (%.2f)"), CurrentMetrics.CurrentFPS);
        
        if (bEnableAutomaticOptimization)
        {
            // Apply lighter optimization
            OptimizeNearbyActors(FarDistance);
        }
    }
}

void UPerf_PhysicsPerformanceIntegrator::ApplyOptimizationSettings()
{
    switch (OptimizationLevel)
    {
        case EPerf_PhysicsOptimizationLevel::Disabled:
            bEnableAutomaticOptimization = false;
            break;
        case EPerf_PhysicsOptimizationLevel::Conservative:
            MaxPhysicsBodies = 1500;
            MaxSimulatingBodies = 300;
            break;
        case EPerf_PhysicsOptimizationLevel::Balanced:
            MaxPhysicsBodies = 1000;
            MaxSimulatingBodies = 200;
            break;
        case EPerf_PhysicsOptimizationLevel::Aggressive:
            MaxPhysicsBodies = 500;
            MaxSimulatingBodies = 100;
            break;
        case EPerf_PhysicsOptimizationLevel::Maximum:
            MaxPhysicsBodies = 250;
            MaxSimulatingBodies = 50;
            break;
    }
}

void UPerf_PhysicsPerformanceIntegrator::OptimizePhysicsBodies()
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 OptimizedBodies = 0;
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            float Distance = CalculateDistance(Actor);
            if (Distance > FarDistance && CurrentMetrics.SimulatingBodies > MaxSimulatingBodies)
            {
                PrimComp->SetSimulatePhysics(false);
                OptimizedBodies++;
            }
        }
    }
    
    if (OptimizedBodies > 0)
    {
        LogOptimizationAction(TEXT("Physics Bodies"), FString::Printf(TEXT("Disabled %d bodies"), OptimizedBodies));
    }
}

void UPerf_PhysicsPerformanceIntegrator::OptimizeCollisionSettings()
{
    if (!bEnableCollisionOptimization)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), AllActors);
    
    int32 OptimizedCollisions = 0;
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        float Distance = CalculateDistance(Actor);
        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        
        if (MeshComp && Distance > FarDistance)
        {
            if (MeshComp->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics)
            {
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                OptimizedCollisions++;
            }
        }
    }
    
    if (OptimizedCollisions > 0)
    {
        LogOptimizationAction(TEXT("Collision Settings"), FString::Printf(TEXT("Optimized %d collisions"), OptimizedCollisions));
    }
}

void UPerf_PhysicsPerformanceIntegrator::UpdatePhysicsQuality()
{
    // This would integrate with UE5's physics settings
    // For now, we'll just log the quality change
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Physics quality updated to %d"), (int32)QualityMode);
}

float UPerf_PhysicsPerformanceIntegrator::CalculateDistance(const AActor* Actor) const
{
    if (!Actor)
    {
        return 0.0f;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return 0.0f;
    }
    
    return FVector::Dist(PlayerPawn->GetActorLocation(), Actor->GetActorLocation());
}

void UPerf_PhysicsPerformanceIntegrator::ApplyDistanceBasedOptimization(AActor* Actor, float Distance)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp)
    {
        return;
    }
    
    // Apply optimization based on distance
    if (Distance > FarDistance)
    {
        // Far distance - minimal physics
        PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PrimComp->SetSimulatePhysics(false);
    }
    else if (Distance > MediumDistance)
    {
        // Medium distance - query only collision
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        PrimComp->SetSimulatePhysics(false);
    }
    else if (Distance > NearDistance)
    {
        // Near distance - full collision, no physics simulation
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        PrimComp->SetSimulatePhysics(false);
    }
    // Close distance - full physics (no changes needed)
}

void UPerf_PhysicsPerformanceIntegrator::CollectPhysicsStatistics()
{
    // Count physics bodies
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    CurrentMetrics.ActivePhysicsBodies = 0;
    CurrentMetrics.SimulatingBodies = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (PrimComp)
        {
            if (PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                CurrentMetrics.ActivePhysicsBodies++;
            }
            
            if (PrimComp->IsSimulatingPhysics())
            {
                CurrentMetrics.SimulatingBodies++;
            }
        }
    }
    
    // Estimate physics step time (simplified)
    CurrentMetrics.PhysicsStepTime = CurrentMetrics.SimulatingBodies * 0.01f; // Rough estimate
    
    // Estimate collision pairs
    CurrentMetrics.CollisionPairs = CurrentMetrics.ActivePhysicsBodies * 2; // Simplified estimate
    
    // Estimate memory usage
    CurrentMetrics.MemoryUsageMB = (CurrentMetrics.ActivePhysicsBodies * 0.1f) + (CurrentMetrics.SimulatingBodies * 0.5f);
}

void UPerf_PhysicsPerformanceIntegrator::UpdateFPSHistory(float CurrentFPS)
{
    FPSHistory.Add(CurrentFPS);
    FrameTimeHistory.Add(1000.0f / FMath::Max(CurrentFPS, 1.0f));
    
    // Maintain history size
    if (FPSHistory.Num() > MaxHistorySize)
    {
        FPSHistory.RemoveAt(0);
    }
    
    if (FrameTimeHistory.Num() > MaxHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
}

float UPerf_PhysicsPerformanceIntegrator::GetAverageFPS() const
{
    if (FPSHistory.Num() == 0)
    {
        return 60.0f;
    }
    
    float Sum = 0.0f;
    for (float FPS : FPSHistory)
    {
        Sum += FPS;
    }
    
    return Sum / FPSHistory.Num();
}

void UPerf_PhysicsPerformanceIntegrator::LogOptimizationAction(const FString& Action, const FString& Details)
{
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: %s - %s"), *Action, *Details);
    }
}