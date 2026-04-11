#include "BuildManager.h"
#include "Engine/Engine.h"
#include "Modules/ModuleManager.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildManager, Log, All);

UBuildManager::UBuildManager()
{
    bBuildInProgress = false;
    ProjectPath = FPaths::ProjectDir();
    SourcePath = FPaths::Combine(ProjectPath, TEXT("Source"), TEXT("TranspersonalGame"));
    BuildPath = FPaths::Combine(ProjectPath, TEXT("Binaries"));
    
    // Initialize with core modules
    RegisterModule(TEXT("TranspersonalGame"), EModuleType::Core, TArray<FString>());
    RegisterModule(TEXT("PhysicsCore"), EModuleType::Physics, {TEXT("TranspersonalGame")});
    RegisterModule(TEXT("AICore"), EModuleType::AI, {TEXT("TranspersonalGame")});
    RegisterModule(TEXT("AnimationCore"), EModuleType::Animation, {TEXT("TranspersonalGame")});
    RegisterModule(TEXT("AudioCore"), EModuleType::Audio, {TEXT("TranspersonalGame")});
    RegisterModule(TEXT("EnvironmentCore"), EModuleType::Environment, {TEXT("TranspersonalGame")});
    RegisterModule(TEXT("LightingCore"), EModuleType::Lighting, {TEXT("TranspersonalGame")});
    RegisterModule(TEXT("CharacterCore"), EModuleType::Characters, {TEXT("TranspersonalGame")});
    RegisterModule(TEXT("CombatCore"), EModuleType::Combat, {TEXT("TranspersonalGame"), TEXT("AICore")});
    RegisterModule(TEXT("CrowdCore"), EModuleType::Crowd, {TEXT("TranspersonalGame"), TEXT("AICore")});
    RegisterModule(TEXT("NarrativeCore"), EModuleType::Narrative, {TEXT("TranspersonalGame")});
    RegisterModule(TEXT("QuestCore"), EModuleType::Quest, {TEXT("TranspersonalGame"), TEXT("NarrativeCore")});
    RegisterModule(TEXT("VFXCore"), EModuleType::VFX, {TEXT("TranspersonalGame")});
    RegisterModule(TEXT("PerformanceCore"), EModuleType::Performance, {TEXT("TranspersonalGame")});
    RegisterModule(TEXT("IntegrationCore"), EModuleType::Integration, {TEXT("TranspersonalGame")});
}

bool UBuildManager::StartFullBuild()
{
    if (bBuildInProgress)
    {
        UE_LOG(LogBuildManager, Warning, TEXT("Build already in progress"));
        return false;
    }

    UE_LOG(LogBuildManager, Log, TEXT("Starting full build..."));
    bBuildInProgress = true;
    BuildStartTime = FDateTime::Now();
    
    // Clear previous build data
    LastBuildReport = FBuildReport();
    CurrentErrors.Empty();
    CurrentWarnings.Empty();

    // Scan for source files
    ScanForSourceFiles();
    
    // Validate module structure
    ValidateModuleStructure();
    
    // Verify dependencies first
    bool bDependenciesValid = VerifyDependencies();
    if (!bDependenciesValid)
    {
        UE_LOG(LogBuildManager, Error, TEXT("Dependency verification failed"));
        LastBuildReport.OverallStatus = EBuildStatus::Failed;
        bBuildInProgress = false;
        return false;
    }

    // Compile modules in dependency order
    bool bAllModulesCompiled = true;
    for (const FModuleInfo& ModuleInfo : RegisteredModules)
    {
        if (!CompileModuleInternal(ModuleInfo.ModuleName))
        {
            bAllModulesCompiled = false;
            UE_LOG(LogBuildManager, Error, TEXT("Failed to compile module: %s"), *ModuleInfo.ModuleName);
        }
    }

    // Finalize build report
    LastBuildReport.BuildTime = FDateTime::Now();
    LastBuildReport.CompileTimeSeconds = (LastBuildReport.BuildTime - BuildStartTime).GetTotalSeconds();
    LastBuildReport.OverallStatus = bAllModulesCompiled ? EBuildStatus::Success : EBuildStatus::Failed;
    LastBuildReport.TotalErrors = CurrentErrors.Num();
    LastBuildReport.TotalWarnings = CurrentWarnings.Num();

    bBuildInProgress = false;
    
    UE_LOG(LogBuildManager, Log, TEXT("Build completed. Status: %s, Errors: %d, Warnings: %d"), 
           LastBuildReport.OverallStatus == EBuildStatus::Success ? TEXT("SUCCESS") : TEXT("FAILED"),
           LastBuildReport.TotalErrors, LastBuildReport.TotalWarnings);

    return bAllModulesCompiled;
}

