#include "Eng_SystemArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "TimerManager.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

// Include core systems
#include "Eng_ArchitectureCore.h"
#include "Core_PhysicsManager.h"
#include "BiomeManager.h"
#include "DinosaurBase.h"

UEng_SystemArchitect::UEng_SystemArchitect()
{
    MetricsUpdateInterval = 1.0f;
    MaxInitializationTime = 10.0f;
    bEnablePerformanceMonitoring = true;
    bEnforceStrictDependencies = true;
}

void UEng_SystemArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("SystemArchitect: Initializing master system coordinator"));
    
    // Register core systems with proper priorities and dependencies
    RegisterSystem(TEXT("ArchitectureCore"), EEng_SystemPriority::Critical, true);
    RegisterSystem(TEXT("PhysicsManager"), EEng_SystemPriority::Critical, true);
    RegisterSystem(TEXT("BiomeManager"), EEng_SystemPriority::High, true);
    RegisterSystem(TEXT("CharacterSystem"), EEng_SystemPriority::High, true);
    RegisterSystem(TEXT("DinosaurAI"), EEng_SystemPriority::Medium, false);
    RegisterSystem(TEXT("CombatSystem"), EEng_SystemPriority::Medium, false);
    RegisterSystem(TEXT("AudioSystem"), EEng_SystemPriority::Low, false);
    RegisterSystem(TEXT("VFXSystem"), EEng_SystemPriority::Low, false);
    
    // Set up dependencies
    if (FEng_SystemInfo* PhysicsInfo = RegisteredSystems.Find(TEXT("PhysicsManager")))
    {
        PhysicsInfo->Dependencies.Add(TEXT("ArchitectureCore"));
    }
    
    if (FEng_SystemInfo* BiomeInfo = RegisteredSystems.Find(TEXT("BiomeManager")))
    {
        BiomeInfo->Dependencies.Add(TEXT("PhysicsManager"));
    }
    
    if (FEng_SystemInfo* CharacterInfo = RegisteredSystems.Find(TEXT("CharacterSystem")))
    {
        CharacterInfo->Dependencies.Add(TEXT("PhysicsManager"));
        CharacterInfo->Dependencies.Add(TEXT("BiomeManager"));
    }
    
    // Start performance monitoring if enabled
    if (bEnablePerformanceMonitoring)
    {
        GetWorld()->GetTimerManager().SetTimer(
            MetricsTimerHandle,
            this,
            &UEng_SystemArchitect::UpdateMetricsTimer,
            MetricsUpdateInterval,
            true
        );
    }
    
    // Initialize systems in proper order
    InitializeAllSystems();
}

void UEng_SystemArchitect::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("SystemArchitect: Shutting down all systems"));
    
    // Clear performance monitoring timer
    if (GetWorld() && MetricsTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(MetricsTimerHandle);
    }
    
    ShutdownAllSystems();
    RegisteredSystems.Empty();
    
    Super::Deinitialize();
}

