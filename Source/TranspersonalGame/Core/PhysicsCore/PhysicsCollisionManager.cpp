#include "PhysicsCollisionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogPhysicsCollision);

UPhysicsCollisionManager::UPhysicsCollisionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default values
    MaxCollisionEvents = 100;
    CollisionEventLifetime = 5.0f;
    bEnableCollisionFiltering = true;
    bEnableCollisionOptimization = true;
    bDebugCollisions = false;
    
    CollisionDistanceThreshold = 1000.0f;
    MinCollisionImpulse = 10.0f;
    MaxCollisionImpulse = 10000.0f;
    
    // Initialize arrays
    CollisionEvents.Reserve(MaxCollisionEvents);
    FilteredCollisionPairs.Reserve(50);
    
    // Set default collision channels
    DefaultCollisionChannel = ECC_WorldStatic;
    CharacterCollisionChannel = ECC_Pawn;
    ProjectileCollisionChannel = ECC_WorldDynamic;
}

void UPhysicsCollisionManager::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        UE_LOG(LogPhysicsCollision, Error, TEXT("PhysicsCollisionManager: No owner actor found"));
        return;
    }
    
    // Initialize collision settings
    InitializeCollisionSettings();
    
    // Bind collision events
    BindCollisionEvents();
    
    UE_LOG(LogPhysicsCollision, Log, TEXT("PhysicsCollisionManager initialized for %s"), *OwnerActor->GetName());
}

void UPhysicsCollisionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update collision events
    UpdateCollisionEvents(DeltaTime);
    
    // Process collision filtering
    if (bEnableCollisionFiltering)
    {
        ProcessCollisionFiltering(DeltaTime);
    }
    
    // Debug collision visualization
    if (bDebugCollisions)
    {
        DebugDrawCollisions();
    }
}

void UPhysicsCollisionManager::RegisterCollisionEvent(const FHitResult& HitResult, const FVector& ImpactVelocity, float ImpactForce)
{
    if (!HitResult.bBlockingHit)
    {
        return;
    }
    
    // Check if impact force meets threshold
    if (ImpactForce < MinCollisionImpulse)
    {
        return;
    }
    
    // Create collision event
    FCore_CollisionEvent NewEvent;
    NewEvent.HitResult = HitResult;
    NewEvent.ImpactVelocity = ImpactVelocity;
    NewEvent.ImpactForce = FMath::Clamp(ImpactForce, MinCollisionImpulse, MaxCollisionImpulse);
    NewEvent.Timestamp = GetWorld()->GetTimeSeconds();
    NewEvent.EventID = FGuid::NewGuid();
    
    // Determine collision type
    NewEvent.CollisionType = DetermineCollisionType(HitResult);
    
    // Add to events list
    if (CollisionEvents.Num() >= MaxCollisionEvents)
    {
        CollisionEvents.RemoveAt(0); // Remove oldest event
    }
    CollisionEvents.Add(NewEvent);
    
    // Process the collision
    ProcessCollisionEvent(NewEvent);
    
    UE_LOG(LogPhysicsCollision, Log, TEXT("Registered collision event: Force=%.2f, Type=%s"), 
           ImpactForce, *UEnum::GetValueAsString(NewEvent.CollisionType));
}

void UPhysicsCollisionManager::SetCollisionChannel(UPrimitiveComponent* Component, ECollisionChannel Channel)
{
    if (!Component)
    {
        UE_LOG(LogPhysicsCollision, Warning, TEXT("SetCollisionChannel: Component is null"));
        return;
    }
    
    Component->SetCollisionObjectType(Channel);
    
    UE_LOG(LogPhysicsCollision, Log, TEXT("Set collision channel for %s to %s"), 
           *Component->GetName(), *UEnum::GetValueAsString(Channel));
}

void UPhysicsCollisionManager::SetCollisionResponse(UPrimitiveComponent* Component, ECollisionChannel Channel, ECollisionResponse Response)
{
    if (!Component)
    {
        UE_LOG(LogPhysicsCollision, Warning, TEXT("SetCollisionResponse: Component is null"));
        return;
    }
    
    Component->SetCollisionResponseToChannel(Channel, Response);
    
    UE_LOG(LogPhysicsCollision, Log, TEXT("Set collision response for %s to channel %s: %s"), 
           *Component->GetName(), *UEnum::GetValueAsString(Channel), *UEnum::GetValueAsString(Response));
}

void UPhysicsCollisionManager::AddCollisionFilter(AActor* Actor1, AActor* Actor2, bool bIgnoreCollision)
{
    if (!Actor1 || !Actor2)
    {
        UE_LOG(LogPhysicsCollision, Warning, TEXT("AddCollisionFilter: One or both actors are null"));
        return;
    }
    
    FCore_CollisionFilter Filter;
    Filter.Actor1 = Actor1;
    Filter.Actor2 = Actor2;
    Filter.bIgnoreCollision = bIgnoreCollision;
    Filter.FilterID = FGuid::NewGuid();
    
    FilteredCollisionPairs.Add(Filter);
    
    // Apply the filter immediately
    ApplyCollisionFilter(Filter);
    
    UE_LOG(LogPhysicsCollision, Log, TEXT("Added collision filter between %s and %s (Ignore: %s)"), 
           *Actor1->GetName(), *Actor2->GetName(), bIgnoreCollision ? TEXT("Yes") : TEXT("No"));
}