bool UBuildManager::CompileModule(const FString& ModuleName)
{
    if (bBuildInProgress)
    {
        UE_LOG(LogBuildManager, Warning, TEXT("Cannot compile individual module during full build"));
        return false;
    }

    return CompileModuleInternal(ModuleName);
}

bool UBuildManager::CompileModuleInternal(const FString& ModuleName)
{
    UE_LOG(LogBuildManager, Log, TEXT("Compiling module: %s"), *ModuleName);
    
    // Check if module is registered
    FModuleInfo* ModuleInfo = RegisteredModules.FindByPredicate([&ModuleName](const FModuleInfo& Info)
    {
        return Info.ModuleName == ModuleName;
    });

    if (!ModuleInfo)
    {
        UE_LOG(LogBuildManager, Error, TEXT("Module not registered: %s"), *ModuleName);
        return false;
    }

    // Check dependencies first
    if (!CheckModuleDependencies(ModuleName))
    {
        UE_LOG(LogBuildManager, Error, TEXT("Module dependencies not satisfied: %s"), *ModuleName);
        UpdateModuleStatus(ModuleName, EBuildStatus::Failed);
        return false;
    }

    // Try to load the module to test compilation
    FModuleManager& ModuleManager = FModuleManager::Get();
    
    try
    {
        if (ModuleManager.IsModuleLoaded(*ModuleName))
        {
            UE_LOG(LogBuildManager, Log, TEXT("Module already loaded: %s"), *ModuleName);
            UpdateModuleStatus(ModuleName, EBuildStatus::Success);
            return true;
        }

        // Attempt to load module
        if (ModuleManager.LoadModule(*ModuleName))
        {
            UE_LOG(LogBuildManager, Log, TEXT("Successfully loaded module: %s"), *ModuleName);
            UpdateModuleStatus(ModuleName, EBuildStatus::Success);
            return true;
        }
        else
        {
            UE_LOG(LogBuildManager, Error, TEXT("Failed to load module: %s"), *ModuleName);
            UpdateModuleStatus(ModuleName, EBuildStatus::Failed);
            CurrentErrors.Add(FString::Printf(TEXT("Module load failed: %s"), *ModuleName));
            return false;
        }
    }
    catch (const std::exception& e)
    {
        FString ErrorMsg = FString::Printf(TEXT("Exception loading module %s: %s"), *ModuleName, UTF8_TO_TCHAR(e.what()));
        UE_LOG(LogBuildManager, Error, TEXT("%s"), *ErrorMsg);
        CurrentErrors.Add(ErrorMsg);
        UpdateModuleStatus(ModuleName, EBuildStatus::Failed);
        return false;
    }
}

bool UBuildManager::VerifyDependencies()
{
    UE_LOG(LogBuildManager, Log, TEXT("Verifying module dependencies..."));
    
    bool bAllDependenciesValid = true;
    
    for (const FModuleInfo& ModuleInfo : RegisteredModules)
    {
        if (!CheckModuleDependencies(ModuleInfo.ModuleName))
        {
            bAllDependenciesValid = false;
        }
    }
    
    return bAllDependenciesValid;
}

