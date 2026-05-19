#include "Core_RigidBodySystem.h"
#include "EngineArchitecturalCore.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY(LogRigidBodySystem);

UCore_RigidBodySystem::UCore_RigidBodySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.033f; // 30 FPS for rigid body updates
    
    // Initialize default rigid body settings
    DefaultRigidBodySettings.Mass = 100.0f;
    DefaultRigidBodySettings.LinearDamping = 0.01f;
    DefaultRigidBodySettings.AngularDamping = 0.05f;
    DefaultRigidBodySettings.Restitution = 0.3f;
    DefaultRigidBodySettings.Friction = 0.7f;
    DefaultRigidBodySettings.bEnableGravity = true;
    DefaultRigidBodySettings.bStartAwake = true;
    DefaultRigidBodySettings.SleepThreshold = 0.1f;
    
    // Performance settings
    MaxRigidBodies = 500;
    UpdateDistance = 3000.0f;
    bEnableLODOptimization = true;
    
    // Statistics
    ActiveRigidBodies = 0;
    SleepingRigidBodies = 0;
    TotalRigidBodies = 0;
}

void UCore_RigidBodySystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with EngineArchitecturalCore
    if (UEngineArchitecturalCore* ArchCore = GetWorld()->GetGameInstance()->GetSubsystem<UEngineArchitecturalCore>())
    {
        ArchCore->RegisterModule(TEXT("RigidBodySystem"), this);
        UE_LOG(LogRigidBodySystem, Log, TEXT("RigidBodySystem registered with EngineArchitecturalCore"));
    }
    
    InitializeRigidBodySystem();
    SetupPhysicalMaterials();
    
    UE_LOG(LogRigidBodySystem, Log, TEXT("Core_RigidBodySystem initialized - Max Bodies: %d"), MaxRigidBodies);
}

void UCore_RigidBodySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateRigidBodyStatistics();
    OptimizeRigidBodyPerformance();
    ProcessRigidBodyEvents();
}

void UCore_RigidBodySystem::InitializeRigidBodySystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogRigidBodySystem, Error, TEXT("Failed to get world for rigid body initialization"));
        return;
    }
    
    // Initialize rigid body tracking
    RigidBodyActors.Empty();
    RigidBodyComponents.Empty();
    
    // Find existing rigid bodies in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && ShouldTrackActor(Actor))
        {
            RegisterRigidBodyActor(Actor);
        }
    }
    
    UE_LOG(LogRigidBodySystem, Log, TEXT("Initialized rigid body system with %d existing bodies"), RigidBodyActors.Num());
}

void UCore_RigidBodySystem::SetupPhysicalMaterials()
{
    // Create physical materials for different prehistoric materials
    PhysicalMaterials.Empty();
    
    // Stone material (rocks, cave walls)
    FCore_PhysicalMaterialData StoneMaterial;
    StoneMaterial.MaterialName = TEXT("Stone");
    StoneMaterial.Friction = 0.8f;
    StoneMaterial.Restitution = 0.2f;
    StoneMaterial.Density = 2500.0f; // kg/m³
    StoneMaterial.SurfaceType = EPhysicalSurface::SurfaceType1;
    PhysicalMaterials.Add(TEXT("Stone"), StoneMaterial);
    
    // Wood material (trees, wooden tools)
    FCore_PhysicalMaterialData WoodMaterial;
    WoodMaterial.MaterialName = TEXT("Wood");
    WoodMaterial.Friction = 0.6f;
    WoodMaterial.Restitution = 0.4f;
    WoodMaterial.Density = 600.0f; // kg/m³
    WoodMaterial.SurfaceType = EPhysicalSurface::SurfaceType2;
    PhysicalMaterials.Add(TEXT("Wood"), WoodMaterial);
    
    // Bone material (dinosaur bones, bone tools)
    FCore_PhysicalMaterialData BoneMaterial;
    BoneMaterial.MaterialName = TEXT("Bone");
    BoneMaterial.Friction = 0.5f;
    BoneMaterial.Restitution = 0.3f;
    BoneMaterial.Density = 1900.0f; // kg/m³
    BoneMaterial.SurfaceType = EPhysicalSurface::SurfaceType3;
    PhysicalMaterials.Add(TEXT("Bone"), BoneMaterial);
    
    // Flesh material (dinosaur bodies)
    FCore_PhysicalMaterialData FleshMaterial;
    FleshMaterial.MaterialName = TEXT("Flesh");
    FleshMaterial.Friction = 0.4f;
    FleshMaterial.Restitution = 0.1f;
    FleshMaterial.Density = 1000.0f; // kg/m³
    FleshMaterial.SurfaceType = EPhysicalSurface::SurfaceType4;
    PhysicalMaterials.Add(TEXT("Flesh"), FleshMaterial);
    
    UE_LOG(LogRigidBodySystem, Log, TEXT("Setup %d physical materials"), PhysicalMaterials.Num());
}

