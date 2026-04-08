#include "StudioDirectorSubsystem.h"
#include "PhysicsCore/PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogStudioDirector, Log, All);

UStudioDirectorSubsystem::UStudioDirectorSubsystem()
{
    // Define system initialization order (critical path)
    SystemInitializationOrder = {
        TEXT("PhysicsSystem"),
        TEXT("PerformanceSystem"), 
        TEXT("WorldGeneration"),
        TEXT("EnvironmentSystem"),
        TEXT("ArchitectureSystem"),
        TEXT("LightingSystem"),
        TEXT("CharacterSystem"),
        TEXT("AnimationSystem"),
        TEXT("NPCBehaviorSystem"),
        TEXT("CombatAISystem"),
        TEXT("CrowdSimulation"),
        TEXT("NarrativeSystem"),
        TEXT("QuestSystem"),
        TEXT("AudioSystem"),
        TEXT("VFXSystem"),
        TEXT("QASystem")
    };
}

void UStudioDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogStudioDirector, Log, TEXT("Studio Director Subsystem initializing..."));
    
    // Initialize performance monitoring
    LastHealthCheckTime = 0.0f;
    SystemFailureCounts.Empty();
    
    // Don't initialize game systems immediately - wait for world to be ready
    bSystemsInitialized = false;
    
    UE_LOG(LogStudioDirector, Log, TEXT("Studio Director Subsystem initialized"));
}

void UStudioDirectorSubsystem::Deinitialize()
{
    UE_LOG(LogStudioDirector, Log, TEXT("Studio Director Subsystem shutting down..."));
    
    ShutdownGameSystems();
    
    RegisteredSystems.Empty();
    SystemPerformanceMetrics.Empty();
    SystemFailureCounts.Empty();
    
    Super::Deinitialize();
    
    UE_LOG(LogStudioDirector, Log, TEXT("Studio Director Subsystem shutdown complete"));
}

void UStudioDirectorSubsystem::Tick(float DeltaTime)
{
    if (!bSystemsInitialized)
    {
        return;
    }
    
    // Update performance monitoring
    LastHealthCheckTime += DeltaTime;
    
    if (bEnableSystemMonitoring && LastHealthCheckTime >= HealthCheckInterval)
    {
        MonitorSystemHealth();
        LastHealthCheckTime = 0.0f;
    }
}

bool UStudioDirectorSubsystem::IsTickable() const
{
    return !IsTemplate() && !HasAnyFlags(RF_ClassDefaultObject);
}

TStatId UStudioDirectorSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UStudioDirectorSubsystem, STATGROUP_Tickables);
}

UStudioDirectorSubsystem* UStudioDirectorSubsystem::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UStudioDirectorSubsystem>();
        }
    }
    return nullptr;
}

void UStudioDirectorSubsystem::InitializeGameSystems()
{
    if (bSystemsInitialized)
    {
        UE_LOG(LogStudioDirector, Warning, TEXT("Systems already initialized"));
        return;
    }
    
    UE_LOG(LogStudioDirector, Log, TEXT("Initializing game systems in order..."));
    
    // Phase 1: Core Engine Systems
    InitializeCoreEngineSystems();
    
    // Phase 2: Gameplay Systems  
    InitializeGameplaySystems();
    
    // Phase 3: Content and Presentation Systems
    InitializeContentSystems();
    
    bSystemsInitialized = true;
    
    UE_LOG(LogStudioDirector, Log, TEXT("All game systems initialized successfully"));
    
    // Start monitoring
    LastHealthCheckTime = 0.0f;
}

