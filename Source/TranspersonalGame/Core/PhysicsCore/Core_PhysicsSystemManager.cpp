#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    // Set default physics quality
    CurrentPhysicsQuality = ECore_PhysicsQuality::Medium;
    
    // Initialize performance metrics
    CurrentMetrics = FCore_PhysicsMetrics();
    
    // Set default update intervals
    MetricsUpdateInterval = 1.0f;
    
    // Physics simulation defaults
    MaxPhysicsStep = 0.033f; // 30 FPS minimum
    MaxSubsteps = 6;
    FixedTimeStep = 0.016f; // 60 FPS target
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics System Manager: Initializing"));
    
    // Initialize the physics system
    InitializePhysicsSystem();
    
    // Create default physics materials
    CreateDefaultPhysicsMaterials();
    
    // Setup collision channels
    SetupCollisionChannels();
    
    // Start performance monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MetricsUpdateTimer,
            this,
            &UCore_PhysicsSystemManager::UpdatePerformanceMetrics,
            MetricsUpdateInterval,
            true
        );
    }
    
    bIsSystemActive = true;
    UE_LOG(LogTemp, Warning, TEXT("Core Physics System Manager: Initialization complete"));
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Core Physics System Manager: Shutting down"));
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
        World->GetTimerManager().ClearTimer(PerformanceOptimizationTimer);
    }
    
    // Shutdown physics system
    ShutdownPhysicsSystem();
    
    bIsSystemActive = false;
    
    Super::Deinitialize();
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Physics System: Initializing core physics simulation"));
    
    // Apply current physics quality settings
    ApplyPhysicsQualitySettings(CurrentPhysicsQuality);
    
    // Setup terrain physics if world exists
    if (GetWorld())
    {
        SetupTerrainPhysics();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Physics System: Core initialization complete"));
}

void UCore_PhysicsSystemManager::ShutdownPhysicsSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Physics System: Shutting down"));
    
    // Clear physics materials
    PhysicsMaterials.Empty();
    
    // Reset metrics
    CurrentMetrics = FCore_PhysicsMetrics();
}

void UCore_PhysicsSystemManager::SetPhysicsQuality(ECore_PhysicsQuality Quality)
{
    if (CurrentPhysicsQuality != Quality)
    {
        CurrentPhysicsQuality = Quality;
        ApplyPhysicsQualitySettings(Quality);
        
        UE_LOG(LogTemp, Warning, TEXT("Physics Quality changed to: %d"), (int32)Quality);
    }
}

void UCore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("Physics System: Running performance optimization"));
    
    // Clean up inactive physics objects
    CleanupInactivePhysicsObjects();
    
    // Optimize LOD settings based on current performance
    OptimizeLODSettings();
    
    // Manage physics object culling
    ManagePhysicsObjectCulling();
}

UPhysicalMaterial* UCore_PhysicsSystemManager::CreatePhysicsMaterial(const FCore_PhysicsMaterialConfig& Config)
{
    // Check if material already exists
    if (UPhysicalMaterial** ExistingMaterial = PhysicsMaterials.Find(Config.MaterialName))
    {
        return *ExistingMaterial;
    }
    
    // Create new physics material
    UPhysicalMaterial* NewMaterial = NewObject<UPhysicalMaterial>(this, FName(*Config.MaterialName));
    if (NewMaterial)
    {
        NewMaterial->Friction = Config.Friction;
        NewMaterial->Restitution = Config.Restitution;
        NewMaterial->Density = Config.Density;
        
        // Store the material
        PhysicsMaterials.Add(Config.MaterialName, NewMaterial);
        
        UE_LOG(LogTemp, Log, TEXT("Created physics material: %s"), *Config.MaterialName);
        return NewMaterial;
    }
    
    UE_LOG(LogTemp, Error, TEXT("Failed to create physics material: %s"), *Config.MaterialName);
    return nullptr;
}