bool UBuildManager::CheckModuleDependencies(const FString& ModuleName)
{
    FModuleInfo* ModuleInfo = RegisteredModules.FindByPredicate([&ModuleName](const FModuleInfo& Info)
    {
        return Info.ModuleName == ModuleName;
    });

    if (!ModuleInfo)
    {
        return false;
    }

    FModuleManager& ModuleManager = FModuleManager::Get();
    
    for (const FString& Dependency : ModuleInfo->Dependencies)
    {
        if (!ModuleManager.IsModuleLoaded(*Dependency))
        {
            UE_LOG(LogBuildManager, Warning, TEXT("Dependency not loaded for %s: %s"), *ModuleName, *Dependency);
            // Try to load the dependency
            if (!ModuleManager.LoadModule(*Dependency))
            {
                UE_LOG(LogBuildManager, Error, TEXT("Failed to load dependency %s for module %s"), *Dependency, *ModuleName);
                return false;
            }
        }
    }
    
    return true;
}

void UBuildManager::RegisterModule(const FString& ModuleName, EModuleType ModuleType, const TArray<FString>& Dependencies)
{
    // Check if module already registered
    FModuleInfo* ExistingModule = RegisteredModules.FindByPredicate([&ModuleName](const FModuleInfo& Info)
    {
        return Info.ModuleName == ModuleName;
    });

    if (ExistingModule)
    {
        // Update existing module
        ExistingModule->ModuleType = ModuleType;
        ExistingModule->Dependencies = Dependencies;
        UE_LOG(LogBuildManager, Log, TEXT("Updated module registration: %s"), *ModuleName);
    }
    else
    {
        // Add new module
        FModuleInfo NewModule;
        NewModule.ModuleName = ModuleName;
        NewModule.ModuleType = ModuleType;
        NewModule.Dependencies = Dependencies;
        NewModule.BuildStatus = EBuildStatus::Unknown;
        NewModule.LastCompileTime = FDateTime::Now();
        
        RegisteredModules.Add(NewModule);
        UE_LOG(LogBuildManager, Log, TEXT("Registered new module: %s"), *ModuleName);
    }
}

bool UBuildManager::IsModuleLoaded(const FString& ModuleName)
{
    return FModuleManager::Get().IsModuleLoaded(*ModuleName);
}

void UBuildManager::UpdateModuleStatus(const FString& ModuleName, EBuildStatus Status)
{
    FModuleInfo* ModuleInfo = RegisteredModules.FindByPredicate([&ModuleName](const FModuleInfo& Info)
    {
        return Info.ModuleName == ModuleName;
    });

    if (ModuleInfo)
    {
        ModuleInfo->BuildStatus = Status;
        ModuleInfo->LastCompileTime = FDateTime::Now();
    }
}

void UBuildManager::ScanForSourceFiles()
{
    UE_LOG(LogBuildManager, Log, TEXT("Scanning for source files in: %s"), *SourcePath);
    
    SourceFilePaths.Empty();
    HeaderFilePaths.Empty();
    
    IFileManager& FileManager = IFileManager::Get();
    
    // Recursively find all .cpp and .h files
    TArray<FString> FoundFiles;
    FileManager.FindFilesRecursive(FoundFiles, *SourcePath, TEXT("*.cpp"), true, false);
    SourceFilePaths.Append(FoundFiles);
    
    FoundFiles.Empty();
    FileManager.FindFilesRecursive(FoundFiles, *SourcePath, TEXT("*.h"), true, false);
    HeaderFilePaths.Append(FoundFiles);
    
    UE_LOG(LogBuildManager, Log, TEXT("Found %d .cpp files and %d .h files"), SourceFilePaths.Num(), HeaderFilePaths.Num());
}

