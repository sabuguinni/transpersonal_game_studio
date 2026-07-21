#include "Core_SystemsRegistryIntegrator.h"
#include "Core_PhysicsManager.h"
#include "Core_CollisionSystem.h"
#include "Core_RagdollSystem.h"
#include "Core_DestructionSystem.h"
#include "Core_MaterialPhysics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UCore_SystemsRegistryIntegrator::UCore_SystemsRegistryIntegrator()
{
    // Initialize pointers
    SystemsRegistry = nullptr;
    PhysicsManager = nullptr;
    CollisionSystem = nullptr;
    MaterialPhysics = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;

    // Initialize status tracking maps
    SystemRegistrationStatus.Empty();
    SystemInitializationStatus.Empty();
    SystemInitializationTimes.Empty();
}

void UCore_SystemsRegistryIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: Initializing..."));

    // Get reference to the Systems Registry
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        SystemsRegistry = GameInstance->GetSubsystem<UEng_SystemsRegistry>();
        if (SystemsRegistry)
        {
            UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: Connected to Systems Registry"));
            
            // Register all physics systems with the registry
            RegisterPhysicsSystems();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: Failed to get Systems Registry"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: Failed to get Game Instance"));
    }
}

void UCore_SystemsRegistryIntegrator::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: Deinitializing..."));

    // Emergency shutdown all physics systems
    EmergencyShutdownPhysicsSystems();

    // Clear all references
    SystemsRegistry = nullptr;
    PhysicsManager = nullptr;
    CollisionSystem = nullptr;
    MaterialPhysics = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;

    // Clear status maps
    SystemRegistrationStatus.Empty();
    SystemInitializationStatus.Empty();
    SystemInitializationTimes.Empty();

    Super::Deinitialize();
}

void UCore_SystemsRegistryIntegrator::RegisterPhysicsSystems()
{
    if (!SystemsRegistry)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: Cannot register systems - no Systems Registry"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: Registering physics systems..."));

    // Create all physics systems
    CreatePhysicsManager();
    CreateCollisionSystem();
    CreateMaterialPhysics();
    CreateRagdollSystem();
    CreateDestructionSystem();

    // Register systems with the registry in priority order
    if (PhysicsManager)
    {
        // Priority 10 - Foundation physics system
        SystemsRegistry->RegisterSystem(TEXT("Core_PhysicsManager"), PhysicsManager, 10);
        SystemRegistrationStatus.Add(TEXT("Core_PhysicsManager"), true);
        LogSystemEvent(TEXT("Core_PhysicsManager"), TEXT("Registered"), true);
    }

    if (CollisionSystem)
    {
        // Priority 11 - Collision detection
        SystemsRegistry->RegisterSystem(TEXT("Core_CollisionSystem"), CollisionSystem, 11);
        SystemRegistrationStatus.Add(TEXT("Core_CollisionSystem"), true);
        LogSystemEvent(TEXT("Core_CollisionSystem"), TEXT("Registered"), true);
    }

    if (MaterialPhysics)
    {
        // Priority 12 - Material properties
        SystemsRegistry->RegisterSystem(TEXT("Core_MaterialPhysics"), MaterialPhysics, 12);
        SystemRegistrationStatus.Add(TEXT("Core_MaterialPhysics"), true);
        LogSystemEvent(TEXT("Core_MaterialPhysics"), TEXT("Registered"), true);
    }

    if (RagdollSystem)
    {
        // Priority 13 - Character physics
        SystemsRegistry->RegisterSystem(TEXT("Core_RagdollSystem"), RagdollSystem, 13);
        SystemRegistrationStatus.Add(TEXT("Core_RagdollSystem"), true);
        LogSystemEvent(TEXT("Core_RagdollSystem"), TEXT("Registered"), true);
    }

    if (DestructionSystem)
    {
        // Priority 14 - Destruction physics
        SystemsRegistry->RegisterSystem(TEXT("Core_DestructionSystem"), DestructionSystem, 14);
        SystemRegistrationStatus.Add(TEXT("Core_DestructionSystem"), true);
        LogSystemEvent(TEXT("Core_DestructionSystem"), TEXT("Registered"), true);
    }

    UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: Physics systems registration complete"));
}

