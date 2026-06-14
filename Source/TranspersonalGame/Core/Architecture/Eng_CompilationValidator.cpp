#include "Eng_CompilationValidator.h"
#include "Engine/Engine.h"
#include "UObject/Class.h"

void UEng_CompilationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("CompilationValidator: Initializing class validation system"));
    
    // Auto-validate on startup
    ValidateAllClasses();
}

void UEng_CompilationValidator::ValidateAllClasses()
{
    ValidationResults.Empty();
    
    TArray<FString> KnownClasses = GetKnownClasses();
    
    UE_LOG(LogTemp, Warning, TEXT("CompilationValidator: Validating %d classes"), KnownClasses.Num());
    
    for (const FString& ClassName : KnownClasses)
    {
        FEng_ClassValidation Result = ValidateClass(ClassName);
        ValidationResults.Add(Result);
    }
    
    GenerateValidationReport();
}

FEng_ClassValidation UEng_CompilationValidator::ValidateClass(const FString& ClassName)
{
    FEng_ClassValidation Validation;
    Validation.ClassName = ClassName;
    
    try
    {
        // Try to load the class
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (LoadedClass)
        {
            Validation.bIsLoaded = true;
            
            // Check if CDO can be created
            UObject* CDO = LoadedClass->GetDefaultObject();
            if (CDO)
            {
                Validation.bHasValidCDO = true;
            }
            
            // Count properties and functions
            ValidateClassProperties(LoadedClass, Validation);
            ValidateClassFunctions(LoadedClass, Validation);
            
            UE_LOG(LogTemp, Log, TEXT("CompilationValidator: %s - VALID (%d props, %d funcs)"), 
                   *ClassName, Validation.PropertyCount, Validation.FunctionCount);
        }
        else
        {
            Validation.ErrorMessage = TEXT("Class not found or failed to load");
            UE_LOG(LogTemp, Warning, TEXT("CompilationValidator: %s - FAILED TO LOAD"), *ClassName);
        }
    }
    catch (...)
    {
        Validation.ErrorMessage = TEXT("Exception during validation");
        UE_LOG(LogTemp, Error, TEXT("CompilationValidator: %s - EXCEPTION"), *ClassName);
    }
    
    return Validation;
}

TArray<FEng_ClassValidation> UEng_CompilationValidator::GetValidationResults() const
{
    return ValidationResults;
}

bool UEng_CompilationValidator::AreAllClassesValid() const
{
    for (const FEng_ClassValidation& Result : ValidationResults)
    {
        if (!Result.bIsLoaded || !Result.bHasValidCDO)
        {
            return false;
        }
    }
    return true;
}

void UEng_CompilationValidator::GenerateValidationReport()
{
    int32 ValidCount = 0;
    int32 InvalidCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("=== COMPILATION VALIDATION REPORT ==="));
    
    for (const FEng_ClassValidation& Result : ValidationResults)
    {
        if (Result.bIsLoaded && Result.bHasValidCDO)
        {
            ValidCount++;
            UE_LOG(LogTemp, Log, TEXT("  [✓] %s - %d props, %d funcs"), 
                   *Result.ClassName, Result.PropertyCount, Result.FunctionCount);
        }
        else
        {
            InvalidCount++;
            UE_LOG(LogTemp, Error, TEXT("  [✗] %s - %s"), 
                   *Result.ClassName, *Result.ErrorMessage);
        }
    }
    
    float SuccessRate = ValidationResults.Num() > 0 ? 
        (float)ValidCount / ValidationResults.Num() * 100.0f : 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("VALIDATION SUMMARY: %d valid, %d invalid (%.1f%% success)"), 
           ValidCount, InvalidCount, SuccessRate);
}

TArray<FString> UEng_CompilationValidator::GetKnownClasses() const
{
    return {
        TEXT("Eng_SystemRegistry"),
        TEXT("Eng_ModuleManager"),
        TEXT("Eng_CompilationValidator"),
        TEXT("Eng_BiomeSystem"),
        TEXT("Eng_WorldArchitecture"),
        TEXT("Eng_PerformanceManager"),
        TEXT("ProductionDirector"),
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("ProceduralWorldManager"),
        TEXT("BuildIntegrationManager")
    };
}

bool UEng_CompilationValidator::ValidateClassProperties(UClass* Class, FEng_ClassValidation& Validation)
{
    if (!Class)
    {
        return false;
    }
    
    int32 PropertyCount = 0;
    
    for (TFieldIterator<FProperty> PropIt(Class); PropIt; ++PropIt)
    {
        FProperty* Property = *PropIt;
        if (Property && Property->HasAnyPropertyFlags(CPF_BlueprintVisible | CPF_Edit))
        {
            PropertyCount++;
        }
    }
    
    Validation.PropertyCount = PropertyCount;
    return true;
}

bool UEng_CompilationValidator::ValidateClassFunctions(UClass* Class, FEng_ClassValidation& Validation)
{
    if (!Class)
    {
        return false;
    }
    
    int32 FunctionCount = 0;
    
    for (TFieldIterator<UFunction> FuncIt(Class); FuncIt; ++FuncIt)
    {
        UFunction* Function = *FuncIt;
        if (Function && Function->HasAnyFunctionFlags(FUNC_BlueprintCallable))
        {
            FunctionCount++;
        }
    }
    
    Validation.FunctionCount = FunctionCount;
    return true;
}