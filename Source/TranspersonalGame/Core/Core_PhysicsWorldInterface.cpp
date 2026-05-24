#include "Core_PhysicsWorldInterface.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

UCore_PhysicsWorldInterface::UCore_PhysicsWorldInterface()
{
    CurrentState = ECore_PhysicsWorldState::Initializing;
    LastStatsUpdateTime = 0.0f;
    StatsUpdateInterval = 1.0f; // Update stats every second
    
    // Initialize default configuration
    WorldConfig = FCore_PhysicsWorldConfig();
}

bool UCore_PhysicsWorldInterface::InitializePhysicsWorld(UWorld* World, const FCore_PhysicsWorldConfig& Config)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsWorldInterface: Cannot initialize with null world"));
        CurrentState = ECore_PhysicsWorldState::Error;
        return false;
    }

    ManagedWorld = World;
    WorldConfig = Config;

    // Apply configuration to the world
    ApplyPhysicsConfig(Config);

    // Configure physics settings
    ConfigurePhysicsSettings();

    // Apply gravity
    ApplyGravityToWorld();

    CurrentState = ECore_PhysicsWorldState::Active;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldInterface: Physics world initialized successfully"));
    UE_LOG(LogTemp, Log, TEXT("Gravity: %f, TimeStep: %f, MaxSubsteps: %d"), 
           Config.Gravity, Config.TimeStep, Config.MaxSubsteps);

    return true;
}

void UCore_PhysicsWorldInterface::ShutdownPhysicsWorld()
{
    if (CurrentState == ECore_PhysicsWorldState::Active)
    {
        CurrentState = ECore_PhysicsWorldState::Suspended;
        ManagedWorld.Reset();
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldInterface: Physics world shutdown"));
    }
}

void UCore_PhysicsWorldInterface::UpdatePhysicsWorld(float DeltaTime)
{
    if (CurrentState != ECore_PhysicsWorldState::Active)
    {
        return;
    }

    if (!ValidatePhysicsWorld())
    {
        CurrentState = ECore_PhysicsWorldState::Error;
        return;
    }

    // Update statistics periodically
    LastStatsUpdateTime += DeltaTime;
    if (LastStatsUpdateTime >= StatsUpdateInterval)
    {
        UpdateStatistics();
        LastStatsUpdateTime = 0.0f;
    }
}

void UCore_PhysicsWorldInterface::SetPhysicsWorldState(ECore_PhysicsWorldState NewState)
{
    if (CurrentState != NewState)
    {
        ECore_PhysicsWorldState OldState = CurrentState;
        CurrentState = NewState;

        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldInterface: State changed from %d to %d"), 
               (int32)OldState, (int32)NewState);

        // Handle state transitions
        switch (NewState)
        {
        case ECore_PhysicsWorldState::Paused:
            // Pause physics simulation
            if (ManagedWorld.IsValid())
            {
                ManagedWorld->GetPhysicsScene()->SetIsStaticLoading(true);
            }
            break;

        case ECore_PhysicsWorldState::Active:
            // Resume physics simulation
            if (ManagedWorld.IsValid())
            {
                ManagedWorld->GetPhysicsScene()->SetIsStaticLoading(false);
            }
            break;

        case ECore_PhysicsWorldState::Suspended:
            // Stop all physics bodies
            SleepAllBodies();
            break;

        default:
            break;
        }
    }
}

void UCore_PhysicsWorldInterface::ApplyPhysicsConfig(const FCore_PhysicsWorldConfig& Config)
{
    WorldConfig = Config;

    if (ManagedWorld.IsValid())
    {
        // Apply gravity
        FVector GravityVector(0.0f, 0.0f, Config.Gravity);
        ManagedWorld->GetPhysicsScene()->SetGravityZ(Config.Gravity);

        // Configure physics settings
        ConfigurePhysicsSettings();

        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldInterface: Applied physics config - Gravity: %f"), Config.Gravity);
    }
}

FCore_PhysicsStats UCore_PhysicsWorldInterface::GetPhysicsStats()
{
    UpdateStatistics();
    return CachedStats;
}

void UCore_PhysicsWorldInterface::ResetPhysicsStats()
{
    CachedStats = FCore_PhysicsStats();
    LastStatsUpdateTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldInterface: Physics stats reset"));
}

void UCore_PhysicsWorldInterface::SetGravity(float NewGravity)
{
    WorldConfig.Gravity = NewGravity;
    ApplyGravityToWorld();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldInterface: Gravity set to %f"), NewGravity);
}

float UCore_PhysicsWorldInterface::GetGravity() const
{
    return WorldConfig.Gravity;
}

void UCore_PhysicsWorldInterface::SetTimeStep(float NewTimeStep)
{
    WorldConfig.TimeStep = FMath::Clamp(NewTimeStep, 0.008333f, 0.033333f); // 30-120 FPS range
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldInterface: Time step set to %f"), WorldConfig.TimeStep);
}

