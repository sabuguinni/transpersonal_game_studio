#include "Perf_PhysicsIntegrationOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"

UPerf_PhysicsIntegrationOptimizer::UPerf_PhysicsIntegrationOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for performance monitoring
    
    // Default optimization settings
    OptimizationSettings.OptimizationLevel = EPerf_PhysicsOptimizationLevel::High;
    OptimizationSettings.MaxPhysicsDistance = 5000.0f;
    OptimizationSettings.MaxSimulatingBodies = 100;
    OptimizationSettings.PhysicsTickRate = 60.0f;
    OptimizationSettings.bEnableAdaptivePhysics = true;
    OptimizationSettings.bOptimizeRagdolls = true;
    OptimizationSettings.bEnableCollisionCulling = true;
}

void UPerf_PhysicsIntegrationOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Integration Optimizer initialized"));
    
    // Initial optimization pass
    OptimizePhysicsPerformance();
    
    // Integrate with Core Physics systems
    IntegrateWithPhysicsSimulation();
    IntegrateWithCollisionManager();
    IntegrateWithDestructionSystem();
}

void UPerf_PhysicsIntegrationOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastPerformanceCheck += DeltaTime;
    
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        UpdatePerformanceMetrics();
        
        // Adaptive optimization based on performance
        if (OptimizationSettings.bEnableAdaptivePhysics)
        {
            AdaptPhysicsQuality();
        }
        
        LastPerformanceCheck = 0.0f;
    }
}

void UPerf_PhysicsIntegrationOptimizer::OptimizePhysicsPerformance()
{
    if (bIsOptimizing) return;
    
    bIsOptimizing = true;
    LastOptimizationTime = GetWorld()->GetTimeSeconds();
    OptimizationCycles++;
    
    UE_LOG(LogTemp, Log, TEXT("Starting physics performance optimization cycle %d"), OptimizationCycles);
    
    // Optimize different physics systems
    OptimizePhysicsActors();
    CullDistantPhysics();
    OptimizeDinosaurPhysics();
    OptimizeRagdollPerformance();
    OptimizeCollisionDetection();
    ManagePhysicsMemory();
    
    bIsOptimizing = false;
    
    UE_LOG(LogTemp, Log, TEXT("Physics optimization complete - Level: %s"), 
           *UEnum::GetValueAsString(OptimizationSettings.OptimizationLevel));
}

void UPerf_PhysicsIntegrationOptimizer::SetOptimizationLevel(EPerf_PhysicsOptimizationLevel NewLevel)
{
    OptimizationSettings.OptimizationLevel = NewLevel;
    
    // Adjust settings based on optimization level
    switch (NewLevel)
    {
        case EPerf_PhysicsOptimizationLevel::Ultra:
            OptimizationSettings.MaxSimulatingBodies = 200;
            OptimizationSettings.MaxPhysicsDistance = 10000.0f;
            OptimizationSettings.PhysicsTickRate = 120.0f;
            break;
            
        case EPerf_PhysicsOptimizationLevel::High:
            OptimizationSettings.MaxSimulatingBodies = 100;
            OptimizationSettings.MaxPhysicsDistance = 5000.0f;
            OptimizationSettings.PhysicsTickRate = 60.0f;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Medium:
            OptimizationSettings.MaxSimulatingBodies = 50;
            OptimizationSettings.MaxPhysicsDistance = 3000.0f;
            OptimizationSettings.PhysicsTickRate = 30.0f;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Low:
            OptimizationSettings.MaxSimulatingBodies = 25;
            OptimizationSettings.MaxPhysicsDistance = 1500.0f;
            OptimizationSettings.PhysicsTickRate = 20.0f;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Disabled:
            OptimizationSettings.MaxSimulatingBodies = 0;
            OptimizationSettings.MaxPhysicsDistance = 0.0f;
            OptimizationSettings.PhysicsTickRate = 10.0f;
            break;
    }
    
    // Apply new optimization settings
    OptimizePhysicsPerformance();
}

