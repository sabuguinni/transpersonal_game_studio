#include "Core/EngineArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"

// ============================================================================
// UEng_ArchitectureSystem Implementation
// ============================================================================

UEng_ArchitectureSystem::UEng_ArchitectureSystem()
{
    bIsInitialized = false;
    LastValidationTime = 0.0f;
    TargetPerformanceTier = EEng_PerformanceTier::Medium;
    bEnableArchitectureValidation = true;
    bEnablePerformanceMonitoring = true;
    SystemUpdateFrequency = 1.0f; // Update every second
}

void UEng_ArchitectureSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Initializing Architecture System"));

    // Create subsystem managers
    SystemManager = NewObject<UEng_SystemManager>(this);
    PerformanceMonitor = NewObject<UEng_PerformanceMonitor>(this);

    // Set default world configuration
    WorldConfig = FEng_WorldConfiguration();
    
    // Register core systems that will be implemented by other agents
    RegisterSystem(TEXT("PhysicsCore"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("WorldGeneration"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("AISystem"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("AudioSystem"), EEng_SystemPriority::Medium);
    RegisterSystem(TEXT("VFXSystem"), EEng_SystemPriority::Low);

    // Set up system dependencies
    AddSystemDependency(TEXT("WorldGeneration"), TEXT("PhysicsCore"));
    AddSystemDependency(TEXT("AISystem"), TEXT("PhysicsCore"));
    AddSystemDependency(TEXT("AudioSystem"), TEXT("WorldGeneration"));
    AddSystemDependency(TEXT("VFXSystem"), TEXT("WorldGeneration"));

    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Architecture System initialized successfully"));
}

void UEng_ArchitectureSystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Shutting down Architecture System"));
    
    ShutdownAllSystems();
    
    SystemManager = nullptr;
    PerformanceMonitor = nullptr;
    
    bIsInitialized = false;
    
    Super::Deinitialize();
}

bool UEng_ArchitectureSystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UEng_ArchitectureSystem::InitializeAllSystems()
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitecture: Cannot initialize systems - Architecture not initialized"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Initializing all registered systems"));

    // Initialize systems in dependency order
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);

    for (const FString& SystemName : SystemNames)
    {
        if (CanSystemStart(SystemName))
        {
            if (SystemManager->InitializeSystem(SystemName))
            {
                FEng_SystemInfo& SystemInfo = RegisteredSystems[SystemName];
                SystemInfo.CurrentState = EEng_SystemState::Active;
                SystemInfo.InitializationTime = FPlatformTime::Seconds();
                
                UE_LOG(LogTemp, Log, TEXT("EngineArchitecture: System '%s' initialized successfully"), *SystemName);
            }
            else
            {
                FEng_SystemInfo& SystemInfo = RegisteredSystems[SystemName];
                SystemInfo.CurrentState = EEng_SystemState::Error;
                SystemInfo.ErrorCount++;
                
                UE_LOG(LogTemp, Error, TEXT("EngineArchitecture: Failed to initialize system '%s'"), *SystemName);
            }
        }
    }

    LogArchitectureState();
}

void UEng_ArchitectureSystem::ShutdownAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Shutting down all systems"));

    for (auto& SystemPair : RegisteredSystems)
    {
        FString SystemName = SystemPair.Key;
        FEng_SystemInfo& SystemInfo = SystemPair.Value;

        if (SystemInfo.CurrentState == EEng_SystemState::Active)
        {
            SystemManager->ShutdownSystem(SystemName);
            SystemInfo.CurrentState = EEng_SystemState::Shutdown;
        }
    }
}

bool UEng_ArchitectureSystem::RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: System '%s' already registered"), *SystemName);
        return false;
    }

    FEng_SystemInfo NewSystemInfo;
    NewSystemInfo.SystemName = SystemName;
    NewSystemInfo.Priority = Priority;
    NewSystemInfo.CurrentState = EEng_SystemState::Uninitialized;

    RegisteredSystems.Add(SystemName, NewSystemInfo);
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitecture: Registered system '%s' with priority %d"), 
           *SystemName, static_cast<int32>(Priority));
    
    return true;
}

