#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/PlatformProcess.h"
#include "Build_CompilationManager.generated.h"

/**
 * Compilation status enumeration
 */
UENUM(BlueprintType)
enum class EBuild_CompilationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Success         UMETA(DisplayName = "Success"),
    Failed          UMETA(DisplayName = "Failed"),
    Cancelled       UMETA(DisplayName = "Cancelled")
};

/**
 * Compilation error severity levels
 */
UENUM(BlueprintType)
enum class EBuild_ErrorSeverity : uint8
{
    Info            UMETA(DisplayName = "Info"),
    Warning         UMETA(DisplayName = "Warning"),
    Error           UMETA(DisplayName = "Error"),
    Fatal           UMETA(DisplayName = "Fatal")
};

/**
 * Individual compilation error structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationError
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EBuild_ErrorSeverity Severity;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString FileName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 LineNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ErrorCode;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString FullErrorText;

    FBuild_CompilationError()
    {
        Severity = EBuild_ErrorSeverity::Info;
        FileName = TEXT("");
        LineNumber = 0;
        ErrorCode = TEXT("");
        ErrorMessage = TEXT("");
        FullErrorText = TEXT("");
    }
};

/**
 * Comprehensive compilation result structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EBuild_CompilationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float CompilationTimeSeconds;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 TotalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 TotalWarnings;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 FilesCompiled;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FBuild_CompilationError> Errors;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FDateTime CompilationTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString BuildConfiguration;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString CompilerVersion;

    FBuild_CompilationResult()
    {
        Status = EBuild_CompilationStatus::Unknown;
        CompilationTimeSeconds = 0.0f;
        TotalErrors = 0;
        TotalWarnings = 0;
        FilesCompiled = 0;
        CompilationTimestamp = FDateTime::Now();
        BuildConfiguration = TEXT("Development");
        CompilerVersion = TEXT("Unknown");
    }
};

/**
 * Compilation Manager - Handles build compilation and error reporting
 * Integrates with UBT (Unreal Build Tool) to manage compilation process
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_CompilationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_CompilationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Start compilation of the TranspersonalGame module
     * @param bCleanBuild Whether to perform a clean build
     * @return True if compilation started successfully
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation", CallInEditor = true)
    bool StartCompilation(bool bCleanBuild = false);

    /**
     * Check current compilation status
     * @return Current compilation status
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    EBuild_CompilationStatus GetCompilationStatus() const;

    /**
     * Get last compilation result
     * @return Most recent compilation result
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    FBuild_CompilationResult GetLastCompilationResult() const { return LastCompilationResult; }

    /**
     * Cancel ongoing compilation
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void CancelCompilation();

    /**
     * Validate source files before compilation
     * @return True if all source files are valid
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation", CallInEditor = true)
    bool ValidateSourceFiles();

    /**
     * Get compilation errors by severity
     * @param Severity Error severity to filter by
     * @return Array of errors matching the severity
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FBuild_CompilationError> GetErrorsBySeverity(EBuild_ErrorSeverity Severity) const;

    /**
     * Check if compilation is currently in progress
     * @return True if compilation is running
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool IsCompilationInProgress() const;

    /**
     * Force refresh of compilation status
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void RefreshCompilationStatus();

protected:
    /**
     * Parse compilation output for errors and warnings
     * @param CompilationOutput Raw output from compiler
     */
    void ParseCompilationOutput(const FString& CompilationOutput);

    /**
     * Parse individual error line
     * @param ErrorLine Single line from compilation output
     * @return Parsed compilation error
     */
    FBuild_CompilationError ParseErrorLine(const FString& ErrorLine);

    /**
     * Update compilation progress
     */
    void UpdateCompilationProgress();

    /**
     * Handle compilation completion
     * @param bSuccess Whether compilation succeeded
     */
    void OnCompilationComplete(bool bSuccess);

private:
    UPROPERTY()
    FBuild_CompilationResult LastCompilationResult;

    UPROPERTY()
    EBuild_CompilationStatus CurrentStatus;

    UPROPERTY()
    FDateTime CompilationStartTime;

    UPROPERTY()
    FProcHandle CompilationProcess;

    UPROPERTY()
    bool bCompilationInProgress;

    UPROPERTY()
    FString ProjectPath;

    UPROPERTY()
    FString SourcePath;

    // Timer for checking compilation progress
    FTimerHandle CompilationCheckTimer;
};