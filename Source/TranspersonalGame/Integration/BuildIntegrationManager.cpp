#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Modules/ModuleManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bIntegrationRunning = false;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing..."));
    
    // Initialize core modules first
    RegisterCoreModules();
    InitializeDefaultModules();
    
    // Start integration process
    StartIntegration();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initialization complete"));
}

void UBuildIntegrationManager::Deinitialize()
{
    StopIntegration();
    RegisteredModules.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Deinitialized"));
    
    Super::Deinitialize();
}

void UBuildIntegrationManager::RegisterModule(const FString& ModuleName, EBuild_SystemPriority Priority, const TArray<FString>& Dependencies)
{
    FBuild_ModuleInfo ModuleInfo;
    ModuleInfo.ModuleName = ModuleName;
    ModuleInfo.Priority = Priority;
    ModuleInfo.Dependencies = Dependencies;
    ModuleInfo.Status = EBuild_ModuleStatus::NotLoaded;
    ModuleInfo.Version = TEXT("1.0.0");
    ModuleInfo.LoadTime = 0.0f;
    
    RegisteredModules.Add(ModuleName, ModuleInfo);
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Registered module '%s' with priority %d"), 
           *ModuleName, (int32)Priority);
    
    BroadcastModuleStatusChange(ModuleInfo);
}

void UBuildIntegrationManager::UnregisterModule(const FString& ModuleName)
{
    if (RegisteredModules.Contains(ModuleName))
    {
        RegisteredModules.Remove(ModuleName);
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Unregistered module '%s'"), *ModuleName);
    }
}

bool UBuildIntegrationManager::IsModuleLoaded(const FString& ModuleName) const
{
    if (const FBuild_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName))
    {
        return ModuleInfo->Status == EBuild_ModuleStatus::Loaded;
    }
    return false;
}

EBuild_ModuleStatus UBuildIntegrationManager::GetModuleStatus(const FString& ModuleName) const
{
    if (const FBuild_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName))
    {
        return ModuleInfo->Status;
    }
    return EBuild_ModuleStatus::NotLoaded;
}

FBuild_ModuleInfo UBuildIntegrationManager::GetModuleInfo(const FString& ModuleName) const
{
    if (const FBuild_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName))
    {
        return *ModuleInfo;
    }
    return FBuild_ModuleInfo();
}

void UBuildIntegrationManager::StartIntegration()
{
    if (bIntegrationRunning)
    {
        return;
    }
    
    bIntegrationRunning = true;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting integration process..."));
    
    // Set up periodic update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            IntegrationTimerHandle,
            this,
            &UBuildIntegrationManager::UpdateModuleStatus,
            1.0f,
            true
        );
    }
    
    // Trigger initial build
    TriggerBuild();
}

void UBuildIntegrationManager::StopIntegration()
{
    if (!bIntegrationRunning)
    {
        return;
    }
    
    bIntegrationRunning = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(IntegrationTimerHandle);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Integration process stopped"));
}

void UBuildIntegrationManager::TriggerBuild()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Triggering build process..."));
    
    ExecuteBuildProcess();
}

TArray<FBuild_ModuleInfo> UBuildIntegrationManager::GetAllModules() const
{
    TArray<FBuild_ModuleInfo> AllModules;
    
    for (const auto& ModulePair : RegisteredModules)
    {
        AllModules.Add(ModulePair.Value);
    }
    
    // Sort by priority
    AllModules.Sort([](const FBuild_ModuleInfo& A, const FBuild_ModuleInfo& B)
    {
        return (int32)A.Priority < (int32)B.Priority;
    });
    
    return AllModules;
}

void UBuildIntegrationManager::ReportModuleError(const FString& ModuleName, const FString& ErrorMessage)
{
    if (FBuild_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName))
    {
        ModuleInfo->Status = EBuild_ModuleStatus::Failed;
        ModuleInfo->LastError = ErrorMessage;
        
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Module '%s' error: %s"), 
               *ModuleName, *ErrorMessage);
        
        BroadcastModuleStatusChange(*ModuleInfo);
    }
}

void UBuildIntegrationManager::ClearModuleErrors(const FString& ModuleName)
{
    if (FBuild_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName))
    {
        ModuleInfo->LastError = TEXT("");
        if (ModuleInfo->Status == EBuild_ModuleStatus::Failed)
        {
            ModuleInfo->Status = EBuild_ModuleStatus::NotLoaded;
        }
        
        BroadcastModuleStatusChange(*ModuleInfo);
    }
}