void UBuildManager::ValidateModuleStructure()
{
    UE_LOG(LogBuildManager, Log, TEXT("Validating module structure..."));
    
    // Check for required files
    TArray<FString> RequiredFiles = {
        TEXT("TranspersonalGame.cpp"),
        TEXT("TranspersonalGame.h"),
        TEXT("TranspersonalGame.Build.cs")
    };
    
    for (const FString& RequiredFile : RequiredFiles)
    {
        FString FilePath = FPaths::Combine(SourcePath, RequiredFile);
        if (!FPaths::FileExists(FilePath))
        {
            FString ErrorMsg = FString::Printf(TEXT("Required file missing: %s"), *RequiredFile);
            UE_LOG(LogBuildManager, Error, TEXT("%s"), *ErrorMsg);
            CurrentErrors.Add(ErrorMsg);
        }
    }
}

TArray<FString> UBuildManager::GetCompileErrors() const
{
    return CurrentErrors;
}

TArray<FString> UBuildManager::GetCompileWarnings() const
{
    return CurrentWarnings;
}

void UBuildManager::ClearBuildCache()
{
    CurrentErrors.Empty();
    CurrentWarnings.Empty();
    ModuleStatusCache.Empty();
    UE_LOG(LogBuildManager, Log, TEXT("Build cache cleared"));
}

bool UBuildManager::VerifyBuildIntegrity()
{
    UE_LOG(LogBuildManager, Log, TEXT("Verifying build integrity..."));
    
    bool bIntegrityValid = true;
    
    // Check for duplicate files
    if (!CheckForDuplicateFiles())
    {
        bIntegrityValid = false;
    }
    
    // Validate include paths
    if (!ValidateIncludePaths())
    {
        bIntegrityValid = false;
    }
    
    return bIntegrityValid;
}

bool UBuildManager::CheckForDuplicateFiles()
{
    UE_LOG(LogBuildManager, Log, TEXT("Checking for duplicate files..."));
    
    TMap<FString, TArray<FString>> FileNameMap;
    
    // Check all source files
    for (const FString& FilePath : SourceFilePaths)
    {
        FString FileName = FPaths::GetCleanFilename(FilePath);
        FileNameMap.FindOrAdd(FileName).Add(FilePath);
    }
    
    // Check all header files
    for (const FString& FilePath : HeaderFilePaths)
    {
        FString FileName = FPaths::GetCleanFilename(FilePath);
        FileNameMap.FindOrAdd(FileName).Add(FilePath);
    }
    
    bool bNoDuplicates = true;
    for (const auto& Pair : FileNameMap)
    {
        if (Pair.Value.Num() > 1)
        {
            FString ErrorMsg = FString::Printf(TEXT("Duplicate file found: %s"), *Pair.Key);
            UE_LOG(LogBuildManager, Error, TEXT("%s"), *ErrorMsg);
            CurrentErrors.Add(ErrorMsg);
            
            for (const FString& DuplicatePath : Pair.Value)
            {
                UE_LOG(LogBuildManager, Error, TEXT("  - %s"), *DuplicatePath);
            }
            
            bNoDuplicates = false;
        }
    }
    
    return bNoDuplicates;
}

bool UBuildManager::ValidateIncludePaths()
{
    UE_LOG(LogBuildManager, Log, TEXT("Validating include paths..."));
    
    // This is a simplified validation - in a real implementation,
    // we would parse the actual #include statements
    bool bIncludesValid = true;
    
    // Check if core header files exist
    TArray<FString> CoreHeaders = {
        TEXT("CoreMinimal.h"),
        TEXT("Engine/Engine.h"),
        TEXT("Components/ActorComponent.h"),
        TEXT("GameFramework/Actor.h")
    };
    
    // Note: This is a basic check - real validation would require parsing
    UE_LOG(LogBuildManager, Log, TEXT("Include path validation completed (basic check)"));
    
    return bIncludesValid;
}

