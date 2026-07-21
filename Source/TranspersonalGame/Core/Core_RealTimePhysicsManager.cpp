#include "Core_RealTimePhysicsManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/BodyInstance.h"
#include "DrawDebugHelpers.h"

UCore_RealTimePhysicsManager::UCore_RealTimePhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default settings
    MaxAllowedPhysicsTime = 16.67f; // Target 60fps
    MaxActivePhysicsActors = 50;
    MinImpactForceThreshold = 100.0f;
    MaxImpactForceProcessed = 10000.0f;
    MinFallHeightThreshold = 100.0f;
    MaxFallVelocityProcessed = 2000.0f;
    MinCollisionVelocityThreshold = 50.0f;
    bProcessStaticCollisions = true;
    bProcessDynamicCollisions = true;
    
    // Initialize runtime state
    CurrentPhysicsFrameTime = 0.0f;
    PhysicsFrameAccumulator = 0.0f;
    PhysicsFrameCounter = 0;
}

void UCore_RealTimePhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Core_RealTimePhysicsManager: Initialized for real-time physics processing"));
    
    // Initialize physics tracking
    ActivePhysicsActors.Empty();
    ActorLastUpdateTime.Empty();
}

void UCore_RealTimePhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePhysicsPerformanceMetrics(DeltaTime);
    
    // Process active physics actors
    ProcessPhysicsActorUpdates();
    
    // Cleanup inactive actors
    CleanupInactivePhysicsActors();
    
    // Optimize settings if needed
    if (CurrentPhysicsFrameTime > MaxAllowedPhysicsTime)
    {
        OptimizePhysicsSettings();
    }
}

void UCore_RealTimePhysicsManager::ProcessImpactEvent(AActor* Actor, const FVector& ImpactLocation, const FVector& ImpactVelocity, float ImpactForce)
{
    if (!Actor || !ShouldProcessPhysicsEvent(Actor))
    {
        return;
    }
    
    // Check impact force threshold
    if (ImpactForce < MinImpactForceThreshold || ImpactForce > MaxImpactForceProcessed)
    {
        return;
    }
    
    // Add to active physics actors if not already tracked
    if (!ActivePhysicsActors.Contains(Actor))
    {
        ActivePhysicsActors.Add(Actor);
        ActorLastUpdateTime.Add(Actor, GetWorld()->GetTimeSeconds());
    }
    
    // Process impact physics
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            // Apply impact response
            FVector ImpactDirection = ImpactVelocity.GetSafeNormal();
            float ScaledForce = FMath::Clamp(ImpactForce, MinImpactForceThreshold, MaxImpactForceProcessed);
            
            PrimComp->AddImpulseAtLocation(ImpactDirection * ScaledForce, ImpactLocation);
            
            // Trigger event
            TriggerPhysicsEvent(Actor, ECore_PhysicsEventType::Impact);
            
            UE_LOG(LogTemp, Log, TEXT("Processed impact event for %s: Force=%.2f at %s"), 
                   *Actor->GetName(), ScaledForce, *ImpactLocation.ToString());
        }
    }
}

void UCore_RealTimePhysicsManager::ProcessFallEvent(AActor* Actor, const FVector& FallLocation, float FallHeight, float FallVelocity)
{
    if (!Actor || !ShouldProcessPhysicsEvent(Actor))
    {
        return;
    }
    
    // Check fall thresholds
    if (FallHeight < MinFallHeightThreshold || FallVelocity > MaxFallVelocityProcessed)
    {
        return;
    }
    
    // Add to active physics actors
    if (!ActivePhysicsActors.Contains(Actor))
    {
        ActivePhysicsActors.Add(Actor);
        ActorLastUpdateTime.Add(Actor, GetWorld()->GetTimeSeconds());
    }
    
    // Calculate fall damage/effects based on height and velocity
    float FallImpactForce = FallHeight * FMath::Abs(FallVelocity) * 0.1f;
    
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            // Apply fall impact
            FVector DownwardForce = FVector(0.0f, 0.0f, -FallImpactForce);
            PrimComp->AddImpulseAtLocation(DownwardForce, FallLocation);
            
            // Trigger event
            TriggerPhysicsEvent(Actor, ECore_PhysicsEventType::Fall);
            
            UE_LOG(LogTemp, Log, TEXT("Processed fall event for %s: Height=%.2f, Velocity=%.2f"), 
                   *Actor->GetName(), FallHeight, FallVelocity);
        }
    }
}

