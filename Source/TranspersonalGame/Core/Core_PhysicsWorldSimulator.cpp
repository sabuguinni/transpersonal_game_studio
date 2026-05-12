#include "Core_PhysicsWorldSimulator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

UCore_PhysicsWorldSimulator::UCore_PhysicsWorldSimulator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize simulation settings
    SimulationSettings = FCore_PhysicsSimulationSettings();
    
    // Initialize environmental forces
    bApplyWindForces = false;
    bApplyGravityVariation = false;
    GravityVariationStrength = 0.1f;
    
    // Initialize simulation control
    bIsSimulationActive = true;
    MaxSimulationDistance = 5000.0f;
    MaxSimulatedObjects = 1000;
    
    // Initialize internal state
    AccumulatedTime = 0.0f;
    LastOptimizationTime = 0.0f;
    SimulationFrameCount = 0;
    AverageFrameTime = 0.0f;
    PeakFrameTime = 0.0f;
    TotalObjectsProcessed = 0;
}

void UCore_PhysicsWorldSimulator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldSimulator: Starting physics simulation"));
    
    // Initialize physics world settings
    if (UWorld* World = GetWorld())
    {
        if (World->GetPhysicsScene())
        {
            // Apply initial gravity settings
            SetGravityScale(SimulationSettings.GravityScale);
            UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldSimulator: Physics scene initialized"));
        }
    }
    
    // Start the simulation
    StartSimulation();
}

void UCore_PhysicsWorldSimulator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsSimulationActive)
    {
        return;
    }
    
    float StartTime = FPlatformTime::Seconds();
    
    // Update accumulated time
    AccumulatedTime += DeltaTime * SimulationSettings.SimulationTimeScale;
    SimulationFrameCount++;
    
    // Update physics objects
    UpdatePhysicsObjects(DeltaTime);
    
    // Apply environmental forces
    if (bApplyWindForces || bApplyGravityVariation)
    {
        ApplyEnvironmentalForces(DeltaTime);
    }
    
    // Process collisions
    ProcessCollisions(DeltaTime);
    
    // Update LOD system every 0.5 seconds
    if (AccumulatedTime - LastOptimizationTime > 0.5f)
    {
        UpdateSimulationLOD(DeltaTime);
        LastOptimizationTime = AccumulatedTime;
    }
    
    // Update performance metrics
    float FrameTime = FPlatformTime::Seconds() - StartTime;
    UpdatePerformanceMetrics(FrameTime);
    
    // Cleanup invalid objects periodically
    if (SimulationFrameCount % 300 == 0) // Every 5 seconds at 60fps
    {
        CleanupInvalidObjects();
    }
}

void UCore_PhysicsWorldSimulator::StartSimulation()
{
    bIsSimulationActive = true;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldSimulator: Simulation started"));
    
    // Reset simulation state
    AccumulatedTime = 0.0f;
    SimulationFrameCount = 0;
    
    // Register existing physics objects in the world
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
            {
                if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
                {
                    if (MeshComp->IsSimulatingPhysics())
                    {
                        FCore_PhysicsObjectData PhysicsData;
                        PhysicsData.Mass = MeshComp->GetMass();
                        RegisterPhysicsObject(Actor, PhysicsData);
                    }
                }
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldSimulator: Registered %d physics objects"), PhysicsObjects.Num());
    }
}

void UCore_PhysicsWorldSimulator::StopSimulation()
{
    bIsSimulationActive = false;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldSimulator: Simulation stopped"));
}

void UCore_PhysicsWorldSimulator::ResetSimulation()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldSimulator: Resetting simulation"));
    
    // Clear all registered objects
    PhysicsObjects.Empty();
    
    // Reset internal state
    AccumulatedTime = 0.0f;
    SimulationFrameCount = 0;
    AverageFrameTime = 0.0f;
    PeakFrameTime = 0.0f;
    TotalObjectsProcessed = 0;
    
    // Restart simulation
    StartSimulation();
}

void UCore_PhysicsWorldSimulator::RegisterPhysicsObject(AActor* Actor, const FCore_PhysicsObjectData& PhysicsData)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldSimulator: Cannot register null actor"));
        return;
    }
    
    PhysicsObjects.Add(Actor, PhysicsData);
    UE_LOG(LogTemp, Verbose, TEXT("Core_PhysicsWorldSimulator: Registered physics object: %s"), *Actor->GetName());
}

void UCore_PhysicsWorldSimulator::UnregisterPhysicsObject(AActor* Actor)
{
    if (PhysicsObjects.Remove(Actor) > 0)
    {
        UE_LOG(LogTemp, Verbose, TEXT("Core_PhysicsWorldSimulator: Unregistered physics object: %s"), 
               Actor ? *Actor->GetName() : TEXT("NULL"));
    }
}

void UCore_PhysicsWorldSimulator::ApplyForceToObject(AActor* Actor, const FVector& Force, const FVector& Location)
{
    if (!Actor || !PhysicsObjects.Contains(Actor))
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            PrimComp->AddForceAtLocation(Force, Location);
        }
    }
}