void UEng_SystemArchitect::InitializeAllSystems()
{
    UE_LOG(LogTemp, Log, TEXT("SystemArchitect: Starting system initialization sequence"));
    
    // Validate dependencies before initialization
    ValidateSystemDependencies();
    
    // Initialize systems by priority order
    InitializeSystemsByPriority();
    
    // Verify all critical systems are ready
    if (!AreAllCriticalSystemsReady())
    {
        UE_LOG(LogTemp, Error, TEXT("SystemArchitect: Critical system initialization failed!"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("SystemArchitect: All systems initialized successfully"));
    LogSystemStatus();
}

void UEng_SystemArchitect::ShutdownAllSystems()
{
    // Shutdown in reverse priority order
    TArray<EEng_SystemPriority> Priorities = {
        EEng_SystemPriority::Background,
        EEng_SystemPriority::Low,
        EEng_SystemPriority::Medium,
        EEng_SystemPriority::High,
        EEng_SystemPriority::Critical
    };
    
    for (EEng_SystemPriority Priority : Priorities)
    {
        for (auto& SystemPair : RegisteredSystems)
        {
            if (SystemPair.Value.Priority == Priority)
            {
                UpdateSystemState(SystemPair.Key, EEng_SystemState::Shutdown);
                UE_LOG(LogTemp, Log, TEXT("SystemArchitect: Shutdown system %s"), *SystemPair.Key);
            }
        }
    }
}

bool UEng_SystemArchitect::RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority, bool bIsEssential)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("SystemArchitect: System %s already registered"), *SystemName);
        return false;
    }
    
    FEng_SystemInfo NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Priority = Priority;
    NewSystem.bIsEssential = bIsEssential;
    NewSystem.State = EEng_SystemState::Uninitialized;
    NewSystem.InitializationTime = 0.0f;
    
    RegisteredSystems.Add(SystemName, NewSystem);
    
    UE_LOG(LogTemp, Log, TEXT("SystemArchitect: Registered system %s (Priority: %d, Essential: %s)"), 
           *SystemName, 
           (int32)Priority, 
           bIsEssential ? TEXT("Yes") : TEXT("No"));
    
    return true;
}

bool UEng_SystemArchitect::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        return false;
    }
    
    // Shutdown system before unregistering
    UpdateSystemState(SystemName, EEng_SystemState::Shutdown);
    RegisteredSystems.Remove(SystemName);
    
    UE_LOG(LogTemp, Log, TEXT("SystemArchitect: Unregistered system %s"), *SystemName);
    return true;
}

void UEng_SystemArchitect::UpdateSystemState(const FString& SystemName, EEng_SystemState NewState)
{
    if (FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        EEng_SystemState OldState = SystemInfo->State;
        SystemInfo->State = NewState;
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("SystemArchitect: System %s state changed from %d to %d"), 
               *SystemName, (int32)OldState, (int32)NewState);
        
        // Handle state transitions
        if (NewState == EEng_SystemState::Error && SystemInfo->bIsEssential)
        {
            UE_LOG(LogTemp, Error, TEXT("SystemArchitect: Critical system %s entered error state!"), *SystemName);
            HandleSystemFailure(SystemName);
        }
    }
}

EEng_SystemState UEng_SystemArchitect::GetSystemState(const FString& SystemName) const
{
    if (const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return SystemInfo->State;
    }
    return EEng_SystemState::Uninitialized;
}

bool UEng_SystemArchitect::IsSystemReady(const FString& SystemName) const
{
    EEng_SystemState State = GetSystemState(SystemName);
    return State == EEng_SystemState::Ready || State == EEng_SystemState::Running;
}

bool UEng_SystemArchitect::AreAllCriticalSystemsReady() const
{
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Priority == EEng_SystemPriority::Critical && SystemPair.Value.bIsEssential)
        {
            if (!IsSystemReady(SystemPair.Key))
            {
                return false;
            }
        }
    }
    return true;
}

TArray<FEng_SystemInfo> UEng_SystemArchitect::GetAllSystemInfo() const
{
    TArray<FEng_SystemInfo> SystemInfoArray;
    for (const auto& SystemPair : RegisteredSystems)
    {
        SystemInfoArray.Add(SystemPair.Value);
    }
    return SystemInfoArray;
}

void UEng_SystemArchitect::UpdatePerformanceMetrics()
{
    if (!bEnablePerformanceMonitoring)
    {
        return;
    }
    
    // Get current world
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Update frame time
    CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Count active actors
    CurrentMetrics.ActiveActors = World->GetActorCount();
    
    // Get physics metrics (simplified)
    CurrentMetrics.PhysicsBodies = 0; // Would need physics world access
    CurrentMetrics.PhysicsTime = 0.0f;
    
    // Memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Render time (would need render thread access)
    CurrentMetrics.RenderTime = 0.0f;
}

