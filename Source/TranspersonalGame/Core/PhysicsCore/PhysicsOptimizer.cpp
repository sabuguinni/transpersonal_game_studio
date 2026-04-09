// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "PhysicsEngine/PhysicsSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsOptimizer, Log, All);

void UPhysicsOptimizer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Physics Optimizer initialized"));
    
    InitializeDefaultLODSettings();
}

void UPhysicsOptimizer::Deinitialize()
{
    // Clear all timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(OptimizationUpdateTimer);
        World->GetTimerManager().ClearTimer(PerformanceMonitorTimer);
    }
    
    RegisteredActors.Empty();
    
    Super::Deinitialize();
}

void UPhysicsOptimizer::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    // Start optimization and performance monitoring timers
    InWorld.GetTimerManager().SetTimer(
        OptimizationUpdateTimer,
        this,
        &UPhysicsOptimizer::ForceOptimizationUpdate,
        OptimizationUpdateFrequency,
        true
    );
    
    InWorld.GetTimerManager().SetTimer(
        PerformanceMonitorTimer,
        this,
        &UPhysicsOptimizer::UpdatePerformanceMetrics,
        0.1f, // Update metrics every 100ms
        true
    );
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Physics Optimizer started for world: %s"), *InWorld.GetName());
}

UPhysicsOptimizer* UPhysicsOptimizer::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UPhysicsOptimizer>();
    }
    return nullptr;
}

void UPhysicsOptimizer::SetOptimizationLevel(EPhysicsOptimizationLevel Level)
{
    if (CurrentOptimizationLevel != Level)
    {
        CurrentOptimizationLevel = Level;
        UE_LOG(LogPhysicsOptimizer, Log, TEXT("Optimization level changed to: %d"), (int32)Level);
        
        // Immediately apply new optimization level
        ForceOptimizationUpdate();
    }
}

void UPhysicsOptimizer::ConfigurePhysicsLOD(const TArray<FPhysicsLODSettings>& LODSettings)
{
    PhysicsLODSettings = LODSettings;
    
    // Sort LOD settings by distance
    PhysicsLODSettings.Sort([](const FPhysicsLODSettings& A, const FPhysicsLODSettings& B)
    {
        return A.Distance < B.Distance;
    });
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Physics LOD configured with %d levels"), PhysicsLODSettings.Num());
    
    // Apply new settings immediately
    ForceOptimizationUpdate();
}

void UPhysicsOptimizer::RegisterPhysicsActor(AActor* Actor, int32 Priority)
{
    if (!IsValid(Actor))
    {
        UE_LOG(LogPhysicsOptimizer, Warning, TEXT("Attempted to register invalid actor"));
        return;
    }
    
    RegisteredActors.Add(Actor, Priority);
    UE_LOG(LogPhysicsOptimizer, VeryVerbose, TEXT("Registered physics actor: %s with priority %d"), 
        *Actor->GetName(), Priority);
}

void UPhysicsOptimizer::UnregisterPhysicsActor(AActor* Actor)
{
    if (RegisteredActors.Remove(Actor) > 0)
    {
        UE_LOG(LogPhysicsOptimizer, VeryVerbose, TEXT("Unregistered physics actor: %s"), 
            IsValid(Actor) ? *Actor->GetName() : TEXT("Invalid"));
    }
}

void UPhysicsOptimizer::UpdateOptimization(FVector ViewerLocation)
{
    CurrentViewerLocation = ViewerLocation;
    
    if (!bAutoOptimizationEnabled)
    {
        return;
    }
    
    // Clean up invalid actors
    TArray<AActor*> ActorsToRemove;
    for (auto& ActorPair : RegisteredActors)
    {
        if (!IsValid(ActorPair.Key))
        {
            ActorsToRemove.Add(ActorPair.Key);
        }
    }
    
    for (AActor* InvalidActor : ActorsToRemove)
    {
        RegisteredActors.Remove(InvalidActor);
    }
    
    // Apply optimization to all registered actors
    for (auto& ActorPair : RegisteredActors)
    {
        AActor* Actor = ActorPair.Key;
        if (IsValid(Actor))
        {
            int32 LODLevel = CalculateLODLevel(Actor, ViewerLocation);
            if (LODLevel >= 0 && LODLevel < PhysicsLODSettings.Num())
            {
                ApplyOptimizationToActor(Actor, PhysicsLODSettings[LODLevel]);
            }
        }
    }
    
    // Auto-adjust optimization if performance is poor
    if (bAutoOptimizationEnabled)
    {
        AutoAdjustOptimization();
    }
}

