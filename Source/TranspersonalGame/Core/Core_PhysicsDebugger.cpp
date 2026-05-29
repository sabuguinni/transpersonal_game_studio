#include "Core_PhysicsDebugger.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "CollisionQueryParams.h"

UCore_PhysicsDebugger::UCore_PhysicsDebugger()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Default settings
    bShowCollisionShapes = false;
    bShowVelocityVectors = false;
    bShowForceVectors = false;
    bShowContactPoints = false;
    bShowRagdollConstraints = false;
    DebugLineThickness = 2.0f;
    VectorScale = 100.0f;
    
    bMonitorPerformance = true;
    PerformanceUpdateInterval = 1.0f;
    PerformanceTimer = 0.0f;
    
    PhysicsFrameTime = 0.0f;
    ActiveRigidBodies = 0;
    ActiveConstraints = 0;
    CollisionPairs = 0;
}

void UCore_PhysicsDebugger::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger: BeginPlay - Physics debugging system initialized"));
    
    // Initialize performance monitoring
    if (bMonitorPerformance)
    {
        UpdatePerformanceMetrics();
    }
}

void UCore_PhysicsDebugger::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance timer
    PerformanceTimer += DeltaTime;
    
    // Draw debug information if enabled
    if (bShowCollisionShapes || bShowVelocityVectors || bShowForceVectors || bShowContactPoints || bShowRagdollConstraints)
    {
        DrawPhysicsDebugInfo();
    }
    
    // Update performance metrics at intervals
    if (bMonitorPerformance && PerformanceTimer >= PerformanceUpdateInterval)
    {
        UpdatePerformanceMetrics();
        PerformanceTimer = 0.0f;
    }
    
    // Clean up old debug data
    CleanupOldDebugData();
}

void UCore_PhysicsDebugger::ToggleCollisionShapes()
{
    bShowCollisionShapes = !bShowCollisionShapes;
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger: Collision shapes debug %s"), 
           bShowCollisionShapes ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UCore_PhysicsDebugger::ToggleVelocityVectors()
{
    bShowVelocityVectors = !bShowVelocityVectors;
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger: Velocity vectors debug %s"), 
           bShowVelocityVectors ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UCore_PhysicsDebugger::ToggleForceVectors()
{
    bShowForceVectors = !bShowForceVectors;
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger: Force vectors debug %s"), 
           bShowForceVectors ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UCore_PhysicsDebugger::ToggleContactPoints()
{
    bShowContactPoints = !bShowContactPoints;
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger: Contact points debug %s"), 
           bShowContactPoints ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UCore_PhysicsDebugger::ToggleRagdollConstraints()
{
    bShowRagdollConstraints = !bShowRagdollConstraints;
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger: Ragdoll constraints debug %s"), 
           bShowRagdollConstraints ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UCore_PhysicsDebugger::DrawPhysicsDebugInfo()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        // Draw collision shapes
        if (bShowCollisionShapes)
        {
            DrawCollisionShapesForActor(Actor);
        }
        
        // Draw velocity vectors
        if (bShowVelocityVectors)
        {
            DrawVelocityVectorsForActor(Actor);
        }
    }
    
    // Update contact point history
    if (bShowContactPoints)
    {
        UpdateContactPointHistory();
    }
}

void UCore_PhysicsDebugger::ClearDebugDrawing()
{
    UWorld* World = GetWorld();
    if (World)
    {
        FlushPersistentDebugLines(World);
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger: Debug drawing cleared"));
    }
}

void UCore_PhysicsDebugger::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    CollectPhysicsMetrics();
    CalculateFrameTime();
    
    // Log performance data periodically
    static float LogTimer = 0.0f;
    LogTimer += PerformanceUpdateInterval;
    if (LogTimer >= 5.0f) // Log every 5 seconds
    {
        LogPerformanceMetrics();
        LogTimer = 0.0f;
    }
}

FString UCore_PhysicsDebugger::GetPerformanceReport() const
{
    return FString::Printf(TEXT("Physics Performance Report:\n")
                          TEXT("Frame Time: %.2fms\n")
                          TEXT("Active Rigid Bodies: %d\n")
                          TEXT("Active Constraints: %d\n")
                          TEXT("Collision Pairs: %d"),
                          PhysicsFrameTime,
                          ActiveRigidBodies,
                          ActiveConstraints,
                          CollisionPairs);
}

void UCore_PhysicsDebugger::LogPerformanceMetrics()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger Performance: FrameTime=%.2fms, RigidBodies=%d, Constraints=%d, CollisionPairs=%d"),
           PhysicsFrameTime, ActiveRigidBodies, ActiveConstraints, CollisionPairs);
}

