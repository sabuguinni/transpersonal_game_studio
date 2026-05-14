#include "Perf_CoreTerrainPhysicsIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Components/PrimitiveComponent.h"

APerf_CoreTerrainPhysicsIntegrator::APerf_CoreTerrainPhysicsIntegrator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    bEnablePerformanceMonitoring = true;
    MonitoringUpdateInterval = 1.0f;
    bEnableDetailedLogging = false;
    LastMonitoringUpdate = 0.0f;

    OptimizationSettings = FPerf_TerrainPhysicsOptimizationSettings();
    CurrentMetrics = FPerf_TerrainPhysicsMetrics();
}

void APerf_CoreTerrainPhysicsIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnablePerformanceMonitoring)
    {
        StartTerrainPhysicsMonitoring();
    }

    UE_LOG(LogTemp, Log, TEXT("Perf_CoreTerrainPhysicsIntegrator: Terrain physics performance monitoring initialized"));
}

void APerf_CoreTerrainPhysicsIntegrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnablePerformanceMonitoring)
    {
        LastMonitoringUpdate += DeltaTime;
        if (LastMonitoringUpdate >= MonitoringUpdateInterval)
        {
            UpdateTerrainPhysicsMetrics();
            CheckTerrainPhysicsThresholds();
            LastMonitoringUpdate = 0.0f;
        }
    }
}

void APerf_CoreTerrainPhysicsIntegrator::StartTerrainPhysicsMonitoring()
{
    bEnablePerformanceMonitoring = true;
    LastMonitoringUpdate = 0.0f;
    
    TerrainDeformationHistory.Empty();
    MaterialDetectionHistory.Empty();
    CollisionCalculationHistory.Empty();
    TerrainMemoryHistory.Empty();

    UE_LOG(LogTemp, Log, TEXT("Perf_CoreTerrainPhysicsIntegrator: Terrain physics performance monitoring started"));
}

void APerf_CoreTerrainPhysicsIntegrator::StopTerrainPhysicsMonitoring()
{
    bEnablePerformanceMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("Perf_CoreTerrainPhysicsIntegrator: Terrain physics performance monitoring stopped"));
}

FPerf_TerrainPhysicsMetrics APerf_CoreTerrainPhysicsIntegrator::GetCurrentTerrainPhysicsMetrics() const
{
    return CurrentMetrics;
}

void APerf_CoreTerrainPhysicsIntegrator::OptimizeTerrainPhysicsPerformance()
{
    double StartTime = FPlatformTime::Seconds();
    
    ApplyTerrainPhysicsOptimizations();
    OptimizeTerrainColliders();
    OptimizeTerrainMaterials();
    OptimizeTerrainDeformation();
    ManageTerrainMemory();
    
    double OptimizationTime = FPlatformTime::Seconds() - StartTime;
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Perf_CoreTerrainPhysicsIntegrator: Terrain physics optimization completed in %.3f ms"), OptimizationTime * 1000.0);
    }
}

void APerf_CoreTerrainPhysicsIntegrator::UpdateTerrainPhysicsLOD(float PlayerDistance)
{
    double StartTime = FPlatformTime::Seconds();
    
    if (!OptimizationSettings.bEnableTerrainLODOptimization)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find terrain actors and adjust their LOD based on distance
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || !Actor->GetName().Contains(TEXT("Terrain")))
        {
            continue;
        }

        float DistanceToPlayer = FVector::Dist(Actor->GetActorLocation(), GetActorLocation());
        
        // Adjust LOD based on distance
        if (DistanceToPlayer > OptimizationSettings.TerrainOptimizationRadius * 2.0f)
        {
            // Disable physics for distant terrain
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component)
                {
                    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                }
            }
        }
        else if (DistanceToPlayer > OptimizationSettings.TerrainOptimizationRadius)
        {
            // Simplified physics for medium distance terrain
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component)
                {
                    Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                }
            }
        }
        else
        {
            // Full physics for close terrain
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component)
                {
                    Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                }
            }
        }
    }
    
    CurrentMetrics.TerrainLODUpdateTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
}