void UStudioDirectorSubsystem::ShutdownGameSystems()
{
    if (!bSystemsInitialized)
    {
        return;
    }
    
    UE_LOG(LogStudioDirector, Log, TEXT("Shutting down game systems..."));
    
    // Shutdown in reverse order
    for (int32 i = SystemInitializationOrder.Num() - 1; i >= 0; --i)
    {
        const FString& SystemName = SystemInitializationOrder[i];
        
        if (UObject** SystemPtr = RegisteredSystems.Find(SystemName))
        {
            UE_LOG(LogStudioDirector, Log, TEXT("Shutting down system: %s"), *SystemName);
            
            // Call shutdown method if it exists
            if (UObject* System = *SystemPtr)
            {
                if (UFunction* ShutdownFunc = System->GetClass()->FindFunctionByName(TEXT("Shutdown")))
                {
                    System->ProcessEvent(ShutdownFunc, nullptr);
                }
            }
        }
    }
    
    RegisteredSystems.Empty();
    bSystemsInitialized = false;
    
    UE_LOG(LogStudioDirector, Log, TEXT("All game systems shutdown complete"));
}

void UStudioDirectorSubsystem::ForceUpdateAllSystems()
{
    UE_LOG(LogStudioDirector, Log, TEXT("Force updating all systems..."));
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        UObject* System = SystemPair.Value;
        
        if (System && IsValid(System))
        {
            // Call update method if it exists
            if (UFunction* UpdateFunc = System->GetClass()->FindFunctionByName(TEXT("ForceUpdate")))
            {
                System->ProcessEvent(UpdateFunc, nullptr);
                UE_LOG(LogStudioDirector, Log, TEXT("Force updated system: %s"), *SystemName);
            }
        }
    }
}

FString UStudioDirectorSubsystem::GetSystemPerformanceReport() const
{
    FString Report = TEXT("=== STUDIO DIRECTOR PERFORMANCE REPORT ===\n");
    Report += FString::Printf(TEXT("Systems Initialized: %s\n"), bSystemsInitialized ? TEXT("YES") : TEXT("NO"));
    Report += FString::Printf(TEXT("Registered Systems: %d\n"), RegisteredSystems.Num());
    Report += FString::Printf(TEXT("Monitoring Enabled: %s\n"), bEnableSystemMonitoring ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("\n--- System Status ---\n");
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        UObject* System = SystemPair.Value;
        
        FString Status = IsValid(System) ? TEXT("ACTIVE") : TEXT("INVALID");
        int32 FailureCount = SystemFailureCounts.FindRef(SystemName);
        
        Report += FString::Printf(TEXT("%s: %s (Failures: %d)\n"), *SystemName, *Status, FailureCount);
    }
    
    if (SystemPerformanceMetrics.Num() > 0)
    {
        Report += TEXT("\n--- Performance Metrics ---\n");
        for (const auto& MetricPair : SystemPerformanceMetrics)
        {
            Report += FString::Printf(TEXT("%s: %.3fms\n"), *MetricPair.Key, MetricPair.Value);
        }
    }
    
    return Report;
}

void UStudioDirectorSubsystem::RegisterSystem(const FString& SystemName, UObject* SystemObject)
{
    if (!SystemObject || !IsValid(SystemObject))
    {
        UE_LOG(LogStudioDirector, Error, TEXT("Cannot register invalid system: %s"), *SystemName);
        return;
    }
    
    RegisteredSystems.Add(SystemName, SystemObject);
    SystemFailureCounts.Add(SystemName, 0);
    
    UE_LOG(LogStudioDirector, Log, TEXT("Registered system: %s (%s)"), 
           *SystemName, *SystemObject->GetClass()->GetName());
}

void UStudioDirectorSubsystem::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Remove(SystemName) > 0)
    {
        SystemFailureCounts.Remove(SystemName);
        SystemPerformanceMetrics.Remove(SystemName);
        
        UE_LOG(LogStudioDirector, Log, TEXT("Unregistered system: %s"), *SystemName);
    }
}

bool UStudioDirectorSubsystem::IsSystemActive(const FString& SystemName) const
{
    if (UObject* const* SystemPtr = RegisteredSystems.Find(SystemName))
    {
        return IsValid(*SystemPtr);
    }
    return false;
}

