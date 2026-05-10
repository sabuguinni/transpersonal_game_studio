#include "Core_PhysicsWorldManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "DrawDebugHelpers.h"

UCore_PhysicsWorldManager::UCore_PhysicsWorldManager()
{
    // Initialize default configuration
    CurrentConfig = FCore_PhysicsWorldConfig();
    PerformanceMetrics = FCore_PhysicsPerformanceMetrics();
    bAutoOptimizePerformance = true;
    PerformanceUpdateInterval = 1.0f;
    MaxAllowedPhysicsFrameTime = 0.033f;
    bIsPhysicsWorldInitialized = false;
    bIsPhysicsPaused = false;
}

void UCore_PhysicsWorldManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Initializing Physics World Manager"));
    
    InitializePhysicsWorld();
    
    if (bAutoOptimizePerformance)
    {
        StartPerformanceMonitoring();
    }
    
    bIsPhysicsWorldInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Physics World Manager initialized successfully"));
}

void UCore_PhysicsWorldManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Deinitializing Physics World Manager"));
    
    StopPerformanceMonitoring();
    bIsPhysicsWorldInitialized = false;
    
    Super::Deinitialize();
}

void UCore_PhysicsWorldManager::ConfigurePhysicsWorld(const FCore_PhysicsWorldConfig& Config)
{
    if (!bIsPhysicsWorldInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsWorldManager: Cannot configure physics world - not initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Configuring physics world with type: %d"), (int32)Config.WorldType);
    
    CurrentConfig = Config;
    ApplyPhysicsConfiguration();
    
    // Log configuration changes
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Physics world configured - Gravity: %f, TimeStep: %f, MaxSubSteps: %d"), 
           Config.GravityScale, Config.PhysicsTimeStep, Config.MaxSubSteps);
}

void UCore_PhysicsWorldManager::SetPhysicsWorldType(ECore_PhysicsWorldType NewType)
{
    if (CurrentConfig.WorldType == NewType)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Changing physics world type from %d to %d"), 
           (int32)CurrentConfig.WorldType, (int32)NewType);
    
    CurrentConfig.WorldType = NewType;
    
    // Adjust configuration based on world type
    switch (NewType)
    {
        case ECore_PhysicsWorldType::Standard:
            CurrentConfig.PhysicsTimeStep = 0.016667f; // 60 FPS
            CurrentConfig.MaxSubSteps = 6;
            CurrentConfig.bEnableAsyncPhysics = true;
            CurrentConfig.bEnableCCD = false;
            break;
            
        case ECore_PhysicsWorldType::HighPrecision:
            CurrentConfig.PhysicsTimeStep = 0.008333f; // 120 FPS
            CurrentConfig.MaxSubSteps = 8;
            CurrentConfig.bEnableAsyncPhysics = true;
            CurrentConfig.bEnableCCD = true;
            break;
            
        case ECore_PhysicsWorldType::LowLatency:
            CurrentConfig.PhysicsTimeStep = 0.033333f; // 30 FPS
            CurrentConfig.MaxSubSteps = 4;
            CurrentConfig.bEnableAsyncPhysics = false;
            CurrentConfig.bEnableCCD = false;
            break;
            
        case ECore_PhysicsWorldType::Destructible:
            CurrentConfig.PhysicsTimeStep = 0.016667f;
            CurrentConfig.MaxSubSteps = 8;
            CurrentConfig.bEnableAsyncPhysics = true;
            CurrentConfig.bEnableCCD = true;
            break;
            
        case ECore_PhysicsWorldType::FluidDynamics:
            CurrentConfig.PhysicsTimeStep = 0.008333f;
            CurrentConfig.MaxSubSteps = 10;
            CurrentConfig.bEnableAsyncPhysics = true;
            CurrentConfig.bEnableCCD = false;
            break;
            
        case ECore_PhysicsWorldType::VehiclePhysics:
            CurrentConfig.PhysicsTimeStep = 0.016667f;
            CurrentConfig.MaxSubSteps = 6;
            CurrentConfig.bEnableAsyncPhysics = true;
            CurrentConfig.bEnableCCD = true;
            break;
    }
    
    ApplyPhysicsConfiguration();
}

