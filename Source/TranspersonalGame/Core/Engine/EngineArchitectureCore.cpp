#include "EngineArchitectureCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

UEngineArchitectureCore::UEngineArchitectureCore()
{
    // Set default values
    CurrentPerformanceTarget = EEng_PerformanceTarget::PC_High;
    TargetFrameTimeMS = 16.67f; // 60 FPS
    TargetMemoryBudgetMB = 8192.0f; // 8GB
    bEnforceStrictDependencies = true;
    bValidatePerformanceBudgets = true;
    WorldPartitionThresholdKM = 4.0f; // Use World Partition for worlds > 4km²
}

void UEngineArchitectureCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Initializing core architecture system"));
    
    InitializeArchitectureRules();
    SetupPerformanceTargets();
    RegisterCoreModules();
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Architecture system initialized successfully"));
}

void UEngineArchitectureCore::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Shutting down architecture system"));
    
    RegisteredSystems.Empty();
    SystemDependencies.Empty();
    
    Super::Deinitialize();
}

bool UEngineArchitectureCore::ValidateSystemIntegration(const FString& SystemName, const FString& TargetSystem)
{
    if (!bEnforceStrictDependencies)
    {
        return true;
    }
    
    // Check if systems are registered
    if (!RegisteredSystems.Contains(SystemName) || !RegisteredSystems.Contains(TargetSystem))
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitectureCore: Unregistered system in integration: %s -> %s"), 
               *SystemName, *TargetSystem);
        return false;
    }
    
    // Check priority hierarchy (lower priority systems cannot directly control higher priority ones)
    EEng_SystemPriority SourcePriority = RegisteredSystems[SystemName];
    EEng_SystemPriority TargetPriority = RegisteredSystems[TargetSystem];
    
    if (SourcePriority > TargetPriority)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Priority violation: %s (P%d) -> %s (P%d)"), 
               *SystemName, (int32)SourcePriority, *TargetSystem, (int32)TargetPriority);
        return false;
    }
    
    return true;
}

void UEngineArchitectureCore::RegisterSystemModule(const FString& ModuleName, EEng_SystemPriority Priority)
{
    if (!ValidateSystemPriority(Priority))
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitectureCore: Invalid priority for module %s"), *ModuleName);
        return;
    }
    
    RegisteredSystems.Add(ModuleName, Priority);
    SystemDependencies.Add(ModuleName, TArray<FString>());
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: Registered module %s with priority %d"), 
           *ModuleName, (int32)Priority);
}

bool UEngineArchitectureCore::CanSystemCommunicate(const FString& SourceSystem, const FString& TargetSystem)
{
    // Core systems can communicate with anyone
    if (SourceSystem.Contains(TEXT("Core")) || TargetSystem.Contains(TEXT("Core")))
    {
        return true;
    }
    
    // Check if there's a valid dependency path
    if (SystemDependencies.Contains(SourceSystem))
    {
        const TArray<FString>& Dependencies = SystemDependencies[SourceSystem];
        return Dependencies.Contains(TargetSystem);
    }
    
    return false;
}

void UEngineArchitectureCore::SetPerformanceTarget(EEng_PerformanceTarget Target)
{
    CurrentPerformanceTarget = Target;
    
    switch (Target)
    {
        case EEng_PerformanceTarget::PC_Low:
            TargetFrameTimeMS = 33.33f; // 30 FPS
            TargetMemoryBudgetMB = 4096.0f; // 4GB
            break;
        case EEng_PerformanceTarget::PC_Medium:
            TargetFrameTimeMS = 20.0f; // 50 FPS
            TargetMemoryBudgetMB = 6144.0f; // 6GB
            break;
        case EEng_PerformanceTarget::PC_High:
            TargetFrameTimeMS = 16.67f; // 60 FPS
            TargetMemoryBudgetMB = 8192.0f; // 8GB
            break;
        case EEng_PerformanceTarget::Console_PS5:
        case EEng_PerformanceTarget::Console_XboxSX:
            TargetFrameTimeMS = 16.67f; // 60 FPS
            TargetMemoryBudgetMB = 12288.0f; // 12GB (shared)
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Performance target set to %d (%.2fms, %.0fMB)"), 
           (int32)Target, TargetFrameTimeMS, TargetMemoryBudgetMB);
}