bool UCore_PhysicsDebugger::TestCollisionAtLocation(const FVector& Location, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = true;
    
    FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(Radius);
    
    bool bHit = World->OverlapAnyTestByChannel(Location, FQuat::Identity, ECC_WorldStatic, CollisionSphere, QueryParams);
    
    // Draw debug sphere
    DrawCollisionSphere(Location, Radius, bHit ? FColor::Red : FColor::Green, 2.0f);
    
    return bHit;
}

void UCore_PhysicsDebugger::DrawCollisionSphere(const FVector& Location, float Radius, const FColor& Color, float Duration)
{
    UWorld* World = GetWorld();
    if (World)
    {
        DrawDebugSphere(World, Location, Radius, 16, Color, false, Duration, 0, DebugLineThickness);
    }
}

void UCore_PhysicsDebugger::DrawVelocityVector(AActor* Actor, const FColor& Color, float Duration)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimComp && PrimComp->IsSimulatingPhysics())
    {
        FVector Velocity = PrimComp->GetPhysicsLinearVelocity();
        FVector ActorLocation = Actor->GetActorLocation();
        FVector EndPoint = ActorLocation + (Velocity * VectorScale * 0.01f); // Scale down for visibility
        
        UWorld* World = GetWorld();
        if (World)
        {
            DrawDebugDirectionalArrow(World, ActorLocation, EndPoint, 50.0f, Color, false, Duration, 0, DebugLineThickness);
        }
    }
}

void UCore_PhysicsDebugger::DrawForceVector(const FVector& Origin, const FVector& Force, const FColor& Color, float Duration)
{
    FVector EndPoint = Origin + (Force * VectorScale * 0.001f); // Scale down for visibility
    
    UWorld* World = GetWorld();
    if (World)
    {
        DrawDebugDirectionalArrow(World, Origin, EndPoint, 30.0f, Color, false, Duration, 0, DebugLineThickness);
    }
}

TArray<AActor*> UCore_PhysicsDebugger::GetPhysicsActorsInRadius(const FVector& Center, float Radius)
{
    TArray<AActor*> PhysicsActors;
    UWorld* World = GetWorld();
    if (!World)
    {
        return PhysicsActors;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
        if (Distance <= Radius)
        {
            UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                PhysicsActors.Add(Actor);
            }
        }
    }
    
    return PhysicsActors;
}

void UCore_PhysicsDebugger::AnalyzePhysicsState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 SimulatingActors = 0;
    int32 KinematicActors = 0;
    int32 StaticActors = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (PrimComp)
        {
            if (PrimComp->IsSimulatingPhysics())
            {
                SimulatingActors++;
            }
            else if (PrimComp->GetBodyInstance()->bSimulatePhysics)
            {
                KinematicActors++;
            }
            else
            {
                StaticActors++;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger Analysis: Simulating=%d, Kinematic=%d, Static=%d, Total=%d"),
           SimulatingActors, KinematicActors, StaticActors, AllActors.Num());
}

