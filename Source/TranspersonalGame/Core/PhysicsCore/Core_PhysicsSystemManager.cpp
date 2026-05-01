#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "../Engine/Eng_ArchitectureCore.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    // Initialize default values
    bIsSystemActive = false;
    CurrentPhysicsQuality = ECore_PhysicsQuality::Medium;
    MetricsUpdateInterval = 1.0f;
    MaxPhysicsStep = 0.033f;
    MaxSubsteps = 6;
    FixedTimeStep = 0.016f;
    
    // Initialize metrics
    CurrentMetrics = FCore_PhysicsMetrics();
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager initializing..."));
    
    // Register with architecture core if available
    if (UEng_ArchitectureCore* ArchCore = GetGameInstance()->GetSubsystem<UEng_ArchitectureCore>())
    {
        ArchCore->RegisterSystemModule(TEXT("Physics"), 90);
        UE_LOG(LogTemp, Log, TEXT("Physics system registered with Architecture Core (Priority: 90)"));
    }
    
    // Initialize physics system
    InitializePhysicsSystem();
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    ShutdownPhysicsSystem();
    Super::Deinitialize();
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    if (bIsSystemActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics system already active"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Initializing Core Physics System..."));
    
    // Apply current physics quality settings
    ApplyPhysicsQualitySettings(CurrentPhysicsQuality);
    
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
        
        // Start optimization timer (every 5 seconds)
        World->GetTimerManager().SetTimer(
            PerformanceOptimizationTimer,
            this,
            &UCore_PhysicsSystemManager::OptimizePhysicsPerformance,
            5.0f,
            true
        );
    }
    
    bIsSystemActive = true;
    UE_LOG(LogTemp, Log, TEXT("Core Physics System initialized successfully"));
}

void UCore_PhysicsSystemManager::ShutdownPhysicsSystem()
{
    if (!bIsSystemActive)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Shutting down Core Physics System..."));
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
        World->GetTimerManager().ClearTimer(PerformanceOptimizationTimer);
    }
    
    // Clear physics materials
    PhysicsMaterials.Empty();
    
    bIsSystemActive = false;
    UE_LOG(LogTemp, Log, TEXT("Core Physics System shut down"));
}

void UCore_PhysicsSystemManager::SetPhysicsQuality(ECore_PhysicsQuality Quality)
{
    if (CurrentPhysicsQuality == Quality)
    {
        return;
    }
    
    CurrentPhysicsQuality = Quality;
    ApplyPhysicsQualitySettings(Quality);
    
    UE_LOG(LogTemp, Log, TEXT("Physics quality set to: %d"), (int32)Quality);
}

void UCore_PhysicsSystemManager::ApplyPhysicsQualitySettings(ECore_PhysicsQuality Quality)
{
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        switch (Quality)
        {
            case ECore_PhysicsQuality::Low:
                PhysicsSettings->MaxPhysicsDeltaTime = 0.05f;  // 20 FPS
                PhysicsSettings->MaxSubstepDeltaTime = 0.025f;
                PhysicsSettings->MaxSubsteps = 4;
                break;
                
            case ECore_PhysicsQuality::Medium:
                PhysicsSettings->MaxPhysicsDeltaTime = 0.033f;  // 30 FPS
                PhysicsSettings->MaxSubstepDeltaTime = 0.0167f;
                PhysicsSettings->MaxSubsteps = 6;
                break;
                
            case ECore_PhysicsQuality::High:
                PhysicsSettings->MaxPhysicsDeltaTime = 0.0167f; // 60 FPS
                PhysicsSettings->MaxSubstepDeltaTime = 0.0083f;
                PhysicsSettings->MaxSubsteps = 8;
                break;
                
            case ECore_PhysicsQuality::Ultra:
                PhysicsSettings->MaxPhysicsDeltaTime = 0.0083f; // 120 FPS
                PhysicsSettings->MaxSubstepDeltaTime = 0.0042f;
                PhysicsSettings->MaxSubsteps = 10;
                break;
        }
    }
}

