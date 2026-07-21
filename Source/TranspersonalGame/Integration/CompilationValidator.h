#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "CompilationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationError
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ErrorType;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString FileName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 LineNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString SuggestedFix;

    FBuild_CompilationError()
    {
        ErrorType = TEXT("Unknown");
        FileName = TEXT("");
        LineNumber = 0;
        ErrorMessage = TEXT("");
        SuggestedFix = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bCompilationSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalWarnings;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_CompilationError> Errors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> MissingImplementations;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> DuplicateDefinitions;

    FBuild_ValidationResult()
    {
        bCompilationSuccessful = false;
        TotalErrors = 0;
        TotalWarnings = 0;
    }
};

/**
 * Compilation Validator - Validates C++ code compilation and identifies common issues
 * Part of the Integration & Build system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCompilationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCompilationValidator();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    FBuild_ValidationResult ValidateProjectCompilation();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    bool CheckHeaderImplementationPairs();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    TArray<FString> FindMissingImplementations();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    TArray<FString> FindDuplicateDefinitions();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    bool ValidateIncludePaths();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    bool ValidateUE5APIUsage();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    void GenerateCompilationReport();

    // Error Analysis
    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    TArray<FBuild_CompilationError> AnalyzeCompilationErrors(const FString& LogContent);

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    FString SuggestFixForError(const FBuild_CompilationError& Error);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Compilation Validation")
    FBuild_ValidationResult LastValidationResult;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation Validation")
    TArray<FString> KnownSourceFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation Validation")
    TArray<FString> KnownHeaderFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation Validation")
    bool bValidationInProgress;

private:
    void ScanSourceFiles();
    void ValidateFileStructure();
    bool CheckFileExists(const FString& FilePath);
    FString GetImplementationPath(const FString& HeaderPath);
    FString GetHeaderPath(const FString& ImplementationPath);
    void LogValidationResults();
};