bool UEngineArchitectureCore::ValidatePerformanceBudget(const FString& SystemName, float CPUBudgetMS, float MemoryBudgetMB)
{
    if (!bValidatePerformanceBudgets)
    {
        return true;
    }
    
    bool bValid = true;
    
    if (CPUBudgetMS > TargetFrameTimeMS * 0.1f) // No system should use more than 10% of frame time
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: CPU budget violation for %s: %.2fms (max: %.2fms)"), 
               *SystemName, CPUBudgetMS, TargetFrameTimeMS * 0.1f);
        bValid = false;
    }
    
    if (MemoryBudgetMB > TargetMemoryBudgetMB * 0.2f) // No system should use more than 20% of memory
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Memory budget violation for %s: %.0fMB (max: %.0fMB)"), 
               *SystemName, MemoryBudgetMB, TargetMemoryBudgetMB * 0.2f);
        bValid = false;
    }
    
    return bValid;
}

bool UEngineArchitectureCore::ShouldUseWorldPartition(float WorldSizeKM) const
{
    return WorldSizeKM > WorldPartitionThresholdKM;
}

int32 UEngineArchitectureCore::GetRecommendedGridSize(float WorldSizeKM) const
{
    if (WorldSizeKM <= 1.0f)
    {
        return 512; // Small worlds - 512m grid
    }
    else if (WorldSizeKM <= 4.0f)
    {
        return 1024; // Medium worlds - 1km grid
    }
    else if (WorldSizeKM <= 16.0f)
    {
        return 2048; // Large worlds - 2km grid
    }
    else
    {
        return 4096; // Massive worlds - 4km grid
    }
}

TArray<FString> UEngineArchitectureCore::GetModuleDependencies(const FString& ModuleName)
{
    if (SystemDependencies.Contains(ModuleName))
    {
        return SystemDependencies[ModuleName];
    }
    
    return TArray<FString>();
}

bool UEngineArchitectureCore::ValidateModuleDependency(const FString& ModuleName, const FString& DependencyName)
{
    // Core modules can depend on anything
    if (ModuleName.Contains(TEXT("Core")))
    {
        return true;
    }
    
    // Check for circular dependencies
    if (SystemDependencies.Contains(DependencyName))
    {
        const TArray<FString>& DependencyDeps = SystemDependencies[DependencyName];
        if (DependencyDeps.Contains(ModuleName))
        {
            UE_LOG(LogTemp, Error, TEXT("EngineArchitectureCore: Circular dependency detected: %s <-> %s"), 
                   *ModuleName, *DependencyName);
            return false;
        }
    }
    
    return true;
}

void UEngineArchitectureCore::InitializeArchitectureRules()
{
    // Set up fundamental architectural rules
    bEnforceStrictDependencies = true;
    bValidatePerformanceBudgets = true;
    WorldPartitionThresholdKM = 4.0f;
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: Architecture rules initialized"));
}

void UEngineArchitectureCore::SetupPerformanceTargets()
{
    // Initialize with PC High target by default
    SetPerformanceTarget(EEng_PerformanceTarget::PC_High);
}

void UEngineArchitectureCore::RegisterCoreModules()
{
    // Register core engine modules with their priorities
    RegisterSystemModule(TEXT("Engine"), EEng_SystemPriority::Critical);
    RegisterSystemModule(TEXT("Physics"), EEng_SystemPriority::Critical);
    RegisterSystemModule(TEXT("Rendering"), EEng_SystemPriority::Critical);
    RegisterSystemModule(TEXT("Audio"), EEng_SystemPriority::High);
    RegisterSystemModule(TEXT("Input"), EEng_SystemPriority::High);
    RegisterSystemModule(TEXT("WorldGeneration"), EEng_SystemPriority::High);
    RegisterSystemModule(TEXT("AI"), EEng_SystemPriority::Medium);
    RegisterSystemModule(TEXT("UI"), EEng_SystemPriority::Medium);
    RegisterSystemModule(TEXT("VFX"), EEng_SystemPriority::Low);
    RegisterSystemModule(TEXT("Analytics"), EEng_SystemPriority::Low);
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: Core modules registered"));
}

