#include "Core_CollisionManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UCore_CollisionManager::UCore_CollisionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize collision settings
    bEnableCollisionDetection = true;
    bEnableAdvancedCollision = true;
    bLogCollisionEvents = false;
    
    // Performance settings
    MaxCollisionChecksPerFrame = 100;
    CollisionCheckRadius = 5000.0f; // 50 meters
    MinCollisionImpulse = 50.0f;
    
    // Initialize counters
    CurrentFrameCollisionChecks = 0;
    TotalCollisionEvents = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Initialized with radius %.2f"), CollisionCheckRadius);
}

void UCore_CollisionManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Setup collision response channels
    SetupCollisionChannels();
    
    // Register collision delegates
    RegisterCollisionDelegates();
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: BeginPlay - Collision system active"));
}

void UCore_CollisionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Reset frame counters
    CurrentFrameCollisionChecks = 0;
    
    // Update collision detection if enabled
    if (bEnableCollisionDetection)
    {
        UpdateCollisionDetection(DeltaTime);
    }
    
    // Clean up old collision events
    CleanupCollisionEvents();
}

void UCore_CollisionManager::SetupCollisionChannels()
{
    // Define custom collision channels for prehistoric game
    // These would normally be set in Project Settings, but we can configure responses here
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Setting up collision channels"));
    
    // Player collision setup
    PlayerCollisionProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    PlayerCollisionProfile.ObjectType = ECollisionChannel::ECC_Pawn;
    PlayerCollisionProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
    PlayerCollisionProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
    PlayerCollisionProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
    
    // Dinosaur collision setup
    DinosaurCollisionProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    DinosaurCollisionProfile.ObjectType = ECollisionChannel::ECC_Pawn;
    DinosaurCollisionProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
    DinosaurCollisionProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
    DinosaurCollisionProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
    
    // Environment collision setup
    EnvironmentCollisionProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    EnvironmentCollisionProfile.ObjectType = ECollisionChannel::ECC_WorldStatic;
    EnvironmentCollisionProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
    EnvironmentCollisionProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
}

void UCore_CollisionManager::RegisterCollisionDelegates()
{
    if (UWorld* World = GetWorld())
    {
        // Register for collision events from all actors
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor) continue;
            
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp)
                {
                    PrimComp->OnComponentHit.AddDynamic(this, &UCore_CollisionManager::OnActorHit);
                    PrimComp->OnComponentBeginOverlap.AddDynamic(this, &UCore_CollisionManager::OnActorBeginOverlap);
                    PrimComp->OnComponentEndOverlap.AddDynamic(this, &UCore_CollisionManager::OnActorEndOverlap);
                }
            }
        }
    }
}

void UCore_CollisionManager::UpdateCollisionDetection(float DeltaTime)
{
    if (!bEnableAdvancedCollision || CurrentFrameCollisionChecks >= MaxCollisionChecksPerFrame)
    {
        return;
    }
    
    // Get player location for proximity checks
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Check collisions for actors near the player
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (CurrentFrameCollisionChecks >= MaxCollisionChecksPerFrame)
            {
                break;
            }
            
            AActor* Actor = *ActorItr;
            if (!Actor || Actor == PlayerPawn) continue;
            
            FVector ActorLocation = Actor->GetActorLocation();
            float DistanceToPlayer = FVector::Dist(ActorLocation, PlayerLocation);
            
            // Only check collisions for nearby actors
            if (DistanceToPlayer <= CollisionCheckRadius)
            {
                CheckActorCollision(Actor, DeltaTime);
                CurrentFrameCollisionChecks++;
            }
        }
    }
}

void UCore_CollisionManager::CheckActorCollision(AActor* Actor, float DeltaTime)
{
    if (!Actor) return;
    
    // Get the actor's primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (!PrimComp || !PrimComp->IsCollisionEnabled()) continue;
        
        // Check for overlapping actors
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Actor);
        
        bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
            OverlapResults,
            PrimComp->GetComponentLocation(),
            PrimComp->GetComponentRotation().Quaternion(),
            ECollisionChannel::ECC_Pawn,
            PrimComp->GetCollisionShape(),
            QueryParams
        );
        
        if (bHasOverlap)
        {
            for (const FOverlapResult& Overlap : OverlapResults)
            {
                if (Overlap.GetActor())
                {
                    ProcessCollisionEvent(Actor, Overlap.GetActor(), PrimComp, Overlap.GetComponent());
                }
            }
        }
    }
}

