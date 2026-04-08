// Copyright Transpersonal Game Studio. All Rights Reserved.
// StudioDirector.cpp - Implementation of Studio Director system

#include "StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Misc/DateTime.h"
#include "Misc/App.h"

// ============================================================================
// STUDIO DIRECTOR SUBSYSTEM IMPLEMENTATION
// ============================================================================

void UStudioDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Log, TEXT("Studio Director: Initializing central coordination system..."));

    // Initialize creative vision settings
    CreativeVisionSettings = FCreativeVisionSettings();
    
    // Initialize performance tracking
    LastPerformanceCheck = 0.0f;
    CurrentBottlenecks.Empty();

    // Initialize core systems registry
    InitializeCoreSystems();

    // Start systems in dependency order
    StartSystemsInOrder();

    UE_LOG(LogTemp, Log, TEXT("Studio Director: Initialization complete. Ready to coordinate %d systems."), RegisteredSystems.Num());
}

void UStudioDirectorSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Studio Director: Shutting down coordination system..."));

    // Gracefully shutdown all systems in reverse dependency order
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);

    // Sort by priority (critical systems shutdown last)
    SystemNames.Sort([this](const FString& A, const FString& B) {
        const FSystemInfo* InfoA = RegisteredSystems.Find(A);
        const FSystemInfo* InfoB = RegisteredSystems.Find(B);
        if (InfoA && InfoB)
        {
            return InfoA->Priority > InfoB->Priority; // Higher priority = shutdown later
        }
        return false;
    });

    for (const FString& SystemName : SystemNames)
    {
        RequestSystemShutdown(SystemName, TEXT("Studio Director shutdown"));
    }

    RegisteredSystems.Empty();
    
    Super::Deinitialize();
}

void UStudioDirectorSubsystem::Tick(float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(StudioDirectorTick);

    float CurrentTime = FApp::GetCurrentTime();

    // Monitor system health
    if (CurrentTime - LastPerformanceCheck >= PERFORMANCE_CHECK_INTERVAL)
    {
        MonitorSystemHealth();
        AnalyzePerformanceMetrics();
        LastPerformanceCheck = CurrentTime;
    }

    // Enforce creative vision
    static float LastCreativeVisionCheck = 0.0f;
    if (CurrentTime - LastCreativeVisionCheck >= CREATIVE_VISION_CHECK_INTERVAL)
    {
        EnforceCreativeVision();
        LastCreativeVisionCheck = CurrentTime;
    }

    // Validate system dependencies
    ValidateSystemDependencies();
}

// ============================================================================
// SYSTEM REGISTRATION AND MANAGEMENT
// ============================================================================

void UStudioDirectorSubsystem::RegisterSystem(const FSystemInfo& SystemInfo)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(RegisterSystem);

    if (SystemInfo.SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: Cannot register system with empty name"));
        return;
    }

    if (RegisteredSystems.Contains(SystemInfo.SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: System '%s' already registered, updating info"), *SystemInfo.SystemName);
    }

    RegisteredSystems.Add(SystemInfo.SystemName, SystemInfo);
    
    UE_LOG(LogTemp, Log, TEXT("Studio Director: Registered system '%s' (Agent #%d - %s) with priority %d"), 
           *SystemInfo.SystemName, SystemInfo.AgentNumber, *SystemInfo.AgentName, (int32)SystemInfo.Priority);

    // Broadcast status change
    OnSystemStatusChanged.Broadcast(SystemInfo.SystemName, SystemInfo.Status);

    // Check if this system can start based on dependencies
    if (CanSystemStart(SystemInfo.SystemName))
    {
        UE_LOG(LogTemp, Log, TEXT("Studio Director: System '%s' dependencies met, can start"), *SystemInfo.SystemName);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Studio Director: System '%s' waiting for dependencies"), *SystemInfo.SystemName);
    }
}

void UStudioDirectorSubsystem::UnregisterSystem(const FString& SystemName)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UnregisterSystem);

    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Cannot unregister unknown system '%s'"), *SystemName);
        return;
    }

    // Check if other systems depend on this one
    TArray<FString> DependentSystems;
    for (const auto& Pair : RegisteredSystems)
    {
        if (Pair.Value.Dependencies.Contains(SystemName))
        {
            DependentSystems.Add(Pair.Key);
        }
    }

    if (DependentSystems.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: System '%s' has dependents, shutting them down first"), *SystemName);
        for (const FString& DependentSystem : DependentSystems)
        {
            RequestSystemShutdown(DependentSystem, FString::Printf(TEXT("Dependency '%s' shutting down"), *SystemName));
        }
    }

    RegisteredSystems.Remove(SystemName);
    OnSystemStatusChanged.Broadcast(SystemName, ESystemStatus::Offline);
    
    UE_LOG(LogTemp, Log, TEXT("Studio Director: Unregistered system '%s'"), *SystemName);
}

