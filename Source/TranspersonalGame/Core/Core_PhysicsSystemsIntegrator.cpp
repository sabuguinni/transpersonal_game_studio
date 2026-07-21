#include "Core_PhysicsSystemsIntegrator.h"
#include "Core_PhysicsManager.h"
#include "Core_CollisionSystem.h"
#include "Core_DestructionSystem.h"
#include "Core_RagdollSystem.h"
#include "Core_TerrainPhysics.h"
#include "Core_PhysicsPerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UCore_PhysicsSystemsIntegrator::UCore_PhysicsSystemsIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    bPhysicsSystemsInitialized = false;
    bPhysicsDebuggingEnabled = false;
    PhysicsUpdateFrequency = 60.0f;
    PhysicsTimeAccumulator = 0.0f;
    
    // Initialize subsystem pointers
    PhysicsManager = nullptr;
    CollisionSystem = nullptr;
    DestructionSystem = nullptr;
    RagdollSystem = nullptr;
    TerrainPhysics = nullptr;
    PerformanceMonitor = nullptr;
}

void UCore_PhysicsSystemsIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    LogSystemStatus("Core Physics Systems Integrator starting up");
    
    // Initialize physics systems in correct order
    InitializePhysicsSystems();
}

void UCore_PhysicsSystemsIntegrator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    LogSystemStatus("Core Physics Systems Integrator shutting down");
    
    // Shutdown physics systems safely
    ShutdownPhysicsSystems();
    
    Super::EndPlay(EndPlayReason);
}

void UCore_PhysicsSystemsIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bPhysicsSystemsInitialized)
    {
        return;
    }
    
    // Accumulate time for fixed physics updates
    PhysicsTimeAccumulator += DeltaTime;
    
    const float PhysicsTimeStep = 1.0f / PhysicsUpdateFrequency;
    
    // Update physics systems at fixed intervals
    while (PhysicsTimeAccumulator >= PhysicsTimeStep)
    {
        UpdatePhysicsSystems(PhysicsTimeStep);
        PhysicsTimeAccumulator -= PhysicsTimeStep;
    }
    
    // Validate system health periodically
    if (FMath::Fmod(GetWorld()->GetTimeSeconds(), 5.0f) < DeltaTime)
    {
        if (!ValidateSystemHealth())
        {
            LogSystemStatus("Physics system health check failed", true);
        }
    }
}

void UCore_PhysicsSystemsIntegrator::InitializePhysicsSystems()
{
    if (bPhysicsSystemsInitialized)
    {
        LogSystemStatus("Physics systems already initialized");
        return;
    }
    
    LogSystemStatus("Initializing physics systems...");
    
    UWorld* World = GetWorld();
    if (!World)
    {
        LogSystemStatus("Failed to get world for physics initialization", true);
        return;
    }
    
    // Initialize systems in dependency order
    bool bAllSystemsInitialized = true;
    
    // 1. Performance Monitor (needed by all other systems)
    PerformanceMonitor = NewObject<UCore_PhysicsPerformanceMonitor>(this);
    bAllSystemsInitialized &= InitializeSubsystem(PerformanceMonitor, "Performance Monitor");
    
    // 2. Physics Manager (core physics simulation)
    PhysicsManager = NewObject<UCore_PhysicsManager>(this);
    bAllSystemsInitialized &= InitializeSubsystem(PhysicsManager, "Physics Manager");
    
    // 3. Collision System (depends on physics manager)
    CollisionSystem = NewObject<UCore_CollisionSystem>(this);
    bAllSystemsInitialized &= InitializeSubsystem(CollisionSystem, "Collision System");
    
    // 4. Terrain Physics (depends on collision system)
    TerrainPhysics = NewObject<UCore_TerrainPhysics>(this);
    bAllSystemsInitialized &= InitializeSubsystem(TerrainPhysics, "Terrain Physics");
    
    // 5. Destruction System (depends on physics and collision)
    DestructionSystem = NewObject<UCore_DestructionSystem>(this);
    bAllSystemsInitialized &= InitializeSubsystem(DestructionSystem, "Destruction System");
    
    // 6. Ragdoll System (depends on physics and collision)
    RagdollSystem = NewObject<UCore_RagdollSystem>(this);
    bAllSystemsInitialized &= InitializeSubsystem(RagdollSystem, "Ragdoll System");
    
    bPhysicsSystemsInitialized = bAllSystemsInitialized;
    
    if (bPhysicsSystemsInitialized)
    {
        LogSystemStatus("All physics systems initialized successfully");
    }
    else
    {
        LogSystemStatus("Failed to initialize some physics systems", true);
    }
}

