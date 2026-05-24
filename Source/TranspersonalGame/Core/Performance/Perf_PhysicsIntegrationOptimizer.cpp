#include "Perf_PhysicsIntegrationOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMeshActor.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsIntegrationOptimizer, Log, All);

UPerf_PhysicsIntegrationOptimizer::UPerf_PhysicsIntegrationOptimizer()
    : bIsMonitoring(false)
    , bAdaptiveQualityEnabled(true)
    , CurrentPhysicsQualityLevel(2) // Medium quality by default
{
    // Initialize default LOD configuration
    PhysicsLODConfig.HighDetailDistance = 1000.0f;
    PhysicsLODConfig.MediumDetailDistance = 2500.0f;
    PhysicsLODConfig.LowDetailDistance = 5000.0f;
    PhysicsLODConfig.bEnablePhysicsLOD = true;
    PhysicsLODConfig.PhysicsUpdateRate = 60.0f;
}

void UPerf_PhysicsIntegrationOptimizer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Physics Integration Optimizer initialized"));
    
    // Start performance monitoring by default
    StartPhysicsPerformanceMonitoring();
    
    // Register console commands
    if (IConsoleManager::Get().FindConsoleCommand(TEXT("perf.physics.startmonitoring")) == nullptr)
    {
        IConsoleManager::Get().RegisterConsoleCommand(
            TEXT("perf.physics.startmonitoring"),
            TEXT("Start physics performance monitoring"),
            FConsoleCommandDelegate::CreateUObject(this, &UPerf_PhysicsIntegrationOptimizer::StartPhysicsPerformanceMonitoring)
        );
    }
    
    if (IConsoleManager::Get().FindConsoleCommand(TEXT("perf.physics.stopmonitoring")) == nullptr)
    {
        IConsoleManager::Get().RegisterConsoleCommand(
            TEXT("perf.physics.stopmonitoring"),
            TEXT("Stop physics performance monitoring"),
            FConsoleCommandDelegate::CreateUObject(this, &UPerf_PhysicsIntegrationOptimizer::StopPhysicsPerformanceMonitoring)
        );
    }
}

void UPerf_PhysicsIntegrationOptimizer::Deinitialize()
{
    StopPhysicsPerformanceMonitoring();
    
    // Unregister console commands
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("perf.physics.startmonitoring"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("perf.physics.stopmonitoring"));
    
    Super::Deinitialize();
}

bool UPerf_PhysicsIntegrationOptimizer::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UPerf_PhysicsIntegrationOptimizer::StartPhysicsPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = true;
    
    UWorld* World = GetWorld();
    if (World && World->GetTimerManager().IsValidHandle(MonitoringTimerHandle) == false)
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerf_PhysicsIntegrationOptimizer::UpdatePhysicsMetrics,
            0.1f, // Update every 100ms
            true
        );
        
        UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Physics performance monitoring started"));
    }
}

void UPerf_PhysicsIntegrationOptimizer::StopPhysicsPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = false;
    
    UWorld* World = GetWorld();
    if (World && World->GetTimerManager().IsValidHandle(MonitoringTimerHandle))
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
        UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Physics performance monitoring stopped"));
    }
}

FPerf_PhysicsIntegrationMetrics UPerf_PhysicsIntegrationOptimizer::GetCurrentPhysicsMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PhysicsIntegrationOptimizer::UpdatePhysicsLOD(const FVector& ViewerLocation)
{
    if (!PhysicsLODConfig.bEnablePhysicsLOD)
    {
        return;
    }
    
    // Update LOD for all registered physics actors
    for (auto ActorPtr : RegisteredPhysicsActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            int32 LODLevel = CalculateLODLevel(Actor->GetActorLocation(), ViewerLocation);
            UpdatePhysicsActorLOD(Actor, LODLevel);
        }
    }
    
    // Clean up invalid actor references
    RegisteredPhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr) {
        return !ActorPtr.IsValid();
    });
}

void UPerf_PhysicsIntegrationOptimizer::SetPhysicsLODConfig(const FPerf_PhysicsLODConfig& NewConfig)
{
    PhysicsLODConfig = NewConfig;
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Physics LOD configuration updated"));
}