void UBuildIntegrationManager::LogModuleStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Modules: %d"), RegisteredModules.Num());
    
    int32 LoadedCount = 0;
    int32 FailedCount = 0;
    
    for (const auto& ModulePair : RegisteredModules)
    {
        const FBuild_ModuleInfo& Info = ModulePair.Value;
        
        FString StatusText;
        switch (Info.Status)
        {
            case EBuild_ModuleStatus::NotLoaded: StatusText = TEXT("NOT LOADED"); break;
            case EBuild_ModuleStatus::Loading: StatusText = TEXT("LOADING"); break;
            case EBuild_ModuleStatus::Loaded: StatusText = TEXT("LOADED"); LoadedCount++; break;
            case EBuild_ModuleStatus::Failed: StatusText = TEXT("FAILED"); FailedCount++; break;
            case EBuild_ModuleStatus::Deprecated: StatusText = TEXT("DEPRECATED"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s (Priority: %d)"), 
               *Info.ModuleName, *StatusText, (int32)Info.Priority);
        
        if (!Info.LastError.IsEmpty())
        {
            UE_LOG(LogTemp, Error, TEXT("    Error: %s"), *Info.LastError);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Loaded: %d, Failed: %d"), LoadedCount, FailedCount);
    UE_LOG(LogTemp, Warning, TEXT("=== END STATUS ==="));
}

void UBuildIntegrationManager::ValidateAllDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATING DEPENDENCIES ==="));
    
    bool bAllValid = true;
    
    for (const auto& ModulePair : RegisteredModules)
    {
        const FString& ModuleName = ModulePair.Key;
        const FBuild_ModuleInfo& Info = ModulePair.Value;
        
        for (const FString& Dependency : Info.Dependencies)
        {
            if (!RegisteredModules.Contains(Dependency))
            {
                UE_LOG(LogTemp, Error, TEXT("Module '%s' depends on unregistered module '%s'"), 
                       *ModuleName, *Dependency);
                bAllValid = false;
            }
        }
    }
    
    // Check for circular dependencies
    if (HasCircularDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Circular dependencies detected!"));
        bAllValid = false;
    }
    
    if (bAllValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("All dependencies are valid"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== DEPENDENCY VALIDATION COMPLETE ==="));
}

void UBuildIntegrationManager::ForceReloadAllModules()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Force reloading all modules..."));
    
    // Reset all module statuses
    for (auto& ModulePair : RegisteredModules)
    {
        FBuild_ModuleInfo& Info = ModulePair.Value;
        Info.Status = EBuild_ModuleStatus::NotLoaded;
        Info.LastError = TEXT("");
        Info.LoadTime = 0.0f;
        
        BroadcastModuleStatusChange(Info);
    }
    
    // Trigger new build
    TriggerBuild();
}

void UBuildIntegrationManager::UpdateModuleStatus()
{
    if (!bIntegrationRunning)
    {
        return;
    }
    
    // Check actual module loading status using UE5's module manager
    FModuleManager& ModuleManager = FModuleManager::Get();
    
    for (auto& ModulePair : RegisteredModules)
    {
        FBuild_ModuleInfo& Info = ModulePair.Value;
        const FString& ModuleName = ModulePair.Key;
        
        // Skip if already processed
        if (Info.Status == EBuild_ModuleStatus::Loaded || Info.Status == EBuild_ModuleStatus::Failed)
        {
            continue;
        }
        
        // Check if module is loaded in UE5
        bool bIsLoaded = ModuleManager.IsModuleLoaded(*ModuleName);
        
        if (bIsLoaded && Info.Status != EBuild_ModuleStatus::Loaded)
        {
            Info.Status = EBuild_ModuleStatus::Loaded;
            Info.LoadTime = FPlatformTime::Seconds();
            
            UE_LOG(LogTemp, Log, TEXT("Module '%s' is now loaded"), *ModuleName);
            BroadcastModuleStatusChange(Info);
        }
        else if (!bIsLoaded && Info.Status == EBuild_ModuleStatus::Loading)
        {
            // Check if loading timed out
            float CurrentTime = FPlatformTime::Seconds();
            if (CurrentTime - Info.LoadTime > 30.0f) // 30 second timeout
            {
                Info.Status = EBuild_ModuleStatus::Failed;
                Info.LastError = TEXT("Module loading timed out");
                
                UE_LOG(LogTemp, Error, TEXT("Module '%s' loading timed out"), *ModuleName);
                BroadcastModuleStatusChange(Info);
            }
        }
    }
}

void UBuildIntegrationManager::ProcessModuleDependencies()
{
    TArray<FString> LoadOrder = ResolveDependencyOrder();
    
    for (const FString& ModuleName : LoadOrder)
    {
        LoadModuleInOrder(ModuleName);
    }
}

void UBuildIntegrationManager::LoadModuleInOrder(const FString& ModuleName)
{
    if (FBuild_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName))
    {
        if (ModuleInfo->Status != EBuild_ModuleStatus::NotLoaded)
        {
            return; // Already processed
        }
        
        // Validate dependencies first
        if (!ValidateModuleDependencies(ModuleName))
        {
            ModuleInfo->Status = EBuild_ModuleStatus::Failed;
            ModuleInfo->LastError = TEXT("Dependency validation failed");
            BroadcastModuleStatusChange(*ModuleInfo);
            return;
        }
        
        // Mark as loading
        ModuleInfo->Status = EBuild_ModuleStatus::Loading;
        ModuleInfo->LoadTime = FPlatformTime::Seconds();
        
        UE_LOG(LogTemp, Log, TEXT("Loading module '%s'..."), *ModuleName);
        BroadcastModuleStatusChange(*ModuleInfo);
        
        // Try to load the module
        FModuleManager& ModuleManager = FModuleManager::Get();
        
        try
        {
            if (ModuleManager.ModuleExists(*ModuleName))
            {
                ModuleManager.LoadModule(*ModuleName);
                UE_LOG(LogTemp, Log, TEXT("Successfully initiated loading for module '%s'"), *ModuleName);
            }
            else
            {
                ModuleInfo->Status = EBuild_ModuleStatus::Failed;
                ModuleInfo->LastError = TEXT("Module does not exist");
                BroadcastModuleStatusChange(*ModuleInfo);
            }
        }
        catch (...)
        {
            ModuleInfo->Status = EBuild_ModuleStatus::Failed;
            ModuleInfo->LastError = TEXT("Exception during module loading");
            BroadcastModuleStatusChange(*ModuleInfo);
        }
    }
}

bool UBuildIntegrationManager::ValidateModuleDependencies(const FString& ModuleName) const
{
    const FBuild_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName);
    if (!ModuleInfo)
    {
        return false;
    }
    
    // Check if all dependencies are loaded
    for (const FString& Dependency : ModuleInfo->Dependencies)
    {
        const FBuild_ModuleInfo* DepInfo = RegisteredModules.Find(Dependency);
        if (!DepInfo || DepInfo->Status != EBuild_ModuleStatus::Loaded)
        {
            UE_LOG(LogTemp, Warning, TEXT("Module '%s' dependency '%s' not loaded"), 
                   *ModuleName, *Dependency);
            return false;
        }
    }
    
    return true;
}

