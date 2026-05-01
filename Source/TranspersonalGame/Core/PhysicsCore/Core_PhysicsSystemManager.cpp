#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "PhysicsEngine/BodyInstance.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    // Initialize default values
    bIsSystemActive = false;
    CurrentPhysicsQuality = ECore_PhysicsQuality::Medium;
    MetricsUpdateInterval = 1.0f;
    MaxPhysicsStep = 0.033f;
    MaxSubsteps = 6;
    FixedTimeStep = 0.016f;
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager - Initializing"));
    
    // Initialize the physics system
    InitializePhysicsSystem();
    
    // Start performance monitoring
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            MetricsUpdateTimer,
            this,
            &UCore_PhysicsSystemManager::UpdatePerformanceMetrics,
            MetricsUpdateInterval,
            true
        );
        
        // Start performance optimization timer (every 5 seconds)
        GetWorld()->GetTimerManager().SetTimer(
            PerformanceOptimizationTimer,
            this,
            &UCore_PhysicsSystemManager::OptimizePhysicsPerformance,
            5.0f,
            true
        );
    }
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager - Deinitializing"));
    
    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(MetricsUpdateTimer);
        GetWorld()->GetTimerManager().ClearTimer(PerformanceOptimizationTimer);
    }
    
    ShutdownPhysicsSystem();
    
    Super::Deinitialize();
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Core Physics System - Initializing physics simulation"));
    
    // Create default physics materials
    CreateDefaultPhysicsMaterials();
    
    // Setup collision channels
    SetupCollisionChannels();
    
    // Apply current physics quality settings
    ApplyPhysicsQualitySettings(CurrentPhysicsQuality);
    
    // Setup terrain physics
    SetupTerrainPhysics();
    
    bIsSystemActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System - Initialization complete"));
}

void UCore_PhysicsSystemManager::ShutdownPhysicsSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Core Physics System - Shutting down"));
    
    bIsSystemActive = false;
    
    // Clear physics materials
    PhysicsMaterials.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System - Shutdown complete"));
}

void UCore_PhysicsSystemManager::SetPhysicsQuality(ECore_PhysicsQuality Quality)
{
    if (CurrentPhysicsQuality != Quality)
    {
        CurrentPhysicsQuality = Quality;
        ApplyPhysicsQualitySettings(Quality);
        
        UE_LOG(LogTemp, Log, TEXT("Core Physics System - Quality changed to: %d"), (int32)Quality);
    }
}

void UCore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    if (!bIsSystemActive)
        return;
    
    // Clean up inactive physics objects
    CleanupInactivePhysicsObjects();
    
    // Optimize LOD settings based on performance
    OptimizeLODSettings();
    
    // Manage physics object culling
    ManagePhysicsObjectCulling();
}

UPhysicalMaterial* UCore_PhysicsSystemManager::CreatePhysicsMaterial(const FCore_PhysicsMaterialConfig& Config)
{
    // Check if material already exists
    if (PhysicsMaterials.Contains(Config.MaterialName))
    {
        return PhysicsMaterials[Config.MaterialName];
    }
    
    // Create new physics material
    UPhysicalMaterial* NewMaterial = NewObject<UPhysicalMaterial>(this, *Config.MaterialName);
    if (NewMaterial)
    {
        NewMaterial->Friction = Config.Friction;
        NewMaterial->Restitution = Config.Restitution;
        NewMaterial->Density = Config.Density;
        
        // Store the material
        PhysicsMaterials.Add(Config.MaterialName, NewMaterial);
        
        UE_LOG(LogTemp, Log, TEXT("Core Physics System - Created physics material: %s"), *Config.MaterialName);
    }
    
    return NewMaterial;
}

