#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_CompilationValidator.generated.h"

/**
 * ENGINE ARCHITECT CYCLE 009 - COMPILATION VALIDATOR
 * 
 * Critical system for ensuring all C++ code compiles correctly and
 * all UCLASS/USTRUCT/UENUM types are properly registered.
 * 
 * This validator runs automatically and reports compilation issues
 * to prevent broken builds from propagating through the agent chain.
 */

// Compilation Issue Severity
UENUM(BlueprintType)
enum class EEng_CompilationSeverity : uint8
{
    Info            UMETA(DisplayName = "Info"),
    Warning         UMETA(DisplayName = "Warning"),
    Error           UMETA(DisplayName = "Error"),
    Critical        UMETA(DisplayName = "Critical")
};

// Compilation Issue Type
UENUM(BlueprintType)
enum class EEng_CompilationIssueType : uint8
{
    MissingHeader       UMETA(DisplayName = "Missing Header"),
    MissingImplementation UMETA(DisplayName = "Missing Implementation"),
    TypeConflict        UMETA(DisplayName = "Type Conflict"),
    DependencyError     UMETA(DisplayName = "Dependency Error"),
    LinkageError        UMETA(DisplayName = "Linkage Error"),
    GeneratedCodeError  UMETA(DisplayName = "Generated Code Error"),
    BlueprintError      UMETA(DisplayName = "Blueprint Error")
};

// Compilation Issue Data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationIssue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    FString FileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    int32 LineNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    EEng_CompilationSeverity Severity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    EEng_CompilationIssueType IssueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    FString SuggestedFix;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    FDateTime DetectedTime;

    FEng_CompilationIssue()
    {
        FileName = TEXT("Unknown");
        LineNumber = 0;
        Severity = EEng_CompilationSeverity::Info;
        IssueType = EEng_CompilationIssueType::MissingHeader;
        Description = TEXT("No description");
        SuggestedFix = TEXT("No suggestion");
        DetectedTime = FDateTime::Now();
    }
};

// File Validation Status
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_FileValidationStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString FilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bHasHeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bHasImplementation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bCompilesClean;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FEng_CompilationIssue> Issues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FDateTime LastValidated;

    FEng_FileValidationStatus()
    {
        FilePath = TEXT("Unknown");
        bHasHeader = false;
        bHasImplementation = false;
        bCompilesClean = false;
        LastValidated = FDateTime::Now();
    }
};

/**
 * COMPILATION VALIDATOR SUBSYSTEM
 * 
 * Monitors the codebase for compilation issues and provides
 * real-time feedback to agents about code quality.
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
    bool ValidateAllFiles();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateFile(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FEng_FileValidationStatus> GetAllFileStatuses();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    FEng_FileValidationStatus GetFileStatus(const FString& FilePath);

    // Issue Management
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FEng_CompilationIssue> GetAllIssues();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FEng_CompilationIssue> GetIssuesBySeverity(EEng_CompilationSeverity Severity);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    int32 GetTotalIssueCount();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    int32 GetCriticalIssueCount();

    // Compilation Status
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool IsCodebaseHealthy();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    float GetCodeHealthScore();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    FString GetCompilationSummary();

    // Type Registry Validation
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateUClassRegistry();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateUStructRegistry();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateUEnumRegistry();

    // Header/Implementation Pairing
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FString> GetOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FString> GetMissingImplementations();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateHeaderImplementationPairs();

    // Automated Fixes
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool AttemptAutomaticFixes();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool FixMissingIncludes();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool FixGeneratedIncludes();

protected:
    // File tracking
    UPROPERTY()
    TMap<FString, FEng_FileValidationStatus> FileStatuses;

    // Issue tracking
    UPROPERTY()
    TArray<FEng_CompilationIssue> AllIssues;

    // Validation settings
    UPROPERTY(EditAnywhere, Category = "Validation")
    bool bAutoValidateOnChange;

    UPROPERTY(EditAnywhere, Category = "Validation")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, Category = "Validation")
    bool bAttemptAutoFix;

    UPROPERTY(EditAnywhere, Category = "Validation")
    TArray<FString> ExcludedDirectories;

    // Internal methods
    void ScanSourceDirectory();
    void ValidateHeaderFile(const FString& HeaderPath);
    void ValidateImplementationFile(const FString& CppPath);
    void CheckHeaderImplementationPair(const FString& BaseName);
    void AddCompilationIssue(const FEng_CompilationIssue& Issue);
    void ClearIssuesForFile(const FString& FilePath);
    bool IsFileExcluded(const FString& FilePath);
    FString GetBaseName(const FString& FilePath);

private:
    // Validation timer
    FTimerHandle ValidationTimer;
    
    // Statistics
    int32 TotalFilesScanned;
    int32 TotalIssuesFound;
    FDateTime LastFullValidation;
};