#include "Perf_CoreSystemsIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

APerf_CoreSystemsIntegrator::APerf_CoreSystemsIntegrator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    MetricsUpdateInterval = 1.0f;
    bEnableAutomaticOptimization = true;
    TargetFrameTime = 16.67f; // 60 FPS target
    CriticalFrameTime = 33.33f; // 30 FPS critical threshold
    
    LastMetricsUpdate = 0.0f;
    AccumulatedFrameTime = 0.0f;
    FrameTimesamples = 0;
    
    // Set default optimization settings
    OptimizationSettings = FPerf_CoreOptimizationSettings();
}

void APerf_CoreSystemsIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Core Systems Integrator: Initializing performance monitoring"));
    
    // Initialize metrics
    UpdateCoreSystemMetrics();
    
    // Apply initial optimization settings
    ApplyOptimizationSettings(OptimizationSettings);
}

void APerf_CoreSystemsIntegrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Accumulate frame time data
    AccumulatedFrameTime += DeltaTime * 1000.0f; // Convert to milliseconds
    FrameTimesamples++;
    
    // Update metrics at specified interval
    LastMetricsUpdate += DeltaTime;
    if (LastMetricsUpdate >= MetricsUpdateInterval)
    {
        UpdateCoreSystemMetrics();
        LastMetricsUpdate = 0.0f;
        
        // Check if automatic optimization is needed
        if (bEnableAutomaticOptimization)
        {
            float avgFrameTime = AccumulatedFrameTime / FMath::Max(FrameTimesamples, 1);
            if (avgFrameTime > CriticalFrameTime)
            {
                UE_LOG(LogTemp, Warning, TEXT("Critical frame time detected: %.2f ms - applying optimizations"), avgFrameTime);
                OptimizePhysicsSystem();
                OptimizeVehicleSystem();
                OptimizeDestructionSystem();
                OnPerformanceThresholdExceeded(avgFrameTime);
            }
            else if (avgFrameTime > TargetFrameTime)
            {
                UE_LOG(LogTemp, Log, TEXT("Target frame time exceeded: %.2f ms - applying light optimizations"), avgFrameTime);
                OptimizePhysicsSystem();
            }
        }
        
        // Reset frame time accumulation
        AccumulatedFrameTime = 0.0f;
        FrameTimesamples = 0;
    }
}

void APerf_CoreSystemsIntegrator::UpdateCoreSystemMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Calculate average frame time
    if (FrameTimesamples > 0)
    {
        CurrentMetrics.PhysicsFrameTime = AccumulatedFrameTime / FrameTimesamples;
    }
    
    // Count active physics actors
    CurrentMetrics.ActivePhysicsActors = 0;
    CurrentMetrics.VehicleSystemsActive = 0;
    
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }
        
        // Check for physics simulation
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                CurrentMetrics.ActivePhysicsActors++;
                break;
            }
        }
        
        // Check for vehicle systems
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Vehicle")) || ActorName.Contains(TEXT("Car")) || ActorName.Contains(TEXT("Bike")))
        {
            CurrentMetrics.VehicleSystemsActive++;
        }
    }
    
    // Estimate memory usage (simplified calculation)
    CurrentMetrics.RigidBodyMemoryUsage = CurrentMetrics.ActivePhysicsActors * 2.5f; // Rough estimate in MB
    
    // Destruction system load (placeholder - would need actual destruction system integration)
    CurrentMetrics.DestructionSystemLoad = FMath::Clamp(CurrentMetrics.ActivePhysicsActors / 100.0f, 0.0f, 1.0f);
    
    // Fluid simulation load (placeholder)
    CurrentMetrics.FluidSimulationLoad = OptimizationSettings.FluidSimulationQuality * 0.5f;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core Systems Metrics - Physics Actors: %d, Frame Time: %.2f ms"), 
           CurrentMetrics.ActivePhysicsActors, CurrentMetrics.PhysicsFrameTime);
}

void APerf_CoreSystemsIntegrator::OptimizePhysicsSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Optimizing Physics System"));
    
    // Reduce physics tick rate if performance is poor
    if (CurrentMetrics.PhysicsFrameTime > TargetFrameTime)
    {
        OptimizationSettings.PhysicsTickRate = FMath::Max(30.0f, OptimizationSettings.PhysicsTickRate * 0.9f);
        OptimizationSettings.MaxSimulatingBodies = FMath::Max(100, OptimizationSettings.MaxSimulatingBodies - 50);
        
        UE_LOG(LogTemp, Log, TEXT("Physics optimization applied - Tick Rate: %.1f, Max Bodies: %d"), 
               OptimizationSettings.PhysicsTickRate, OptimizationSettings.MaxSimulatingBodies);
        
        OnOptimizationApplied(TEXT("Physics"), OptimizationSettings.PhysicsSystemPriority);
    }
}