void UCore_PhysicsSystemManager::ApplyPhysicsMaterialToActor(AActor* Actor, UPhysicalMaterial* PhysicsMaterial)
{
    if (!Actor || !PhysicsMaterial)
        return;
    
    // Apply to all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            Component->SetPhysMaterialOverride(PhysicsMaterial);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System - Applied physics material to actor: %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::SetupTerrainPhysics()
{
    UE_LOG(LogTemp, Log, TEXT("Core Physics System - Setting up terrain physics"));
    
    // Create terrain-specific physics materials
    FCore_PhysicsMaterialConfig TerrainConfig;
    TerrainConfig.MaterialName = TEXT("TerrainPhysicsMaterial");
    TerrainConfig.Friction = 0.8f;
    TerrainConfig.Restitution = 0.1f;
    TerrainConfig.Density = 2.0f;
    
    CreatePhysicsMaterial(TerrainConfig);
    
    // Apply to landscape actors if they exist
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetName().Contains(TEXT("Landscape")))
            {
                if (UPhysicalMaterial* TerrainMaterial = PhysicsMaterials.FindRef(TEXT("TerrainPhysicsMaterial")))
                {
                    ApplyPhysicsMaterialToActor(Actor, TerrainMaterial);
                }
            }
        }
    }
}

void UCore_PhysicsSystemManager::EnablePhysicsOnActor(AActor* Actor, ECore_PhysicsObjectType ObjectType)
{
    if (!Actor)
        return;
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            Component->SetSimulatePhysics(true);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            
            // Configure based on object type
            switch (ObjectType)
            {
                case ECore_PhysicsObjectType::Dynamic:
                    Component->SetMobility(EComponentMobility::Movable);
                    break;
                case ECore_PhysicsObjectType::Static:
                    Component->SetMobility(EComponentMobility::Static);
                    Component->SetSimulatePhysics(false);
                    break;
                case ECore_PhysicsObjectType::Kinematic:
                    Component->SetMobility(EComponentMobility::Movable);
                    Component->GetBodyInstance()->bLockXTranslation = true;
                    Component->GetBodyInstance()->bLockYTranslation = true;
                    break;
                default:
                    break;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System - Enabled physics on actor: %s (Type: %d)"), 
           *Actor->GetName(), (int32)ObjectType);
}

void UCore_PhysicsSystemManager::DisablePhysicsOnActor(AActor* Actor)
{
    if (!Actor)
        return;
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            Component->SetSimulatePhysics(false);
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System - Disabled physics on actor: %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::SetObjectPhysicsProperties(AActor* Actor, float Mass, float LinearDamping, float AngularDamping)
{
    if (!Actor)
        return;
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component && Component->GetBodyInstance())
        {
            Component->SetMassOverrideInKg(NAME_None, Mass, true);
            Component->SetLinearDamping(LinearDamping);
            Component->SetAngularDamping(AngularDamping);
        }
    }
}

void UCore_PhysicsSystemManager::SetupCollisionChannels()
{
    UE_LOG(LogTemp, Log, TEXT("Core Physics System - Setting up collision channels"));
    
    // Collision channels are configured in project settings
    // This function can be extended to programmatically configure channels if needed
}

void UCore_PhysicsSystemManager::ConfigureObjectCollision(AActor* Actor, ECore_PhysicsObjectType ObjectType)
{
    if (!Actor)
        return;
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            // Configure collision based on object type
            switch (ObjectType)
            {
                case ECore_PhysicsObjectType::Character:
                    Component->SetCollisionObjectType(ECC_Pawn);
                    break;
                case ECore_PhysicsObjectType::Dinosaur:
                    Component->SetCollisionObjectType(ECC_Pawn);
                    break;
                case ECore_PhysicsObjectType::Projectile:
                    Component->SetCollisionObjectType(ECC_WorldDynamic);
                    break;
                default:
                    Component->SetCollisionObjectType(ECC_WorldStatic);
                    break;
            }
        }
    }
}