void UCore_CollisionManager::ProcessCollisionEvent(AActor* ActorA, AActor* ActorB, UPrimitiveComponent* ComponentA, UPrimitiveComponent* ComponentB)
{
    if (!ActorA || !ActorB || ActorA == ActorB) return;
    
    // Create collision event data
    FCore_CollisionEvent CollisionEvent;
    CollisionEvent.ActorA = ActorA;
    CollisionEvent.ActorB = ActorB;
    CollisionEvent.ComponentA = ComponentA;
    CollisionEvent.ComponentB = ComponentB;
    CollisionEvent.CollisionTime = GetWorld()->GetTimeSeconds();
    CollisionEvent.CollisionLocation = ComponentA ? ComponentA->GetComponentLocation() : ActorA->GetActorLocation();
    
    // Calculate impact force if both components are simulating physics
    if (ComponentA && ComponentB && ComponentA->IsSimulatingPhysics() && ComponentB->IsSimulatingPhysics())
    {
        FVector VelocityA = ComponentA->GetPhysicsLinearVelocity();
        FVector VelocityB = ComponentB->GetPhysicsLinearVelocity();
        FVector RelativeVelocity = VelocityA - VelocityB;
        
        float Mass = ComponentA->GetMass() + ComponentB->GetMass();
        CollisionEvent.ImpactForce = RelativeVelocity.Size() * Mass;
    }
    else
    {
        CollisionEvent.ImpactForce = 0.0f;
    }
    
    // Store the collision event
    RecentCollisionEvents.Add(CollisionEvent);
    TotalCollisionEvents++;
    
    // Log significant collisions
    if (bLogCollisionEvents && CollisionEvent.ImpactForce > MinCollisionImpulse)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Collision between %s and %s, Impact: %.2f"), 
            *ActorA->GetName(), *ActorB->GetName(), CollisionEvent.ImpactForce);
    }
    
    // Trigger collision response
    HandleCollisionResponse(CollisionEvent);
}

void UCore_CollisionManager::HandleCollisionResponse(const FCore_CollisionEvent& CollisionEvent)
{
    // Handle different types of collision responses
    
    // Player-Dinosaur collision
    if (IsPlayerActor(CollisionEvent.ActorA) && IsDinosaurActor(CollisionEvent.ActorB))
    {
        HandlePlayerDinosaurCollision(CollisionEvent.ActorA, CollisionEvent.ActorB, CollisionEvent.ImpactForce);
    }
    else if (IsPlayerActor(CollisionEvent.ActorB) && IsDinosaurActor(CollisionEvent.ActorA))
    {
        HandlePlayerDinosaurCollision(CollisionEvent.ActorB, CollisionEvent.ActorA, CollisionEvent.ImpactForce);
    }
    
    // Dinosaur-Environment collision
    else if (IsDinosaurActor(CollisionEvent.ActorA) && IsEnvironmentActor(CollisionEvent.ActorB))
    {
        HandleDinosaurEnvironmentCollision(CollisionEvent.ActorA, CollisionEvent.ActorB, CollisionEvent.ImpactForce);
    }
    else if (IsDinosaurActor(CollisionEvent.ActorB) && IsEnvironmentActor(CollisionEvent.ActorA))
    {
        HandleDinosaurEnvironmentCollision(CollisionEvent.ActorB, CollisionEvent.ActorA, CollisionEvent.ImpactForce);
    }
    
    // High-impact collisions (damage/destruction)
    if (CollisionEvent.ImpactForce > MinCollisionImpulse * 5.0f)
    {
        HandleHighImpactCollision(CollisionEvent);
    }
}

void UCore_CollisionManager::HandlePlayerDinosaurCollision(AActor* Player, AActor* Dinosaur, float ImpactForce)
{
    if (!Player || !Dinosaur) return;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_CollisionManager: Player-Dinosaur collision! Impact: %.2f"), ImpactForce);
    
    // Apply damage to player based on impact force
    if (ImpactForce > MinCollisionImpulse)
    {
        float Damage = FMath::Clamp(ImpactForce / 100.0f, 1.0f, 50.0f);
        
        // TODO: Apply damage to player character
        // This would integrate with the character's health system
        
        UE_LOG(LogTemp, Warning, TEXT("Core_CollisionManager: Player takes %.2f damage from dinosaur collision"), Damage);
    }
}

