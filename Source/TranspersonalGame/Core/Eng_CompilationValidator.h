#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_CompilationValidator.generated.h"

/**
 * ENGINE ARCHITECT CYCLE 001 - COMPILATION VALIDATOR
 * 
 * This system ensures that all C++ code written by agents follows
 * UE5 compilation rules and architectural standards. It validates:
 * 1. Header/implementation file pairing
 * 2. UE5 macro usage (UCLASS, USTRUCT, UENUM)
 * 3. Include dependencies and module references
 * 4. Type naming conventions and conflicts
 * 5. Blueprint exposure and editor integration
 * 
 * CRITICAL: This runs automatically after each agent cycle
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
    MissingImplementation   UMETA(DisplayName = "Missing Implementation"),
    InvalidMacroUsage       UMETA(DisplayName = "Invalid Macro Usage"),
    IncludeDependency       UMETA(DisplayName = "Include Dependency"),
    TypeNamingConflict      UMETA(DisplayName = "Type Naming Conflict"),
    ModuleDependency        UMETA(DisplayName = "Module Dependency"),
    BlueprintExposure       UMETA(DisplayName = "Blueprint Exposure"),
    PerformanceIssue        UMETA(DisplayName = "Performance Issue"),
    ArchitecturalViolation  UMETA(DisplayName = "Architectural Violation")
};

// Compilation Issue Data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationIssue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Issue")
    FString FilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Issue")
    int32 LineNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Issue")
    EEng_CompilationSeverity Severity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Issue")
    EEng_CompilationIssueType IssueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Issue")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Issue")
    FString SuggestedFix;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Issue")
    FString AgentResponsible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Issue")
    FDateTime DetectedAt;

    FEng_CompilationIssue()
    {
        LineNumber = 0;
        Severity = EEng_CompilationSeverity::Info;
        IssueType = EEng_CompilationIssueType::MissingImplementation;
        DetectedAt = FDateTime::Now();
    }
};

// File Validation Result
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_FileValidationResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString FilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bHasImplementation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bValidMacroUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bValidIncludes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bValidTypeNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FEng_CompilationIssue> Issues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FDateTime LastValidated;

    FEng_FileValidationResult()
    {
        bHasImplementation = false;
        bValidMacroUsage = false;
        bValidIncludes = false;
        bValidTypeNames = false;
        LastValidated = FDateTime::Now();
    }
};

/**
 * COMPILATION VALIDATOR SUBSYSTEM
 * 
 * This subsystem validates all C++ code for compilation safety
 * and architectural compliance. It runs after each agent cycle.
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
    FEng_FileValidationResult ValidateSourceFile(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateHeaderImplementationPairing();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateUE5MacroUsage();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateIncludeDependencies();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateTypeNamingConventions();

    // Issue Management
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FEng_CompilationIssue> GetAllIssues();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FEng_CompilationIssue> GetIssuesBySeverity(EEng_CompilationSeverity Severity);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FEng_CompilationIssue> GetIssuesByAgent(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    int32 GetIssueCount();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    int32 GetCriticalIssueCount();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    FString GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool SaveValidationReport(const FString& FilePath);

    // Auto-Fix Capabilities
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool AttemptAutoFix(const FEng_CompilationIssue& Issue);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    int32 AttemptAutoFixAll();

    // Agent Integration
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateAgentOutput(const FString& AgentName, const TArray<FString>& FilePaths);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool BlockAgentIfCriticalIssues(const FString& AgentName);

protected:
    // Internal validation data
    UPROPERTY()
    TArray<FEng_CompilationIssue> DetectedIssues;

    UPROPERTY()
    TMap<FString, FEng_FileValidationResult> FileValidationCache;

    UPROPERTY()
    TArray<FString> KnownTypeNames;

    UPROPERTY()
    TArray<FString> RequiredIncludes;

    // Validation settings
    UPROPERTY(EditAnywhere, Category = "Validation")
    bool bEnableAutoValidation;

    UPROPERTY(EditAnywhere, Category = "Validation")
    bool bEnableAutoFix;

    UPROPERTY(EditAnywhere, Category = "Validation")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, Category = "Validation")
    bool bBlockOnCriticalIssues;

    // Internal methods
    void ScanSourceDirectory();
    void ValidateFileContent(const FString& FilePath, const FString& Content);
    void CheckHeaderImplementationPair(const FString& HeaderPath);
    void CheckUE5MacroSyntax(const FString& FilePath, const FString& Content);
    void CheckIncludeStatements(const FString& FilePath, const FString& Content);
    void CheckTypeNaming(const FString& FilePath, const FString& Content);
    void AddIssue(const FEng_CompilationIssue& Issue);
    void LogValidationEvent(const FString& Event, const FString& Details);

private:
    // Validation timer
    FTimerHandle ValidationTimer;
    
    // File system monitoring
    TArray<FString> MonitoredDirectories;
    TMap<FString, FDateTime> FileModificationTimes;
};