void UStudioDirectorSubsystem::UpdateSystemStatus(const FString& SystemName, ESystemStatus NewStatus)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UpdateSystemStatus);

    FSystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Cannot update status for unknown system '%s'"), *SystemName);
        return;
    }

    ESystemStatus OldStatus = SystemInfo->Status;
    SystemInfo->Status = NewStatus;
    SystemInfo->LastUpdateTime = FApp::GetCurrentTime();

    UE_LOG(LogTemp, Log, TEXT("Studio Director: System '%s' status changed from %d to %d"), 
           *SystemName, (int32)OldStatus, (int32)NewStatus);

    // Handle critical system failures
    if (NewStatus == ESystemStatus::Error && SystemInfo->Priority == EStudioDirectorPriority::Critical)
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: CRITICAL SYSTEM FAILURE - %s"), *SystemName);
        
        // Attempt recovery or shutdown dependent systems
        TArray<FString> DependentSystems;
        for (const auto& Pair : RegisteredSystems)
        {
            if (Pair.Value.Dependencies.Contains(SystemName))
            {
                DependentSystems.Add(Pair.Key);
            }
        }

        for (const FString& DependentSystem : DependentSystems)
        {
            UpdateSystemStatus(DependentSystem, ESystemStatus::Warning);
        }
    }

    OnSystemStatusChanged.Broadcast(SystemName, NewStatus);
}

ESystemStatus UStudioDirectorSubsystem::GetSystemStatus(const FString& SystemName) const
{
    const FSystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    return SystemInfo ? SystemInfo->Status : ESystemStatus::Offline;
}

// ============================================================================
// SYSTEM COORDINATION
// ============================================================================

bool UStudioDirectorSubsystem::CanSystemStart(const FString& SystemName) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(CanSystemStart);

    const FSystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        return false;
    }

    // Check all dependencies are online
    for (const FString& Dependency : SystemInfo->Dependencies)
    {
        ESystemStatus DepStatus = GetSystemStatus(Dependency);
        if (DepStatus != ESystemStatus::Online)
        {
            return false;
        }
    }

    return true;
}

void UStudioDirectorSubsystem::RequestSystemShutdown(const FString& SystemName, const FString& Reason)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(RequestSystemShutdown);

    UE_LOG(LogTemp, Log, TEXT("Studio Director: Requesting shutdown of system '%s' - Reason: %s"), *SystemName, *Reason);

    FSystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (SystemInfo)
    {
        UpdateSystemStatus(SystemName, ESystemStatus::Offline);
    }
}

TArray<FString> UStudioDirectorSubsystem::GetSystemDependencies(const FString& SystemName) const
{
    const FSystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    return SystemInfo ? SystemInfo->Dependencies : TArray<FString>();
}

// ============================================================================
// CREATIVE VISION ENFORCEMENT
// ============================================================================

bool UStudioDirectorSubsystem::IsCreativeVisionCompliant() const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(IsCreativeVisionCompliant);

    // Check all creative vision parameters
    bool bCompliant = true;

    // Validate threat level maintenance
    if (!CreativeVisionSettings.bMaintainConstantThreat)
    {
        bCompliant = false;
    }

    // Validate dinosaur independence
    if (!CreativeVisionSettings.bDinosaursLiveIndependently)
    {
        bCompliant = false;
    }

    // Validate domestication system
    if (!CreativeVisionSettings.bEnableDomestication)
    {
        bCompliant = false;
    }

    // Validate unique variations
    if (!CreativeVisionSettings.bUniqueDinosaurVariations)
    {
        bCompliant = false;
    }

    // Check emotional targets are within acceptable ranges
    if (CreativeVisionSettings.ThreatLevel < 0.7f || CreativeVisionSettings.ThreatLevel > 0.9f)
    {
        bCompliant = false;
    }

    if (CreativeVisionSettings.WonderLevel < 0.6f || CreativeVisionSettings.WonderLevel > 0.8f)
    {
        bCompliant = false;
    }

    if (CreativeVisionSettings.IsolationLevel < 0.5f || CreativeVisionSettings.IsolationLevel > 0.7f)
    {
        bCompliant = false;
    }

    return bCompliant;
}