bool UCore_RigidBodySystem::ShouldTrackActor(AActor* Actor) const
{
    if (!Actor) return false;
    
    // Check if actor has physics components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Primitive : PrimitiveComponents)
    {
        if (Primitive && Primitive->IsSimulatingPhysics())
        {
            return true;
        }
    }
    
    return false;
}

void UCore_RigidBodySystem::RegisterRigidBodyActor(AActor* Actor)
{
    if (!Actor || RigidBodyActors.Contains(Actor))
    {
        return;
    }
    
    // Add to tracking
    RigidBodyActors.Add(Actor);
    
    // Get all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Primitive : PrimitiveComponents)
    {
        if (Primitive && Primitive->IsSimulatingPhysics())
        {
            RigidBodyComponents.Add(Primitive);
            
            // Apply default settings if needed
            ApplyRigidBodySettings(Primitive, DefaultRigidBodySettings);
        }
    }
    
    UE_LOG(LogRigidBodySystem, Log, TEXT("Registered rigid body actor: %s"), *Actor->GetName());
}

void UCore_RigidBodySystem::UnregisterRigidBodyActor(AActor* Actor)
{
    if (!Actor) return;
    
    RigidBodyActors.Remove(Actor);
    
    // Remove associated components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Primitive : PrimitiveComponents)
    {
        RigidBodyComponents.Remove(Primitive);
    }
    
    UE_LOG(LogRigidBodySystem, Log, TEXT("Unregistered rigid body actor: %s"), *Actor->GetName());
}

void UCore_RigidBodySystem::ApplyRigidBodySettings(UPrimitiveComponent* Component, const FCore_RigidBodySettings& Settings)
{
    if (!Component) return;
    
    FBodyInstance* BodyInstance = Component->GetBodyInstance();
    if (!BodyInstance) return;
    
    // Apply mass settings
    BodyInstance->SetMassOverride(Settings.Mass, true);
    
    // Apply damping
    BodyInstance->LinearDamping = Settings.LinearDamping;
    BodyInstance->AngularDamping = Settings.AngularDamping;
    
    // Apply physics material properties
    if (BodyInstance->GetSimplePhysicalMaterial())
    {
        BodyInstance->GetSimplePhysicalMaterial()->Friction = Settings.Friction;
        BodyInstance->GetSimplePhysicalMaterial()->Restitution = Settings.Restitution;
    }
    
    // Apply gravity and sleep settings
    BodyInstance->bEnableGravity = Settings.bEnableGravity;
    BodyInstance->bStartAwake = Settings.bStartAwake;
    BodyInstance->SleepFamily = Settings.SleepThreshold < 0.1f ? ESleepFamily::Sensitive : ESleepFamily::Normal;
    
    UE_LOG(LogRigidBodySystem, Log, TEXT("Applied rigid body settings to component: %s"), *Component->GetName());
}

