#include "Eng_CompilationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UEng_CompilationValidator::UEng_CompilationValidator()
{
    bIsInitialized = false;
    LastValidationTime = FDateTime::Now();
    
    // Initialize required core classes list
    PopulateRequiredClasses();
}

void UEng_CompilationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Compilation Validator initialized"));
    
    // Perform initial validation
    FEng_CompilationReport InitialReport = ValidateCompilation();
    
    if (!InitialReport.bAllClassesLoaded)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect - CRITICAL: %d classes failed to load"), 
               RequiredCoreClasses.Num() - InitialReport.LoadedClasses.Num());
        
        for (const FString& MissingClass : RequiredCoreClasses)
        {
            if (!InitialReport.LoadedClasses.Contains(MissingClass))
            {
                UE_LOG(LogTemp, Error, TEXT("Engine Architect - Missing class: %s"), *MissingClass);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect - All %d core classes loaded successfully"), 
               InitialReport.LoadedClasses.Num());
    }
}

void UEng_CompilationValidator::Deinitialize()
{
    bIsInitialized = false;
    Super::Deinitialize();
}

FEng_CompilationReport UEng_CompilationValidator::ValidateCompilation()
{
    FEng_CompilationReport Report;
    LastValidationTime = FDateTime::Now();
    
    // Find orphaned headers (headers without matching .cpp files)
    Report.MissingCppFiles = FindOrphanedHeaders();
    
    // Count total files
    FString SourcePath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/");
    TArray<FString> HeaderFiles;
    TArray<FString> CppFiles;
    
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourcePath, TEXT("*.h"), true, false);
    IFileManager::Get().FindFilesRecursive(CppFiles, *SourcePath, TEXT("*.cpp"), true, false);
    
    Report.TotalHeaderFiles = HeaderFiles.Num();
    Report.TotalCppFiles = CppFiles.Num();
    
    // Validate core classes
    int32 LoadedCount = 0;
    for (const FString& ClassName : RequiredCoreClasses)
    {
        if (IsClassLoaded(ClassName))
        {
            Report.LoadedClasses.Add(ClassName);
            LoadedCount++;
        }
        else
        {
            Report.CompilationErrors.Add(FString::Printf(TEXT("Failed to load class: %s"), *ClassName));
        }
    }
    
    Report.bAllClassesLoaded = (LoadedCount == RequiredCoreClasses.Num());
    
    // Store the report
    LastReport = Report;
    
    return Report;
}

bool UEng_CompilationValidator::IsClassLoaded(const FString& ClassName)
{
    FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *FullClassName);
    return (LoadedClass != nullptr);
}

bool UEng_CompilationValidator::ForceRecompile()
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect - Cannot force recompile: Validator not initialized"));
        return false;
    }
    
    // This would trigger a hot reload in the editor
    // For now, we log the request
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Recompile requested. Use Ctrl+Alt+F11 in editor."));
    
    return true;
}

FString UEng_CompilationValidator::GetCompilationSummary()
{
    FEng_CompilationReport CurrentReport = ValidateCompilation();
    
    FString Summary = FString::Printf(
        TEXT("Engine Architect - Compilation Summary:\n")
        TEXT("Header Files: %d\n")
        TEXT("CPP Files: %d\n")
        TEXT("Missing CPP Files: %d\n")
        TEXT("Loaded Classes: %d/%d\n")
        TEXT("Compilation Errors: %d\n")
        TEXT("All Classes Loaded: %s"),
        CurrentReport.TotalHeaderFiles,
        CurrentReport.TotalCppFiles,
        CurrentReport.MissingCppFiles.Num(),
        CurrentReport.LoadedClasses.Num(),
        RequiredCoreClasses.Num(),
        CurrentReport.CompilationErrors.Num(),
        CurrentReport.bAllClassesLoaded ? TEXT("YES") : TEXT("NO")
    );
    
    return Summary;
}

bool UEng_CompilationValidator::ValidateCoreClasses()
{
    int32 LoadedCount = 0;
    
    for (const FString& ClassName : RequiredCoreClasses)
    {
        if (IsClassLoaded(ClassName))
        {
            LoadedCount++;
            UE_LOG(LogTemp, Log, TEXT("Engine Architect - Core class loaded: %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Engine Architect - Core class MISSING: %s"), *ClassName);
        }
    }
    
    bool bAllLoaded = (LoadedCount == RequiredCoreClasses.Num());
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Core validation: %d/%d classes loaded"), 
           LoadedCount, RequiredCoreClasses.Num());
    
    return bAllLoaded;
}

TArray<FString> UEng_CompilationValidator::FindOrphanedHeaders()
{
    TArray<FString> OrphanedHeaders;
    
    FString SourcePath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/");
    TArray<FString> HeaderFiles;
    
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourcePath, TEXT("*.h"), true, false);
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        // Skip certain system headers
        if (HeaderFile.Contains(TEXT(".generated.h")) || 
            HeaderFile.Contains(TEXT("SharedTypes.h")) ||
            HeaderFile.Contains(TEXT("MODULE_MAPPING.h")))
        {
            continue;
        }
        
        if (!DoesCppExist(HeaderFile))
        {
            OrphanedHeaders.Add(HeaderFile);
        }
    }
    
    return OrphanedHeaders;
}

bool UEng_CompilationValidator::DoesCppExist(const FString& HeaderPath)
{
    FString CppPath = HeaderPath;
    CppPath = CppPath.Replace(TEXT(".h"), TEXT(".cpp"));
    
    return IFileManager::Get().FileExists(*CppPath);
}

void UEng_CompilationValidator::PopulateRequiredClasses()
{
    RequiredCoreClasses.Empty();
    
    // Core gameplay classes that MUST be loaded
    RequiredCoreClasses.Add(TEXT("TranspersonalCharacter"));
    RequiredCoreClasses.Add(TEXT("TranspersonalGameMode"));
    RequiredCoreClasses.Add(TEXT("TranspersonalGameState"));
    RequiredCoreClasses.Add(TEXT("PCGWorldGenerator"));
    RequiredCoreClasses.Add(TEXT("FoliageManager"));
    RequiredCoreClasses.Add(TEXT("CrowdSimulationManager"));
    RequiredCoreClasses.Add(TEXT("ProceduralWorldManager"));
    RequiredCoreClasses.Add(TEXT("BuildIntegrationManager"));
    
    // Engine Architect classes
    RequiredCoreClasses.Add(TEXT("Eng_CompilationValidator"));
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect - Populated %d required core classes"), RequiredCoreClasses.Num());
}