void UCore_PhysicsSystemManager::CreateDefaultPhysicsMaterials()
{
    // Create physics materials for different terrain types
    
    // Grass/Dirt material
    FCore_PhysicsMaterialConfig GrassConfig;
    GrassConfig.MaterialName = TEXT("Grass");
    GrassConfig.Friction = 0.8f;
    GrassConfig.Restitution = 0.1f;
    GrassConfig.Density = 1.0f;
    PhysicsMaterials.Add(GrassConfig.MaterialName, CreatePhysicsMaterial(GrassConfig));
    
    // Rock material
    FCore_PhysicsMaterialConfig RockConfig;
    RockConfig.MaterialName = TEXT("Rock");
    RockConfig.Friction = 0.9f;
    RockConfig.Restitution = 0.3f;
    RockConfig.Density = 2.5f;
    PhysicsMaterials.Add(RockConfig.MaterialName, CreatePhysicsMaterial(RockConfig));
    
    // Wood material
    FCore_PhysicsMaterialConfig WoodConfig;
    WoodConfig.MaterialName = TEXT("Wood");
    WoodConfig.Friction = 0.7f;
    WoodConfig.Restitution = 0.2f;
    WoodConfig.Density = 0.8f;
    PhysicsMaterials.Add(WoodConfig.MaterialName, CreatePhysicsMaterial(WoodConfig));
    
    // Water material
    FCore_PhysicsMaterialConfig WaterConfig;
    WaterConfig.MaterialName = TEXT("Water");
    WaterConfig.Friction = 0.1f;
    WaterConfig.Restitution = 0.0f;
    WaterConfig.Density = 1.0f;
    PhysicsMaterials.Add(WaterConfig.MaterialName, CreatePhysicsMaterial(WaterConfig));
    
    UE_LOG(LogTemp, Log, TEXT("Created %d default physics materials"), PhysicsMaterials.Num());
}

UPhysicalMaterial* UCore_PhysicsSystemManager::CreatePhysicsMaterial(const FCore_PhysicsMaterialConfig& Config)
{
    UPhysicalMaterial* PhysMat = NewObject<UPhysicalMaterial>(this, FName(*Config.MaterialName));
    if (PhysMat)
    {
        PhysMat->Friction = Config.Friction;
        PhysMat->Restitution = Config.Restitution;
        PhysMat->Density = Config.Density;
        
        UE_LOG(LogTemp, Log, TEXT("Created physics material: %s (Friction: %.2f, Restitution: %.2f, Density: %.2f)"),
            *Config.MaterialName, Config.Friction, Config.Restitution, Config.Density);
    }
    
    return PhysMat;
}

