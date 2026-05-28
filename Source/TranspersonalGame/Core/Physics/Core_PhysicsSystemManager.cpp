#include "Core_PhysicsSystemManager.h"
#include "Core/Engine/EngArch_CoreSystemRegistry.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogPhysicsSystem);

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    bSystemInitialized = false;
    bPhysicsEnabled = true;
    GlobalPhysicsScale = 1.0f;
    CachedWorld = nullptr;
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogPhysicsSystem, Log, TEXT("Initializing Core Physics System Manager"));
    
    InitializePhysicsSystem();
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogPhysicsSystem, Log, TEXT("Deinitializing Core Physics System Manager"));
    
    ShutdownPhysicsSystem();
    
    Super::Deinitialize();
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    if (bSystemInitialized)
    {
        UE_LOG(LogPhysicsSystem, Warning, TEXT("Physics system already initialized"));
        return;
    }

    // Cache world reference
    if (UWorld* World = GetWorld())
    {
        CachedWorld = World;
    }

    // Setup physics materials
    SetupPhysicsMaterials();
    
    // Configure physics settings
    ConfigurePhysicsSettings();
    
    // Register with core system registry
    RegisterWithCoreRegistry();
    
    bSystemInitialized = true;
    
    UE_LOG(LogPhysicsSystem, Log, TEXT("Core Physics System initialized successfully"));
}

void UCore_PhysicsSystemManager::ShutdownPhysicsSystem()
{
    if (!bSystemInitialized)
    {
        return;
    }

    // Unbind collision events
    UnbindCollisionEvents();
    
    // Clear registered actors
    RegisteredActors.Empty();
    
    // Clear material caches
    MaterialDensities.Empty();
    MaterialFrictions.Empty();
    MaterialRestitutions.Empty();
    
    CachedWorld = nullptr;
    bSystemInitialized = false;
    
    UE_LOG(LogPhysicsSystem, Log, TEXT("Core Physics System shutdown complete"));
}

void UCore_PhysicsSystemManager::SetupPhysicsMaterials()
{
    // Initialize material density values (kg/m³)
    MaterialDensities.Add(ECore_PhysicsMaterial::Rock, 2500.0f);
    MaterialDensities.Add(ECore_PhysicsMaterial::Wood, 600.0f);
    MaterialDensities.Add(ECore_PhysicsMaterial::Bone, 1900.0f);
    MaterialDensities.Add(ECore_PhysicsMaterial::Flesh, 1000.0f);
    MaterialDensities.Add(ECore_PhysicsMaterial::Water, 1000.0f);
    MaterialDensities.Add(ECore_PhysicsMaterial::Sand, 1600.0f);
    MaterialDensities.Add(ECore_PhysicsMaterial::Mud, 1200.0f);
    MaterialDensities.Add(ECore_PhysicsMaterial::Ice, 917.0f);
    MaterialDensities.Add(ECore_PhysicsMaterial::Metal, 7800.0f);
    MaterialDensities.Add(ECore_PhysicsMaterial::Vegetation, 400.0f);

    // Initialize material friction coefficients
    MaterialFrictions.Add(ECore_PhysicsMaterial::Rock, 0.8f);
    MaterialFrictions.Add(ECore_PhysicsMaterial::Wood, 0.6f);
    MaterialFrictions.Add(ECore_PhysicsMaterial::Bone, 0.7f);
    MaterialFrictions.Add(ECore_PhysicsMaterial::Flesh, 0.4f);
    MaterialFrictions.Add(ECore_PhysicsMaterial::Water, 0.1f);
    MaterialFrictions.Add(ECore_PhysicsMaterial::Sand, 0.5f);
    MaterialFrictions.Add(ECore_PhysicsMaterial::Mud, 0.3f);
    MaterialFrictions.Add(ECore_PhysicsMaterial::Ice, 0.1f);
    MaterialFrictions.Add(ECore_PhysicsMaterial::Metal, 0.9f);
    MaterialFrictions.Add(ECore_PhysicsMaterial::Vegetation, 0.4f);

    // Initialize material restitution (bounciness)
    MaterialRestitutions.Add(ECore_PhysicsMaterial::Rock, 0.3f);
    MaterialRestitutions.Add(ECore_PhysicsMaterial::Wood, 0.2f);
    MaterialRestitutions.Add(ECore_PhysicsMaterial::Bone, 0.4f);
    MaterialRestitutions.Add(ECore_PhysicsMaterial::Flesh, 0.1f);
    MaterialRestitutions.Add(ECore_PhysicsMaterial::Water, 0.0f);
    MaterialRestitutions.Add(ECore_PhysicsMaterial::Sand, 0.1f);
    MaterialRestitutions.Add(ECore_PhysicsMaterial::Mud, 0.05f);
    MaterialRestitutions.Add(ECore_PhysicsMaterial::Ice, 0.2f);
    MaterialRestitutions.Add(ECore_PhysicsMaterial::Metal, 0.6f);
    MaterialRestitutions.Add(ECore_PhysicsMaterial::Vegetation, 0.15f);

    UE_LOG(LogPhysicsSystem, Log, TEXT("Physics materials setup complete"));
}