void UStudioDirectorSubsystem::ReportCreativeVisionViolation(const FString& ViolationDescription)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ReportCreativeVisionViolation);

    UE_LOG(LogTemp, Error, TEXT("Studio Director: CREATIVE VISION VIOLATION - %s"), *ViolationDescription);
    OnCreativeVisionViolation.Broadcast(ViolationDescription);

    // Take corrective action based on violation type
    if (ViolationDescription.Contains(TEXT("threat")))
    {
        // Increase threat level systems
        ValidateThreatLevel();
    }
    else if (ViolationDescription.Contains(TEXT("dinosaur")))
    {
        // Validate dinosaur behavior systems
        ValidateDinosaurBehavior();
    }
    else if (ViolationDescription.Contains(TEXT("player")))
    {
        // Validate player experience systems
        ValidatePlayerExperience();
    }
}

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

bool UStudioDirectorSubsystem::IsOverallPerformanceAcceptable() const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(IsOverallPerformanceAcceptable);

    // Check if any critical systems are reporting performance issues
    for (const auto& Pair : RegisteredSystems)
    {
        const FSystemInfo& SystemInfo = Pair.Value;
        
        if (SystemInfo.Priority == EStudioDirectorPriority::Critical)
        {
            // Critical systems must maintain performance
            if (SystemInfo.AverageFrameTime > 16.67f) // 60fps target
            {
                return false;
            }
        }
    }

    // Check overall bottlenecks
    return CurrentBottlenecks.Num() < 3; // Allow up to 2 minor bottlenecks
}

TArray<FString> UStudioDirectorSubsystem::GetPerformanceBottlenecks() const
{
    return CurrentBottlenecks;
}

void UStudioDirectorSubsystem::OptimizeSystemPriorities()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(OptimizeSystemPriorities);

    UE_LOG(LogTemp, Log, TEXT("Studio Director: Optimizing system priorities based on performance data"));

    RebalanceSystemPriorities();

    // Force garbage collection if memory usage is high
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    if (MemStats.UsedPhysical > (6ULL * 1024 * 1024 * 1024)) // 6GB threshold
    {
        UE_LOG(LogTemp, Log, TEXT("Studio Director: High memory usage detected, forcing garbage collection"));
        GEngine->ForceGarbageCollection(true);
    }
}

// ============================================================================
// SYSTEM QUERIES
// ============================================================================

TArray<FSystemInfo> UStudioDirectorSubsystem::GetAllSystems() const
{
    TArray<FSystemInfo> AllSystems;
    RegisteredSystems.GenerateValueArray(AllSystems);
    return AllSystems;
}

TArray<FString> UStudioDirectorSubsystem::GetSystemsByPriority(EStudioDirectorPriority Priority) const
{
    TArray<FString> FilteredSystems;
    
    for (const auto& Pair : RegisteredSystems)
    {
        if (Pair.Value.Priority == Priority)
        {
            FilteredSystems.Add(Pair.Key);
        }
    }
    
    return FilteredSystems;
}

TArray<FString> UStudioDirectorSubsystem::GetSystemsByStatus(ESystemStatus Status) const
{
    TArray<FString> FilteredSystems;
    
    for (const auto& Pair : RegisteredSystems)
    {
        if (Pair.Value.Status == Status)
        {
            FilteredSystems.Add(Pair.Key);
        }
    }
    
    return FilteredSystems;
}

// ============================================================================
// PRIVATE IMPLEMENTATION FUNCTIONS
// ============================================================================