void UPhysicsCollisionManager::RemoveCollisionFilter(AActor* Actor1, AActor* Actor2)
{
    if (!Actor1 || !Actor2)
    {
        return;
    }
    
    FilteredCollisionPairs.RemoveAll([Actor1, Actor2](const FCore_CollisionFilter& Filter)
    {
        return (Filter.Actor1 == Actor1 && Filter.Actor2 == Actor2) ||
               (Filter.Actor1 == Actor2 && Filter.Actor2 == Actor1);
    });
    
    UE_LOG(LogPhysicsCollision, Log, TEXT("Removed collision filter between %s and %s"), 
           *Actor1->GetName(), *Actor2->GetName());
}

bool UPhysicsCollisionManager::ShouldIgnoreCollision(AActor* Actor1, AActor* Actor2) const
{
    if (!Actor1 || !Actor2)
    {
        return false;
    }
    
    // Check filtered pairs
    for (const FCore_CollisionFilter& Filter : FilteredCollisionPairs)
    {
        if (((Filter.Actor1 == Actor1 && Filter.Actor2 == Actor2) ||
             (Filter.Actor1 == Actor2 && Filter.Actor2 == Actor1)) &&
            Filter.bIgnoreCollision)
        {
            return true;
        }
    }
    
    return false;
}

TArray<FCore_CollisionEvent> UPhysicsCollisionManager::GetRecentCollisionEvents(float TimeWindow) const
{
    TArray<FCore_CollisionEvent> RecentEvents;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (const FCore_CollisionEvent& Event : CollisionEvents)
    {
        if (CurrentTime - Event.Timestamp <= TimeWindow)
        {
            RecentEvents.Add(Event);
        }
    }
    
    return RecentEvents;
}

void UPhysicsCollisionManager::ClearCollisionEvents()
{
    CollisionEvents.Empty();
    UE_LOG(LogPhysicsCollision, Log, TEXT("Cleared all collision events"));
}

void UPhysicsCollisionManager::InitializeCollisionSettings()
{
    if (!OwnerActor)
    {
        return;
    }
    
    // Get all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    OwnerActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            // Set default collision settings
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetCollisionObjectType(DefaultCollisionChannel);
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            
            // Enable collision events
            Component->SetNotifyRigidBodyCollision(true);
        }
    }
    
    UE_LOG(LogPhysicsCollision, Log, TEXT("Initialized collision settings for %d components"), PrimitiveComponents.Num());
}

void UPhysicsCollisionManager::BindCollisionEvents()
{
    if (!OwnerActor)
    {
        return;
    }
    
    // Get all primitive components and bind hit events
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    OwnerActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            Component->OnComponentHit.AddDynamic(this, &UPhysicsCollisionManager::OnComponentHit);
        }
    }
    
    UE_LOG(LogPhysicsCollision, Log, TEXT("Bound collision events for %d components"), PrimitiveComponents.Num());
}

void UPhysicsCollisionManager::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HitComp || !OtherActor || !OtherComp)
    {
        return;
    }
    
    // Check if collision should be ignored
    if (ShouldIgnoreCollision(OwnerActor, OtherActor))
    {
        return;
    }
    
    // Calculate impact force
    float ImpactForce = NormalImpulse.Size();
    
    // Get impact velocity
    FVector ImpactVelocity = FVector::ZeroVector;
    if (HitComp->IsSimulatingPhysics())
    {
        ImpactVelocity = HitComp->GetPhysicsLinearVelocity();
    }
    
    // Register the collision event
    RegisterCollisionEvent(Hit, ImpactVelocity, ImpactForce);
    
    // Broadcast collision event
    OnCollisionDetected.Broadcast(Hit, ImpactVelocity, ImpactForce);
}

ECore_CollisionType UPhysicsCollisionManager::DetermineCollisionType(const FHitResult& HitResult) const
{
    if (!HitResult.GetActor())
    {
        return ECore_CollisionType::Environment;
    }
    
    AActor* HitActor = HitResult.GetActor();
    
    // Check if it's a character
    if (HitActor->IsA<APawn>())
    {
        return ECore_CollisionType::Character;
    }
    
    // Check if it's a projectile (simple heuristic based on velocity)
    UPrimitiveComponent* HitComponent = HitResult.GetComponent();
    if (HitComponent && HitComponent->IsSimulatingPhysics())
    {
        FVector Velocity = HitComponent->GetPhysicsLinearVelocity();
        if (Velocity.Size() > 500.0f) // Fast moving objects are likely projectiles
        {
            return ECore_CollisionType::Projectile;
        }
        
        return ECore_CollisionType::Physics;
    }
    
    return ECore_CollisionType::Environment;
}