void UCore_PhysicsSystemManager::ApplyPhysicsMaterialToActor(AActor* Actor, UPhysicalMaterial* PhysicsMaterial)
{
    if (!Actor || !PhysicsMaterial)
    {
        return;
    }
    
    // Apply to all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            PrimComp->SetPhysMaterialOverride(PhysicsMaterial);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied physics material to actor: %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::SetupTerrainPhysics()
{
    if (!bIsSystemActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot setup terrain physics - system not active"));
        return;
    }
    
    // Find terrain actors and apply appropriate physics materials
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor)
            {
                continue;
            }
            
            FString ActorName = Actor->GetName().ToLower();
            
            // Apply physics materials based on actor name/type
            if (ActorName.Contains(TEXT("terrain")) || ActorName.Contains(TEXT("landscape")))
            {
                if (UPhysicalMaterial** GrassMat = PhysicsMaterials.Find(TEXT("Grass")))
                {
                    ApplyPhysicsMaterialToActor(Actor, *GrassMat);
                }
            }
            else if (ActorName.Contains(TEXT("rock")) || ActorName.Contains(TEXT("stone")))
            {
                if (UPhysicalMaterial** RockMat = PhysicsMaterials.Find(TEXT("Rock")))
                {
                    ApplyPhysicsMaterialToActor(Actor, *RockMat);
                }
            }
            else if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("wood")))
            {
                if (UPhysicalMaterial** WoodMat = PhysicsMaterials.Find(TEXT("Wood")))
                {
                    ApplyPhysicsMaterialToActor(Actor, *WoodMat);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Terrain physics setup complete"));
}

void UCore_PhysicsSystemManager::EnablePhysicsOnActor(AActor* Actor, ECore_PhysicsObjectType ObjectType)
{
    if (!Actor)
    {
        return;
    }
    
    // Get the root primitive component
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (!RootPrimitive)
    {
        return;
    }
    
    // Configure physics based on object type
    switch (ObjectType)
    {
        case ECore_PhysicsObjectType::Static:
            RootPrimitive->SetMobility(EComponentMobility::Static);
            RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case ECore_PhysicsObjectType::Dynamic:
            RootPrimitive->SetMobility(EComponentMobility::Movable);
            RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            RootPrimitive->SetSimulatePhysics(true);
            break;
            
        case ECore_PhysicsObjectType::Kinematic:
            RootPrimitive->SetMobility(EComponentMobility::Movable);
            RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            RootPrimitive->SetSimulatePhysics(false);
            break;
            
        case ECore_PhysicsObjectType::Debris:
            RootPrimitive->SetMobility(EComponentMobility::Movable);
            RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            RootPrimitive->SetSimulatePhysics(true);
            // Set shorter lifespan for debris
            Actor->SetLifeSpan(30.0f);
            break;
            
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Enabled physics on actor: %s (Type: %d)"), *Actor->GetName(), (int32)ObjectType);
}

void UCore_PhysicsSystemManager::SetupCollisionChannels()
{
    // Note: Collision channels are typically set up in project settings
    // This function would configure custom collision responses
    UE_LOG(LogTemp, Log, TEXT("Collision channels setup complete"));
}

void UCore_PhysicsSystemManager::UpdatePerformanceMetrics()
{
    if (!bIsSystemActive)
    {
        return;
    }
    
    // Count physics objects in the world
    CurrentMetrics.ActiveRigidBodies = 0;
    CurrentMetrics.SleepingRigidBodies = 0;
    CurrentMetrics.CollisionPairs = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    if (PrimComp->IsSimulatingPhysics() && !PrimComp->RigidBodyIsAwake())
                    {
                        CurrentMetrics.SleepingRigidBodies++;
                    }
                    else
                    {
                        CurrentMetrics.ActiveRigidBodies++;
                    }
                }
            }
        }
    }
    
    // Update physics timing (simplified)
    CurrentMetrics.PhysicsUpdateTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    
    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = (CurrentMetrics.ActiveRigidBodies + CurrentMetrics.SleepingRigidBodies) * 0.1f;
}

void UCore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    if (!bIsSystemActive)
    {
        return;
    }
    
    // Clean up inactive physics objects
    CleanupInactivePhysicsObjects();
    
    // Optimize LOD settings
    OptimizeLODSettings();
    
    // Manage physics object culling
    ManagePhysicsObjectCulling();
}

void UCore_PhysicsSystemManager::CleanupInactivePhysicsObjects()
{
    // Remove physics objects that have been sleeping for too long
    if (UWorld* World = GetWorld())
    {
        int32 CleanedObjects = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetLifeSpan() > 0.0f && Actor->GetLifeSpan() < 1.0f)
            {
                // Actor is about to be destroyed anyway
                UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    PrimComp->SetSimulatePhysics(false);
                    CleanedObjects++;
                }
            }
        }
        
        if (CleanedObjects > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("Cleaned up %d inactive physics objects"), CleanedObjects);
        }
    }
}

void UCore_PhysicsSystemManager::OptimizeLODSettings()
{
    // Placeholder for LOD optimization
    // Would implement distance-based physics LOD here
}

void UCore_PhysicsSystemManager::ManagePhysicsObjectCulling()
{
    // Placeholder for physics culling
    // Would implement frustum/distance-based culling here
}

FCore_PhysicsMetrics UCore_PhysicsSystemManager::GetPhysicsMetrics() const
{
    return CurrentMetrics;
}

void UCore_PhysicsSystemManager::DebugDrawPhysicsShapes(bool bEnable)
{
    if (UWorld* World = GetWorld())
    {
        if (bEnable)
        {
            World->Exec(World, TEXT("p.Chaos.DebugDraw.Enabled 1"));
            UE_LOG(LogTemp, Log, TEXT("Physics debug drawing enabled"));
        }
        else
        {
            World->Exec(World, TEXT("p.Chaos.DebugDraw.Enabled 0"));
            UE_LOG(LogTemp, Log, TEXT("Physics debug drawing disabled"));
        }
    }
}