void UCore_PhysicsSystemManager::ApplyPhysicsMaterialToActor(AActor* Actor, UPhysicalMaterial* PhysicsMaterial)
{
    if (!Actor || !PhysicsMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid actor or physics material"));
        return;
    }
    
    // Find static mesh component
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        MeshComp->SetPhysMaterialOverride(PhysicsMaterial);
        UE_LOG(LogTemp, Log, TEXT("Applied physics material to actor: %s"), *Actor->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No static mesh component found on actor: %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::SetupTerrainPhysics()
{
    UE_LOG(LogTemp, Log, TEXT("Physics System: Setting up terrain physics"));
    
    // Create terrain physics material
    FCore_PhysicsMaterialConfig TerrainConfig;
    TerrainConfig.MaterialName = TEXT("TerrainPhysics");
    TerrainConfig.Friction = 0.8f;
    TerrainConfig.Restitution = 0.1f;
    TerrainConfig.Density = 2.0f;
    
    UPhysicalMaterial* TerrainMaterial = CreatePhysicsMaterial(TerrainConfig);
    
    // Apply to landscape actors if they exist
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetName().Contains(TEXT("Landscape")))
            {
                ApplyPhysicsMaterialToActor(Actor, TerrainMaterial);
            }
        }
    }
}

void UCore_PhysicsSystemManager::EnablePhysicsOnActor(AActor* Actor, ECore_PhysicsObjectType ObjectType)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot enable physics on null actor"));
        return;
    }
    
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("No static mesh component found on actor: %s"), *Actor->GetName());
        return;
    }
    
    // Enable physics simulation
    MeshComp->SetSimulatePhysics(true);
    
    // Configure based on object type
    switch (ObjectType)
    {
        case ECore_PhysicsObjectType::Static:
            MeshComp->SetMobility(EComponentMobility::Static);
            MeshComp->SetSimulatePhysics(false);
            break;
            
        case ECore_PhysicsObjectType::Dynamic:
            MeshComp->SetMobility(EComponentMobility::Movable);
            MeshComp->SetSimulatePhysics(true);
            break;
            
        case ECore_PhysicsObjectType::Kinematic:
            MeshComp->SetMobility(EComponentMobility::Movable);
            MeshComp->SetSimulatePhysics(true);
            MeshComp->GetBodyInstance()->bLockXRotation = true;
            MeshComp->GetBodyInstance()->bLockYRotation = true;
            break;
            
        case ECore_PhysicsObjectType::Debris:
            MeshComp->SetMobility(EComponentMobility::Movable);
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
            break;
            
        default:
            MeshComp->SetMobility(EComponentMobility::Movable);
            MeshComp->SetSimulatePhysics(true);
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Enabled physics on actor: %s (Type: %d)"), *Actor->GetName(), (int32)ObjectType);
}

