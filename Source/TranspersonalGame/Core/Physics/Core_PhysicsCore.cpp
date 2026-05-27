#include "Core_PhysicsCore.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsSettings.h"

DEFINE_LOG_CATEGORY(LogCorePhysics);

UCore_PhysicsCore::UCore_PhysicsCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Default physics settings for prehistoric world
    PhysicsSettings = FCore_PhysicsSettings();
    Mass = 100.0f;
    bOverrideMass = false;
    Friction = 0.7f;
    Restitution = 0.3f;
    bCanBeDestroyed = true;
    DestructionThreshold = 1000.0f;
    AccumulatedDamage = 0.0f;
    bPhysicsInitialized = false;
    PhysicsComponent = nullptr;
}

void UCore_PhysicsCore::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysics();
    UpdatePhysicsProperties();
    
    UE_LOG(LogCorePhysics, Log, TEXT("Core Physics Component initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_PhysicsCore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bPhysicsInitialized || !PhysicsComponent)
        return;
    
    // Monitor physics state and handle events
    if (PhysicsComponent->IsSimulatingPhysics())
    {
        FVector CurrentVelocity = PhysicsComponent->GetPhysicsLinearVelocity();
        float Speed = CurrentVelocity.Size();
        
        // Check for high-impact collisions
        if (Speed > 500.0f) // Threshold for significant impact
        {
            OnPhysicsImpact.Broadcast(GetOwner(), Speed);
        }
        
        // Check destruction threshold
        if (bCanBeDestroyed && AccumulatedDamage >= DestructionThreshold)
        {
            TriggerDestruction(AccumulatedDamage);
        }
    }
}

void UCore_PhysicsCore::InitializePhysics()
{
    if (!GetOwner())
    {
        UE_LOG(LogCorePhysics, Error, TEXT("Core Physics Component has no owner"));
        return;
    }
    
    // Find the main physics component
    PhysicsComponent = GetOwner()->FindComponentByClass<UPrimitiveComponent>();
    
    if (!PhysicsComponent)
    {
        // Try to find StaticMeshComponent or SkeletalMeshComponent
        PhysicsComponent = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
        if (!PhysicsComponent)
        {
            PhysicsComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        }
    }
    
    if (PhysicsComponent)
    {
        bPhysicsInitialized = true;
        UE_LOG(LogCorePhysics, Log, TEXT("Physics component found: %s"), *PhysicsComponent->GetName());
    }
    else
    {
        UE_LOG(LogCorePhysics, Warning, TEXT("No physics component found for %s"), *GetOwner()->GetName());
    }
}

void UCore_PhysicsCore::UpdatePhysicsProperties()
{
    if (!PhysicsComponent)
        return;
    
    // Set mass
    if (bOverrideMass && PhysicsComponent->GetBodyInstance())
    {
        PhysicsComponent->SetMassOverrideInKg(NAME_None, Mass, true);
    }
    
    // Set material properties
    if (PhysicsComponent->GetBodyInstance())
    {
        // Create or update physics material
        UPhysicalMaterial* PhysMat = PhysicsComponent->GetBodyInstance()->GetSimplePhysicalMaterial();
        if (!PhysMat)
        {
            // Use default material with our settings
            PhysicsComponent->SetPhysMaterialOverride(nullptr);
        }
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics properties updated for %s"), *GetOwner()->GetName());
}

void UCore_PhysicsCore::ApplyForce(const FVector& Force, bool bVelChange)
{
    if (!PhysicsComponent || !PhysicsComponent->IsSimulatingPhysics())
        return;
    
    PhysicsComponent->AddForce(Force, NAME_None, bVelChange);
    UE_LOG(LogCorePhysics, VeryVerbose, TEXT("Applied force %s to %s"), 
           *Force.ToString(), *GetOwner()->GetName());
}

void UCore_PhysicsCore::ApplyImpulse(const FVector& Impulse, bool bVelChange)
{
    if (!PhysicsComponent || !PhysicsComponent->IsSimulatingPhysics())
        return;
    
    PhysicsComponent->AddImpulse(Impulse, NAME_None, bVelChange);
    UE_LOG(LogCorePhysics, VeryVerbose, TEXT("Applied impulse %s to %s"), 
           *Impulse.ToString(), *GetOwner()->GetName());
}

void UCore_PhysicsCore::SetPhysicsEnabled(bool bEnabled)
{
    if (!PhysicsComponent)
        return;
    
    PhysicsComponent->SetSimulatePhysics(bEnabled);
    UE_LOG(LogCorePhysics, Log, TEXT("Physics %s for %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"), *GetOwner()->GetName());
}

void UCore_PhysicsCore::SetMass(float NewMass)
{
    Mass = FMath::Max(0.1f, NewMass); // Minimum mass to prevent physics issues
    bOverrideMass = true;
    
    if (PhysicsComponent && PhysicsComponent->GetBodyInstance())
    {
        PhysicsComponent->SetMassOverrideInKg(NAME_None, Mass, true);
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Mass set to %f for %s"), Mass, *GetOwner()->GetName());
}

float UCore_PhysicsCore::GetMass() const
{
    if (PhysicsComponent && PhysicsComponent->GetBodyInstance())
    {
        return PhysicsComponent->GetMass();
    }
    return Mass;
}

FVector UCore_PhysicsCore::GetVelocity() const
{
    if (PhysicsComponent && PhysicsComponent->IsSimulatingPhysics())
    {
        return PhysicsComponent->GetPhysicsLinearVelocity();
    }
    return FVector::ZeroVector;
}

void UCore_PhysicsCore::SetVelocity(const FVector& NewVelocity)
{
    if (!PhysicsComponent || !PhysicsComponent->IsSimulatingPhysics())
        return;
    
    PhysicsComponent->SetPhysicsLinearVelocity(NewVelocity);
    UE_LOG(LogCorePhysics, VeryVerbose, TEXT("Velocity set to %s for %s"), 
           *NewVelocity.ToString(), *GetOwner()->GetName());
}

void UCore_PhysicsCore::SetCollisionEnabled(bool bEnabled)
{
    if (!PhysicsComponent)
        return;
    
    ECollisionEnabled::Type CollisionType = bEnabled ? 
        ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision;
    
    PhysicsComponent->SetCollisionEnabled(CollisionType);
    UE_LOG(LogCorePhysics, Log, TEXT("Collision %s for %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"), *GetOwner()->GetName());
}

void UCore_PhysicsCore::SetCollisionResponseToChannel(ECollisionChannel Channel, ECollisionResponse Response)
{
    if (!PhysicsComponent)
        return;
    
    PhysicsComponent->SetCollisionResponseToChannel(Channel, Response);
    UE_LOG(LogCorePhysics, Log, TEXT("Collision response set for channel %d on %s"), 
           (int32)Channel, *GetOwner()->GetName());
}

void UCore_PhysicsCore::TriggerDestruction(float ImpactForce)
{
    if (!bCanBeDestroyed)
        return;
    
    UE_LOG(LogCorePhysics, Log, TEXT("Triggering destruction for %s with impact force %f"), 
           *GetOwner()->GetName(), ImpactForce);
    
    // Broadcast destruction event
    OnPhysicsDestruction.Broadcast(GetOwner());
    
    // Disable physics and collision
    SetPhysicsEnabled(false);
    SetCollisionEnabled(false);
    
    // Mark for destruction (let the destruction system handle the actual destruction)
    bCanBeDestroyed = false;
}

void UCore_PhysicsCore::HandleCollision(const FHitResult& Hit, float ImpactForce)
{
    if (ImpactForce > 100.0f) // Minimum force to register damage
    {
        AccumulatedDamage += ImpactForce * 0.1f; // Scale factor for damage accumulation
        
        UE_LOG(LogCorePhysics, VeryVerbose, TEXT("Collision damage: %f, Total: %f for %s"), 
               ImpactForce * 0.1f, AccumulatedDamage, *GetOwner()->GetName());
        
        // Broadcast impact event
        OnPhysicsImpact.Broadcast(GetOwner(), ImpactForce);
    }
}