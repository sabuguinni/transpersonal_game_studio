#include "Perf_PhysicsPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "HAL/PlatformMemory.h"
#include "../Core_PhysicsWorldManager.h"

APerf_PhysicsPerformanceIntegrator::APerf_PhysicsPerformanceIntegrator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms
    
    PerformanceUpdateInterval = 1.0f;
    bEnablePerformanceOptimization = true;
    bLogPerformanceMetrics = false;
    LastPerformanceUpdate = 0.0f;
    PhysicsWorldManager = nullptr;
    
    // Initialize optimization settings
    OptimizationSettings = FPerf_PhysicsOptimizationSettings();
    CurrentMetrics = FPerf_PhysicsPerformanceMetrics();
}

void APerf_PhysicsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the Core_PhysicsWorldManager in the world
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<ACore_PhysicsWorldManager> ActorItr(World); ActorItr; ++ActorItr)
        {
            PhysicsWorldManager = *ActorItr;
            break;
        }
    }
    
    if (PhysicsWorldManager)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsPerformanceIntegrator: Connected to PhysicsWorldManager"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsPerformanceIntegrator: No PhysicsWorldManager found"));
    }
    
    // Initialize performance tracking
    PhysicsFrameTimeHistory.Reserve(60); // Store 1 second of history at 60fps
}

void APerf_PhysicsPerformanceIntegrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bEnablePerformanceOptimization)
    {
        return;
    }
    
    LastPerformanceUpdate += DeltaTime;
    
    if (LastPerformanceUpdate >= PerformanceUpdateInterval)
    {
        UpdatePhysicsPerformanceMetrics();
        OptimizePhysicsPerformance();
        LastPerformanceUpdate = 0.0f;
    }
}

void APerf_PhysicsPerformanceIntegrator::UpdatePhysicsPerformanceMetrics()
{
    CalculatePhysicsFrameTime();
    CountActivePhysicsBodies();
    AnalyzePhysicsMemoryUsage();
    
    if (bLogPerformanceMetrics)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics Performance - Frame Time: %.2fms, Active Bodies: %d, Memory: %.2fMB"),
            CurrentMetrics.PhysicsFrameTime,
            CurrentMetrics.ActivePhysicsBodies,
            CurrentMetrics.PhysicsMemoryUsage);
    }
}

void APerf_PhysicsPerformanceIntegrator::OptimizePhysicsPerformance()
{
    if (!IsPhysicsPerformanceAcceptable())
    {
        ApplyPhysicsOptimizations();
    }
    
    if (OptimizationSettings.bEnablePhysicsLOD)
    {
        UpdatePhysicsLODSystem();
    }
    
    if (OptimizationSettings.bEnablePhysicsCulling)
    {
        CullDistantPhysicsActors();
    }
}

void APerf_PhysicsPerformanceIntegrator::SetPhysicsLODLevel(AActor* Actor, int32 LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        return;
    }
    
    switch (LODLevel)
    {
        case 0: // High quality - full physics
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case 1: // Medium quality - simplified physics
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
            
        case 2: // Low quality - no physics
            MeshComp->SetSimulatePhysics(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
            
        default:
            break;
    }
}

void APerf_PhysicsPerformanceIntegrator::RegisterPhysicsActor(AActor* Actor)
{
    if (Actor && !TrackedPhysicsActors.Contains(Actor))
    {
        TrackedPhysicsActors.Add(Actor);
    }
}

void APerf_PhysicsPerformanceIntegrator::UnregisterPhysicsActor(AActor* Actor)
{
    TrackedPhysicsActors.Remove(Actor);
}

FPerf_PhysicsPerformanceMetrics APerf_PhysicsPerformanceIntegrator::GetCurrentPhysicsMetrics() const
{
    return CurrentMetrics;
}

void APerf_PhysicsPerformanceIntegrator::SetOptimizationSettings(const FPerf_PhysicsOptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
}

bool APerf_PhysicsPerformanceIntegrator::IsPhysicsPerformanceAcceptable() const
{
    return CurrentMetrics.PhysicsFrameTime <= OptimizationSettings.MaxPhysicsFrameTime &&
           CurrentMetrics.ActivePhysicsBodies <= OptimizationSettings.MaxActivePhysicsBodies;
}

void APerf_PhysicsPerformanceIntegrator::EnablePhysicsOptimization(bool bEnable)
{
    bEnablePerformanceOptimization = bEnable;
}

void APerf_PhysicsPerformanceIntegrator::CullDistantPhysicsActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    for (AActor* Actor : TrackedPhysicsActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        if (Distance > OptimizationSettings.PhysicsLODDistance3)
        {
            SetPhysicsSimulationEnabled(Actor, false);
        }
        else
        {
            SetPhysicsSimulationEnabled(Actor, true);
            AdjustPhysicsComplexity(Actor, Distance);
        }
    }
}