void UPhysicsOptimizer::GetPerformanceMetrics(float& FrameTime, int32& ActiveBodies, float& PhysicsTime) const
{
    FrameTime = LastFrameTime;
    ActiveBodies = LastActiveBodies;
    PhysicsTime = LastPhysicsTime;
}

void UPhysicsOptimizer::SetAutoOptimizationEnabled(bool bEnabled)
{
    bAutoOptimizationEnabled = bEnabled;
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Auto optimization %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPhysicsOptimizer::ForceOptimizationUpdate()
{
    if (RegisteredActors.Num() > 0)
    {
        UpdateOptimization(CurrentViewerLocation);
    }
}

void UPhysicsOptimizer::InitializeDefaultLODSettings()
{
    PhysicsLODSettings.Empty();
    
    // LOD 0: High detail (0-500 units)
    FPhysicsLODSettings LOD0;
    LOD0.Distance = 500.0f;
    LOD0.SimulationFrequency = 1.0f;
    LOD0.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    LOD0.bUseSimplifiedCollision = false;
    LOD0.MaxPhysicsBodies = 100;
    PhysicsLODSettings.Add(LOD0);
    
    // LOD 1: Medium detail (500-1500 units)
    FPhysicsLODSettings LOD1;
    LOD1.Distance = 1500.0f;
    LOD1.SimulationFrequency = 0.75f;
    LOD1.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    LOD1.bUseSimplifiedCollision = true;
    LOD1.MaxPhysicsBodies = 200;
    PhysicsLODSettings.Add(LOD1);
    
    // LOD 2: Low detail (1500-5000 units)
    FPhysicsLODSettings LOD2;
    LOD2.Distance = 5000.0f;
    LOD2.SimulationFrequency = 0.5f;
    LOD2.CollisionEnabled = ECollisionEnabled::QueryOnly;
    LOD2.bUseSimplifiedCollision = true;
    LOD2.MaxPhysicsBodies = 500;
    PhysicsLODSettings.Add(LOD2);
    
    // LOD 3: Minimal detail (5000+ units)
    FPhysicsLODSettings LOD3;
    LOD3.Distance = 15000.0f;
    LOD3.SimulationFrequency = 0.25f;
    LOD3.CollisionEnabled = ECollisionEnabled::NoCollision;
    LOD3.bUseSimplifiedCollision = true;
    LOD3.MaxPhysicsBodies = 1000;
    PhysicsLODSettings.Add(LOD3);
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Initialized default LOD settings with %d levels"), PhysicsLODSettings.Num());
}

void UPhysicsOptimizer::ApplyOptimizationToActor(AActor* Actor, const FPhysicsLODSettings& LODSettings)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    ApplyLODToPhysicsComponents(Actor, LODSettings);
}

int32 UPhysicsOptimizer::CalculateLODLevel(AActor* Actor, FVector ViewerLocation) const
{
    if (!IsValid(Actor))
    {
        return -1;
    }
    
    float Distance = GetDistanceToViewer(Actor);
    
    // Find appropriate LOD level based on distance
    for (int32 i = 0; i < PhysicsLODSettings.Num(); ++i)
    {
        if (Distance <= PhysicsLODSettings[i].Distance)
        {
            return i;
        }
    }
    
    // Return highest LOD level if beyond all distances
    return PhysicsLODSettings.Num() - 1;
}