void UCore_PhysicsSystemManager::ConfigurePhysicsSettings()
{
    // Configure global physics settings for prehistoric world realism
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        // Set realistic gravity (Earth gravity)
        PhysicsSettings->DefaultGravityZ = -980.0f * GlobalPhysicsScale;
        
        // Configure solver settings for stability
        PhysicsSettings->bSubstepping = true;
        PhysicsSettings->bSubsteppingAsync = true;
        PhysicsSettings->MaxSubstepDeltaTime = 0.016667f; // 60 FPS
        PhysicsSettings->MaxSubsteps = 6;
        
        UE_LOG(LogPhysicsSystem, Log, TEXT("Physics settings configured for prehistoric world"));
    }
}

void UCore_PhysicsSystemManager::SetPhysicsMaterial(UStaticMeshComponent* MeshComponent, ECore_PhysicsMaterial MaterialType)
{
    if (!MeshComponent)
    {
        UE_LOG(LogPhysicsSystem, Warning, TEXT("SetPhysicsMaterial: Invalid mesh component"));
        return;
    }

    ApplyMaterialProperties(MeshComponent, MaterialType);
    
    UE_LOG(LogPhysicsSystem, Log, TEXT("Applied physics material %d to static mesh component"), (int32)MaterialType);
}

void UCore_PhysicsSystemManager::SetSkeletalPhysicsMaterial(USkeletalMeshComponent* SkeletalMesh, ECore_PhysicsMaterial MaterialType)
{
    if (!SkeletalMesh)
    {
        UE_LOG(LogPhysicsSystem, Warning, TEXT("SetSkeletalPhysicsMaterial: Invalid skeletal mesh component"));
        return;
    }

    ApplyMaterialProperties(SkeletalMesh, MaterialType);
    
    UE_LOG(LogPhysicsSystem, Log, TEXT("Applied physics material %d to skeletal mesh component"), (int32)MaterialType);
}

void UCore_PhysicsSystemManager::ApplyMaterialProperties(UPrimitiveComponent* Component, ECore_PhysicsMaterial MaterialType)
{
    if (!Component)
    {
        return;
    }

    // Get material properties
    float Density = GetMaterialDensity(MaterialType);
    float Friction = GetMaterialFriction(MaterialType);
    float Restitution = GetMaterialRestitution(MaterialType);

    // Apply to body instance
    FBodyInstance* BodyInstance = Component->GetBodyInstance();
    if (BodyInstance)
    {
        // Set mass based on density (simplified calculation)
        BodyInstance->SetMassOverride(Density * 0.1f); // Scale for gameplay
        BodyInstance->bOverrideMass = true;
        
        // Update physics properties
        Component->SetNotifyRigidBodyCollision(true);
        Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    UE_LOG(LogPhysicsSystem, VeryVerbose, TEXT("Applied material properties: Density=%.2f, Friction=%.2f, Restitution=%.2f"), 
           Density, Friction, Restitution);
}

float UCore_PhysicsSystemManager::GetMaterialDensity(ECore_PhysicsMaterial MaterialType) const
{
    if (const float* Density = MaterialDensities.Find(MaterialType))
    {
        return *Density;
    }
    return 1000.0f; // Default density
}

float UCore_PhysicsSystemManager::GetMaterialFriction(ECore_PhysicsMaterial MaterialType) const
{
    if (const float* Friction = MaterialFrictions.Find(MaterialType))
    {
        return *Friction;
    }
    return 0.5f; // Default friction
}

float UCore_PhysicsSystemManager::GetMaterialRestitution(ECore_PhysicsMaterial MaterialType) const
{
    if (const float* Restitution = MaterialRestitutions.Find(MaterialType))
    {
        return *Restitution;
    }
    return 0.2f; // Default restitution
}

void UCore_PhysicsSystemManager::ApplyForceToActor(AActor* TargetActor, const FVector& Force, const FVector& Location)
{
    if (!TargetActor)
    {
        UE_LOG(LogPhysicsSystem, Warning, TEXT("ApplyForceToActor: Invalid target actor"));
        return;
    }

    if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent()))
    {
        if (Location.IsZero())
        {
            RootPrimitive->AddForce(Force * GlobalPhysicsScale);
        }
        else
        {
            RootPrimitive->AddForceAtLocation(Force * GlobalPhysicsScale, Location);
        }
        
        UE_LOG(LogPhysicsSystem, VeryVerbose, TEXT("Applied force %.2f to actor %s"), 
               Force.Size(), *TargetActor->GetName());
    }
}