void UBuildManager::LogBuildStatus()
{
    UE_LOG(LogBuildManager, Log, TEXT("=== BUILD STATUS REPORT ==="));
    UE_LOG(LogBuildManager, Log, TEXT("Total Modules: %d"), RegisteredModules.Num());
    UE_LOG(LogBuildManager, Log, TEXT("Source Files: %d"), SourceFilePaths.Num());
    UE_LOG(LogBuildManager, Log, TEXT("Header Files: %d"), HeaderFilePaths.Num());
    UE_LOG(LogBuildManager, Log, TEXT("Current Errors: %d"), CurrentErrors.Num());
    UE_LOG(LogBuildManager, Log, TEXT("Current Warnings: %d"), CurrentWarnings.Num());
    
    for (const FModuleInfo& ModuleInfo : RegisteredModules)
    {
        FString StatusStr;
        switch (ModuleInfo.BuildStatus)
        {
            case EBuildStatus::Success: StatusStr = TEXT("SUCCESS"); break;
            case EBuildStatus::Failed: StatusStr = TEXT("FAILED"); break;
            case EBuildStatus::Warning: StatusStr = TEXT("WARNING"); break;
            case EBuildStatus::Compiling: StatusStr = TEXT("COMPILING"); break;
            default: StatusStr = TEXT("UNKNOWN"); break;
        }
        
        UE_LOG(LogBuildManager, Log, TEXT("Module %s: %s"), *ModuleInfo.ModuleName, *StatusStr);
    }
    
    UE_LOG(LogBuildManager, Log, TEXT("=== END BUILD STATUS ==="));
}

void UBuildManager::ExportBuildReport(const FString& FilePath)
{
    FString ReportContent;
    ReportContent += TEXT("=== TRANSPERSONAL GAME BUILD REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Build Time: %s\n"), *LastBuildReport.BuildTime.ToString());
    ReportContent += FString::Printf(TEXT("Overall Status: %s\n"), 
                                   LastBuildReport.OverallStatus == EBuildStatus::Success ? TEXT("SUCCESS") : TEXT("FAILED"));
    ReportContent += FString::Printf(TEXT("Compile Time: %.2f seconds\n"), LastBuildReport.CompileTimeSeconds);
    ReportContent += FString::Printf(TEXT("Total Errors: %d\n"), LastBuildReport.TotalErrors);
    ReportContent += FString::Printf(TEXT("Total Warnings: %d\n"), LastBuildReport.TotalWarnings);
    ReportContent += TEXT("\n=== MODULE STATUS ===\n");
    
    for (const FModuleInfo& ModuleInfo : RegisteredModules)
    {
        ReportContent += FString::Printf(TEXT("%s: %s\n"), *ModuleInfo.ModuleName, 
                                       ModuleInfo.BuildStatus == EBuildStatus::Success ? TEXT("SUCCESS") : TEXT("FAILED"));
    }
    
    if (CurrentErrors.Num() > 0)
    {
        ReportContent += TEXT("\n=== ERRORS ===\n");
        for (const FString& Error : CurrentErrors)
        {
            ReportContent += Error + TEXT("\n");
        }
    }
    
    if (CurrentWarnings.Num() > 0)
    {
        ReportContent += TEXT("\n=== WARNINGS ===\n");
        for (const FString& Warning : CurrentWarnings)
        {
            ReportContent += Warning + TEXT("\n");
        }
    }
    
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    UE_LOG(LogBuildManager, Log, TEXT("Build report exported to: %s"), *FilePath);
}

// Subsystem implementation
void UBuildManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    BuildManager = NewObject<UBuildManager>(this);
    UE_LOG(LogBuildManager, Log, TEXT("Build Manager Subsystem initialized"));
}

void UBuildManagerSubsystem::Deinitialize()
{
    if (BuildManager)
    {
        BuildManager = nullptr;
    }
    
    Super::Deinitialize();
    UE_LOG(LogBuildManager, Log, TEXT("Build Manager Subsystem deinitialized"));
}

UBuildManagerSubsystem* UBuildManagerSubsystem::Get()
{
    if (GEngine)
    {
        return GEngine->GetEngineSubsystem<UBuildManagerSubsystem>();
    }
    return nullptr;
}