void UPhysicsOptimizer::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get frame time
    LastFrameTime = World->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    
    // Count active physics bodies
    LastActiveBodies = 0;
    for (auto& ActorPair : RegisteredActors)
    {
        if (AActor* Actor = ActorPair.Key)
        {
            if (IsValid(Actor))
            {
                TArray<UPrimitiveComponent*> PrimitiveComponents;
                Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
                
                for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
                {
                    if (PrimComp && PrimComp->IsSimulatingPhysics())
                    {
                        LastActiveBodies++;
                    }
                }
            }
        }
    }
    
    // Estimate physics time (simplified calculation)
    LastPhysicsTime = LastFrameTime * 0.3f; // Assume physics takes ~30% of frame time
}

void UPhysicsOptimizer::AutoAdjustOptimization()
{
    // Check if we need to adjust optimization level
    bool bPerformancePoor = LastFrameTime > TargetFrameTime || LastPhysicsTime > MaxPhysicsTime;
    bool bPerformanceGood = LastFrameTime < TargetFrameTime * 0.8f && LastPhysicsTime < MaxPhysicsTime * 0.5f;
    
    if (bPerformancePoor && CurrentOptimizationLevel < EPhysicsOptimizationLevel::Aggressive)
    {
        // Increase optimization level
        EPhysicsOptimizationLevel NewLevel = static_cast<EPhysicsOptimizationLevel>(
            static_cast<int32>(CurrentOptimizationLevel) + 1);
        SetOptimizationLevel(NewLevel);
        
        UE_LOG(LogPhysicsOptimizer, Warning, TEXT("Performance poor (Frame: %.2fms, Physics: %.2fms), increased optimization to level %d"), 
            LastFrameTime, LastPhysicsTime, (int32)NewLevel);
    }
    else if (bPerformanceGood && CurrentOptimizationLevel > EPhysicsOptimizationLevel::Disabled)
    {
        // Decrease optimization level for better quality
        EPhysicsOptimizationLevel NewLevel = static_cast<EPhysicsOptimizationLevel>(
            static_cast<int32>(CurrentOptimizationLevel) - 1);
        SetOptimizationLevel(NewLevel);
        
        UE_LOG(LogPhysicsOptimizer, Log, TEXT("Performance good (Frame: %.2fms, Physics: %.2fms), decreased optimization to level %d"), 
            LastFrameTime, LastPhysicsTime, (int32)NewLevel);
    }
}

float UPhysicsOptimizer::GetDistanceToViewer(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return FLT_MAX;
    }
    
    return FVector::Dist(Actor->GetActorLocation(), CurrentViewerLocation);
}

void UPhysicsOptimizer::ApplyLODToPhysicsComponents(AActor* Actor, const FPhysicsLODSettings& LODSettings)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    // Get all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (!PrimComp)
        {
            continue;
        }
        
        // Apply collision settings
        PrimComp->SetCollisionEnabled(LODSettings.CollisionEnabled);
        
        // Apply simplified collision if needed
        if (LODSettings.bUseSimplifiedCollision)
        {
            PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
        }
        
        // Adjust physics simulation frequency (simplified approach)
        if (PrimComp->IsSimulatingPhysics())
        {
            // For simulation frequency, we could disable/enable physics periodically
            // This is a simplified implementation
            if (LODSettings.SimulationFrequency < 1.0f)
            {
                // Reduce physics update rate by temporarily disabling physics
                bool bShouldSimulate = FMath::RandRange(0.0f, 1.0f) < LODSettings.SimulationFrequency;
                if (!bShouldSimulate)
                {
                    PrimComp->SetSimulatePhysics(false);
                    
                    // Re-enable physics next frame
                    GetWorld()->GetTimerManager().SetTimerForNextTick([PrimComp]()
                    {
                        if (IsValid(PrimComp))
                        {
                            PrimComp->SetSimulatePhysics(true);
                        }
                    });
                }
            }
        }
    }
}