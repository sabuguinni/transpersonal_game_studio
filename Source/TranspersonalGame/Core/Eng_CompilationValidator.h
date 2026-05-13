#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_CompilationValidator.generated.h"

/**
 * ENGINE ARCHITECT CYCLE 004 - COMPILATION VALIDATOR
 * 
 * This system ensures that all C++ code follows UE5 compilation rules
 * and prevents common compilation errors that break the build.
 * 
 * CRITICAL FUNCTIONS:
 * 1. Validates USTRUCT/UENUM placement (must be global scope)
 * 2. Checks for unique type names across the project
 * 3. Validates .generated.h include order
 * 4. Ensures all .h files have matching .cpp files
 * 5. Validates UPROPERTY/UFUNCTION macro usage
 */

// Compilation Error Types
UENUM(BlueprintType)
enum class EEng_CompilationErrorType : uint8
{
    None                    UMETA(DisplayName = "None"),
    MissingCppFile         UMETA(DisplayName = "Missing Cpp File"),
    InvalidIncludeOrder    UMETA(DisplayName = "Invalid Include Order"),
    DuplicateTypeName      UMETA(DisplayName = "Duplicate Type Name"),
    InvalidMacroUsage      UMETA(DisplayName = "Invalid Macro Usage"),
    MissingGeneratedBody   UMETA(DisplayName = "Missing Generated Body"),
    InvalidStructPlacement UMETA(DisplayName = "Invalid Struct Placement"),
    ModuleDependencyError  UMETA(DisplayName = "Module Dependency Error")
};

// Compilation Validation Result
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationValidationResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString FileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EEng_CompilationErrorType ErrorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    int32 LineNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bIsError;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FDateTime ValidationTime;

    FEng_CompilationValidationResult()
    {
        FileName = TEXT("");
        ErrorType = EEng_CompilationErrorType::None;
        ErrorMessage = TEXT("");
        LineNumber = 0;
        bIsError = false;
        ValidationTime = FDateTime::Now();
    }
};

// File Validation Status
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_FileValidationStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "File")
    FString HeaderFile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "File")
    FString CppFile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "File")
    bool bHasMatchingCpp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "File")
    bool bHasValidIncludes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "File")
    bool bHasValidMacros;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "File")
    TArray<FString> TypesDeclarated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "File")
    TArray<FEng_CompilationValidationResult> ValidationErrors;

    FEng_FileValidationStatus()
    {
        HeaderFile = TEXT("");
        CppFile = TEXT("");
        bHasMatchingCpp = false;
        bHasValidIncludes = false;
        bHasValidMacros = false;
    }
};

/**
 * COMPILATION VALIDATOR SUBSYSTEM
 * 
 * Validates all C++ files in the project to ensure they follow
 * UE5 compilation rules and prevent build errors.
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_CompilationValidator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Validation Methods
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateAllSourceFiles();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateHeaderFile(const FString& HeaderPath);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateCppFile(const FString& CppPath);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FEng_FileValidationStatus> GetAllFileValidationStatus();

    // Specific Validation Rules
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateIncludeOrder(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateTypeNames(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateMacroUsage(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateModuleDependencies(const FString& FilePath);

    // Error Reporting
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FEng_CompilationValidationResult> GetValidationErrors();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    int32 GetErrorCount();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    int32 GetWarningCount();

    // File Management
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FString> FindOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FString> FindMissingCppFiles();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool CreateMissingCppStub(const FString& HeaderPath);

    // Type Registry Validation
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateUniqueTypeNames();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FString> FindDuplicateTypeNames();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateSharedTypesUsage();

protected:
    // Internal validation data
    UPROPERTY()
    TArray<FEng_FileValidationStatus> FileValidationStatuses;

    UPROPERTY()
    TArray<FEng_CompilationValidationResult> ValidationErrors;

    UPROPERTY()
    TMap<FString, FString> TypeNameRegistry;

    // Validation settings
    UPROPERTY(EditAnywhere, Category = "Validation")
    bool bAutoValidateOnStartup;

    UPROPERTY(EditAnywhere, Category = "Validation")
    bool bTreatWarningsAsErrors;

    UPROPERTY(EditAnywhere, Category = "Validation")
    float ValidationInterval;

    // Internal methods
    void ScanSourceDirectory(const FString& DirectoryPath);
    bool ValidateFileContent(const FString& FilePath, FEng_FileValidationStatus& OutStatus);
    void AddValidationError(const FString& FileName, EEng_CompilationErrorType ErrorType, 
                          const FString& Message, int32 LineNumber = 0);
    bool IsValidHeaderFile(const FString& FilePath);
    bool IsValidCppFile(const FString& FilePath);
    FString GetCorrespondingCppFile(const FString& HeaderPath);
    FString GetCorrespondingHeaderFile(const FString& CppPath);

private:
    // Validation timer
    FTimerHandle ValidationTimer;
    
    // Project paths
    FString ProjectSourcePath;
    FString TranspersonalGamePath;
};