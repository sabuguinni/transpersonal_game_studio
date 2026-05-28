#include "Core_CollisionManager.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UCore_CollisionManager::UCore_CollisionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default surface response multipliers
    SurfaceResponseMultipliers.Add(SurfaceType_Default, 1.0f);
    SurfaceResponseMultipliers.Add(SurfaceType1, 0.8f); // Soft surfaces
    SurfaceResponseMultipliers.Add(SurfaceType2, 1.5f); // Hard surfaces
    SurfaceResponseMultipliers.Add(SurfaceType3, 0.3f); // Water/mud
    SurfaceResponseMultipliers.Add(SurfaceType4, 2.0f); // Rock/stone
}

void UCore_CollisionManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Register this component for collision tracking
    if (GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CollisionManager initialized for actor: %s"), 
               *GetOwner()->GetName());
    }
}

void UCore_CollisionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update collision tracking
    LastCollisionTime += DeltaTime;
}

bool UCore_CollisionManager::PerformAdvancedCollisionCheck(AActor* ActorA, AActor* ActorB, FHitResult& OutHitResult)
{
    if (!ActorA || !ActorB)
    {
        return false;
    }

    // Get primitive components for collision
    UPrimitiveComponent* PrimA = ActorA->FindComponentByClass<UPrimitiveComponent>();
    UPrimitiveComponent* PrimB = ActorB->FindComponentByClass<UPrimitiveComponent>();
    
    if (!PrimA || !PrimB)
    {
        return false;
    }

    // Perform line trace between actors
    FVector StartLocation = ActorA->GetActorLocation();
    FVector EndLocation = ActorB->GetActorLocation();
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(ActorA);
    QueryParams.bTraceComplex = true;
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        OutHitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit && bEnableAdvancedCollisionLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Advanced collision detected between %s and %s"), 
               *ActorA->GetName(), *ActorB->GetName());
    }

    return bHit;
}

void UCore_CollisionManager::HandleDinosaurCollision(AActor* Dinosaur, const FHitResult& HitResult)
{
    if (!Dinosaur)
    {
        return;
    }

    // Calculate impact force based on dinosaur mass and velocity
    UPrimitiveComponent* DinosaurPrimitive = Dinosaur->FindComponentByClass<UPrimitiveComponent>();
    if (DinosaurPrimitive)
    {
        FVector Velocity = DinosaurPrimitive->GetPhysicsLinearVelocity();
        float ImpactForce = Velocity.Size() * DinosaurPrimitive->GetMass() * 0.01f;
        
        if (ImpactForce > MinimumImpactVelocity)
        {
            ProcessCollisionDamage(Dinosaur, ImpactForce);
            ApplyCollisionEffects(HitResult, ImpactForce);
            
            if (bEnableAdvancedCollisionLogging)
            {
                UE_LOG(LogTemp, Warning, TEXT("Dinosaur collision: %s, Impact Force: %f"), 
                       *Dinosaur->GetName(), ImpactForce);
            }
        }
    }
}

void UCore_CollisionManager::HandlePlayerCollision(AActor* Player, const FHitResult& HitResult)
{
    if (!Player)
    {
        return;
    }

    // Player collision handling - typically less destructive than dinosaur collisions
    UPrimitiveComponent* PlayerPrimitive = Player->FindComponentByClass<UPrimitiveComponent>();
    if (PlayerPrimitive)
    {
        FVector Velocity = PlayerPrimitive->GetPhysicsLinearVelocity();
        float ImpactForce = Velocity.Size() * 0.5f; // Players are lighter
        
        if (ImpactForce > MinimumImpactVelocity * 0.5f)
        {
            ApplyCollisionEffects(HitResult, ImpactForce);
            
            if (bEnableAdvancedCollisionLogging)
            {
                UE_LOG(LogTemp, Log, TEXT("Player collision: %s, Impact Force: %f"), 
                       *Player->GetName(), ImpactForce);
            }
        }
    }
}

void UCore_CollisionManager::HandleEnvironmentCollision(AActor* EnvironmentActor, const FHitResult& HitResult)
{
    if (!EnvironmentActor)
    {
        return;
    }

    // Environment collision - handle destructible objects
    if (EnvironmentActor->GetName().Contains(TEXT("Rock")) || 
        EnvironmentActor->GetName().Contains(TEXT("Tree")) ||
        EnvironmentActor->GetName().Contains(TEXT("Boulder")))
    {
        float ImpactForce = 500.0f; // Base environmental impact
        ProcessCollisionDamage(EnvironmentActor, ImpactForce);
        
        if (bEnableAdvancedCollisionLogging)
        {
            UE_LOG(LogTemp, Log, TEXT("Environment collision: %s"), 
                   *EnvironmentActor->GetName());
        }
    }
}

void UCore_CollisionManager::ProcessCollisionDamage(AActor* Actor, float ImpactForce)
{
    if (!Actor)
    {
        return;
    }

    // Apply damage based on impact force and collision damage multiplier
    float FinalDamage = ImpactForce * CollisionDamageMultiplier;
    
    // TODO: Integrate with health/damage system when available
    if (bEnableAdvancedCollisionLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Collision damage applied to %s: %f"), 
               *Actor->GetName(), FinalDamage);
    }
}

void UCore_CollisionManager::ApplyCollisionEffects(const FHitResult& HitResult, float ImpactForce)
{
    // Apply surface-specific collision effects
    EPhysicalSurface SurfaceType = HitResult.PhysMaterial.IsValid() ? 
        HitResult.PhysMaterial->SurfaceType : SurfaceType_Default;
    
    float* SurfaceMultiplier = SurfaceResponseMultipliers.Find(SurfaceType);
    float EffectIntensity = ImpactForce * (SurfaceMultiplier ? *SurfaceMultiplier : 1.0f);
    
    // TODO: Spawn particle effects, sound effects based on surface type and impact force
    if (bEnableAdvancedCollisionLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Collision effects applied at location %s, Intensity: %f"), 
               *HitResult.Location.ToString(), EffectIntensity);
    }
}