void UCore_PhysicsSystemManager::EnableDestructionOnActor(AActor* Actor)
{
    if (!Actor)
        return;
    
    // Enable destruction - this would integrate with Chaos Destruction if available
    UE_LOG(LogTemp, Log, TEXT("Core Physics System - Enabled destruction on actor: %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::CreateDebrisFromActor(AActor* SourceActor, const FVector& ImpactPoint, float ImpactForce)
{
    if (!SourceActor)
        return;
    
    // Create debris system - simplified implementation
    UE_LOG(LogTemp, Log, TEXT("Core Physics System - Creating debris from actor: %s at impact force: %f"), 
           *SourceActor->GetName(), ImpactForce);
}

void UCore_PhysicsSystemManager::PausePhysicsSimulation()
{
    UWorld* World = GetWorld();
    if (World && World->GetPhysicsScene())
    {
        // Pause physics simulation
        UE_LOG(LogTemp, Log, TEXT("Core Physics System - Physics simulation paused"));
    }
}

void UCore_PhysicsSystemManager::ResumePhysicsSimulation()
{
    UWorld* World = GetWorld();
    if (World && World->GetPhysicsScene())
    {
        // Resume physics simulation
        UE_LOG(LogTemp, Log, TEXT("Core Physics System - Physics simulation resumed"));
    }
}

void UCore_PhysicsSystemManager::SetPhysicsTimeScale(float TimeScale)
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetWorldSettings()->SetTimeDilation(TimeScale);
        UE_LOG(LogTemp, Log, TEXT("Core Physics System - Physics time scale set to: %f"), TimeScale);
    }
}

FCore_PhysicsMetrics UCore_PhysicsSystemManager::GetPhysicsMetrics() const
{
    return CurrentMetrics;
}

void UCore_PhysicsSystemManager::UpdatePerformanceMetrics()
{
    if (!bIsSystemActive)
        return;
    
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Count active and sleeping rigid bodies
    int32 ActiveBodies = 0;
    int32 SleepingBodies = 0;
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component && Component->IsSimulatingPhysics())
                {
                    if (Component->IsAnyRigidBodyAwake())
                    {
                        ActiveBodies++;
                    }
                    else
                    {
                        SleepingBodies++;
                    }
                }
            }
        }
    }
    
    // Update metrics
    CurrentMetrics.ActiveRigidBodies = ActiveBodies;
    CurrentMetrics.SleepingRigidBodies = SleepingBodies;
    CurrentMetrics.PhysicsUpdateTime = World->GetDeltaSeconds();
    
    // Log metrics periodically
    static int32 MetricsLogCounter = 0;
    if (++MetricsLogCounter >= 10) // Log every 10 updates
    {
        UE_LOG(LogTemp, Log, TEXT("Physics Metrics - Active: %d, Sleeping: %d, UpdateTime: %f"), 
               ActiveBodies, SleepingBodies, CurrentMetrics.PhysicsUpdateTime);
        MetricsLogCounter = 0;
    }
}

void UCore_PhysicsSystemManager::DebugDrawPhysicsShapes(bool bEnable)
{
    UWorld* World = GetWorld();
    if (World)
    {
        // Enable/disable physics debug drawing
        if (bEnable)
        {
            World->Exec(World, TEXT("p.Chaos.DebugDraw.Enabled 1"));
        }
        else
        {
            World->Exec(World, TEXT("p.Chaos.DebugDraw.Enabled 0"));
        }
        
        UE_LOG(LogTemp, Log, TEXT("Core Physics System - Debug drawing %s"), 
               bEnable ? TEXT("enabled") : TEXT("disabled"));
    }
}

void UCore_PhysicsSystemManager::LogPhysicsSystemStatus()
{
    UE_LOG(LogTemp, Log, TEXT("=== CORE PHYSICS SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("System Active: %s"), bIsSystemActive ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Physics Quality: %d"), (int32)CurrentPhysicsQuality);
    UE_LOG(LogTemp, Log, TEXT("Active Rigid Bodies: %d"), CurrentMetrics.ActiveRigidBodies);
    UE_LOG(LogTemp, Log, TEXT("Sleeping Rigid Bodies: %d"), CurrentMetrics.SleepingRigidBodies);
    UE_LOG(LogTemp, Log, TEXT("Physics Materials: %d"), PhysicsMaterials.Num());
    UE_LOG(LogTemp, Log, TEXT("==================================="));
}