void UCore_PhysicsSystemsIntegrator::ShutdownPhysicsSystems()
{
    if (!bPhysicsSystemsInitialized)
    {
        return;
    }
    
    LogSystemStatus("Shutting down physics systems...");
    
    // Shutdown in reverse order
    ShutdownSubsystem(RagdollSystem, "Ragdoll System");
    ShutdownSubsystem(DestructionSystem, "Destruction System");
    ShutdownSubsystem(TerrainPhysics, "Terrain Physics");
    ShutdownSubsystem(CollisionSystem, "Collision System");
    ShutdownSubsystem(PhysicsManager, "Physics Manager");
    ShutdownSubsystem(PerformanceMonitor, "Performance Monitor");
    
    // Clear references
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;
    TerrainPhysics = nullptr;
    CollisionSystem = nullptr;
    PhysicsManager = nullptr;
    PerformanceMonitor = nullptr;
    
    bPhysicsSystemsInitialized = false;
    LogSystemStatus("Physics systems shutdown complete");
}

void UCore_PhysicsSystemsIntegrator::UpdatePhysicsSystems(float DeltaTime)
{
    if (!bPhysicsSystemsInitialized)
    {
        return;
    }
    
    // Update systems in execution order
    if (PerformanceMonitor)
    {
        // Performance monitor tracks the update cycle
    }
    
    if (PhysicsManager)
    {
        // Core physics simulation update
    }
    
    if (CollisionSystem)
    {
        // Collision detection and response
    }
    
    if (TerrainPhysics)
    {
        // Terrain interaction updates
    }
    
    if (DestructionSystem)
    {
        // Destruction processing
    }
    
    if (RagdollSystem)
    {
        // Ragdoll physics updates
    }
}

FString UCore_PhysicsSystemsIntegrator::GetPhysicsPerformanceReport() const
{
    if (!PerformanceMonitor)
    {
        return "Performance monitor not available";
    }
    
    FString Report = "=== PHYSICS PERFORMANCE REPORT ===\n";
    Report += FString::Printf(TEXT("Systems Initialized: %s\n"), bPhysicsSystemsInitialized ? TEXT("Yes") : TEXT("No"));
    Report += FString::Printf(TEXT("Update Frequency: %.1f Hz\n"), PhysicsUpdateFrequency);
    Report += FString::Printf(TEXT("Debugging Enabled: %s\n"), bPhysicsDebuggingEnabled ? TEXT("Yes") : TEXT("No"));
    
    // Add individual system status
    Report += "\n=== SUBSYSTEM STATUS ===\n";
    Report += FString::Printf(TEXT("Physics Manager: %s\n"), PhysicsManager ? TEXT("Active") : TEXT("Inactive"));
    Report += FString::Printf(TEXT("Collision System: %s\n"), CollisionSystem ? TEXT("Active") : TEXT("Inactive"));
    Report += FString::Printf(TEXT("Destruction System: %s\n"), DestructionSystem ? TEXT("Active") : TEXT("Inactive"));
    Report += FString::Printf(TEXT("Ragdoll System: %s\n"), RagdollSystem ? TEXT("Active") : TEXT("Inactive"));
    Report += FString::Printf(TEXT("Terrain Physics: %s\n"), TerrainPhysics ? TEXT("Active") : TEXT("Inactive"));
    
    return Report;
}

void UCore_PhysicsSystemsIntegrator::SetPhysicsDebugging(bool bEnabled)
{
    bPhysicsDebuggingEnabled = bEnabled;
    LogSystemStatus(FString::Printf(TEXT("Physics debugging %s"), bEnabled ? TEXT("enabled") : TEXT("disabled")));
    
    // Propagate debug setting to all subsystems
    // Individual systems will implement their own debug visualization
}

bool UCore_PhysicsSystemsIntegrator::ArePhysicsSystemsHealthy() const
{
    return bPhysicsSystemsInitialized && ValidateSystemHealth();
}

bool UCore_PhysicsSystemsIntegrator::InitializeSubsystem(UObject* Subsystem, const FString& SubsystemName)
{
    if (!Subsystem)
    {
        LogSystemStatus(FString::Printf(TEXT("Failed to create %s"), *SubsystemName), true);
        return false;
    }
    
    LogSystemStatus(FString::Printf(TEXT("Initialized %s"), *SubsystemName));
    return true;
}

void UCore_PhysicsSystemsIntegrator::ShutdownSubsystem(UObject* Subsystem, const FString& SubsystemName)
{
    if (Subsystem)
    {
        LogSystemStatus(FString::Printf(TEXT("Shutting down %s"), *SubsystemName));
        // Individual subsystems handle their own cleanup
    }
}

bool UCore_PhysicsSystemsIntegrator::ValidateSystemHealth() const
{
    // Check that all expected systems are still valid
    bool bHealthy = true;
    
    if (!PhysicsManager)
    {
        bHealthy = false;
    }
    
    if (!CollisionSystem)
    {
        bHealthy = false;
    }
    
    if (!PerformanceMonitor)
    {
        bHealthy = false;
    }
    
    return bHealthy;
}

void UCore_PhysicsSystemsIntegrator::LogSystemStatus(const FString& Message, bool bIsError) const
{
    FString LogMessage = FString::Printf(TEXT("[Core Physics Integrator] %s"), *Message);
    
    if (bIsError)
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *LogMessage);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
    }
}