FPerf_PhysicsPerformanceMetrics UPerf_PhysicsIntegrationOptimizer::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PhysicsIntegrationOptimizer::OptimizeActorPhysics(AActor* Actor, float Distance)
{
    if (!Actor) return;
    
    UPrimitiveComponent* PrimComp = Actor->GetRootPrimitiveComponent();
    if (!PrimComp) return;
    
    // Distance-based optimization
    bool bShouldSimulate = Distance <= OptimizationSettings.MaxPhysicsDistance;
    
    if (OptimizationSettings.OptimizationLevel == EPerf_PhysicsOptimizationLevel::Disabled)
    {
        bShouldSimulate = false;
    }
    
    // Apply physics optimization
    if (PrimComp->IsSimulatingPhysics() != bShouldSimulate)
    {
        PrimComp->SetSimulatePhysics(bShouldSimulate);
    }
    
    // Collision optimization
    if (OptimizationSettings.bEnableCollisionCulling && Distance > OptimizationSettings.MaxPhysicsDistance * 0.5f)
    {
        PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    else
    {
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

void UPerf_PhysicsIntegrationOptimizer::OptimizeDinosaurPhysics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Find and optimize dinosaur actors
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        FString ActorName = Actor->GetName().ToLower();
        bool bIsDinosaur = ActorName.Contains(TEXT("trex")) || 
                          ActorName.Contains(TEXT("raptor")) || 
                          ActorName.Contains(TEXT("tricera")) || 
                          ActorName.Contains(TEXT("brachi"));
        
        if (bIsDinosaur)
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            OptimizeActorPhysics(Actor, Distance);
            
            // Special dinosaur optimizations
            if (Distance > OptimizationSettings.MaxPhysicsDistance * 2.0f)
            {
                // Disable tick for very distant dinosaurs
                Actor->SetActorTickEnabled(false);
            }
            else
            {
                Actor->SetActorTickEnabled(true);
            }
        }
    }
}

void UPerf_PhysicsIntegrationOptimizer::OptimizeRagdollPerformance()
{
    if (!OptimizationSettings.bOptimizeRagdolls) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find skeletal mesh actors with ragdolls
    for (TActorIterator<ASkeletalMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ASkeletalMeshActor* SkeletalActor = *ActorItr;
        if (!SkeletalActor) continue;
        
        USkeletalMeshComponent* SkeletalComp = SkeletalActor->GetSkeletalMeshComponent();
        if (!SkeletalComp) continue;
        
        // Optimize ragdoll based on distance and performance level
        if (OptimizationSettings.OptimizationLevel <= EPerf_PhysicsOptimizationLevel::Medium)
        {
            // Reduce ragdoll complexity for medium/low settings
            SkeletalComp->SetAllBodiesSimulatePhysics(false);
        }
    }
}

void UPerf_PhysicsIntegrationOptimizer::OptimizeCollisionDetection()
{
    if (!OptimizationSettings.bEnableCollisionCulling) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Optimize collision for static mesh actors
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AStaticMeshActor* StaticActor = *ActorItr;
        if (!StaticActor) continue;
        
        float Distance = FVector::Dist(PlayerLocation, StaticActor->GetActorLocation());
        
        UStaticMeshComponent* MeshComp = StaticActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            if (Distance > OptimizationSettings.MaxPhysicsDistance)
            {
                MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
            else
            {
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
    }
}

void UPerf_PhysicsIntegrationOptimizer::IntegrateWithPhysicsSimulation()
{
    // Integration with Core_PhysicsSimulation system
    UE_LOG(LogTemp, Log, TEXT("Integrating with Core Physics Simulation system"));
    
    // This would integrate with the Core_PhysicsSimulation class
    // when it's fully implemented and available
}

void UPerf_PhysicsIntegrationOptimizer::IntegrateWithCollisionManager()
{
    // Integration with Core_CollisionManager system
    UE_LOG(LogTemp, Log, TEXT("Integrating with Core Collision Manager system"));
    
    // This would integrate with the Core_CollisionManager class
    // when it's fully implemented and available
}

void UPerf_PhysicsIntegrationOptimizer::IntegrateWithDestructionSystem()
{
    // Integration with Core_DestructionSystem
    UE_LOG(LogTemp, Log, TEXT("Integrating with Core Destruction System"));
    
    // This would integrate with the Core_DestructionSystem class
    // when it's fully implemented and available
}

void UPerf_PhysicsIntegrationOptimizer::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    CurrentMetrics.ActivePhysicsActors = 0;
    CurrentMetrics.SimulatingBodies = 0;
    CurrentMetrics.RagdollBodies = 0;
    CurrentMetrics.CollisionChecks = 0;
    
    // Count physics actors
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        UPrimitiveComponent* PrimComp = Actor->GetRootPrimitiveComponent();
        if (PrimComp)
        {
            CurrentMetrics.ActivePhysicsActors++;
            
            if (PrimComp->IsSimulatingPhysics())
            {
                CurrentMetrics.SimulatingBodies++;
            }
        }
    }
    
    // Estimate frame time (simplified)
    CurrentMetrics.PhysicsFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to ms
    
    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = (CurrentMetrics.ActivePhysicsActors * 0.5f) + (CurrentMetrics.SimulatingBodies * 2.0f);
}

