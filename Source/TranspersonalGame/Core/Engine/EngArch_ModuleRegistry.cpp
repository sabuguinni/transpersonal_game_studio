#include "EngArch_ModuleRegistry.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UEngArch_ModuleRegistry::UEngArch_ModuleRegistry()
{
    MaxFrameTime = 16.67f; // 60 FPS target
    MaxDrawCalls = 2000;
    MaxMemoryUsageMB = 4096.0f;
    LastMetricsUpdate = 0.0f;
    bInitializationOrderDirty = true;
}

void UEngArch_ModuleRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: Initializing module registry system"));
    
    InitializeCoreModules();
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: Registry initialized with %d core modules"), RegisteredModules.Num());
}

void UEngArch_ModuleRegistry::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: Shutting down module registry"));
    
    RegisteredModules.Empty();
    ModuleInitOrder.Empty();
    
    Super::Deinitialize();
}

bool UEngArch_ModuleRegistry::RegisterModule(const FString& ModuleName, const FString& AgentResponsible, const TArray<FString>& Dependencies)
{
    if (ModuleName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_ModuleRegistry: Cannot register module with empty name"));
        return false;
    }

    if (RegisteredModules.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: Module %s already registered, updating info"), *ModuleName);
    }

    // Check for circular dependencies
    if (CheckCircularDependencies(ModuleName, Dependencies))
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_ModuleRegistry: Circular dependency detected for module %s"), *ModuleName);
        return false;
    }

    FEng_ModuleInfo ModuleInfo;
    ModuleInfo.ModuleName = ModuleName;
    ModuleInfo.AgentResponsible = AgentResponsible;
    ModuleInfo.Dependencies = Dependencies;
    ModuleInfo.DependencyCount = Dependencies.Num();
    ModuleInfo.Status = EEng_ModuleStatus::Registered;
    ModuleInfo.InitializationTime = FPlatformTime::Seconds();

    RegisteredModules.Add(ModuleName, ModuleInfo);
    bInitializationOrderDirty = true;

    UE_LOG(LogTemp, Log, TEXT("EngArch_ModuleRegistry: Registered module %s (Agent: %s, Dependencies: %d)"), 
           *ModuleName, *AgentResponsible, Dependencies.Num());

    return true;
}

bool UEngArch_ModuleRegistry::UnregisterModule(const FString& ModuleName)
{
    if (!RegisteredModules.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: Module %s not found for unregistration"), *ModuleName);
        return false;
    }

    RegisteredModules.Remove(ModuleName);
    bInitializationOrderDirty = true;

    UE_LOG(LogTemp, Log, TEXT("EngArch_ModuleRegistry: Unregistered module %s"), *ModuleName);
    return true;
}

void UEngArch_ModuleRegistry::SetModuleStatus(const FString& ModuleName, EEng_ModuleStatus NewStatus)
{
    if (FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName))
    {
        EEng_ModuleStatus OldStatus = ModuleInfo->Status;
        ModuleInfo->Status = NewStatus;
        
        UE_LOG(LogTemp, Log, TEXT("EngArch_ModuleRegistry: Module %s status changed from %d to %d"), 
               *ModuleName, (int32)OldStatus, (int32)NewStatus);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: Cannot set status for unknown module %s"), *ModuleName);
    }
}

FEng_ModuleInfo UEngArch_ModuleRegistry::GetModuleInfo(const FString& ModuleName) const
{
    if (const FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName))
    {
        return *ModuleInfo;
    }
    
    return FEng_ModuleInfo(); // Return default empty info
}

TArray<FString> UEngArch_ModuleRegistry::GetAllModules() const
{
    TArray<FString> ModuleNames;
    RegisteredModules.GetKeys(ModuleNames);
    return ModuleNames;
}

TArray<FString> UEngArch_ModuleRegistry::GetModulesByStatus(EEng_ModuleStatus Status) const
{
    TArray<FString> FilteredModules;
    
    for (const auto& ModulePair : RegisteredModules)
    {
        if (ModulePair.Value.Status == Status)
        {
            FilteredModules.Add(ModulePair.Key);
        }
    }
    
    return FilteredModules;
}

TArray<FString> UEngArch_ModuleRegistry::GetModulesByAgent(const FString& AgentName) const
{
    TArray<FString> FilteredModules;
    
    for (const auto& ModulePair : RegisteredModules)
    {
        if (ModulePair.Value.AgentResponsible == AgentName)
        {
            FilteredModules.Add(ModulePair.Key);
        }
    }
    
    return FilteredModules;
}