void UStudioDirectorSubsystem::InitializeCoreEngineSystems()
{
    UE_LOG(LogStudioDirector, Log, TEXT("Initializing core engine systems..."));
    
    // Initialize Physics System
    if (UWorld* World = GetWorld())
    {
        PhysicsManager = UPhysicsSystemManager::Get(World);
        if (!PhysicsManager)
        {
            PhysicsManager = NewObject<UPhysicsSystemManager>(this);
            PhysicsManager->Initialize();
        }
        RegisterSystem(TEXT("PhysicsSystem"), PhysicsManager);
    }
}

void UStudioDirectorSubsystem::InitializeGameplaySystems()
{
    UE_LOG(LogStudioDirector, Log, TEXT("Initializing gameplay systems..."));
    
    // These will be implemented by other agents
    // For now, we create placeholder registrations
    
    TArray<FString> GameplaySystems = {
        TEXT("PerformanceSystem"),
        TEXT("WorldGeneration"), 
        TEXT("EnvironmentSystem"),
        TEXT("ArchitectureSystem"),
        TEXT("CharacterSystem"),
        TEXT("AnimationSystem"),
        TEXT("NPCBehaviorSystem"),
        TEXT("CombatAISystem"),
        TEXT("CrowdSimulation")
    };
    
    for (const FString& SystemName : GameplaySystems)
    {
        // Register placeholder - actual systems will register themselves
        RegisterSystem(SystemName, this); // Temporary placeholder
    }
}

void UStudioDirectorSubsystem::InitializeContentSystems()
{
    UE_LOG(LogStudioDirector, Log, TEXT("Initializing content and presentation systems..."));
    
    TArray<FString> ContentSystems = {
        TEXT("LightingSystem"),
        TEXT("NarrativeSystem"),
        TEXT("QuestSystem"), 
        TEXT("AudioSystem"),
        TEXT("VFXSystem"),
        TEXT("QASystem")
    };
    
    for (const FString& SystemName : ContentSystems)
    {
        // Register placeholder - actual systems will register themselves
        RegisterSystem(SystemName, this); // Temporary placeholder
    }
}

void UStudioDirectorSubsystem::MonitorSystemHealth()
{
    if (!bEnableSystemMonitoring)
    {
        return;
    }
    
    int32 HealthySystemCount = 0;
    int32 FailedSystemCount = 0;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        UObject* System = SystemPair.Value;
        
        if (!IsValid(System))
        {
            HandleSystemFailure(SystemName, TEXT("System object is invalid"));
            FailedSystemCount++;
        }
        else
        {
            HealthySystemCount++;
        }
    }
    
    if (bLogSystemPerformance)
    {
        UE_LOG(LogStudioDirector, Log, TEXT("System Health Check - Healthy: %d, Failed: %d"), 
               HealthySystemCount, FailedSystemCount);
    }
}

void UStudioDirectorSubsystem::HandleSystemFailure(const FString& SystemName, const FString& ErrorMessage)
{
    int32& FailureCount = SystemFailureCounts.FindOrAdd(SystemName);
    FailureCount++;
    
    UE_LOG(LogStudioDirector, Error, TEXT("System failure detected - %s: %s (Failure #%d)"), 
           *SystemName, *ErrorMessage, FailureCount);
    
    if (FailureCount >= MaxSystemFailures)
    {
        UE_LOG(LogStudioDirector, Error, TEXT("System %s has exceeded maximum failures (%d), removing from registry"), 
               *SystemName, MaxSystemFailures);
        
        UnregisterSystem(SystemName);
        
        // Attempt auto-recovery if enabled
        if (bAutoRecoverFromFailures)
        {
            UE_LOG(LogStudioDirector, Warning, TEXT("Attempting auto-recovery for system: %s"), *SystemName);
            // Auto-recovery logic would go here
        }
    }
}