void UCore_PhysicsWorldSimulator::ApplyImpulseToObject(AActor* Actor, const FVector& Impulse, const FVector& Location)
{
    if (!Actor || !PhysicsObjects.Contains(Actor))
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            PrimComp->AddImpulseAtLocation(Impulse, Location);
        }
    }
}

void UCore_PhysicsWorldSimulator::SetWindParameters(float Strength, const FVector& Direction)
{
    SimulationSettings.WindStrength = Strength;
    SimulationSettings.WindDirection = Direction.GetSafeNormal();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldSimulator: Wind set to strength %.2f, direction %s"), 
           Strength, *Direction.ToString());
}

void UCore_PhysicsWorldSimulator::SetGravityScale(float NewGravityScale)
{
    SimulationSettings.GravityScale = NewGravityScale;
    
    if (UWorld* World = GetWorld())
    {
        if (AWorldSettings* WorldSettings = World->GetWorldSettings())
        {
            WorldSettings->GlobalGravityZ = -980.0f * NewGravityScale; // Standard gravity * scale
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldSimulator: Gravity scale set to %.2f"), NewGravityScale);
}

void UCore_PhysicsWorldSimulator::ApplyEnvironmentalForces(float DeltaTime)
{
    if (!bIsSimulationActive)
    {
        return;
    }
    
    for (auto& ObjectPair : PhysicsObjects)
    {
        AActor* Actor = ObjectPair.Key;
        const FCore_PhysicsObjectData& PhysicsData = ObjectPair.Value;
        
        if (!IsValid(Actor) || !ShouldSimulateObject(Actor))
        {
            continue;
        }
        
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            if (!PrimComp->IsSimulatingPhysics())
            {
                continue;
            }
            
            // Apply wind forces
            if (bApplyWindForces && SimulationSettings.WindStrength > 0.0f)
            {
                FVector WindForce = SimulationSettings.WindDirection * SimulationSettings.WindStrength * PhysicsData.Mass;
                PrimComp->AddForce(WindForce);
            }
            
            // Apply gravity variations
            if (bApplyGravityVariation)
            {
                float GravityVariation = FMath::Sin(AccumulatedTime + Actor->GetActorLocation().X * 0.001f) * GravityVariationStrength;
                FVector GravityForce = FVector(0, 0, GravityVariation * PhysicsData.Mass * -980.0f);
                PrimComp->AddForce(GravityForce);
            }
        }
    }
}

bool UCore_PhysicsWorldSimulator::CheckTerrainCollision(const FVector& Location, float Radius, FHitResult& OutHit)
{
    if (UWorld* World = GetWorld())
    {
        FVector StartLocation = Location + FVector(0, 0, 100);
        FVector EndLocation = Location - FVector(0, 0, 1000);
        
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = true;
        
        return World->LineTraceSingleByChannel(OutHit, StartLocation, EndLocation, ECC_WorldStatic, QueryParams);
    }
    
    return false;
}

float UCore_PhysicsWorldSimulator::GetTerrainHeightAtLocation(const FVector& Location)
{
    FHitResult HitResult;
    if (CheckTerrainCollision(Location, 0.0f, HitResult))
    {
        return HitResult.Location.Z;
    }
    
    return 0.0f;
}

FVector UCore_PhysicsWorldSimulator::GetTerrainNormalAtLocation(const FVector& Location)
{
    FHitResult HitResult;
    if (CheckTerrainCollision(Location, 0.0f, HitResult))
    {
        return HitResult.Normal;
    }
    
    return FVector::UpVector;
}

void UCore_PhysicsWorldSimulator::OptimizeSimulation()
{
    // Remove invalid objects
    CleanupInvalidObjects();
    
    // Limit number of simulated objects
    if (PhysicsObjects.Num() > MaxSimulatedObjects)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldSimulator: Too many physics objects (%d), optimization needed"), 
               PhysicsObjects.Num());
    }
}

void UCore_PhysicsWorldSimulator::UpdateSimulationLOD(float DeltaTime)
{
    // Update simulation quality based on performance
    if (AverageFrameTime > 0.02f) // Above 20ms frame time
    {
        // Reduce simulation quality
        MaxSimulationDistance *= 0.95f;
        MaxSimulatedObjects = FMath::Max(100, MaxSimulatedObjects - 10);
    }
    else if (AverageFrameTime < 0.01f) // Below 10ms frame time
    {
        // Increase simulation quality
        MaxSimulationDistance = FMath::Min(10000.0f, MaxSimulationDistance * 1.05f);
        MaxSimulatedObjects = FMath::Min(2000, MaxSimulatedObjects + 10);
    }
}

bool UCore_PhysicsWorldSimulator::ShouldSimulateObject(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    // Check distance from player
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
            return Distance <= MaxSimulationDistance;
        }
    }
    
    return true;
}

