#include "Core_PhysicsManager.h"
#include "EngineArchitecturalCore.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY(LogPhysicsManager);

UCore_PhysicsManager::UCore_PhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    // Initialize physics settings
    PhysicsSettings.Gravity = FVector(0.0f, 0.0f, -980.0f); // Standard gravity
    PhysicsSettings.LinearDamping = 0.01f;
    PhysicsSettings.AngularDamping = 0.05f;
    PhysicsSettings.MaxAngularVelocity = 3600.0f;
    PhysicsSettings.bEnableCCD = true;
    PhysicsSettings.bEnableAsyncScene = true;
    
    // Performance settings
    MaxSimulatedBodies = 1000;
    PhysicsLODDistance = 5000.0f;
    bEnablePhysicsOptimization = true;
    
    // Statistics
    ActiveBodies = 0;
    SleepingBodies = 0;
    PhysicsFrameTime = 0.0f;
}

void UCore_PhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with EngineArchitecturalCore
    if (UEngineArchitecturalCore* ArchCore = GetWorld()->GetGameInstance()->GetSubsystem<UEngineArchitecturalCore>())
    {
        ArchCore->RegisterModule(TEXT("PhysicsManager"), this);
        UE_LOG(LogPhysicsManager, Log, TEXT("PhysicsManager registered with EngineArchitecturalCore"));
    }
    
    InitializePhysicsWorld();
    SetupPerformanceMonitoring();
    
    UE_LOG(LogPhysicsManager, Log, TEXT("Core_PhysicsManager initialized - Gravity: %s"), *PhysicsSettings.Gravity.ToString());
}

void UCore_PhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdatePhysicsStatistics();
    OptimizePhysicsPerformance();
    
    // Update frame time
    PhysicsFrameTime = DeltaTime;
}

void UCore_PhysicsManager::InitializePhysicsWorld()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogPhysicsManager, Error, TEXT("Failed to get world for physics initialization"));
        return;
    }
    
    // Apply physics settings to world
    if (UPhysicsSettings* Settings = UPhysicsSettings::Get())
    {
        // Configure physics solver settings
        Settings->DefaultGravityZ = PhysicsSettings.Gravity.Z;
        Settings->bEnableAsyncScene = PhysicsSettings.bEnableAsyncScene;
        Settings->bEnablePCM = true; // Persistent Contact Manifold
        Settings->bEnableStabilization = true;
        
        UE_LOG(LogPhysicsManager, Log, TEXT("Applied physics settings - Gravity Z: %f"), Settings->DefaultGravityZ);
    }
    
    // Initialize collision profiles
    SetupCollisionProfiles();
}

void UCore_PhysicsManager::SetupCollisionProfiles()
{
    // Define custom collision profiles for prehistoric game
    CollisionProfiles.Add(TEXT("Dinosaur"), FCore_CollisionProfile{
        TEXT("Dinosaur"),
        ECollisionEnabled::QueryAndPhysics,
        ECollisionObjectType::WorldDynamic,
        ECR_Block, ECR_Block, ECR_Block, ECR_Ignore, ECR_Block
    });
    
    CollisionProfiles.Add(TEXT("PrehistoricVegetation"), FCore_CollisionProfile{
        TEXT("PrehistoricVegetation"),
        ECollisionEnabled::QueryOnly,
        ECollisionObjectType::WorldStatic,
        ECR_Block, ECR_Ignore, ECR_Block, ECR_Ignore, ECR_Block
    });
    
    CollisionProfiles.Add(TEXT("Projectile"), FCore_CollisionProfile{
        TEXT("Projectile"),
        ECollisionEnabled::QueryAndPhysics,
        ECollisionObjectType::WorldDynamic,
        ECR_Block, ECR_Block, ECR_Block, ECR_Block, ECR_Ignore
    });
    
    UE_LOG(LogPhysicsManager, Log, TEXT("Initialized %d collision profiles"), CollisionProfiles.Num());
}

void UCore_PhysicsManager::UpdatePhysicsStatistics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    ActiveBodies = 0;
    SleepingBodies = 0;
    
    // Count physics bodies in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Primitive : PrimitiveComponents)
        {
            if (Primitive && Primitive->IsSimulatingPhysics())
            {
                FBodyInstance* BodyInstance = Primitive->GetBodyInstance();
                if (BodyInstance)
                {
                    if (BodyInstance->IsInstanceAwake())
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
    
    // Update performance metrics
    float TotalBodies = ActiveBodies + SleepingBodies;
    if (TotalBodies > MaxSimulatedBodies * 0.8f)
    {
        UE_LOG(LogPhysicsManager, Warning, TEXT("High physics body count: %d/%d"), (int32)TotalBodies, MaxSimulatedBodies);
    }
}

void UCore_PhysicsManager::OptimizePhysicsPerformance()
{
    if (!bEnablePhysicsOptimization) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Optimize physics bodies based on distance from player
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Primitive : PrimitiveComponents)
        {
            if (Primitive && Primitive->IsSimulatingPhysics())
            {
                // Disable physics simulation for distant objects
                if (Distance > PhysicsLODDistance)
                {
                    if (Primitive->IsSimulatingPhysics())
                    {
                        Primitive->SetSimulatePhysics(false);
                    }
                }
                else if (Distance < PhysicsLODDistance * 0.8f)
                {
                    if (!Primitive->IsSimulatingPhysics())
                    {
                        Primitive->SetSimulatePhysics(true);
                    }
                }
            }
        }
    }
}

