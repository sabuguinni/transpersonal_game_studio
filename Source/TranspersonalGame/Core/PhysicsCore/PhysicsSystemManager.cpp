#include "PhysicsSystemManager.h"
#include "PhysicsCollisionManager.h"
#include "RagdollSystemManager.h"
#include "DestructionSystemManager.h"
#include "VehiclePhysicsManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UPhysicsSystemManager::UPhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    CurrentPhysicsLOD = 0; // Start with highest quality
    MaxActivePhysicsBodies = 2000; // Conservative default for performance
    bPhysicsEnabled = true;
    AveragePhysicsFrameTime = 0.0f;
    LastFrameTime = 0.0f;
    
    // Reserve space for performance samples
    PhysicsFrameTimeSamples.Reserve(MaxFrameTimeSamples);
}

void UPhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Initializing physics systems..."));
    
    // Store world reference
    WorldContext = GetWorld();
    
    if (WorldContext.IsValid())
    {
        InitializePhysicsSystems(WorldContext.Get());
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: All physics systems initialized successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Failed to get valid world context"));
    }
}

void UPhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bPhysicsEnabled || !WorldContext.IsValid())
    {
        return;
    }
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Check if we need to adjust physics LOD
    float CurrentFPS = 1.0f / DeltaTime;
    float TargetFPS = 60.0f; // Default to PC target, could be adjusted based on platform
    
    if (GEngine && GEngine->GetGameUserSettings())
    {
        // Adjust target based on platform or settings
        // For now, keep 60fps target
    }
    
    UpdatePhysicsLOD(CurrentFPS, TargetFPS);
    
    // Update all subsystem managers
    if (CollisionManager)
    {
        CollisionManager->UpdateCollisionSystem(DeltaTime);
    }
    
    if (RagdollManager)
    {
        RagdollManager->UpdateRagdollSystem(DeltaTime);
    }
    
    if (DestructionManager)
    {
        DestructionManager->UpdateDestructionSystem(DeltaTime);
    }
    
    if (VehicleManager)
    {
        VehicleManager->UpdateVehiclePhysics(DeltaTime);
    }
}

void UPhysicsSystemManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Shutting down physics systems..."));
    ShutdownPhysicsSystems();
    Super::EndPlay(EndPlayReason);
}

void UPhysicsSystemManager::InitializePhysicsSystems(UWorld* InWorld)
{
    if (!InWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Cannot initialize with null world"));
        return;
    }
    
    CreateSubsystemManagers();
    
    // Initialize each subsystem
    if (CollisionManager)
    {
        CollisionManager->InitializeCollisionSystem(InWorld);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Collision system initialized"));
    }
    
    if (RagdollManager)
    {
        RagdollManager->InitializeRagdollSystem(InWorld);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Ragdoll system initialized"));
    }
    
    if (DestructionManager)
    {
        DestructionManager->InitializeDestructionSystem(InWorld);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Destruction system initialized"));
    }
    
    if (VehicleManager)
    {
        VehicleManager->InitializeVehiclePhysics(InWorld);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Vehicle physics initialized"));
    }
}

