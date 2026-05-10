#include "Eng_CompilationOrchestrator_v3.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

UEng_CompilationOrchestrator_v3::UEng_CompilationOrchestrator_v3()
{
    // Initialize critical class names that must be loadable
    CriticalClassNames = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("DinosaurBase"),
        TEXT("BiomeManager"),
        TEXT("Dir_ProductionCoordinator"),
        TEXT("Core_PhysicsManager"),
        TEXT("Core_CharacterMovement"),
        TEXT("Core_CollisionSystem")
    };

    OrphanedHeaderCount = 0;
    FailedClassCount = 0;
    bCompilationHealthy = false;
}

void UEng_CompilationOrchestrator_v3::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Compilation Orchestrator v3 Initialized"));
    
    // Perform initial compilation validation
    ValidateCompilationState();
}

void UEng_CompilationOrchestrator_v3::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Compilation Orchestrator v3 Deinitialized"));
    Super::Deinitialize();
}

bool UEng_CompilationOrchestrator_v3::ValidateCompilationState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT - COMPILATION STATE VALIDATION ==="));

    // Reset status tracking
    ClassLoadingStatus.Empty();
    FailedClassCount = 0;

    // Validate critical class loading
    bool bAllClassesValid = ValidateCriticalClasses();
    
    // Check for orphaned headers
    bool bNoOrphanedHeaders = CheckHeaderCppPairs();
    
    // Validate include structure
    bool bIncludesValid = ValidateIncludeStructure();
    
    // Update overall compilation health
    bCompilationHealthy = bAllClassesValid && bNoOrphanedHeaders && bIncludesValid;
    
    UpdateCompilationMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Compilation Health Status: %s"), 
           bCompilationHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
    
    return bCompilationHealthy;
}

bool UEng_CompilationOrchestrator_v3::ValidateCriticalClasses()
{
    UE_LOG(LogTemp, Warning, TEXT("--- Validating Critical Classes ---"));
    
    bool bAllValid = true;
    
    for (const FString& ClassName : CriticalClassNames)
    {
        bool bClassValid = ValidateClassLoading(ClassName);
        ClassLoadingStatus.Add(ClassName, bClassValid);
        
        if (!bClassValid)
        {
            bAllValid = false;
            FailedClassCount++;
            UE_LOG(LogTemp, Error, TEXT("CRITICAL CLASS FAILED: %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("✓ Critical class loaded: %s"), *ClassName);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Critical Classes Status: %d/%d loaded successfully"), 
           CriticalClassNames.Num() - FailedClassCount, CriticalClassNames.Num());
    
    return bAllValid;
}

bool UEng_CompilationOrchestrator_v3::ValidateClassLoading(const FString& ClassName)
{
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (LoadedClass)
    {
        // Verify class can create default object
        UObject* CDO = LoadedClass->GetDefaultObject();
        return CDO != nullptr;
    }
    
    return false;
}

bool UEng_CompilationOrchestrator_v3::CheckHeaderCppPairs()
{
    UE_LOG(LogTemp, Warning, TEXT("--- Checking Header/CPP Pairs ---"));
    
    FString SourcePath = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame/Core");
    TArray<FString> HeaderFiles;
    TArray<FString> OrphanedHeaders;
    
    // Find all header files
    IFileManager& FileManager = IFileManager::Get();
    FileManager.FindFiles(HeaderFiles, *SourcePath, TEXT("*.h"));
    
    OrphanedHeaderCount = 0;
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString CppFile = HeaderFile;
        CppFile = CppFile.Replace(TEXT(".h"), TEXT(".cpp"));
        
        FString CppPath = SourcePath / CppFile;
        
        if (!FileManager.FileExists(*CppPath))
        {
            OrphanedHeaders.Add(HeaderFile);
            OrphanedHeaderCount++;
            UE_LOG(LogTemp, Warning, TEXT("ORPHANED HEADER: %s"), *HeaderFile);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d orphaned headers out of %d total headers"), 
           OrphanedHeaderCount, HeaderFiles.Num());
    
    return OrphanedHeaderCount == 0;
}

bool UEng_CompilationOrchestrator_v3::ValidateIncludeStructure()
{
    UE_LOG(LogTemp, Warning, TEXT("--- Validating Include Structure ---"));
    
    // This is a simplified validation - in a full implementation,
    // we would parse source files and validate include dependencies
    bool bIncludesValid = true;
    
    // Check for common include issues
    if (!ValidateUPropertyUsage())
    {
        bIncludesValid = false;
        UE_LOG(LogTemp, Error, TEXT("UPROPERTY validation failed"));
    }
    
    if (!ValidateUFunctionImplementations())
    {
        bIncludesValid = false;
        UE_LOG(LogTemp, Error, TEXT("UFUNCTION implementation validation failed"));
    }
    
    if (!CheckGeneratedBodyMacros())
    {
        bIncludesValid = false;
        UE_LOG(LogTemp, Error, TEXT("GENERATED_BODY macro validation failed"));
    }
    
    return bIncludesValid;
}

bool UEng_CompilationOrchestrator_v3::FixOrphanedHeaders()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FIXING ORPHANED HEADERS ==="));
    
    // This would implement automatic stub generation for orphaned headers
    // For now, we just report the issue
    if (OrphanedHeaderCount > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Found %d orphaned headers - manual intervention required"), 
               OrphanedHeaderCount);
        return false;
    }
    
    return true;
}