void UCore_PhysicsWorldInterface::WakeAllBodies()
{
    if (!ManagedWorld.IsValid())
    {
        return;
    }

    int32 WokenBodies = 0;
    for (TActorIterator<AActor> ActorItr(ManagedWorld.Get()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                PrimComp->WakeRigidBody();
                WokenBodies++;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldInterface: Woke %d physics bodies"), WokenBodies);
}

void UCore_PhysicsWorldInterface::SleepAllBodies()
{
    if (!ManagedWorld.IsValid())
    {
        return;
    }

    int32 SleptBodies = 0;
    for (TActorIterator<AActor> ActorItr(ManagedWorld.Get()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                PrimComp->PutRigidBodyToSleep();
                SleptBodies++;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldInterface: Put %d physics bodies to sleep"), SleptBodies);
}

int32 UCore_PhysicsWorldInterface::GetActiveBodyCount()
{
    UpdateStatistics();
    return CachedStats.ActiveBodies;
}

void UCore_PhysicsWorldInterface::EnableContinuousCollisionDetection(bool bEnable)
{
    WorldConfig.bEnableCCD = bEnable;
    
    // Apply to existing physics settings
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        PhysicsSettings->bEnablePCM = bEnable;
        PhysicsSettings->bEnableStabilization = bEnable;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldInterface: CCD %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_PhysicsWorldInterface::DebugDrawPhysicsWorld()
{
    if (!ManagedWorld.IsValid())
    {
        return;
    }

    // Draw physics bodies as wireframes
    for (TActorIterator<AActor> ActorItr(ManagedWorld.Get()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                FVector Location = Actor->GetActorLocation();
                FVector Extent = PrimComp->Bounds.BoxExtent;
                
                // Draw bounding box
                DrawDebugBox(ManagedWorld.Get(), Location, Extent, FColor::Green, false, 5.0f, 0, 2.0f);
                
                // Draw velocity vector
                FVector Velocity = PrimComp->GetPhysicsLinearVelocity();
                if (!Velocity.IsNearlyZero())
                {
                    DrawDebugDirectionalArrow(ManagedWorld.Get(), Location, Location + Velocity * 0.1f, 
                                            50.0f, FColor::Red, false, 5.0f, 0, 3.0f);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldInterface: Debug draw complete"));
}

void UCore_PhysicsWorldInterface::LogPhysicsStats()
{
    FCore_PhysicsStats Stats = GetPhysicsStats();
    
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS WORLD STATS ==="));
    UE_LOG(LogTemp, Log, TEXT("Active Bodies: %d"), Stats.ActiveBodies);
    UE_LOG(LogTemp, Log, TEXT("Sleeping Bodies: %d"), Stats.SleepingBodies);
    UE_LOG(LogTemp, Log, TEXT("Constraints: %d"), Stats.Constraints);
    UE_LOG(LogTemp, Log, TEXT("Simulation Time: %.3f ms"), Stats.SimulationTime);
    UE_LOG(LogTemp, Log, TEXT("Average Frame Time: %.3f ms"), Stats.AverageFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Current State: %d"), (int32)CurrentState);
    UE_LOG(LogTemp, Log, TEXT("Gravity: %.2f"), WorldConfig.Gravity);
    UE_LOG(LogTemp, Log, TEXT("==========================="));
}

void UCore_PhysicsWorldInterface::UpdateStatistics()
{
    if (!ManagedWorld.IsValid())
    {
        return;
    }

    // Reset counters
    CachedStats.ActiveBodies = 0;
    CachedStats.SleepingBodies = 0;
    CachedStats.Constraints = 0;

    // Count physics bodies
    for (TActorIterator<AActor> ActorItr(ManagedWorld.Get()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                if (PrimComp->RigidBodyIsAwake())
                {
                    CachedStats.ActiveBodies++;
                }
                else
                {
                    CachedStats.SleepingBodies++;
                }
            }
        }
    }

    // Update timing information
    CachedStats.SimulationTime = ManagedWorld->GetDeltaSeconds() * 1000.0f; // Convert to ms
    CachedStats.AverageFrameTime = CachedStats.SimulationTime; // Simplified for now
}

void UCore_PhysicsWorldInterface::ApplyGravityToWorld()
{
    if (ManagedWorld.IsValid())
    {
        ManagedWorld->GetPhysicsScene()->SetGravityZ(WorldConfig.Gravity);
    }
}

void UCore_PhysicsWorldInterface::ConfigurePhysicsSettings()
{
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        PhysicsSettings->DefaultGravityZ = WorldConfig.Gravity;
        PhysicsSettings->bEnablePCM = WorldConfig.bEnableCCD;
        PhysicsSettings->bEnableStabilization = WorldConfig.bEnableCCD;
        PhysicsSettings->bEnableAsyncScene = WorldConfig.bEnableAsyncPhysics;
        
        // Apply damping settings
        PhysicsSettings->DefaultLinearDamping = WorldConfig.LinearDamping;
        PhysicsSettings->DefaultAngularDamping = WorldConfig.AngularDamping;
    }
}

bool UCore_PhysicsWorldInterface::ValidatePhysicsWorld() const
{
    if (!ManagedWorld.IsValid())
    {
        return false;
    }

    if (!ManagedWorld->GetPhysicsScene())
    {
        return false;
    }

    return true;
}