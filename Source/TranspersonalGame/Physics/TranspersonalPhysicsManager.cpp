// TranspersonalPhysicsManager.cpp
// Implementation of core physics management system

#include "TranspersonalPhysicsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Chaos/ChaosEngineInterface.h"
#include "TimerManager.h"

UTranspersonalPhysicsManager::UTranspersonalPhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60fps target
    
    PhysicsFrameTime = 0.0f;
    ActivePhysicsActors = 0;
}

void UTranspersonalPhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize physics world
    if (UWorld* World = GetWorld())
    {
        InitializePhysicsWorld(World);
        
        // Set up performance monitoring timer
        World->GetTimerManager().SetTimer(
            PhysicsUpdateTimer,
            this,
            &UTranspersonalPhysicsManager::UpdatePhysicsLOD,
            0.1f, // Update LOD every 100ms
            true
        );
    }
}

void UTranspersonalPhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Track physics performance
    float StartTime = FPlatformTime::Seconds();
    
    // Process physics operations queue
    ProcessPhysicsQueue();
    
    // Calculate frame time
    PhysicsFrameTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f; // Convert to milliseconds
    
    // Performance warning if exceeding budget
    if (PhysicsFrameTime > MaxPhysicsFrameTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics frame time exceeded budget: %.2fms (max: %.2fms)"), 
               PhysicsFrameTime, MaxPhysicsFrameTime);
    }
}

bool UTranspersonalPhysicsManager::InitializePhysicsWorld(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize physics: World is null"));
        return false;
    }
    
    // Configure Chaos Physics settings for optimal performance
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        // Set solver iterations for balance between quality and performance
        PhysicsSettings->SolverOptions.SolverIterations = 8;
        PhysicsSettings->SolverOptions.VelocityIterations = 1;
        PhysicsSettings->SolverOptions.ProjectionIterations = 1;
        
        // Configure collision settings
        PhysicsSettings->DefaultGravityZ = -980.0f; // Earth gravity
        PhysicsSettings->DefaultTerminalVelocity = 4000.0f;
        
        // Optimize for open world
        PhysicsSettings->bSubstepping = true;
        PhysicsSettings->bSubsteppingAsync = true;
        PhysicsSettings->MaxSubstepDeltaTime = 0.016f; // 60fps max
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics world initialized successfully"));
    return true;
}

void UTranspersonalPhysicsManager::RegisterPhysicsActor(AActor* Actor, EPhysicsType PhysicsType)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register null actor for physics"));
        return;
    }
    
    // Check if we're at the actor limit
    if (RegisteredActors.Num() >= MaxActivePhysicsActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics actor limit reached (%d). Consider increasing limit or optimizing."), 
               MaxActivePhysicsActors);
        return;
    }
    
    RegisteredActors.Add(Actor, PhysicsType);
    ActivePhysicsActors = RegisteredActors.Num();
    
    // Set up collision event binding for important actors
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PhysicsType == EPhysicsType::Destructible || PhysicsType == EPhysicsType::HighPrecision)
        {
            PrimComp->OnComponentHit.AddDynamic(this, &UTranspersonalPhysicsManager::OnPhysicsCollision);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Registered physics actor: %s (Type: %d)"), 
           *Actor->GetName(), (int32)PhysicsType);
}

void UTranspersonalPhysicsManager::UnregisterPhysicsActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    if (RegisteredActors.Remove(Actor) > 0)
    {
        ActivePhysicsActors = RegisteredActors.Num();
        
        // Remove collision event binding
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->OnComponentHit.RemoveAll(this);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Unregistered physics actor: %s"), *Actor->GetName());
    }
}

void UTranspersonalPhysicsManager::ApplyOptimizedImpulse(AActor* Actor, FVector Impulse, FVector Location, bool bVelChange)
{
    if (!Actor)
    {
        return;
    }
    
    // Queue the operation for performance optimization
    FPhysicsOperation Operation;
    Operation.TargetActor = Actor;
    Operation.ImpulseVector = Impulse;
    Operation.Location = Location;
    Operation.Priority = CalculateOperationPriority(Actor, Impulse.Size());
    Operation.Timestamp = GetWorld()->GetTimeSeconds();
    
    PhysicsOperationQueue.Enqueue(Operation);
}