bool UEng_ArchitectureSystem::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        return false;
    }

    // Shutdown system if it's running
    FEng_SystemInfo& SystemInfo = RegisteredSystems[SystemName];
    if (SystemInfo.CurrentState == EEng_SystemState::Active)
    {
        SystemManager->ShutdownSystem(SystemName);
    }

    RegisteredSystems.Remove(SystemName);
    SystemDependencies.Remove(SystemName);
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitecture: Unregistered system '%s'"), *SystemName);
    
    return true;
}

EEng_SystemState UEng_ArchitectureSystem::GetSystemState(const FString& SystemName) const
{
    if (const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return SystemInfo->CurrentState;
    }
    
    return EEng_SystemState::Uninitialized;
}

TArray<FEng_SystemInfo> UEng_ArchitectureSystem::GetAllSystemsInfo() const
{
    TArray<FEng_SystemInfo> SystemInfoArray;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        SystemInfoArray.Add(SystemPair.Value);
    }
    
    return SystemInfoArray;
}

FEng_PerformanceMetrics UEng_ArchitectureSystem::GetCurrentPerformanceMetrics() const
{
    if (PerformanceMonitor)
    {
        return PerformanceMonitor->GetLatestMetrics();
    }
    
    return FEng_PerformanceMetrics();
}

EEng_PerformanceTier UEng_ArchitectureSystem::GetCurrentPerformanceTier() const
{
    return TargetPerformanceTier;
}

void UEng_ArchitectureSystem::SetPerformanceTier(EEng_PerformanceTier NewTier)
{
    if (NewTier != TargetPerformanceTier)
    {
        TargetPerformanceTier = NewTier;
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Performance tier changed to %d"), static_cast<int32>(NewTier));
    }
}

void UEng_ArchitectureSystem::ConfigureWorld(const FEng_WorldConfiguration& Config)
{
    WorldConfig = Config;
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: World configured - Scale: %d, Size: %s"), 
           static_cast<int32>(Config.Scale), *Config.WorldSize.ToString());

    // Automatically enable World Partition for large worlds
    if (Config.WorldSize.X > EngineArchitecture::WORLD_PARTITION_THRESHOLD || 
        Config.WorldSize.Y > EngineArchitecture::WORLD_PARTITION_THRESHOLD)
    {
        WorldConfig.bUseWorldPartition = true;
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: World Partition enabled automatically for large world"));
    }
}

FEng_WorldConfiguration UEng_ArchitectureSystem::GetWorldConfiguration() const
{
    return WorldConfig;
}

bool UEng_ArchitectureSystem::ShouldUseWorldPartition() const
{
    return WorldConfig.bUseWorldPartition;
}

bool UEng_ArchitectureSystem::ValidateArchitecture()
{
    CachedWarnings.Empty();
    
    // Check system dependencies
    ValidateSystemDependencies();
    
    // Check performance thresholds
    CheckPerformanceThresholds();
    
    // Check world configuration
    if (WorldConfig.WorldSize.X > EngineArchitecture::WORLD_PARTITION_THRESHOLD && !WorldConfig.bUseWorldPartition)
    {
        CachedWarnings.Add(TEXT("Large world detected but World Partition is disabled"));
    }
    
    if (WorldConfig.MaxSimultaneousActors > EngineArchitecture::MAX_ACTORS_WITHOUT_PARTITION && !WorldConfig.bUseWorldPartition)
    {
        CachedWarnings.Add(TEXT("High actor count requires World Partition"));
    }
    
    LastValidationTime = FPlatformTime::Seconds();
    
    bool bIsValid = CachedWarnings.Num() == 0;
    UE_LOG(LogTemp, Log, TEXT("EngineArchitecture: Architecture validation %s - %d warnings"), 
           bIsValid ? TEXT("PASSED") : TEXT("FAILED"), CachedWarnings.Num());
    
    return bIsValid;
}

TArray<FString> UEng_ArchitectureSystem::GetArchitectureWarnings() const
{
    return CachedWarnings;
}

bool UEng_ArchitectureSystem::CanSystemStart(const FString& SystemName) const
{
    // Check if all dependencies are satisfied
    if (const TArray<FString>* Dependencies = SystemDependencies.Find(SystemName))
    {
        for (const FString& Dependency : *Dependencies)
        {
            EEng_SystemState DepState = GetSystemState(Dependency);
            if (DepState != EEng_SystemState::Active)
            {
                return false;
            }
        }
    }
    
    return true;
}

