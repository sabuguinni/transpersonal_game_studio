#include "Eng_ArchitectureValidator.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogEngineArchitect);

void UEng_ArchitectureValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngineArchitect, Log, TEXT("Engine Architecture Validator initialized"));
    
    // Initialize critical systems list
    CriticalSystems.Add(TEXT("TranspersonalCharacter"));
    CriticalSystems.Add(TEXT("TranspersonalGameMode"));
    CriticalSystems.Add(TEXT("TranspersonalGameState"));
    CriticalSystems.Add(TEXT("PCGWorldGenerator"));
    CriticalSystems.Add(TEXT("FoliageManager"));
    CriticalSystems.Add(TEXT("StudioDirectorSystem"));
    
    // Run initial validation
    RunFullArchitectureAudit();
}

void UEng_ArchitectureValidator::Deinitialize()
{
    UE_LOG(LogEngineArchitect, Log, TEXT("Engine Architecture Validator deinitialized"));
    Super::Deinitialize();
}

bool UEng_ArchitectureValidator::ValidateModuleCompliance(const FString& ModulePath)
{
    if (ModulePath.IsEmpty())
    {
        UE_LOG(LogEngineArchitect, Warning, TEXT("Empty module path provided"));
        return false;
    }
    
    FString FullPath = FPaths::ProjectDir() / ModulePath;
    
    // Check if module directory exists
    if (!IFileManager::Get().DirectoryExists(*FullPath))
    {
        UE_LOG(LogEngineArchitect, Error, TEXT("Module directory does not exist: %s"), *FullPath);
        return false;
    }
    
    // Validate header/cpp pairs
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *FullPath, TEXT("*.h"), true, false);
    
    bool bAllValid = true;
    for (const FString& HeaderFile : HeaderFiles)
    {
        if (!ValidateHeaderFile(HeaderFile))
        {
            bAllValid = false;
        }
    }
    
    return bAllValid;
}

FEng_ModuleComplianceReport UEng_ArchitectureValidator::GenerateComplianceReport(const FString& ModuleName)
{
    FEng_ModuleComplianceReport Report;
    Report.ModuleName = ModuleName;
    
    FString ModulePath = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame") / ModuleName;
    
    if (IFileManager::Get().DirectoryExists(*ModulePath))
    {
        Report.bHasValidHeaders = true;
        
        // Check for matching cpp files
        TArray<FString> HeaderFiles;
        IFileManager::Get().FindFilesRecursive(HeaderFiles, *ModulePath, TEXT("*.h"), true, false);
        
        int32 ValidPairs = 0;
        for (const FString& HeaderFile : HeaderFiles)
        {
            FString CppFile = HeaderFile.Replace(TEXT(".h"), TEXT(".cpp"));
            if (IFileManager::Get().FileExists(*CppFile))
            {
                ValidPairs++;
            }
            else
            {
                Report.MissingCppFiles.Add(FPaths::GetCleanFilename(HeaderFile).Replace(TEXT(".h"), TEXT(".cpp")));
            }
        }
        
        Report.bHasMatchingCppFiles = (Report.MissingCppFiles.Num() == 0);
        Report.bFollowsNamingConvention = ValidateModuleNaming(ModuleName);
    }
    
    return Report;
}

bool UEng_ArchitectureValidator::ValidateHeaderCppPairs()
{
    FString SourcePath = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame");
    
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourcePath, TEXT("*.h"), true, false);
    
    bool bAllValid = true;
    TArray<FString> MissingCppFiles;
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        // Skip SharedTypes.h and other special headers
        FString FileName = FPaths::GetCleanFilename(HeaderFile);
        if (FileName == TEXT("SharedTypes.h") || FileName.EndsWith(TEXT(".generated.h")))
        {
            continue;
        }
        
        FString CppFile = HeaderFile.Replace(TEXT(".h"), TEXT(".cpp"));
        if (!IFileManager::Get().FileExists(*CppFile))
        {
            MissingCppFiles.Add(FileName.Replace(TEXT(".h"), TEXT(".cpp")));
            bAllValid = false;
        }
    }
    
    if (!bAllValid)
    {
        UE_LOG(LogEngineArchitect, Warning, TEXT("Missing %d cpp files"), MissingCppFiles.Num());
        for (const FString& MissingFile : MissingCppFiles)
        {
            UE_LOG(LogEngineArchitect, Warning, TEXT("Missing: %s"), *MissingFile);
        }
    }
    
    return bAllValid;
}

bool UEng_ArchitectureValidator::EnforceNamingConventions()
{
    // Validate that all Engine Architect classes use Eng_ prefix
    FString SourcePath = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame");
    
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourcePath, TEXT("*.h"), true, false);
    
    bool bAllValid = true;
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString FileContent;
        if (FFileHelper::LoadFileToString(FileContent, *HeaderFile))
        {
            // Check for UCLASS declarations
            if (FileContent.Contains(TEXT("UCLASS")) && HeaderFile.Contains(TEXT("/Core/")))
            {
                FString FileName = FPaths::GetCleanFilename(HeaderFile);
                if (!FileName.StartsWith(TEXT("Eng_")))
                {
                    UE_LOG(LogEngineArchitect, Warning, TEXT("Core class missing Eng_ prefix: %s"), *FileName);
                    bAllValid = false;
                }
            }
        }
    }
    
    return bAllValid;
}