void UCore_PhysicsManager::SetupPerformanceMonitoring()
{
    // Set up performance monitoring timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceTimerHandle,
            this,
            &UCore_PhysicsManager::LogPerformanceMetrics,
            5.0f, // Log every 5 seconds
            true
        );
    }
}

void UCore_PhysicsManager::LogPerformanceMetrics()
{
    float TotalBodies = ActiveBodies + SleepingBodies;
    float BodyUtilization = (MaxSimulatedBodies > 0) ? (TotalBodies / MaxSimulatedBodies) * 100.0f : 0.0f;
    
    UE_LOG(LogPhysicsManager, Log, TEXT("Physics Metrics - Active: %d, Sleeping: %d, Utilization: %.1f%%, Frame Time: %.3fms"), 
           ActiveBodies, SleepingBodies, BodyUtilization, PhysicsFrameTime * 1000.0f);
    
    // Broadcast performance event
    OnPhysicsPerformanceUpdate.Broadcast(ActiveBodies, SleepingBodies, PhysicsFrameTime);
}

void UCore_PhysicsManager::ApplyImpulseToActor(AActor* Actor, const FVector& Impulse, const FVector& Location)
{
    if (!Actor)
    {
        UE_LOG(LogPhysicsManager, Warning, TEXT("ApplyImpulseToActor: Actor is null"));
        return;
    }
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (!RootPrimitive)
    {
        UE_LOG(LogPhysicsManager, Warning, TEXT("ApplyImpulseToActor: Actor has no primitive root component"));
        return;
    }
    
    if (!RootPrimitive->IsSimulatingPhysics())
    {
        UE_LOG(LogPhysicsManager, Warning, TEXT("ApplyImpulseToActor: Actor is not simulating physics"));
        return;
    }
    
    RootPrimitive->AddImpulseAtLocation(Impulse, Location);
    UE_LOG(LogPhysicsManager, Log, TEXT("Applied impulse %s at location %s to actor %s"), 
           *Impulse.ToString(), *Location.ToString(), *Actor->GetName());
}

void UCore_PhysicsManager::SetActorPhysicsEnabled(AActor* Actor, bool bEnabled)
{
    if (!Actor)
    {
        UE_LOG(LogPhysicsManager, Warning, TEXT("SetActorPhysicsEnabled: Actor is null"));
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Primitive : PrimitiveComponents)
    {
        if (Primitive)
        {
            Primitive->SetSimulatePhysics(bEnabled);
        }
    }
    
    UE_LOG(LogPhysicsManager, Log, TEXT("Set physics %s for actor %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"), *Actor->GetName());
}

FCore_PhysicsStats UCore_PhysicsManager::GetPhysicsStatistics() const
{
    FCore_PhysicsStats Stats;
    Stats.ActiveBodies = ActiveBodies;
    Stats.SleepingBodies = SleepingBodies;
    Stats.TotalBodies = ActiveBodies + SleepingBodies;
    Stats.FrameTime = PhysicsFrameTime;
    Stats.BodyUtilization = (MaxSimulatedBodies > 0) ? (Stats.TotalBodies / MaxSimulatedBodies) * 100.0f : 0.0f;
    return Stats;
}

void UCore_PhysicsManager::SetPhysicsSettings(const FCore_PhysicsSettings& NewSettings)
{
    PhysicsSettings = NewSettings;
    
    // Apply new settings to world
    if (UPhysicsSettings* Settings = UPhysicsSettings::Get())
    {
        Settings->DefaultGravityZ = PhysicsSettings.Gravity.Z;
        Settings->bEnableAsyncScene = PhysicsSettings.bEnableAsyncScene;
    }
    
    UE_LOG(LogPhysicsManager, Log, TEXT("Updated physics settings - Gravity: %s"), *PhysicsSettings.Gravity.ToString());
}

bool UCore_PhysicsManager::ValidatePhysicsIntegrity() const
{
    // Validate physics world state
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogPhysicsManager, Error, TEXT("ValidatePhysicsIntegrity: No world"));
        return false;
    }
    
    // Check for physics scene
    if (!World->GetPhysicsScene())
    {
        UE_LOG(LogPhysicsManager, Error, TEXT("ValidatePhysicsIntegrity: No physics scene"));
        return false;
    }
    
    // Validate body count is within limits
    float TotalBodies = ActiveBodies + SleepingBodies;
    if (TotalBodies > MaxSimulatedBodies)
    {
        UE_LOG(LogPhysicsManager, Warning, TEXT("ValidatePhysicsIntegrity: Body count exceeds limit (%d/%d)"), 
               (int32)TotalBodies, MaxSimulatedBodies);
        return false;
    }
    
    UE_LOG(LogPhysicsManager, Log, TEXT("Physics integrity validation passed"));
    return true;
}