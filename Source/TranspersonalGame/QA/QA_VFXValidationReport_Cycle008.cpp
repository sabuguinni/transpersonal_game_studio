#include "QA_VFXValidationReport_Cycle008.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"

UQA_VFXValidationReport_Cycle008::UQA_VFXValidationReport_Cycle008()
{
    // Initialize validation report
    ReportTimestamp = FDateTime::Now();
    ValidationStatus = EQA_ValidationStatus::InProgress;
    CriticalIssuesFound = 0;
    WarningsFound = 0;
    TestsPassed = 0;
    TestsFailed = 0;
}

void UQA_VFXValidationReport_Cycle008::BeginDestroy()
{
    Super::BeginDestroy();
}

void UQA_VFXValidationReport_Cycle008::InitializeValidation()
{
    ValidationStatus = EQA_ValidationStatus::InProgress;
    CriticalIssuesFound = 0;
    WarningsFound = 0;
    TestsPassed = 0;
    TestsFailed = 0;
    ValidationResults.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("QA VFX Validation Report Cycle 008 - Initialized"));
}

bool UQA_VFXValidationReport_Cycle008::ValidateVFXNiagaraLibrary()
{
    FQA_ValidationResult Result;
    Result.TestName = TEXT("VFX_NiagaraLibrary Class Loading");
    Result.Category = TEXT("VFX System");
    Result.Timestamp = FDateTime::Now();
    
    // Attempt to find VFX_NiagaraLibrary class
    UClass* VFXLibClass = FindObject<UClass>(ANY_PACKAGE, TEXT("VFX_NiagaraLibrary"));
    if (VFXLibClass)
    {
        Result.bPassed = true;
        Result.Message = TEXT("VFX_NiagaraLibrary class loaded successfully");
        TestsPassed++;
        UE_LOG(LogTemp, Log, TEXT("✓ VFX_NiagaraLibrary validation PASSED"));
    }
    else
    {
        Result.bPassed = false;
        Result.Message = TEXT("VFX_NiagaraLibrary class not found - compilation issue");
        TestsFailed++;
        CriticalIssuesFound++;
        UE_LOG(LogTemp, Warning, TEXT("✗ VFX_NiagaraLibrary validation FAILED"));
    }
    
    ValidationResults.Add(Result);
    return Result.bPassed;
}

bool UQA_VFXValidationReport_Cycle008::ValidateVFXEffectManager()
{
    FQA_ValidationResult Result;
    Result.TestName = TEXT("VFX_EffectManager Class Loading");
    Result.Category = TEXT("VFX System");
    Result.Timestamp = FDateTime::Now();
    
    // Attempt to find VFX_EffectManager class
    UClass* EffectMgrClass = FindObject<UClass>(ANY_PACKAGE, TEXT("VFX_EffectManager"));
    if (EffectMgrClass)
    {
        Result.bPassed = true;
        Result.Message = TEXT("VFX_EffectManager class loaded successfully");
        TestsPassed++;
        UE_LOG(LogTemp, Log, TEXT("✓ VFX_EffectManager validation PASSED"));
    }
    else
    {
        Result.bPassed = false;
        Result.Message = TEXT("VFX_EffectManager class not found - compilation issue");
        TestsFailed++;
        CriticalIssuesFound++;
        UE_LOG(LogTemp, Warning, TEXT("✗ VFX_EffectManager validation FAILED"));
    }
    
    ValidationResults.Add(Result);
    return Result.bPassed;
}

bool UQA_VFXValidationReport_Cycle008::ValidateCriticalSystems()
{
    bool bAllSystemsValid = true;
    
    // Validate TranspersonalCharacter
    FQA_ValidationResult CharResult;
    CharResult.TestName = TEXT("TranspersonalCharacter Class Loading");
    CharResult.Category = TEXT("Core System");
    CharResult.Timestamp = FDateTime::Now();
    
    UClass* CharClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    if (CharClass)
    {
        CharResult.bPassed = true;
        CharResult.Message = TEXT("TranspersonalCharacter class loaded successfully");
        TestsPassed++;
    }
    else
    {
        CharResult.bPassed = false;
        CharResult.Message = TEXT("TranspersonalCharacter class not found");
        TestsFailed++;
        CriticalIssuesFound++;
        bAllSystemsValid = false;
    }
    ValidationResults.Add(CharResult);
    
    // Validate TranspersonalGameState
    FQA_ValidationResult GameStateResult;
    GameStateResult.TestName = TEXT("TranspersonalGameState Class Loading");
    GameStateResult.Category = TEXT("Core System");
    GameStateResult.Timestamp = FDateTime::Now();
    
    UClass* GameStateClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameState"));
    if (GameStateClass)
    {
        GameStateResult.bPassed = true;
        GameStateResult.Message = TEXT("TranspersonalGameState class loaded successfully");
        TestsPassed++;
    }
    else
    {
        GameStateResult.bPassed = false;
        GameStateResult.Message = TEXT("TranspersonalGameState class not found");
        TestsFailed++;
        CriticalIssuesFound++;
        bAllSystemsValid = false;
    }
    ValidationResults.Add(GameStateResult);
    
    return bAllSystemsValid;
}

