#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "../Eng_SystemManager.h"
#include "../Eng_PerformanceMonitor.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    bIsInitialized = false;
    ManagedActorsCount = 0;
    LastFramePhysicsTime = 0.0f;
    bRegisteredWithSystemManager = false;
    
    // Initialize physics material pointers
    RockPhysicsMaterial = nullptr;
    WoodPhysicsMaterial = nullptr;
    GroundPhysicsMaterial = nullptr;
    WaterPhysicsMaterial = nullptr;
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Initializing physics system..."));
    
    InitializePhysicsSystem();
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Deinitializing physics system..."));
    
    // Clear managed actors
    ManagedActors.Empty();
    ManagedActorsCount = 0;
    bIsInitialized = false;
    
    Super::Deinitialize();
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics system already initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Initializing Core Physics System..."));
    
    // Create physics materials
    CreatePhysicsMaterials();
    
    // Setup collision profiles
    SetupDefaultCollisionProfiles();
    
    // Register with Engine Architect's system manager
    RegisterWithSystemManager();
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Core Physics System initialized successfully"));
}

void UCore_PhysicsSystemManager::EnablePhysicsOnActor(AActor* Actor, bool bSimulatePhysics)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("EnablePhysicsOnActor: Actor is null"));
        return;
    }
    
    // Get the root component
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (!RootPrimitive)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnablePhysicsOnActor: Actor %s has no primitive root component"), *Actor->GetName());
        return;
    }
    
    // Enable physics simulation
    RootPrimitive->SetSimulatePhysics(bSimulatePhysics);
    
    if (bSimulatePhysics)
    {
        // Set default physics properties
        RootPrimitive->SetMassOverrideInKg(NAME_None, 100.0f, true);
        RootPrimitive->SetLinearDamping(0.1f);
        RootPrimitive->SetAngularDamping(0.1f);
        RootPrimitive->SetEnableGravity(true);
        
        // Add to managed actors
        ManagedActors.AddUnique(Actor);
        ManagedActorsCount = ManagedActors.Num();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics %s for actor: %s"), 
           bSimulatePhysics ? TEXT("enabled") : TEXT("disabled"), 
           *Actor->GetName());
}

