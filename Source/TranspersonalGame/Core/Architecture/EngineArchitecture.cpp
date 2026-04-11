#include "EngineArchitecture.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY_STATIC(LogTranspersonalArchitecture, Log, All);

void UEngineArchitectureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("=== TRANSPERSONAL GAME STUDIO - ENGINE ARCHITECTURE INITIALIZED ==="));
    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("Agent #02 - Engine Architect - Architecture Subsystem Online"));

    // Initialize constraints based on current performance tier
    InitializeConstraintsForTier(CurrentPerformanceTier);

    // Log architecture status
    LogArchitectureStatus();

    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("Architecture constraints enforced. All systems must comply."));
}

void UEngineArchitectureSubsystem::Deinitialize()
{
    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("Engine Architecture Subsystem shutting down"));
    
    // Clear all registered systems
    SystemBudgets.Empty();
    SystemPriorities.Empty();

    Super::Deinitialize();
}

bool UEngineArchitectureSubsystem::RegisterSystem(const FString& SystemName, ETranspersonalSystemPriority Priority, const FSystemResourceBudget& Budget)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTranspersonalArchitecture, Error, TEXT("Cannot register system with empty name"));
        return false;
    }

    if (SystemBudgets.Contains(SystemName))
    {
        UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("System %s already registered, updating budget"), *SystemName);
    }

    SystemBudgets.Add(SystemName, Budget);
    SystemPriorities.Add(SystemName, Priority);

    UE_LOG(LogTranspersonalArchitecture, Log, TEXT("Registered system: %s (Priority: %d, CPU: %.2fms, Memory: %dMB)"), 
           *SystemName, 
           (int32)Priority, 
           Budget.MaxCPUTimeMs, 
           Budget.MaxMemoryMB);

    return true;
}

bool UEngineArchitectureSubsystem::ValidateSystemConstraints(const FString& SystemName)
{
    if (!SystemBudgets.Contains(SystemName))
    {
        UE_LOG(LogTranspersonalArchitecture, Error, TEXT("System %s not registered"), *SystemName);
        return false;
    }

    const FSystemResourceBudget& Budget = SystemBudgets[SystemName];
    
    // Get current system performance (this would be implemented by each system)
    float CurrentCPUTime = GetSystemCPUTime(SystemName);
    int32 CurrentMemory = GetSystemMemoryUsage(SystemName);

    bool bWithinBudget = true;

    if (CurrentCPUTime > Budget.MaxCPUTimeMs)
    {
        UE_LOG(LogTranspersonalArchitecture, Error, TEXT("System %s exceeds CPU budget: %.2fms > %.2fms"), 
               *SystemName, CurrentCPUTime, Budget.MaxCPUTimeMs);
        bWithinBudget = false;
    }

    if (CurrentMemory > Budget.MaxMemoryMB)
    {
        UE_LOG(LogTranspersonalArchitecture, Error, TEXT("System %s exceeds memory budget: %dMB > %dMB"), 
               *SystemName, CurrentMemory, Budget.MaxMemoryMB);
        bWithinBudget = false;
    }

    return bWithinBudget;
}

void UEngineArchitectureSubsystem::EnforcePerformanceBudgets()
{
    UE_LOG(LogTranspersonalArchitecture, Log, TEXT("Enforcing performance budgets for %d systems"), SystemBudgets.Num());

    float TotalCPUTime = 0.0f;
    int32 TotalMemory = 0;
    int32 ViolatingSystemsCount = 0;

    for (const auto& SystemPair : SystemBudgets)
    {
        const FString& SystemName = SystemPair.Key;
        
        if (!ValidateSystemConstraints(SystemName))
        {
            ViolatingSystemsCount++;
        }

        TotalCPUTime += GetSystemCPUTime(SystemName);
        TotalMemory += GetSystemMemoryUsage(SystemName);
    }

    // Check overall performance targets
    if (TotalCPUTime > Constraints.TargetFrameTimeMs)
    {
        UE_LOG(LogTranspersonalArchitecture, Error, TEXT("CRITICAL: Total CPU time %.2fms exceeds target %.2fms"), 
               TotalCPUTime, Constraints.TargetFrameTimeMs);
    }

    if (TotalMemory > Constraints.MaxTotalMemoryMB)
    {
        UE_LOG(LogTranspersonalArchitecture, Error, TEXT("CRITICAL: Total memory %dMB exceeds budget %dMB"), 
               TotalMemory, Constraints.MaxTotalMemoryMB);
    }

    UE_LOG(LogTranspersonalArchitecture, Log, TEXT("Performance enforcement complete. Violations: %d"), ViolatingSystemsCount);
}

float UEngineArchitectureSubsystem::GetSystemCPUTime(const FString& SystemName)
{
    // This would be implemented by hooking into UE5's stat system
    // For now, return a placeholder value
    return 0.5f; // 0.5ms default
}

