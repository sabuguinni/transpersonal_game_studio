#include "Core_PhysicsOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/IConsoleManager.h"

UCore_PhysicsOptimizer::UCore_PhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick 10 times per second for optimization
    
    // Initialize LOD settings with reasonable defaults
    LODSettings.NearDistance = 500.0f;
    LODSettings.MidDistance = 1500.0f;
    LODSettings.FarDistance = 3000.0f;
    LODSettings.MaxNearPhysicsActors = 50;
    LODSettings.MaxMidPhysicsActors = 25;
    LODSettings.MaxFarPhysicsActors = 10;
    
    // Initialize performance tracking
    FrameTimeHistory.Reserve(MaxFrameHistory);
}

void UCore_PhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize performance monitoring
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsOptimizer: Initialized with LOD distances Near=%f, Mid=%f, Far=%f"), 
           LODSettings.NearDistance, LODSettings.MidDistance, LODSettings.FarDistance);
}

void UCore_PhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics every tick
    UpdatePerformanceMetrics();
    
    // Track frame time for adaptive quality
    if (FrameTimeHistory.Num() >= MaxFrameHistory)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    FrameTimeHistory.Add(DeltaTime * 1000.0f); // Convert to milliseconds
    
    // Run optimization at intervals
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastOptimizationTime >= OptimizationInterval)
    {
        if (bEnablePhysicsLOD)
        {
            OptimizePhysicsLOD();
        }
        
        if (bEnableAdaptiveQuality)
        {
            AdjustPhysicsQuality(TargetFrameTime);
        }
        
        LastOptimizationTime = CurrentTime;
    }
}

void UCore_PhysicsOptimizer::OptimizePhysicsLOD()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get player location for distance calculations
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Clean up invalid actor references
    TrackedPhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor) {
        return !WeakActor.IsValid();
    });
    
    // Apply LOD to all tracked physics actors
    for (const TWeakObjectPtr<AActor>& WeakActor : TrackedPhysicsActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            ApplyLODToActor(Actor, Distance);
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsOptimizer: Optimized LOD for %d physics actors"), TrackedPhysicsActors.Num());
}

void UCore_PhysicsOptimizer::RegisterPhysicsActor(AActor* Actor)
{
    if (Actor && Actor->GetRootComponent() && Actor->GetRootComponent()->IsSimulatingPhysics())
    {
        TrackedPhysicsActors.AddUnique(Actor);
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsOptimizer: Registered physics actor %s"), *Actor->GetName());
    }
}

void UCore_PhysicsOptimizer::UnregisterPhysicsActor(AActor* Actor)
{
    if (Actor)
    {
        TrackedPhysicsActors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& WeakActor) {
            return WeakActor.Get() == Actor;
        });
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsOptimizer: Unregistered physics actor %s"), *Actor->GetName());
    }
}

float UCore_PhysicsOptimizer::GetAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Total = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Total += FrameTime;
    }
    
    return Total / FrameTimeHistory.Num();
}

void UCore_PhysicsOptimizer::AdjustPhysicsQuality(float TargetFrameTime)
{
    float AverageFrameTime = GetAverageFrameTime();
    
    if (AverageFrameTime > TargetFrameTime * 1.2f) // 20% over target
    {
        // Reduce physics quality
        PhysicsSubstepScale = FMath::Max(0.5f, PhysicsSubstepScale - 0.1f);
        OptimizePhysicsSettings();
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsOptimizer: Reducing physics quality - FrameTime: %fms, Target: %fms"), 
               AverageFrameTime, TargetFrameTime);
    }
    else if (AverageFrameTime < TargetFrameTime * 0.8f) // 20% under target
    {
        // Increase physics quality
        PhysicsSubstepScale = FMath::Min(1.0f, PhysicsSubstepScale + 0.05f);
        OptimizePhysicsSettings();
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsOptimizer: Increasing physics quality - FrameTime: %fms, Target: %fms"), 
               AverageFrameTime, TargetFrameTime);
    }
}

void UCore_PhysicsOptimizer::SetPhysicsSubstepScale(float NewScale)
{
    PhysicsSubstepScale = FMath::Clamp(NewScale, 0.1f, 2.0f);
    OptimizePhysicsSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsOptimizer: Physics substep scale set to %f"), PhysicsSubstepScale);
}

