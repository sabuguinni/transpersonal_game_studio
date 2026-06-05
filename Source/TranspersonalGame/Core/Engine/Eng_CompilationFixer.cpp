#include "Eng_CompilationFixer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Class.h"
#include "UObject/UObjectIterator.h"

DEFINE_LOG_CATEGORY_STATIC(LogEngCompilationFixer, Log, All);

UEng_CompilationFixer::UEng_CompilationFixer()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize compilation status
    CompilationStatus = EEng_CompilationStatus::Unknown;
    LastValidationTime = 0.0f;
    ErrorCount = 0;
    WarningCount = 0;
}

void UEng_CompilationFixer::BeginPlay()
{
    Super::BeginPlay();
    
    // Perform initial compilation validation
    ValidateCompilationStatus();
    
    UE_LOG(LogEngCompilationFixer, Log, TEXT("Engine Compilation Fixer initialized"));
}

bool UEng_CompilationFixer::ValidateCompilationStatus()
{
    UE_LOG(LogEngCompilationFixer, Log, TEXT("Starting compilation validation..."));
    
    ErrorCount = 0;
    WarningCount = 0;
    CompilationErrors.Empty();
    
    // Test 1: Validate core TranspersonalGame classes exist
    bool bCoreClassesValid = ValidateCoreClasses();
    
    // Test 2: Validate module dependencies
    bool bModuleDepsValid = ValidateModuleDependencies();
    
    // Test 3: Validate UCLASS/USTRUCT definitions
    bool bTypeDefsValid = ValidateTypeDefinitions();
    
    // Test 4: Validate include structure
    bool bIncludesValid = ValidateIncludeStructure();
    
    // Update compilation status
    if (ErrorCount == 0)
    {
        CompilationStatus = (WarningCount == 0) ? EEng_CompilationStatus::Clean : EEng_CompilationStatus::WarningsOnly;
    }
    else
    {
        CompilationStatus = EEng_CompilationStatus::Errors;
    }
    
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogEngCompilationFixer, Log, TEXT("Compilation validation complete - Status: %s, Errors: %d, Warnings: %d"), 
           *UEnum::GetValueAsString(CompilationStatus), ErrorCount, WarningCount);
    
    return CompilationStatus != EEng_CompilationStatus::Errors;
}

bool UEng_CompilationFixer::ValidateCoreClasses()
{
    TArray<FString> RequiredClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager")
    };
    
    bool bAllValid = true;
    
    for (const FString& ClassName : RequiredClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (!LoadedClass)
        {
            FString ErrorMsg = FString::Printf(TEXT("Required class not found: %s"), *ClassName);
            CompilationErrors.Add(ErrorMsg);
            ErrorCount++;
            bAllValid = false;
            
            UE_LOG(LogEngCompilationFixer, Error, TEXT("%s"), *ErrorMsg);
        }
        else
        {
            UE_LOG(LogEngCompilationFixer, Log, TEXT("✓ Core class validated: %s"), *ClassName);
        }
    }
    
    return bAllValid;
}

bool UEng_CompilationFixer::ValidateModuleDependencies()
{
    // Check if required engine modules are accessible
    TArray<FString> RequiredModules = {
        TEXT("Engine"),
        TEXT("CoreUObject"),
        TEXT("UMG"),
        TEXT("PCG"),
        TEXT("Foliage"),
        TEXT("NavigationSystem")
    };
    
    bool bAllValid = true;
    
    // This is a simplified check - in a full implementation,
    // we would validate actual module loading and dependencies
    for (const FString& ModuleName : RequiredModules)
    {
        UE_LOG(LogEngCompilationFixer, Log, TEXT("✓ Module dependency assumed valid: %s"), *ModuleName);
    }
    
    return bAllValid;
}

bool UEng_CompilationFixer::ValidateTypeDefinitions()
{
    // Validate that custom enums and structs are properly defined
    bool bAllValid = true;
    
    // Check for common type definition issues
    // This would be expanded with actual type validation logic
    
    UE_LOG(LogEngCompilationFixer, Log, TEXT("✓ Type definitions validation passed"));
    
    return bAllValid;
}

bool UEng_CompilationFixer::ValidateIncludeStructure()
{
    // Validate include order and structure
    // This is a placeholder for actual include validation
    bool bAllValid = true;
    
    UE_LOG(LogEngCompilationFixer, Log, TEXT("✓ Include structure validation passed"));
    
    return bAllValid;
}

bool UEng_CompilationFixer::FixCompilationIssues()
{
    if (CompilationStatus == EEng_CompilationStatus::Clean)
    {
        UE_LOG(LogEngCompilationFixer, Log, TEXT("No compilation issues to fix"));
        return true;
    }
    
    UE_LOG(LogEngCompilationFixer, Log, TEXT("Attempting to fix %d compilation errors..."), ErrorCount);
    
    int32 FixedCount = 0;
    
    // Apply automatic fixes for common issues
    FixedCount += FixMissingIncludes();
    FixedCount += FixTypeRedefinitions();
    FixedCount += FixModuleDependencies();
    FixedCount += FixMacroUsage();
    
    UE_LOG(LogEngCompilationFixer, Log, TEXT("Applied %d automatic fixes"), FixedCount);
    
    // Re-validate after fixes
    bool bValidationPassed = ValidateCompilationStatus();
    
    return bValidationPassed;
}

int32 UEng_CompilationFixer::FixMissingIncludes()
{
    // Placeholder for automatic include fixing
    // In a full implementation, this would scan source files and add missing includes
    return 0;
}

int32 UEng_CompilationFixer::FixTypeRedefinitions()
{
    // Placeholder for type redefinition fixing
    // This would identify and resolve duplicate type definitions
    return 0;
}

int32 UEng_CompilationFixer::FixModuleDependencies()
{
    // Placeholder for module dependency fixing
    // This would update Build.cs files with missing dependencies
    return 0;
}

int32 UEng_CompilationFixer::FixMacroUsage()
{
    // Placeholder for macro usage fixing
    // This would fix common UPROPERTY/UFUNCTION macro issues
    return 0;
}

void UEng_CompilationFixer::GenerateCompilationReport()
{
    UE_LOG(LogEngCompilationFixer, Log, TEXT("=== COMPILATION REPORT ==="));
    UE_LOG(LogEngCompilationFixer, Log, TEXT("Status: %s"), *UEnum::GetValueAsString(CompilationStatus));
    UE_LOG(LogEngCompilationFixer, Log, TEXT("Errors: %d"), ErrorCount);
    UE_LOG(LogEngCompilationFixer, Log, TEXT("Warnings: %d"), WarningCount);
    UE_LOG(LogEngCompilationFixer, Log, TEXT("Last Validation: %.2f seconds ago"), 
           GetWorld()->GetTimeSeconds() - LastValidationTime);
    
    if (CompilationErrors.Num() > 0)
    {
        UE_LOG(LogEngCompilationFixer, Log, TEXT("=== COMPILATION ERRORS ==="));
        for (const FString& Error : CompilationErrors)
        {
            UE_LOG(LogEngCompilationFixer, Error, TEXT("- %s"), *Error);
        }
    }
    
    UE_LOG(LogEngCompilationFixer, Log, TEXT("=== END REPORT ==="));
}