void UBuildIntegrationManager::GenerateBuildReport()
{
    LastBuildReport = FBuild_IntegrationReport();
    LastBuildReport.BuildTime = FDateTime::Now();
    LastBuildReport.TotalModules = RegisteredModules.Num();
    
    int32 LoadedCount = 0;
    int32 FailedCount = 0;
    
    for (const auto& ModulePair : RegisteredModules)
    {
        const FBuild_ModuleInfo& Info = ModulePair.Value;
        
        switch (Info.Status)
        {
            case EBuild_ModuleStatus::Loaded:
                LoadedCount++;
                break;
            case EBuild_ModuleStatus::Failed:
                FailedCount++;
                LastBuildReport.CriticalErrors.Add(FString::Printf(TEXT("Module '%s': %s"), 
                    *Info.ModuleName, *Info.LastError));
                break;
            default:
                LastBuildReport.Warnings.Add(FString::Printf(TEXT("Module '%s' not fully loaded"), 
                    *Info.ModuleName));
                break;
        }
    }
    
    LastBuildReport.LoadedModules = LoadedCount;
    LastBuildReport.FailedModules = FailedCount;
    LastBuildReport.bBuildSuccessful = (FailedCount == 0);
    LastBuildReport.TotalBuildTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Build Report Generated: %d/%d modules loaded successfully"), 
           LoadedCount, LastBuildReport.TotalModules);
    
    OnIntegrationComplete.Broadcast(LastBuildReport);
}

void UBuildIntegrationManager::BroadcastModuleStatusChange(const FBuild_ModuleInfo& ModuleInfo)
{
    OnModuleStatusChanged.Broadcast(ModuleInfo);
}

