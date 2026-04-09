// Copyright Transpersonal Game Studio. All Rights Reserved.
// TranspersonalCoreSubsystem.cpp - Master core subsystem implementation

#include "TranspersonalCoreSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "TimerManager.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY(LogTranspersonalCore);

// Critical system initialization order - DO NOT CHANGE
const TArray<FString> UTranspersonalCoreSubsystem::CoreSystemInitializationOrder = {
    TEXT("PhysicsCore"),
    TEXT("RenderingCore"), 
    TEXT("AudioCore"),
    TEXT("AICore"),
    TEXT("GameplayCore")
};

void UTranspersonalCoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTranspersonalCore, Warning, TEXT("=== TRANSPERSONAL CORE SUBSYSTEM INITIALIZING ==="));
    UE_LOG(LogTranspersonalCore, Warning, TEXT("Engine Architect establishing technical foundation..."));
    
    // Initialize architecture settings with defaults
    WorldPartitionSettings = FTranspersonalWorldPartitionSettings();
    NaniteSettings = FTranspersonalNaniteSettings();
    LumenSettings = FTranspersonalLumenSettings();
    VSMSettings = FTranspersonalVSMSettings();
    MassAISettings = FTranspersonalMassAISettings();
    PerformanceTargets = FTranspersonalPerformanceTargets();
    
    // Initialize all core systems
    if (InitializeAllSystems())
    {
        UE_LOG(LogTranspersonalCore, Warning, TEXT("✅ All core systems initialized successfully"));
        
        // Start performance monitoring
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(PerformanceMonitoringTimer, 
                this, &UTranspersonalCoreSubsystem::MonitorPerformance, 
                PerformanceMonitoringInterval, true);
                
            World->GetTimerManager().SetTimer(ArchitecturalValidationTimer,
                this, &UTranspersonalCoreSubsystem::ValidateArchitecture,
                ArchitecturalValidationInterval, true);
        }
        
        bAllSystemsInitialized = true;
        OnAllSystemsReady.Broadcast();
    }
    else
    {
        UE_LOG(LogTranspersonalCore, Error, TEXT("❌ Failed to initialize core systems"));
    }
}

void UTranspersonalCoreSubsystem::Deinitialize()
{
    UE_LOG(LogTranspersonalCore, Warning, TEXT("=== TRANSPERSONAL CORE SUBSYSTEM SHUTTING DOWN ==="));
    
    ShutdownAllSystems();
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceMonitoringTimer);
        World->GetTimerManager().ClearTimer(ArchitecturalValidationTimer);
    }
    
    Super::Deinitialize();
}

bool UTranspersonalCoreSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Always create this subsystem - it's critical for the engine
    return true;
}

UTranspersonalCoreSubsystem* UTranspersonalCoreSubsystem::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UTranspersonalCoreSubsystem>();
        }
    }
    return nullptr;
}

bool UTranspersonalCoreSubsystem::InitializeAllSystems()
{
    UE_LOG(LogTranspersonalCore, Warning, TEXT("Initializing systems in architectural order..."));
    
    bool bAllSuccessful = true;
    
    // Initialize systems in the correct order
    InitializeSystemsInOrder();
    
    // Validate that all critical systems are ready
    for (const FString& SystemName : CoreSystemInitializationOrder)
    {
        if (!IsSystemReady(SystemName))
        {
            UE_LOG(LogTranspersonalCore, Error, TEXT("Critical system failed to initialize: %s"), *SystemName);
            bAllSuccessful = false;
        }
    }
    
    return bAllSuccessful;
}