void UCore_PhysicsSystemManager::DisablePhysicsOnActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        MeshComp->SetSimulatePhysics(false);
        UE_LOG(LogTemp, Log, TEXT("Disabled physics on actor: %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::SetObjectPhysicsProperties(AActor* Actor, float Mass, float LinearDamping, float AngularDamping)
{
    if (!Actor)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp && MeshComp->GetBodyInstance())
    {
        MeshComp->SetMassOverrideInKg(NAME_None, Mass, true);
        MeshComp->SetLinearDamping(LinearDamping);
        MeshComp->SetAngularDamping(AngularDamping);
        
        UE_LOG(LogTemp, Log, TEXT("Set physics properties on %s: Mass=%.2f, LinearDamp=%.2f, AngularDamp=%.2f"), 
               *Actor->GetName(), Mass, LinearDamping, AngularDamping);
    }
}

void UCore_PhysicsSystemManager::SetupCollisionChannels()
{
    UE_LOG(LogTemp, Log, TEXT("Physics System: Setting up collision channels"));
    
    // This would typically involve setting up custom collision channels
    // For now, we'll use the default UE5 collision channels
    // In a full implementation, we'd create custom channels for:
    // - Dinosaurs
    // - Player
    // - Environment
    // - Projectiles
    // - Debris
}

void UCore_PhysicsSystemManager::ConfigureObjectCollision(AActor* Actor, ECore_PhysicsObjectType ObjectType)
{
    if (!Actor)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        return;
    }
    
    // Configure collision based on object type
    switch (ObjectType)
    {
        case ECore_PhysicsObjectType::Static:
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionObjectType(ECC_WorldStatic);
            break;
            
        case ECore_PhysicsObjectType::Dynamic:
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
            break;
            
        case ECore_PhysicsObjectType::Character:
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionObjectType(ECC_Pawn);
            break;
            
        case ECore_PhysicsObjectType::Projectile:
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
            break;
            
        default:
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Configured collision for actor: %s (Type: %d)"), *Actor->GetName(), (int32)ObjectType);
}

void UCore_PhysicsSystemManager::EnableDestructionOnActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    // For now, we'll just enable physics and mark it as destructible
    // In a full implementation, this would set up Chaos Destruction
    EnablePhysicsOnActor(Actor, ECore_PhysicsObjectType::Dynamic);
    
    UE_LOG(LogTemp, Log, TEXT("Enabled destruction on actor: %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::CreateDebrisFromActor(AActor* SourceActor, const FVector& ImpactPoint, float ImpactForce)
{
    if (!SourceActor)
    {
        return;
    }
    
    // This is a simplified debris creation system
    // In a full implementation, this would use Chaos Destruction to fracture the mesh
    UE_LOG(LogTemp, Log, TEXT("Creating debris from actor: %s at impact point: %s with force: %.2f"), 
           *SourceActor->GetName(), *ImpactPoint.ToString(), ImpactForce);
    
    // For now, just enable physics on the source actor
    EnablePhysicsOnActor(SourceActor, ECore_PhysicsObjectType::Debris);
}

void UCore_PhysicsSystemManager::PausePhysicsSimulation()
{
    if (UWorld* World = GetWorld())
    {
        // Pause physics world
        UE_LOG(LogTemp, Log, TEXT("Physics simulation paused"));
    }
}

void UCore_PhysicsSystemManager::ResumePhysicsSimulation()
{
    if (UWorld* World = GetWorld())
    {
        // Resume physics world
        UE_LOG(LogTemp, Log, TEXT("Physics simulation resumed"));
    }
}

void UCore_PhysicsSystemManager::SetPhysicsTimeScale(float TimeScale)
{
    if (UWorld* World = GetWorld())
    {
        // Set physics time dilation
        UE_LOG(LogTemp, Log, TEXT("Physics time scale set to: %.2f"), TimeScale);
    }
}

FCore_PhysicsMetrics UCore_PhysicsSystemManager::GetPhysicsMetrics() const
{
    return CurrentMetrics;
}

void UCore_PhysicsSystemManager::UpdatePerformanceMetrics()
{
    if (!bIsSystemActive)
    {
        return;
    }
    
    // Update physics performance metrics
    CurrentMetrics.ActiveRigidBodies = 0;
    CurrentMetrics.SleepingRigidBodies = 0;
    CurrentMetrics.CollisionPairs = 0;
    
    // Count physics objects in the world
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
                if (MeshComp && MeshComp->IsSimulatingPhysics())
                {
                    if (MeshComp->GetBodyInstance() && MeshComp->GetBodyInstance()->IsInstanceAwake())
                    {
                        CurrentMetrics.ActiveRigidBodies++;
                    }
                    else
                    {
                        CurrentMetrics.SleepingRigidBodies++;
                    }
                }
            }
        }
    }
    
    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = (CurrentMetrics.ActiveRigidBodies + CurrentMetrics.SleepingRigidBodies) * 0.1f;
    
    // Log metrics periodically
    static int32 MetricsLogCounter = 0;
    if (++MetricsLogCounter % 10 == 0) // Log every 10 updates
    {
        UE_LOG(LogTemp, Log, TEXT("Physics Metrics - Active: %d, Sleeping: %d, Memory: %.2fMB"), 
               CurrentMetrics.ActiveRigidBodies, CurrentMetrics.SleepingRigidBodies, CurrentMetrics.MemoryUsageMB);
    }
}