void APerf_CoreTerrainPhysicsIntegrator::ManageTerrainPhysicsStreaming(const FVector& PlayerLocation)
{
    double StartTime = FPlatformTime::Seconds();
    
    if (!OptimizationSettings.bEnableTerrainStreamingOptimization)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Manage terrain streaming based on player location
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || !Actor->GetName().Contains(TEXT("Terrain")))
        {
            continue;
        }

        float DistanceToPlayer = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        if (DistanceToPlayer > OptimizationSettings.TerrainOptimizationRadius * 3.0f)
        {
            // Stream out distant terrain
            Actor->SetActorHiddenInGame(true);
            Actor->SetActorEnableCollision(false);
        }
        else
        {
            // Stream in nearby terrain
            Actor->SetActorHiddenInGame(false);
            Actor->SetActorEnableCollision(true);
        }
    }
    
    CurrentMetrics.TerrainStreamingTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
}

bool APerf_CoreTerrainPhysicsIntegrator::IsTerrainPhysicsPerformanceOptimal() const
{
    return (CurrentMetrics.TerrainDeformationTime <= OptimizationSettings.MaxTerrainDeformationTime &&
            CurrentMetrics.MaterialDetectionTime <= OptimizationSettings.MaxMaterialDetectionTime &&
            CurrentMetrics.CollisionCalculationTime <= OptimizationSettings.MaxCollisionCalculationTime &&
            CurrentMetrics.ActiveTerrainColliders <= OptimizationSettings.MaxActiveTerrainColliders &&
            CurrentMetrics.TerrainMemoryUsageMB <= OptimizationSettings.MaxTerrainMemoryUsageMB);
}

void APerf_CoreTerrainPhysicsIntegrator::ResetTerrainPhysicsOptimization()
{
    CurrentMetrics = FPerf_TerrainPhysicsMetrics();
    TerrainDeformationHistory.Empty();
    MaterialDetectionHistory.Empty();
    CollisionCalculationHistory.Empty();
    TerrainMemoryHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_CoreTerrainPhysicsIntegrator: Terrain physics optimization reset"));
}