bool UEngArch_ModuleRegistry::ValidateDependencies(const FString& ModuleName) const
{
    const FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName);
    if (!ModuleInfo)
    {
        return false;
    }

    for (const FString& Dependency : ModuleInfo->Dependencies)
    {
        const FEng_ModuleInfo* DepInfo = RegisteredModules.Find(Dependency);
        if (!DepInfo || DepInfo->Status != EEng_ModuleStatus::Active)
        {
            return false;
        }
    }

    return true;
}

TArray<FString> UEngArch_ModuleRegistry::GetUnmetDependencies(const FString& ModuleName) const
{
    TArray<FString> UnmetDeps;
    
    const FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName);
    if (!ModuleInfo)
    {
        return UnmetDeps;
    }

    for (const FString& Dependency : ModuleInfo->Dependencies)
    {
        const FEng_ModuleInfo* DepInfo = RegisteredModules.Find(Dependency);
        if (!DepInfo || DepInfo->Status != EEng_ModuleStatus::Active)
        {
            UnmetDeps.Add(Dependency);
        }
    }

    return UnmetDeps;
}

TArray<FString> UEngArch_ModuleRegistry::GetInitializationOrder() const
{
    if (bInitializationOrderDirty)
    {
        const_cast<UEngArch_ModuleRegistry*>(this)->RecalculateInitializationOrder();
    }
    
    return ModuleInitOrder;
}

void UEngArch_ModuleRegistry::UpdatePerformanceMetrics()
{
    float CurrentTime = FPlatformTime::Seconds();
    
    // Update metrics every second
    if (CurrentTime - LastMetricsUpdate < 1.0f)
    {
        return;
    }

    LastMetricsUpdate = CurrentTime;

    // Get frame time from stats
    CurrentMetrics.FrameTime = FPlatformTime::ToMilliseconds(FStats::GetStatValueFloat(STAT_FrameTime));
    CurrentMetrics.GameThreadTime = FPlatformTime::ToMilliseconds(FStats::GetStatValueFloat(STAT_GameThreadTime));
    CurrentMetrics.RenderThreadTime = FPlatformTime::ToMilliseconds(FStats::GetStatValueFloat(STAT_RenderThreadTime));

    // Count active actors in world
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActiveActors = World->GetActorCount();
    }

    // Memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);

    // Draw calls would need RHI access, simplified for now
    CurrentMetrics.DrawCalls = 0;
}

FEng_PerformanceMetrics UEngArch_ModuleRegistry::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

bool UEngArch_ModuleRegistry::IsPerformanceWithinLimits() const
{
    return CurrentMetrics.FrameTime <= MaxFrameTime &&
           CurrentMetrics.DrawCalls <= MaxDrawCalls &&
           CurrentMetrics.MemoryUsageMB <= MaxMemoryUsageMB;
}

void UEngArch_ModuleRegistry::ValidateAllModules()
{
    UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: Validating all %d registered modules"), RegisteredModules.Num());
    
    int32 ValidModules = 0;
    int32 InvalidModules = 0;

    for (const auto& ModulePair : RegisteredModules)
    {
        const FString& ModuleName = ModulePair.Key;
        const FEng_ModuleInfo& ModuleInfo = ModulePair.Value;

        bool bIsValid = ValidateDependencies(ModuleName);
        if (bIsValid)
        {
            ValidModules++;
            UE_LOG(LogTemp, Log, TEXT("  ✓ %s (Agent: %s)"), *ModuleName, *ModuleInfo.AgentResponsible);
        }
        else
        {
            InvalidModules++;
            TArray<FString> UnmetDeps = GetUnmetDependencies(ModuleName);
            FString DepList = FString::Join(UnmetDeps, TEXT(", "));
            UE_LOG(LogTemp, Error, TEXT("  ✗ %s - Unmet dependencies: %s"), *ModuleName, *DepList);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: Validation complete - %d valid, %d invalid modules"), 
           ValidModules, InvalidModules);
}

void UEngArch_ModuleRegistry::PrintModuleStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MODULE REGISTRY STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Modules: %d"), RegisteredModules.Num());

    for (int32 StatusInt = 0; StatusInt <= (int32)EEng_ModuleStatus::Error; StatusInt++)
    {
        EEng_ModuleStatus Status = (EEng_ModuleStatus)StatusInt;
        TArray<FString> ModulesWithStatus = GetModulesByStatus(Status);
        
        if (ModulesWithStatus.Num() > 0)
        {
            FString StatusName;
            switch (Status)
            {
                case EEng_ModuleStatus::Uninitialized: StatusName = TEXT("Uninitialized"); break;
                case EEng_ModuleStatus::Registered: StatusName = TEXT("Registered"); break;
                case EEng_ModuleStatus::Initializing: StatusName = TEXT("Initializing"); break;
                case EEng_ModuleStatus::Active: StatusName = TEXT("Active"); break;
                case EEng_ModuleStatus::Suspended: StatusName = TEXT("Suspended"); break;
                case EEng_ModuleStatus::Error: StatusName = TEXT("Error"); break;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("%s (%d): %s"), 
                   *StatusName, ModulesWithStatus.Num(), *FString::Join(ModulesWithStatus, TEXT(", ")));
        }
    }

    UpdatePerformanceMetrics();
    UE_LOG(LogTemp, Warning, TEXT("Performance: %.2fms frame, %d actors, %.1fMB memory"), 
           CurrentMetrics.FrameTime, CurrentMetrics.ActiveActors, CurrentMetrics.MemoryUsageMB);
}