void UPhysicsSystemManager::ShutdownPhysicsSystems()
{
    // Shutdown in reverse order
    if (VehicleManager)
    {
        VehicleManager->ShutdownVehiclePhysics();
        VehicleManager = nullptr;
    }
    
    if (DestructionManager)
    {
        DestructionManager->ShutdownDestructionSystem();
        DestructionManager = nullptr;
    }
    
    if (RagdollManager)
    {
        RagdollManager->ShutdownRagdollSystem();
        RagdollManager = nullptr;
    }
    
    if (CollisionManager)
    {
        CollisionManager->ShutdownCollisionSystem();
        CollisionManager = nullptr;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: All physics systems shut down"));
}

void UPhysicsSystemManager::UpdatePhysicsLOD(float CurrentFPS, float TargetFPS)
{
    int32 NewLOD = CalculateOptimalLOD(CurrentFPS, TargetFPS);
    
    if (NewLOD != CurrentPhysicsLOD)
    {
        CurrentPhysicsLOD = NewLOD;
        
        // Apply LOD settings to all subsystems
        if (CollisionManager)
        {
            CollisionManager->SetLODLevel(CurrentPhysicsLOD);
        }
        
        if (RagdollManager)
        {
            RagdollManager->SetLODLevel(CurrentPhysicsLOD);
        }
        
        if (DestructionManager)
        {
            DestructionManager->SetLODLevel(CurrentPhysicsLOD);
        }
        
        if (VehicleManager)
        {
            VehicleManager->SetLODLevel(CurrentPhysicsLOD);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics LOD changed to level %d (FPS: %.1f/%.1f)"), 
               CurrentPhysicsLOD, CurrentFPS, TargetFPS);
    }
}

void UPhysicsSystemManager::SetPhysicsEnabled(bool bEnabled)
{
    bPhysicsEnabled = bEnabled;
    
    if (CollisionManager)
    {
        CollisionManager->SetEnabled(bEnabled);
    }
    
    if (RagdollManager)
    {
        RagdollManager->SetEnabled(bEnabled);
    }
    
    if (DestructionManager)
    {
        DestructionManager->SetEnabled(bEnabled);
    }
    
    if (VehicleManager)
    {
        VehicleManager->SetEnabled(bEnabled);
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics simulation %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

FString UPhysicsSystemManager::GetPhysicsPerformanceReport() const
{
    FString Report = TEXT("=== PHYSICS PERFORMANCE REPORT ===\n");
    Report += FString::Printf(TEXT("Current LOD Level: %d\n"), CurrentPhysicsLOD);
    Report += FString::Printf(TEXT("Average Frame Time: %.3f ms\n"), AveragePhysicsFrameTime * 1000.0f);
    Report += FString::Printf(TEXT("Max Active Bodies: %d\n"), MaxActivePhysicsBodies);
    Report += FString::Printf(TEXT("Physics Enabled: %s\n"), bPhysicsEnabled ? TEXT("Yes") : TEXT("No"));
    
    if (CollisionManager)
    {
        Report += CollisionManager->GetPerformanceReport();
    }
    
    if (RagdollManager)
    {
        Report += RagdollManager->GetPerformanceReport();
    }
    
    if (DestructionManager)
    {
        Report += DestructionManager->GetPerformanceReport();
    }
    
    if (VehicleManager)
    {
        Report += VehicleManager->GetPerformanceReport();
    }
    
    return Report;
}

void UPhysicsSystemManager::CreateSubsystemManagers()
{
    // Create collision manager
    CollisionManager = NewObject<UPhysicsCollisionManager>(this, UPhysicsCollisionManager::StaticClass());
    
    // Create ragdoll manager
    RagdollManager = NewObject<URagdollSystemManager>(this, URagdollSystemManager::StaticClass());
    
    // Create destruction manager
    DestructionManager = NewObject<UDestructionSystemManager>(this, UDestructionSystemManager::StaticClass());
    
    // Create vehicle manager
    VehicleManager = NewObject<UVehiclePhysicsManager>(this, UVehiclePhysicsManager::StaticClass());
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: All subsystem managers created"));
}

void UPhysicsSystemManager::UpdatePerformanceMetrics(float DeltaTime)
{
    // Add current frame time to samples
    PhysicsFrameTimeSamples.Add(DeltaTime);
    
    // Keep only the most recent samples
    if (PhysicsFrameTimeSamples.Num() > MaxFrameTimeSamples)
    {
        PhysicsFrameTimeSamples.RemoveAt(0);
    }
    
    // Calculate average frame time
    float TotalTime = 0.0f;
    for (float FrameTime : PhysicsFrameTimeSamples)
    {
        TotalTime += FrameTime;
    }
    
    AveragePhysicsFrameTime = TotalTime / PhysicsFrameTimeSamples.Num();
    LastFrameTime = DeltaTime;
}

int32 UPhysicsSystemManager::CalculateOptimalLOD(float CurrentFPS, float TargetFPS) const
{
    float FPSRatio = CurrentFPS / TargetFPS;
    
    // LOD 0: Full quality (FPS >= target)
    if (FPSRatio >= 1.0f)
    {
        return 0;
    }
    // LOD 1: High quality (FPS >= 80% of target)
    else if (FPSRatio >= 0.8f)
    {
        return 1;
    }
    // LOD 2: Medium quality (FPS >= 60% of target)
    else if (FPSRatio >= 0.6f)
    {
        return 2;
    }
    // LOD 3: Low quality (FPS < 60% of target)
    else
    {
        return 3;
    }
}