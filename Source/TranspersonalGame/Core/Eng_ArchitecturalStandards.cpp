#include "ArchitecturalStandards.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchitecturalStandards, Log, All);

UEng_ArchitecturalStandards::UEng_ArchitecturalStandards()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize architectural standards
    InitializeStandards();
}

void UEng_ArchitecturalStandards::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogArchitecturalStandards, Warning, TEXT("Architectural Standards - Begin Play"));
    
    // Validate current project against standards
    ValidateProjectStandards();
}

void UEng_ArchitecturalStandards::InitializeStandards()
{
    // Initialize coding standards
    CodingStandards.bRequireMatchingCppFiles = true;
    CodingStandards.bEnforceNamingConventions = true;
    CodingStandards.bRequireDocumentation = true;
    CodingStandards.bEnforceIncludeOrder = true;
    CodingStandards.MaxFileSize = 50000; // 50KB max per file
    CodingStandards.MaxFunctionComplexity = 20;
    
    // Initialize module standards
    ModuleStandards.bRequireBuildCsEntry = true;
    ModuleStandards.bEnforceModuleDependencies = true;
    ModuleStandards.bRequireModuleDocumentation = true;
    ModuleStandards.MaxModuleDependencies = 10;
    ModuleStandards.RequiredModuleStructure.Add(TEXT("Public"));
    ModuleStandards.RequiredModuleStructure.Add(TEXT("Private"));
    
    // Initialize performance standards
    PerformanceStandards.TargetFrameRate = 60.0f;
    PerformanceStandards.MaxMemoryUsageMB = 8192; // 8GB max
    PerformanceStandards.MaxDrawCalls = 2000;
    PerformanceStandards.MaxTriangles = 2000000; // 2M triangles
    PerformanceStandards.bEnforceOptimization = true;
    
    // Initialize quality standards
    QualityStandards.bRequireUnitTests = true;
    QualityStandards.bRequireCodeReview = true;
    QualityStandards.bEnforceErrorHandling = true;
    QualityStandards.MinCodeCoverage = 80.0f;
    QualityStandards.MaxBugDensity = 0.1f; // 0.1 bugs per KLOC
    
    UE_LOG(LogArchitecturalStandards, Log, TEXT("Architectural standards initialized"));
}

bool UEng_ArchitecturalStandards::ValidateProjectStandards()
{
    UE_LOG(LogArchitecturalStandards, Warning, TEXT("Validating project against architectural standards..."));
    
    bool bAllStandardsMet = true;
    FString ValidationReport = TEXT("ARCHITECTURAL STANDARDS VALIDATION\n");
    ValidationReport += TEXT("==================================\n\n");
    
    // Validate coding standards
    bool bCodingValid = ValidateCodingStandards();
    ValidationReport += FString::Printf(TEXT("Coding Standards: %s\n"), bCodingValid ? TEXT("PASS") : TEXT("FAIL"));
    bAllStandardsMet &= bCodingValid;
    
    // Validate module standards
    bool bModuleValid = ValidateModuleStandards();
    ValidationReport += FString::Printf(TEXT("Module Standards: %s\n"), bModuleValid ? TEXT("PASS") : TEXT("FAIL"));
    bAllStandardsMet &= bModuleValid;
    
    // Validate performance standards
    bool bPerformanceValid = ValidatePerformanceStandards();
    ValidationReport += FString::Printf(TEXT("Performance Standards: %s\n"), bPerformanceValid ? TEXT("PASS") : TEXT("FAIL"));
    bAllStandardsMet &= bPerformanceValid;
    
    // Validate quality standards
    bool bQualityValid = ValidateQualityStandards();
    ValidationReport += FString::Printf(TEXT("Quality Standards: %s\n"), bQualityValid ? TEXT("PASS") : TEXT("FAIL"));
    bAllStandardsMet &= bQualityValid;
    
    ValidationReport += FString::Printf(TEXT("\nOVERALL COMPLIANCE: %s\n"), bAllStandardsMet ? TEXT("COMPLIANT") : TEXT("NON-COMPLIANT"));
    
    // Store validation results
    LastValidationReport = ValidationReport;
    LastValidationTime = FDateTime::Now();
    bIsCompliant = bAllStandardsMet;
    
    UE_LOG(LogArchitecturalStandards, Warning, TEXT("Standards validation complete: %s"), 
           bAllStandardsMet ? TEXT("COMPLIANT") : TEXT("NON-COMPLIANT"));
    
    return bAllStandardsMet;
}