bool UQA_VFXValidationReport_Cycle008::ValidatePerformanceMetrics()
{
    FQA_ValidationResult Result;
    Result.TestName = TEXT("Performance Metrics Validation");
    Result.Category = TEXT("Performance");
    Result.Timestamp = FDateTime::Now();
    
    // Check world actor count
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (World)
    {
        int32 ActorCount = World->GetActorCount();
        if (ActorCount > 1000)
        {
            Result.bPassed = false;
            Result.Message = FString::Printf(TEXT("High actor count detected: %d (performance risk)"), ActorCount);
            WarningsFound++;
            TestsFailed++;
        }
        else
        {
            Result.bPassed = true;
            Result.Message = FString::Printf(TEXT("Actor count acceptable: %d"), ActorCount);
            TestsPassed++;
        }
    }
    else
    {
        Result.bPassed = false;
        Result.Message = TEXT("Could not access world for performance validation");
        TestsFailed++;
        CriticalIssuesFound++;
    }
    
    ValidationResults.Add(Result);
    return Result.bPassed;
}

void UQA_VFXValidationReport_Cycle008::FinalizeValidation()
{
    // Determine overall validation status
    if (CriticalIssuesFound > 0)
    {
        ValidationStatus = EQA_ValidationStatus::Failed;
    }
    else if (WarningsFound > 0)
    {
        ValidationStatus = EQA_ValidationStatus::PassedWithWarnings;
    }
    else
    {
        ValidationStatus = EQA_ValidationStatus::Passed;
    }
    
    // Log final results
    UE_LOG(LogTemp, Log, TEXT("=== QA VFX VALIDATION CYCLE 008 COMPLETE ==="));
    UE_LOG(LogTemp, Log, TEXT("Status: %s"), *GetValidationStatusString());
    UE_LOG(LogTemp, Log, TEXT("Tests Passed: %d"), TestsPassed);
    UE_LOG(LogTemp, Log, TEXT("Tests Failed: %d"), TestsFailed);
    UE_LOG(LogTemp, Log, TEXT("Critical Issues: %d"), CriticalIssuesFound);
    UE_LOG(LogTemp, Log, TEXT("Warnings: %d"), WarningsFound);
}

FString UQA_VFXValidationReport_Cycle008::GetValidationStatusString() const
{
    switch (ValidationStatus)
    {
        case EQA_ValidationStatus::NotStarted:
            return TEXT("Not Started");
        case EQA_ValidationStatus::InProgress:
            return TEXT("In Progress");
        case EQA_ValidationStatus::Passed:
            return TEXT("Passed");
        case EQA_ValidationStatus::PassedWithWarnings:
            return TEXT("Passed with Warnings");
        case EQA_ValidationStatus::Failed:
            return TEXT("Failed");
        default:
            return TEXT("Unknown");
    }
}

FString UQA_VFXValidationReport_Cycle008::GenerateDetailedReport() const
{
    FString Report;
    Report += TEXT("=== QA VFX VALIDATION REPORT - CYCLE 008 ===\n");
    Report += FString::Printf(TEXT("Timestamp: %s\n"), *ReportTimestamp.ToString());
    Report += FString::Printf(TEXT("Status: %s\n"), *GetValidationStatusString());
    Report += FString::Printf(TEXT("Tests Passed: %d\n"), TestsPassed);
    Report += FString::Printf(TEXT("Tests Failed: %d\n"), TestsFailed);
    Report += FString::Printf(TEXT("Critical Issues: %d\n"), CriticalIssuesFound);
    Report += FString::Printf(TEXT("Warnings: %d\n"), WarningsFound);
    Report += TEXT("\n=== DETAILED RESULTS ===\n");
    
    for (const FQA_ValidationResult& Result : ValidationResults)
    {
        Report += FString::Printf(TEXT("[%s] %s: %s - %s\n"), 
            Result.bPassed ? TEXT("PASS") : TEXT("FAIL"),
            *Result.Category,
            *Result.TestName,
            *Result.Message);
    }
    
    return Report;
}