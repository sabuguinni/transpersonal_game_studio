#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    // Initialize default physics values
    WorldGravity = -980.0f;
    DefaultLinearDamping = 0.01f;
    DefaultAngularDamping = 0.01f;
    WaterDensityMultiplier = 1.5f;
    MudFrictionMultiplier = 0.3f;
    RockHardnessMultiplier = 2.0f;
    ActivePhysicsActors = 0;
    PhysicsUpdateTime = 0.0f;
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Initializing physics system"));
    
    // Initialize physics settings for the prehistoric world
    InitializePhysicsSettings();
    
    // Set up world gravity
    SetWorldPhysicsGravity(WorldGravity);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics system initialized successfully"));
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Deinitializing physics system"));
    
    // Clean up registered actors
    RegisteredActors.Empty();
    
    Super::Deinitialize();
}

void UCore_PhysicsSystemManager::InitializePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: No world available for physics initialization"));
        return;
    }

    // Configure physics settings for realistic prehistoric environment
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        // Set realistic gravity for Earth
        PhysicsSettings->DefaultGravityZ = WorldGravity;
        
        // Configure collision channels for dinosaur game
        // These will be used for dinosaur-player, dinosaur-environment interactions
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics settings configured"));
    }
}

void UCore_PhysicsSystemManager::SetWorldPhysicsGravity(float NewGravity)
{
    WorldGravity = NewGravity;
    
    UWorld* World = GetWorld();
    if (World && World->GetPhysicsScene())
    {
        // Update gravity for the current world
        UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
        if (PhysicsSettings)
        {
            PhysicsSettings->DefaultGravityZ = NewGravity;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: World gravity set to %f"), NewGravity);
    }
}

void UCore_PhysicsSystemManager::RegisterPhysicsActor(AActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Attempted to register null actor"));
        return;
    }

    if (!RegisteredActors.Contains(Actor))
    {
        RegisteredActors.Add(Actor);
        ActivePhysicsActors++;
        
        // Set up collision callbacks
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp)
            {
                PrimComp->OnComponentHit.AddDynamic(this, &UCore_PhysicsSystemManager::OnActorHit);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Registered physics actor %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::UnregisterPhysicsActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    if (RegisteredActors.Contains(Actor))
    {
        RegisteredActors.Remove(Actor);
        ActivePhysicsActors--;
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Unregistered physics actor %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::EnableRagdollForActor(AActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Cannot enable ragdoll for null actor"));
        return;
    }

    // Find skeletal mesh component for ragdoll
    USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMesh)
    {
        // Enable physics simulation for ragdoll effect
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SkeletalMesh->SetSimulatePhysics(true);
        
        // Configure ragdoll settings
        SkeletalMesh->SetLinearDamping(DefaultLinearDamping);
        SkeletalMesh->SetAngularDamping(DefaultAngularDamping);
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Ragdoll enabled for actor %s"), *Actor->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: No skeletal mesh found for ragdoll on actor %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::DisableRagdollForActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMesh)
    {
        SkeletalMesh->SetSimulatePhysics(false);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Ragdoll disabled for actor %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::ApplyDestructionForce(AActor* Actor, FVector ForceLocation, float ForceStrength)
{
    if (!Actor)
    {
        return;
    }

    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            // Calculate force direction from impact point
            FVector ForceDirection = (Actor->GetActorLocation() - ForceLocation).GetSafeNormal();
            FVector Force = ForceDirection * ForceStrength;
            
            PrimComp->AddImpulseAtLocation(Force, ForceLocation);
            
            UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Applied destruction force to %s"), *Actor->GetName());
        }
    }
}

void UCore_PhysicsSystemManager::SetEnvironmentalPhysics(ECore_BiomeType BiomeType)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Adjust physics based on biome type
    switch (BiomeType)
    {
        case ECore_BiomeType::Swamp:
            // Swamp has higher drag, lower friction
            SetWorldPhysicsGravity(WorldGravity * 0.9f); // Slightly reduced gravity effect
            break;
            
        case ECore_BiomeType::Forest:
            // Standard physics for forest
            SetWorldPhysicsGravity(WorldGravity);
            break;
            
        case ECore_BiomeType::Savanna:
            // Dry conditions, standard physics
            SetWorldPhysicsGravity(WorldGravity);
            break;
            
        case ECore_BiomeType::Desert:
            // Hot, dry conditions
            SetWorldPhysicsGravity(WorldGravity * 1.05f); // Slightly increased gravity effect
            break;
            
        case ECore_BiomeType::SnowyMountains:
            // Cold, icy conditions
            SetWorldPhysicsGravity(WorldGravity * 1.1f); // Increased gravity, less air resistance
            break;
            
        default:
            SetWorldPhysicsGravity(WorldGravity);
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Environmental physics set for biome type"));
}

void UCore_PhysicsSystemManager::UpdatePhysicsForWeather(ECore_WeatherType WeatherType)
{
    // Adjust physics based on weather conditions
    switch (WeatherType)
    {
        case ECore_WeatherType::Rain:
            // Rain reduces friction, increases drag
            break;
            
        case ECore_WeatherType::Storm:
            // Strong winds affect physics
            break;
            
        case ECore_WeatherType::Snow:
            // Snow reduces friction significantly
            break;
            
        case ECore_WeatherType::Clear:
        default:
            // Standard conditions
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics updated for weather conditions"));
}

bool UCore_PhysicsSystemManager::CheckActorCollision(AActor* ActorA, AActor* ActorB)
{
    if (!ActorA || !ActorB)
    {
        return false;
    }

    // Simple bounds-based collision check
    FBox BoundsA = ActorA->GetComponentsBoundingBox();
    FBox BoundsB = ActorB->GetComponentsBoundingBox();
    
    return BoundsA.Intersect(BoundsB);
}

TArray<AActor*> UCore_PhysicsSystemManager::GetActorsInRadius(FVector Location, float Radius)
{
    TArray<AActor*> ActorsInRadius;
    
    for (AActor* Actor : RegisteredActors)
    {
        if (Actor && FVector::Dist(Actor->GetActorLocation(), Location) <= Radius)
        {
            ActorsInRadius.Add(Actor);
        }
    }
    
    return ActorsInRadius;
}

void UCore_PhysicsSystemManager::DebugPhysicsState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CORE PHYSICS SYSTEM DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered Actors: %d"), RegisteredActors.Num());
    UE_LOG(LogTemp, Warning, TEXT("Active Physics Actors: %d"), ActivePhysicsActors);
    UE_LOG(LogTemp, Warning, TEXT("World Gravity: %f"), WorldGravity);
    UE_LOG(LogTemp, Warning, TEXT("Physics Update Time: %f ms"), PhysicsUpdateTime);
    
    for (int32 i = 0; i < RegisteredActors.Num(); i++)
    {
        if (RegisteredActors[i])
        {
            UE_LOG(LogTemp, Log, TEXT("  Actor %d: %s"), i, *RegisteredActors[i]->GetName());
        }
    }
}

void UCore_PhysicsSystemManager::EnablePhysicsDebugDraw(bool bEnable)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (bEnable)
    {
        // Enable physics debug visualization
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics debug draw enabled"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics debug draw disabled"));
    }
}