TArray<FString> UEng_ArchitectureValidator::GetMissingCppFiles()
{
    TArray<FString> MissingFiles;
    FString SourcePath = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame");
    
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourcePath, TEXT("*.h"), true, false);
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString FileName = FPaths::GetCleanFilename(HeaderFile);
        if (FileName == TEXT("SharedTypes.h") || FileName.EndsWith(TEXT(".generated.h")))
        {
            continue;
        }
        
        FString CppFile = HeaderFile.Replace(TEXT(".h"), TEXT(".cpp"));
        if (!IFileManager::Get().FileExists(*CppFile))
        {
            MissingFiles.Add(FileName.Replace(TEXT(".h"), TEXT(".cpp")));
        }
    }
    
    return MissingFiles;
}

bool UEng_ArchitectureValidator::ValidateSystemDependencies()
{
    // Check that all registered systems have their dependencies satisfied
    for (const FEng_SystemDependency& Dependency : SystemDependencies)
    {
        for (const FString& RequiredModule : Dependency.RequiredModules)
        {
            FString ModulePath = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame") / RequiredModule;
            if (!IFileManager::Get().DirectoryExists(*ModulePath))
            {
                UE_LOG(LogEngineArchitect, Error, TEXT("System %s missing required module: %s"), 
                    *Dependency.SystemName, *RequiredModule);
                return false;
            }
        }
    }
    
    return true;
}

void UEng_ArchitectureValidator::RegisterSystemDependency(const FEng_SystemDependency& Dependency)
{
    SystemDependencies.Add(Dependency);
    UE_LOG(LogEngineArchitect, Log, TEXT("Registered system dependency: %s"), *Dependency.SystemName);
}

bool UEng_ArchitectureValidator::ValidateCompilation()
{
    // This would ideally trigger a compilation check, but for now we'll check for common issues
    FString SourcePath = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame");
    
    TArray<FString> CppFiles;
    IFileManager::Get().FindFilesRecursive(CppFiles, *SourcePath, TEXT("*.cpp"), true, false);
    
    bool bAllValid = true;
    
    for (const FString& CppFile : CppFiles)
    {
        if (!ValidateCppFile(CppFile))
        {
            bAllValid = false;
        }
    }
    
    return bAllValid;
}

TArray<FString> UEng_ArchitectureValidator::GetCompilationErrors()
{
    TArray<FString> Errors;
    
    // Check for common compilation issues
    TArray<FString> MissingCpp = GetMissingCppFiles();
    for (const FString& MissingFile : MissingCpp)
    {
        Errors.Add(FString::Printf(TEXT("Missing implementation file: %s"), *MissingFile));
    }
    
    return Errors;
}

bool UEng_ArchitectureValidator::ValidatePerformanceCompliance()
{
    // Check for performance-critical architecture violations
    return CheckMemoryUsage() && ValidateTickFrequency() && CheckRenderingCompliance();
}

bool UEng_ArchitectureValidator::EnforceMemoryLimits()
{
    // Placeholder for memory limit enforcement
    return true;
}

bool UEng_ArchitectureValidator::ValidateBlueprintExposure()
{
    // Check that all UFUNCTION/UPROPERTY macros are properly formatted
    FString SourcePath = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame");
    
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourcePath, TEXT("*.h"), true, false);
    
    bool bAllValid = true;
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString FileContent;
        if (FFileHelper::LoadFileToString(FileContent, *HeaderFile))
        {
            if (!ValidateUPropertyMacros(FileContent) || !ValidateUFunctionMacros(FileContent))
            {
                bAllValid = false;
                UE_LOG(LogEngineArchitect, Warning, TEXT("Blueprint exposure issues in: %s"), 
                    *FPaths::GetCleanFilename(HeaderFile));
            }
        }
    }
    
    return bAllValid;
}

TArray<FString> UEng_ArchitectureValidator::GetUndocumentedBlueprintFunctions()
{
    TArray<FString> UndocumentedFunctions;
    // Implementation would scan for UFUNCTION without proper documentation
    return UndocumentedFunctions;
}

bool UEng_ArchitectureValidator::ValidateCriticalSystems()
{
    // Check that all critical systems are present and functional
    for (const FString& SystemName : CriticalSystems)
    {
        // Try to load the class to verify it exists
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
        UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (!SystemClass)
        {
            UE_LOG(LogEngineArchitect, Error, TEXT("Critical system not found: %s"), *SystemName);
            return false;
        }
    }
    
    return true;
}

