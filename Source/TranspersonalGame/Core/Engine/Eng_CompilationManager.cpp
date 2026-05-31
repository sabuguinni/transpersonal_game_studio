#include "Eng_CompilationManager.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

void UEng_CompilationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Define critical modules that must always be enabled
    CriticalModules = {
        TEXT("TranspersonalGame"),
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameMode"),
        TEXT("TranspersonalGameState"),
        TEXT("DinosaurTRex"),
        TEXT("DinosaurCombatAIController")
    };
    
    TotalSourceFiles = 0;
    EnabledSourceFiles = 0;
    DisabledSourceFiles = 0;
    
    InitializeModuleTracking();
    ScanSourceDirectory();
    
    UE_LOG(LogTemp, Log, TEXT("Engine Compilation Manager initialized - %d enabled, %d disabled files"), 
           EnabledSourceFiles, DisabledSourceFiles);
}

void UEng_CompilationManager::Deinitialize()
{
    ModuleInfos.Empty();
    CriticalModules.Empty();
    Super::Deinitialize();
}

bool UEng_CompilationManager::CheckModuleCompilationStatus(const FString& ModuleName)
{
    // Try to load the module class to check if it compiled successfully
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ModuleName);
    UClass* ModuleClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    EEng_CompilationStatus Status = ModuleClass ? EEng_CompilationStatus::Success : EEng_CompilationStatus::Failed;
    UpdateModuleStatus(ModuleName, Status);
    
    return ModuleClass != nullptr;
}

TArray<FEng_ModuleCompilationInfo> UEng_CompilationManager::GetAllModuleStatus()
{
    // Update status for all tracked modules
    for (FEng_ModuleCompilationInfo& ModuleInfo : ModuleInfos)
    {
        CheckModuleCompilationStatus(ModuleInfo.ModuleName);
    }
    
    return ModuleInfos;
}

bool UEng_CompilationManager::EnableModule(const FString& ModuleName)
{
    // This would require file system operations to rename .cpp.disabled to .cpp
    // For now, just track the request
    UpdateModuleStatus(ModuleName, EEng_CompilationStatus::Unknown);
    
    UE_LOG(LogTemp, Warning, TEXT("Module enable requested for %s - requires file system operations"), *ModuleName);
    return false; // Not implemented in this version
}

bool UEng_CompilationManager::DisableModule(const FString& ModuleName)
{
    // Check if module is critical
    if (CriticalModules.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot disable critical module: %s"), *ModuleName);
        return false;
    }
    
    // This would require file system operations to rename .cpp to .cpp.disabled
    // For now, just track the request
    UpdateModuleStatus(ModuleName, EEng_CompilationStatus::Disabled);
    
    UE_LOG(LogTemp, Warning, TEXT("Module disable requested for %s - requires file system operations"), *ModuleName);
    return false; // Not implemented in this version
}

int32 UEng_CompilationManager::GetTotalEnabledModules() const
{
    return EnabledSourceFiles;
}

int32 UEng_CompilationManager::GetTotalDisabledModules() const
{
    return DisabledSourceFiles;
}

bool UEng_CompilationManager::ValidateMinimalBuild()
{
    bool bAllCriticalModulesValid = true;
    
    for (const FString& CriticalModule : CriticalModules)
    {
        if (!CheckModuleCompilationStatus(CriticalModule))
        {
            UE_LOG(LogTemp, Error, TEXT("Critical module %s failed validation"), *CriticalModule);
            bAllCriticalModulesValid = false;
        }
    }
    
    if (bAllCriticalModulesValid)
    {
        UE_LOG(LogTemp, Log, TEXT("Minimal build validation PASSED - all critical modules compiled"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Minimal build validation FAILED - critical modules missing"));
    }
    
    return bAllCriticalModulesValid;
}

TArray<FString> UEng_CompilationManager::GetCriticalModules() const
{
    return CriticalModules;
}

void UEng_CompilationManager::InitializeModuleTracking()
{
    ModuleInfos.Empty();
    
    // Initialize tracking for critical modules
    for (const FString& ModuleName : CriticalModules)
    {
        FEng_ModuleCompilationInfo ModuleInfo;
        ModuleInfo.ModuleName = ModuleName;
        ModuleInfo.Status = EEng_CompilationStatus::Unknown;
        ModuleInfo.bIsEnabled = true;
        ModuleInfos.Add(ModuleInfo);
    }
    
    // Add other known modules
    TArray<FString> AdditionalModules = {
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("ProceduralWorldManager"),
        TEXT("BuildIntegrationManager")
    };
    
    for (const FString& ModuleName : AdditionalModules)
    {
        FEng_ModuleCompilationInfo ModuleInfo;
        ModuleInfo.ModuleName = ModuleName;
        ModuleInfo.Status = EEng_CompilationStatus::Unknown;
        ModuleInfo.bIsEnabled = true;
        ModuleInfos.Add(ModuleInfo);
    }
}

void UEng_CompilationManager::UpdateModuleStatus(const FString& ModuleName, EEng_CompilationStatus Status)
{
    for (FEng_ModuleCompilationInfo& ModuleInfo : ModuleInfos)
    {
        if (ModuleInfo.ModuleName == ModuleName)
        {
            ModuleInfo.Status = Status;
            ModuleInfo.LastCompileTime = FPlatformTime::Seconds();
            return;
        }
    }
    
    // If module not found, add it
    FEng_ModuleCompilationInfo NewModuleInfo;
    NewModuleInfo.ModuleName = ModuleName;
    NewModuleInfo.Status = Status;
    NewModuleInfo.LastCompileTime = FPlatformTime::Seconds();
    NewModuleInfo.bIsEnabled = true;
    ModuleInfos.Add(NewModuleInfo);
}

bool UEng_CompilationManager::IsModuleFileEnabled(const FString& FilePath)
{
    return !FilePath.EndsWith(TEXT(".disabled"));
}

void UEng_CompilationManager::ScanSourceDirectory()
{
    FString SourcePath = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame");
    
    TArray<FString> FoundFiles;
    IFileManager::Get().FindFilesRecursive(FoundFiles, *SourcePath, TEXT("*.cpp"), true, false);
    
    EnabledSourceFiles = 0;
    DisabledSourceFiles = 0;
    
    for (const FString& FilePath : FoundFiles)
    {
        if (IsModuleFileEnabled(FilePath))
        {
            EnabledSourceFiles++;
        }
        else
        {
            DisabledSourceFiles++;
        }
    }
    
    TotalSourceFiles = EnabledSourceFiles + DisabledSourceFiles;
    
    UE_LOG(LogTemp, Log, TEXT("Source scan complete: %d total files (%d enabled, %d disabled)"), 
           TotalSourceFiles, EnabledSourceFiles, DisabledSourceFiles);
}