void UCore_RealTimePhysicsManager::ProcessCollisionEvent(AActor* ActorA, AActor* ActorB, const FVector& CollisionPoint, const FVector& CollisionNormal)
{
    if (!ActorA || !ActorB)
    {
        return;
    }
    
    // Check if we should process this collision type
    bool bActorAStatic = !ActorA->FindComponentByClass<UPrimitiveComponent>() || !ActorA->FindComponentByClass<UPrimitiveComponent>()->IsSimulatingPhysics();
    bool bActorBStatic = !ActorB->FindComponentByClass<UPrimitiveComponent>() || !ActorB->FindComponentByClass<UPrimitiveComponent>()->IsSimulatingPhysics();
    
    if ((bActorAStatic && bActorBStatic) || 
        (bActorAStatic && !bProcessStaticCollisions) ||
        (!bActorAStatic && !bActorBStatic && !bProcessDynamicCollisions))
    {
        return;
    }
    
    // Add actors to tracking
    if (ShouldProcessPhysicsEvent(ActorA) && !ActivePhysicsActors.Contains(ActorA))
    {
        ActivePhysicsActors.Add(ActorA);
        ActorLastUpdateTime.Add(ActorA, GetWorld()->GetTimeSeconds());
    }
    
    if (ShouldProcessPhysicsEvent(ActorB) && !ActivePhysicsActors.Contains(ActorB))
    {
        ActivePhysicsActors.Add(ActorB);
        ActorLastUpdateTime.Add(ActorB, GetWorld()->GetTimeSeconds());
    }
    
    // Calculate collision response
    UPrimitiveComponent* PrimCompA = ActorA->FindComponentByClass<UPrimitiveComponent>();
    UPrimitiveComponent* PrimCompB = ActorB->FindComponentByClass<UPrimitiveComponent>();
    
    if (PrimCompA && PrimCompB)
    {
        FVector VelocityA = PrimCompA->GetPhysicsLinearVelocity();
        FVector VelocityB = PrimCompB->GetPhysicsLinearVelocity();
        
        float RelativeVelocity = FVector::DotProduct(VelocityA - VelocityB, CollisionNormal);
        
        if (FMath::Abs(RelativeVelocity) > MinCollisionVelocityThreshold)
        {
            // Trigger collision events
            TriggerPhysicsEvent(ActorA, ECore_PhysicsEventType::Collision);
            TriggerPhysicsEvent(ActorB, ECore_PhysicsEventType::Collision);
            
            UE_LOG(LogTemp, Log, TEXT("Processed collision between %s and %s: RelVel=%.2f"), 
                   *ActorA->GetName(), *ActorB->GetName(), RelativeVelocity);
        }
    }
}

void UCore_RealTimePhysicsManager::SetActorPhysicsProperties(AActor* Actor, float Mass, float LinearDamping, float AngularDamping, bool bEnableGravity)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (FBodyInstance* BodyInstance = PrimComp->GetBodyInstance())
        {
            BodyInstance->SetMassOverride(Mass, true);
            BodyInstance->LinearDamping = LinearDamping;
            BodyInstance->AngularDamping = AngularDamping;
            BodyInstance->bEnableGravity = bEnableGravity;
            
            // Update physics state
            PrimComp->RecreatePhysicsState();
            
            UE_LOG(LogTemp, Log, TEXT("Updated physics properties for %s: Mass=%.2f, LinDamp=%.2f, AngDamp=%.2f"), 
                   *Actor->GetName(), Mass, LinearDamping, AngularDamping);
        }
    }
}

void UCore_RealTimePhysicsManager::ApplyInstantForce(AActor* Actor, const FVector& Force, const FVector& Location, bool bIsImpulse)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            if (bIsImpulse)
            {
                PrimComp->AddImpulseAtLocation(Force, Location);
            }
            else
            {
                PrimComp->AddForceAtLocation(Force, Location);
            }
            
            // Add to active tracking
            if (!ActivePhysicsActors.Contains(Actor))
            {
                ActivePhysicsActors.Add(Actor);
                ActorLastUpdateTime.Add(Actor, GetWorld()->GetTimeSeconds());
            }
            
            UE_LOG(LogTemp, Log, TEXT("Applied %s force to %s: %s at %s"), 
                   bIsImpulse ? TEXT("impulse") : TEXT("continuous"), 
                   *Actor->GetName(), *Force.ToString(), *Location.ToString());
        }
    }
}

void UCore_RealTimePhysicsManager::ApplyInstantTorque(AActor* Actor, const FVector& Torque, bool bIsImpulse)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            if (bIsImpulse)
            {
                PrimComp->AddAngularImpulseInRadians(Torque);
            }
            else
            {
                PrimComp->AddTorqueInRadians(Torque);
            }
            
            // Add to active tracking
            if (!ActivePhysicsActors.Contains(Actor))
            {
                ActivePhysicsActors.Add(Actor);
                ActorLastUpdateTime.Add(Actor, GetWorld()->GetTimeSeconds());
            }
            
            UE_LOG(LogTemp, Log, TEXT("Applied %s torque to %s: %s"), 
                   bIsImpulse ? TEXT("impulse") : TEXT("continuous"), 
                   *Actor->GetName(), *Torque.ToString());
        }
    }
}