void UCore_CollisionManager::HandleDinosaurEnvironmentCollision(AActor* Dinosaur, AActor* Environment, float ImpactForce)
{
    if (!Dinosaur || !Environment) return;
    
    // Handle dinosaur collision with environment (trees, rocks, etc.)
    if (ImpactForce > MinCollisionImpulse * 2.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Dinosaur %s collided with environment %s, Impact: %.2f"), 
            *Dinosaur->GetName(), *Environment->GetName(), ImpactForce);
        
        // TODO: Trigger environment destruction or dinosaur stagger
    }
}

void UCore_CollisionManager::HandleHighImpactCollision(const FCore_CollisionEvent& CollisionEvent)
{
    UE_LOG(LogTemp, Warning, TEXT("Core_CollisionManager: High impact collision! Force: %.2f"), CollisionEvent.ImpactForce);
    
    // TODO: Trigger destruction effects, screen shake, sound effects
    // This would integrate with the destruction system
}

void UCore_CollisionManager::CleanupCollisionEvents()
{
    // Remove collision events older than 5 seconds
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float MaxAge = 5.0f;
    
    RecentCollisionEvents.RemoveAll([CurrentTime, MaxAge](const FCore_CollisionEvent& Event)
    {
        return (CurrentTime - Event.CollisionTime) > MaxAge;
    });
}

bool UCore_CollisionManager::IsPlayerActor(AActor* Actor) const
{
    return Actor && (Actor->IsA<ACharacter>() || Actor->GetName().Contains("Character"));
}

bool UCore_CollisionManager::IsDinosaurActor(AActor* Actor) const
{
    return Actor && (Actor->GetName().Contains("Dinosaur") || Actor->GetName().Contains("TRex") || 
                     Actor->GetName().Contains("Raptor") || Actor->GetName().Contains("Brachi"));
}

bool UCore_CollisionManager::IsEnvironmentActor(AActor* Actor) const
{
    return Actor && (Actor->GetName().Contains("Tree") || Actor->GetName().Contains("Rock") || 
                     Actor->GetName().Contains("Landscape") || Actor->GetName().Contains("Environment"));
}

void UCore_CollisionManager::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HitComponent || !OtherActor) return;
    
    AActor* ThisActor = HitComponent->GetOwner();
    if (!ThisActor) return;
    
    // Calculate impact force from normal impulse
    float ImpactForce = NormalImpulse.Size();
    
    // Process the hit as a collision event
    ProcessCollisionEvent(ThisActor, OtherActor, HitComponent, OtherComponent);
    
    if (bLogCollisionEvents && ImpactForce > MinCollisionImpulse)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Hit event - %s hit %s, Force: %.2f"), 
            *ThisActor->GetName(), *OtherActor->GetName(), ImpactForce);
    }
}

void UCore_CollisionManager::OnActorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OverlappedComponent || !OtherActor) return;
    
    AActor* ThisActor = OverlappedComponent->GetOwner();
    if (!ThisActor) return;
    
    // Process overlap as a low-force collision
    ProcessCollisionEvent(ThisActor, OtherActor, OverlappedComponent, OtherComponent);
    
    if (bLogCollisionEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Begin overlap - %s overlaps %s"), 
            *ThisActor->GetName(), *OtherActor->GetName());
    }
}

void UCore_CollisionManager::OnActorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (!OverlappedComponent || !OtherActor) return;
    
    if (bLogCollisionEvents)
    {
        AActor* ThisActor = OverlappedComponent->GetOwner();
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: End overlap - %s stops overlapping %s"), 
            ThisActor ? *ThisActor->GetName() : TEXT("Unknown"), *OtherActor->GetName());
    }
}

FCore_CollisionStats UCore_CollisionManager::GetCollisionStatistics() const
{
    FCore_CollisionStats Stats;
    Stats.TotalCollisionEvents = TotalCollisionEvents;
    Stats.RecentCollisionEvents = RecentCollisionEvents.Num();
    Stats.CurrentFrameChecks = CurrentFrameCollisionChecks;
    Stats.MaxChecksPerFrame = MaxCollisionChecksPerFrame;
    Stats.CollisionCheckRadius = CollisionCheckRadius;
    Stats.MinImpulseThreshold = MinCollisionImpulse;
    
    return Stats;
}