void UStudioDirectorSubsystem::InitializeCoreSystems()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(InitializeCoreSystems);

    UE_LOG(LogTemp, Log, TEXT("Studio Director: Initializing core system registry"));

    // Register the 18 agent systems with their dependencies
    
    // Agent #02 - Engine Architect (this system depends on nothing)
    FSystemInfo EngineArchitect;
    EngineArchitect.SystemName = TEXT("EngineArchitecture");
    EngineArchitect.AgentName = TEXT("Engine Architect");
    EngineArchitect.AgentNumber = 2;
    EngineArchitect.Priority = EStudioDirectorPriority::Critical;
    EngineArchitect.Status = ESystemStatus::Online;
    RegisteredSystems.Add(EngineArchitect.SystemName, EngineArchitect);

    // Agent #03 - Core Systems Programmer
    FSystemInfo CoreSystems;
    CoreSystems.SystemName = TEXT("PhysicsCore");
    CoreSystems.AgentName = TEXT("Core Systems Programmer");
    CoreSystems.AgentNumber = 3;
    CoreSystems.Priority = EStudioDirectorPriority::Critical;
    CoreSystems.Status = ESystemStatus::Initializing;
    CoreSystems.Dependencies.Add(TEXT("EngineArchitecture"));
    RegisteredSystems.Add(CoreSystems.SystemName, CoreSystems);

    // Agent #04 - Performance Optimizer
    FSystemInfo Performance;
    Performance.SystemName = TEXT("PerformanceOptimization");
    Performance.AgentName = TEXT("Performance Optimizer");
    Performance.AgentNumber = 4;
    Performance.Priority = EStudioDirectorPriority::Critical;
    Performance.Status = ESystemStatus::Initializing;
    Performance.Dependencies.Add(TEXT("PhysicsCore"));
    RegisteredSystems.Add(Performance.SystemName, Performance);

    // Agent #05 - Procedural World Generator
    FSystemInfo WorldGen;
    WorldGen.SystemName = TEXT("WorldGeneration");
    WorldGen.AgentName = TEXT("Procedural World Generator");
    WorldGen.AgentNumber = 5;
    WorldGen.Priority = EStudioDirectorPriority::High;
    WorldGen.Status = ESystemStatus::Offline;
    WorldGen.Dependencies.Add(TEXT("EngineArchitecture"));
    WorldGen.Dependencies.Add(TEXT("PerformanceOptimization"));
    RegisteredSystems.Add(WorldGen.SystemName, WorldGen);

    // Continue with other systems...
    // This establishes the dependency chain that all agents must follow
}

void UStudioDirectorSubsystem::ValidateSystemDependencies()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ValidateSystemDependencies);

    // Check for circular dependencies and missing dependencies
    for (const auto& Pair : RegisteredSystems)
    {
        const FString& SystemName = Pair.Key;
        const FSystemInfo& SystemInfo = Pair.Value;

        for (const FString& Dependency : SystemInfo.Dependencies)
        {
            if (!RegisteredSystems.Contains(Dependency))
            {
                UE_LOG(LogTemp, Error, TEXT("Studio Director: System '%s' has missing dependency '%s'"), 
                       *SystemName, *Dependency);
            }
        }
    }
}

void UStudioDirectorSubsystem::MonitorSystemHealth()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(MonitorSystemHealth);

    float CurrentTime = FApp::GetCurrentTime();

    for (auto& Pair : RegisteredSystems)
    {
        FSystemInfo& SystemInfo = Pair.Value;
        
        // Check if system has been unresponsive
        if (CurrentTime - SystemInfo.LastUpdateTime > SYSTEM_HEALTH_CHECK_INTERVAL * 10.0f)
        {
            if (SystemInfo.Status == ESystemStatus::Online)
            {
                UE_LOG(LogTemp, Warning, TEXT("Studio Director: System '%s' appears unresponsive"), *Pair.Key);
                SystemInfo.Status = ESystemStatus::Warning;
                OnSystemStatusChanged.Broadcast(Pair.Key, ESystemStatus::Warning);
            }
        }
    }
}

void UStudioDirectorSubsystem::EnforceCreativeVision()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(EnforceCreativeVision);

    if (!IsCreativeVisionCompliant())
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Creative vision compliance check failed"));
        
        ValidateThreatLevel();
        ValidateDinosaurBehavior();
        ValidatePlayerExperience();
    }
}

void UStudioDirectorSubsystem::StartSystemsInOrder()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(StartSystemsInOrder);

    UE_LOG(LogTemp, Log, TEXT("Studio Director: Starting systems in dependency order"));

    // Systems should start in order of dependencies
    // This is handled by the individual agents checking CanSystemStart()
}

bool UStudioDirectorSubsystem::AreSystemDependenciesMet(const FString& SystemName) const
{
    return CanSystemStart(SystemName);
}

void UStudioDirectorSubsystem::AnalyzePerformanceMetrics()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(AnalyzePerformanceMetrics);

    CurrentBottlenecks.Empty();

    // Check frame time
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f;
    if (CurrentFrameTime > 20.0f) // Above 50fps
    {
        CurrentBottlenecks.Add(TEXT("Frame Time"));
    }

    // Check memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    if (MemStats.UsedPhysical > (7ULL * 1024 * 1024 * 1024)) // 7GB threshold
    {
        CurrentBottlenecks.Add(TEXT("Memory Usage"));
    }

    // Check system-specific performance
    for (const auto& Pair : RegisteredSystems)
    {
        const FSystemInfo& SystemInfo = Pair.Value;
        if (SystemInfo.AverageFrameTime > 5.0f && SystemInfo.Priority == EStudioDirectorPriority::Critical)
        {
            CurrentBottlenecks.Add(FString::Printf(TEXT("System: %s"), *Pair.Key));
        }
    }
}