void UBuildIntegrationManager::RegisterCoreModules()
{
    // Register essential UE5 modules
    TArray<FString> EmptyDeps;
    
    RegisterModule(TEXT("Core"), EBuild_SystemPriority::Critical, EmptyDeps);
    RegisterModule(TEXT("CoreUObject"), EBuild_SystemPriority::Critical, {TEXT("Core")});
    RegisterModule(TEXT("Engine"), EBuild_SystemPriority::Critical, {TEXT("Core"), TEXT("CoreUObject")});
    RegisterModule(TEXT("TranspersonalGame"), EBuild_SystemPriority::Critical, {TEXT("Engine")});
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Core modules registered"));
}

void UBuildIntegrationManager::InitializeDefaultModules()
{
    // Register game-specific modules based on project structure
    TArray<FString> GameDeps = {TEXT("TranspersonalGame")};
    
    RegisterModule(TEXT("Physics"), EBuild_SystemPriority::High, GameDeps);
    RegisterModule(TEXT("AI"), EBuild_SystemPriority::High, GameDeps);
    RegisterModule(TEXT("Animation"), EBuild_SystemPriority::Medium, GameDeps);
    RegisterModule(TEXT("Audio"), EBuild_SystemPriority::Medium, GameDeps);
    RegisterModule(TEXT("VFX"), EBuild_SystemPriority::Low, GameDeps);
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Default modules registered"));
}

void UBuildIntegrationManager::ExecuteBuildProcess()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Executing build process..."));
    
    float StartTime = FPlatformTime::Seconds();
    
    // Process modules in dependency order
    ProcessModuleDependencies();
    
    // Generate build report
    GenerateBuildReport();
    
    float EndTime = FPlatformTime::Seconds();
    LastBuildReport.TotalBuildTime = EndTime - StartTime;
    
    OnBuildComplete(LastBuildReport.bBuildSuccessful);
}

void UBuildIntegrationManager::OnBuildComplete(bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Build completed successfully!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Build completed with errors"));
    }
    
    LogModuleStatus();
}

TArray<FString> UBuildIntegrationManager::ResolveDependencyOrder() const
{
    TArray<FString> LoadOrder;
    TSet<FString> Processed;
    TSet<FString> Processing;
    
    // Topological sort with dependency resolution
    for (const auto& ModulePair : RegisteredModules)
    {
        const FString& ModuleName = ModulePair.Key;
        if (!Processed.Contains(ModuleName))
        {
            ResolveDependencyOrderRecursive(ModuleName, LoadOrder, Processed, Processing);
        }
    }
    
    return LoadOrder;
}

void UBuildIntegrationManager::ResolveDependencyOrderRecursive(const FString& ModuleName, 
    TArray<FString>& LoadOrder, TSet<FString>& Processed, TSet<FString>& Processing) const
{
    if (Processing.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Error, TEXT("Circular dependency detected involving module '%s'"), *ModuleName);
        return;
    }
    
    if (Processed.Contains(ModuleName))
    {
        return;
    }
    
    Processing.Add(ModuleName);
    
    const FBuild_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName);
    if (ModuleInfo)
    {
        // Process dependencies first
        for (const FString& Dependency : ModuleInfo->Dependencies)
        {
            ResolveDependencyOrderRecursive(Dependency, LoadOrder, Processed, Processing);
        }
    }
    
    Processing.Remove(ModuleName);
    Processed.Add(ModuleName);
    LoadOrder.Add(ModuleName);
}

bool UBuildIntegrationManager::HasCircularDependencies() const
{
    TSet<FString> Visited;
    TSet<FString> RecStack;
    
    for (const auto& ModulePair : RegisteredModules)
    {
        const FString& ModuleName = ModulePair.Key;
        if (HasCircularDependenciesRecursive(ModuleName, Visited, RecStack))
        {
            return true;
        }
    }
    
    return false;
}

bool UBuildIntegrationManager::HasCircularDependenciesRecursive(const FString& ModuleName, 
    TSet<FString>& Visited, TSet<FString>& RecStack) const
{
    if (RecStack.Contains(ModuleName))
    {
        return true; // Circular dependency found
    }
    
    if (Visited.Contains(ModuleName))
    {
        return false; // Already processed
    }
    
    Visited.Add(ModuleName);
    RecStack.Add(ModuleName);
    
    const FBuild_ModuleInfo* ModuleInfo = RegisteredModules.Find(ModuleName);
    if (ModuleInfo)
    {
        for (const FString& Dependency : ModuleInfo->Dependencies)
        {
            if (HasCircularDependenciesRecursive(Dependency, Visited, RecStack))
            {
                return true;
            }
        }
    }
    
    RecStack.Remove(ModuleName);
    return false;
}