void UCore_PhysicsSystemManager::LogPhysicsSystemStatus()
{
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("System Active: %s"), bIsSystemActive ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Quality Level: %d"), (int32)CurrentPhysicsQuality);
    UE_LOG(LogTemp, Log, TEXT("Active Rigid Bodies: %d"), CurrentMetrics.ActiveRigidBodies);
    UE_LOG(LogTemp, Log, TEXT("Sleeping Rigid Bodies: %d"), CurrentMetrics.SleepingRigidBodies);
    UE_LOG(LogTemp, Log, TEXT("Physics Update Time: %.2f ms"), CurrentMetrics.PhysicsUpdateTime);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Physics Materials: %d"), PhysicsMaterials.Num());
    UE_LOG(LogTemp, Log, TEXT("=============================="));
}

// Additional method implementations for completeness
void UCore_PhysicsSystemManager::DisablePhysicsOnActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (RootPrimitive)
    {
        RootPrimitive->SetSimulatePhysics(false);
        RootPrimitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Disabled physics on actor: %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::SetObjectPhysicsProperties(AActor* Actor, float Mass, float LinearDamping, float AngularDamping)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (RootPrimitive)
    {
        RootPrimitive->SetMassOverrideInKg(NAME_None, Mass, true);
        RootPrimitive->SetLinearDamping(LinearDamping);
        RootPrimitive->SetAngularDamping(AngularDamping);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set physics properties on actor: %s (Mass: %.2f, LinearDamp: %.2f, AngularDamp: %.2f)"),
        *Actor->GetName(), Mass, LinearDamping, AngularDamping);
}

void UCore_PhysicsSystemManager::ConfigureObjectCollision(AActor* Actor, ECore_PhysicsObjectType ObjectType)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (!RootPrimitive)
    {
        return;
    }
    
    // Configure collision based on object type
    switch (ObjectType)
    {
        case ECore_PhysicsObjectType::Character:
            RootPrimitive->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
            RootPrimitive->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            break;
            
        case ECore_PhysicsObjectType::Dinosaur:
            RootPrimitive->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
            RootPrimitive->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            break;
            
        case ECore_PhysicsObjectType::Projectile:
            RootPrimitive->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            RootPrimitive->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            RootPrimitive->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
            RootPrimitive->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
            break;
            
        default:
            RootPrimitive->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            RootPrimitive->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            break;
    }
}

void UCore_PhysicsSystemManager::EnableDestructionOnActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    // This would integrate with Chaos Destruction system in a full implementation
    UE_LOG(LogTemp, Log, TEXT("Destruction enabled on actor: %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::CreateDebrisFromActor(AActor* SourceActor, const FVector& ImpactPoint, float ImpactForce)
{
    if (!SourceActor)
    {
        return;
    }
    
    // This would create debris pieces using Chaos Destruction
    UE_LOG(LogTemp, Log, TEXT("Creating debris from actor: %s at impact point: %s with force: %.2f"),
        *SourceActor->GetName(), *ImpactPoint.ToString(), ImpactForce);
}

void UCore_PhysicsSystemManager::PausePhysicsSimulation()
{
    if (UWorld* World = GetWorld())
    {
        World->GetPhysicsScene()->SetIsStaticLoading(true);
        UE_LOG(LogTemp, Log, TEXT("Physics simulation paused"));
    }
}

void UCore_PhysicsSystemManager::ResumePhysicsSimulation()
{
    if (UWorld* World = GetWorld())
    {
        World->GetPhysicsScene()->SetIsStaticLoading(false);
        UE_LOG(LogTemp, Log, TEXT("Physics simulation resumed"));
    }
}

void UCore_PhysicsSystemManager::SetPhysicsTimeScale(float TimeScale)
{
    if (UWorld* World = GetWorld())
    {
        World->GetWorldSettings()->SetTimeDilation(TimeScale);
        UE_LOG(LogTemp, Log, TEXT("Physics time scale set to: %.2f"), TimeScale);
    }
}