void UEng_ArchitectureSystem::AddSystemDependency(const FString& SystemName, const FString& DependsOn)
{
    TArray<FString>& Dependencies = SystemDependencies.FindOrAdd(SystemName);
    Dependencies.AddUnique(DependsOn);
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitecture: Added dependency: %s depends on %s"), *SystemName, *DependsOn);
}

void UEng_ArchitectureSystem::UpdateSystemStates()
{
    float CurrentTime = FPlatformTime::Seconds();
    
    for (auto& SystemPair : RegisteredSystems)
    {
        FEng_SystemInfo& SystemInfo = SystemPair.Value;
        SystemInfo.LastUpdateTime = CurrentTime;
    }
}

void UEng_ArchitectureSystem::ValidateSystemDependencies()
{
    for (const auto& DepPair : SystemDependencies)
    {
        const FString& SystemName = DepPair.Key;
        const TArray<FString>& Dependencies = DepPair.Value;
        
        for (const FString& Dependency : Dependencies)
        {
            if (!RegisteredSystems.Contains(Dependency))
            {
                CachedWarnings.Add(FString::Printf(TEXT("System '%s' depends on unregistered system '%s'"), 
                                                  *SystemName, *Dependency));
            }
        }
    }
}

void UEng_ArchitectureSystem::CheckPerformanceThresholds()
{
    if (PerformanceMonitor && bEnablePerformanceMonitoring)
    {
        FEng_PerformanceMetrics Metrics = PerformanceMonitor->GetLatestMetrics();
        
        if (Metrics.FrameTime > EngineArchitecture::MAX_FRAME_TIME_MS * 1.5f)
        {
            CachedWarnings.Add(TEXT("Frame time exceeds target by 50%"));
        }
        
        if (Metrics.MemoryUsageMB > EngineArchitecture::MAX_MEMORY_PC * 0.9f)
        {
            CachedWarnings.Add(TEXT("Memory usage approaching limit"));
        }
    }
}

void UEng_ArchitectureSystem::LogArchitectureState() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURE STATE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Performance Tier: %d"), static_cast<int32>(TargetPerformanceTier));
    UE_LOG(LogTemp, Warning, TEXT("World Partition: %s"), WorldConfig.bUseWorldPartition ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("World Size: %s"), *WorldConfig.WorldSize.ToString());
    UE_LOG(LogTemp, Warning, TEXT("========================="));
}

// ============================================================================
// UEng_SystemManager Implementation
// ============================================================================

UEng_SystemManager::UEng_SystemManager()
{
}

bool UEng_SystemManager::InitializeSystem(const FString& SystemName)
{
    if (ManagedSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("SystemManager: System '%s' already managed"), *SystemName);
        return true;
    }

    // Create system info
    FEng_SystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.CurrentState = EEng_SystemState::Initializing;
    SystemInfo.InitializationTime = FPlatformTime::Seconds();

    ManagedSystems.Add(SystemName, SystemInfo);
    
    UE_LOG(LogTemp, Log, TEXT("SystemManager: System '%s' added to management"), *SystemName);
    return true;
}

bool UEng_SystemManager::ShutdownSystem(const FString& SystemName)
{
    if (FEng_SystemInfo* SystemInfo = ManagedSystems.Find(SystemName))
    {
        SystemInfo->CurrentState = EEng_SystemState::Shutdown;
        UE_LOG(LogTemp, Log, TEXT("SystemManager: System '%s' shutdown"), *SystemName);
        return true;
    }
    
    return false;
}

void UEng_SystemManager::UpdateSystems(float DeltaTime)
{
    for (auto& SystemPair : ManagedSystems)
    {
        FEng_SystemInfo& SystemInfo = SystemPair.Value;
        SystemInfo.LastUpdateTime = FPlatformTime::Seconds();
    }
}

bool UEng_SystemManager::IsSystemActive(const FString& SystemName) const
{
    if (const FEng_SystemInfo* SystemInfo = ManagedSystems.Find(SystemName))
    {
        return SystemInfo->CurrentState == EEng_SystemState::Active;
    }
    
    return false;
}