void UCore_SystemsRegistryIntegrator::InitializePhysicsSystems()
{
    if (!ValidateSystemDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: System dependencies not satisfied"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: Initializing physics systems..."));

    // Initialize systems in dependency order with timing
    double StartTime, EndTime;

    // Initialize Physics Manager (Priority 10)
    if (PhysicsManager)
    {
        StartTime = FPlatformTime::Seconds();
        bool bSuccess = PhysicsManager->InitializePhysicsManager();
        EndTime = FPlatformTime::Seconds();
        
        SystemInitializationStatus.Add(TEXT("Core_PhysicsManager"), bSuccess);
        SystemInitializationTimes.Add(TEXT("Core_PhysicsManager"), EndTime - StartTime);
        LogSystemEvent(TEXT("Core_PhysicsManager"), TEXT("Initialized"), bSuccess);
    }

    // Initialize Collision System (Priority 11)
    if (CollisionSystem)
    {
        StartTime = FPlatformTime::Seconds();
        bool bSuccess = CollisionSystem->InitializeCollisionSystem();
        EndTime = FPlatformTime::Seconds();
        
        SystemInitializationStatus.Add(TEXT("Core_CollisionSystem"), bSuccess);
        SystemInitializationTimes.Add(TEXT("Core_CollisionSystem"), EndTime - StartTime);
        LogSystemEvent(TEXT("Core_CollisionSystem"), TEXT("Initialized"), bSuccess);
    }

    // Initialize Material Physics (Priority 12)
    if (MaterialPhysics)
    {
        StartTime = FPlatformTime::Seconds();
        bool bSuccess = MaterialPhysics->InitializeMaterialPhysics();
        EndTime = FPlatformTime::Seconds();
        
        SystemInitializationStatus.Add(TEXT("Core_MaterialPhysics"), bSuccess);
        SystemInitializationTimes.Add(TEXT("Core_MaterialPhysics"), EndTime - StartTime);
        LogSystemEvent(TEXT("Core_MaterialPhysics"), TEXT("Initialized"), bSuccess);
    }

    // Initialize Ragdoll System (Priority 13)
    if (RagdollSystem)
    {
        StartTime = FPlatformTime::Seconds();
        bool bSuccess = RagdollSystem->InitializeRagdollSystem();
        EndTime = FPlatformTime::Seconds();
        
        SystemInitializationStatus.Add(TEXT("Core_RagdollSystem"), bSuccess);
        SystemInitializationTimes.Add(TEXT("Core_RagdollSystem"), EndTime - StartTime);
        LogSystemEvent(TEXT("Core_RagdollSystem"), TEXT("Initialized"), bSuccess);
    }

    // Initialize Destruction System (Priority 14)
    if (DestructionSystem)
    {
        StartTime = FPlatformTime::Seconds();
        bool bSuccess = DestructionSystem->InitializeDestructionSystem();
        EndTime = FPlatformTime::Seconds();
        
        SystemInitializationStatus.Add(TEXT("Core_DestructionSystem"), bSuccess);
        SystemInitializationTimes.Add(TEXT("Core_DestructionSystem"), EndTime - StartTime);
        LogSystemEvent(TEXT("Core_DestructionSystem"), TEXT("Initialized"), bSuccess);
    }

    UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: Physics systems initialization complete"));
}

