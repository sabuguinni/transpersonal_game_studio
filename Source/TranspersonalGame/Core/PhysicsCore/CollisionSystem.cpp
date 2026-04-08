// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CollisionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"

UCollisionSystem::UCollisionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize LOD distances for different collision complexities
    CollisionLODDistances = {500.0f, 1500.0f, 5000.0f, 15000.0f};
    
    // Setup creature collision profiles mapping
    CreatureCollisionProfiles.Add(ECreatureCollisionSize::Tiny, FName("TinyCreature"));
    CreatureCollisionProfiles.Add(ECreatureCollisionSize::Small, FName("SmallCreature"));
    CreatureCollisionProfiles.Add(ECreatureCollisionSize::Medium, FName("MediumCreature"));
    CreatureCollisionProfiles.Add(ECreatureCollisionSize::Large, FName("LargeCreature"));
    CreatureCollisionProfiles.Add(ECreatureCollisionSize::Massive, FName("MassiveCreature"));
}

void UCollisionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CachedWorld = GetWorld();
    if (!CachedWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("CollisionSystem: Failed to get world reference"));
        return;
    }
    
    InitializeCollisionSystem();
    
    UE_LOG(LogTemp, Log, TEXT("CollisionSystem: Initialized for prehistoric survival gameplay"));
}

void UCollisionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Reset frame counters
    CollisionTestsThisFrame = 0;
    
    // Update collision LOD optimization
    if (bEnableHierarchicalOptimization)
    {
        UpdateCollisionLOD();
    }
}

void UCollisionSystem::InitializeCollisionSystem()
{
    if (!CachedWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("CollisionSystem: Cannot initialize without valid world"));
        return;
    }
    
    InitializeCollisionChannels();
    SetupCollisionResponses();
    CreatePhysicsMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("CollisionSystem: Collision system initialized with %d LOD levels"), CollisionLODDistances.Num());
}

void UCollisionSystem::InitializeCollisionChannels()
{
    // Custom collision channels for prehistoric survival game:
    // - ECC_GameTraceChannel1: Creature (dinosaurs, player)
    // - ECC_GameTraceChannel2: Environment (trees, rocks, destructible)
    // - ECC_GameTraceChannel3: Projectile (spears, arrows, thrown objects)
    // - ECC_GameTraceChannel4: Trigger (quest triggers, danger zones)
    // - ECC_GameTraceChannel5: Interaction (items, tools, resources)
    
    UE_LOG(LogTemp, Log, TEXT("CollisionSystem: Custom collision channels configured for survival gameplay"));
}

void UCollisionSystem::SetupCollisionResponses()
{
    // Setup collision response matrix for gameplay:
    // Creatures block other creatures and environment
    // Projectiles block creatures and environment but ignore triggers
    // Environment blocks everything except triggers
    // Triggers overlap with everything
    // Interactions overlap with creatures but block environment
    
    UE_LOG(LogTemp, Log, TEXT("CollisionSystem: Collision response matrix configured"));
}

