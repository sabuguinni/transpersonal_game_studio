#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsSettings.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default physics settings
    PhysicsSettings.GravityScale = 1.0f;
    PhysicsSettings.LinearDamping = 0.01f;
    PhysicsSettings.AngularDamping = 0.0f;
    PhysicsSettings.bEnableGravity = true;
    PhysicsSettings.PhysicsMode = ECore_PhysicsMode::Realistic;
    
    WorldGravityZ = -980.0f;
    bEnablePhysicsSimulation = true;
    MaxPhysicsSubsteps = 6.0f;
    MaxSubstepDeltaTime = 0.016667f;
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UpdatePhysicsSettings();
    ValidatePhysicsComponents();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Initialized with mode %d"), (int32)PhysicsSettings.PhysicsMode);
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Monitor physics performance every 60 frames
    static int32 FrameCounter = 0;
    FrameCounter++;
    
    if (FrameCounter >= 60)
    {
        ValidatePhysicsComponents();
        FrameCounter = 0;
    }
}

void UCore_PhysicsSystemManager::SetWorldGravity(float NewGravityZ)
{
    WorldGravityZ = NewGravityZ;
    
    if (UWorld* World = GetWorld())
    {
        FVector NewGravity(0.0f, 0.0f, WorldGravityZ);
        World->GetPhysicsScene()->SetGravity(NewGravity);
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: World gravity set to %f"), NewGravityZ);
    }
}

void UCore_PhysicsSystemManager::SetPhysicsMode(ECore_PhysicsMode NewMode)
{
    PhysicsSettings.PhysicsMode = NewMode;
    
    switch (NewMode)
    {
        case ECore_PhysicsMode::Realistic:
            PhysicsSettings.GravityScale = 1.0f;
            PhysicsSettings.LinearDamping = 0.01f;
            PhysicsSettings.AngularDamping = 0.0f;
            WorldGravityZ = -980.0f;
            break;
            
        case ECore_PhysicsMode::Arcade:
            PhysicsSettings.GravityScale = 0.7f;
            PhysicsSettings.LinearDamping = 0.1f;
            PhysicsSettings.AngularDamping = 0.05f;
            WorldGravityZ = -686.0f;
            break;
            
        case ECore_PhysicsMode::Cinematic:
            PhysicsSettings.GravityScale = 0.5f;
            PhysicsSettings.LinearDamping = 0.2f;
            PhysicsSettings.AngularDamping = 0.1f;
            WorldGravityZ = -490.0f;
            break;
    }
    
    UpdatePhysicsSettings();
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics mode changed to %d"), (int32)NewMode);
}

void UCore_PhysicsSystemManager::ApplyPhysicsSettingsToActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && PrimComp->GetBodyInstance())
        {
            PrimComp->SetLinearDamping(PhysicsSettings.LinearDamping);
            PrimComp->SetAngularDamping(PhysicsSettings.AngularDamping);
            PrimComp->SetEnableGravity(PhysicsSettings.bEnableGravity);
            
            // Apply gravity scale
            if (FBodyInstance* BodyInstance = PrimComp->GetBodyInstance())
            {
                BodyInstance->SetInstanceSimulatePhysics(bEnablePhysicsSimulation);
            }
        }
    }
    
    // Track this actor for future updates
    ManagedActors.AddUnique(TargetActor);
}

void UCore_PhysicsSystemManager::EnablePhysicsForActor(AActor* TargetActor, bool bEnable)
{
    if (!TargetActor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            PrimComp->SetSimulatePhysics(bEnable);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics %s for actor %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"), 
           *TargetActor->GetName());
}

FVector UCore_PhysicsSystemManager::CalculateImpactForce(const FVector& Velocity, float Mass, float ImpactDuration)
{
    // F = ma, where a = Δv/Δt
    FVector Acceleration = Velocity / FMath::Max(ImpactDuration, 0.001f);
    return Acceleration * Mass;
}

void UCore_PhysicsSystemManager::SimulateExplosion(const FVector& ExplosionLocation, float ExplosionRadius, float ExplosionForce)
{
    if (UWorld* World = GetWorld())
    {
        // Find all actors within explosion radius
        TArray<AActor*> FoundActors;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && FVector::Dist(Actor->GetActorLocation(), ExplosionLocation) <= ExplosionRadius)
            {
                TArray<UPrimitiveComponent*> PrimitiveComponents;
                Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
                
                for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
                {
                    if (PrimComp && PrimComp->IsSimulatingPhysics())
                    {
                        FVector Direction = (Actor->GetActorLocation() - ExplosionLocation).GetSafeNormal();
                        float Distance = FVector::Dist(Actor->GetActorLocation(), ExplosionLocation);
                        float ForceFalloff = 1.0f - (Distance / ExplosionRadius);
                        FVector ImpulseForce = Direction * ExplosionForce * ForceFalloff;
                        
                        PrimComp->AddImpulse(ImpulseForce);
                    }
                }
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Explosion simulated at %s with radius %f and force %f"), 
               *ExplosionLocation.ToString(), ExplosionRadius, ExplosionForce);
    }
}

void UCore_PhysicsSystemManager::UpdatePhysicsSettings()
{
    SetWorldGravity(WorldGravityZ);
    
    // Apply settings to all managed actors
    for (AActor* Actor : ManagedActors)
    {
        if (IsValid(Actor))
        {
            ApplyPhysicsSettingsToActor(Actor);
        }
    }
}

void UCore_PhysicsSystemManager::ValidatePhysicsComponents()
{
    // Clean up invalid actor references
    ManagedActors.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
    
    // Log physics performance metrics
    if (UWorld* World = GetWorld())
    {
        int32 SimulatingActors = 0;
        for (AActor* Actor : ManagedActors)
        {
            if (IsValid(Actor))
            {
                TArray<UPrimitiveComponent*> PrimitiveComponents;
                Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
                
                for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
                {
                    if (PrimComp && PrimComp->IsSimulatingPhysics())
                    {
                        SimulatingActors++;
                        break;
                    }
                }
            }
        }
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsSystemManager: Managing %d actors, %d simulating physics"), 
               ManagedActors.Num(), SimulatingActors);
    }
}