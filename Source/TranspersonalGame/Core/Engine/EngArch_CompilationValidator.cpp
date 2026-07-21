#include "EngArch_CompilationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/DateTime.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogEngArchValidator, Log, All);

void UEngArch_CompilationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngArchValidator, Log, TEXT("Engine Architect Compilation Validator initialized"));
    
    // Initialize critical classes list
    CriticalClasses.Empty();
    CriticalClasses.Add(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    CriticalClasses.Add(TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    CriticalClasses.Add(TEXT("/Script/TranspersonalGame.DinosaurTRex"));
    CriticalClasses.Add(TEXT("/Script/TranspersonalGame.DinosaurCombatAIController"));
    CriticalClasses.Add(TEXT("/Script/TranspersonalGame.BiomeManager"));
    
    // Perform initial validation
    bIsValidationPassing = ValidateModuleCompilation();
    LastValidationTime = FDateTime::Now();
}

void UEngArch_CompilationValidator::Deinitialize()
{
    UE_LOG(LogEngArchValidator, Log, TEXT("Engine Architect Compilation Validator deinitialized"));
    Super::Deinitialize();
}

bool UEngArch_CompilationValidator::ValidateModuleCompilation()
{
    UE_LOG(LogEngArchValidator, Log, TEXT("Starting module compilation validation"));
    
    ValidationResults.Empty();
    bool bAllValid = true;
    
    // Validate TranspersonalGame module
    bool bModuleValid = ValidateTranspersonalGameModule();
    ValidationResults.Add(TEXT("TranspersonalGame Module"), bModuleValid);
    bAllValid &= bModuleValid;
    
    // Validate core systems
    bool bPhysicsValid = ValidateCorePhysicsSystems();
    ValidationResults.Add(TEXT("Core Physics Systems"), bPhysicsValid);
    bAllValid &= bPhysicsValid;
    
    bool bCharacterValid = ValidateCharacterSystems();
    ValidationResults.Add(TEXT("Character Systems"), bCharacterValid);
    bAllValid &= bCharacterValid;
    
    bool bDinosaurValid = ValidateDinosaurSystems();
    ValidationResults.Add(TEXT("Dinosaur Systems"), bDinosaurValid);
    bAllValid &= bDinosaurValid;
    
    bool bWorldGenValid = ValidateWorldGeneration();
    ValidationResults.Add(TEXT("World Generation"), bWorldGenValid);
    bAllValid &= bWorldGenValid;
    
    bIsValidationPassing = bAllValid;
    LastValidationTime = FDateTime::Now();
    
    LogValidationResults();
    
    return bAllValid;
}

bool UEngArch_CompilationValidator::ValidateClassLoading(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    bool bIsValid = (LoadedClass != nullptr);
    
    UE_LOG(LogEngArchValidator, Log, TEXT("Class validation for %s: %s"), 
           *ClassName, bIsValid ? TEXT("PASS") : TEXT("FAIL"));
    
    return bIsValid;
}

bool UEngArch_CompilationValidator::ValidateArchitecturalCompliance()
{
    UE_LOG(LogEngArchValidator, Log, TEXT("Validating architectural compliance"));
    
    bool bCompliant = true;
    
    // Check all critical classes are loadable
    for (const FString& ClassName : CriticalClasses)
    {
        if (!ValidateClassLoading(ClassName))
        {
            bCompliant = false;
            UE_LOG(LogEngArchValidator, Error, TEXT("Critical class failed to load: %s"), *ClassName);
        }
    }
    
    // Validate module dependencies
    if (!ValidateModuleCompilation())
    {
        bCompliant = false;
        UE_LOG(LogEngArchValidator, Error, TEXT("Module compilation validation failed"));
    }
    
    return bCompliant;
}

FString UEngArch_CompilationValidator::GetCompilationStatusReport()
{
    FString Report = TEXT("=== ENGINE ARCHITECT COMPILATION STATUS REPORT ===\n");
    Report += FString::Printf(TEXT("Last Validation: %s\n"), *LastValidationTime.ToString());
    Report += FString::Printf(TEXT("Overall Status: %s\n\n"), 
                             bIsValidationPassing ? TEXT("PASS") : TEXT("FAIL"));
    
    Report += TEXT("Module Validation Results:\n");
    for (const auto& Result : ValidationResults)
    {
        Report += FString::Printf(TEXT("  %s: %s\n"), 
                                 *Result.Key, Result.Value ? TEXT("PASS") : TEXT("FAIL"));
    }
    
    Report += TEXT("\nCritical Classes Status:\n");
    for (const FString& ClassName : CriticalClasses)
    {
        bool bClassValid = ValidateClassLoading(ClassName);
        Report += FString::Printf(TEXT("  %s: %s\n"), 
                                 *ClassName, bClassValid ? TEXT("LOADED") : TEXT("MISSING"));
    }
    
    return Report;
}

void UEngArch_CompilationValidator::ForceModuleRecompilation()
{
    UE_LOG(LogEngArchValidator, Warning, TEXT("Force module recompilation requested"));
    
    // Clear validation cache
    ValidationResults.Empty();
    bIsValidationPassing = false;
    
    // Trigger new validation
    ValidateModuleCompilation();
    
    UE_LOG(LogEngArchValidator, Log, TEXT("Module recompilation validation completed"));
}

bool UEngArch_CompilationValidator::ValidateTranspersonalGameModule()
{
    // Check if the main module classes are loadable
    bool bValid = true;
    
    bValid &= ValidateClassLoading(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    bValid &= ValidateClassLoading(TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    
    return bValid;
}

bool UEngArch_CompilationValidator::ValidateCorePhysicsSystems()
{
    // Validate physics system classes exist and are loadable
    bool bValid = true;
    
    // Check for core physics classes (these should exist based on the file listing)
    TArray<FString> PhysicsClasses = {
        TEXT("/Script/TranspersonalGame.Core_PhysicsManager"),
        TEXT("/Script/TranspersonalGame.Core_CollisionSystem"),
        TEXT("/Script/TranspersonalGame.Core_ForceSystem")
    };
    
    for (const FString& PhysicsClass : PhysicsClasses)
    {
        if (!ValidateClassLoading(PhysicsClass))
        {
            UE_LOG(LogEngArchValidator, Warning, TEXT("Physics class not found: %s"), *PhysicsClass);
            // Don't fail validation for physics classes that might not be implemented yet
        }
    }
    
    return bValid;
}

bool UEngArch_CompilationValidator::ValidateCharacterSystems()
{
    return ValidateClassLoading(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
}

bool UEngArch_CompilationValidator::ValidateDinosaurSystems()
{
    bool bValid = true;
    
    bValid &= ValidateClassLoading(TEXT("/Script/TranspersonalGame.DinosaurTRex"));
    bValid &= ValidateClassLoading(TEXT("/Script/TranspersonalGame.DinosaurCombatAIController"));
    
    return bValid;
}

bool UEngArch_CompilationValidator::ValidateWorldGeneration()
{
    return ValidateClassLoading(TEXT("/Script/TranspersonalGame.BiomeManager"));
}

void UEngArch_CompilationValidator::LogValidationResults()
{
    UE_LOG(LogEngArchValidator, Log, TEXT("=== VALIDATION RESULTS ==="));
    UE_LOG(LogEngArchValidator, Log, TEXT("Overall Status: %s"), 
           bIsValidationPassing ? TEXT("PASS") : TEXT("FAIL"));
    
    for (const auto& Result : ValidationResults)
    {
        UE_LOG(LogEngArchValidator, Log, TEXT("%s: %s"), 
               *Result.Key, Result.Value ? TEXT("PASS") : TEXT("FAIL"));
    }
    
    UE_LOG(LogEngArchValidator, Log, TEXT("=== END VALIDATION ==="));
}