void APerf_PhysicsPerformanceIntegrator::UpdatePhysicsLODSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    for (AActor* Actor : TrackedPhysicsActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        float Distance = GetDistanceToPlayer(Actor);
        int32 LODLevel = 0;
        
        if (Distance > OptimizationSettings.PhysicsLODDistance3)
        {
            LODLevel = 2; // No physics
        }
        else if (Distance > OptimizationSettings.PhysicsLODDistance2)
        {
            LODLevel = 2; // No physics
        }
        else if (Distance > OptimizationSettings.PhysicsLODDistance1)
        {
            LODLevel = 1; // Simplified physics
        }
        else
        {
            LODLevel = 0; // Full physics
        }
        
        SetPhysicsLODLevel(Actor, LODLevel);
    }
}

int32 APerf_PhysicsPerformanceIntegrator::GetOptimalPhysicsBodyCount() const
{
    float FrameTimeRatio = CurrentMetrics.PhysicsFrameTime / OptimizationSettings.MaxPhysicsFrameTime;
    
    if (FrameTimeRatio > 1.0f)
    {
        // Reduce physics bodies if frame time is too high
        return FMath::Max(100, CurrentMetrics.ActivePhysicsBodies - 50);
    }
    else if (FrameTimeRatio < 0.5f)
    {
        // Can increase physics bodies if frame time is low
        return FMath::Min(OptimizationSettings.MaxActivePhysicsBodies, CurrentMetrics.ActivePhysicsBodies + 25);
    }
    
    return CurrentMetrics.ActivePhysicsBodies;
}

void APerf_PhysicsPerformanceIntegrator::CalculatePhysicsFrameTime()
{
    // Estimate physics frame time based on current frame time
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    PhysicsFrameTimeHistory.Add(CurrentFrameTime);
    
    if (PhysicsFrameTimeHistory.Num() > 60)
    {
        PhysicsFrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average physics frame time
    float TotalTime = 0.0f;
    for (float FrameTime : PhysicsFrameTimeHistory)
    {
        TotalTime += FrameTime;
    }
    
    CurrentMetrics.PhysicsFrameTime = PhysicsFrameTimeHistory.Num() > 0 ? TotalTime / PhysicsFrameTimeHistory.Num() : 0.0f;
    CurrentMetrics.AveragePhysicsStepTime = CurrentMetrics.PhysicsFrameTime * 0.3f; // Estimate physics portion
}

void APerf_PhysicsPerformanceIntegrator::CountActivePhysicsBodies()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 ActiveBodies = 0;
    int32 CollisionChecks = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            ActiveBodies++;
            
            if (PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                CollisionChecks++;
            }
        }
    }
    
    CurrentMetrics.ActivePhysicsBodies = ActiveBodies;
    CurrentMetrics.CollisionChecksPerFrame = CollisionChecks;
}

void APerf_PhysicsPerformanceIntegrator::AnalyzePhysicsMemoryUsage()
{
    // Estimate physics memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.PhysicsMemoryUsage = MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

void APerf_PhysicsPerformanceIntegrator::ApplyPhysicsOptimizations()
{
    if (CurrentMetrics.PhysicsFrameTime > OptimizationSettings.MaxPhysicsFrameTime)
    {
        // Reduce physics quality for distant objects
        for (AActor* Actor : TrackedPhysicsActors)
        {
            if (Actor)
            {
                float Distance = GetDistanceToPlayer(Actor);
                if (Distance > OptimizationSettings.PhysicsLODDistance1)
                {
                    SetPhysicsLODLevel(Actor, 1); // Simplified physics
                }
            }
        }
    }
    
    if (CurrentMetrics.ActivePhysicsBodies > OptimizationSettings.MaxActivePhysicsBodies)
    {
        // Disable physics for the most distant objects
        CullDistantPhysicsActors();
    }
}

float APerf_PhysicsPerformanceIntegrator::GetDistanceToPlayer(AActor* Actor) const
{
    UWorld* World = GetWorld();
    if (!World || !Actor)
    {
        return 0.0f;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return 0.0f;
    }
    
    return FVector::Dist(PC->GetPawn()->GetActorLocation(), Actor->GetActorLocation());
}

void APerf_PhysicsPerformanceIntegrator::SetPhysicsSimulationEnabled(AActor* Actor, bool bEnabled)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimComp)
    {
        PrimComp->SetSimulatePhysics(bEnabled);
    }
}

void APerf_PhysicsPerformanceIntegrator::AdjustPhysicsComplexity(AActor* Actor, float Distance)
{
    if (!Actor)
    {
        return;
    }
    
    int32 LODLevel = 0;
    
    if (Distance > OptimizationSettings.PhysicsLODDistance2)
    {
        LODLevel = 2; // No physics
    }
    else if (Distance > OptimizationSettings.PhysicsLODDistance1)
    {
        LODLevel = 1; // Simplified physics
    }
    else
    {
        LODLevel = 0; // Full physics
    }
    
    SetPhysicsLODLevel(Actor, LODLevel);
}