void UTranspersonalCoreSubsystem::InitializeSystemsInOrder()
{
    // Initialize Physics Core
    if (InitializePhysicsCore())
    {
        FTranspersonalSystemInfo PhysicsInfo;
        PhysicsInfo.SystemName = TEXT("PhysicsCore");
        PhysicsInfo.Status = ETranspersonalSystemStatus::Ready;
        PhysicsInfo.InitializationTime = 0.1f; // Simulated
        SystemsStatus.Add(TEXT("PhysicsCore"), PhysicsInfo);
        UE_LOG(LogTranspersonalCore, Warning, TEXT("✅ PhysicsCore initialized"));
    }
    
    // Initialize Rendering Core
    if (InitializeRenderingCore())
    {
        FTranspersonalSystemInfo RenderingInfo;
        RenderingInfo.SystemName = TEXT("RenderingCore");
        RenderingInfo.Status = ETranspersonalSystemStatus::Ready;
        RenderingInfo.InitializationTime = 0.2f; // Simulated
        SystemsStatus.Add(TEXT("RenderingCore"), RenderingInfo);
        UE_LOG(LogTranspersonalCore, Warning, TEXT("✅ RenderingCore initialized"));
    }
    
    // Initialize Audio Core
    if (InitializeAudioCore())
    {
        FTranspersonalSystemInfo AudioInfo;
        AudioInfo.SystemName = TEXT("AudioCore");
        AudioInfo.Status = ETranspersonalSystemStatus::Ready;
        AudioInfo.InitializationTime = 0.05f; // Simulated
        SystemsStatus.Add(TEXT("AudioCore"), AudioInfo);
        UE_LOG(LogTranspersonalCore, Warning, TEXT("✅ AudioCore initialized"));
    }
    
    // Initialize AI Core
    if (InitializeAICore())
    {
        FTranspersonalSystemInfo AIInfo;
        AIInfo.SystemName = TEXT("AICore");
        AIInfo.Status = ETranspersonalSystemStatus::Ready;
        AIInfo.InitializationTime = 0.15f; // Simulated
        SystemsStatus.Add(TEXT("AICore"), AIInfo);
        UE_LOG(LogTranspersonalCore, Warning, TEXT("✅ AICore initialized"));
    }
    
    // Initialize Gameplay Core
    if (InitializeGameplayCore())
    {
        FTranspersonalSystemInfo GameplayInfo;
        GameplayInfo.SystemName = TEXT("GameplayCore");
        GameplayInfo.Status = ETranspersonalSystemStatus::Ready;
        GameplayInfo.InitializationTime = 0.08f; // Simulated
        SystemsStatus.Add(TEXT("GameplayCore"), GameplayInfo);
        UE_LOG(LogTranspersonalCore, Warning, TEXT("✅ GameplayCore initialized"));
    }
}

bool UTranspersonalCoreSubsystem::InitializePhysicsCore()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing Physics Core..."));
    
    // Enable Chaos Physics (UE5 default)
    // Validate physics settings
    // Initialize collision systems
    // Setup physics materials
    
    return true; // Simulated success
}

bool UTranspersonalCoreSubsystem::InitializeRenderingCore()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing Rendering Core..."));
    
    // Initialize World Partition
    InitializeWorldPartition();
    
    // Initialize Nanite
    InitializeNaniteSettings();
    
    // Initialize Lumen
    InitializeLumenSettings();
    
    // Initialize Virtual Shadow Maps
    InitializeVSMSettings();
    
    return true; // Simulated success
}

bool UTranspersonalCoreSubsystem::InitializeAudioCore()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing Audio Core..."));
    
    // Initialize MetaSounds
    // Setup audio streaming
    // Configure 3D audio
    // Initialize audio occlusion
    
    return true; // Simulated success
}

bool UTranspersonalCoreSubsystem::InitializeAICore()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing AI Core..."));
    
    // Initialize Mass AI
    InitializeMassAISettings();
    
    // Setup Behavior Trees
    // Initialize State Trees
    // Configure AI perception
    
    return true; // Simulated success
}

bool UTranspersonalCoreSubsystem::InitializeGameplayCore()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing Gameplay Core..."));
    
    // Initialize Game Framework
    // Setup Player Controllers
    // Initialize Game State
    // Configure Input System
    
    return true; // Simulated success
}