void UCore_PhysicsSystemManager::ApplyImpulseToActor(AActor* TargetActor, const FVector& Impulse, const FVector& Location)
{
    if (!TargetActor)
    {
        UE_LOG(LogPhysicsSystem, Warning, TEXT("ApplyImpulseToActor: Invalid target actor"));
        return;
    }

    if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent()))
    {
        if (Location.IsZero())
        {
            RootPrimitive->AddImpulse(Impulse * GlobalPhysicsScale);
        }
        else
        {
            RootPrimitive->AddImpulseAtLocation(Impulse * GlobalPhysicsScale, Location);
        }
        
        UE_LOG(LogPhysicsSystem, VeryVerbose, TEXT("Applied impulse %.2f to actor %s"), 
               Impulse.Size(), *TargetActor->GetName());
    }
}

void UCore_PhysicsSystemManager::ApplyRadialForce(const FVector& Origin, float Radius, float Strength, bool bImpulse)
{
    if (!CachedWorld)
    {
        UE_LOG(LogPhysicsSystem, Warning, TEXT("ApplyRadialForce: No valid world"));
        return;
    }

    TArray<AActor*> ActorsInRadius = GetActorsInRadius(Origin, Radius);
    
    for (AActor* Actor : ActorsInRadius)
    {
        if (!Actor)
        {
            continue;
        }

        FVector Direction = (Actor->GetActorLocation() - Origin).GetSafeNormal();
        float Distance = FVector::Dist(Actor->GetActorLocation(), Origin);
        float ForceMagnitude = Strength * (1.0f - (Distance / Radius)); // Falloff with distance
        
        FVector Force = Direction * ForceMagnitude;
        
        if (bImpulse)
        {
            ApplyImpulseToActor(Actor, Force);
        }
        else
        {
            ApplyForceToActor(Actor, Force);
        }
    }
    
    UE_LOG(LogPhysicsSystem, Log, TEXT("Applied radial force to %d actors"), ActorsInRadius.Num());
}

void UCore_PhysicsSystemManager::RegisterForImpactEvents(AActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(LogPhysicsSystem, Warning, TEXT("RegisterForImpactEvents: Invalid actor"));
        return;
    }

    // Add to registered actors list
    RegisteredActors.AddUnique(Actor);
    
    // Bind hit event
    Actor->OnActorHit.AddDynamic(this, &UCore_PhysicsSystemManager::OnActorHit);
    
    UE_LOG(LogPhysicsSystem, Log, TEXT("Registered actor %s for impact events"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::UnregisterFromImpactEvents(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    // Remove from registered actors list
    RegisteredActors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return WeakActor.Get() == Actor;
    });
    
    // Unbind hit event
    Actor->OnActorHit.RemoveDynamic(this, &UCore_PhysicsSystemManager::OnActorHit);
    
    UE_LOG(LogPhysicsSystem, Log, TEXT("Unregistered actor %s from impact events"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!SelfActor || !OtherActor)
    {
        return;
    }

    // Calculate impact velocity and force
    FVector ImpactVelocity = NormalImpulse;
    float Mass = 1.0f; // Default mass
    
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SelfActor->GetRootComponent()))
    {
        Mass = PrimComp->GetMass();
    }
    
    float ImpactForce = CalculateImpactForce(ImpactVelocity, Mass);
    
    // Process impact event
    ProcessImpactEvent(SelfActor, Hit, ImpactVelocity);
    
    UE_LOG(LogPhysicsSystem, VeryVerbose, TEXT("Impact detected: %s hit %s with force %.2f"), 
           *SelfActor->GetName(), *OtherActor->GetName(), ImpactForce);
}

