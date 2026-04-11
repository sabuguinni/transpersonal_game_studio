#include "IntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Modules/ModuleManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogIntegration);

UIntegrationManager::UIntegrationManager()
{
    NextBuildNumber = 1;
    bSystemsInitialized = false;
    LastIntegrationCheck = 0.0;
    IntegrationCheckInterval = 5.0f; // Check every 5 seconds
}

void UIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogIntegration, Log, TEXT("Integration Manager initializing..."));
    
    // Initialize build status
    CurrentBuild.BuildNumber = NextBuildNumber++;
    CurrentBuild.BuildTime = FDateTime::Now();
    CurrentBuild.bIsStable = false;
    CurrentBuild.BuildNotes = TEXT("Initial build - Integration Manager startup");
    
    // Register core modules
    RegisterCoreModules();
    
    // Set up system initialization order
    SystemInitOrder.Add(TEXT("TranspersonalGame"));
    SystemInitOrder.Add(TEXT("Core"));
    SystemInitOrder.Add(TEXT("Physics"));
    SystemInitOrder.Add(TEXT("Consciousness"));
    SystemInitOrder.Add(TEXT("Environment"));
    SystemInitOrder.Add(TEXT("Character"));
    SystemInitOrder.Add(TEXT("AI"));
    SystemInitOrder.Add(TEXT("Audio"));
    SystemInitOrder.Add(TEXT("VFX"));
    
    UE_LOG(LogIntegration, Log, TEXT("Integration Manager initialized successfully"));
}

void UIntegrationManager::Deinitialize()
{
    UE_LOG(LogIntegration, Log, TEXT("Integration Manager shutting down..."));
    
    ShutdownAllSystems();
    
    Super::Deinitialize();
}

void UIntegrationManager::RegisterCoreModules()
{
    UE_LOG(LogIntegration, Log, TEXT("Registering core modules..."));
    
    // Core UE5 modules
    TArray<FString> CoreModules = {
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("Engine"),
        TEXT("UnrealEd"),
        TEXT("TranspersonalGame")
    };
    
    for (const FString& ModuleName : CoreModules)
    {
        FModuleStatus Status;
        Status.ModuleName = ModuleName;
        Status.bIsLoaded = FModuleManager::Get().IsModuleLoaded(*ModuleName);
        Status.bIsCompiled = Status.bIsLoaded; // Assume compiled if loaded
        Status.LoadTime = 0.0f;
        
        if (Status.bIsLoaded)
        {
            UE_LOG(LogIntegration, Log, TEXT("Module %s: LOADED"), *ModuleName);
        }
        else
        {
            UE_LOG(LogIntegration, Warning, TEXT("Module %s: NOT LOADED"), *ModuleName);
            Status.LastError = TEXT("Module not loaded");
        }
        
        ModuleRegistry.Add(ModuleName, Status);
    }
    
    // Update current build status
    CurrentBuild.ModuleStatuses.Empty();
    for (auto& ModulePair : ModuleRegistry)
    {
        CurrentBuild.ModuleStatuses.Add(ModulePair.Value);
    }
}

bool UIntegrationManager::LoadGameModule(const FString& ModuleName)
{
    UE_LOG(LogIntegration, Log, TEXT("Attempting to load module: %s"), *ModuleName);
    
    double StartTime = FPlatformTime::Seconds();
    
    try
    {
        if (FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            UE_LOG(LogIntegration, Log, TEXT("Module %s already loaded"), *ModuleName);
            UpdateModuleStatus(ModuleName, true);
            return true;
        }
        
        FModuleManager::Get().LoadModule(*ModuleName);
        
        bool bLoaded = FModuleManager::Get().IsModuleLoaded(*ModuleName);
        float LoadTime = FPlatformTime::Seconds() - StartTime;
        
        if (bLoaded)
        {
            UE_LOG(LogIntegration, Log, TEXT("Module %s loaded successfully in %.3f seconds"), *ModuleName, LoadTime);
            UpdateModuleStatus(ModuleName, true);
            
            // Update module status with load time
            if (FModuleStatus* Status = ModuleRegistry.Find(ModuleName))
            {
                Status->LoadTime = LoadTime;
            }
        }
        else
        {
            UE_LOG(LogIntegration, Error, TEXT("Failed to load module %s"), *ModuleName);
            UpdateModuleStatus(ModuleName, false, TEXT("Load failed"));
        }
        
        return bLoaded;
    }
    catch (...)
    {
        UE_LOG(LogIntegration, Error, TEXT("Exception while loading module %s"), *ModuleName);
        UpdateModuleStatus(ModuleName, false, TEXT("Exception during load"));
        return false;
    }
}