void UCore_RigidBodySystem::UpdateRigidBodyStatistics()
{
    ActiveRigidBodies = 0;
    SleepingRigidBodies = 0;
    
    // Count active and sleeping rigid bodies
    for (UPrimitiveComponent* Component : RigidBodyComponents)
    {
        if (!Component || !IsValid(Component)) continue;
        
        FBodyInstance* BodyInstance = Component->GetBodyInstance();
        if (BodyInstance)
        {
            if (BodyInstance->IsInstanceAwake())
            {
                ActiveRigidBodies++;
            }
            else
            {
                SleepingRigidBodies++;
            }
        }
    }
    
    TotalRigidBodies = ActiveRigidBodies + SleepingRigidBodies;
    
    // Clean up invalid references
    RigidBodyComponents.RemoveAll([](UPrimitiveComponent* Component) {
        return !IsValid(Component);
    });
    
    RigidBodyActors.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
}

void UCore_RigidBodySystem::OptimizeRigidBodyPerformance()
{
    if (!bEnableLODOptimization) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Optimize rigid bodies based on distance
    for (AActor* Actor : RigidBodyActors)
    {
        if (!IsValid(Actor)) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Primitive : PrimitiveComponents)
        {
            if (Primitive && Primitive->IsSimulatingPhysics())
            {
                OptimizeComponentByDistance(Primitive, Distance);
            }
        }
    }
}

void UCore_RigidBodySystem::OptimizeComponentByDistance(UPrimitiveComponent* Component, float Distance)
{
    if (!Component) return;
    
    FBodyInstance* BodyInstance = Component->GetBodyInstance();
    if (!BodyInstance) return;
    
    // Disable physics for very distant objects
    if (Distance > UpdateDistance * 2.0f)
    {
        if (Component->IsSimulatingPhysics())
        {
            Component->SetSimulatePhysics(false);
        }
    }
    // Re-enable physics for objects coming back into range
    else if (Distance < UpdateDistance * 1.5f)
    {
        if (!Component->IsSimulatingPhysics())
        {
            Component->SetSimulatePhysics(true);
        }
    }
    
    // Adjust simulation quality based on distance
    if (Distance > UpdateDistance)
    {
        // Lower quality for distant objects
        BodyInstance->bEnableGravity = false;
        BodyInstance->LinearDamping = 0.5f; // Higher damping to settle faster
    }
    else
    {
        // Full quality for nearby objects
        BodyInstance->bEnableGravity = true;
        BodyInstance->LinearDamping = DefaultRigidBodySettings.LinearDamping;
    }
}

void UCore_RigidBodySystem::ProcessRigidBodyEvents()
{
    // Process collision events and other rigid body events
    for (UPrimitiveComponent* Component : RigidBodyComponents)
    {
        if (!IsValid(Component)) continue;
        
        FBodyInstance* BodyInstance = Component->GetBodyInstance();
        if (!BodyInstance) continue;
        
        // Check for high velocity objects that might need special handling
        FVector LinearVelocity = BodyInstance->GetUnrealWorldVelocity();
        float Speed = LinearVelocity.Size();
        
        if (Speed > 2000.0f) // High speed threshold
        {
            // Enable continuous collision detection for fast objects
            BodyInstance->bUseCCD = true;
            
            // Broadcast high speed event
            OnHighSpeedRigidBody.Broadcast(Component, Speed);
        }
        else if (Speed < 10.0f && BodyInstance->bUseCCD)
        {
            // Disable CCD for slow objects to save performance
            BodyInstance->bUseCCD = false;
        }
    }
}