int32 UEng_CompilationOrchestrator_v3::GetOrphanedHeaderCount()
{
    return OrphanedHeaderCount;
}

bool UEng_CompilationOrchestrator_v3::EnforceArchitecturalStandards()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENFORCING ARCHITECTURAL STANDARDS ==="));
    
    bool bStandardsEnforced = true;
    
    // Validate module dependencies
    if (!ValidateModuleDependencies())
    {
        bStandardsEnforced = false;
        UE_LOG(LogTemp, Error, TEXT("Module dependency validation failed"));
    }
    
    // Additional architectural validations would go here
    
    return bStandardsEnforced;
}

bool UEng_CompilationOrchestrator_v3::ValidateModuleDependencies()
{
    // Simplified module dependency validation
    // In a full implementation, this would check Build.cs files
    return true;
}

FString UEng_CompilationOrchestrator_v3::GenerateCompilationReport()
{
    FString Report;
    Report += TEXT("=== ENGINE ARCHITECT COMPILATION REPORT ===\n");
    Report += FString::Printf(TEXT("Compilation Health: %s\n"), 
                              bCompilationHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
    Report += FString::Printf(TEXT("Critical Classes: %d/%d loaded\n"), 
                              CriticalClassNames.Num() - FailedClassCount, CriticalClassNames.Num());
    Report += FString::Printf(TEXT("Orphaned Headers: %d\n"), OrphanedHeaderCount);
    Report += FString::Printf(TEXT("Failed Classes: %d\n"), FailedClassCount);
    
    Report += TEXT("\n--- Class Loading Status ---\n");
    for (const auto& ClassStatus : ClassLoadingStatus)
    {
        Report += FString::Printf(TEXT("%s: %s\n"), 
                                  *ClassStatus.Key, 
                                  ClassStatus.Value ? TEXT("LOADED") : TEXT("FAILED"));
    }
    
    return Report;
}

bool UEng_CompilationOrchestrator_v3::IsCompilationHealthy()
{
    return bCompilationHealthy;
}

void UEng_CompilationOrchestrator_v3::UpdateCompilationMetrics()
{
    // Update internal metrics for monitoring
    UE_LOG(LogTemp, Warning, TEXT("=== COMPILATION METRICS UPDATE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Critical Classes: %d"), CriticalClassNames.Num());
    UE_LOG(LogTemp, Warning, TEXT("Failed Class Count: %d"), FailedClassCount);
    UE_LOG(LogTemp, Warning, TEXT("Orphaned Header Count: %d"), OrphanedHeaderCount);
    UE_LOG(LogTemp, Warning, TEXT("Overall Health: %s"), 
           bCompilationHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
}

bool UEng_CompilationOrchestrator_v3::ValidateUPropertyUsage()
{
    // Simplified validation - would check for proper UPROPERTY usage
    return true;
}

bool UEng_CompilationOrchestrator_v3::ValidateUFunctionImplementations()
{
    // Simplified validation - would check UFUNCTION implementations exist
    return true;
}

bool UEng_CompilationOrchestrator_v3::CheckGeneratedBodyMacros()
{
    // Simplified validation - would check GENERATED_BODY macro usage
    return true;
}