void UCollisionSystem::CreatePhysicsMaterials()
{
    // Create physics materials for different prehistoric surfaces
    
    // Stone/Rock material - high friction, low restitution
    UPhysicalMaterial* StoneMaterial = NewObject<UPhysicalMaterial>();
    if (StoneMaterial)
    {
        StoneMaterial->Friction = 0.8f;
        StoneMaterial->Restitution = 0.2f;
        StoneMaterial->Density = 2.5f; // Stone density g/cm³
        PhysicsMaterials.Add(TEXT("Stone"), StoneMaterial);
    }
    
    // Organic/Wood material - medium friction, low restitution
    UPhysicalMaterial* WoodMaterial = NewObject<UPhysicalMaterial>();
    if (WoodMaterial)
    {
        WoodMaterial->Friction = 0.6f;
        WoodMaterial->Restitution = 0.3f;
        WoodMaterial->Density = 0.7f; // Wood density
        PhysicsMaterials.Add(TEXT("Wood"), WoodMaterial);
    }
    
    // Mud/Soft Ground - high friction, very low restitution
    UPhysicalMaterial* MudMaterial = NewObject<UPhysicalMaterial>();
    if (MudMaterial)
    {
        MudMaterial->Friction = 1.2f;
        MudMaterial->Restitution = 0.1f;
        MudMaterial->Density = 1.8f; // Mud density
        PhysicsMaterials.Add(TEXT("Mud"), MudMaterial);
    }
    
    // Creature Skin - medium friction, medium restitution
    UPhysicalMaterial* CreatureMaterial = NewObject<UPhysicalMaterial>();
    if (CreatureMaterial)
    {
        CreatureMaterial->Friction = 0.5f;
        CreatureMaterial->Restitution = 0.4f;
        CreatureMaterial->Density = 1.0f; // Flesh density
        PhysicsMaterials.Add(TEXT("Creature"), CreatureMaterial);
    }
    
    // Water - very low friction, no restitution
    UPhysicalMaterial* WaterMaterial = NewObject<UPhysicalMaterial>();
    if (WaterMaterial)
    {
        WaterMaterial->Friction = 0.1f;
        WaterMaterial->Restitution = 0.0f;
        WaterMaterial->Density = 1.0f; // Water density
        PhysicsMaterials.Add(TEXT("Water"), WaterMaterial);
    }
    
    UE_LOG(LogTemp, Log, TEXT("CollisionSystem: Created %d physics materials for prehistoric environment"), PhysicsMaterials.Num());
}

