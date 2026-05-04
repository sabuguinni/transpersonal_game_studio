#include "Eng_CompilationManager.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

DEFINE_LOG_CATEGORY(LogEngCompilation);

UEng_CompilationManager::UEng_CompilationManager()
{
    GhostHeaderCount = 0;
    MissingCppCount = 0;
    
    // Initialize critical classes that must be loadable
    CriticalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.Eng_BiomeManager"),
        TEXT("/Script/TranspersonalGame.ProductionCoordinator")
    };
    
    // Initialize source directories to scan
    SourceDirectories = {
        TEXT("Core"),
        TEXT("Characters"),
        TEXT("World"),
        TEXT("Environment"),
        TEXT("AI"),
        TEXT("Combat"),
        TEXT("Quest")
    };
}

void UEng_CompilationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngCompilation, Log, TEXT("Engine Architect Compilation Manager initialized"));
    
    // Perform initial validation
    ValidateHeaderCppPairs();
    ValidateModuleLoading();
}

void UEng_CompilationManager::Deinitialize()
{
    UE_LOG(LogEngCompilation, Log, TEXT("Engine Architect Compilation Manager deinitialized"));
    Super::Deinitialize();
}

bool UEng_CompilationManager::ValidateHeaderCppPairs()
{
    UE_LOG(LogEngCompilation, Log, TEXT("=== VALIDATING HEADER/CPP PAIRS ==="));
    
    TArray<FString> MissingCppFiles;
    FString ProjectSourceDir = GetProjectSourceDirectory();
    
    if (ProjectSourceDir.IsEmpty())
    {
        UE_LOG(LogEngCompilation, Error, TEXT("Could not find project source directory"));
        return false;
    }
    
    // Scan each source directory
    for (const FString& SubDir : SourceDirectories)
    {
        FString FullPath = FPaths::Combine(ProjectSourceDir, SubDir);
        ScanDirectoryForPairs(FullPath, MissingCppFiles);
    }
    
    MissingCppCount = MissingCppFiles.Num();
    GhostHeaderCount = MissingCppCount; // Same thing in this context
    LastValidationTime = FDateTime::Now();
    
    UE_LOG(LogEngCompilation, Warning, TEXT("Found %d headers without corresponding .cpp files"), MissingCppCount);
    
    for (int32 i = 0; i < FMath::Min(10, MissingCppFiles.Num()); ++i)
    {
        UE_LOG(LogEngCompilation, Warning, TEXT("  Missing .cpp: %s"), *MissingCppFiles[i]);
    }
    
    return MissingCppCount == 0;
}

bool UEng_CompilationManager::ValidateModuleLoading()
{
    UE_LOG(LogEngCompilation, Log, TEXT("=== VALIDATING MODULE LOADING ==="));
    
    bool bAllClassesLoadable = true;
    
    for (const FString& ClassName : CriticalClasses)
    {
        bool bCanLoad = CanLoadClass(ClassName);
        
        if (bCanLoad)
        {
            UE_LOG(LogEngCompilation, Log, TEXT("✓ Class loadable: %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogEngCompilation, Error, TEXT("✗ Class NOT loadable: %s"), *ClassName);
            bAllClassesLoadable = false;
        }
    }
    
    return bAllClassesLoadable;
}

int32 UEng_CompilationManager::CleanupGhostHeaders()
{
    UE_LOG(LogEngCompilation, Warning, TEXT("=== GHOST HEADER CLEANUP ==="));
    UE_LOG(LogEngCompilation, Warning, TEXT("This function identifies ghost headers but does not delete them automatically"));
    UE_LOG(LogEngCompilation, Warning, TEXT("Manual review required before deletion"));
    
    // Re-validate to get current count
    ValidateHeaderCppPairs();
    
    UE_LOG(LogEngCompilation, Warning, TEXT("Found %d ghost headers that need manual review"), GhostHeaderCount);
    
    return GhostHeaderCount;
}

bool UEng_CompilationManager::ForceModuleRecompile()
{
    UE_LOG(LogEngCompilation, Log, TEXT("=== FORCE MODULE RECOMPILE ==="));
    UE_LOG(LogEngCompilation, Warning, TEXT("Module recompilation must be triggered externally via UnrealBuildTool"));
    UE_LOG(LogEngCompilation, Log, TEXT("Command: Build.sh TranspersonalGame Linux Development"));
    
    // We cannot actually trigger recompilation from within the running editor
    // This would need to be done via external build system
    return false;
}

FString UEng_CompilationManager::GetCompilationStatusReport()
{
    FString Report;
    Report += FString::Printf(TEXT("=== ENGINE ARCHITECT COMPILATION STATUS ===\n"));
    Report += FString::Printf(TEXT("Last Validation: %s\n"), *LastValidationTime.ToString());
    Report += FString::Printf(TEXT("Ghost Headers: %d\n"), GhostHeaderCount);
    Report += FString::Printf(TEXT("Missing .cpp Files: %d\n"), MissingCppCount);
    Report += FString::Printf(TEXT("Critical Classes Checked: %d\n"), CriticalClasses.Num());
    
    // Test module loading status
    int32 LoadableClasses = 0;
    for (const FString& ClassName : CriticalClasses)
    {
        if (CanLoadClass(ClassName))
        {
            LoadableClasses++;
        }
    }
    
    Report += FString::Printf(TEXT("Loadable Classes: %d/%d\n"), LoadableClasses, CriticalClasses.Num());
    
    if (GhostHeaderCount > 0)
    {
        Report += FString::Printf(TEXT("STATUS: CRITICAL - Ghost headers detected\n"));
    }
    else if (LoadableClasses < CriticalClasses.Num())
    {
        Report += FString::Printf(TEXT("STATUS: WARNING - Some classes not loadable\n"));
    }
    else
    {
        Report += FString::Printf(TEXT("STATUS: HEALTHY - All systems operational\n"));
    }
    
    return Report;
}

void UEng_CompilationManager::ScanDirectoryForPairs(const FString& Directory, TArray<FString>& OutMissingCpp)
{
    if (!FPaths::DirectoryExists(Directory))
    {
        return;
    }
    
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFiles(HeaderFiles, *(Directory / TEXT("*.h")), true, false);
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString HeaderPath = FPaths::Combine(Directory, HeaderFile);
        FString CppPath = HeaderPath.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!FPaths::FileExists(CppPath))
        {
            OutMissingCpp.Add(HeaderPath);
        }
    }
    
    // Recursively scan subdirectories
    TArray<FString> SubDirectories;
    IFileManager::Get().FindFiles(SubDirectories, *(Directory / TEXT("*")), false, true);
    
    for (const FString& SubDir : SubDirectories)
    {
        FString SubDirPath = FPaths::Combine(Directory, SubDir);
        ScanDirectoryForPairs(SubDirPath, OutMissingCpp);
    }
}

bool UEng_CompilationManager::CanLoadClass(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    return LoadedClass != nullptr;
}

FString UEng_CompilationManager::GetProjectSourceDirectory() const
{
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source"), TEXT("TranspersonalGame"));
    
    if (FPaths::DirectoryExists(SourceDir))
    {
        return SourceDir;
    }
    
    return FString();
}