bool UEngineArchitectureCore::ValidateSystemPriority(EEng_SystemPriority Priority) const
{
    return Priority >= EEng_SystemPriority::Low && Priority <= EEng_SystemPriority::Critical;
}

// World Architecture Manager Implementation
UWorldArchitectureManager::UWorldArchitectureManager()
{
    // Set default LOD distances
    DefaultLODDistances = {1000.0f, 2500.0f, 5000.0f, 10000.0f};
}

void UWorldArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("WorldArchitectureManager: Initializing world-specific architecture"));
    
    SetupCullingRules();
    InitializeLODSystem();
}

void UWorldArchitectureManager::PostInitialize()
{
    Super::PostInitialize();
    
    // Get the core architecture system
    if (UEngineArchitectureCore* CoreArch = GetGameInstance()->GetSubsystem<UEngineArchitectureCore>())
    {
        // Apply world-specific optimizations based on performance target
        OptimizeWorldForTarget(CoreArch->CurrentPerformanceTarget);
    }
}

void UWorldArchitectureManager::OptimizeWorldForTarget(EEng_PerformanceTarget Target)
{
    switch (Target)
    {
        case EEng_PerformanceTarget::PC_Low:
            // Aggressive culling and LOD
            DefaultLODDistances = {500.0f, 1000.0f, 2000.0f, 4000.0f};
            break;
        case EEng_PerformanceTarget::PC_Medium:
            // Balanced culling and LOD
            DefaultLODDistances = {750.0f, 1500.0f, 3000.0f, 6000.0f};
            break;
        case EEng_PerformanceTarget::PC_High:
        case EEng_PerformanceTarget::Console_PS5:
        case EEng_PerformanceTarget::Console_XboxSX:
            // Conservative culling and LOD
            DefaultLODDistances = {1000.0f, 2500.0f, 5000.0f, 10000.0f};
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("WorldArchitectureManager: Optimized for target %d"), (int32)Target);
}

void UWorldArchitectureManager::SetupLODChain(AActor* Actor, const TArray<float>& LODDistances)
{
    if (!Actor)
    {
        return;
    }
    
    // This would set up LOD components on the actor
    // Implementation depends on the specific actor type and LOD system used
    UE_LOG(LogTemp, Log, TEXT("WorldArchitectureManager: Setting up LOD chain for %s"), *Actor->GetName());
}

bool UWorldArchitectureManager::ShouldUseCulling(const FString& ActorType) const
{
    if (CullingRules.Contains(ActorType))
    {
        return CullingRules[ActorType];
    }
    
    // Default to using culling for most actor types
    return true;
}

void UWorldArchitectureManager::SetupCullingRules()
{
    // Define which actor types should use culling
    CullingRules.Add(TEXT("StaticMeshActor"), true);
    CullingRules.Add(TEXT("SkeletalMeshActor"), true);
    CullingRules.Add(TEXT("Foliage"), true);
    CullingRules.Add(TEXT("Landscape"), false); // Landscape has its own culling
    CullingRules.Add(TEXT("DirectionalLight"), false); // Lights don't need distance culling
    CullingRules.Add(TEXT("PlayerController"), false);
    CullingRules.Add(TEXT("GameMode"), false);
    
    UE_LOG(LogTemp, Log, TEXT("WorldArchitectureManager: Culling rules initialized"));
}

void UWorldArchitectureManager::InitializeLODSystem()
{
    // Initialize the LOD system with default settings
    UE_LOG(LogTemp, Log, TEXT("WorldArchitectureManager: LOD system initialized with %d levels"), 
           DefaultLODDistances.Num());
}