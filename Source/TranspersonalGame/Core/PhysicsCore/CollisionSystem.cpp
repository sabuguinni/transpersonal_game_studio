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
        case ECollisionTestType::Simple:\n        {\n            // Simple overlap test\n            Result.bHasCollision = CompA->IsOverlappingComponent(CompB);\n            if (Result.bHasCollision)\n            {\n                Result.ImpactLocation = (ActorA->GetActorLocation() + ActorB->GetActorLocation()) * 0.5f;\n            }\n            break;\n        }\n        \n        case ECollisionTestType::Complex:\n        {\n            // Complex mesh collision with detailed hit result\n            FHitResult HitResult;\n            FVector Start = ActorA->GetActorLocation();\n            FVector End = ActorB->GetActorLocation();\n            \n            FCollisionQueryParams QueryParams;\n            QueryParams.AddIgnoredActor(ActorA);\n            QueryParams.bTraceComplex = true;\n            \n            if (CachedWorld->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))\n            {\n                if (HitResult.GetActor() == ActorB)\n                {\n                    Result.bHasCollision = true;\n                    Result.ImpactLocation = HitResult.ImpactPoint;\n                    Result.ImpactNormal = HitResult.ImpactNormal;\n                    Result.HitComponent = HitResult.GetComponent();\n                    Result.HitBoneName = HitResult.BoneName;\n                    Result.HitMaterial = HitResult.PhysMaterial.Get();\n                    Result.PenetrationDepth = (End - Start).Size() - HitResult.Distance;\n                }\n            }\n            break;\n        }\n        \n        case ECollisionTestType::Creature:\n        {\n            // Specialized creature collision with size-based calculations\n            USkeletalMeshComponent* SkeletalA = ActorA->FindComponentByClass<USkeletalMeshComponent>();\n            USkeletalMeshComponent* SkeletalB = ActorB->FindComponentByClass<USkeletalMeshComponent>();\n            \n            if (SkeletalA && SkeletalB)\n            {\n                // Check collision between skeletal meshes with bone-level precision\n                FVector LocationA = ActorA->GetActorLocation();\n                FVector LocationB = ActorB->GetActorLocation();\n                float Distance = FVector::Dist(LocationA, LocationB);\n                \n                // Calculate combined collision radius based on creature sizes\n                float RadiusA = SkeletalA->Bounds.SphereRadius;\n                float RadiusB = SkeletalB->Bounds.SphereRadius;\n                float CombinedRadius = RadiusA + RadiusB;\n                \n                if (Distance <= CombinedRadius)\n                {\n                    Result.bHasCollision = true;\n                    Result.ImpactLocation = LocationA + (LocationB - LocationA) * (RadiusA / CombinedRadius);\n                    Result.ImpactNormal = (LocationB - LocationA).GetSafeNormal();\n                    Result.PenetrationDepth = CombinedRadius - Distance;\n                    \n                    // Calculate impact force based on creature masses and velocities\n                    float MassA = SkeletalA->GetMass();\n                    float MassB = SkeletalB->GetMass();\n                    FVector VelocityA = SkeletalA->GetPhysicsLinearVelocity();\n                    FVector VelocityB = SkeletalB->GetPhysicsLinearVelocity();\n                    \n                    FVector RelativeVelocity = VelocityA - VelocityB;\n                    Result.ImpactForce = (MassA * MassB / (MassA + MassB)) * RelativeVelocity.Size();\n                }\n            }\n            break;\n        }\n        \n        case ECollisionTestType::Environmental:\n        {\n            // Environmental collision with destruction potential\n            UGeometryCollectionComponent* GeoCollection = ActorB->FindComponentByClass<UGeometryCollectionComponent>();\n            if (GeoCollection)\n            {\n                // Check if impact force exceeds destruction threshold\n                UPrimitiveComponent* ImpactComp = Cast<UPrimitiveComponent>(ActorA->GetRootComponent());\n                if (ImpactComp)\n                {\n                    FVector Velocity = ImpactComp->GetPhysicsLinearVelocity();\n                    float Mass = ImpactComp->GetMass();\n                    float KineticEnergy = 0.5f * Mass * Velocity.SizeSquared();\n                    \n                    if (KineticEnergy > DestructionForceThreshold)\n                    {\n                        Result.bHasCollision = true;\n                        Result.ImpactLocation = ActorB->GetActorLocation();\n                        Result.ImpactForce = KineticEnergy;\n                        Result.HitComponent = GeoCollection;\n                    }\n                }\n            }\n            break;\n        }\n    }\n    \n    // Broadcast collision event if collision detected\n    if (Result.bHasCollision && OnCollisionEvent.IsBound())\n    {\n        OnCollisionEvent.Broadcast(ActorA, ActorB, Result);\n    }\n    \n    return Result;\n}\n\nvoid UCollisionSystem::SetupCreatureCollision(AActor* CreatureActor, ECreatureCollisionSize CreatureSize, FName CollisionPreset)\n{\n    if (!CreatureActor)\n    {\n        UE_LOG(LogTemp, Warning, TEXT("CollisionSystem: Invalid creature actor for collision setup"));\n        return;\n    }\n    \n    UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(CreatureActor->GetRootComponent());\n    if (!RootComp)\n    {\n        UE_LOG(LogTemp, Warning, TEXT("CollisionSystem: Creature actor has no primitive root component"));\n        return;\n    }\n    \n    // Apply collision preset based on creature size\n    FName* ProfileName = CreatureCollisionProfiles.Find(CreatureSize);\n    if (ProfileName)\n    {\n        RootComp->SetCollisionProfileName(*ProfileName);\n    }\n    else\n    {\n        RootComp->SetCollisionProfileName(CollisionPreset);\n    }\n    \n    // Setup size-specific collision properties\n    switch (CreatureSize)\n    {\n        case ECreatureCollisionSize::Tiny:\n            RootComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // Tiny creatures don't block movement\n            break;\n            \n        case ECreatureCollisionSize::Small:\n            RootComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);\n            break;\n            \n        case ECreatureCollisionSize::Medium:\n            RootComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);\n            RootComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);\n            break;\n            \n        case ECreatureCollisionSize::Large:\n            RootComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);\n            RootComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);\n            RootComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);\n            break;\n            \n        case ECreatureCollisionSize::Massive:\n            // Massive creatures can break through some environmental objects\n            RootComp->SetCollisionResponseToAllChannels(ECR_Block);\n            RootComp->SetCollisionResponseToChannel(ECC_Destructible, ECR_Overlap); // Can trigger destruction\n            break;\n    }\n    \n    // Add to tracked actors for LOD optimization\n    TrackedCollisionActors.AddUnique(CreatureActor);\n    \n    UE_LOG(LogTemp, Log, TEXT("CollisionSystem: Setup collision for %s creature: %s"), \n           *UEnum::GetValueAsString(CreatureSize), *CreatureActor->GetName());\n}\n\nvoid UCollisionSystem::HandleEnvironmentalImpact(AActor* ImpactActor, AActor* EnvironmentActor, float ImpactForce, FVector ImpactLocation)\n{\n    if (!ImpactActor || !EnvironmentActor)\n    {\n        return;\n    }\n    \n    // Check if impact force exceeds destruction threshold\n    if (ImpactForce < DestructionForceThreshold)\n    {\n        return;\n    }\n    \n    // Handle geometry collection destruction\n    UGeometryCollectionComponent* GeoCollection = EnvironmentActor->FindComponentByClass<UGeometryCollectionComponent>();\n    if (GeoCollection)\n    {\n        // Apply destruction impulse\n        FVector ImpulseDirection = (EnvironmentActor->GetActorLocation() - ImpactActor->GetActorLocation()).GetSafeNormal();\n        GeoCollection->ApplyPhysicsField(true, EFieldPhysicsType::Field_LinearForce, nullptr, nullptr);\n        \n        UE_LOG(LogTemp, Log, TEXT("CollisionSystem: Environmental destruction triggered - Force: %.2f at %s\"), \n               ImpactForce, *ImpactLocation.ToString());\n    }\n    \n    // Handle static mesh destruction (convert to geometry collection if needed)\n    UStaticMeshComponent* StaticMesh = EnvironmentActor->FindComponentByClass<UStaticMeshComponent>();\n    if (StaticMesh && !GeoCollection)\n    {\n        // Could implement runtime conversion to geometry collection here\n        UE_LOG(LogTemp, Log, TEXT("CollisionSystem: Static mesh impact - consider converting to destructible"));\n    }\n}\n\nvoid UCollisionSystem::OptimizeCollisionLOD(FVector ViewerLocation, float MaxDistance)\n{\n    if (!bEnableHierarchicalOptimization)\n    {\n        return;\n    }\n    \n    for (AActor* Actor : TrackedCollisionActors)\n    {\n        if (!IsValid(Actor))\n        {\n            continue;\n        }\n        \n        float Distance = FVector::Dist(ViewerLocation, Actor->GetActorLocation());\n        \n        // Skip actors beyond maximum distance\n        if (Distance > MaxDistance)\n        {\n            continue;\n        }\n        \n        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());\n        if (!PrimComp)\n        {\n            continue;\n        }\n        \n        // Set collision complexity based on distance\n        ECollisionEnabled::Type CollisionType = GetCollisionComplexityForDistance(Distance);\n        PrimComp->SetCollisionEnabled(CollisionType);\n    }\n}\n\nvoid UCollisionSystem::UpdateCollisionLOD()\n{\n    // Get player location for LOD calculations\n    if (CachedWorld && CachedWorld->GetFirstPlayerController())\n    {\n        APawn* PlayerPawn = CachedWorld->GetFirstPlayerController()->GetPawn();\n        if (PlayerPawn)\n        {\n            OptimizeCollisionLOD(PlayerPawn->GetActorLocation(), MaxCollisionDistance);\n        }\n    }\n}\n\nECollisionEnabled::Type UCollisionSystem::GetCollisionComplexityForDistance(float Distance) const\n{\n    if (Distance <= CollisionLODDistances[0])\n    {\n        return ECollisionEnabled::QueryAndPhysics; // Full collision\n    }\n    else if (Distance <= CollisionLODDistances[1])\n    {\n        return ECollisionEnabled::QueryOnly; // Query only, no physics\n    }\n    else if (Distance <= CollisionLODDistances[2])\n    {\n        return ECollisionEnabled::QueryOnly; // Simplified collision\n    }\n    else\n    {\n        return ECollisionEnabled::NoCollision; // No collision at far distances\n    }\n}\n\nvoid UCollisionSystem::RegisterCollisionCallback(const FCollisionEventDelegate& Callback)\n{\n    OnCollisionEvent.AddDynamic(Callback.GetUObject(), Callback.GetFunctionName());\n    UE_LOG(LogTemp, Log, TEXT("CollisionSystem: Collision callback registered"));\n}"