void APerf_CoreSystemsIntegrator::OptimizeVehicleSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Optimizing Vehicle System"));
    
    if (CurrentMetrics.VehicleSystemsActive > OptimizationSettings.MaxActiveVehicles)
    {
        OptimizationSettings.MaxActiveVehicles = FMath::Max(5, OptimizationSettings.MaxActiveVehicles - 2);
        OptimizationSettings.VehicleSystemPriority = EPerf_CoreSystemPriority::Low;
        
        UE_LOG(LogTemp, Log, TEXT("Vehicle optimization applied - Max Vehicles: %d"), OptimizationSettings.MaxActiveVehicles);
        
        OnOptimizationApplied(TEXT("Vehicle"), OptimizationSettings.VehicleSystemPriority);
    }
}

void APerf_CoreSystemsIntegrator::OptimizeDestructionSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Optimizing Destruction System"));
    
    if (CurrentMetrics.DestructionSystemLoad > 0.8f)
    {
        OptimizationSettings.MaxDestructibleActors = FMath::Max(25, OptimizationSettings.MaxDestructibleActors - 25);
        OptimizationSettings.DestructionSystemPriority = EPerf_CoreSystemPriority::Low;
        
        UE_LOG(LogTemp, Log, TEXT("Destruction optimization applied - Max Destructible: %d"), OptimizationSettings.MaxDestructibleActors);
        
        OnOptimizationApplied(TEXT("Destruction"), OptimizationSettings.DestructionSystemPriority);
    }
}

void APerf_CoreSystemsIntegrator::OptimizeFluidSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Optimizing Fluid System"));
    
    if (CurrentMetrics.FluidSimulationLoad > 0.7f)
    {
        OptimizationSettings.FluidSimulationQuality = FMath::Max(0.3f, OptimizationSettings.FluidSimulationQuality - 0.2f);
        OptimizationSettings.FluidSystemPriority = EPerf_CoreSystemPriority::Low;
        
        UE_LOG(LogTemp, Log, TEXT("Fluid optimization applied - Quality: %.2f"), OptimizationSettings.FluidSimulationQuality);
        
        OnOptimizationApplied(TEXT("Fluid"), OptimizationSettings.FluidSystemPriority);
    }
}

void APerf_CoreSystemsIntegrator::ApplyOptimizationSettings(const FPerf_CoreOptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    
    UE_LOG(LogTemp, Log, TEXT("Applied new optimization settings - Physics Priority: %d, Vehicle Priority: %d"), 
           (int32)OptimizationSettings.PhysicsSystemPriority, (int32)OptimizationSettings.VehicleSystemPriority);
    
    // Apply physics settings
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        // Note: These would need to be applied through proper physics subsystem in a real implementation
        UE_LOG(LogTemp, Log, TEXT("Physics settings would be applied here in full implementation"));
    }
}

FPerf_CoreSystemMetrics APerf_CoreSystemsIntegrator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerf_CoreSystemsIntegrator::SetSystemPriority(EPerf_CoreSystemPriority NewPriority, const FString& SystemName)
{
    if (SystemName == TEXT("Physics"))
    {
        OptimizationSettings.PhysicsSystemPriority = NewPriority;
    }
    else if (SystemName == TEXT("Vehicle"))
    {
        OptimizationSettings.VehicleSystemPriority = NewPriority;
    }
    else if (SystemName == TEXT("Destruction"))
    {
        OptimizationSettings.DestructionSystemPriority = NewPriority;
    }
    else if (SystemName == TEXT("Fluid"))
    {
        OptimizationSettings.FluidSystemPriority = NewPriority;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set %s system priority to %d"), *SystemName, (int32)NewPriority);
    OnOptimizationApplied(SystemName, NewPriority);
}

bool APerf_CoreSystemsIntegrator::IsPerformanceWithinTargets() const
{
    return CurrentMetrics.PhysicsFrameTime <= TargetFrameTime;
}

float APerf_CoreSystemsIntegrator::GetOverallPerformanceScore() const
{
    float frameTimeScore = FMath::Clamp(TargetFrameTime / FMath::Max(CurrentMetrics.PhysicsFrameTime, 1.0f), 0.0f, 1.0f);
    float memoryScore = FMath::Clamp(1.0f - (CurrentMetrics.RigidBodyMemoryUsage / 1000.0f), 0.0f, 1.0f);
    float systemLoadScore = FMath::Clamp(1.0f - CurrentMetrics.DestructionSystemLoad, 0.0f, 1.0f);
    
    return (frameTimeScore + memoryScore + systemLoadScore) / 3.0f;
}