FEng_PerformanceMetrics UEng_SystemArchitect::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UEng_SystemArchitect::LogSystemStatus() const
{
    UE_LOG(LogTemp, Log, TEXT("=== SYSTEM ARCHITECT STATUS REPORT ==="));
    
    int32 TotalSystems = RegisteredSystems.Num();
    int32 ReadySystems = 0;
    int32 ErrorSystems = 0;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemInfo& Info = SystemPair.Value;
        
        FString StateString;
        switch (Info.State)
        {
            case EEng_SystemState::Uninitialized: StateString = TEXT("UNINITIALIZED"); break;
            case EEng_SystemState::Initializing: StateString = TEXT("INITIALIZING"); break;
            case EEng_SystemState::Ready: StateString = TEXT("READY"); ReadySystems++; break;
            case EEng_SystemState::Running: StateString = TEXT("RUNNING"); ReadySystems++; break;
            case EEng_SystemState::Error: StateString = TEXT("ERROR"); ErrorSystems++; break;
            case EEng_SystemState::Shutdown: StateString = TEXT("SHUTDOWN"); break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("  %s: %s (Priority: %d, Essential: %s)"), 
               *Info.SystemName, 
               *StateString, 
               (int32)Info.Priority,
               Info.bIsEssential ? TEXT("Yes") : TEXT("No"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Systems Ready: %d/%d | Errors: %d"), ReadySystems, TotalSystems, ErrorSystems);
    UE_LOG(LogTemp, Log, TEXT("Performance: %.2fms frame, %d actors, %.1fMB memory"), 
           CurrentMetrics.FrameTime, CurrentMetrics.ActiveActors, CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("====================================="));
}

bool UEng_SystemArchitect::ValidateSystemArchitecture()
{
    bool bArchitectureValid = true;
    
    // Check for circular dependencies
    for (const auto& SystemPair : RegisteredSystems)
    {
        // Simplified dependency check - would need more sophisticated graph analysis
        if (SystemPair.Value.Dependencies.Num() > 5)
        {
            UE_LOG(LogTemp, Warning, TEXT("SystemArchitect: System %s has excessive dependencies"), *SystemPair.Key);
        }
    }
    
    // Validate critical systems are present
    TArray<FString> RequiredSystems = {TEXT("ArchitectureCore"), TEXT("PhysicsManager")};
    for (const FString& Required : RequiredSystems)
    {
        if (!RegisteredSystems.Contains(Required))
        {
            UE_LOG(LogTemp, Error, TEXT("SystemArchitect: Required system %s not registered"), *Required);
            bArchitectureValid = false;
        }
    }
    
    return bArchitectureValid;
}

void UEng_SystemArchitect::EnforceArchitecturalStandards()
{
    if (!bEnforceStrictDependencies)
    {
        return;
    }
    
    // Enforce initialization order based on dependencies
    ValidateSystemDependencies();
    
    // Check for systems that have been initializing too long
    for (auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.State == EEng_SystemState::Initializing && 
            SystemPair.Value.InitializationTime > MaxInitializationTime)
        {
            UE_LOG(LogTemp, Error, TEXT("SystemArchitect: System %s initialization timeout"), *SystemPair.Key);
            UpdateSystemState(SystemPair.Key, EEng_SystemState::Error);
        }
    }
}

void UEng_SystemArchitect::EmergencySystemShutdown(const FString& SystemName)
{
    UE_LOG(LogTemp, Warning, TEXT("SystemArchitect: Emergency shutdown of system %s"), *SystemName);
    UpdateSystemState(SystemName, EEng_SystemState::Shutdown);
}

void UEng_SystemArchitect::RestartFailedSystems()
{
    for (auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.State == EEng_SystemState::Error)
        {
            UE_LOG(LogTemp, Log, TEXT("SystemArchitect: Attempting to restart failed system %s"), *SystemPair.Key);
            UpdateSystemState(SystemPair.Key, EEng_SystemState::Initializing);
        }
    }
}