bool UIntegrationManager::UnloadGameModule(const FString& ModuleName)
{
    UE_LOG(LogIntegration, Log, TEXT("Attempting to unload module: %s"), *ModuleName);
    
    try
    {
        if (!FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            UE_LOG(LogIntegration, Log, TEXT("Module %s not loaded"), *ModuleName);
            return true;
        }
        
        FModuleManager::Get().UnloadModule(*ModuleName);
        
        bool bUnloaded = !FModuleManager::Get().IsModuleLoaded(*ModuleName);
        
        if (bUnloaded)
        {
            UE_LOG(LogIntegration, Log, TEXT("Module %s unloaded successfully"), *ModuleName);
            UpdateModuleStatus(ModuleName, false);
        }
        else
        {
            UE_LOG(LogIntegration, Error, TEXT("Failed to unload module %s"), *ModuleName);
            UpdateModuleStatus(ModuleName, true, TEXT("Unload failed"));
        }
        
        return bUnloaded;
    }
    catch (...)
    {
        UE_LOG(LogIntegration, Error, TEXT("Exception while unloading module %s"), *ModuleName);
        return false;
    }
}

FModuleStatus UIntegrationManager::GetModuleStatus(const FString& ModuleName)
{
    if (FModuleStatus* Status = ModuleRegistry.Find(ModuleName))
    {
        // Update real-time status
        Status->bIsLoaded = FModuleManager::Get().IsModuleLoaded(*ModuleName);
        return *Status;
    }
    
    // Create new status for unknown module
    FModuleStatus NewStatus;
    NewStatus.ModuleName = ModuleName;
    NewStatus.bIsLoaded = FModuleManager::Get().IsModuleLoaded(*ModuleName);
    NewStatus.bIsCompiled = NewStatus.bIsLoaded;
    NewStatus.LastError = TEXT("Module not registered");
    
    return NewStatus;
}

TArray<FModuleStatus> UIntegrationManager::GetAllModuleStatuses()
{
    TArray<FModuleStatus> Statuses;
    
    for (auto& ModulePair : ModuleRegistry)
    {
        // Update real-time status
        ModulePair.Value.bIsLoaded = FModuleManager::Get().IsModuleLoaded(*ModulePair.Key);
        Statuses.Add(ModulePair.Value);
    }
    
    return Statuses;
}

FBuildStatus UIntegrationManager::GetCurrentBuildStatus()
{
    // Update module statuses in current build
    CurrentBuild.ModuleStatuses = GetAllModuleStatuses();
    
    // Check if build is stable (all core modules loaded)
    bool bAllCoreLoaded = true;
    for (const FModuleStatus& Status : CurrentBuild.ModuleStatuses)
    {
        if (Status.ModuleName == TEXT("TranspersonalGame") || 
            Status.ModuleName == TEXT("Core") || 
            Status.ModuleName == TEXT("Engine"))
        {
            if (!Status.bIsLoaded)
            {
                bAllCoreLoaded = false;
                break;
            }
        }
    }
    
    CurrentBuild.bIsStable = bAllCoreLoaded;
    
    return CurrentBuild;
}