void UTranspersonalCoreSubsystem::ShutdownAllSystems()
{
    UE_LOG(LogTranspersonalCore, Warning, TEXT("Shutting down all systems..."));
    
    for (auto& SystemPair : SystemsStatus)
    {
        SystemPair.Value.Status = ETranspersonalSystemStatus::Uninitialized;
        OnSystemStatusChanged.Broadcast(SystemPair.Key);
    }
    
    SystemsStatus.Empty();
    bAllSystemsInitialized = false;
}

bool UTranspersonalCoreSubsystem::IsSystemReady(const FString& SystemName) const
{
    if (const FTranspersonalSystemInfo* SystemInfo = SystemsStatus.Find(SystemName))
    {
        return SystemInfo->Status == ETranspersonalSystemStatus::Ready;
    }
    return false;
}

ETranspersonalSystemStatus UTranspersonalCoreSubsystem::GetSystemStatus(const FString& SystemName) const
{
    if (const FTranspersonalSystemInfo* SystemInfo = SystemsStatus.Find(SystemName))
    {
        return SystemInfo->Status;
    }
    return ETranspersonalSystemStatus::Uninitialized;
}

TArray<FTranspersonalSystemInfo> UTranspersonalCoreSubsystem::GetAllSystemsInfo() const
{
    TArray<FTranspersonalSystemInfo> AllSystems;
    for (const auto& SystemPair : SystemsStatus)
    {
        AllSystems.Add(SystemPair.Value);
    }
    return AllSystems;
}

bool UTranspersonalCoreSubsystem::InitializeWorldPartition()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing World Partition with settings:"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("  Cell Size: %d meters"), WorldPartitionSettings.CellSize);
    UE_LOG(LogTranspersonalCore, Log, TEXT("  Loading Range: %.1f meters"), WorldPartitionSettings.LoadingRange);
    UE_LOG(LogTranspersonalCore, Log, TEXT("  HLOD Enabled: %s"), WorldPartitionSettings.bEnableHLOD ? TEXT("Yes") : TEXT("No"));
    
    return true;
}

bool UTranspersonalCoreSubsystem::InitializeNaniteSettings()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing Nanite with settings:"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("  Min Triangle Count: %d"), NaniteSettings.MinTriangleCount);
    UE_LOG(LogTranspersonalCore, Log, TEXT("  Nanite Foliage: %s"), NaniteSettings.bEnableNaniteFoliage ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("  Nanite Assemblies: %s"), NaniteSettings.bEnableNaniteAssemblies ? TEXT("Enabled") : TEXT("Disabled"));
    
    return true;
}

bool UTranspersonalCoreSubsystem::InitializeLumenSettings()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing Lumen with settings:"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("  Global Illumination: %s"), LumenSettings.bEnableLumenGI ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("  Reflections: %s"), LumenSettings.bEnableLumenReflections ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("  Scene View Distance: %.1f meters"), LumenSettings.LumenSceneViewDistance);
    
    return true;
}

bool UTranspersonalCoreSubsystem::InitializeVSMSettings()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing Virtual Shadow Maps with settings:"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("  VSM Enabled: %s"), VSMSettings.bEnableVirtualShadowMaps ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("  Resolution: %d"), VSMSettings.VirtualShadowMapResolution);
    UE_LOG(LogTranspersonalCore, Log, TEXT("  Caching: %s"), VSMSettings.bEnableCaching ? TEXT("Enabled") : TEXT("Disabled"));
    
    return true;
}

