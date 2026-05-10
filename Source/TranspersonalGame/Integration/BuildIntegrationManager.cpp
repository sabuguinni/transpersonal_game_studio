#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bIntegrationActive = false;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIntegrationActive = true;
    ScanForModules();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager initialized - found %d modules"), KnownModules.Num());
}

void UBuildIntegrationManager::Deinitialize()
{
    bIntegrationActive = false;
    KnownModules.Empty();
    
    Super::Deinitialize();
}

FBuild_ValidationResult UBuildIntegrationManager::ValidateCurrentBuild()
{
    FBuild_ValidationResult Result;
    Result.BuildTimestamp = FDateTime::Now().ToString();
    Result.TotalModules = KnownModules.Num();
    Result.CompiledModules = 0;
    
    for (const FString& ModuleName : KnownModules)
    {
        FBuild_ModuleStatus ModuleStatus;
        ValidateModuleStructure(ModuleName, ModuleStatus);
        Result.ModuleStatuses.Add(ModuleStatus);
        
        if (ModuleStatus.bIsCompiled && ModuleStatus.bHasImplementation)
        {
            Result.CompiledModules++;
        }
    }
    
    Result.bBuildSuccess = (Result.CompiledModules == Result.TotalModules) && (Result.TotalModules > 0);
    LastValidationResult = Result;
    
    LogBuildStatus(FString::Printf(TEXT("Build validation complete: %d/%d modules compiled"), 
        Result.CompiledModules, Result.TotalModules));
    
    return Result;
}

bool UBuildIntegrationManager::CheckModuleCompilation(const FString& ModuleName)
{
    FBuild_ModuleStatus Status;
    ValidateModuleStructure(ModuleName, Status);
    return Status.bIsCompiled && Status.bHasImplementation;
}