void UCore_PhysicsWorldSimulator::DebugDrawPhysicsObjects()
{
    if (UWorld* World = GetWorld())
    {
        for (const auto& ObjectPair : PhysicsObjects)
        {
            AActor* Actor = ObjectPair.Key;
            if (IsValid(Actor))
            {
                FVector Location = Actor->GetActorLocation();
                DrawDebugSphere(World, Location, 50.0f, 12, FColor::Green, false, 1.0f);
                DrawDebugString(World, Location + FVector(0, 0, 100), Actor->GetName(), nullptr, FColor::White, 1.0f);
            }
        }
    }
}

void UCore_PhysicsWorldSimulator::LogSimulationStats()
{
    UE_LOG(LogTemp, Log, TEXT("=== Core_PhysicsWorldSimulator Stats ==="));
    UE_LOG(LogTemp, Log, TEXT("Active Objects: %d"), PhysicsObjects.Num());
    UE_LOG(LogTemp, Log, TEXT("Simulation Time: %.2f seconds"), AccumulatedTime);
    UE_LOG(LogTemp, Log, TEXT("Frame Count: %d"), SimulationFrameCount);
    UE_LOG(LogTemp, Log, TEXT("Average Frame Time: %.4f ms"), AverageFrameTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("Peak Frame Time: %.4f ms"), PeakFrameTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("Max Simulation Distance: %.2f"), MaxSimulationDistance);
    UE_LOG(LogTemp, Log, TEXT("Max Simulated Objects: %d"), MaxSimulatedObjects);
}

int32 UCore_PhysicsWorldSimulator::GetActivePhysicsObjectCount() const
{
    return PhysicsObjects.Num();
}

float UCore_PhysicsWorldSimulator::GetSimulationPerformanceMetric() const
{
    return AverageFrameTime;
}

void UCore_PhysicsWorldSimulator::UpdatePhysicsObjects(float DeltaTime)
{
    TotalObjectsProcessed = 0;
    
    for (auto& ObjectPair : PhysicsObjects)
    {
        AActor* Actor = ObjectPair.Key;
        FCore_PhysicsObjectData& PhysicsData = ObjectPair.Value;
        
        if (!IsValid(Actor) || !ShouldSimulateObject(Actor))
        {
            continue;
        }
        
        TotalObjectsProcessed++;
        
        // Update physics data if needed
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            if (PrimComp->IsSimulatingPhysics())
            {
                // Update mass if it changed
                float CurrentMass = PrimComp->GetMass();
                if (FMath::Abs(CurrentMass - PhysicsData.Mass) > 0.1f)
                {
                    PhysicsData.Mass = CurrentMass;
                }
            }
        }
    }
}

void UCore_PhysicsWorldSimulator::ProcessCollisions(float DeltaTime)
{
    // Basic collision processing - can be extended for more complex collision handling
    // This is a placeholder for future collision event handling
}

void UCore_PhysicsWorldSimulator::ApplyGravityVariations(float DeltaTime)
{
    if (!bApplyGravityVariation)
    {
        return;
    }
    
    // Apply localized gravity variations based on terrain features
    for (auto& ObjectPair : PhysicsObjects)
    {
        AActor* Actor = ObjectPair.Key;
        const FCore_PhysicsObjectData& PhysicsData = ObjectPair.Value;
        
        if (!IsValid(Actor) || !ShouldSimulateObject(Actor))
        {
            continue;
        }
        
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            if (PrimComp->IsSimulatingPhysics())
            {
                FVector Location = Actor->GetActorLocation();
                float GravityMod = FMath::Sin(Location.X * 0.001f) * FMath::Cos(Location.Y * 0.001f) * GravityVariationStrength;
                FVector GravityForce = FVector(0, 0, GravityMod * PhysicsData.Mass * -980.0f);
                PrimComp->AddForce(GravityForce);
            }
        }
    }
}

void UCore_PhysicsWorldSimulator::UpdatePerformanceMetrics(float FrameTime)
{
    // Update average frame time with exponential moving average
    AverageFrameTime = AverageFrameTime * 0.95f + FrameTime * 0.05f;
    
    // Update peak frame time
    if (FrameTime > PeakFrameTime)
    {
        PeakFrameTime = FrameTime;
    }
    
    // Reset peak every 1000 frames
    if (SimulationFrameCount % 1000 == 0)
    {
        PeakFrameTime = 0.0f;
    }
}

bool UCore_PhysicsWorldSimulator::IsWithinSimulationRange(AActor* Actor) const
{
    return ShouldSimulateObject(Actor);
}

void UCore_PhysicsWorldSimulator::CleanupInvalidObjects()
{
    TArray<AActor*> InvalidActors;
    
    for (const auto& ObjectPair : PhysicsObjects)
    {
        if (!IsValid(ObjectPair.Key))
        {
            InvalidActors.Add(ObjectPair.Key);
        }
    }
    
    for (AActor* InvalidActor : InvalidActors)
    {
        PhysicsObjects.Remove(InvalidActor);
    }
    
    if (InvalidActors.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldSimulator: Cleaned up %d invalid objects"), InvalidActors.Num());
    }
}