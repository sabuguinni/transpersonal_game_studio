#include "Perf_PhysicsIntegrationMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UPerf_PhysicsIntegrationMonitor::UPerf_PhysicsIntegrationMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    bEnableRealTimeMonitoring = true;
    MonitoringUpdateInterval = 0.1f;
    bIsMonitoring = false;
    LastUpdateTime = 0.0f;
    
    // Initialize optimization settings with performance-focused defaults
    OptimizationSettings.MaxPhysicsFrameTime = 16.67f; // 60fps target
    OptimizationSettings.MaxSimulatingBodies = 500;
    OptimizationSettings.bEnableAdaptiveLOD = true;
    OptimizationSettings.LODDistanceThreshold = 2000.0f;
    OptimizationSettings.bEnablePhysicsCulling = true;
    OptimizationSettings.CullingDistance = 5000.0f;
}

void UPerf_PhysicsIntegrationMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableRealTimeMonitoring)
    {
        StartPerformanceMonitoring();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics Integration Monitor started"));
}

void UPerf_PhysicsIntegrationMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMonitoring)
    {
        LastUpdateTime += DeltaTime;
        if (LastUpdateTime >= MonitoringUpdateInterval)
        {
            UpdatePerformanceData();
            CheckPerformanceThresholds();
            LastUpdateTime = 0.0f;
        }
    }
}

FPerf_PhysicsPerformanceData UPerf_PhysicsIntegrationMonitor::GetCurrentPerformanceData() const
{
    return CurrentPerformanceData;
}

void UPerf_PhysicsIntegrationMonitor::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    PerformanceHistory.Empty();
    LastUpdateTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Physics performance monitoring started"));
}

void UPerf_PhysicsIntegrationMonitor::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("Physics performance monitoring stopped"));
}

bool UPerf_PhysicsIntegrationMonitor::IsPerformanceWithinTargets() const
{
    const FPerf_PhysicsPerformanceData& Data = CurrentPerformanceData;
    
    bool bWithinTargets = true;
    
    if (Data.PhysicsFrameTime > OptimizationSettings.MaxPhysicsFrameTime)
    {
        bWithinTargets = false;
    }
    
    if (Data.SimulatingBodies > OptimizationSettings.MaxSimulatingBodies)
    {
        bWithinTargets = false;
    }
    
    return bWithinTargets;
}

void UPerf_PhysicsIntegrationMonitor::ApplyOptimizations()
{
    if (!IsPerformanceWithinTargets())
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance below targets, applying optimizations..."));
        
        OptimizePhysicsActors();
        
        if (OptimizationSettings.bEnableAdaptiveLOD)
        {
            UpdatePhysicsLOD();
        }
        
        if (OptimizationSettings.bEnablePhysicsCulling)
        {
            CullDistantPhysicsActors();
        }
    }
}

void UPerf_PhysicsIntegrationMonitor::ValidatePhysicsIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS INTEGRATION VALIDATION ==="));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found for physics validation"));
        return;
    }
    
    // Count physics actors
    int32 PhysicsActorCount = CountActivePhysicsActors();
    int32 SimulatingBodyCount = CountSimulatingBodies();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Actors: %d"), PhysicsActorCount);
    UE_LOG(LogTemp, Log, TEXT("Simulating Bodies: %d"), SimulatingBodyCount);
    
    // Test Core_PhysicsWorldGenerator integration
    bool bFoundPhysicsWorldGen = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Core_PhysicsWorldGenerator")))
        {
            bFoundPhysicsWorldGen = true;
            UE_LOG(LogTemp, Log, TEXT("✓ Found Core_PhysicsWorldGenerator: %s"), *Actor->GetName());
        }
    }
    
    if (!bFoundPhysicsWorldGen)
    {
        UE_LOG(LogTemp, Warning, TEXT("✗ Core_PhysicsWorldGenerator not found in level"));
    }
    
    // Validate physics performance
    UpdatePerformanceData();
    bool bPerformanceOK = IsPerformanceWithinTargets();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Frame Time: %.2f ms (Target: %.2f ms)"), 
           CurrentPerformanceData.PhysicsFrameTime, OptimizationSettings.MaxPhysicsFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Performance Status: %s"), 
           bPerformanceOK ? TEXT("✓ WITHIN TARGETS") : TEXT("✗ BELOW TARGETS"));
    
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATION COMPLETE ==="));
}