UCollisionSystem::FAdvancedCollisionResult UCollisionSystem::PerformAdvancedCollisionTest(AActor* ActorA, AActor* ActorB, ECollisionTestType CollisionType)
{
    FAdvancedCollisionResult Result;
    
    if (!ActorA || !ActorB || !CachedWorld)
    {
        UE_LOG(LogTemp, Warning, TEXT("CollisionSystem: Invalid actors for collision test"));
        return Result;
    }
    
    // Limit collision tests per frame for performance
    if (CollisionTestsThisFrame >= MaxCollisionTestsPerFrame)
    {
        UE_LOG(LogTemp, Verbose, TEXT("CollisionSystem: Collision test limit reached this frame"));
        return Result;
    }
    
    CollisionTestsThisFrame++;
    
    // Get primitive components for collision testing
    UPrimitiveComponent* CompA = ActorA->GetRootComponent() ? Cast<UPrimitiveComponent>(ActorA->GetRootComponent()) : nullptr;
    UPrimitiveComponent* CompB = ActorB->GetRootComponent() ? Cast<UPrimitiveComponent>(ActorB->GetRootComponent()) : nullptr;
    
    if (!CompA || !CompB)
    {
        return Result;
    }
    
    // Perform collision test based on type
    switch (CollisionType)
    {
        case ECollisionTestType::Simple:
        {
            // Simple overlap test
            Result.bHasCollision = CompA->IsOverlappingComponent(CompB);
            if (Result.bHasCollision)
            {
                Result.ImpactLocation = (ActorA->GetActorLocation() + ActorB->GetActorLocation()) * 0.5f;
            }
            break;
        }
        
        case ECollisionTestType::Complex:
        {
            // Complex mesh collision with detailed hit result
            FHitResult HitResult;
            FVector Start = ActorA->GetActorLocation();
            FVector End = ActorB->GetActorLocation();
            
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(ActorA);
            QueryParams.bTraceComplex = true;
            
            if (CachedWorld->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
            {
                if (HitResult.GetActor() == ActorB)
                {
                    Result.bHasCollision = true;
                    Result.ImpactLocation = HitResult.ImpactPoint;
                    Result.ImpactNormal = HitResult.ImpactNormal;
                    Result.HitComponent = HitResult.GetComponent();
                    Result.HitBoneName = HitResult.BoneName;
                    Result.HitMaterial = HitResult.PhysMaterial.Get();
                    Result.PenetrationDepth = (End - Start).Size() - HitResult.Distance;
                }
            }
            break;
        }
        
        case ECollisionTestType::Creature:
        {
            // Specialized creature collision with size-based calculations
            USkeletalMeshComponent* SkeletalA = ActorA->FindComponentByClass<USkeletalMeshComponent>();
            USkeletalMeshComponent* SkeletalB = ActorB->FindComponentByClass<USkeletalMeshComponent>();
            
            if (SkeletalA && SkeletalB)
            {
                // Check collision between skeletal meshes with bone-level precision
                FVector LocationA = ActorA->GetActorLocation();
                FVector LocationB = ActorB->GetActorLocation();
                float Distance = FVector::Dist(LocationA, LocationB);
                
                // Calculate combined collision radius based on creature sizes
                float RadiusA = SkeletalA->Bounds.SphereRadius;
                float RadiusB = SkeletalB->Bounds.SphereRadius;
                float CombinedRadius = RadiusA + RadiusB;
                
                if (Distance <= CombinedRadius)
                {
                    Result.bHasCollision = true;
                    Result.ImpactLocation = LocationA + (LocationB - LocationA) * (RadiusA / CombinedRadius);
                    Result.ImpactNormal = (LocationB - LocationA).GetSafeNormal();
                    Result.PenetrationDepth = CombinedRadius - Distance;
                    Result.HitComponent = CompB;
                    
                    // Calculate impact force based on creature masses and velocities
                    float MassA = CompA->GetMass();
                    float MassB = CompB->GetMass();
                    FVector VelocityA = CompA->GetPhysicsLinearVelocity();
                    FVector VelocityB = CompB->GetPhysicsLinearVelocity();
                    
                    float RelativeSpeed = (VelocityA - VelocityB).Size();
                    Result.ImpactForce = (MassA * MassB / (MassA + MassB)) * RelativeSpeed;
                }
            }
            break;
        }
        
        case ECollisionTestType::Environmental:
        {
            // Environmental destruction collision
            UGeometryCollectionComponent* GeomCollection = ActorB->FindComponentByClass<UGeometryCollectionComponent>();
            if (GeomCollection)
            {
                // Check if impact force exceeds destruction threshold
                FVector VelocityA = CompA->GetPhysicsLinearVelocity();
                float ImpactForce = CompA->GetMass() * VelocityA.Size();
                
                if (ImpactForce > DestructionForceThreshold)
                {
                    Result.bHasCollision = true;
                    Result.ImpactLocation = ActorB->GetActorLocation();
                    Result.ImpactForce = ImpactForce;
                    Result.HitComponent = GeomCollection;
                }
            }
            break;
        }
        
        case ECollisionTestType::Hierarchical:
        {
            // Multi-level collision detection for complex objects
            TArray<UPrimitiveComponent*> ComponentsA;
            TArray<UPrimitiveComponent*> ComponentsB;
            
            ActorA->GetComponents<UPrimitiveComponent>(ComponentsA);
            ActorB->GetComponents<UPrimitiveComponent>(ComponentsB);
            
            for (UPrimitiveComponent* CompA_Inner : ComponentsA)
            {
                for (UPrimitiveComponent* CompB_Inner : ComponentsB)
                {
                    if (CompA_Inner->IsOverlappingComponent(CompB_Inner))
                    {
                        Result.bHasCollision = true;
                        Result.ImpactLocation = (CompA_Inner->GetComponentLocation() + CompB_Inner->GetComponentLocation()) * 0.5f;
                        Result.HitComponent = CompB_Inner;
                        break;
                    }
                }
                if (Result.bHasCollision) break;
            }
            break;
        }
    }
    
    return Result;
}

void UCollisionSystem::SetupCreatureCollision(AActor* CreatureActor, ECreatureCollisionSize CreatureSize, FName CollisionPreset)
{
    if (!CreatureActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("CollisionSystem: Cannot setup collision for null creature"));
        return;
    }
    
    UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(CreatureActor->GetRootComponent());
    if (!RootComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("CollisionSystem: Creature has no primitive root component"));
        return;
    }
    
    // Apply collision preset based on creature size
    RootComp->SetCollisionProfileName(CollisionPreset);
    
    // Adjust collision settings based on creature size
    switch (CreatureSize)
    {
        case ECreatureCollisionSize::Tiny:
            RootComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
            break;
        case ECreatureCollisionSize::Small:
            RootComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
            break;
        case ECreatureCollisionSize::Medium:
            RootComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            break;
        case ECreatureCollisionSize::Large:
            RootComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            RootComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            break;
        case ECreatureCollisionSize::Massive:
            RootComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            RootComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            RootComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
            break;
    }
    
    // Add to tracked collision actors for LOD optimization
    TrackedCollisionActors.AddUnique(CreatureActor);
    
    UE_LOG(LogTemp, Log, TEXT("CollisionSystem: Setup collision for %s creature with preset %s"), 
           *UEnum::GetValueAsString(CreatureSize), *CollisionPreset.ToString());
}