bool UIntegrationManager::ValidateBuildIntegrity()
{
    UE_LOG(LogIntegration, Log, TEXT("Validating build integrity..."));
    
    bool bIntegrityValid = true;
    
    // Check all registered modules
    for (auto& ModulePair : ModuleRegistry)
    {
        const FString& ModuleName = ModulePair.Key;
        bool bCurrentlyLoaded = FModuleManager::Get().IsModuleLoaded(*ModuleName);
        
        if (!bCurrentlyLoaded && (ModuleName == TEXT("TranspersonalGame") || ModuleName == TEXT("Core")))
        {
            UE_LOG(LogIntegration, Error, TEXT("Critical module %s is not loaded"), *ModuleName);
            bIntegrityValid = false;
        }
    }
    
    // Validate dependencies
    ValidateModuleDependencies();
    
    UE_LOG(LogIntegration, Log, TEXT("Build integrity validation: %s"), 
           bIntegrityValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bIntegrityValid;
}

void UIntegrationManager::CreateBuildSnapshot(const FString& BuildNotes)
{
    UE_LOG(LogIntegration, Log, TEXT("Creating build snapshot..."));
    
    FBuildStatus Snapshot = GetCurrentBuildStatus();
    Snapshot.BuildNotes = BuildNotes;
    
    BuildHistory.Add(Snapshot);
    
    // Keep only last 10 builds
    if (BuildHistory.Num() > 10)
    {
        BuildHistory.RemoveAt(0);
    }
    
    UE_LOG(LogIntegration, Log, TEXT("Build snapshot created: Build #%d"), Snapshot.BuildNumber);
}

bool UIntegrationManager::InitializeAllSystems()
{
    UE_LOG(LogIntegration, Log, TEXT("Initializing all systems in order..."));
    
    bool bAllInitialized = true;
    
    for (const FString& SystemName : SystemInitOrder)
    {
        UE_LOG(LogIntegration, Log, TEXT("Initializing system: %s"), *SystemName);
        
        if (!LoadGameModule(SystemName))
        {
            UE_LOG(LogIntegration, Error, TEXT("Failed to initialize system: %s"), *SystemName);
            bAllInitialized = false;
        }
    }
    
    bSystemsInitialized = bAllInitialized;
    
    UE_LOG(LogIntegration, Log, TEXT("System initialization: %s"), 
           bAllInitialized ? TEXT("SUCCESS") : TEXT("FAILED"));
    
    return bAllInitialized;
}

bool UIntegrationManager::ShutdownAllSystems()
{
    UE_LOG(LogIntegration, Log, TEXT("Shutting down all systems..."));
    
    // Shutdown in reverse order
    for (int32 i = SystemInitOrder.Num() - 1; i >= 0; i--)
    {
        const FString& SystemName = SystemInitOrder[i];
        UE_LOG(LogIntegration, Log, TEXT("Shutting down system: %s"), *SystemName);
        // Note: We don't actually unload core modules as it can cause crashes
    }
    
    bSystemsInitialized = false;
    
    UE_LOG(LogIntegration, Log, TEXT("System shutdown complete"));
    
    return true;
}

void UIntegrationManager::RefreshSystemDependencies()
{
    UE_LOG(LogIntegration, Log, TEXT("Refreshing system dependencies..."));
    
    ValidateModuleDependencies();
    RegisterCoreModules();
    
    UE_LOG(LogIntegration, Log, TEXT("System dependencies refreshed"));
}

void UIntegrationManager::RunIntegrationDiagnostics()
{
    UE_LOG(LogIntegration, Log, TEXT("=== INTEGRATION DIAGNOSTICS ==="));
    
    // Module status
    UE_LOG(LogIntegration, Log, TEXT("Module Status:"));
    for (auto& ModulePair : ModuleRegistry)
    {
        const FModuleStatus& Status = ModulePair.Value;
        UE_LOG(LogIntegration, Log, TEXT("  %s: %s (Load Time: %.3fs)"), 
               *Status.ModuleName,
               Status.bIsLoaded ? TEXT("LOADED") : TEXT("NOT LOADED"),
               Status.LoadTime);
        
        if (!Status.LastError.IsEmpty())
        {
            UE_LOG(LogIntegration, Log, TEXT("    Error: %s"), *Status.LastError);
        }
    }
    
    // Build status
    FBuildStatus BuildStatus = GetCurrentBuildStatus();
    UE_LOG(LogIntegration, Log, TEXT("Build #%d: %s"), 
           BuildStatus.BuildNumber,
           BuildStatus.bIsStable ? TEXT("STABLE") : TEXT("UNSTABLE"));
    
    // System status
    UE_LOG(LogIntegration, Log, TEXT("Systems Initialized: %s"), 
           bSystemsInitialized ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogIntegration, Log, TEXT("=== DIAGNOSTICS COMPLETE ==="));
}

FString UIntegrationManager::GenerateSystemReport()
{
    FString Report = TEXT("=== TRANSPERSONAL GAME STUDIO - SYSTEM REPORT ===\n\n");
    
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Build: #%d (%s)\n\n"), 
                             CurrentBuild.BuildNumber,
                             CurrentBuild.bIsStable ? TEXT("STABLE") : TEXT("UNSTABLE"));
    
    Report += TEXT("MODULE STATUS:\n");
    for (auto& ModulePair : ModuleRegistry)
    {
        const FModuleStatus& Status = ModulePair.Value;
        Report += FString::Printf(TEXT("  %-20s: %s\n"), 
                                 *Status.ModuleName,
                                 Status.bIsLoaded ? TEXT("LOADED") : TEXT("NOT LOADED"));
    }
    
    Report += TEXT("\nSYSTEM INITIALIZATION ORDER:\n");
    for (int32 i = 0; i < SystemInitOrder.Num(); i++)
    {
        Report += FString::Printf(TEXT("  %d. %s\n"), i + 1, *SystemInitOrder[i]);
    }
    
    Report += FString::Printf(TEXT("\nSystems Initialized: %s\n"), 
                             bSystemsInitialized ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("\n=== END REPORT ===");
    
    return Report;
}

void UIntegrationManager::LogSystemStatus()
{
    RunIntegrationDiagnostics();
}

void UIntegrationManager::ValidateModuleDependencies()
{
    UE_LOG(LogIntegration, Log, TEXT("Validating module dependencies..."));
    
    // Check that core dependencies are met
    TMap<FString, TArray<FString>> Dependencies;
    Dependencies.Add(TEXT("TranspersonalGame"), {TEXT("Core"), TEXT("CoreUObject"), TEXT("Engine")});
    Dependencies.Add(TEXT("Physics"), {TEXT("TranspersonalGame"), TEXT("Core")});
    Dependencies.Add(TEXT("Consciousness"), {TEXT("TranspersonalGame"), TEXT("Core")});
    
    for (auto& DepPair : Dependencies)
    {
        const FString& ModuleName = DepPair.Key;
        const TArray<FString>& RequiredModules = DepPair.Value;
        
        if (FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            for (const FString& RequiredModule : RequiredModules)
            {
                if (!FModuleManager::Get().IsModuleLoaded(*RequiredModule))
                {
                    UE_LOG(LogIntegration, Warning, 
                           TEXT("Module %s is loaded but dependency %s is not"), 
                           *ModuleName, *RequiredModule);
                }
            }
        }
    }
}

bool UIntegrationManager::CheckModuleCompatibility(const FString& ModuleName)
{
    // Basic compatibility check - can be extended
    return FModuleManager::Get().ModuleExists(*ModuleName);
}

void UIntegrationManager::UpdateModuleStatus(const FString& ModuleName, bool bLoaded, const FString& Error)
{
    if (FModuleStatus* Status = ModuleRegistry.Find(ModuleName))
    {
        Status->bIsLoaded = bLoaded;
        Status->bIsCompiled = bLoaded;
        Status->LastError = Error;
    }
    else
    {
        FModuleStatus NewStatus;
        NewStatus.ModuleName = ModuleName;
        NewStatus.bIsLoaded = bLoaded;
        NewStatus.bIsCompiled = bLoaded;
        NewStatus.LastError = Error;
        ModuleRegistry.Add(ModuleName, NewStatus);
    }
}