TArray<FString> UEng_SystemManager::GetActiveSystemNames() const
{
    TArray<FString> ActiveSystems;
    
    for (const auto& SystemPair : ManagedSystems)
    {
        if (SystemPair.Value.CurrentState == EEng_SystemState::Active)
        {
            ActiveSystems.Add(SystemPair.Key);
        }
    }
    
    return ActiveSystems;
}

int32 UEng_SystemManager::GetSystemCount() const
{
    return ManagedSystems.Num();
}

void UEng_SystemManager::SortSystemsByPriority()
{
    // Implementation for priority-based system ordering
    // This would be used during initialization
}

bool UEng_SystemManager::ValidateSystemInitialization(const FString& SystemName) const
{
    return ManagedSystems.Contains(SystemName);
}

// ============================================================================
// UEng_PerformanceMonitor Implementation
// ============================================================================

UEng_PerformanceMonitor::UEng_PerformanceMonitor()
{
    CurrentTier = EEng_PerformanceTier::Medium;
    FrameTimeHistory.Reserve(MAX_HISTORY_SAMPLES);
}

void UEng_PerformanceMonitor::UpdateMetrics()
{
    CollectFrameTimeMetrics();
    CollectMemoryMetrics();
    CollectRenderMetrics();
    
    // Update performance tier recommendation
    CurrentTier = CalculateOptimalTier();
}

FEng_PerformanceMetrics UEng_PerformanceMonitor::GetLatestMetrics() const
{
    return CurrentMetrics;
}

bool UEng_PerformanceMonitor::IsPerformanceAcceptable() const
{
    return CurrentMetrics.FrameTime <= EngineArchitecture::MAX_FRAME_TIME_MS * 1.2f; // 20% tolerance
}

EEng_PerformanceTier UEng_PerformanceMonitor::RecommendPerformanceTier() const
{
    return CalculateOptimalTier();
}

TArray<float> UEng_PerformanceMonitor::GetFrameTimeHistory(int32 SampleCount) const
{
    int32 StartIndex = FMath::Max(0, FrameTimeHistory.Num() - SampleCount);
    return TArray<float>(FrameTimeHistory.GetData() + StartIndex, FrameTimeHistory.Num() - StartIndex);
}

float UEng_PerformanceMonitor::GetAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Sum = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Sum += FrameTime;
    }
    
    return Sum / FrameTimeHistory.Num();
}

void UEng_PerformanceMonitor::CollectFrameTimeMetrics()
{
    // Get current frame time
    float DeltaTime = FApp::GetDeltaTime();
    float FrameTimeMs = DeltaTime * 1000.0f;
    
    CurrentMetrics.FrameTime = FrameTimeMs;
    CurrentMetrics.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
    CurrentMetrics.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    
    // Add to history
    FrameTimeHistory.Add(FrameTimeMs);
    if (FrameTimeHistory.Num() > MAX_HISTORY_SAMPLES)
    {
        FrameTimeHistory.RemoveAt(0);
    }
}

void UEng_PerformanceMonitor::CollectMemoryMetrics()
{
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

void UEng_PerformanceMonitor::CollectRenderMetrics()
{
    // These would be collected from render thread
    // For now, set placeholder values
    CurrentMetrics.DrawCalls = 1000; // Placeholder
    CurrentMetrics.Triangles = 100000; // Placeholder
    CurrentMetrics.GPUTime = 8.0f; // Placeholder
}

EEng_PerformanceTier UEng_PerformanceMonitor::CalculateOptimalTier() const
{
    float AvgFrameTime = GetAverageFrameTime();
    
    if (AvgFrameTime <= 8.0f) // 120+ FPS
    {
        return EEng_PerformanceTier::Ultra;
    }
    else if (AvgFrameTime <= 16.67f) // 60+ FPS
    {
        return EEng_PerformanceTier::High;
    }
    else if (AvgFrameTime <= 33.33f) // 30+ FPS
    {
        return EEng_PerformanceTier::Medium;
    }
    else if (AvgFrameTime <= 50.0f) // 20+ FPS
    {
        return EEng_PerformanceTier::Low;
    }
    else
    {
        return EEng_PerformanceTier::Potato;
    }
}