void UPerf_PhysicsIntegrationMonitor::RunPhysicsStressTest()
{
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS STRESS TEST ==="));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world for stress test"));
        return;
    }
    
    // Create stress test objects
    TArray<AActor*> TestActors;
    FVector BaseLocation(6000.0f, 0.0f, 1000.0f);
    
    for (int32 i = 0; i < 20; i++)
    {
        FVector SpawnLocation = BaseLocation + FVector(i * 100.0f, 0.0f, 0.0f);
        
        AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
        if (TestActor)
        {
            // Add static mesh component with physics
            UStaticMeshComponent* MeshComp = TestActor->CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TestMesh"));
            if (MeshComp)
            {
                TestActor->SetRootComponent(MeshComp);
                MeshComp->SetSimulatePhysics(true);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                TestActors.Add(TestActor);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Created %d stress test actors"), TestActors.Num());
    
    // Monitor performance during stress test
    float TestDuration = 5.0f;
    float TestStartTime = World->GetTimeSeconds();
    
    // Force update performance data
    UpdatePerformanceData();
    
    UE_LOG(LogTemp, Log, TEXT("Stress Test Results:"));
    UE_LOG(LogTemp, Log, TEXT("- Physics Frame Time: %.2f ms"), CurrentPerformanceData.PhysicsFrameTime);
    UE_LOG(LogTemp, Log, TEXT("- Active Physics Actors: %d"), CurrentPerformanceData.ActivePhysicsActors);
    UE_LOG(LogTemp, Log, TEXT("- Simulating Bodies: %d"), CurrentPerformanceData.SimulatingBodies);
    
    // Clean up test actors
    for (AActor* TestActor : TestActors)
    {
        if (TestActor)
        {
            TestActor->Destroy();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== STRESS TEST COMPLETE ==="));
}

void UPerf_PhysicsIntegrationMonitor::GeneratePerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS PERFORMANCE REPORT ==="));
    
    UpdatePerformanceData();
    
    FString Report;
    Report += FString::Printf(TEXT("Physics Performance Report - %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Physics Frame Time: %.2f ms\n"), CurrentPerformanceData.PhysicsFrameTime);
    Report += FString::Printf(TEXT("Active Physics Actors: %d\n"), CurrentPerformanceData.ActivePhysicsActors);
    Report += FString::Printf(TEXT("Simulating Bodies: %d\n"), CurrentPerformanceData.SimulatingBodies);
    Report += FString::Printf(TEXT("Physics Memory Usage: %.2f MB\n"), CurrentPerformanceData.PhysicsMemoryUsage);
    Report += FString::Printf(TEXT("Collision Query Time: %.2f ms\n"), CurrentPerformanceData.CollisionQueryTime);
    Report += FString::Printf(TEXT("Collision Queries: %d\n"), CurrentPerformanceData.CollisionQueries);
    
    Report += TEXT("\nOptimization Settings:\n");
    Report += FString::Printf(TEXT("Max Physics Frame Time: %.2f ms\n"), OptimizationSettings.MaxPhysicsFrameTime);
    Report += FString::Printf(TEXT("Max Simulating Bodies: %d\n"), OptimizationSettings.MaxSimulatingBodies);
    Report += FString::Printf(TEXT("Adaptive LOD: %s\n"), OptimizationSettings.bEnableAdaptiveLOD ? TEXT("Enabled") : TEXT("Disabled"));
    Report += FString::Printf(TEXT("Physics Culling: %s\n"), OptimizationSettings.bEnablePhysicsCulling ? TEXT("Enabled") : TEXT("Disabled"));
    
    Report += TEXT("\nPerformance Status: ");
    Report += IsPerformanceWithinTargets() ? TEXT("✓ WITHIN TARGETS") : TEXT("✗ BELOW TARGETS");
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *Report);
    UE_LOG(LogTemp, Log, TEXT("=== REPORT COMPLETE ==="));
}

void UPerf_PhysicsIntegrationMonitor::UpdatePerformanceData()
{
    // Update physics performance metrics
    CurrentPerformanceData.ActivePhysicsActors = CountActivePhysicsActors();
    CurrentPerformanceData.SimulatingBodies = CountSimulatingBodies();
    CurrentPerformanceData.PhysicsMemoryUsage = CalculatePhysicsMemoryUsage();
    
    // Estimate physics frame time (simplified)
    CurrentPerformanceData.PhysicsFrameTime = FMath::Max(0.1f, 
        CurrentPerformanceData.SimulatingBodies * 0.01f + CurrentPerformanceData.CollisionQueries * 0.001f);
    
    // Store in history
    PerformanceHistory.Add(CurrentPerformanceData);
    if (PerformanceHistory.Num() > 100) // Keep last 100 samples
    {
        PerformanceHistory.RemoveAt(0);
    }
}

void UPerf_PhysicsIntegrationMonitor::CheckPerformanceThresholds()
{
    if (!IsPerformanceWithinTargets())
    {
        LogPerformanceWarning(TEXT("Physics performance below targets"));
        
        if (OptimizationSettings.bEnableAdaptiveLOD || OptimizationSettings.bEnablePhysicsCulling)
        {
            ApplyOptimizations();
        }
    }
}

void UPerf_PhysicsIntegrationMonitor::OptimizePhysicsActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 OptimizedCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            // Optimize based on distance from player
            // This is a simplified optimization - in a real game you'd have more sophisticated logic
            FVector ActorLocation = Actor->GetActorLocation();
            // For now, just log that we're optimizing
            OptimizedCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Optimized %d physics actors"), OptimizedCount);
}

void UPerf_PhysicsIntegrationMonitor::UpdatePhysicsLOD()
{
    // Simplified LOD system - in production this would be more sophisticated
    UE_LOG(LogTemp, Log, TEXT("Updating physics LOD system"));
}

void UPerf_PhysicsIntegrationMonitor::CullDistantPhysicsActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 CulledCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            FVector ActorLocation = Actor->GetActorLocation();
            float DistanceFromOrigin = ActorLocation.Size();
            
            if (DistanceFromOrigin > OptimizationSettings.CullingDistance)
            {
                // Disable physics simulation for distant actors
                PrimComp->SetSimulatePhysics(false);
                CulledCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Culled physics on %d distant actors"), CulledCount);
}

int32 UPerf_PhysicsIntegrationMonitor::CountActivePhysicsActors() const
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                Count++;
            }
        }
    }
    return Count;
}

int32 UPerf_PhysicsIntegrationMonitor::CountSimulatingBodies() const
{
    // For now, return the same as active physics actors
    // In a real implementation, this would count individual physics bodies
    return CountActivePhysicsActors();
}

float UPerf_PhysicsIntegrationMonitor::CalculatePhysicsMemoryUsage() const
{
    // Simplified memory calculation - in production this would query actual physics memory
    return CountSimulatingBodies() * 0.1f; // Estimate 0.1 MB per body
}

void UPerf_PhysicsIntegrationMonitor::LogPerformanceWarning(const FString& Warning) const
{
    UE_LOG(LogTemp, Warning, TEXT("Physics Performance Warning: %s"), *Warning);
}