void UCore_RealTimePhysicsManager::UpdatePhysicsPerformanceMetrics(float DeltaTime)
{
    PhysicsFrameAccumulator += DeltaTime * 1000.0f; // Convert to milliseconds
    PhysicsFrameCounter++;
    
    // Update average every 10 frames
    if (PhysicsFrameCounter >= 10)
    {
        CurrentPhysicsFrameTime = PhysicsFrameAccumulator / PhysicsFrameCounter;
        PhysicsFrameAccumulator = 0.0f;
        PhysicsFrameCounter = 0;
    }
}

void UCore_RealTimePhysicsManager::ProcessPhysicsActorUpdates()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (AActor* Actor : ActivePhysicsActors)
    {
        if (IsValid(Actor))
        {
            // Update last update time
            ActorLastUpdateTime.FindOrAdd(Actor) = CurrentTime;
            
            // Check if actor is still moving
            if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    FVector Velocity = PrimComp->GetPhysicsLinearVelocity();
                    if (Velocity.SizeSquared() < 1.0f) // Very slow movement
                    {
                        // Actor has stopped moving, can be removed from active list
                        ActorLastUpdateTime.Remove(Actor);
                    }
                }
            }
        }
    }
}

void UCore_RealTimePhysicsManager::CleanupInactivePhysicsActors()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    const float InactiveThreshold = 5.0f; // 5 seconds of inactivity
    
    // Remove actors that haven't been updated recently
    ActivePhysicsActors.RemoveAll([this, CurrentTime, InactiveThreshold](AActor* Actor)
    {
        if (!IsValid(Actor))
        {
            ActorLastUpdateTime.Remove(Actor);
            return true;
        }
        
        float* LastUpdatePtr = ActorLastUpdateTime.Find(Actor);
        if (!LastUpdatePtr || (CurrentTime - *LastUpdatePtr) > InactiveThreshold)
        {
            ActorLastUpdateTime.Remove(Actor);
            return true;
        }
        
        return false;
    });
}

void UCore_RealTimePhysicsManager::TriggerPhysicsEvent(AActor* Actor, ECore_PhysicsEventType EventType)
{
    if (OnPhysicsEventTriggered.IsBound())
    {
        OnPhysicsEventTriggered.Broadcast(Actor, EventType);
    }
}

bool UCore_RealTimePhysicsManager::ShouldProcessPhysicsEvent(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return false;
    }
    
    // Check if we're at the limit of active actors
    if (ActivePhysicsActors.Num() >= MaxActivePhysicsActors && !ActivePhysicsActors.Contains(Actor))
    {
        return false;
    }
    
    // Check if actor has physics component
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    return PrimComp && PrimComp->IsSimulatingPhysics();
}

void UCore_RealTimePhysicsManager::OptimizePhysicsSettings()
{
    // Reduce active actor limit if performance is poor
    if (CurrentPhysicsFrameTime > MaxAllowedPhysicsTime * 1.5f)
    {
        MaxActivePhysicsActors = FMath::Max(10, MaxActivePhysicsActors - 5);
        UE_LOG(LogTemp, Warning, TEXT("Physics performance degraded, reducing max active actors to %d"), MaxActivePhysicsActors);
    }
    
    // Limit active actors immediately
    LimitActivePhysicsActors();
}

void UCore_RealTimePhysicsManager::LimitActivePhysicsActors()
{
    if (ActivePhysicsActors.Num() > MaxActivePhysicsActors)
    {
        // Remove oldest actors first
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        ActivePhysicsActors.Sort([this, CurrentTime](const AActor& A, const AActor& B)
        {
            float TimeA = ActorLastUpdateTime.FindRef(&A);
            float TimeB = ActorLastUpdateTime.FindRef(&B);
            return TimeA < TimeB; // Oldest first
        });
        
        // Remove excess actors
        int32 ActorsToRemove = ActivePhysicsActors.Num() - MaxActivePhysicsActors;
        for (int32 i = 0; i < ActorsToRemove; i++)
        {
            AActor* ActorToRemove = ActivePhysicsActors[i];
            ActorLastUpdateTime.Remove(ActorToRemove);
        }
        
        ActivePhysicsActors.RemoveAt(0, ActorsToRemove);
        
        UE_LOG(LogTemp, Log, TEXT("Limited active physics actors: removed %d actors"), ActorsToRemove);
    }
}