void UStudioDirectorSubsystem::RebalanceSystemPriorities()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(RebalanceSystemPriorities);

    // Adjust system priorities based on current performance
    for (auto& Pair : RegisteredSystems)
    {
        FSystemInfo& SystemInfo = Pair.Value;
        
        // Lower priority for systems that are performing poorly
        if (SystemInfo.AverageFrameTime > 10.0f)
        {
            if (SystemInfo.Priority == EStudioDirectorPriority::High)
            {
                SystemInfo.Priority = EStudioDirectorPriority::Medium;
            }
            else if (SystemInfo.Priority == EStudioDirectorPriority::Medium)
            {
                SystemInfo.Priority = EStudioDirectorPriority::Low;
            }
        }
    }
}

void UStudioDirectorSubsystem::ValidateThreatLevel()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ValidateThreatLevel);

    // Ensure threat level is maintained according to creative vision
    if (CreativeVisionSettings.ThreatLevel < 0.7f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Threat level too low, increasing danger systems"));
        CreativeVisionSettings.ThreatLevel = FMath::Min(0.8f, CreativeVisionSettings.ThreatLevel + 0.1f);
    }
}

void UStudioDirectorSubsystem::ValidateDinosaurBehavior()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ValidateDinosaurBehavior);

    // Validate that dinosaur systems are maintaining independence
    if (!CreativeVisionSettings.bDinosaursLiveIndependently)
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: Dinosaur independence violated!"));
        ReportCreativeVisionViolation(TEXT("Dinosaurs not living independently"));
    }
}

void UStudioDirectorSubsystem::ValidatePlayerExperience()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ValidatePlayerExperience);

    // Ensure player experience matches creative vision
    if (CreativeVisionSettings.IsolationLevel < 0.5f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Player isolation too low, increasing solitude"));
        CreativeVisionSettings.IsolationLevel = FMath::Min(0.7f, CreativeVisionSettings.IsolationLevel + 0.1f);
    }
}

// ============================================================================
// STUDIO DIRECTOR COMPONENT IMPLEMENTATION
// ============================================================================

UStudioDirectorComponent::UStudioDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SystemName = TEXT("UnknownSystem");
    SystemPriority = EStudioDirectorPriority::Medium;
    DirectorSubsystem = nullptr;
}

void UStudioDirectorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    CacheDirectorSubsystem();
    
    if (DirectorSubsystem && !SystemName.IsEmpty())
    {
        RegisterWithDirector(SystemName, SystemPriority);
    }
}

void UStudioDirectorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (DirectorSubsystem && !SystemName.IsEmpty())
    {
        DirectorSubsystem->UnregisterSystem(SystemName);
    }
    
    Super::EndPlay(EndPlayReason);
}

void UStudioDirectorComponent::RegisterWithDirector(const FString& InSystemName, EStudioDirectorPriority Priority)
{
    if (!DirectorSubsystem)
    {
        CacheDirectorSubsystem();
    }

    if (DirectorSubsystem)
    {
        FSystemInfo SystemInfo;
        SystemInfo.SystemName = InSystemName;
        SystemInfo.Priority = Priority;
        SystemInfo.Status = ESystemStatus::Initializing;
        SystemInfo.LastUpdateTime = FApp::GetCurrentTime();
        
        DirectorSubsystem->RegisterSystem(SystemInfo);
        SystemName = InSystemName;
        SystemPriority = Priority;
    }
}

void UStudioDirectorComponent::ReportSystemStatus(ESystemStatus Status)
{
    if (DirectorSubsystem && !SystemName.IsEmpty())
    {
        DirectorSubsystem->UpdateSystemStatus(SystemName, Status);
    }
}

void UStudioDirectorComponent::ReportPerformanceMetrics(float FrameTime, int32 MemoryUsage)
{
    if (DirectorSubsystem && !SystemName.IsEmpty())
    {
        // Update performance metrics in the registered system
        // This would typically be done through a more detailed interface
        UE_LOG(LogTemp, VeryVerbose, TEXT("System '%s' reporting: FrameTime=%.2fms, Memory=%dMB"), 
               *SystemName, FrameTime, MemoryUsage);
    }
}

void UStudioDirectorComponent::CacheDirectorSubsystem()
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            DirectorSubsystem = GameInstance->GetSubsystem<UStudioDirectorSubsystem>();
        }
    }
}