void UCore_PhysicsSystemManager::ProcessImpactEvent(AActor* Actor, const FHitResult& Hit, const FVector& ImpactVelocity)
{
    if (!Actor)
    {
        return;
    }

    // Create impact data
    FCore_PhysicsImpact ImpactData;
    ImpactData.ImpactActor = Actor;
    ImpactData.ImpactLocation = Hit.ImpactPoint;
    ImpactData.ImpactVelocity = ImpactVelocity;
    ImpactData.MaterialType = DetermineMaterialType(Actor);
    
    // Calculate impact force
    float Mass = 1.0f;
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        Mass = PrimComp->GetMass();
    }
    ImpactData.ImpactForce = CalculateImpactForce(ImpactVelocity, Mass);
    
    // Broadcast impact event
    BroadcastImpactEvent(ImpactData);
}

void UCore_PhysicsSystemManager::BroadcastImpactEvent(const FCore_PhysicsImpact& ImpactData)
{
    OnPhysicsImpact.Broadcast(ImpactData);
}

float UCore_PhysicsSystemManager::CalculateImpactForce(const FVector& Velocity, float Mass) const
{
    // F = ma, where a is deceleration (velocity change over time)
    // Simplified calculation for impact force
    float Speed = Velocity.Size();
    return Speed * Mass * 0.1f; // Scale for gameplay
}

ECore_PhysicsMaterial UCore_PhysicsSystemManager::DetermineMaterialType(AActor* Actor) const
{
    if (!Actor)
    {
        return ECore_PhysicsMaterial::Rock;
    }

    // Simple material determination based on actor name/class
    FString ActorName = Actor->GetName().ToLower();
    
    if (ActorName.Contains(TEXT("rock")) || ActorName.Contains(TEXT("stone")))
    {
        return ECore_PhysicsMaterial::Rock;
    }
    else if (ActorName.Contains(TEXT("wood")) || ActorName.Contains(TEXT("tree")))
    {
        return ECore_PhysicsMaterial::Wood;
    }
    else if (ActorName.Contains(TEXT("dinosaur")) || ActorName.Contains(TEXT("character")))
    {
        return ECore_PhysicsMaterial::Flesh;
    }
    else if (ActorName.Contains(TEXT("bone")))
    {
        return ECore_PhysicsMaterial::Bone;
    }
    else if (ActorName.Contains(TEXT("water")))
    {
        return ECore_PhysicsMaterial::Water;
    }
    
    return ECore_PhysicsMaterial::Rock; // Default
}

bool UCore_PhysicsSystemManager::LineTracePhysics(const FVector& Start, const FVector& End, FHitResult& OutHit, bool bIgnoreActors)
{
    if (!CachedWorld)
    {
        return false;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = true;
    
    if (bIgnoreActors)
    {
        QueryParams.AddIgnoredActors(GetActorsInRadius((Start + End) * 0.5f, FVector::Dist(Start, End)));
    }

    return CachedWorld->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, QueryParams);
}

bool UCore_PhysicsSystemManager::SphereTracePhysics(const FVector& Start, const FVector& End, float Radius, FHitResult& OutHit)
{
    if (!CachedWorld)
    {
        return false;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = true;

    return CachedWorld->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, ECC_WorldStatic, 
                                           FCollisionShape::MakeSphere(Radius), QueryParams);
}

TArray<AActor*> UCore_PhysicsSystemManager::GetActorsInRadius(const FVector& Center, float Radius)
{
    TArray<AActor*> FoundActors;
    
    if (!CachedWorld)
    {
        return FoundActors;
    }

    // Use overlap sphere to find actors
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    bool bHit = CachedWorld->OverlapMultiByChannel(OverlapResults, Center, FQuat::Identity, ECC_WorldDynamic,
                                                  FCollisionShape::MakeSphere(Radius), QueryParams);

    if (bHit)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (Result.GetActor())
            {
                FoundActors.Add(Result.GetActor());
            }
        }
    }

    return FoundActors;
}