bool UTranspersonalCoreSubsystem::InitializeMassAISettings()
{
    UE_LOG(LogTranspersonalCore, Log, TEXT("Initializing Mass AI with settings:"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("  Max Concurrent Entities: %d"), MassAISettings.MaxConcurrentEntities);
    UE_LOG(LogTranspersonalCore, Log, TEXT("  Mass Movement: %s"), MassAISettings.bEnableMassMovement ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTranspersonalCore, Log, TEXT("  Mass Spawning: %s"), MassAISettings.bEnableMassSpawning ? TEXT("Enabled") : TEXT("Disabled"));
    
    return true;
}

void UTranspersonalCoreSubsystem::MonitorPerformance()
{
    // Get current frame time
    LastFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Check if we're exceeding performance targets
    if (LastFrameTime > PerformanceWarningThreshold)
    {
        UE_LOG(LogTranspersonalCore, Warning, TEXT("⚠️ Performance Warning: Frame time %.2fms exceeds threshold %.2fms"), 
            LastFrameTime, PerformanceWarningThreshold);
        OnPerformanceWarning.Broadcast(LastFrameTime);
    }
    
    // Log performance metrics periodically
    static int32 PerformanceLogCounter = 0;
    if (++PerformanceLogCounter >= 10) // Every 10 seconds
    {
        UE_LOG(LogTranspersonalCore, Log, TEXT("Performance Metrics - Frame Time: %.2fms, Memory: %.1fMB"), 
            LastFrameTime, GetCurrentMemoryUsage());
        PerformanceLogCounter = 0;
    }
}

void UTranspersonalCoreSubsystem::ValidateArchitecture()
{
    bArchitecturalComplianceValidated = ValidateArchitecturalCompliance();
    
    if (!bArchitecturalComplianceValidated)
    {
        TArray<FString> Violations = GetArchitecturalViolations();
        for (const FString& Violation : Violations)
        {
            UE_LOG(LogTranspersonalCore, Error, TEXT("Architectural Violation: %s"), *Violation);
        }
    }
}

bool UTranspersonalCoreSubsystem::ValidateArchitecturalCompliance() const
{
    // Validate all architectural requirements
    bool bCompliant = true;
    
    // Check World Partition
    if (!IsWorldPartitionActive())
    {
        bCompliant = false;
    }
    
    // Check Nanite
    if (!IsNaniteEnabled())
    {
        bCompliant = false;
    }
    
    // Check Lumen
    if (!IsLumenEnabled())
    {
        bCompliant = false;
    }
    
    // Check VSM
    if (!IsVSMEnabled())
    {
        bCompliant = false;
    }
    
    // Check Mass AI
    if (!IsMassAIEnabled())
    {
        bCompliant = false;
    }
    
    return bCompliant;
}

TArray<FString> UTranspersonalCoreSubsystem::GetArchitecturalViolations() const
{
    TArray<FString> Violations;
    
    if (!IsWorldPartitionActive())
    {
        Violations.Add(TEXT("World Partition is not active"));
    }
    
    if (!IsNaniteEnabled())
    {
        Violations.Add(TEXT("Nanite is not enabled"));
    }
    
    if (!IsLumenEnabled())
    {
        Violations.Add(TEXT("Lumen is not enabled"));
    }
    
    if (!IsVSMEnabled())
    {
        Violations.Add(TEXT("Virtual Shadow Maps are not enabled"));
    }
    
    if (!IsMassAIEnabled())
    {
        Violations.Add(TEXT("Mass AI is not enabled"));
    }
    
    return Violations;
}

bool UTranspersonalCoreSubsystem::IsPerformanceTargetMet() const
{
    return LastFrameTime <= (1000.0f / PerformanceTargets.TargetFPS_PC);
}

float UTranspersonalCoreSubsystem::GetCurrentFrameTime() const
{
    return LastFrameTime;
}

float UTranspersonalCoreSubsystem::GetCurrentMemoryUsage() const
{
    // Simplified memory usage calculation
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

bool UTranspersonalCoreSubsystem::IsWorldPartitionActive() const
{
    return WorldPartitionSettings.CellSize > 0;
}

bool UTranspersonalCoreSubsystem::IsNaniteEnabled() const
{
    return NaniteSettings.MinTriangleCount > 0;
}

bool UTranspersonalCoreSubsystem::IsLumenEnabled() const
{
    return LumenSettings.bEnableLumenGI;
}

bool UTranspersonalCoreSubsystem::IsVSMEnabled() const
{
    return VSMSettings.bEnableVirtualShadowMaps;
}

bool UTranspersonalCoreSubsystem::IsMassAIEnabled() const
{
    return MassAISettings.MaxConcurrentEntities > 0;
}