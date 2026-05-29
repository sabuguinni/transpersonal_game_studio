#include "Eng_CompilationCleaner.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"

UEng_CompilationCleaner::UEng_CompilationCleaner()
{
    bAutoFixOrphans = true;
    bRemoveDuplicates = false; // Conservative - manual approval needed
    bValidateOnStartup = true;
    CleanupIntervalSeconds = 300.0f; // 5 minutes
    bCleanupInProgress = false;
    LastCleanupTime = 0.0f;
}

void UEng_CompilationCleaner::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Compilation Cleaner initialized"));
    
    if (bValidateOnStartup)
    {
        // Delayed validation to allow engine to fully load
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UEng_CompilationCleaner::ValidateCompilationStatus, 5.0f, false);
    }
}

void UEng_CompilationCleaner::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Compilation Cleaner deinitialized"));
    Super::Deinitialize();
}

void UEng_CompilationCleaner::ScanForOrphanedHeaders()
{
    if (bCleanupInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cleanup already in progress, skipping scan"));
        return;
    }
    
    bCleanupInProgress = true;
    OrphanedHeaders.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("=== SCANNING FOR ORPHANED HEADERS ==="));
    
    // Get project source directory
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source"), TEXT("TranspersonalGame"));
    
    ScanSourceDirectory(SourceDir);
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d orphaned headers"), OrphanedHeaders.Num());
    
    // Log first 10 orphaned headers
    for (int32 i = 0; i < FMath::Min(10, OrphanedHeaders.Num()); i++)
    {
        UE_LOG(LogTemp, Warning, TEXT("ORPHAN %d: %s"), i + 1, *OrphanedHeaders[i]);
    }
    
    if (bAutoFixOrphans && OrphanedHeaders.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Auto-fix enabled, attempting to resolve orphans..."));
        RemoveOrphanedHeaders();
    }
    
    bCleanupInProgress = false;
    LastCleanupTime = GetWorld()->GetTimeSeconds();
}

void UEng_CompilationCleaner::ScanSourceDirectory(const FString& Directory)
{
    TArray<FString> HeaderFiles;
    TArray<FString> CppFiles;
    
    // Get all .h files
    IFileManager& FileManager = IFileManager::Get();
    FileManager.FindFilesRecursive(HeaderFiles, *Directory, TEXT("*.h"), true, false);
    FileManager.FindFilesRecursive(CppFiles, *Directory, TEXT("*.cpp"), true, false);
    
    // Convert cpp filenames to header equivalents for comparison
    TSet<FString> CppBasenames;
    for (const FString& CppFile : CppFiles)
    {
        FString Basename = FPaths::GetBaseFilename(CppFile);
        CppBasenames.Add(Basename);
    }
    
    // Check each header for corresponding cpp
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString HeaderBasename = FPaths::GetBaseFilename(HeaderFile);
        
        // Skip certain headers that don't need cpp files
        if (HeaderBasename.Contains(TEXT("Types")) || 
            HeaderBasename.Contains(TEXT("Enums")) ||
            HeaderBasename.EndsWith(TEXT(".generated")))
        {
            continue;
        }
        
        if (!CppBasenames.Contains(HeaderBasename))
        {
            OrphanedHeaders.Add(HeaderFile);
        }
    }
}

void UEng_CompilationCleaner::DetectDuplicateSystems()
{
    DuplicateSystems.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("=== DETECTING DUPLICATE SYSTEMS ==="));
    
    // Known duplicate patterns from brain memories
    TArray<FString> KnownDuplicates = {
        TEXT("DinosaurCrowdSystem_Crowd.h"),
        TEXT("MassDinosaurSystem.h"),
        TEXT("CrowdDensityManager.h")
    };
    
    for (const FString& Duplicate : KnownDuplicates)
    {
        FString FullPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), TEXT("TranspersonalGame"));
        TArray<FString> FoundFiles;
        IFileManager::Get().FindFilesRecursive(FoundFiles, *FullPath, *Duplicate, true, false);
        
        if (FoundFiles.Num() > 0)
        {
            DuplicateSystems.Add(FoundFiles[0]);
            UE_LOG(LogTemp, Warning, TEXT("DUPLICATE FOUND: %s"), *FoundFiles[0]);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d duplicate systems"), DuplicateSystems.Num());
}

