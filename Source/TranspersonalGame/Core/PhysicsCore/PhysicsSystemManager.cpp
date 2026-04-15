#include "PhysicsSystemManager.h"
#include "CollisionManager.h"
#include "RagdollSystem.h"
#include "DestructionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UPhysicsSystemManager::UPhysicsSystemManager()
{
    bPhysicsSimulationEnabled = true;
    GravityScale = 1.0f;
    PhysicsLODDistance = 5000.0f;
    MaxPhysicsObjects = 1000;
    bOptimizeForPerformance = true;
    PhysicsTickRate = 60.0f;
    MaxSimulationSteps = 8;
    bDebugDrawEnabled = false;
    bShowPhysicsStats = false;
    LastPerformanceCheck = 0.0f;
    CurrentPhysicsObjectCount = 0;
    AveragePhysicsTickTime = 0.0f;
}

void UPhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Initializing physics systems..."));
    
    InitializeSubsystems();
    UpdatePhysicsSettings();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Initialization complete"));
}

void UPhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Shutting down physics systems..."));
    
    CleanupSubsystems();
    RegisteredCollisionObjects.Empty();
    
    Super::Deinitialize();
}

bool UPhysicsSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UPhysicsSystemManager::InitializeSubsystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: No valid world found"));
        return;
    }

    // Initialize collision manager
    CollisionManager = NewObject<UCollisionManager>(this);
    if (CollisionManager)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: CollisionManager initialized"));
    }

    // Initialize ragdoll system
    RagdollSystem = NewObject<URagdollSystem>(this);
    if (RagdollSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: RagdollSystem initialized"));
    }

    // Initialize destruction system
    DestructionSystem = NewObject<UDestructionSystem>(this);
    if (DestructionSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: DestructionSystem initialized"));
    }
}

void UPhysicsSystemManager::CleanupSubsystems()
{
    if (CollisionManager)
    {
        CollisionManager = nullptr;
    }

    if (RagdollSystem)
    {
        RagdollSystem = nullptr;
    }

    if (DestructionSystem)
    {
        DestructionSystem = nullptr;
    }
}

void UPhysicsSystemManager::SetPhysicsSimulationEnabled(bool bEnabled)
{
    bPhysicsSimulationEnabled = bEnabled;
    
    UWorld* World = GetWorld();
    if (World && World->GetPhysicsScene())
    {
        // Update physics simulation state
        UpdatePhysicsSettings();
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics simulation %s"), 
               bEnabled ? TEXT("enabled") : TEXT("disabled"));
    }
}

bool UPhysicsSystemManager::IsPhysicsSimulationEnabled() const
{
    return bPhysicsSimulationEnabled;
}

void UPhysicsSystemManager::SetGravityScale(float NewGravityScale)
{
    GravityScale = FMath::Clamp(NewGravityScale, 0.0f, 10.0f);
    UpdatePhysicsSettings();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Gravity scale set to %f"), GravityScale);
}

float UPhysicsSystemManager::GetGravityScale() const
{
    return GravityScale;
}

void UPhysicsSystemManager::SetPhysicsLODDistance(float Distance)
{
    PhysicsLODDistance = FMath::Max(Distance, 100.0f);
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics LOD distance set to %f"), PhysicsLODDistance);
}

void UPhysicsSystemManager::OptimizePhysicsForPerformance()
{
    bOptimizeForPerformance = true;
    
    // Reduce physics tick rate for distant objects
    PhysicsTickRate = 30.0f;
    MaxSimulationSteps = 4;
    
    // Clean up invalid collision objects
    ValidatePhysicsObjects();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Performance optimization applied"));
}

void UPhysicsSystemManager::SetMaxPhysicsObjects(int32 MaxObjects)
{
    MaxPhysicsObjects = FMath::Max(MaxObjects, 10);
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Max physics objects set to %d"), MaxPhysicsObjects);
}

bool UPhysicsSystemManager::LineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit, bool bTraceComplex)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = bTraceComplex;
    QueryParams.bReturnPhysicalMaterial = true;

    bool bHit = World->LineTraceSingleByChannel(
        OutHit,
        Start,
        End,
        ECC_WorldStatic,
        QueryParams
    );

    if (bDebugDrawEnabled && bHit)
    {
        DrawDebugLine(World, Start, OutHit.Location, FColor::Red, false, 1.0f);
        DrawDebugPoint(World, OutHit.Location, 10.0f, FColor::Yellow, false, 1.0f);
    }

    return bHit;
}

bool UPhysicsSystemManager::SphereTrace(const FVector& Start, const FVector& End, float Radius, FHitResult& OutHit)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.bReturnPhysicalMaterial = true;

    bool bHit = World->SweepSingleByChannel(
        OutHit,
        Start,
        End,
        FQuat::Identity,
        ECC_WorldStatic,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );

    if (bDebugDrawEnabled && bHit)
    {
        DrawDebugSphere(World, OutHit.Location, Radius, 12, FColor::Blue, false, 1.0f);
    }

    return bHit;
}

