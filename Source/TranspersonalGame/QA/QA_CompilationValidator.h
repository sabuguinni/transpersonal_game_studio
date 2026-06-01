#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "QA_CompilationValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_CompilationStatus : uint8
{
    Unknown UMETA(DisplayName = "Unknown"),
    Compiling UMETA(DisplayName = "Compiling"),
    Success UMETA(DisplayName = "Success"),
    Failed UMETA(DisplayName = "Failed"),
    Warning UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_CompilationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EQA_CompilationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 WarningCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> WarningMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float CompilationTimeSeconds;

    FQA_CompilationResult()
    {
        Status = EQA_CompilationStatus::Unknown;
        ModuleName = TEXT("");
        ErrorCount = 0;
        WarningCount = 0;
        CompilationTimeSeconds = 0.0f;
    }
};

/**
 * QA Compilation Validator - Validates C++ module compilation status
 * Monitors compilation errors, warnings, and build times
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_CompilationValidator : public UObject
{
    GENERATED_BODY()

public:
    UQA_CompilationValidator();

    // Validate compilation status of all TranspersonalGame modules
    UFUNCTION(BlueprintCallable, Category = "QA|Compilation")
    bool ValidateAllModules();

    // Validate specific module compilation
    UFUNCTION(BlueprintCallable, Category = "QA|Compilation")
    FQA_CompilationResult ValidateModule(const FString& ModuleName);

    // Check for compilation errors in the output log
    UFUNCTION(BlueprintCallable, Category = "QA|Compilation")
    bool CheckCompilationErrors();

    // Get last compilation results
    UFUNCTION(BlueprintCallable, Category = "QA|Compilation")
    TArray<FQA_CompilationResult> GetLastCompilationResults() const { return LastCompilationResults; }

    // Force recompile of all modules
    UFUNCTION(BlueprintCallable, Category = "QA|Compilation", CallInEditor = true)
    void ForceRecompileAllModules();

    // Get compilation statistics
    UFUNCTION(BlueprintCallable, Category = "QA|Compilation")
    void GetCompilationStats(int32& TotalModules, int32& SuccessfulModules, int32& FailedModules);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA|Compilation")
    TArray<FQA_CompilationResult> LastCompilationResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA|Compilation")
    TArray<FString> ModulesToValidate;

    UPROPERTY(BlueprintReadOnly, Category = "QA|Compilation")
    bool bIsValidating;

    UPROPERTY(BlueprintReadOnly, Category = "QA|Compilation")
    float LastValidationTime;

private:
    // Parse compilation output for errors and warnings
    FQA_CompilationResult ParseCompilationOutput(const FString& ModuleName, const FString& Output);

    // Initialize list of modules to validate
    void InitializeModuleList();

    // Check if module exists and is loadable
    bool IsModuleValid(const FString& ModuleName);
};