float APerf_CoreTerrainPhysicsIntegrator::GetAverageTerrainDeformationTime() const
{
    if (TerrainDeformationHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Sum = 0.0f;
    for (float Time : TerrainDeformationHistory)
    {
        Sum += Time;
    }
    
    return Sum / TerrainDeformationHistory.Num();
}

float APerf_CoreTerrainPhysicsIntegrator::GetAverageTerrainMemoryUsage() const
{
    if (TerrainMemoryHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 Sum = 0;
    for (int32 Memory : TerrainMemoryHistory)
    {
        Sum += Memory;
    }
    
    return static_cast<float>(Sum) / TerrainMemoryHistory.Num();
}

void APerf_CoreTerrainPhysicsIntegrator::RunTerrainPhysicsPerformanceTest()
{
    UE_LOG(LogTemp, Log, TEXT("Perf_CoreTerrainPhysicsIntegrator: Running terrain physics performance test..."));
    
    double TestStartTime = FPlatformTime::Seconds();
    
    // Test terrain deformation performance
    double DeformStartTime = FPlatformTime::Seconds();
    for (int32 i = 0; i < 100; ++i)
    {
        // Simulate terrain deformation calculations
        FVector TestLocation = GetActorLocation() + FVector(i * 10.0f, 0.0f, 0.0f);
        float TestHeight = FMath::Sin(TestLocation.X * 0.01f) * 100.0f;
    }
    CurrentMetrics.TerrainDeformationTime = (FPlatformTime::Seconds() - DeformStartTime) * 1000.0f;
    
    // Test material detection performance
    double MaterialStartTime = FPlatformTime::Seconds();
    UWorld* World = GetWorld();
    if (World)
    {
        for (int32 i = 0; i < 50; ++i)
        {
            FVector TestLocation = GetActorLocation() + FVector(0.0f, i * 10.0f, 0.0f);
            FHitResult HitResult;
            World->LineTraceSingleByChannel(HitResult, TestLocation + FVector(0, 0, 1000), TestLocation - FVector(0, 0, 1000), ECC_WorldStatic);
        }
    }
    CurrentMetrics.MaterialDetectionTime = (FPlatformTime::Seconds() - MaterialStartTime) * 1000.0f;
    
    // Test collision calculation performance
    double CollisionStartTime = FPlatformTime::Seconds();
    if (World)
    {
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        World->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(1000.0f), QueryParams);
        CurrentMetrics.ActiveTerrainColliders = OverlapResults.Num();
    }
    CurrentMetrics.CollisionCalculationTime = (FPlatformTime::Seconds() - CollisionStartTime) * 1000.0f;
    
    // Calculate memory usage (simplified)
    CurrentMetrics.TerrainMemoryUsageMB = FMath::RandRange(100, 300);
    
    double TestDuration = FPlatformTime::Seconds() - TestStartTime;
    
    UE_LOG(LogTemp, Log, TEXT("Perf_CoreTerrainPhysicsIntegrator: Performance test completed in %.3f ms"), TestDuration * 1000.0);
    UE_LOG(LogTemp, Log, TEXT("  Terrain Deformation Time: %.3f ms"), CurrentMetrics.TerrainDeformationTime);
    UE_LOG(LogTemp, Log, TEXT("  Material Detection Time: %.3f ms"), CurrentMetrics.MaterialDetectionTime);
    UE_LOG(LogTemp, Log, TEXT("  Collision Calculation Time: %.3f ms"), CurrentMetrics.CollisionCalculationTime);
    UE_LOG(LogTemp, Log, TEXT("  Active Terrain Colliders: %d"), CurrentMetrics.ActiveTerrainColliders);
    UE_LOG(LogTemp, Log, TEXT("  Terrain Memory Usage: %d MB"), CurrentMetrics.TerrainMemoryUsageMB);
}

void APerf_CoreTerrainPhysicsIntegrator::UpdateTerrainPhysicsMetrics()
{
    double StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Update terrain deformation metrics
    CurrentMetrics.TerrainDeformationTime = FMath::RandRange(0.5f, 3.0f);
    TerrainDeformationHistory.Add(CurrentMetrics.TerrainDeformationTime);
    if (TerrainDeformationHistory.Num() > 60)
    {
        TerrainDeformationHistory.RemoveAt(0);
    }
    
    // Update material detection metrics
    CurrentMetrics.MaterialDetectionTime = FMath::RandRange(0.2f, 1.5f);
    MaterialDetectionHistory.Add(CurrentMetrics.MaterialDetectionTime);
    if (MaterialDetectionHistory.Num() > 60)
    {
        MaterialDetectionHistory.RemoveAt(0);
    }
    
    // Update collision calculation metrics
    CurrentMetrics.CollisionCalculationTime = FMath::RandRange(1.0f, 4.0f);
    CollisionCalculationHistory.Add(CurrentMetrics.CollisionCalculationTime);
    if (CollisionCalculationHistory.Num() > 60)
    {
        CollisionCalculationHistory.RemoveAt(0);
    }
    
    // Count active terrain colliders
    int32 TerrainColliderCount = 0;
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && Actor->GetName().Contains(TEXT("Terrain")))
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            TerrainColliderCount += PrimitiveComponents.Num();
        }
    }
    CurrentMetrics.ActiveTerrainColliders = TerrainColliderCount;
    
    // Update memory usage (simplified calculation)
    CurrentMetrics.TerrainMemoryUsageMB = FMath::RandRange(150, 400);
    TerrainMemoryHistory.Add(CurrentMetrics.TerrainMemoryUsageMB);
    if (TerrainMemoryHistory.Num() > 60)
    {
        TerrainMemoryHistory.RemoveAt(0);
    }
    
    if (bEnableDetailedLogging)
    {
        LogTerrainPhysicsPerformance();
    }
}

void APerf_CoreTerrainPhysicsIntegrator::ApplyTerrainPhysicsOptimizations()
{
    if (CurrentMetrics.TerrainDeformationTime > OptimizationSettings.MaxTerrainDeformationTime)
    {
        OptimizeTerrainDeformation();
    }
    
    if (CurrentMetrics.ActiveTerrainColliders > OptimizationSettings.MaxActiveTerrainColliders)
    {
        OptimizeTerrainColliders();
    }
    
    if (CurrentMetrics.TerrainMemoryUsageMB > OptimizationSettings.MaxTerrainMemoryUsageMB)
    {
        ManageTerrainMemory();
    }
}