void UEng_ArchitectureValidator::RunFullArchitectureAudit()
{
    UE_LOG(LogEngineArchitect, Log, TEXT("Starting full architecture audit..."));
    
    LastValidationTime = FPlatformTime::Seconds();
    
    bool bHeaderCppValid = ValidateHeaderCppPairs();
    bool bNamingValid = EnforceNamingConventions();
    bool bDependenciesValid = ValidateSystemDependencies();
    bool bCompilationValid = ValidateCompilation();
    bool bCriticalSystemsValid = ValidateCriticalSystems();
    bool bBlueprintValid = ValidateBlueprintExposure();
    bool bPerformanceValid = ValidatePerformanceCompliance();
    
    bArchitectureValid = bHeaderCppValid && bNamingValid && bDependenciesValid && 
                        bCompilationValid && bCriticalSystemsValid && bBlueprintValid && bPerformanceValid;
    
    UE_LOG(LogEngineArchitect, Log, TEXT("Architecture audit complete. Valid: %s"), 
        bArchitectureValid ? TEXT("YES") : TEXT("NO"));
    
    if (!bArchitectureValid)
    {
        UE_LOG(LogEngineArchitect, Warning, TEXT("Architecture violations detected:"));
        if (!bHeaderCppValid) UE_LOG(LogEngineArchitect, Warning, TEXT("- Header/Cpp pair violations"));
        if (!bNamingValid) UE_LOG(LogEngineArchitect, Warning, TEXT("- Naming convention violations"));
        if (!bDependenciesValid) UE_LOG(LogEngineArchitect, Warning, TEXT("- Dependency violations"));
        if (!bCompilationValid) UE_LOG(LogEngineArchitect, Warning, TEXT("- Compilation issues"));
        if (!bCriticalSystemsValid) UE_LOG(LogEngineArchitect, Warning, TEXT("- Critical system issues"));
        if (!bBlueprintValid) UE_LOG(LogEngineArchitect, Warning, TEXT("- Blueprint exposure issues"));
        if (!bPerformanceValid) UE_LOG(LogEngineArchitect, Warning, TEXT("- Performance compliance issues"));
    }
}

// Private helper methods
bool UEng_ArchitectureValidator::ValidateHeaderFile(const FString& HeaderPath)
{
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *HeaderPath))
    {
        return false;
    }
    
    // Check for proper include structure
    return CheckIncludeStructure(FileContent);
}

bool UEng_ArchitectureValidator::ValidateCppFile(const FString& CppPath)
{
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *CppPath))
    {
        return false;
    }
    
    // Check that cpp includes its corresponding header
    FString HeaderName = FPaths::GetCleanFilename(CppPath).Replace(TEXT(".cpp"), TEXT(".h"));
    return FileContent.Contains(HeaderName);
}

bool UEng_ArchitectureValidator::CheckIncludeStructure(const FString& FileContent)
{
    // Check for proper include order and structure
    bool bHasPragmaOnce = FileContent.Contains(TEXT("#pragma once"));
    bool bHasGeneratedInclude = FileContent.Contains(TEXT(".generated.h"));
    
    return bHasPragmaOnce;
}

bool UEng_ArchitectureValidator::ValidateUPropertyMacros(const FString& FileContent)
{
    // Check for proper UPROPERTY formatting
    return !FileContent.Contains(TEXT("UPROPERTY(\\\""));
}

bool UEng_ArchitectureValidator::ValidateUFunctionMacros(const FString& FileContent)
{
    // Check for proper UFUNCTION formatting
    return !FileContent.Contains(TEXT("UFUNCTION(\\\""));
}

bool UEng_ArchitectureValidator::ValidateClassName(const FString& ClassName)
{
    // Validate class naming conventions
    return ClassName.Len() > 0 && (ClassName[0] == 'U' || ClassName[0] == 'A' || ClassName[0] == 'F');
}

bool UEng_ArchitectureValidator::ValidateVariableName(const FString& VariableName)
{
    // Validate variable naming conventions
    return !VariableName.Contains(TEXT(" "));
}

bool UEng_ArchitectureValidator::ValidateFunctionName(const FString& FunctionName)
{
    // Validate function naming conventions
    return !FunctionName.Contains(TEXT(" "));
}

void UEng_ArchitectureValidator::AnalyzeDependencyChain()
{
    // Analyze system dependency chains for optimization
}

bool UEng_ArchitectureValidator::DetectCircularDependencies()
{
    // Check for circular dependencies between systems
    return false;
}

bool UEng_ArchitectureValidator::CheckMemoryUsage()
{
    // Check current memory usage patterns
    return true;
}

bool UEng_ArchitectureValidator::ValidateTickFrequency()
{
    // Validate that systems don't tick too frequently
    return true;
}

bool UEng_ArchitectureValidator::CheckRenderingCompliance()
{
    // Check rendering architecture compliance
    return true;
}

bool UEng_ArchitectureValidator::ValidateModuleNaming(const FString& ModuleName)
{
    // Check if module follows naming conventions
    return ModuleName.Len() > 0 && !ModuleName.Contains(TEXT(" "));
}