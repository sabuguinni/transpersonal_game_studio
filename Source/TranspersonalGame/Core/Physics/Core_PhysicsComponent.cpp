#include "Core_PhysicsComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogCorePhysics);

UCore_PhysicsComponent::UCore_PhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    PhysicsType = ECore_PhysicsType::Dynamic;
    bAutoApplySettings = true;
    PhysicsComponent = nullptr;
    bIsDestroyed = false;
    LastImpactTime = 0.0f;
    ImpactCooldown = 0.1f;
}

void UCore_PhysicsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsComponent();
    
    if (bAutoApplySettings)
    {
        ApplyPhysicsSettings();
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Core Physics Component initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_PhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update impact cooldown
    if (LastImpactTime > 0.0f)
    {
        LastImpactTime -= DeltaTime;
        if (LastImpactTime <= 0.0f)
        {
            LastImpactTime = 0.0f;
        }
    }
}

void UCore_PhysicsComponent::InitializePhysicsComponent()
{
    if (!GetOwner())
    {
        UE_LOG(LogCorePhysics, Warning, TEXT("No owner found for Core Physics Component"));
        return;
    }
    
    // Find the main physics component (StaticMesh or SkeletalMesh)
    PhysicsComponent = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    if (!PhysicsComponent)
    {
        PhysicsComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    }
    
    if (!PhysicsComponent)
    {
        UE_LOG(LogCorePhysics, Warning, TEXT("No physics component found on %s"), *GetOwner()->GetName());
        return;
    }
    
    // Bind collision events
    PhysicsComponent->OnComponentHit.AddDynamic(this, &UCore_PhysicsComponent::OnComponentHit);
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics component initialized: %s"), *PhysicsComponent->GetName());
}

void UCore_PhysicsComponent::ApplyPhysicsSettings()
{
    if (!PhysicsComponent)
    {
        UE_LOG(LogCorePhysics, Warning, TEXT("Cannot apply physics settings - no physics component"));
        return;
    }
    
    // Apply physics simulation
    PhysicsComponent->SetSimulatePhysics(PhysicsSettings.bSimulatePhysics);
    PhysicsComponent->SetEnableGravity(PhysicsSettings.bEnableGravity);
    
    // Apply mass
    if (PhysicsComponent->GetBodyInstance())
    {
        PhysicsComponent->SetMassOverrideInKg(NAME_None, PhysicsSettings.Mass, true);
        PhysicsComponent->SetLinearDamping(PhysicsSettings.LinearDamping);
        PhysicsComponent->SetAngularDamping(PhysicsSettings.AngularDamping);
    }
    
    // Apply material properties
    if (PhysicsComponent->GetBodyInstance())
    {
        PhysicsComponent->SetPhysMaterialOverride(nullptr); // Use default for now
        // Note: In a full implementation, we would create and apply custom physics materials
    }
    
    // Set collision based on physics type
    switch (PhysicsType)
    {
        case ECore_PhysicsType::Static:
            PhysicsComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PhysicsComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
            PhysicsComponent->SetSimulatePhysics(false);
            break;
            
        case ECore_PhysicsType::Dynamic:
            PhysicsComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PhysicsComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            PhysicsComponent->SetSimulatePhysics(true);
            break;
            
        case ECore_PhysicsType::Kinematic:
            PhysicsComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PhysicsComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            PhysicsComponent->SetSimulatePhysics(false);
            break;
            
        case ECore_PhysicsType::Destructible:
            PhysicsComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PhysicsComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            PhysicsComponent->SetSimulatePhysics(true);
            break;
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics settings applied to %s - Type: %d, Mass: %.2f"), 
           *GetOwner()->GetName(), (int32)PhysicsType, PhysicsSettings.Mass);
}

void UCore_PhysicsComponent::AddForceAtLocation(const FVector& Force, const FVector& Location)
{
    if (!PhysicsComponent || !PhysicsComponent->IsSimulatingPhysics())
    {
        return;
    }
    
    PhysicsComponent->AddForceAtLocation(Force, Location);
    UE_LOG(LogCorePhysics, VeryVerbose, TEXT("Force applied: %s at %s"), *Force.ToString(), *Location.ToString());
}