void UCore_PhysicsWorldManager::UpdatePerformanceMetrics()
{
    UWorld* World = GetPhysicsWorld();
    if (!World)
    {
        return;
    }
    
    // Update performance metrics
    PerformanceMetrics.PhysicsFrameTime = World->GetDeltaSeconds();
    
    // Count active and sleeping rigid bodies
    PerformanceMetrics.ActiveRigidBodies = 0;
    PerformanceMetrics.SleepingRigidBodies = 0;
    PerformanceMetrics.CollisionPairs = 0;
    
    // Iterate through all actors to count physics bodies
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                if (PrimComp->IsAnyRigidBodyAwake())
                {
                    PerformanceMetrics.ActiveRigidBodies++;
                }
                else
                {
                    PerformanceMetrics.SleepingRigidBodies++;
                }
            }
        }
    }
    
    // Estimate memory usage (simplified)
    PerformanceMetrics.MemoryUsageMB = (PerformanceMetrics.ActiveRigidBodies + PerformanceMetrics.SleepingRigidBodies) * 0.1f;
    
    // Check if performance is optimal
    PerformanceMetrics.bIsPerformanceOptimal = 
        PerformanceMetrics.PhysicsFrameTime < MaxAllowedPhysicsFrameTime &&
        PerformanceMetrics.ActiveRigidBodies < 1000 &&
        PerformanceMetrics.MemoryUsageMB < 500.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Performance - FrameTime: %f, Active: %d, Sleeping: %d, Memory: %f MB"), 
           PerformanceMetrics.PhysicsFrameTime, PerformanceMetrics.ActiveRigidBodies, 
           PerformanceMetrics.SleepingRigidBodies, PerformanceMetrics.MemoryUsageMB);
}

void UCore_PhysicsWorldManager::OptimizePhysicsPerformance()
{
    if (!bIsPhysicsWorldInitialized)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Optimizing physics performance"));
    
    UpdatePerformanceMetrics();
    
    if (!PerformanceMetrics.bIsPerformanceOptimal)
    {
        HandlePerformanceIssues();
    }
}

void UCore_PhysicsWorldManager::ResetPhysicsWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Resetting physics world"));
    
    UWorld* World = GetPhysicsWorld();
    if (World)
    {
        // Reset all physics bodies
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    PrimComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
                    PrimComp->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
                    PrimComp->WakeRigidBody();
                }
            }
        }
    }
    
    // Reapply configuration
    ApplyPhysicsConfiguration();
}

void UCore_PhysicsWorldManager::PausePhysicsSimulation(bool bPause)
{
    bIsPhysicsPaused = bPause;
    
    UWorld* World = GetPhysicsWorld();
    if (World)
    {
        World->GetPhysicsScene()->SetIsStaticLoading(bPause);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Physics simulation %s"), 
           bPause ? TEXT("PAUSED") : TEXT("RESUMED"));
}

void UCore_PhysicsWorldManager::SetGravityScale(float NewGravityScale)
{
    CurrentConfig.GravityScale = NewGravityScale;
    
    UWorld* World = GetPhysicsWorld();
    if (World)
    {
        World->GetPhysicsScene()->GetPxScene()->setGravity(physx::PxVec3(0.0f, 0.0f, -980.0f * NewGravityScale));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Gravity scale set to %f"), NewGravityScale);
}

void UCore_PhysicsWorldManager::EnableContinuousCollisionDetection(bool bEnable)
{
    CurrentConfig.bEnableCCD = bEnable;
    ApplyPhysicsConfiguration();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Continuous Collision Detection %s"), 
           bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UCore_PhysicsWorldManager::SetPhysicsTimeStep(float NewTimeStep)
{
    CurrentConfig.PhysicsTimeStep = FMath::Clamp(NewTimeStep, 0.001f, 0.1f);
    ApplyPhysicsConfiguration();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Physics time step set to %f"), CurrentConfig.PhysicsTimeStep);
}

void UCore_PhysicsWorldManager::SetMaxSubSteps(int32 NewMaxSubSteps)
{
    CurrentConfig.MaxSubSteps = FMath::Clamp(NewMaxSubSteps, 1, 20);
    ApplyPhysicsConfiguration();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Max sub steps set to %d"), CurrentConfig.MaxSubSteps);
}

void UCore_PhysicsWorldManager::ValidatePhysicsWorldSetup()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS WORLD VALIDATION ==="));
    
    UWorld* World = GetPhysicsWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Physics world is NULL!"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Physics World Type: %d"), (int32)CurrentConfig.WorldType);
    UE_LOG(LogTemp, Warning, TEXT("Gravity Scale: %f"), CurrentConfig.GravityScale);
    UE_LOG(LogTemp, Warning, TEXT("Physics Time Step: %f"), CurrentConfig.PhysicsTimeStep);
    UE_LOG(LogTemp, Warning, TEXT("Max Sub Steps: %d"), CurrentConfig.MaxSubSteps);
    UE_LOG(LogTemp, Warning, TEXT("Async Physics: %s"), CurrentConfig.bEnableAsyncPhysics ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("CCD: %s"), CurrentConfig.bEnableCCD ? TEXT("Enabled") : TEXT("Disabled"));
    
    UpdatePerformanceMetrics();
    UE_LOG(LogTemp, Warning, TEXT("Active Bodies: %d"), PerformanceMetrics.ActiveRigidBodies);
    UE_LOG(LogTemp, Warning, TEXT("Sleeping Bodies: %d"), PerformanceMetrics.SleepingRigidBodies);
    UE_LOG(LogTemp, Warning, TEXT("Performance Optimal: %s"), PerformanceMetrics.bIsPerformanceOptimal ? TEXT("Yes") : TEXT("No"));
    
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATION COMPLETE ==="));
}