bool UPerf_PhysicsIntegrationOptimizer::IsPerformanceOptimal() const
{
    // Check if performance is within acceptable limits
    bool bFrameTimeGood = CurrentMetrics.PhysicsFrameTime < 16.67f; // 60 FPS
    bool bBodyCountGood = CurrentMetrics.SimulatingBodies <= OptimizationSettings.MaxSimulatingBodies;
    bool bMemoryGood = CurrentMetrics.MemoryUsageMB < 500.0f; // 500MB limit
    
    return bFrameTimeGood && bBodyCountGood && bMemoryGood;
}

void UPerf_PhysicsIntegrationOptimizer::LogPerformanceReport() const
{
    UE_LOG(LogTemp, Log, TEXT("=== Physics Performance Report ==="));
    UE_LOG(LogTemp, Log, TEXT("Active Physics Actors: %d"), CurrentMetrics.ActivePhysicsActors);
    UE_LOG(LogTemp, Log, TEXT("Simulating Bodies: %d"), CurrentMetrics.SimulatingBodies);
    UE_LOG(LogTemp, Log, TEXT("Physics Frame Time: %.2f ms"), CurrentMetrics.PhysicsFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Optimization Level: %s"), *UEnum::GetValueAsString(OptimizationSettings.OptimizationLevel));
    UE_LOG(LogTemp, Log, TEXT("Performance Optimal: %s"), IsPerformanceOptimal() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Optimization Cycles: %d"), OptimizationCycles);
}

void UPerf_PhysicsIntegrationOptimizer::OptimizePhysicsActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Optimize all physics actors based on distance
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        OptimizeActorPhysics(Actor, Distance);
    }
}

void UPerf_PhysicsIntegrationOptimizer::CullDistantPhysics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Disable physics for very distant actors
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        if (Distance > OptimizationSettings.MaxPhysicsDistance * 2.0f)
        {
            UPrimitiveComponent* PrimComp = Actor->GetRootPrimitiveComponent();
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                PrimComp->SetSimulatePhysics(false);
            }
        }
    }
}

void UPerf_PhysicsIntegrationOptimizer::AdaptPhysicsQuality()
{
    // Adaptive physics quality based on current performance
    if (!IsPerformanceOptimal())
    {
        // Reduce optimization level if performance is poor
        if (OptimizationSettings.OptimizationLevel > EPerf_PhysicsOptimizationLevel::Low)
        {
            EPerf_PhysicsOptimizationLevel NewLevel = static_cast<EPerf_PhysicsOptimizationLevel>(
                static_cast<uint8>(OptimizationSettings.OptimizationLevel) + 1);
            SetOptimizationLevel(NewLevel);
            
            UE_LOG(LogTemp, Warning, TEXT("Performance degraded - reducing physics quality to %s"), 
                   *UEnum::GetValueAsString(NewLevel));
        }
    }
    else if (CurrentMetrics.PhysicsFrameTime < 8.0f) // Very good performance
    {
        // Increase optimization level if performance is very good
        if (OptimizationSettings.OptimizationLevel > EPerf_PhysicsOptimizationLevel::Ultra)
        {
            EPerf_PhysicsOptimizationLevel NewLevel = static_cast<EPerf_PhysicsOptimizationLevel>(
                static_cast<uint8>(OptimizationSettings.OptimizationLevel) - 1);
            SetOptimizationLevel(NewLevel);
            
            UE_LOG(LogTemp, Log, TEXT("Performance excellent - increasing physics quality to %s"), 
                   *UEnum::GetValueAsString(NewLevel));
        }
    }
}

void UPerf_PhysicsIntegrationOptimizer::ManagePhysicsMemory()
{
    // Simple memory management for physics
    if (CurrentMetrics.MemoryUsageMB > 400.0f) // High memory usage
    {
        UE_LOG(LogTemp, Warning, TEXT("High physics memory usage: %.2f MB - optimizing"), CurrentMetrics.MemoryUsageMB);
        
        // Force garbage collection if memory is too high
        if (CurrentMetrics.MemoryUsageMB > 500.0f)
        {
            GEngine->ForceGarbageCollection(true);
        }
    }
}