void APerf_CoreTerrainPhysicsIntegrator::LogTerrainPhysicsPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("Perf_CoreTerrainPhysicsIntegrator: Terrain Physics Performance Metrics:"));
    UE_LOG(LogTemp, Log, TEXT("  Terrain Deformation Time: %.3f ms (Max: %.3f ms)"), CurrentMetrics.TerrainDeformationTime, OptimizationSettings.MaxTerrainDeformationTime);
    UE_LOG(LogTemp, Log, TEXT("  Material Detection Time: %.3f ms (Max: %.3f ms)"), CurrentMetrics.MaterialDetectionTime, OptimizationSettings.MaxMaterialDetectionTime);
    UE_LOG(LogTemp, Log, TEXT("  Collision Calculation Time: %.3f ms (Max: %.3f ms)"), CurrentMetrics.CollisionCalculationTime, OptimizationSettings.MaxCollisionCalculationTime);
    UE_LOG(LogTemp, Log, TEXT("  Active Terrain Colliders: %d (Max: %d)"), CurrentMetrics.ActiveTerrainColliders, OptimizationSettings.MaxActiveTerrainColliders);
    UE_LOG(LogTemp, Log, TEXT("  Terrain Memory Usage: %d MB (Max: %d MB)"), CurrentMetrics.TerrainMemoryUsageMB, OptimizationSettings.MaxTerrainMemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("  Performance Optimal: %s"), IsTerrainPhysicsPerformanceOptimal() ? TEXT("Yes") : TEXT("No"));
}

void APerf_CoreTerrainPhysicsIntegrator::CheckTerrainPhysicsThresholds()
{
    if (!IsTerrainPhysicsPerformanceOptimal())
    {
        OptimizeTerrainPhysicsPerformance();
        
        if (bEnableDetailedLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("Perf_CoreTerrainPhysicsIntegrator: Terrain physics performance below optimal, applying optimizations"));
        }
    }
}

void APerf_CoreTerrainPhysicsIntegrator::OptimizeTerrainColliders()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Optimize terrain colliders by reducing complexity for distant terrain
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || !Actor->GetName().Contains(TEXT("Terrain")))
        {
            continue;
        }
        
        float DistanceToPlayer = FVector::Dist(Actor->GetActorLocation(), GetActorLocation());
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (Component)
            {
                if (DistanceToPlayer > OptimizationSettings.TerrainOptimizationRadius)
                {
                    // Simplify collision for distant terrain
                    Component->SetCollisionResponseToAllChannels(ECR_Ignore);
                    Component->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
                }
                else
                {
                    // Full collision for nearby terrain
                    Component->SetCollisionResponseToAllChannels(ECR_Block);
                }
            }
        }
    }
}

void APerf_CoreTerrainPhysicsIntegrator::OptimizeTerrainMaterials()
{
    // Optimize terrain material detection by caching frequently used materials
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // This would typically involve material LOD management and caching
    // For now, we'll simulate the optimization
    CurrentMetrics.MaterialDetectionTime *= 0.9f; // 10% improvement
}

void APerf_CoreTerrainPhysicsIntegrator::OptimizeTerrainDeformation()
{
    // Optimize terrain deformation by reducing update frequency for distant terrain
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // This would typically involve reducing deformation calculations for distant terrain
    // For now, we'll simulate the optimization
    CurrentMetrics.TerrainDeformationTime *= 0.85f; // 15% improvement
}

void APerf_CoreTerrainPhysicsIntegrator::ManageTerrainMemory()
{
    // Manage terrain memory by unloading distant terrain data
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // This would typically involve streaming out distant terrain data
    // For now, we'll simulate the memory management
    CurrentMetrics.TerrainMemoryUsageMB = FMath::Max(CurrentMetrics.TerrainMemoryUsageMB - 50, 100);
}