bool UCore_SystemsRegistryIntegrator::ValidatePhysicsSystemsIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: Validating physics systems integration..."));

    bool bAllSystemsValid = true;
    int32 ValidSystems = 0;
    int32 TotalSystems = 5; // PhysicsManager, CollisionSystem, MaterialPhysics, RagdollSystem, DestructionSystem

    // Check each system's registration and initialization status
    TArray<FString> SystemNames = {
        TEXT("Core_PhysicsManager"),
        TEXT("Core_CollisionSystem"),
        TEXT("Core_MaterialPhysics"),
        TEXT("Core_RagdollSystem"),
        TEXT("Core_DestructionSystem")
    };

    for (const FString& SystemName : SystemNames)
    {
        bool bRegistered = SystemRegistrationStatus.Contains(SystemName) && SystemRegistrationStatus[SystemName];
        bool bInitialized = SystemInitializationStatus.Contains(SystemName) && SystemInitializationStatus[SystemName];

        if (bRegistered && bInitialized)
        {
            ValidSystems++;
            UE_LOG(LogTemp, Log, TEXT("✓ %s: VALID (Registered & Initialized)"), *SystemName);
        }
        else
        {
            bAllSystemsValid = false;
            UE_LOG(LogTemp, Warning, TEXT("✗ %s: INVALID (Registered: %s, Initialized: %s)"), 
                *SystemName, bRegistered ? TEXT("Yes") : TEXT("No"), bInitialized ? TEXT("Yes") : TEXT("No"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: Validation complete - %d/%d systems valid"), 
        ValidSystems, TotalSystems);

    return bAllSystemsValid;
}

FString UCore_SystemsRegistryIntegrator::GetPhysicsSystemsStatus()
{
    FString StatusReport = TEXT("=== PHYSICS SYSTEMS STATUS REPORT ===\n");

    // Add registry connection status
    StatusReport += FString::Printf(TEXT("Systems Registry Connected: %s\n"), 
        SystemsRegistry ? TEXT("Yes") : TEXT("No"));

    // Add individual system status
    TArray<FString> SystemNames = {
        TEXT("Core_PhysicsManager"),
        TEXT("Core_CollisionSystem"),
        TEXT("Core_MaterialPhysics"),
        TEXT("Core_RagdollSystem"),
        TEXT("Core_DestructionSystem")
    };

    StatusReport += TEXT("\nSYSTEM STATUS:\n");
    for (const FString& SystemName : SystemNames)
    {
        bool bRegistered = SystemRegistrationStatus.Contains(SystemName) && SystemRegistrationStatus[SystemName];
        bool bInitialized = SystemInitializationStatus.Contains(SystemName) && SystemInitializationStatus[SystemName];
        float InitTime = SystemInitializationTimes.Contains(SystemName) ? SystemInitializationTimes[SystemName] : 0.0f;

        StatusReport += FString::Printf(TEXT("  %s: Reg=%s, Init=%s, Time=%.3fs\n"),
            *SystemName, 
            bRegistered ? TEXT("✓") : TEXT("✗"),
            bInitialized ? TEXT("✓") : TEXT("✗"),
            InitTime);
    }

    return StatusReport;
}

void UCore_SystemsRegistryIntegrator::EmergencyShutdownPhysicsSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_SystemsRegistryIntegrator: Emergency shutdown initiated"));

    // Shutdown systems in reverse order
    if (DestructionSystem)
    {
        DestructionSystem->ShutdownDestructionSystem();
        LogSystemEvent(TEXT("Core_DestructionSystem"), TEXT("Emergency Shutdown"), true);
    }

    if (RagdollSystem)
    {
        RagdollSystem->ShutdownRagdollSystem();
        LogSystemEvent(TEXT("Core_RagdollSystem"), TEXT("Emergency Shutdown"), true);
    }

    if (MaterialPhysics)
    {
        MaterialPhysics->ShutdownMaterialPhysics();
        LogSystemEvent(TEXT("Core_MaterialPhysics"), TEXT("Emergency Shutdown"), true);
    }

    if (CollisionSystem)
    {
        CollisionSystem->ShutdownCollisionSystem();
        LogSystemEvent(TEXT("Core_CollisionSystem"), TEXT("Emergency Shutdown"), true);
    }

    if (PhysicsManager)
    {
        PhysicsManager->ShutdownPhysicsManager();
        LogSystemEvent(TEXT("Core_PhysicsManager"), TEXT("Emergency Shutdown"), true);
    }

    UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: Emergency shutdown complete"));
}

void UCore_SystemsRegistryIntegrator::CreatePhysicsManager()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        PhysicsManager = GameInstance->GetSubsystem<UCore_PhysicsManager>();
        if (!PhysicsManager)
        {
            UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: Failed to get PhysicsManager subsystem"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: PhysicsManager created"));
        }
    }
}

void UCore_SystemsRegistryIntegrator::CreateCollisionSystem()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        CollisionSystem = GameInstance->GetSubsystem<UCore_CollisionSystem>();
        if (!CollisionSystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: Failed to get CollisionSystem subsystem"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: CollisionSystem created"));
        }
    }
}

void UCore_SystemsRegistryIntegrator::CreateMaterialPhysics()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        MaterialPhysics = GameInstance->GetSubsystem<UCore_MaterialPhysics>();
        if (!MaterialPhysics)
        {
            UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: Failed to get MaterialPhysics subsystem"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: MaterialPhysics created"));
        }
    }
}

void UCore_SystemsRegistryIntegrator::CreateRagdollSystem()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        RagdollSystem = GameInstance->GetSubsystem<UCore_RagdollSystem>();
        if (!RagdollSystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: Failed to get RagdollSystem subsystem"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: RagdollSystem created"));
        }
    }
}

void UCore_SystemsRegistryIntegrator::CreateDestructionSystem()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        DestructionSystem = GameInstance->GetSubsystem<UCore_DestructionSystem>();
        if (!DestructionSystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: Failed to get DestructionSystem subsystem"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: DestructionSystem created"));
        }
    }
}

bool UCore_SystemsRegistryIntegrator::ValidateSystemDependencies()
{
    // Check that all required systems are available
    bool bDependenciesSatisfied = true;

    if (!PhysicsManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: PhysicsManager dependency not satisfied"));
        bDependenciesSatisfied = false;
    }

    if (!CollisionSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: CollisionSystem dependency not satisfied"));
        bDependenciesSatisfied = false;
    }

    if (!MaterialPhysics)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: MaterialPhysics dependency not satisfied"));
        bDependenciesSatisfied = false;
    }

    if (!RagdollSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: RagdollSystem dependency not satisfied"));
        bDependenciesSatisfied = false;
    }

    if (!DestructionSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: DestructionSystem dependency not satisfied"));
        bDependenciesSatisfied = false;
    }

    return bDependenciesSatisfied;
}

void UCore_SystemsRegistryIntegrator::LogSystemEvent(const FString& SystemName, const FString& Event, bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_SystemsRegistryIntegrator: %s - %s: SUCCESS"), *SystemName, *Event);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Core_SystemsRegistryIntegrator: %s - %s: FAILED"), *SystemName, *Event);
    }
}