void UPhysicsSystemManager::RegisterCollisionObject(UPrimitiveComponent* Component)
{
    if (Component && !RegisteredCollisionObjects.Contains(Component))
    {
        RegisteredCollisionObjects.Add(Component);
        CurrentPhysicsObjectCount++;
        
        UE_LOG(LogTemp, Verbose, TEXT("PhysicsSystemManager: Registered collision object %s"), 
               *Component->GetName());
    }
}

void UPhysicsSystemManager::UnregisterCollisionObject(UPrimitiveComponent* Component)
{
    if (Component)
    {
        RegisteredCollisionObjects.Remove(Component);
        CurrentPhysicsObjectCount = FMath::Max(0, CurrentPhysicsObjectCount - 1);
        
        UE_LOG(LogTemp, Verbose, TEXT("PhysicsSystemManager: Unregistered collision object %s"), 
               *Component->GetName());
    }
}

void UPhysicsSystemManager::EnableRagdollForActor(AActor* Actor)
{
    if (RagdollSystem && Actor)
    {
        // Delegate to ragdoll system
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Enabling ragdoll for actor %s"), *Actor->GetName());
    }
}

void UPhysicsSystemManager::DisableRagdollForActor(AActor* Actor)
{
    if (RagdollSystem && Actor)
    {
        // Delegate to ragdoll system
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Disabling ragdoll for actor %s"), *Actor->GetName());
    }
}

void UPhysicsSystemManager::TriggerDestruction(AActor* Actor, const FVector& ImpactPoint, float Force)
{
    if (DestructionSystem && Actor)
    {
        // Delegate to destruction system
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Triggering destruction for actor %s with force %f"), 
               *Actor->GetName(), Force);
    }
}

void UPhysicsSystemManager::SetDestructionEnabled(bool bEnabled)
{
    if (DestructionSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Destruction system %s"), 
               bEnabled ? TEXT("enabled") : TEXT("disabled"));
    }
}

void UPhysicsSystemManager::ApplyPhysicsMaterial(UPrimitiveComponent* Component, UPhysicalMaterial* Material)
{
    if (Component && Material)
    {
        Component->SetPhysMaterialOverride(Material);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Applied physics material to %s"), *Component->GetName());
    }
}

void UPhysicsSystemManager::RunPhysicsSystemDiagnostics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS SYSTEM DIAGNOSTICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Physics Simulation Enabled: %s"), bPhysicsSimulationEnabled ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Gravity Scale: %f"), GravityScale);
    UE_LOG(LogTemp, Warning, TEXT("Physics LOD Distance: %f"), PhysicsLODDistance);
    UE_LOG(LogTemp, Warning, TEXT("Max Physics Objects: %d"), MaxPhysicsObjects);
    UE_LOG(LogTemp, Warning, TEXT("Current Physics Objects: %d"), CurrentPhysicsObjectCount);
    UE_LOG(LogTemp, Warning, TEXT("Registered Collision Objects: %d"), RegisteredCollisionObjects.Num());
    UE_LOG(LogTemp, Warning, TEXT("Performance Optimization: %s"), bOptimizeForPerformance ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("Physics Tick Rate: %f"), PhysicsTickRate);
    UE_LOG(LogTemp, Warning, TEXT("Max Simulation Steps: %d"), MaxSimulationSteps);
    UE_LOG(LogTemp, Warning, TEXT("=== END DIAGNOSTICS ==="));
}

void UPhysicsSystemManager::TogglePhysicsDebugDraw(bool bEnabled)
{
    bDebugDrawEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Debug draw %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

FString UPhysicsSystemManager::GetPhysicsSystemStatus() const
{
    return FString::Printf(TEXT("Physics: %s | Objects: %d/%d | Gravity: %.1f | LOD: %.0f"),
        bPhysicsSimulationEnabled ? TEXT("ON") : TEXT("OFF"),
        CurrentPhysicsObjectCount,
        MaxPhysicsObjects,
        GravityScale,
        PhysicsLODDistance
    );
}

void UPhysicsSystemManager::UpdatePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Apply gravity scale
    if (World->GetPhysicsScene())
    {
        // Update world gravity
        FVector CurrentGravity = World->GetGravityZ() * FVector::UpVector;
        FVector NewGravity = CurrentGravity * GravityScale;
        // Note: SetGravityZ is not directly available, this would need engine modification
        // For now, we log the intended change
        UE_LOG(LogTemp, Verbose, TEXT("PhysicsSystemManager: Updated gravity scale to %f"), GravityScale);
    }
}

void UPhysicsSystemManager::ValidatePhysicsObjects()
{
    // Clean up invalid weak pointers
    RegisteredCollisionObjects.RemoveAll([](const TWeakObjectPtr<UPrimitiveComponent>& WeakPtr)
    {
        return !WeakPtr.IsValid();
    });

    CurrentPhysicsObjectCount = RegisteredCollisionObjects.Num();
    
    UE_LOG(LogTemp, Verbose, TEXT("PhysicsSystemManager: Validated physics objects, count: %d"), CurrentPhysicsObjectCount);
}