void UCore_PhysicsWorldManager::LogPhysicsWorldStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS WORLD STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Initialized: %s"), bIsPhysicsWorldInitialized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Paused: %s"), bIsPhysicsPaused ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Auto Optimize: %s"), bAutoOptimizePerformance ? TEXT("Yes") : TEXT("No"));
    
    ValidatePhysicsWorldSetup();
}

void UCore_PhysicsWorldManager::DebugDrawPhysicsWorld(bool bEnable)
{
    UWorld* World = GetPhysicsWorld();
    if (!World)
    {
        return;
    }
    
    if (bEnable)
    {
        // Enable physics debug drawing
        World->GetPhysicsScene()->GetPxScene()->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
        World->GetPhysicsScene()->GetPxScene()->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
        World->GetPhysicsScene()->GetPxScene()->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_AABBS, 1.0f);
    }
    else
    {
        // Disable physics debug drawing
        World->GetPhysicsScene()->GetPxScene()->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 0.0f);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Physics debug drawing %s"), 
           bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UCore_PhysicsWorldManager::InitializePhysicsWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Initializing physics world"));
    
    ApplyPhysicsConfiguration();
}

void UCore_PhysicsWorldManager::ApplyPhysicsConfiguration()
{
    UWorld* World = GetPhysicsWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsWorldManager: Cannot apply configuration - world is NULL"));
        return;
    }
    
    // Apply gravity
    SetGravityScale(CurrentConfig.GravityScale);
    
    // Apply physics settings
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        PhysicsSettings->DefaultGravityZ = -980.0f * CurrentConfig.GravityScale;
        PhysicsSettings->bEnableAsyncScene = CurrentConfig.bEnableAsyncPhysics;
        PhysicsSettings->MaxSubstepDeltaTime = CurrentConfig.PhysicsTimeStep;
        PhysicsSettings->MaxSubsteps = CurrentConfig.MaxSubSteps;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Physics configuration applied"));
}

void UCore_PhysicsWorldManager::StartPerformanceMonitoring()
{
    UWorld* World = GetPhysicsWorld();
    if (!World)
    {
        return;
    }
    
    World->GetTimerManager().SetTimer(
        PerformanceUpdateTimer,
        this,
        &UCore_PhysicsWorldManager::OnPerformanceUpdate,
        PerformanceUpdateInterval,
        true
    );
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Performance monitoring started"));
}

void UCore_PhysicsWorldManager::StopPerformanceMonitoring()
{
    UWorld* World = GetPhysicsWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Performance monitoring stopped"));
}

void UCore_PhysicsWorldManager::OnPerformanceUpdate()
{
    UpdatePerformanceMetrics();
    
    if (!PerformanceMetrics.bIsPerformanceOptimal && bAutoOptimizePerformance)
    {
        HandlePerformanceIssues();
    }
}

void UCore_PhysicsWorldManager::HandlePerformanceIssues()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Handling performance issues"));
    
    // If frame time is too high, reduce physics quality
    if (PerformanceMetrics.PhysicsFrameTime > MaxAllowedPhysicsFrameTime)
    {
        if (CurrentConfig.MaxSubSteps > 2)
        {
            CurrentConfig.MaxSubSteps--;
            UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Reduced max sub steps to %d"), CurrentConfig.MaxSubSteps);
        }
        
        if (CurrentConfig.PhysicsTimeStep < 0.033f)
        {
            CurrentConfig.PhysicsTimeStep = 0.033f; // Reduce to 30 FPS
            UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Reduced physics time step to %f"), CurrentConfig.PhysicsTimeStep);
        }
        
        ApplyPhysicsConfiguration();
    }
    
    // If too many active bodies, put some to sleep
    if (PerformanceMetrics.ActiveRigidBodies > 500)
    {
        UWorld* World = GetPhysicsWorld();
        if (World)
        {
            int32 BodiesPutToSleep = 0;
            for (TActorIterator<AActor> ActorItr(World); ActorItr && BodiesPutToSleep < 100; ++ActorItr)
            {
                AActor* Actor = *ActorItr;
                if (Actor && Actor->GetRootComponent())
                {
                    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                    if (PrimComp && PrimComp->IsSimulatingPhysics() && PrimComp->IsAnyRigidBodyAwake())
                    {
                        FVector Velocity = PrimComp->GetPhysicsLinearVelocity();
                        if (Velocity.SizeSquared() < 100.0f) // Low velocity threshold
                        {
                            PrimComp->PutRigidBodyToSleep();
                            BodiesPutToSleep++;
                        }
                    }
                }
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Put %d bodies to sleep for performance"), BodiesPutToSleep);
        }
    }
}

UWorld* UCore_PhysicsWorldManager::GetPhysicsWorld() const
{
    if (GetGameInstance())
    {
        return GetGameInstance()->GetWorld();
    }
    return nullptr;
}