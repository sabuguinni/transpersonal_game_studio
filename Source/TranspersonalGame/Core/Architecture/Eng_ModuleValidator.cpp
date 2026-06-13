#include "Eng_ModuleValidator.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"

void UEng_ModuleValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Warning, TEXT("Module Validator initialized"));
}

FEng_ModuleValidationResult UEng_ModuleValidator::ValidateModule(const FString& ModuleName)
{
    FEng_ModuleValidationResult Result;
    Result.ModuleName = ModuleName;
    Result.ValidationTime = FPlatformTime::Seconds();
    
    if (ModuleName == TEXT("TranspersonalGame"))
    {
        ValidateTranspersonalGameModule(Result);
    }
    else if (ModuleName == TEXT("EngineArchitecture"))
    {
        ValidateEngineArchitecture(Result);
    }
    else
    {
        Result.ValidationErrors.Add(FString::Printf(TEXT("Unknown module: %s"), *ModuleName));
        Result.bIsValid = false;
    }
    
    Result.ValidationTime = FPlatformTime::Seconds() - Result.ValidationTime;
    return Result;
}

TArray<FEng_ModuleValidationResult> UEng_ModuleValidator::ValidateAllModules()
{
    TArray<FEng_ModuleValidationResult> Results;
    
    Results.Add(ValidateModule(TEXT("TranspersonalGame")));
    Results.Add(ValidateModule(TEXT("EngineArchitecture")));
    
    UE_LOG(LogTemp, Warning, TEXT("Validated %d modules"), Results.Num());
    return Results;
}

void UEng_ModuleValidator::RunCompilationTest()
{
    UE_LOG(LogTemp, Warning, TEXT("=== COMPILATION TEST ==="));
    
    // Test core classes
    TArray<FString> ClassesToTest = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.Eng_SystemRegistry"),
        TEXT("/Script/TranspersonalGame.Eng_PerformanceMonitor"),
        TEXT("/Script/TranspersonalGame.Eng_ModuleValidator")
    };
    
    int32 PassedTests = 0;
    for (const FString& ClassName : ClassesToTest)
    {
        if (CheckClassAvailability(ClassName))
        {
            PassedTests++;
            UE_LOG(LogTemp, Log, TEXT("PASS: %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FAIL: %s"), *ClassName);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Compilation test: %d/%d classes passed"), PassedTests, ClassesToTest.Num());
}

bool UEng_ModuleValidator::CheckClassAvailability(const FString& ClassName)
{
    try
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        return LoadedClass != nullptr;
    }
    catch (...)
    {
        return false;
    }
}

void UEng_ModuleValidator::ValidateTranspersonalGameModule(FEng_ModuleValidationResult& Result)
{
    Result.bIsValid = true;
    
    // Check core game classes
    if (!CheckClassAvailability(TEXT("/Script/TranspersonalGame.TranspersonalGameState")))
    {
        Result.ValidationErrors.Add(TEXT("TranspersonalGameState class not found"));
        Result.bIsValid = false;
    }
    
    if (!CheckClassAvailability(TEXT("/Script/TranspersonalGame.TranspersonalCharacter")))
    {
        Result.ValidationErrors.Add(TEXT("TranspersonalCharacter class not found"));
        Result.bIsValid = false;
    }
    
    // Check if module is loaded
    if (!FModuleManager::Get().IsModuleLoaded(TEXT("TranspersonalGame")))
    {
        Result.ValidationWarnings.Add(TEXT("TranspersonalGame module not loaded"));
    }
    
    if (Result.bIsValid)
    {
        UE_LOG(LogTemp, Log, TEXT("TranspersonalGame module validation: PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TranspersonalGame module validation: FAILED"));
    }
}

void UEng_ModuleValidator::ValidateEngineArchitecture(FEng_ModuleValidationResult& Result)
{
    Result.bIsValid = true;
    
    // Check architecture classes
    if (!CheckClassAvailability(TEXT("/Script/TranspersonalGame.Eng_SystemRegistry")))
    {
        Result.ValidationErrors.Add(TEXT("Eng_SystemRegistry class not found"));
        Result.bIsValid = false;
    }
    
    if (!CheckClassAvailability(TEXT("/Script/TranspersonalGame.Eng_PerformanceMonitor")))
    {
        Result.ValidationErrors.Add(TEXT("Eng_PerformanceMonitor class not found"));
        Result.bIsValid = false;
    }
    
    CheckRequiredClasses(Result);
    
    if (Result.bIsValid)
    {
        UE_LOG(LogTemp, Log, TEXT("Engine Architecture validation: PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architecture validation: FAILED"));
    }
}

void UEng_ModuleValidator::CheckRequiredClasses(FEng_ModuleValidationResult& Result)
{
    // Verify that all required UE5 base classes are available
    TArray<FString> RequiredClasses = {
        TEXT("/Script/Engine.GameInstanceSubsystem"),
        TEXT("/Script/Engine.Actor"),
        TEXT("/Script/Engine.GameModeBase"),
        TEXT("/Script/Engine.Character")
    };
    
    for (const FString& ClassName : RequiredClasses)
    {
        if (!CheckClassAvailability(ClassName))
        {
            Result.ValidationErrors.Add(FString::Printf(TEXT("Required UE5 class not found: %s"), *ClassName));
            Result.bIsValid = false;
        }
    }
}