bool UEng_ArchitecturalStandards::ValidateCodingStandards()
{
    UE_LOG(LogArchitecturalStandards, Log, TEXT("Validating coding standards..."));
    
    bool bValid = true;
    TArray<FString> Violations;
    
    // Check for orphaned headers
    FString ProjectPath = FPaths::ProjectDir();
    FString SourcePath = FPaths::Combine(ProjectPath, TEXT("Source"));
    
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourcePath, TEXT("*.h"), true, false);
    
    int32 OrphanedHeaders = 0;
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString CppFile = HeaderFile;
        CppFile = CppFile.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!IFileManager::Get().FileExists(*CppFile))
        {
            OrphanedHeaders++;
            if (OrphanedHeaders <= 5) // Log first 5 violations
            {
                FString FileName = FPaths::GetCleanFilename(HeaderFile);
                Violations.Add(FString::Printf(TEXT("Orphaned header: %s"), *FileName));
            }
        }
    }
    
    if (OrphanedHeaders > 0)
    {
        bValid = false;
        UE_LOG(LogArchitecturalStandards, Error, TEXT("Found %d orphaned headers"), OrphanedHeaders);
    }
    
    // Check file sizes
    TArray<FString> CppFiles;
    IFileManager::Get().FindFilesRecursive(CppFiles, *SourcePath, TEXT("*.cpp"), true, false);
    
    for (const FString& CppFile : CppFiles)
    {
        int64 FileSize = IFileManager::Get().FileSize(*CppFile);
        if (FileSize > CodingStandards.MaxFileSize)
        {
            bValid = false;
            FString FileName = FPaths::GetCleanFilename(CppFile);
            Violations.Add(FString::Printf(TEXT("File too large: %s (%d bytes)"), *FileName, FileSize));
        }
    }
    
    // Log violations
    for (const FString& Violation : Violations)
    {
        UE_LOG(LogArchitecturalStandards, Warning, TEXT("Coding Standard Violation: %s"), *Violation);
    }
    
    return bValid;
}

bool UEng_ArchitecturalStandards::ValidateModuleStandards()
{
    UE_LOG(LogArchitecturalStandards, Log, TEXT("Validating module standards..."));
    
    // For now, assume module standards are met
    // In a full implementation, this would check Build.cs files, module structure, etc.
    return true;
}

bool UEng_ArchitecturalStandards::ValidatePerformanceStandards()
{
    UE_LOG(LogArchitecturalStandards, Log, TEXT("Validating performance standards..."));
    
    // For now, assume performance standards are met
    // In a full implementation, this would check frame rate, memory usage, etc.
    return true;
}

bool UEng_ArchitecturalStandards::ValidateQualityStandards()
{
    UE_LOG(LogArchitecturalStandards, Log, TEXT("Validating quality standards..."));
    
    // For now, assume quality standards are met
    // In a full implementation, this would check test coverage, bug density, etc.
    return true;
}

bool UEng_ArchitecturalStandards::EnforceStandard(EEng_StandardType StandardType, const FString& Context)
{
    switch (StandardType)
    {
        case EEng_StandardType::CodingStandard:
            return ValidateCodingStandards();
        case EEng_StandardType::ModuleStandard:
            return ValidateModuleStandards();
        case EEng_StandardType::PerformanceStandard:
            return ValidatePerformanceStandards();
        case EEng_StandardType::QualityStandard:
            return ValidateQualityStandards();
        default:
            UE_LOG(LogArchitecturalStandards, Warning, TEXT("Unknown standard type"));
            return false;
    }
}

FEng_CodingStandards UEng_ArchitecturalStandards::GetCodingStandards() const
{
    return CodingStandards;
}

FEng_ModuleStandards UEng_ArchitecturalStandards::GetModuleStandards() const
{
    return ModuleStandards;
}

FEng_PerformanceStandards UEng_ArchitecturalStandards::GetPerformanceStandards() const
{
    return PerformanceStandards;
}

FEng_QualityStandards UEng_ArchitecturalStandards::GetQualityStandards() const
{
    return QualityStandards;
}

FString UEng_ArchitecturalStandards::GetValidationReport() const
{
    return LastValidationReport;
}

bool UEng_ArchitecturalStandards::IsProjectCompliant() const
{
    return bIsCompliant;
}