int32 UEngineArchitectureSubsystem::GetSystemMemoryUsage(const FString& SystemName)
{
    // This would be implemented by tracking memory allocations per system
    // For now, return a placeholder value
    return 50; // 50MB default
}

bool UEngineArchitectureSubsystem::IsSystemWithinBudget(const FString& SystemName)
{
    return ValidateSystemConstraints(SystemName);
}

void UEngineArchitectureSubsystem::SetPerformanceTier(ETranspersonalPerformanceTier NewTier)
{
    if (CurrentPerformanceTier != NewTier)
    {
        UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("Changing performance tier from %d to %d"), 
               (int32)CurrentPerformanceTier, (int32)NewTier);
        
        CurrentPerformanceTier = NewTier;
        InitializeConstraintsForTier(NewTier);
        
        // Re-validate all systems with new constraints
        ValidateAllSystems();
    }
}

void UEngineArchitectureSubsystem::InitializeConstraintsForTier(ETranspersonalPerformanceTier Tier)
{
    switch (Tier)
    {
    case ETranspersonalPerformanceTier::Ultra:
        Constraints.MaxWorldSizeKm = 16.0f;
        Constraints.MaxConcurrentActors = 150000;
        Constraints.MaxMassAgents = 75000;
        Constraints.TargetFrameTimeMs = 13.33f; // 75fps
        Constraints.MaxTotalMemoryMB = 16384;
        Constraints.MaxVRAMUsageMB = 12288;
        Constraints.LOD0Distance = 750.0f;
        Constraints.CullDistance = 15000.0f;
        break;

    case ETranspersonalPerformanceTier::High:
        Constraints.MaxWorldSizeKm = 12.0f;
        Constraints.MaxConcurrentActors = 100000;
        Constraints.MaxMassAgents = 50000;
        Constraints.TargetFrameTimeMs = 16.67f; // 60fps
        Constraints.MaxTotalMemoryMB = 12288;
        Constraints.MaxVRAMUsageMB = 8192;
        Constraints.LOD0Distance = 500.0f;
        Constraints.CullDistance = 10000.0f;
        break;

    case ETranspersonalPerformanceTier::Medium:
        Constraints.MaxWorldSizeKm = 8.0f;
        Constraints.MaxConcurrentActors = 75000;
        Constraints.MaxMassAgents = 35000;
        Constraints.TargetFrameTimeMs = 33.33f; // 30fps
        Constraints.MaxTotalMemoryMB = 8192;
        Constraints.MaxVRAMUsageMB = 6144;
        Constraints.LOD0Distance = 300.0f;
        Constraints.CullDistance = 7500.0f;
        break;

    case ETranspersonalPerformanceTier::Low:
        Constraints.MaxWorldSizeKm = 4.0f;
        Constraints.MaxConcurrentActors = 50000;
        Constraints.MaxMassAgents = 20000;
        Constraints.TargetFrameTimeMs = 33.33f; // 30fps
        Constraints.MaxTotalMemoryMB = 6144;
        Constraints.MaxVRAMUsageMB = 4096;
        Constraints.LOD0Distance = 200.0f;
        Constraints.CullDistance = 5000.0f;
        break;
    }

    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("Performance tier %d constraints applied"), (int32)Tier);
}

void UEngineArchitectureSubsystem::ValidateAllSystems()
{
    for (const auto& SystemPair : SystemBudgets)
    {
        ValidateSystemConstraints(SystemPair.Key);
    }
}

void UEngineArchitectureSubsystem::LogArchitectureStatus()
{
    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("=== ARCHITECTURE STATUS ==="));
    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("Performance Tier: %d"), (int32)CurrentPerformanceTier);
    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("Max World Size: %.1f km"), Constraints.MaxWorldSizeKm);
    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("Max Actors: %d"), Constraints.MaxConcurrentActors);
    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("Max Mass Agents: %d"), Constraints.MaxMassAgents);
    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("Target Frame Time: %.2f ms"), Constraints.TargetFrameTimeMs);
    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("Memory Budget: %d MB"), Constraints.MaxTotalMemoryMB);
    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("Registered Systems: %d"), SystemBudgets.Num());
    UE_LOG(LogTranspersonalArchitecture, Warning, TEXT("========================"));
}

// Base System Implementation
void UTranspersonalSystemBase::RegisterWithArchitecture()
{
    if (UEngineArchitectureSubsystem* ArchSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UEngineArchitectureSubsystem>())
    {
        ArchSubsystem->RegisterSystem(GetSystemName(), GetSystemPriority(), ResourceBudget);
    }
}

void UTranspersonalSystemBase::UnregisterFromArchitecture()
{
    // Implementation for unregistering from architecture subsystem
    // This would remove the system from tracking
}