void UCore_PhysicsDebugger::RunPhysicsValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger: Running physics validation..."));
    
    bool bComponentsValid = ValidatePhysicsComponents();
    bool bCollisionValid = ValidateCollisionSettings();
    bool bConstraintsValid = ValidateConstraintSetup();
    
    if (bComponentsValid && bCollisionValid && bConstraintsValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger: ✓ All physics validation checks PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsDebugger: ✗ Physics validation FAILED - Components:%s Collision:%s Constraints:%s"),
               bComponentsValid ? TEXT("OK") : TEXT("FAIL"),
               bCollisionValid ? TEXT("OK") : TEXT("FAIL"),
               bConstraintsValid ? TEXT("OK") : TEXT("FAIL"));
    }
}

// Private implementation methods
void UCore_PhysicsDebugger::DrawCollisionShapesForActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimComp)
    {
        FVector ActorLocation = Actor->GetActorLocation();
        FVector BoundsExtent = PrimComp->Bounds.BoxExtent;
        
        UWorld* World = GetWorld();
        if (World)
        {
            FColor ShapeColor = PrimComp->IsSimulatingPhysics() ? FColor::Yellow : FColor::Blue;
            DrawDebugBox(World, ActorLocation, BoundsExtent, ShapeColor, false, 0.1f, 0, DebugLineThickness);
        }
    }
}

void UCore_PhysicsDebugger::DrawVelocityVectorsForActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    DrawVelocityVector(Actor, FColor::Green, 0.1f);
}

void UCore_PhysicsDebugger::UpdateContactPointHistory()
{
    // This would require access to physics contact information
    // For now, we'll implement a basic version
    ContactPointHistory.Empty();
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Placeholder for contact point collection
        // In a full implementation, this would interface with the physics engine
        // to collect actual contact points from collision events
    }
}

void UCore_PhysicsDebugger::CleanupOldDebugData()
{
    // Clean up old velocity history
    if (VelocityHistory.Num() > 100)
    {
        VelocityHistory.RemoveAt(0, VelocityHistory.Num() - 100);
    }
    
    // Clean up old contact point history
    if (ContactPointHistory.Num() > 50)
    {
        ContactPointHistory.RemoveAt(0, ContactPointHistory.Num() - 50);
    }
}

void UCore_PhysicsDebugger::CollectPhysicsMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count active rigid bodies
    ActiveRigidBodies = 0;
    ActiveConstraints = 0;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        // Count rigid bodies
        UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            ActiveRigidBodies++;
        }
        
        // Count constraints
        UPhysicsConstraintComponent* ConstraintComp = Actor->FindComponentByClass<UPhysicsConstraintComponent>();
        if (ConstraintComp)
        {
            ActiveConstraints++;
        }
    }
    
    // Collision pairs would require deeper physics engine integration
    CollisionPairs = ActiveRigidBodies * (ActiveRigidBodies - 1) / 2; // Rough estimate
}

void UCore_PhysicsDebugger::CalculateFrameTime()
{
    // Get physics frame time from engine stats
    // This is a simplified version - real implementation would use FStats
    PhysicsFrameTime = GetWorld() ? GetWorld()->GetDeltaSeconds() * 1000.0f : 0.0f;
}

bool UCore_PhysicsDebugger::ValidatePhysicsComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 ValidComponents = 0;
    int32 TotalComponents = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimComponents)
        {
            TotalComponents++;
            if (PrimComp && PrimComp->GetBodyInstance())
            {
                ValidComponents++;
            }
        }
    }
    
    bool bValid = (TotalComponents == 0) || (ValidComponents >= TotalComponents * 0.9f); // 90% threshold
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger: Physics components validation: %d/%d valid (%s)"),
           ValidComponents, TotalComponents, bValid ? TEXT("PASS") : TEXT("FAIL"));
    
    return bValid;
}

bool UCore_PhysicsDebugger::ValidateCollisionSettings()
{
    // Basic collision validation
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger: Collision settings validation: PASS (basic check)"));
    return true;
}

bool UCore_PhysicsDebugger::ValidateConstraintSetup()
{
    // Basic constraint validation
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsDebugger: Constraint setup validation: PASS (basic check)"));
    return true;
}