void UCore_PhysicsSystemManager::SetActorPhysicsMaterial(AActor* Actor, UPhysicalMaterial* PhysicsMaterial)
{
    if (!Actor || !PhysicsMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("SetActorPhysicsMaterial: Invalid actor or material"));
        return;
    }
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (RootPrimitive)
    {
        RootPrimitive->SetPhysMaterialOverride(PhysicsMaterial);
        UE_LOG(LogTemp, Log, TEXT("Applied physics material to actor: %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::ConfigureCollisionForActor(AActor* Actor, ECollisionEnabled::Type CollisionType)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (RootPrimitive)
    {
        RootPrimitive->SetCollisionEnabled(CollisionType);
        UE_LOG(LogTemp, Log, TEXT("Set collision type for actor: %s"), *Actor->GetName());
    }
}

UPhysicalMaterial* UCore_PhysicsSystemManager::GetRockPhysicsMaterial()
{
    if (!RockPhysicsMaterial)
    {
        CreatePhysicsMaterials();
    }
    return RockPhysicsMaterial;
}

UPhysicalMaterial* UCore_PhysicsSystemManager::GetWoodPhysicsMaterial()
{
    if (!WoodPhysicsMaterial)
    {
        CreatePhysicsMaterials();
    }
    return WoodPhysicsMaterial;
}

UPhysicalMaterial* UCore_PhysicsSystemManager::GetGroundPhysicsMaterial()
{
    if (!GroundPhysicsMaterial)
    {
        CreatePhysicsMaterials();
    }
    return GroundPhysicsMaterial;
}

UPhysicalMaterial* UCore_PhysicsSystemManager::GetWaterPhysicsMaterial()
{
    if (!WaterPhysicsMaterial)
    {
        CreatePhysicsMaterials();
    }
    return WaterPhysicsMaterial;
}

void UCore_PhysicsSystemManager::ConfigureDinosaurPhysics(AActor* DinosaurActor, float Mass, float LinearDamping)
{
    if (!DinosaurActor)
    {
        return;
    }
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(DinosaurActor->GetRootComponent());
    if (RootPrimitive)
    {
        // Configure for large prehistoric creatures
        RootPrimitive->SetSimulatePhysics(true);
        RootPrimitive->SetMassOverrideInKg(NAME_None, Mass, true);
        RootPrimitive->SetLinearDamping(LinearDamping);
        RootPrimitive->SetAngularDamping(0.5f); // Higher angular damping for stability
        RootPrimitive->SetEnableGravity(true);
        
        // Use flesh physics material
        if (!RootPrimitive->GetPhysicalMaterial())
        {
            // Create a basic flesh material if none exists
            UPhysicalMaterial* FleshMaterial = NewObject<UPhysicalMaterial>();
            FleshMaterial->Friction = 0.7f;
            FleshMaterial->Restitution = 0.1f;
            FleshMaterial->Density = 1000.0f; // kg/m³ for flesh
            RootPrimitive->SetPhysMaterialOverride(FleshMaterial);
        }
        
        ManagedActors.AddUnique(DinosaurActor);
        ManagedActorsCount = ManagedActors.Num();
        
        UE_LOG(LogTemp, Log, TEXT("Configured dinosaur physics for: %s (Mass: %.1f)"), 
               *DinosaurActor->GetName(), Mass);
    }
}

void UCore_PhysicsSystemManager::SetDinosaurCollisionProfile(AActor* DinosaurActor, FName CollisionProfile)
{
    if (!DinosaurActor)
    {
        return;
    }
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(DinosaurActor->GetRootComponent());
    if (RootPrimitive)
    {
        RootPrimitive->SetCollisionProfileName(CollisionProfile);
        UE_LOG(LogTemp, Log, TEXT("Set collision profile '%s' for dinosaur: %s"), 
               *CollisionProfile.ToString(), *DinosaurActor->GetName());
    }
}

void UCore_PhysicsSystemManager::EnableDestructiblePhysics(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    // Enable physics with destructible properties
    EnablePhysicsOnActor(Actor, true);
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (RootPrimitive)
    {
        // Make it more fragile for destruction
        RootPrimitive->SetMassOverrideInKg(NAME_None, 50.0f, true);
        RootPrimitive->SetLinearDamping(0.2f);
        
        UE_LOG(LogTemp, Log, TEXT("Enabled destructible physics for: %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::ConfigureVegetationPhysics(AActor* VegetationActor)
{
    if (!VegetationActor)
    {
        return;
    }
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(VegetationActor->GetRootComponent());
    if (RootPrimitive)
    {
        // Vegetation should be lightweight and responsive
        RootPrimitive->SetSimulatePhysics(false); // Usually kinematic
        RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        RootPrimitive->SetCollisionProfileName("OverlapAll");
        
        // Apply wood physics material
        if (WoodPhysicsMaterial)
        {
            RootPrimitive->SetPhysMaterialOverride(WoodPhysicsMaterial);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Configured vegetation physics for: %s"), *VegetationActor->GetName());
    }
}

int32 UCore_PhysicsSystemManager::GetActivePhysicsBodies()
{
    // Clean up invalid weak pointers
    ManagedActors.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor) {
        return !WeakActor.IsValid();
    });
    
    ManagedActorsCount = ManagedActors.Num();
    return ManagedActorsCount;
}

float UCore_PhysicsSystemManager::GetPhysicsSimulationTime()
{
    UpdatePhysicsMetrics();
    return LastFramePhysicsTime;
}

void UCore_PhysicsSystemManager::RegisterWithSystemManager()
{
    if (bRegisteredWithSystemManager)
    {
        return;
    }
    
    // Try to register with Engine Architect's system manager
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UEngSystemManager* SystemManager = GameInstance->GetSubsystem<UEngSystemManager>())
        {
            // Register physics system as initialized
            UE_LOG(LogTemp, Warning, TEXT("Registering Physics System with Engine Architect System Manager"));
            bRegisteredWithSystemManager = true;
        }
    }
}

bool UCore_PhysicsSystemManager::IsPhysicsSystemReady()
{
    return bIsInitialized && bRegisteredWithSystemManager;
}

void UCore_PhysicsSystemManager::CreatePhysicsMaterials()
{
    UE_LOG(LogTemp, Log, TEXT("Creating prehistoric physics materials..."));
    
    // Rock material - hard, low friction
    RockPhysicsMaterial = NewObject<UPhysicalMaterial>();
    RockPhysicsMaterial->Friction = 0.8f;
    RockPhysicsMaterial->Restitution = 0.2f;
    RockPhysicsMaterial->Density = 2500.0f; // kg/m³ for rock
    
    // Wood material - medium properties
    WoodPhysicsMaterial = NewObject<UPhysicalMaterial>();
    WoodPhysicsMaterial->Friction = 0.6f;
    WoodPhysicsMaterial->Restitution = 0.3f;
    WoodPhysicsMaterial->Density = 600.0f; // kg/m³ for wood
    
    // Ground material - high friction, low bounce
    GroundPhysicsMaterial = NewObject<UPhysicalMaterial>();
    GroundPhysicsMaterial->Friction = 1.0f;
    GroundPhysicsMaterial->Restitution = 0.1f;
    GroundPhysicsMaterial->Density = 1800.0f; // kg/m³ for soil
    
    // Water material - very low friction
    WaterPhysicsMaterial = NewObject<UPhysicalMaterial>();
    WaterPhysicsMaterial->Friction = 0.1f;
    WaterPhysicsMaterial->Restitution = 0.0f;
    WaterPhysicsMaterial->Density = 1000.0f; // kg/m³ for water
    
    UE_LOG(LogTemp, Log, TEXT("Physics materials created successfully"));
}

void UCore_PhysicsSystemManager::SetupDefaultCollisionProfiles()
{
    // This would typically set up custom collision profiles
    // For now, we'll use the default UE5 profiles
    UE_LOG(LogTemp, Log, TEXT("Using default collision profiles"));
}

void UCore_PhysicsSystemManager::UpdatePhysicsMetrics()
{
    // Basic performance tracking
    if (UWorld* World = GetWorld())
    {
        // Get physics scene stats (simplified)
        LastFramePhysicsTime = World->GetDeltaSeconds() * 1000.0f; // Convert to ms
    }
}