void UEngArch_ModuleRegistry::ForceReloadModule(const FString& ModuleName)
{
    if (RegisteredModules.Contains(ModuleName))
    {
        SetModuleStatus(ModuleName, EEng_ModuleStatus::Initializing);
        UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: Force reloading module %s"), *ModuleName);
        
        // In a real implementation, this would trigger module reinitialization
        // For now, just mark as active after a brief delay
        SetModuleStatus(ModuleName, EEng_ModuleStatus::Active);
    }
}

void UEngArch_ModuleRegistry::InitializeCoreModules()
{
    // Register essential core modules that should always be present
    TArray<FString> EmptyDeps;
    
    RegisterModule(TEXT("CoreEngine"), TEXT("Engine"), EmptyDeps);
    SetModuleStatus(TEXT("CoreEngine"), EEng_ModuleStatus::Active);
    
    RegisterModule(TEXT("Physics"), TEXT("Agent03_CoreSystems"), EmptyDeps);
    RegisterModule(TEXT("WorldGeneration"), TEXT("Agent05_WorldGenerator"), TArray<FString>{TEXT("CoreEngine")});
    RegisterModule(TEXT("CharacterSystem"), TEXT("Agent09_CharacterArtist"), TArray<FString>{TEXT("Physics")});
    RegisterModule(TEXT("DinosaurAI"), TEXT("Agent12_CombatAI"), TArray<FString>{TEXT("Physics"), TEXT("CharacterSystem")});
    RegisterModule(TEXT("AudioSystem"), TEXT("Agent16_Audio"), TArray<FString>{TEXT("CoreEngine")});
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_ModuleRegistry: Initialized %d core modules"), RegisteredModules.Num());
}

bool UEngArch_ModuleRegistry::CheckCircularDependencies(const FString& ModuleName, const TArray<FString>& Dependencies) const
{
    // Simple circular dependency check - in production this would be more sophisticated
    for (const FString& Dependency : Dependencies)
    {
        if (Dependency == ModuleName)
        {
            return true; // Self-dependency
        }
        
        // Check if dependency depends on this module
        const FEng_ModuleInfo* DepInfo = RegisteredModules.Find(Dependency);
        if (DepInfo && DepInfo->Dependencies.Contains(ModuleName))
        {
            return true; // Circular dependency
        }
    }
    
    return false;
}

void UEngArch_ModuleRegistry::RecalculateInitializationOrder()
{
    ModuleInitOrder.Empty();
    TSet<FString> ProcessedModules;
    TSet<FString> ProcessingModules;

    // Topological sort to determine initialization order
    TFunction<bool(const FString&)> VisitModule = [&](const FString& ModuleName) -> bool
    {
        if (ProcessedModules.Contains(ModuleName))
        {
            return true;
        }
        
        if (ProcessingModules.Contains(ModuleName))
        {
            // Circular dependency detected
            return false;
        }
        
        ProcessingModules.Add(ModuleName);
        
        const FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName);
        if (ModuleInfo)
        {
            for (const FString& Dependency : ModuleInfo->Dependencies)
            {
                if (!VisitModule(Dependency))
                {
                    return false;
                }
            }
        }
        
        ProcessingModules.Remove(ModuleName);
        ProcessedModules.Add(ModuleName);
        ModuleInitOrder.Add(ModuleName);
        
        return true;
    };

    for (const auto& ModulePair : RegisteredModules)
    {
        VisitModule(ModulePair.Key);
    }

    bInitializationOrderDirty = false;
    
    UE_LOG(LogTemp, Log, TEXT("EngArch_ModuleRegistry: Recalculated initialization order: %s"), 
           *FString::Join(ModuleInitOrder, TEXT(" -> ")));
}