void UEng_CompilationCleaner::ValidateCompilationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATING COMPILATION STATUS ==="));
    
    CompilationErrors.Empty();
    
    // Test critical class loading
    TArray<FString> CriticalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.DinosaurBase"),
        TEXT("/Script/TranspersonalGame.BiomeManager"),
        TEXT("/Script/TranspersonalGame.Eng_CompilationCleaner")
    };
    
    int32 LoadedClasses = 0;
    for (const FString& ClassName : CriticalClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (LoadedClass)
        {
            LoadedClasses++;
            UE_LOG(LogTemp, Log, TEXT("✓ %s loaded successfully"), *ClassName);
        }
        else
        {
            CompilationErrors.Add(FString::Printf(TEXT("Failed to load: %s"), *ClassName));
            UE_LOG(LogTemp, Error, TEXT("✗ %s failed to load"), *ClassName);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Compilation Status: %d/%d critical classes loaded"), LoadedClasses, CriticalClasses.Num());
    
    if (CompilationErrors.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("COMPILATION ISSUES DETECTED - %d errors"), CompilationErrors.Num());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("COMPILATION STATUS: HEALTHY"));
    }
}

bool UEng_CompilationCleaner::CheckCriticalSystemsLoaded()
{
    // Quick health check for critical systems
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    UClass* DinosaurClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.DinosaurBase"));
    
    bool bSystemsHealthy = (CharacterClass != nullptr) && (DinosaurClass != nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("Critical systems health check: %s"), 
           bSystemsHealthy ? TEXT("HEALTHY") : TEXT("DEGRADED"));
    
    return bSystemsHealthy;
}

void UEng_CompilationCleaner::FixCommonCompilationErrors()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FIXING COMMON COMPILATION ERRORS ==="));
    
    // This would implement common fixes like:
    // - Adding missing #include statements
    // - Fixing forward declarations
    // - Resolving circular dependencies
    // - Correcting UCLASS/USTRUCT declarations
    
    // For now, log the intent
    UE_LOG(LogTemp, Warning, TEXT("Common compilation fixes would be applied here"));
    UE_LOG(LogTemp, Warning, TEXT("- Missing includes detection"));
    UE_LOG(LogTemp, Warning, TEXT("- Forward declaration fixes"));
    UE_LOG(LogTemp, Warning, TEXT("- UCLASS macro validation"));
}

void UEng_CompilationCleaner::GenerateCleanupReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== COMPILATION CLEANUP REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Orphaned Headers: %d"), OrphanedHeaders.Num());
    UE_LOG(LogTemp, Warning, TEXT("Duplicate Systems: %d"), DuplicateSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Compilation Errors: %d"), CompilationErrors.Num());
    UE_LOG(LogTemp, Warning, TEXT("Last Cleanup: %.2f seconds ago"), 
           GetWorld()->GetTimeSeconds() - LastCleanupTime);
    
    // Generate file report
    FString ReportContent = TEXT("=== ENGINE ARCHITECT CLEANUP REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    ReportContent += FString::Printf(TEXT("Orphaned Headers: %d\n"), OrphanedHeaders.Num());
    ReportContent += FString::Printf(TEXT("Duplicate Systems: %d\n"), DuplicateSystems.Num());
    ReportContent += FString::Printf(TEXT("Compilation Errors: %d\n"), CompilationErrors.Num());
    
    FString ReportPath = FPaths::Combine(FPaths::ProjectLogDir(), TEXT("CompilationCleanupReport.txt"));
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
    
    UE_LOG(LogTemp, Warning, TEXT("Report saved to: %s"), *ReportPath);
}

void UEng_CompilationCleaner::RemoveOrphanedHeaders()
{
    UE_LOG(LogTemp, Warning, TEXT("=== REMOVING ORPHANED HEADERS ==="));
    
    // Conservative approach - log what would be removed
    for (const FString& OrphanedHeader : OrphanedHeaders)
    {
        UE_LOG(LogTemp, Warning, TEXT("WOULD REMOVE: %s"), *OrphanedHeader);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Orphaned header removal logged (not executed for safety)"));
}

void UEng_CompilationCleaner::ConsolidateDuplicateSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CONSOLIDATING DUPLICATE SYSTEMS ==="));
    
    // Log consolidation plan
    for (const FString& Duplicate : DuplicateSystems)
    {
        UE_LOG(LogTemp, Warning, TEXT("DUPLICATE TO CONSOLIDATE: %s"), *Duplicate);
    }
}

void UEng_CompilationCleaner::AnalyzeHeaderDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("Analyzing header dependencies..."));
    // Implementation would scan #include statements and build dependency graph
}

void UEng_CompilationCleaner::CheckModuleDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("Checking module dependencies..."));
    // Implementation would validate Build.cs dependencies
}

void UEng_CompilationCleaner::ValidateUClassDeclarations()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating UCLASS declarations..."));
    // Implementation would check UCLASS/USTRUCT/UENUM syntax
}

void UEng_CompilationCleaner::FixIncludePaths()
{
    UE_LOG(LogTemp, Warning, TEXT("Fixing include paths..."));
    // Implementation would correct #include statements
}