void UPerf_PhysicsIntegrationOptimizer::OptimizePhysicsForFrameRate(float TargetFrameRate)
{
    float TargetFrameTime = 1000.0f / TargetFrameRate; // Convert to milliseconds
    
    if (CurrentMetrics.PhysicsFrameTime > TargetFrameTime * 1.2f) // 20% tolerance
    {
        // Reduce physics quality
        if (CurrentPhysicsQualityLevel > 0)
        {
            SetPhysicsQualityLevel(CurrentPhysicsQualityLevel - 1);
            UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Reduced physics quality to level %d for target framerate %.1f"), 
                CurrentPhysicsQualityLevel, TargetFrameRate);
        }
    }
    else if (CurrentMetrics.PhysicsFrameTime < TargetFrameTime * 0.8f) // 20% under target
    {
        // Increase physics quality if we have headroom
        if (CurrentPhysicsQualityLevel < 3)
        {
            SetPhysicsQualityLevel(CurrentPhysicsQualityLevel + 1);
            UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Increased physics quality to level %d"), 
                CurrentPhysicsQualityLevel);
        }
    }
}

void UPerf_PhysicsIntegrationOptimizer::EnableAdaptivePhysicsQuality(bool bEnable)
{
    bAdaptiveQualityEnabled = bEnable;
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Adaptive physics quality %s"), 
        bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_PhysicsIntegrationOptimizer::SetPhysicsQualityLevel(int32 QualityLevel)
{
    CurrentPhysicsQualityLevel = FMath::Clamp(QualityLevel, 0, 3);
    
    // Apply quality settings based on level
    switch (CurrentPhysicsQualityLevel)
    {
        case 0: // Low quality
            PhysicsLODConfig.PhysicsUpdateRate = 30.0f;
            PhysicsLODConfig.HighDetailDistance = 500.0f;
            PhysicsLODConfig.MediumDetailDistance = 1000.0f;
            break;
            
        case 1: // Medium-Low quality
            PhysicsLODConfig.PhysicsUpdateRate = 45.0f;
            PhysicsLODConfig.HighDetailDistance = 750.0f;
            PhysicsLODConfig.MediumDetailDistance = 1500.0f;
            break;
            
        case 2: // Medium quality (default)
            PhysicsLODConfig.PhysicsUpdateRate = 60.0f;
            PhysicsLODConfig.HighDetailDistance = 1000.0f;
            PhysicsLODConfig.MediumDetailDistance = 2500.0f;
            break;
            
        case 3: // High quality
            PhysicsLODConfig.PhysicsUpdateRate = 90.0f;
            PhysicsLODConfig.HighDetailDistance = 1500.0f;
            PhysicsLODConfig.MediumDetailDistance = 3500.0f;
            break;
    }
    
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Physics quality level set to %d"), CurrentPhysicsQualityLevel);
}

void UPerf_PhysicsIntegrationOptimizer::RegisterPhysicsActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    // Check if actor has physics components
    bool bHasPhysics = false;
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component && Component->IsSimulatingPhysics())
        {
            bHasPhysics = true;
            break;
        }
    }
    
    if (bHasPhysics)
    {
        RegisteredPhysicsActors.AddUnique(Actor);
        UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Registered physics actor: %s"), *Actor->GetName());
    }
}

void UPerf_PhysicsIntegrationOptimizer::UnregisterPhysicsActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    RegisteredPhysicsActors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& ActorPtr) {
        return ActorPtr.Get() == Actor;
    });
    
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Unregistered physics actor: %s"), *Actor->GetName());
}

void UPerf_PhysicsIntegrationOptimizer::UpdatePhysicsActorLOD(AActor* Actor, int32 LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    ApplyPhysicsLOD(Actor, LODLevel);
}

void UPerf_PhysicsIntegrationOptimizer::RunPhysicsPerformanceTest()
{
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Running physics performance test..."));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn test physics actors
    TArray<AActor*> TestActors;
    FVector SpawnLocation(0.0f, 0.0f, 1000.0f);
    
    for (int32 i = 0; i < 20; i++)
    {
        FVector Location = SpawnLocation + FVector(i * 100.0f, 0.0f, 0.0f);
        AActor* TestActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
        
        if (TestActor)
        {
            TestActors.Add(TestActor);
            RegisterPhysicsActor(TestActor);
        }
    }
    
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Spawned %d test physics actors"), TestActors.Num());
    
    // Run performance analysis
    StartPhysicsPerformanceMonitoring();
    
    // Schedule cleanup
    FTimerHandle CleanupTimer;
    World->GetTimerManager().SetTimer(CleanupTimer, [this, TestActors]() {
        for (AActor* Actor : TestActors)
        {
            if (Actor)
            {
                UnregisterPhysicsActor(Actor);
                Actor->Destroy();
            }
        }
        UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Physics performance test completed and cleaned up"));
    }, 10.0f, false);
}