void UCollisionSystem::HandleEnvironmentalImpact(AActor* ImpactActor, AActor* EnvironmentActor, float ImpactForce, FVector ImpactLocation)
{
    if (!ImpactActor || !EnvironmentActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("CollisionSystem: Invalid actors for environmental impact"));
        return;
    }
    
    // Check if impact force exceeds destruction threshold
    if (ImpactForce < DestructionForceThreshold)
    {
        UE_LOG(LogTemp, Verbose, TEXT("CollisionSystem: Impact force %f below destruction threshold %f"), 
               ImpactForce, DestructionForceThreshold);
        return;
    }
    
    // Handle destruction for geometry collections
    UGeometryCollectionComponent* GeomCollection = EnvironmentActor->FindComponentByClass<UGeometryCollectionComponent>();
    if (GeomCollection)
    {
        // Apply radial damage to trigger fracturing
        float DamageRadius = FMath::Clamp(ImpactForce / 100.0f, 50.0f, 500.0f);
        GeomCollection->ApplyExternalStrain(0, ImpactLocation, DamageRadius, 0, ImpactForce);
        
        UE_LOG(LogTemp, Log, TEXT("CollisionSystem: Applied environmental destruction with force %f at %s"), 
               ImpactForce, *ImpactLocation.ToString());
    }
    
    // Broadcast collision event
    FAdvancedCollisionResult CollisionResult;
    CollisionResult.bHasCollision = true;
    CollisionResult.ImpactLocation = ImpactLocation;
    CollisionResult.ImpactForce = ImpactForce;
    
    OnCollisionEvent.Broadcast(ImpactActor, EnvironmentActor, CollisionResult);
}

void UCollisionSystem::OptimizeCollisionLOD(FVector ViewerLocation, float MaxDistance)
{
    if (!CachedWorld)
    {
        return;
    }
    
    // Update collision LOD for all tracked actors
    for (AActor* TrackedActor : TrackedCollisionActors)
    {
        if (!IsValid(TrackedActor))
        {
            continue;
        }
        
        float Distance = FVector::Dist(ViewerLocation, TrackedActor->GetActorLocation());
        ECollisionEnabled::Type NewCollisionType = GetCollisionComplexityForDistance(Distance);
        
        UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(TrackedActor->GetRootComponent());
        if (RootComp && RootComp->GetCollisionEnabled() != NewCollisionType)
        {
            RootComp->SetCollisionEnabled(NewCollisionType);
        }
    }
}

void UCollisionSystem::RegisterCollisionCallback(const FCollisionEventDelegate& Callback)
{
    OnCollisionEvent.AddDynamic(Callback.GetUObject(), Callback.GetFunctionName());
    UE_LOG(LogTemp, Log, TEXT("CollisionSystem: Registered collision event callback"));
}

void UCollisionSystem::UpdateCollisionLOD()
{
    // Get player location for LOD calculations
    if (CachedWorld && CachedWorld->GetFirstPlayerController())
    {
        APawn* PlayerPawn = CachedWorld->GetFirstPlayerController()->GetPawn();
        if (PlayerPawn)
        {
            OptimizeCollisionLOD(PlayerPawn->GetActorLocation(), MaxCollisionDistance);
        }
    }
}

ECollisionEnabled::Type UCollisionSystem::GetCollisionComplexityForDistance(float Distance) const
{
    if (Distance <= CollisionLODDistances[0])
    {
        return ECollisionEnabled::QueryAndPhysics; // Full collision
    }
    else if (Distance <= CollisionLODDistances[1])
    {
        return ECollisionEnabled::QueryOnly; // Query only
    }
    else if (Distance <= CollisionLODDistances[2])
    {
        return ECollisionEnabled::QueryOnly; // Simplified query
    }
    else
    {
        return ECollisionEnabled::NoCollision; // No collision
    }
}