void UPhysicsCollisionManager::ProcessCollisionEvent(const FCore_CollisionEvent& Event)
{
    // Apply collision effects based on type and force
    switch (Event.CollisionType)
    {
        case ECore_CollisionType::Character:
            ProcessCharacterCollision(Event);
            break;
            
        case ECore_CollisionType::Physics:
            ProcessPhysicsCollision(Event);
            break;
            
        case ECore_CollisionType::Projectile:
            ProcessProjectileCollision(Event);
            break;
            
        case ECore_CollisionType::Environment:
            ProcessEnvironmentCollision(Event);
            break;
    }
}

void UPhysicsCollisionManager::ProcessCharacterCollision(const FCore_CollisionEvent& Event)
{
    // Handle character-specific collision logic
    if (Event.ImpactForce > 500.0f)
    {
        UE_LOG(LogPhysicsCollision, Log, TEXT("High-impact character collision detected: Force=%.2f"), Event.ImpactForce);
        
        // Could trigger ragdoll, damage, or other character-specific effects
        OnHighImpactCollision.Broadcast(Event.HitResult, Event.ImpactVelocity, Event.ImpactForce);
    }
}

void UPhysicsCollisionManager::ProcessPhysicsCollision(const FCore_CollisionEvent& Event)
{
    // Handle physics object collision
    UE_LOG(LogPhysicsCollision, Verbose, TEXT("Physics collision: Force=%.2f"), Event.ImpactForce);
}

void UPhysicsCollisionManager::ProcessProjectileCollision(const FCore_CollisionEvent& Event)
{
    // Handle projectile collision
    UE_LOG(LogPhysicsCollision, Log, TEXT("Projectile collision detected: Force=%.2f"), Event.ImpactForce);
    
    // Could trigger destruction, damage, or particle effects
    OnProjectileCollision.Broadcast(Event.HitResult, Event.ImpactVelocity, Event.ImpactForce);
}

void UPhysicsCollisionManager::ProcessEnvironmentCollision(const FCore_CollisionEvent& Event)
{
    // Handle environment collision
    if (Event.ImpactForce > 1000.0f)
    {
        UE_LOG(LogPhysicsCollision, Log, TEXT("High-impact environment collision: Force=%.2f"), Event.ImpactForce);
    }
}

void UPhysicsCollisionManager::UpdateCollisionEvents(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old events
    CollisionEvents.RemoveAll([CurrentTime, this](const FCore_CollisionEvent& Event)
    {
        return CurrentTime - Event.Timestamp > CollisionEventLifetime;
    });
}

void UPhysicsCollisionManager::ProcessCollisionFiltering(float DeltaTime)
{
    // Remove invalid filters
    FilteredCollisionPairs.RemoveAll([](const FCore_CollisionFilter& Filter)
    {
        return !Filter.Actor1 || !Filter.Actor2 || 
               !IsValid(Filter.Actor1) || !IsValid(Filter.Actor2);
    });
}

void UPhysicsCollisionManager::ApplyCollisionFilter(const FCore_CollisionFilter& Filter)
{
    if (!Filter.Actor1 || !Filter.Actor2)
    {
        return;
    }
    
    // Get primitive components from both actors
    TArray<UPrimitiveComponent*> Components1, Components2;
    Filter.Actor1->GetComponents<UPrimitiveComponent>(Components1);
    Filter.Actor2->GetComponents<UPrimitiveComponent>(Components2);
    
    // Apply collision filtering between all component pairs
    for (UPrimitiveComponent* Comp1 : Components1)
    {
        for (UPrimitiveComponent* Comp2 : Components2)
        {
            if (Comp1 && Comp2)
            {
                if (Filter.bIgnoreCollision)
                {
                    Comp1->IgnoreComponentWhenMoving(Comp2, true);
                    Comp2->IgnoreComponentWhenMoving(Comp1, true);
                }
                else
                {
                    Comp1->IgnoreComponentWhenMoving(Comp2, false);
                    Comp2->IgnoreComponentWhenMoving(Comp1, false);
                }
            }
        }
    }
}

void UPhysicsCollisionManager::DebugDrawCollisions()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Draw recent collision events
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (const FCore_CollisionEvent& Event : CollisionEvents)
    {
        if (CurrentTime - Event.Timestamp <= 1.0f) // Show events from last second
        {
            FColor DebugColor = FColor::Red;
            
            switch (Event.CollisionType)
            {
                case ECore_CollisionType::Character:
                    DebugColor = FColor::Blue;
                    break;
                case ECore_CollisionType::Physics:
                    DebugColor = FColor::Green;
                    break;
                case ECore_CollisionType::Projectile:
                    DebugColor = FColor::Yellow;
                    break;
                case ECore_CollisionType::Environment:
                    DebugColor = FColor::Red;
                    break;
            }
            
            // Draw impact point
            DrawDebugSphere(GetWorld(), Event.HitResult.ImpactPoint, 10.0f, 12, DebugColor, false, 0.1f);
            
            // Draw impact normal
            DrawDebugLine(GetWorld(), Event.HitResult.ImpactPoint, 
                         Event.HitResult.ImpactPoint + Event.HitResult.ImpactNormal * 50.0f, 
                         DebugColor, false, 0.1f, 0, 2.0f);
        }
    }
}