void UCore_PhysicsSystemManager::TriggerDestruction(AActor* TargetActor, const FVector& ImpactPoint, float DestructionForce)
{
    if (!TargetActor)
    {
        UE_LOG(LogPhysicsSystem, Warning, TEXT("TriggerDestruction: Invalid target actor"));
        return;
    }

    if (!CanActorBeDestroyed(TargetActor, DestructionForce))
    {
        UE_LOG(LogPhysicsSystem, Log, TEXT("Actor %s cannot be destroyed with force %.2f"), 
               *TargetActor->GetName(), DestructionForce);
        return;
    }

    // Apply destruction impulse
    FVector DestructionImpulse = (TargetActor->GetActorLocation() - ImpactPoint).GetSafeNormal() * DestructionForce;
    ApplyImpulseToActor(TargetActor, DestructionImpulse, ImpactPoint);

    // Create impact event for destruction
    FCore_PhysicsImpact DestructionImpact;
    DestructionImpact.ImpactActor = TargetActor;
    DestructionImpact.ImpactLocation = ImpactPoint;
    DestructionImpact.ImpactForce = DestructionForce;
    DestructionImpact.MaterialType = DetermineMaterialType(TargetActor);
    
    BroadcastImpactEvent(DestructionImpact);
    
    UE_LOG(LogPhysicsSystem, Log, TEXT("Triggered destruction on actor %s with force %.2f"), 
           *TargetActor->GetName(), DestructionForce);
}

bool UCore_PhysicsSystemManager::CanActorBeDestroyed(AActor* TargetActor, float RequiredForce) const
{
    if (!TargetActor)
    {
        return false;
    }

    // Simple destruction threshold based on material type
    ECore_PhysicsMaterial MaterialType = DetermineMaterialType(TargetActor);
    
    float DestructionThreshold = 1000.0f; // Default threshold
    
    switch (MaterialType)
    {
        case ECore_PhysicsMaterial::Rock:
            DestructionThreshold = 5000.0f;
            break;
        case ECore_PhysicsMaterial::Wood:
            DestructionThreshold = 2000.0f;
            break;
        case ECore_PhysicsMaterial::Bone:
            DestructionThreshold = 3000.0f;
            break;
        case ECore_PhysicsMaterial::Flesh:
            DestructionThreshold = 500.0f;
            break;
        case ECore_PhysicsMaterial::Vegetation:
            DestructionThreshold = 100.0f;
            break;
        default:
            break;
    }

    return RequiredForce >= DestructionThreshold;
}

void UCore_PhysicsSystemManager::SetGlobalPhysicsScale(float Scale)
{
    GlobalPhysicsScale = FMath::Clamp(Scale, 0.1f, 10.0f);
    
    // Reconfigure physics settings with new scale
    ConfigurePhysicsSettings();
    
    UE_LOG(LogPhysicsSystem, Log, TEXT("Global physics scale set to %.2f"), GlobalPhysicsScale);
}

void UCore_PhysicsSystemManager::SetPhysicsSimulationEnabled(bool bEnabled)
{
    bPhysicsEnabled = bEnabled;
    
    UE_LOG(LogPhysicsSystem, Log, TEXT("Physics simulation %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_PhysicsSystemManager::RegisterWithCoreRegistry()
{
    if (UEngArch_CoreSystemRegistry* Registry = GetGameInstance()->GetSubsystem<UEngArch_CoreSystemRegistry>())
    {
        Registry->RegisterSystem(TEXT("PhysicsSystem"), this);
        CoreRegistry = Registry;
        
        UE_LOG(LogPhysicsSystem, Log, TEXT("Registered with Core System Registry"));
    }
    else
    {
        UE_LOG(LogPhysicsSystem, Warning, TEXT("Failed to find Core System Registry"));
    }
}

void UCore_PhysicsSystemManager::BindCollisionEvents()
{
    // Collision events are bound per-actor in RegisterForImpactEvents
}

void UCore_PhysicsSystemManager::UnbindCollisionEvents()
{
    // Clean up all registered actors
    for (TWeakObjectPtr<AActor>& WeakActor : RegisteredActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            Actor->OnActorHit.RemoveDynamic(this, &UCore_PhysicsSystemManager::OnActorHit);
        }
    }
    
    RegisteredActors.Empty();
}