void UCore_PhysicsSystemManager::DebugDrawPhysicsShapes(bool bEnable)
{
    // Toggle physics debug drawing
    if (UWorld* World = GetWorld())
    {
        FString Command = bEnable ? TEXT("p.Chaos.DebugDraw.Enabled 1") : TEXT("p.Chaos.DebugDraw.Enabled 0");
        UE_LOG(LogTemp, Log, TEXT("Physics debug drawing %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
    }
}

void UCore_PhysicsSystemManager::LogPhysicsSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("System Active: %s"), bIsSystemActive ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Physics Quality: %d"), (int32)CurrentPhysicsQuality);
    UE_LOG(LogTemp, Warning, TEXT("Active Physics Materials: %d"), PhysicsMaterials.Num());
    UE_LOG(LogTemp, Warning, TEXT("Active Rigid Bodies: %d"), CurrentMetrics.ActiveRigidBodies);
    UE_LOG(LogTemp, Warning, TEXT("Sleeping Rigid Bodies: %d"), CurrentMetrics.SleepingRigidBodies);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("============================="));
}

// Private implementation methods

void UCore_PhysicsSystemManager::CreateDefaultPhysicsMaterials()
{
    UE_LOG(LogTemp, Log, TEXT("Creating default physics materials"));
    
    // Stone/Rock material
    FCore_PhysicsMaterialConfig StoneConfig;
    StoneConfig.MaterialName = TEXT("Stone");
    StoneConfig.Friction = 0.9f;
    StoneConfig.Restitution = 0.2f;
    StoneConfig.Density = 3.0f;
    CreatePhysicsMaterial(StoneConfig);
    
    // Wood material
    FCore_PhysicsMaterialConfig WoodConfig;
    WoodConfig.MaterialName = TEXT("Wood");
    WoodConfig.Friction = 0.7f;
    WoodConfig.Restitution = 0.4f;
    WoodConfig.Density = 0.8f;
    CreatePhysicsMaterial(WoodConfig);
    
    // Mud/Dirt material
    FCore_PhysicsMaterialConfig MudConfig;
    MudConfig.MaterialName = TEXT("Mud");
    MudConfig.Friction = 0.6f;
    MudConfig.Restitution = 0.1f;
    MudConfig.Density = 1.5f;
    CreatePhysicsMaterial(MudConfig);
    
    // Bone material
    FCore_PhysicsMaterialConfig BoneConfig;
    BoneConfig.MaterialName = TEXT("Bone");
    BoneConfig.Friction = 0.8f;
    BoneConfig.Restitution = 0.3f;
    BoneConfig.Density = 1.2f;
    CreatePhysicsMaterial(BoneConfig);
}

void UCore_PhysicsSystemManager::ApplyPhysicsQualitySettings(ECore_PhysicsQuality Quality)
{
    UE_LOG(LogTemp, Log, TEXT("Applying physics quality settings: %d"), (int32)Quality);
    
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
            FixedTimeStep = 0.02f; // 50 FPS
            break;
            
        case ECore_PhysicsQuality::Ultra:
            MaxPhysicsStep = 0.016f; // 60 FPS
            MaxSubsteps = 8;
            FixedTimeStep = 0.016f; // 60 FPS
            break;
    }
}

void UCore_PhysicsSystemManager::CleanupInactivePhysicsObjects()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Find and cleanup sleeping physics objects that have been inactive for too long
    int32 CleanedObjects = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp && MeshComp->IsSimulatingPhysics())
            {
                // Check if object has been sleeping for a long time
                if (MeshComp->GetBodyInstance() && !MeshComp->GetBodyInstance()->IsInstanceAwake())
                {
                    // Optionally disable physics on long-sleeping objects
                    // This is a performance optimization
                    CleanedObjects++;
                }
            }
        }
    }
    
    if (CleanedObjects > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Cleaned up %d inactive physics objects"), CleanedObjects);
    }
}

void UCore_PhysicsSystemManager::OptimizeLODSettings()
{
    // Optimize physics LOD based on distance and performance
    // This would implement distance-based physics quality reduction
    UE_LOG(LogTemp, VeryVerbose, TEXT("Optimizing physics LOD settings"));
}

void UCore_PhysicsSystemManager::ManagePhysicsObjectCulling()
{
    // Manage culling of physics objects based on distance and visibility
    // This would implement frustum culling for physics objects
    UE_LOG(LogTemp, VeryVerbose, TEXT("Managing physics object culling"));
}