void UEng_SystemArchitect::InitializeSystemsByPriority()
{
    // Initialize in priority order
    TArray<EEng_SystemPriority> Priorities = {
        EEng_SystemPriority::Critical,
        EEng_SystemPriority::High,
        EEng_SystemPriority::Medium,
        EEng_SystemPriority::Low,
        EEng_SystemPriority::Background
    };
    
    for (EEng_SystemPriority Priority : Priorities)
    {
        for (auto& SystemPair : RegisteredSystems)
        {
            if (SystemPair.Value.Priority == Priority)
            {
                UpdateSystemState(SystemPair.Key, EEng_SystemState::Initializing);
                
                // Simulate initialization time
                SystemPair.Value.InitializationTime = FPlatformTime::Seconds();
                
                // Mark as ready (real implementation would do actual initialization)
                UpdateSystemState(SystemPair.Key, EEng_SystemState::Ready);
                
                UE_LOG(LogTemp, Log, TEXT("SystemArchitect: Initialized system %s"), *SystemPair.Key);
            }
        }
    }
}

void UEng_SystemArchitect::ValidateSystemDependencies()
{
    // Simple dependency validation - real implementation would use topological sort
    for (const auto& SystemPair : RegisteredSystems)
    {
        for (const FString& Dependency : SystemPair.Value.Dependencies)
        {
            if (!RegisteredSystems.Contains(Dependency))
            {
                UE_LOG(LogTemp, Error, TEXT("SystemArchitect: System %s depends on unregistered system %s"), 
                       *SystemPair.Key, *Dependency);
            }
        }
    }
}

bool UEng_SystemArchitect::CheckSystemHealth(const FString& SystemName)
{
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        return false;
    }
    
    // Basic health checks
    return SystemInfo->State != EEng_SystemState::Error && 
           SystemInfo->State != EEng_SystemState::Uninitialized;
}

void UEng_SystemArchitect::HandleSystemFailure(const FString& SystemName)
{
    UE_LOG(LogTemp, Error, TEXT("SystemArchitect: Handling failure of system %s"), *SystemName);
    
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (SystemInfo && SystemInfo->bIsEssential)
    {
        // For critical systems, attempt restart
        UE_LOG(LogTemp, Warning, TEXT("SystemArchitect: Attempting restart of critical system %s"), *SystemName);
        UpdateSystemState(SystemName, EEng_SystemState::Initializing);
    }
}

void UEng_SystemArchitect::UpdateMetricsTimer()
{
    UpdatePerformanceMetrics();
}

// World System Coordinator Implementation
UEng_WorldSystemCoordinator::UEng_WorldSystemCoordinator()
{
    bWorldSystemsInitialized = false;
    WorldSystemUpdateRate = 60.0f; // 60 FPS
}

void UEng_WorldSystemCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("WorldSystemCoordinator: Initializing world-specific systems"));
}

void UEng_WorldSystemCoordinator::Deinitialize()
{
    if (GetWorld() && WorldUpdateTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(WorldUpdateTimerHandle);
    }
    
    bWorldSystemsInitialized = false;
    Super::Deinitialize();
}

void UEng_WorldSystemCoordinator::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    InitializeWorldSystems();
}

void UEng_WorldSystemCoordinator::InitializeWorldSystems()
{
    UE_LOG(LogTemp, Log, TEXT("WorldSystemCoordinator: Initializing world systems"));
    
    // Start world system update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            WorldUpdateTimerHandle,
            this,
            &UEng_WorldSystemCoordinator::WorldSystemUpdateTick,
            1.0f / WorldSystemUpdateRate,
            true
        );
    }
    
    bWorldSystemsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("WorldSystemCoordinator: World systems initialized"));
}

void UEng_WorldSystemCoordinator::UpdateWorldSystems(float DeltaTime)
{
    if (!bWorldSystemsInitialized)
    {
        return;
    }
    
    // Update world-specific systems here
    // This would coordinate with BiomeManager, DinosaurAI, etc.
}

bool UEng_WorldSystemCoordinator::AreWorldSystemsReady() const
{
    return bWorldSystemsInitialized;
}

void UEng_WorldSystemCoordinator::WorldSystemUpdateTick()
{
    float DeltaTime = 1.0f / WorldSystemUpdateRate;
    UpdateWorldSystems(DeltaTime);
}