void UCore_PhysicsOptimizer::DebugPhysicsPerformance()
{
    if (GEngine)
    {
        FString DebugString = FString::Printf(
            TEXT("Physics Performance:\nFrame Time: %.2fms\nActive Bodies: %d\nSleeping Bodies: %d\nCollision Checks: %d\nLOD Scale: %.2f"),
            CurrentMetrics.AveragePhysicsStepTime,
            CurrentMetrics.ActiveRigidBodies,
            CurrentMetrics.SleepingRigidBodies,
            CurrentMetrics.CollisionChecks,
            PhysicsSubstepScale
        );
        
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DebugString);
    }
}

void UCore_PhysicsOptimizer::LogPhysicsStats()
{
    UE_LOG(LogTemp, Log, TEXT("=== Core Physics Optimizer Stats ==="));
    UE_LOG(LogTemp, Log, TEXT("Physics Frame Time: %.2fms"), CurrentMetrics.PhysicsFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Average Frame Time: %.2fms"), GetAverageFrameTime());
    UE_LOG(LogTemp, Log, TEXT("Active Rigid Bodies: %d"), CurrentMetrics.ActiveRigidBodies);
    UE_LOG(LogTemp, Log, TEXT("Sleeping Rigid Bodies: %d"), CurrentMetrics.SleepingRigidBodies);
    UE_LOG(LogTemp, Log, TEXT("Collision Checks: %d"), CurrentMetrics.CollisionChecks);
    UE_LOG(LogTemp, Log, TEXT("Tracked Physics Actors: %d"), TrackedPhysicsActors.Num());
    UE_LOG(LogTemp, Log, TEXT("Physics Substep Scale: %.2f"), PhysicsSubstepScale);
    UE_LOG(LogTemp, Log, TEXT("====================================="));
}

void UCore_PhysicsOptimizer::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get physics statistics from the world
    UWorld* World = GetWorld();
    if (World->GetPhysicsScene())
    {
        // Update basic metrics (simplified for now)
        CurrentMetrics.PhysicsFrameTime = GetAverageFrameTime();
        CurrentMetrics.AveragePhysicsStepTime = CurrentMetrics.PhysicsFrameTime;
        
        // Count physics actors in the world
        int32 ActiveBodies = 0;
        int32 SleepingBodies = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    if (PrimComp->IsAnyRigidBodyAwake())
                    {
                        ActiveBodies++;
                    }
                    else
                    {
                        SleepingBodies++;
                    }
                }
            }
        }
        
        CurrentMetrics.ActiveRigidBodies = ActiveBodies;
        CurrentMetrics.SleepingRigidBodies = SleepingBodies;
        CurrentMetrics.CollisionChecks = ActiveBodies * 2; // Rough estimate
    }
}

void UCore_PhysicsOptimizer::ApplyLODToActor(AActor* Actor, float Distance)
{
    if (!Actor)
    {
        return;
    }
    
    ECore_PhysicsLODLevel LODLevel = CalculateLODLevel(Distance);
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    
    if (!PrimComp)
    {
        return;
    }
    
    // Apply LOD settings based on distance
    switch (LODLevel)
    {
        case ECore_PhysicsLODLevel::Near:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetSimulatePhysics(true);
            break;
            
        case ECore_PhysicsLODLevel::Mid:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetSimulatePhysics(true);
            // Reduce physics update frequency
            break;
            
        case ECore_PhysicsLODLevel::Far:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            PrimComp->SetSimulatePhysics(false);
            break;
            
        case ECore_PhysicsLODLevel::Disabled:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            PrimComp->SetSimulatePhysics(false);
            break;
    }
}

void UCore_PhysicsOptimizer::OptimizePhysicsSettings()
{
    // Apply global physics optimization settings
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        // Adjust physics solver iterations based on performance
        // This would require engine modifications or console commands
        
        if (GetWorld())
        {
            // Use console commands to adjust physics settings
            FString SubstepCommand = FString::Printf(TEXT("p.Chaos.Solver.FixedDeltaTime %f"), 
                                                   0.0166f * PhysicsSubstepScale);
            GetWorld()->Exec(GetWorld(), *SubstepCommand);
        }
    }
}

ECore_PhysicsLODLevel UCore_PhysicsOptimizer::CalculateLODLevel(float Distance) const
{
    if (Distance <= LODSettings.NearDistance)
    {
        return ECore_PhysicsLODLevel::Near;
    }
    else if (Distance <= LODSettings.MidDistance)
    {
        return ECore_PhysicsLODLevel::Mid;
    }
    else if (Distance <= LODSettings.FarDistance)
    {
        return ECore_PhysicsLODLevel::Far;
    }
    else
    {
        return ECore_PhysicsLODLevel::Disabled;
    }
}