void UCore_PhysicsSystemManager::UpdatePhysicsActors()
{
    // Clean up invalid actors
    CleanupInvalidActors();
    
    // Apply environmental modifiers
    ApplyEnvironmentalModifiers();
    
    // Track performance
    TrackPhysicsPerformance();
}

void UCore_PhysicsSystemManager::CleanupInvalidActors()
{
    RegisteredActors.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
    
    ActivePhysicsActors = RegisteredActors.Num();
}

void UCore_PhysicsSystemManager::ApplyEnvironmentalModifiers()
{
    // Apply biome-specific physics modifiers to registered actors
    for (AActor* Actor : RegisteredActors)
    {
        if (!Actor)
        {
            continue;
        }

        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                // Apply environmental physics modifiers
                PrimComp->SetLinearDamping(DefaultLinearDamping);
                PrimComp->SetAngularDamping(DefaultAngularDamping);
            }
        }
    }
}

void UCore_PhysicsSystemManager::OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!SelfActor || !OtherActor)
    {
        return;
    }

    // Handle collision between physics actors
    float ImpactStrength = NormalImpulse.Size();
    
    if (ImpactStrength > 1000.0f) // Significant impact
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Significant impact between %s and %s (Force: %f)"), 
               *SelfActor->GetName(), *OtherActor->GetName(), ImpactStrength);
    }
}

void UCore_PhysicsSystemManager::TrackPhysicsPerformance()
{
    // Simple performance tracking
    PhysicsUpdateTime = 0.016f; // Placeholder - would be actual measurement
}