void UCore_PhysicsComponent::AddImpulseAtLocation(const FVector& Impulse, const FVector& Location)
{
    if (!PhysicsComponent || !PhysicsComponent->IsSimulatingPhysics())
    {
        return;
    }
    
    PhysicsComponent->AddImpulseAtLocation(Impulse, Location);
    UE_LOG(LogCorePhysics, VeryVerbose, TEXT("Impulse applied: %s at %s"), *Impulse.ToString(), *Location.ToString());
}

void UCore_PhysicsComponent::SetPhysicsType(ECore_PhysicsType NewType)
{
    PhysicsType = NewType;
    ApplyPhysicsSettings();
}

void UCore_PhysicsComponent::EnablePhysicsSimulation(bool bEnable)
{
    PhysicsSettings.bSimulatePhysics = bEnable;
    if (PhysicsComponent)
    {
        PhysicsComponent->SetSimulatePhysics(bEnable);
    }
}

void UCore_PhysicsComponent::SetMass(float NewMass)
{
    PhysicsSettings.Mass = FMath::Max(0.1f, NewMass);
    if (PhysicsComponent && PhysicsComponent->GetBodyInstance())
    {
        PhysicsComponent->SetMassOverrideInKg(NAME_None, PhysicsSettings.Mass, true);
    }
}

float UCore_PhysicsComponent::GetMass() const
{
    if (PhysicsComponent && PhysicsComponent->GetBodyInstance())
    {
        return PhysicsComponent->GetMass();
    }
    return PhysicsSettings.Mass;
}

FVector UCore_PhysicsComponent::GetVelocity() const
{
    if (PhysicsComponent && PhysicsComponent->IsSimulatingPhysics())
    {
        return PhysicsComponent->GetPhysicsLinearVelocity();
    }
    return FVector::ZeroVector;
}

float UCore_PhysicsComponent::GetSpeed() const
{
    return GetVelocity().Size();
}

void UCore_PhysicsComponent::TriggerDestruction(float ImpactForce)
{
    if (bIsDestroyed || PhysicsType != ECore_PhysicsType::Destructible)
    {
        return;
    }
    
    if (ImpactForce >= PhysicsSettings.DestructionThreshold)
    {
        HandlePhysicsDestruction();
    }
}

void UCore_PhysicsComponent::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!GetOwner() || !OtherActor || LastImpactTime > 0.0f)
    {
        return;
    }
    
    // Calculate impact force
    float ImpactForce = NormalImpulse.Size();
    
    // Broadcast impact event
    OnPhysicsImpact.Broadcast(OtherActor, ImpactForce);
    
    // Check for destruction
    if (ShouldDestroyFromImpact(ImpactForce))
    {
        TriggerDestruction(ImpactForce);
    }
    
    // Set cooldown to prevent spam
    LastImpactTime = ImpactCooldown;
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics impact: %s hit %s with force %.2f"), 
           *GetOwner()->GetName(), *OtherActor->GetName(), ImpactForce);
}

bool UCore_PhysicsComponent::ShouldDestroyFromImpact(float ImpactForce) const
{
    return (PhysicsType == ECore_PhysicsType::Destructible && 
            ImpactForce >= PhysicsSettings.DestructionThreshold &&
            !bIsDestroyed);
}

void UCore_PhysicsComponent::HandlePhysicsDestruction()
{
    if (bIsDestroyed)
    {
        return;
    }
    
    bIsDestroyed = true;
    
    // Broadcast destruction event
    OnPhysicsDestruction.Broadcast(GetOwner());
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics destruction triggered for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    
    // In a full implementation, this would:
    // - Spawn debris particles
    // - Create destruction fragments
    // - Apply forces to nearby objects
    // - Play destruction sound effects
    
    // For now, just disable physics simulation
    if (PhysicsComponent)
    {
        PhysicsComponent->SetSimulatePhysics(false);
        PhysicsComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}