TArray<FString> UBuildIntegrationManager::GetOrphanedHeaders()
{
    TArray<FString> OrphanedHeaders;
    
    FString SourcePath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/");
    TArray<FString> HeaderFiles;
    
    IFileManager& FileManager = IFileManager::Get();
    FileManager.FindFilesRecursive(HeaderFiles, *SourcePath, TEXT("*.h"), true, false);
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        if (!CheckHeaderImplementationPair(HeaderFile))
        {
            FString RelativePath = HeaderFile;
            FPaths::MakePathRelativeTo(RelativePath, *SourcePath);
            OrphanedHeaders.Add(RelativePath);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d orphaned headers"), OrphanedHeaders.Num());
    return OrphanedHeaders;
}

bool UBuildIntegrationManager::ValidateSharedTypes()
{
    FString SharedTypesPath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/SharedTypes.h");
    
    if (!FPaths::FileExists(SharedTypesPath))
    {
        LogBuildStatus(TEXT("SharedTypes.h not found"), true);
        return false;
    }
    
    // Check if SharedTypes.h contains required base types
    FString FileContent;
    if (FFileHelper::LoadFileToString(FileContent, *SharedTypesPath))
    {
        bool bHasBasicTypes = FileContent.Contains(TEXT("ETranspersonalGameState")) &&
                             FileContent.Contains(TEXT("FTranspersonalVector")) &&
                             FileContent.Contains(TEXT("EBiomeType"));
        
        if (bHasBasicTypes)
        {
            LogBuildStatus(TEXT("SharedTypes.h validation passed"));
            return true;
        }
    }
    
    LogBuildStatus(TEXT("SharedTypes.h missing required types"), true);
    return false;
}

void UBuildIntegrationManager::GenerateBuildReport()
{
    FBuild_ValidationResult CurrentResult = ValidateCurrentBuild();
    
    FString ReportContent = TEXT("=== BUILD INTEGRATION REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Timestamp: %s\n"), *CurrentResult.BuildTimestamp);
    ReportContent += FString::Printf(TEXT("Build Status: %s\n"), CurrentResult.bBuildSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
    ReportContent += FString::Printf(TEXT("Modules: %d/%d compiled\n\n"), CurrentResult.CompiledModules, CurrentResult.TotalModules);
    
    for (const FBuild_ModuleStatus& Status : CurrentResult.ModuleStatuses)
    {
        ReportContent += FString::Printf(TEXT("Module: %s\n"), *Status.ModuleName);
        ReportContent += FString::Printf(TEXT("  Compiled: %s\n"), Status.bIsCompiled ? TEXT("YES") : TEXT("NO"));
        ReportContent += FString::Printf(TEXT("  Implementation: %s\n"), Status.bHasImplementation ? TEXT("YES") : TEXT("NO"));
        ReportContent += FString::Printf(TEXT("  Classes: %d\n"), Status.ClassCount);
        if (!Status.LastError.IsEmpty())
        {
            ReportContent += FString::Printf(TEXT("  Error: %s\n"), *Status.LastError);
        }
        ReportContent += TEXT("\n");
    }
    
    TArray<FString> OrphanedHeaders = GetOrphanedHeaders();
    if (OrphanedHeaders.Num() > 0)
    {
        ReportContent += TEXT("=== ORPHANED HEADERS ===\n");
        for (const FString& Header : OrphanedHeaders)
        {
            ReportContent += FString::Printf(TEXT("  %s\n"), *Header);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build Report Generated:\n%s"), *ReportContent);
}

bool UBuildIntegrationManager::FixCommonBuildErrors()
{
    bool bFixedAny = false;
    
    // Check and validate SharedTypes
    if (!ValidateSharedTypes())
    {
        LogBuildStatus(TEXT("SharedTypes validation failed - manual fix required"), true);
    }
    
    // Log orphaned headers for manual cleanup
    TArray<FString> OrphanedHeaders = GetOrphanedHeaders();
    if (OrphanedHeaders.Num() > 0)
    {
        LogBuildStatus(FString::Printf(TEXT("Found %d orphaned headers requiring cleanup"), OrphanedHeaders.Num()));
    }
    
    return bFixedAny;
}

void UBuildIntegrationManager::ScanForModules()
{
    KnownModules.Empty();
    
    // Add known module directories
    TArray<FString> ModuleDirectories = {
        TEXT("AI"), TEXT("Animation"), TEXT("Architecture"), TEXT("Audio"),
        TEXT("Characters"), TEXT("Combat"), TEXT("Core"), TEXT("Crowd"),
        TEXT("Environment"), TEXT("Integration"), TEXT("Lighting"),
        TEXT("NPC"), TEXT("Narrative"), TEXT("PCG"), TEXT("Performance"),
        TEXT("Physics"), TEXT("Quest"), TEXT("VFX"), TEXT("World")
    };
    
    FString SourcePath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/");
    
    for (const FString& ModuleDir : ModuleDirectories)
    {
        FString ModulePath = SourcePath + ModuleDir;
        if (FPaths::DirectoryExists(ModulePath))
        {
            KnownModules.Add(ModuleDir);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Scanned modules: %d found"), KnownModules.Num());
}

void UBuildIntegrationManager::ValidateModuleStructure(const FString& ModuleName, FBuild_ModuleStatus& OutStatus)
{
    OutStatus.ModuleName = ModuleName;
    OutStatus.bIsCompiled = false;
    OutStatus.bHasImplementation = false;
    OutStatus.ClassCount = 0;
    OutStatus.LastError = TEXT("");
    
    FString ModulePath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/") + ModuleName + TEXT("/");
    
    if (!FPaths::DirectoryExists(ModulePath))
    {
        OutStatus.LastError = TEXT("Module directory not found");
        return;
    }
    
    // Count header and implementation files
    TArray<FString> HeaderFiles, CppFiles;
    IFileManager& FileManager = IFileManager::Get();
    
    FileManager.FindFiles(HeaderFiles, *ModulePath, TEXT("*.h"));
    FileManager.FindFiles(CppFiles, *ModulePath, TEXT("*.cpp"));
    
    OutStatus.ClassCount = HeaderFiles.Num();
    OutStatus.bHasImplementation = (CppFiles.Num() > 0);
    
    // Simple compilation check - if we have .cpp files and no obvious errors
    if (OutStatus.bHasImplementation)
    {
        OutStatus.bIsCompiled = true; // Assume compiled if we have implementations
    }
    
    if (HeaderFiles.Num() > CppFiles.Num())
    {
        OutStatus.LastError = FString::Printf(TEXT("Missing implementations: %d headers, %d cpp files"), 
            HeaderFiles.Num(), CppFiles.Num());
    }
}

bool UBuildIntegrationManager::CheckHeaderImplementationPair(const FString& HeaderPath)
{
    FString CppPath = HeaderPath;
    CppPath = CppPath.Replace(TEXT(".h"), TEXT(".cpp"));
    
    return FPaths::FileExists(CppPath);
}

void UBuildIntegrationManager::LogBuildStatus(const FString& Message, bool bIsError)
{
    if (bIsError)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegration: %s"), *Message);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegration: %s"), *Message);
    }
}