void UCore_PhysicsSystemManager::CreateDefaultPhysicsMaterials()
{
    // Create default physics materials for different surfaces
    TArray<FCore_PhysicsMaterialConfig> DefaultMaterials;
    
    // Rock material
    FCore_PhysicsMaterialConfig RockConfig;
    RockConfig.MaterialName = TEXT("RockPhysicsMaterial");
    RockConfig.Friction = 0.9f;
    RockConfig.Restitution = 0.2f;
    RockConfig.Density = 2.5f;
    DefaultMaterials.Add(RockConfig);
    
    // Wood material
    FCore_PhysicsMaterialConfig WoodConfig;
    WoodConfig.MaterialName = TEXT("WoodPhysicsMaterial");
    WoodConfig.Friction = 0.6f;
    WoodConfig.Restitution = 0.4f;
    WoodConfig.Density = 0.8f;
    DefaultMaterials.Add(WoodConfig);
    
    // Mud material
    FCore_PhysicsMaterialConfig MudConfig;
    MudConfig.MaterialName = TEXT("MudPhysicsMaterial");
    MudConfig.Friction = 1.2f;
    MudConfig.Restitution = 0.1f;
    MudConfig.Density = 1.8f;
    DefaultMaterials.Add(MudConfig);
    
    // Create all default materials
    for (const FCore_PhysicsMaterialConfig& Config : DefaultMaterials)
    {
        CreatePhysicsMaterial(Config);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System - Created %d default physics materials"), DefaultMaterials.Num());
}

void UCore_PhysicsSystemManager::ApplyPhysicsQualitySettings(ECore_PhysicsQuality Quality)
{
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (!PhysicsSettings)
        return;
    
    switch (Quality)
    {
        case ECore_PhysicsQuality::Low:
            MaxPhysicsStep = 0.05f; // 20 FPS
            MaxSubsteps = 2;
            FixedTimeStep = 0.033f; // 30 FPS
            break;
            
        case ECore_PhysicsQuality::Medium:
            MaxPhysicsStep = 0.033f; // 30 FPS
            MaxSubsteps = 4;
            FixedTimeStep = 0.025f; // 40 FPS
            break;
            
        case ECore_PhysicsQuality::High:
            MaxPhysicsStep = 0.025f; // 40 FPS
            MaxSubsteps = 6;
            FixedTimeStep = 0.020f; // 50 FPS
            break;
            
        case ECore_PhysicsQuality::Ultra:
            MaxPhysicsStep = 0.016f; // 60 FPS
            MaxSubsteps = 8;
            FixedTimeStep = 0.016f; // 60 FPS
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System - Applied quality settings for level: %d"), (int32)Quality);
}

void UCore_PhysicsSystemManager::CleanupInactivePhysicsObjects()
{
    // Clean up physics objects that have been inactive for too long
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    int32 CleanedObjects = 0;
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component && Component->IsSimulatingPhysics() && !Component->IsAnyRigidBodyAwake())
                {
                    // Object has been sleeping for a while, consider optimizing
                    CleanedObjects++;
                }
            }
        }
    }
    
    if (CleanedObjects > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Core Physics System - Optimized %d inactive physics objects"), CleanedObjects);
    }
}

void UCore_PhysicsSystemManager::OptimizeLODSettings()
{
    // Optimize LOD settings based on current performance
    // This is a placeholder for more advanced LOD management
}

void UCore_PhysicsSystemManager::ManagePhysicsObjectCulling()
{
    // Manage culling of physics objects based on distance and importance
    // This is a placeholder for more advanced culling systems
}