void UTranspersonalPhysicsManager::SetPhysicsEnabled(AActor* Actor, bool bEnable)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        PrimComp->SetSimulatePhysics(bEnable);
        
        // Optimize collision when physics is disabled
        if (!bEnable)
        {
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        else
        {
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
}

FPhysicsPerformanceData UTranspersonalPhysicsManager::GetPhysicsPerformanceData() const
{
    FPhysicsPerformanceData Data;
    Data.FrameTime = PhysicsFrameTime;
    Data.ActiveActors = ActivePhysicsActors;
    Data.CollisionChecks = GetCollisionCheckCount();
    Data.MemoryUsage = GetPhysicsMemoryUsage();
    
    return Data;
}

void UTranspersonalPhysicsManager::UpdatePhysicsLOD()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get player location for distance calculations
    FVector PlayerLocation = FVector::ZeroVector;
    if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    // Update LOD for all registered actors
    for (auto& ActorPair : RegisteredActors)
    {
        AActor* Actor = ActorPair.Key;
        EPhysicsType PhysicsType = ActorPair.Value;
        
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        // Apply LOD based on distance and physics type
        ApplyPhysicsLOD(Actor, Distance, PhysicsType);
    }
}

void UTranspersonalPhysicsManager::ProcessPhysicsQueue()
{
    const int32 MaxOperationsPerFrame = 10; // Limit operations per frame
    int32 ProcessedOperations = 0;
    
    FPhysicsOperation Operation;
    while (PhysicsOperationQueue.Dequeue(Operation) && ProcessedOperations < MaxOperationsPerFrame)
    {
        if (Operation.TargetActor && IsValid(Operation.TargetActor))
        {
            // Apply the impulse
            if (UPrimitiveComponent* PrimComp = Operation.TargetActor->FindComponentByClass<UPrimitiveComponent>())
            {
                PrimComp->AddImpulseAtLocation(Operation.ImpulseVector, Operation.Location);
            }
        }
        
        ProcessedOperations++;
    }
}

void UTranspersonalPhysicsManager::OnPhysicsCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HitComp || !OtherActor)
    {
        return;
    }
    
    float ImpactMagnitude = NormalImpulse.Size();
    
    // Trigger events based on impact magnitude
    if (ImpactMagnitude > 1000.0f) // Major impact threshold
    {
        OnMajorImpact.Broadcast(HitComp->GetOwner(), Hit.ImpactPoint);
    }
    
    // Check for destructible objects
    if (RegisteredActors.Contains(HitComp->GetOwner()))
    {
        EPhysicsType PhysicsType = RegisteredActors[HitComp->GetOwner()];
        if (PhysicsType == EPhysicsType::Destructible && ImpactMagnitude > 500.0f)
        {
            OnDestructionEvent.Broadcast(HitComp->GetOwner(), Hit.ImpactPoint);
        }
    }
}

void UTranspersonalPhysicsManager::ApplyPhysicsLOD(AActor* Actor, float Distance, EPhysicsType PhysicsType)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp)
    {
        return;
    }
    
    // Determine LOD level based on distance
    EPhysicsLOD LODLevel = EPhysicsLOD::High;
    
    if (Distance > LowDetailDistance)
    {
        LODLevel = EPhysicsLOD::Disabled;
    }
    else if (Distance > MediumDetailDistance)
    {
        LODLevel = EPhysicsLOD::Low;
    }
    else if (Distance > HighDetailDistance)
    {
        LODLevel = EPhysicsLOD::Medium;
    }
    
    // Apply LOD settings
    switch (LODLevel)
    {
        case EPhysicsLOD::High:
            PrimComp->SetSimulatePhysics(true);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case EPhysicsLOD::Medium:
            PrimComp->SetSimulatePhysics(true);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            // Reduce solver iterations for medium LOD
            break;
            
        case EPhysicsLOD::Low:
            PrimComp->SetSimulatePhysics(false);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
            
        case EPhysicsLOD::Disabled:
            PrimComp->SetSimulatePhysics(false);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
    }
}

float UTranspersonalPhysicsManager::CalculateOperationPriority(AActor* Actor, float ImpulseMagnitude) const
{
    if (!Actor)
    {
        return 0.0f;
    }
    
    float Priority = 1.0f;
    
    // Higher priority for player-related physics
    if (Actor->IsA<ACharacter>())
    {
        Priority += 2.0f;
    }
    
    // Higher priority for larger impulses
    Priority += FMath::Clamp(ImpulseMagnitude / 1000.0f, 0.0f, 2.0f);
    
    // Lower priority for distant objects
    if (GetWorld())
    {
        if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
            Priority *= FMath::Clamp(1.0f - (Distance / 5000.0f), 0.1f, 1.0f);
        }
    }
    
    return Priority;
}

int32 UTranspersonalPhysicsManager::GetCollisionCheckCount() const
{
    // This would be implemented with actual Chaos physics profiling
    // For now, return estimated value based on active actors
    return ActivePhysicsActors * 4; // Estimated collision checks per actor
}

float UTranspersonalPhysicsManager::GetPhysicsMemoryUsage() const
{
    // This would be implemented with actual memory profiling
    // For now, return estimated value
    return ActivePhysicsActors * 0.5f; // Estimated MB per physics actor
}