AActor* UCore_RigidBodySystem::CreateRigidBodyActor(const FVector& Location, const FRotator& Rotation, const FCore_RigidBodySettings& Settings)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogRigidBodySystem, Error, TEXT("CreateRigidBodyActor: No world"));
        return nullptr;
    }
    
    // Spawn a static mesh actor
    AStaticMeshActor* NewActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (!NewActor)
    {
        UE_LOG(LogRigidBodySystem, Error, TEXT("CreateRigidBodyActor: Failed to spawn actor"));
        return nullptr;
    }
    
    // Configure the static mesh component
    UStaticMeshComponent* MeshComponent = NewActor->GetStaticMeshComponent();
    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(true);
        ApplyRigidBodySettings(MeshComponent, Settings);
        
        // Register with the system
        RegisterRigidBodyActor(NewActor);
    }
    
    UE_LOG(LogRigidBodySystem, Log, TEXT("Created rigid body actor at location: %s"), *Location.ToString());
    return NewActor;
}

void UCore_RigidBodySystem::ApplyPhysicalMaterial(UPrimitiveComponent* Component, const FString& MaterialName)
{
    if (!Component)
    {
        UE_LOG(LogRigidBodySystem, Warning, TEXT("ApplyPhysicalMaterial: Component is null"));
        return;
    }
    
    if (!PhysicalMaterials.Contains(MaterialName))
    {
        UE_LOG(LogRigidBodySystem, Warning, TEXT("ApplyPhysicalMaterial: Material '%s' not found"), *MaterialName);
        return;
    }
    
    const FCore_PhysicalMaterialData& MaterialData = PhysicalMaterials[MaterialName];
    FBodyInstance* BodyInstance = Component->GetBodyInstance();
    
    if (BodyInstance && BodyInstance->GetSimplePhysicalMaterial())
    {
        BodyInstance->GetSimplePhysicalMaterial()->Friction = MaterialData.Friction;
        BodyInstance->GetSimplePhysicalMaterial()->Restitution = MaterialData.Restitution;
        BodyInstance->GetSimplePhysicalMaterial()->SurfaceType = MaterialData.SurfaceType;
        
        // Apply density as mass modifier
        float Volume = BodyInstance->GetBodyMass() / 1000.0f; // Estimate volume
        float NewMass = Volume * MaterialData.Density;
        BodyInstance->SetMassOverride(NewMass, true);
    }
    
    UE_LOG(LogRigidBodySystem, Log, TEXT("Applied physical material '%s' to component: %s"), 
           *MaterialName, *Component->GetName());
}

FCore_RigidBodyStats UCore_RigidBodySystem::GetRigidBodyStatistics() const
{
    FCore_RigidBodyStats Stats;
    Stats.ActiveBodies = ActiveRigidBodies;
    Stats.SleepingBodies = SleepingRigidBodies;
    Stats.TotalBodies = TotalRigidBodies;
    Stats.MaxBodies = MaxRigidBodies;
    Stats.BodyUtilization = (MaxRigidBodies > 0) ? (TotalRigidBodies / MaxRigidBodies) * 100.0f : 0.0f;
    return Stats;
}

void UCore_RigidBodySystem::SetMaxRigidBodies(int32 NewMax)
{
    MaxRigidBodies = FMath::Max(NewMax, 10); // Minimum of 10 bodies
    UE_LOG(LogRigidBodySystem, Log, TEXT("Set max rigid bodies to: %d"), MaxRigidBodies);
}

bool UCore_RigidBodySystem::ValidateRigidBodySystem() const
{
    // Validate system integrity
    if (TotalRigidBodies > MaxRigidBodies)
    {
        UE_LOG(LogRigidBodySystem, Warning, TEXT("ValidateRigidBodySystem: Body count exceeds limit (%d/%d)"), 
               TotalRigidBodies, MaxRigidBodies);
        return false;
    }
    
    // Check for null references
    int32 NullReferences = 0;
    for (AActor* Actor : RigidBodyActors)
    {
        if (!IsValid(Actor))
        {
            NullReferences++;
        }
    }
    
    if (NullReferences > 0)
    {
        UE_LOG(LogRigidBodySystem, Warning, TEXT("ValidateRigidBodySystem: %d null actor references found"), 
               NullReferences);
    }
    
    UE_LOG(LogRigidBodySystem, Log, TEXT("Rigid body system validation completed"));
    return true;
}