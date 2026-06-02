#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "UObject/NoExportTypes.h"
#include "Build_CriticalCompilationFixer.generated.h"

UENUM(BlueprintType)
enum class EBuild_CompilationErrorType : uint8
{
    None = 0,
    MissingHeader,
    DuplicateType,
    InvalidSyntax,
    LinkingError,
    ModuleDependency,
    UHTError,
    GeneratedFileError,
    IncludeOrderError
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationError
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString FileName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 LineNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EBuild_CompilationErrorType ErrorType;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString SuggestedFix;

    FBuild_CompilationError()
    {
        FileName = TEXT("");
        LineNumber = 0;
        ErrorMessage = TEXT("");
        ErrorType = EBuild_CompilationErrorType::None;
        SuggestedFix = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationFixReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FBuild_CompilationError> Errors;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> FixedFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bAllErrorsFixed;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float FixDurationSeconds;

    FBuild_CompilationFixReport()
    {
        bAllErrorsFixed = false;
        FixDurationSeconds = 0.0f;
    }
};

/**
 * Critical compilation fixer that automatically resolves common UE5 C++ compilation errors
 * Focuses on header issues, duplicate types, and missing implementations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_CriticalCompilationFixer : public UObject
{
    GENERATED_BODY()

public:
    UBuild_CriticalCompilationFixer();

    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor = true)
    FBuild_CompilationFixReport FixAllCompilationErrors();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool FixMissingHeaders();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool FixDuplicateTypes();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool FixIncludeOrder();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool FixMissingImplementations();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_CompilationError> ScanForErrors();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateModuleDependencies();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_CompilationError> LastErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FDateTime LastScanTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bAutoFixEnabled;

private:
    bool FixSpecificError(const FBuild_CompilationError& Error);
    FString GenerateFixSuggestion(const FBuild_CompilationError& Error);
    bool ApplyAutomaticFix(const FBuild_CompilationError& Error);
    TArray<FString> GetAllSourceFiles();
    bool IsHeaderFile(const FString& FilePath);
    bool IsCppFile(const FString& FilePath);
    FString GetCorrespondingCppFile(const FString& HeaderPath);
    FString GetCorrespondingHeaderFile(const FString& CppPath);
};