void UPerf_PhysicsIntegrationOptimizer::GeneratePhysicsPerformanceReport()
{
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("=== PHYSICS PERFORMANCE REPORT ==="));
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Physics Frame Time: %.2f ms"), CurrentMetrics.PhysicsFrameTime);
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Active Physics Actors: %d"), CurrentMetrics.ActivePhysicsActors);
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Simulating Bodies: %d"), CurrentMetrics.SimulatingBodies);
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Collision Detection Time: %.2f ms"), CurrentMetrics.CollisionDetectionTime);
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Rigid Body Update Time: %.2f ms"), CurrentMetrics.RigidBodyUpdateTime);
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Physics LOD Level: %d"), CurrentMetrics.PhysicsLODLevel);
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Current Quality Level: %d"), CurrentPhysicsQualityLevel);
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Registered Physics Actors: %d"), RegisteredPhysicsActors.Num());
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("Adaptive Quality Enabled: %s"), bAdaptiveQualityEnabled ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogPhysicsIntegrationOptimizer, Log, TEXT("=== END REPORT ==="));
}

void UPerf_PhysicsIntegrationOptimizer::UpdatePhysicsMetrics()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Update basic metrics
    CurrentMetrics.ActivePhysicsActors = RegisteredPhysicsActors.Num();
    
    // Count simulating bodies
    int32 SimulatingBodies = 0;
    for (auto ActorPtr : RegisteredPhysicsActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component && Component->IsSimulatingPhysics())
                {
                    SimulatingBodies++;
                }
            }
        }
    }
    CurrentMetrics.SimulatingBodies = SimulatingBodies;
    
    // Estimate frame times (simplified)
    CurrentMetrics.PhysicsFrameTime = FMath::RandRange(8.0f, 20.0f); // Placeholder
    CurrentMetrics.CollisionDetectionTime = CurrentMetrics.PhysicsFrameTime * 0.4f;
    CurrentMetrics.RigidBodyUpdateTime = CurrentMetrics.PhysicsFrameTime * 0.6f;
    CurrentMetrics.PhysicsLODLevel = CurrentPhysicsQualityLevel;
    
    // Apply adaptive quality if enabled
    if (bAdaptiveQualityEnabled)
    {
        AdjustPhysicsQuality();
    }
}

void UPerf_PhysicsIntegrationOptimizer::ApplyPhysicsLOD(AActor* Actor, int32 LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (!Component)
        {
            continue;
        }
        
        switch (LODLevel)
        {
            case 0: // High detail
                // Full physics simulation
                break;
                
            case 1: // Medium detail
                // Reduced physics update rate
                break;
                
            case 2: // Low detail
                // Minimal physics
                break;
                
            case 3: // Very low detail
                // Physics disabled or kinematic only
                break;
        }
    }
}

int32 UPerf_PhysicsIntegrationOptimizer::CalculateLODLevel(const FVector& ActorLocation, const FVector& ViewerLocation) const
{
    float Distance = FVector::Dist(ActorLocation, ViewerLocation);
    
    if (Distance <= PhysicsLODConfig.HighDetailDistance)
    {
        return 0; // High detail
    }
    else if (Distance <= PhysicsLODConfig.MediumDetailDistance)
    {
        return 1; // Medium detail
    }
    else if (Distance <= PhysicsLODConfig.LowDetailDistance)
    {
        return 2; // Low detail
    }
    else
    {
        return 3; // Very low detail
    }
}

void UPerf_PhysicsIntegrationOptimizer::AdjustPhysicsQuality()
{
    // Adjust quality based on current performance
    if (CurrentMetrics.PhysicsFrameTime > HIGH_PHYSICS_FRAME_TIME_THRESHOLD)
    {
        if (CurrentPhysicsQualityLevel > 0)
        {
            SetPhysicsQualityLevel(CurrentPhysicsQualityLevel - 1);
        }
    }
    else if (CurrentMetrics.PhysicsFrameTime < HIGH_PHYSICS_FRAME_TIME_THRESHOLD * 0.7f)
    {
        if (CurrentPhysicsQualityLevel < 3 && CurrentMetrics.ActivePhysicsActors < MAX_PHYSICS_ACTORS_HIGH_QUALITY)
        {
            SetPhysicsQualityLevel(CurrentPhysicsQualityLevel + 1);
        }
    }
}